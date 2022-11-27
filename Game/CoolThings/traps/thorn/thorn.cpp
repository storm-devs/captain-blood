#include "Thorn.h"
#include "..\..\..\..\common_h\icharacter.h"

TrapThorn::TrapThorn()
{
	m_fStateTimer = -1.f;
	m_fActiveTime = 0.f;	
	m_complexData.warningTime = 0.f;
	m_complexData.damage = 0.f;
	m_heights.fDisable = m_heights.fWarning = m_heights.fActive = 0.f;
	m_curState = state_inactive;
	m_bMoving = false;
	ZERO( m_sounds );
}

TrapThorn::~TrapThorn()
{
	// unregistry tarp from pattern
	if( m_PatternObj.Validate() )
	{
		((ThornPattern*)m_PatternObj.Ptr())->UnregistryTrap( this );
		m_PatternObj.Reset();
	}
}

void TrapThorn::Show(bool isShow)
{
	TrapBase::Show(isShow);
}

void TrapThorn::Activate(bool isActive)
{
	TrapBase::Activate(isActive);
	if( isActive && m_curState == state_inactive )
	{
		SwitchState(state_warning);
		Sound().Create3D( m_sounds.pcStartMove, GetTransform().pos, _FL_ );
	}
	if( !isActive && m_curState != state_inactive )
	{
		ClearExcludeList();
		m_fStateTimer = -1.f;
		m_fCurHeight = m_heights.fDisable;
		m_bMoving = false;
		SetTransform( GetCurrentMatrix() );
		SetDamageActive(false);
		m_curState = state_inactive;
	}
}

bool TrapThorn::Create(MOPReader & reader)
{
	ReadMOPs(reader);
	return true;
}

bool TrapThorn::EditMode_Create(MOPReader & reader)
{
	ReadMOPs(reader);
	UpdatePattern();
	return true;
}

void TrapThorn::PostCreate()
{
	UpdatePattern();
}

bool TrapThorn::EditMode_Update(MOPReader & reader)
{
	ReadMOPs(reader);
	UpdatePattern();
	return true;
}

void TrapThorn::InflictDamage(DamageReceiver* pObj, TrapPatternBase::CharReactionData& reactions)
{
	if( !pObj->IsActive() || pObj->GetHP()<=0.f ) return;
	Matrix m(true);
	pObj->GetMatrix(m);
	if( pObj->Is(ICharacter::GetTypeId()) )
	{
		((ICharacter*)pObj)->Hit(DamageReceiver::ds_trap_stakes, m_complexData.damage, reactions.pcHitReaction, m_complexData.damage, reactions.pcBlockReaction, reactions.pcDieReaction );
		AddExcludeObj(pObj,m_complexData.m_fImmuneTime);
	}
	else if( pObj->Attack(this, DamageReceiver::ds_trap_stakes, m_complexData.damage, m.pos, 2.f) )
		AddExcludeObj(pObj,m_complexData.m_fImmuneTime);
}

void TrapThorn::Frame(float fDeltaTime)
{
	// таймер перехода в следущее состояние
	if( m_fStateTimer >= 0.f )
	{
		m_fStateTimer -= fDeltaTime;
		if( m_fStateTimer <= 0.f )
		{
			// меняем состояние
			SwitchState( m_curState == state_warning ? state_active : state_inactive );
		}
	}

	// движение
	if( m_bMoving )
	{
		float fh = m_fHeightSpeed * fDeltaTime + m_fHeightAccelerate * fDeltaTime*fDeltaTime;
		m_fHeightSpeed += m_fHeightAccelerate * fDeltaTime;
		if( m_fCurHeight > m_fMaxHeight )
		{
			m_fCurHeight -= fh;
			if( m_fCurHeight <= m_fMaxHeight ) StopMove();
		}
		else
		{
			m_fCurHeight += fh;
			if( m_fCurHeight >= m_fMaxHeight ) StopMove();
		}
		SetTransform( GetCurrentMatrix() );
	}
}

void TrapThorn::EditorDraw()
{
}

void TrapThorn::UpdatePattern()
{
	FindObject(m_pcPatternName, m_PatternObj);
	static const ConstString tid("ThornPattern");
	if( m_PatternObj.Ptr() && m_PatternObj.Ptr()->Is(tid) )
	{
		ThornPattern* pPattern = (ThornPattern*)m_PatternObj.Ptr();
		pPattern->RegistryTrap( this );
		SetStandartParams( (TrapPatternBase*)pPattern );
		m_heights = pPattern->GetHeights();
		m_fActiveTime = pPattern->GetActiveTime();		
		m_fCurHeight = m_heights.fDisable;
		m_sounds = pPattern->GetSounds();
		pPattern->GetComplexityData(m_complexData);
		// берем модельку
		SetModel( pPattern->GetModel() );
		SetTransform( GetCurrentMatrix() );
		SetDamageBox( pPattern->GetDamageBoxSize(), pPattern->GetDamageBoxCenter() );
	}
	else
	{
		m_PatternObj.Reset();
		SetModel( NULL );
	}
}

void TrapThorn::SwitchState(State st)
{
	switch(st)
	{
	case state_inactive:
		TrapBase::Activate(false);
		ClearExcludeList();
		m_fStateTimer = -1.f;
		StartMoving(m_heights.fActive, m_heights.fDisable, 0.f, m_heights.fDisableAccelerate);
		SetDamageActive(false);
		SetSound( Sound().Create3D( m_sounds.pcDeactivate, GetTransform().pos, _FL_, true, false ) );
	break;
	case state_warning:
		m_fStateTimer = m_complexData.warningTime;
		StartMoving(m_heights.fDisable, m_heights.fWarning, 0.f, m_heights.fWarningAccelerate);
		SetDamageActive(false);
		SetSound( Sound().Create3D( m_sounds.pcWarning, GetTransform().pos, _FL_, true, false ) );
	break;
	case state_active:
		m_fStateTimer = m_fActiveTime;
		StartMoving(m_heights.fWarning, m_heights.fActive, 0.f, m_heights.fActiveAccelerate);
		SetDamageActive(true);
		SetSound( Sound().Create3D( m_sounds.pcActivate, GetTransform().pos, _FL_, true, false ) );
		ClearExcludeList();
	break;
	}
	m_curState = st;
}

void TrapThorn::ReadMOPs(MOPReader & reader)
{
	m_pcPatternName = reader.String();

	Vector pos = reader.Position();
	Vector ang = reader.Angles();
	m_mtxBaseTransform.Build( ang, pos );

	Show( reader.Bool() );
	Activate( reader.Bool() );
}

Matrix TrapThorn::GetCurrentMatrix()
{
	Matrix m = m_mtxBaseTransform;
	m.pos += m_mtxBaseTransform.MulNormal( Vector(0.f,m_fCurHeight,0.f) );
	return m;
}

void TrapThorn::StartMoving(float fStartHeight, float fEndHeight, float speed, float accelerate)
{
	m_bMoving = true;
	m_fHeightSpeed = speed;
	m_fHeightAccelerate = accelerate;
	m_fCurHeight = fStartHeight;
	m_fMaxHeight = fEndHeight;
	SetTransform( GetCurrentMatrix() );
}

MOP_BEGINLISTCG(TrapThorn, "Thorn trap", '1.00', 0, "Object for trap thorn", "Default")
	MOP_STRINGC("Pattern", "Thorn pattern", "Pattern name for trap")
	MOP_POSITIONC("Position", Vector(0.f), "Trap position")
	MOP_ANGLESC("Angle", Vector(0.f), "Trap angle")
	MOP_BOOLC("Show", true, "Show trap")
	MOP_BOOLC("Active", false, "Activate trap")
MOP_ENDLIST(TrapThorn)
