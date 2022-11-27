#include "TrapBase.h"

#define ML_TRAPDRAWLEVEL ML_DYNAMIC1

TrapBase::TrapBase() :
	m_aExcludeList(_FL_),
	m_aDelParticles(_FL_)
{
	m_pModel = NULL;
	m_bDamageActive = false;
	m_pSound = NULL;
	m_vDamageBoxCenter = 0.f;
	m_vDamageBoxSize = 0.f;

	m_reactions.pcHitReaction = "";
	m_reactions.pcBlockReaction = "";
	m_reactions.pcDieReaction = "";

	m_renderparams.color = (dword)0;
	m_renderparams.dynamicLighting = true;
	m_renderparams.shadowCast = false;
	m_renderparams.shadowReceive = false;
	m_renderparams.seaReflection = false;

	m_baseparams.isLocalParticles = false;
}

TrapBase::~TrapBase()
{
	RELEASE( m_pModel );
	RELEASE( m_pSound );
	ReleaseParticles();
}

void TrapBase::Show(bool isShow)
{
	DelUpdate(&TrapBase::Work);
	DelUpdate(&TrapBase::EditorWork);
	if( isShow )
	{
		SetUpdate(&TrapBase::Work, ML_TRAPDRAWLEVEL);
		if( EditMode_IsOn() )
			SetUpdate(&TrapBase::EditorWork, ML_TRAPDRAWLEVEL+1);
	}
	else
	{
		RELEASE( m_pSound );
		ReleaseParticles();
	}
	m_bDamageActive = false;

	MissionObject::Show( isShow );
}

void _cdecl TrapBase::Work(float fDeltaTime, long level)
{
	Matrix mOldTransform = m_mtxTransform;

	// освобождаем список исключенных объектов по таймауту
	for( long n=0; n<m_aExcludeList; n++ )
	{
		if( m_aExcludeList[n].time >= 0.f )
		{
			m_aExcludeList[n].time -= fDeltaTime;
			if( m_aExcludeList[n].time <= 0.f )
			{
				m_aExcludeList.DelIndex( n );
				n--;
			}
		}
	}

	// отработка на кадре
	Frame(fDeltaTime);

	// получаем скорость на этом кадре
	m_vSpeed = fDeltaTime>0.f ? (1.f/fDeltaTime) * (m_mtxTransform.pos - mOldTransform.pos) : 0.f;
	m_vSpeed.Clamp( -1000.f, 1000.f );

	// рисуем модельку
	if( m_pModel )
	{
		m_pModel->SetUserColor( m_renderparams.color );
		m_pModel->SetDynamicLightState( m_renderparams.dynamicLighting );

		m_pModel->SetTransform( m_mtxTransform );
		m_pModel->Draw();
	}

	// обновить позицию звука
	if( m_pSound )
	{
		m_pSound->SetPosition( m_mtxTransform.pos );
	}
	// обновить позицию партиклов
	TransformParticles( m_mtxTransform );
	// удаляемые партиклы
	for( long np=0; np<m_aDelParticles; np++ )
	{
		m_aDelParticles[np].time -= fDeltaTime;
		if( m_aDelParticles[np].time <= 0.f )
		{
			RELEASE( m_aDelParticles[np].particle );
			m_aDelParticles.DelIndex( np );
			np--;
		}
	}
	// текущие рабочие партиклы
	for( np=0; np<maxparticles; np++ )
		if( m_pParticle[np].p && !m_pParticle[np].p->IsLooped() && !m_pParticle[np].p->IsAlive() )
			m_pParticle[np].p->Restart(0);

	// ищем кому бы вломить
	if( m_bDamageActive )
	{
		Matrix mdb(m_mtxTransform);
		mdb.pos += m_mtxTransform.MulNormal(m_vDamageBoxCenter);

		// ищем по АВВ
		Vector abbMin, abbMax;
		Box::FindABBforOBB(m_mtxTransform, m_vDamageBoxCenter-m_vDamageBoxSize,m_vDamageBoxCenter+m_vDamageBoxSize, abbMin,abbMax);
		dword n = QTFindObjects(MG_DAMAGEACCEPTOR, abbMin, abbMax);
		for (dword i = 0; i < n; i++)
		{
			MissionObject& mo = QTGetObject(i)->GetMissionObject();
			if( IsExclude( (DamageReceiver*)&mo ) ) continue;
			MO_IS_IF_NOT(tid, "DamageReceiver", &mo)
			{
				continue;
			}
			// проверим по ОВВ
			Matrix mtx(true);
			Vector vmin,vmax;
			mo.GetMatrix(mtx);
			mo.GetBox(vmin,vmax);
			Vector vp[4];
			vp[0] = mtx.MulVertex( Vector( vmin.x, vmin.y, vmin.z ) );
			vp[1] = mtx.MulVertex( Vector( vmin.x, vmax.y, vmin.z ) );
			vp[2] = mtx.MulVertex( Vector( vmax.x, vmax.y, vmax.z ) );
			vp[3] = mtx.MulVertex( Vector( vmax.x, vmin.y, vmax.z ) );
			if( Box::OverlapsBoxPoly(mdb,m_vDamageBoxSize,vp) )
			{
				// пошлем его ловушке на получение кренделей
				InflictDamage( (DamageReceiver*)&mo, m_reactions );
			}
		}
	}
}

void _cdecl TrapBase::EditorWork(float fDeltaTime, long level)
{
	// рисуем дамаг ящик
	Render().DrawBox( m_vDamageBoxCenter-m_vDamageBoxSize, m_vDamageBoxCenter+m_vDamageBoxSize, GetTransform(), 0xFFFF0000 );

	// ловушка рисует что ей надо
	EditorDraw();
}

void _cdecl TrapBase::ShadowCast(const char * group, MissionObject * sender)
{
	if(!EditMode_IsVisible() || !IsShow()) return;
	if( m_pModel )
	{
		m_pModel->SetTransform(m_mtxTransform);
		const Vector & vMin = m_pModel->GetBound().vMin;
		const Vector & vMax = m_pModel->GetBound().vMax;
		((MissionShadowCaster *)sender)->AddObject(this, &TrapBase::ShadowDraw, vMin, vMax);
	}
}

void _cdecl TrapBase::ShadowDraw(const char * group, MissionObject * sender)
{
	if(!EditMode_IsVisible() || !IsShow()) return;
	if(m_pModel)
	{
		m_pModel->SetTransform(m_mtxTransform);
		m_pModel->Draw();
	}
}

void TrapBase::AddExcludeObj(DamageReceiver* pObj,float fTime)
{
	long n;
	for( n=0; n<m_aExcludeList; n++ )
		if( m_aExcludeList[n].obj == pObj )
		{
			m_aExcludeList[n].time = fTime;
			return;
		}
	n = m_aExcludeList.Add();
	m_aExcludeList[n].obj = pObj;
	m_aExcludeList[n].time = fTime;
}

void TrapBase::DelExcludeObj(DamageReceiver* pObj)
{
	for( long n=0; n<m_aExcludeList; n++ )
		if( m_aExcludeList[n].obj == pObj )
		{
			m_aExcludeList.DelIndex( n );
			return;
		}
}

void TrapBase::SetSound(ISound3D* pSound)
{
	RELEASE( m_pSound );
	m_pSound = pSound;
}

void TrapBase::SetParticle(dword n, IParticleSystem* pParticle, const Vector& pos, bool bLocal)
{
	if( n<maxparticles )
	{
		SetParticleToDeleteList( m_pParticle[n].p );
		m_pParticle[n].p = pParticle;
		m_pParticle[n].pos = pos;
		m_pParticle[n].islocal = bLocal;
		if( pParticle )
		{
			if( bLocal )
			{
				Matrix m(m_mtxTransform);
				m.pos += m_mtxTransform.MulNormal( pos );
				pParticle->SetTransform( m );
			}
			else
			{
				pParticle->SetTransform( Matrix(true).BuildPosition(pos) );
			}
		}
	}
}

void TrapBase::TransformParticles(const Matrix & mtx)
{
	for( long n=0; n<maxparticles; n++ )
		if( m_pParticle[n].p && m_pParticle[n].islocal )
		{
			Matrix m(mtx);
			m.pos += mtx.MulNormal( m_pParticle[n].pos );
			if( m_baseparams.isLocalParticles )
				m_pParticle[n].p->Teleport( m );
			else
				m_pParticle[n].p->SetTransform( m );
		}
}

void TrapBase::MoveParticle(dword n, const Vector & pos)
{
	if( n<maxparticles && m_pParticle[n].p )
	{
		m_pParticle[n].pos = pos;
		if( m_pParticle[n].islocal )
		{
			Matrix m(m_mtxTransform);
			m.pos += m_mtxTransform.MulNormal( pos );
			m_pParticle[n].p->SetTransform( m );
		}
		else
		{
			m_pParticle[n].p->SetTransform( Matrix(true).BuildPosition(pos) );
		}
	}
}

void TrapBase::SetStandartParams(TrapPatternBase* pPattern)
{
	if( pPattern )
	{
		m_reactions = pPattern->GetReactions();
		m_renderparams = pPattern->GetRenderParams();
		m_baseparams = pPattern->GetBaseParams();

		if( m_renderparams.shadowCast )
		{
			Registry(MG_SHADOWCAST, (MOF_EVENT)&TrapBase::ShadowCast, ML_TRAPDRAWLEVEL);
		}else{
			Unregistry(MG_SHADOWCAST);
		}

		if( m_renderparams.shadowReceive )
		{
			Registry(MG_SHADOWRECEIVE, (MOF_EVENT)&TrapBase::ShadowDraw, ML_TRAPDRAWLEVEL);
		}else{
			Unregistry(MG_SHADOWRECEIVE);
		}

		if( m_renderparams.seaReflection )
		{
			Registry(MG_SEAREFLECTION, (MOF_EVENT)&TrapBase::ShadowDraw, ML_TRAPDRAWLEVEL);
		}else{
			Unregistry(MG_SEAREFLECTION);
		}
	}
}

void TrapBase::SetModel(IGMXScene* pModel)
{
	RELEASE(m_pModel);
	m_pModel = pModel;
	if( m_pModel )
		m_pModel->AddRef();
}

void TrapBase::SetDamageActive(bool bActive)
{
	m_bDamageActive = bActive;
}

void TrapBase::SetParticleToDeleteList(IParticleSystem* pParticle)
{
	if( pParticle )
	{
		long n = m_aDelParticles.Add();
		m_aDelParticles[n].particle = pParticle;
		m_aDelParticles[n].time = 1.f;
		pParticle->PauseEmission(true);
	}
}

void TrapBase::ReleaseParticles()
{
	long n;
	for( n=0; n<maxparticles; n++ )
		RELEASE( m_pParticle[n].p );
	for( n=0; n<m_aDelParticles; n++ )
	{
		RELEASE( m_aDelParticles[n].particle );
	}
	m_aDelParticles.DelAll();
}

bool TrapBase::IsExclude(DamageReceiver* pObj)
{
	for( long n=0; n<m_aExcludeList; n++ )
		if( m_aExcludeList[n].obj == pObj )
			return true;
	return false;
}



TrapPatternBase::TrapPatternBase() :
	m_aTraps(_FL_)
{
}

void TrapPatternBase::ReadCommonData(MOPReader & reader)
{
	// reactions
	m_reactions.pcHitReaction = reader.String().c_str();
	m_reactions.pcBlockReaction = reader.String().c_str();
	m_reactions.pcDieReaction = reader.String().c_str();

	// render params
	m_renderparams.color = reader.Colors();
	m_renderparams.dynamicLighting = reader.Bool();
	m_renderparams.shadowCast = reader.Bool();
	m_renderparams.shadowReceive = reader.Bool();
	m_renderparams.seaReflection = reader.Bool();

	//m_baseparams.isLocalParticles = reader.Bool();
}

void TrapPatternBase::UpdateTraps()
{
	for( long n=0; n<m_aTraps; n++ )
		m_aTraps[n]->UpdatePattern();
}

void TrapPatternBase::RegistryTrap(TrapBase* pTrap)
{
	if( pTrap && m_aTraps.Find(pTrap)==INVALID_ARRAY_INDEX )
		m_aTraps.Add( pTrap );
}

void TrapPatternBase::UnregistryTrap(TrapBase* pTrap)
{
	long n = m_aTraps.Find(pTrap);
	if( n != INVALID_ARRAY_INDEX )
		m_aTraps.DelIndex(n);
}
