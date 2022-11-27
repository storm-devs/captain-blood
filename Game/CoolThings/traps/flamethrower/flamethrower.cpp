#include "Flamethrower.h"
#include "..\..\..\..\common_h\icharacter.h"

TrapFlamethrower::TrapFlamethrower()
{
	m_flame.moveAngle = 0.f;
	m_flame.damageAngle = 0.f;
	m_flame.distance = 0.f;
	m_flame.startAng = 0.f;
	m_flame.rotateSpeed = 0.f;
	m_complexity.prepareTime = 0.f;
	m_complexity.activeTime = 0.f;
	m_complexity.damage = 0.f;
	m_complexity.immuneTime = 0.f;
	m_particles.prepare.sfx = "";
	m_particles.active.sfx = "";
	m_particles.trackPrepare.sfx = "";
	m_particles.track.sfx = "";
	m_sounds.pcActive = "";
	m_sounds.pcDeactivate = "";
	m_sounds.pcPrepare = "";
	m_fStateTimer = -1.f;
	m_curState = state_inactive;
	m_bMove = false;
	m_fCurPosition = 0.f;
	m_fCurSpeed = 0.f;
	m_bMoveForward = true;
	m_fMakeTrackTime = -1.f;
}

TrapFlamethrower::~TrapFlamethrower()
{
	MOSafePointer mo;
	FindObject( m_pcPattern, mo );
	static const ConstString tid("FlametrapPattern");
	if( mo.Ptr() && mo.Ptr()->Is(tid) )
		((FlametrapPattern*)mo.Ptr())->UnregistryTrap( this );
}

void TrapFlamethrower::Show(bool isShow)
{
	TrapBase::Show( isShow );
}

void TrapFlamethrower::Activate(bool isActive)
{
	MOSafePointer sp;
	if( EditMode_IsOn() && !FindObject( m_pcPattern, sp ) )
		isActive = false;

	TrapBase::Activate( isActive );

	if( m_pcTieObjectName.NotEmpty() )
		SwitchState(isActive ? state_active : state_inactive);
	else
		if( isActive && m_curState==state_inactive )
			SwitchState(state_warning);
}

//Инициализировать объект
bool TrapFlamethrower::Create(MOPReader & reader)
{
	ReadMOPs(reader);
	return true;
}

// Инициализировать в режиме редактора
bool TrapFlamethrower::EditMode_Create(MOPReader & reader)
{
	ReadMOPs(reader);
	UpdatePattern();
	return true;
}

//Вызываеться, когда все объекты созданны но ещё не началось исполнение миссии
void TrapFlamethrower::PostCreate()
{
	UpdatePattern();
}

//Обновить параметры
bool TrapFlamethrower::EditMode_Update(MOPReader & reader)
{
	ReadMOPs(reader);
	UpdatePattern();
	return true;
}

void TrapFlamethrower::Command(const char * id, dword numParams, const char ** params)
{
	if( !id ) return;
	if( id[0] == 'm' && string::IsEqual(id,"mode") )
	{
		if( numParams>0 && params[0] )
		{
			if( params[0][0] == 'i' && string::IsEqual(params[0],"inactive") )
				SwitchState( state_inactive );
			else if( params[0][0] == 'a' && string::IsEqual(params[0],"active") )
				SwitchState( state_active );
			else if( params[0][0] == 'w' && string::IsEqual(params[0],"warning") )
				SwitchState( state_warning );
		}
		
	}
}

// реализация дамага от конкретной ловушки
void TrapFlamethrower::InflictDamage(DamageReceiver* pObj, TrapPatternBase::CharReactionData& reactions)
{
	if( !pObj ) return;

	// матрица цели
	Matrix mtxObj(true);
	pObj->GetMatrix( mtxObj );

	Vector vmin,vmax;
	pObj->GetBox( vmin, vmax );

	// матрица направления линии огнемета
	Matrix mtxDir = GetTransform();// Matrix( m_flame.moveAngle * m_fCurPosition ) * m_mtxBaseTransform;
	// коллизия
	bool bCollision = false;
	float fcos = cosf(m_flame.damageAngle);
	Vector vtargdir;
	// проверяем дистанцию
	vtargdir = mtxDir.MulVertexByInverse( mtxObj.MulVertex(vmin) );
	if( vtargdir.Normalize() <= m_flame.distance )
		// косинус угла между направлением на цель и направлением огнемета
		if( vtargdir.z >= fcos )
			bCollision = true;
	vtargdir = mtxDir.MulVertexByInverse( mtxObj.MulVertex(vmax) );
	if( !bCollision && vtargdir.Normalize() <= m_flame.distance )
		// косинус угла между направлением на цель и направлением огнемета
		if( vtargdir.z >= fcos )
			bCollision = true;
	vtargdir = mtxDir.MulVertexByInverse( mtxObj.pos );
	if( !bCollision && vtargdir.Normalize() <= m_flame.distance )
		// косинус угла между направлением на цель и направлением огнемета
		if( vtargdir.z >= fcos )
			bCollision = true;

	// наносим дамаг
	if( pObj->Is(ICharacter::GetTypeId()) )
	{
		((ICharacter*)pObj)->Hit( DamageReceiver::ds_trap_flame, m_complexity.damage, reactions.pcHitReaction, m_complexity.damage, reactions.pcBlockReaction, reactions.pcDieReaction );
		AddExcludeObj(pObj, m_complexity.immuneTime);
	}
	else if( pObj->Attack(this, DamageReceiver::ds_trap_flame, m_complexity.damage, mtxObj.pos, 2.f) )
		AddExcludeObj(pObj, m_complexity.immuneTime);
}

// обработка на кадре
void TrapFlamethrower::Frame(float fDeltaTime)
{
	if( m_TieObject.Ptr() )
	{
		if( m_TieObject.Validate() )
		{
			Matrix m(true);
			m_TieObject.Ptr()->GetMatrix(m);
			SetTransform( m );
			return;
		}
	}

	// смена режима по таймауту
	if( m_fStateTimer >= 0.f )
	{
		m_fStateTimer -= fDeltaTime;
		if( m_fStateTimer <= 0.f )
		{
			if( m_curState == state_warning )
				SwitchState( state_active );
			else
				SwitchState( state_inactive );
		}
	}

	// поджог пути прохождения струи пламени
	if( m_fMakeTrackTime >= 0.f )
	{
		m_fMakeTrackTime -= fDeltaTime;
		if( m_fMakeTrackTime <= 0.f )
		{
			m_fMakeTrackTime = m_particles.trackUpdateTime;
			Vector vpos;
			if( GetPointOnCharacterPath(vpos) )
				LaunchParticle( 1, m_particles.track.sfx, m_particles.track.scale, m_particles.track.timescale, false, vpos );
		}
	}

	// движение
	if( m_bMove )
	{
		// ограничитель скорости (замедление в крайних позициях)
		float fSpeedLimit = fabs(m_flame.rotateSpeed) * (1.2f - m_fCurPosition*m_fCurPosition);
		// движение к максимальной позиции
		if( m_bMoveForward )
		{
			// набираем скорость
			if( m_fCurSpeed < fSpeedLimit )
			{
				m_fCurSpeed += fDeltaTime * fabs(m_flame.rotateSpeed);
				if( m_fCurSpeed > fSpeedLimit )
					m_fCurSpeed = fSpeedLimit;
			}
			else m_fCurSpeed = fSpeedLimit;

			// поворачиваем
			m_fCurPosition += m_fCurSpeed * fDeltaTime;
			// достигли края - разворачиваем движение
			if( m_fCurPosition >= 1.f )
			{
				m_fCurPosition = 1.f;
				m_fCurSpeed = 0.f;
				m_bMoveForward = false;
			}
		}
		// движение к минимальной позиции
		else
		{
			// набираем скорость
			if( m_fCurSpeed < fSpeedLimit )
			{
				m_fCurSpeed += fDeltaTime * fabs(m_flame.rotateSpeed);
				if( m_fCurSpeed > fSpeedLimit )
					m_fCurSpeed = fSpeedLimit;
			}
			else m_fCurSpeed = fSpeedLimit;

			// поворачиваем
			m_fCurPosition -= m_fCurSpeed * fDeltaTime;
			// достигли края - разворачиваем движение
			if( m_fCurPosition <= -1.f )
			{
				m_fCurPosition = -1.f;
				m_fCurSpeed = 0.f;
				m_bMoveForward = true;
			}
		}
		SetTransform( CalculateMatrix() );
	}
}

void TrapFlamethrower::EditorDraw()
{
	// draw move sector
	Matrix m( m_flame.moveAngle );
	Vector vMoveSector[3];
	vMoveSector[0] = 0.f;
	vMoveSector[1] = m.MulNormal( Vector(0.f,0.f,m_flame.distance) );
	vMoveSector[2] = m.MulNormalByInverse( Vector(0.f,0.f,m_flame.distance) );
	Render().DrawPolygon( vMoveSector, 3, 0x80808080, m_mtxBaseTransform );
	vMoveSector[0] = vMoveSector[1];
	vMoveSector[1] = 0.f;
	Render().DrawPolygon( vMoveSector, 3, 0x80808080, m_mtxBaseTransform );

	// draw start position
	m.Build( m_flame.moveAngle * m_flame.startAng );
	Render().DrawVector( m_mtxBaseTransform.pos,
		m_mtxBaseTransform.pos+(m*m_mtxBaseTransform).MulNormal(Vector(0.f,0.f,m_flame.distance*1.1f)), 0xFF0000FF );

	// draw damage zone
	Vector vDamageZone[6];
	vDamageZone[0] = 0.f;
	float fDelta = sinf(m_flame.damageAngle) * m_flame.distance;
	vDamageZone[1] = m.MulNormal( Vector(-fDelta,fDelta,m_flame.distance) );
	vDamageZone[2] = m.MulNormal( Vector(fDelta,fDelta,m_flame.distance) );
	vDamageZone[3] = m.MulNormal( Vector(fDelta,-fDelta,m_flame.distance) );
	vDamageZone[4] = m.MulNormal( Vector(-fDelta,-fDelta,m_flame.distance) );
	vDamageZone[5] = vDamageZone[1];
	Render().DrawPolygon( vDamageZone, 6, 0x80800000, m_mtxBaseTransform );
	Vector vTmp = vDamageZone[2];
	vDamageZone[2] = vDamageZone[4];
	vDamageZone[4] = vTmp;
	Render().DrawPolygon( vDamageZone, 6, 0x80800000, m_mtxBaseTransform );
}

void TrapFlamethrower::UpdatePattern()
{
	m_TieObject.Reset();
	if( m_pcTieObjectName.NotEmpty() )
		FindObject( m_pcTieObjectName, m_TieObject );

	MOSafePointer mo;
	FindObject( m_pcPattern, mo );
	static const ConstString tid("FlametrapPattern");
	if( mo.Ptr() && mo.Ptr()->Is(tid) )
	{
		FlametrapPattern* pPattern = (FlametrapPattern*)mo.Ptr();
		pPattern->RegistryTrap( this );
		SetStandartParams( pPattern );
		m_flame = pPattern->GetFlameParams();
		pPattern->GetComplexityData( m_complexity );
		m_particles = pPattern->GetParticlesData();
		m_sounds = pPattern->GetSoundsData();
		m_fCurPosition = m_flame.startAng;
		m_bMoveForward = m_flame.rotateSpeed > 0.f;
		SetModel( pPattern->GetModel() );
		SetTransform( CalculateMatrix() );
		UpdateDamageZone();
	}
	else
	{
		SetModel( NULL );
	}

	Activate( IsActive() );
}

void TrapFlamethrower::SwitchState(State st)
{
	if( m_curState == st ) return;
	Vector v;
	Vector vnull = 0.f;
	switch(st)
	{
	case state_inactive:
		m_fStateTimer = -1.f;
		LaunchParticle(0, "", 1.f, 1.f, true, vnull );
		LaunchParticle(1, "", 1.f, 1.f, false, vnull );
		SetMove( false );
		SetDamageActive( false );
		ClearExcludeList();
		m_fMakeTrackTime = -1.f;
		SetSound( Sound().Create3D( m_sounds.pcDeactivate, GetTransform().pos, _FL_ ) );
	break;
	case state_warning:
		m_fStateTimer = m_complexity.prepareTime;
		LaunchParticle(0, m_particles.prepare.sfx, m_particles.prepare.scale, m_particles.prepare.timescale, true, vnull);
		//LaunchParticle(1, "", 1.f, false, vnull );
		SetMove( false );
		SetDamageActive( false );
		m_fMakeTrackTime = -1.f;
		if( m_particles.trackPrepare.sfx && m_particles.trackPrepare.sfx[0] && GetPointOnCharacterPath(v) )
			LaunchParticle( 1, m_particles.trackPrepare.sfx, m_particles.trackPrepare.scale, m_particles.trackPrepare.timescale, false, v );
		SetSound( Sound().Create3D( m_sounds.pcPrepare, GetTransform().pos, _FL_ ) );
	break;
	case state_active:
		m_fStateTimer = m_complexity.activeTime;
		LaunchParticle(0, m_particles.active.sfx, m_particles.active.scale, m_particles.active.timescale, true, vnull);
		LaunchParticle(1, "", 1.f, 1.f, false, vnull );
		SetMove( true );
		SetDamageActive( true );
		ClearExcludeList();
		if( m_particles.track.sfx && m_particles.track.sfx[0] )
			m_fMakeTrackTime = m_particles.trackStartTime;
		else
			m_fMakeTrackTime = -1;
		SetSound( Sound().Create3D( m_sounds.pcActive, GetTransform().pos, _FL_, true, false ) );
	break;
	}
	m_curState = st;
}

void TrapFlamethrower::ReadMOPs(MOPReader & reader)
{
	m_pcPattern = reader.String();

	Vector pos = reader.Position();
	Vector ang = reader.Angles();
	m_mtxBaseTransform.Build( ang, pos );

	m_pcTieObjectName = reader.String();
	FindObject( m_pcTieObjectName, m_TieObject );

	Show( reader.Bool() );
	MissionObject::Activate( reader.Bool() );
}

void TrapFlamethrower::SetMove(bool bMove)
{
	m_bMove = bMove;
	m_fCurSpeed = 0.f;
	//float fAngleDiff = m_fCurAngle
}

Matrix TrapFlamethrower::CalculateMatrix()
{
	Matrix m( m_flame.moveAngle * m_fCurPosition );
	return m * m_mtxBaseTransform;
}

void TrapFlamethrower::LaunchParticle(long n, const char* sfx, float scale, float timescale, bool bLocal, const Vector& pos)
{
	if( !sfx )
	{
		SetParticle( n, NULL, pos, bLocal );
		return;
	}

	IParticleSystem* particle = Particles().CreateParticleSystemEx( sfx, _FL_ );
	SetParticle( n, particle, pos, bLocal );
	if( !particle ) return;

	particle->SetScale( scale );
	particle->SetTimeScale( timescale );
}

bool TrapFlamethrower::GetPointOnCharacterPath(Vector& pos)
{
	Matrix mtxDir = Matrix( m_flame.moveAngle * m_fCurPosition ) * m_mtxBaseTransform;
	IPhysicsScene::RaycastResult res;
	IPhysBase * physobject = Physics().Raycast(mtxDir.pos, mtxDir.MulVertex( Vector(0.f,0.f,m_flame.distance+1.f) ), phys_mask(phys_character), &res); //phys_bloodpatch
	if( physobject )
	{
		pos = res.position;
		return true;
	}
	return false;
}

void TrapFlamethrower::UpdateDamageZone()
{
	Vector vdir = Vector(0.f,0.f,m_flame.distance );
	float fs = sinf(m_flame.damageAngle);
	float fc = cosf(m_flame.damageAngle);
	Vector vr = Vector(m_flame.distance*fs*fc,m_flame.distance*fs,m_flame.distance*fc*fc );
	Vector vl = Vector(-vr.x,-vr.y,vr.z);

	Vector vs1 = 0.f;
	vs1.Min( vr );
	vs1.Min( vl );
	vs1.Min( vdir );
	Vector vs2 = 0.f;
	vs2.Max( vr );
	vs2.Max( vl );
	vs2.Max( vdir );

	Vector vCenter = (vs1 + vs2)*0.5f;
	Vector vSize = vs2 - vCenter;
	SetDamageBox( vSize, vCenter );
}

static char FlametrapDescription[] =
"-= Object for trap - flamethrower =-\n"
"Supported commands:\n"
"mode <mode> - switch trap mode (active,inactive,warning)";

MOP_BEGINLISTCG(TrapFlamethrower, "Flamethrower trap", '1.00', 0, FlametrapDescription, "Default")
	MOP_STRINGC("Pattern", "Flame trap pattern", "Pattern name for trap")
	MOP_POSITIONC("Position", Vector(0.f), "Trap position")
	MOP_ANGLESC("Angle", Vector(0.f), "Trap angle")
	MOP_STRINGC("Owner object", "", "Mission object which ruled transform of trap")
	MOP_BOOLC("Show", true, "Show trap")
	MOP_BOOLC("Active", false, "Activate trap")
MOP_ENDLIST(TrapFlamethrower)
