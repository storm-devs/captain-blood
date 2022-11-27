#include "Minigun.h"
#include "physic.h"
#include "..\..\..\Common_h\ICharactersArbiter.h"
#include "..\..\..\Common_h\bonuses.h"
#include "..\..\..\Common_h\IGUIElement.h"
#include "..\..\..\Character\Auxiliary objects\CharacterEffect\CharacterEffect.h"
#include "AnimHandler.h"
#include <cmath>

#define MAX_HEIGHT 0.2f

#define TraceBufferBlockQnt	32

// дистанция на которой мы не попадаем (обязана быть больше 0, т.к. на нее есть деление)
#define DONT_SHOOT_DISTANCE 1.f

void Minigun::RememberTargetDamage()
{
	if( m_targetDescr.type == TargetDescribe::tt_none ) return;

	float fTime = (m_targetDescr.pos - vPos).GetLength() / traceparam.fSpeed;

	long n = m_aWaitingTargets.Add();
	m_aWaitingTargets[n].time = fTime;
	m_aWaitingTargets[n].targtype = m_targetDescr.type;
	m_aWaitingTargets[n].normal = m_targetDescr.normal;
	m_aWaitingTargets[n].phys = m_targetDescr.phys;

	Matrix mtx(true);
	switch( m_targetDescr.type )
	{
	case TargetDescribe::tt_waterlevel:
	case TargetDescribe::tt_Collider:
		m_aWaitingTargets[n].mosp.Reset();
		m_aWaitingTargets[n].pos = m_targetDescr.pos;
	break;
	case TargetDescribe::tt_Ragdoll:
		m_aWaitingTargets[n].mosp.Reset();
		m_aWaitingTargets[n].pos = m_targetDescr.pos;
		m_aWaitingTargets[n].phys->AddRef();
	break;

	case TargetDescribe::tt_DamageAcceptor:
	case TargetDescribe::tt_ShooterTarget:
	case TargetDescribe::tt_PassiveShooterTarget:
		m_targetDescr.mo->BuildSafePointer(m_targetDescr.mo,m_aWaitingTargets[n].mosp);
		m_targetDescr.mo->GetMatrix(mtx);
		m_aWaitingTargets[n].pos = mtx.MulVertexByInverse( m_targetDescr.pos );
	break;
	}
}

void Minigun::UpdateTargetDamage(float fDeltaTime)
{
	for( long n=0; n<m_aWaitingTargets; n++ )
	{
		m_aWaitingTargets[n].time -= fDeltaTime;
		if( m_aWaitingTargets[n].time <= 0.f )
		{
			TargetDamage( m_aWaitingTargets[n] );
			m_aWaitingTargets.DelIndex( n );
			n--;
		}
	}
}

void Minigun::TargetDamage( WaitingTarget & targ )
{
	if( targ.targtype == TargetDescribe::tt_none ) return;

	switch( targ.targtype )
	{
	case TargetDescribe::tt_waterlevel:
		EffectStart( fxWaterSplash, targ.pos, targ.normal );
	break;

	case TargetDescribe::tt_Collider:
		EffectStart( fxColliderSplash, targ.pos, targ.normal );
	break;

	case TargetDescribe::tt_Ragdoll:
		EffectStart( fxRagdollSplash, targ.pos, targ.normal );
		((IPhysRagdoll*)targ.phys)->ApplyImpulse( (!(targ.pos - vPos)) * fRagdollBulletImpulse );
		targ.phys->Release();
	break;

	case TargetDescribe::tt_DamageAcceptor:
		if( targ.mosp.Validate() && !targ.mosp.Ptr()->IsDead() )
		{
			Matrix mtx(true);
			targ.mosp.Ptr()->GetMatrix(mtx);
			targ.pos = mtx.MulVertex(targ.pos);
			EffectStart( fxColliderSplash, targ.pos, targ.normal );
			// если не можем отдамаджить по заданной точке, то дамагаем в центр - там уж точно попадем
			if( !((DamageReceiver*)targ.mosp.Ptr())->Attack( this, DamageReceiver::ds_shooter, damage, vPos, targ.pos ) )
				((DamageReceiver*)targ.mosp.Ptr())->Attack( this, DamageReceiver::ds_shooter, damage, targ.pos, mtx.pos );
		}
	break;

	case TargetDescribe::tt_ShooterTarget:
		if( targ.mosp.Validate() )
		{
			Matrix mtx(true);
			targ.mosp.Ptr()->GetMatrix(mtx);
			targ.pos = mtx.MulVertex(targ.pos);
			((ShooterTargets*)targ.mosp.Ptr())->CheckHit( targ.phys, targ.pos, targ.normal );
		}
	break;

	case TargetDescribe::tt_PassiveShooterTarget:
		if( targ.mosp.Validate() )
		{
			Matrix mtx(true);
			targ.mosp.Ptr()->GetMatrix(mtx);
			targ.pos = mtx.MulVertex(targ.pos);
			((ShooterTargets*)targ.mosp.Ptr())->CheckHit( targ.phys, targ.pos, targ.normal );
		}
	break;
	}
}


//Конструктор
Minigun::Minigun() :
	aBulletTrace(_FL_, 16),
	m_aComplexityData( _FL_ ),
	m_aWaitingTargets( _FL_ )
{
	//m_fTmpRotateSpeed = 100.f;

	m_fShoterSensitivity = 1.f;
	m_bShoterInverse = false;
	m_bShoterAIM = true;

	bPointerBuilded = false;

	radius = 2.0f;
	damage = 0.0f;
	power = 100.0f;

	fRagdollBulletImpulse = 2.f;

	bLogicDebug = false;
	bWantToShoot = false;
	bShootMoment = false;
	showColliders = false;

	bShadowCast = false;

	pSoundRotate = NULL;

	init_zoomOn = false;

	bExplodeBullets = false;

	Weapon = NULL;
	pAnim = NULL;
	Listener = NULL;

	WeaponAddition = NULL;
	pAnimAddition = NULL;

	WeaponLoader = NULL;
	pAnimLoader = NULL;
	pLoaderListener = NULL;

	m_nShootLocatorsQuantity = 0;
	m_nCurShootLocatorIndex = 0;

	m_fMaxShootDist = 1000.f;
	m_fMinShootDist = 10.f;

	fRotSpeedH = 0.0f;
	fRotSpeedV = 0.0f;

	fJoyMaxRotSpeed = 1.5f;
	fJoyRotAccelerate = 4.5f;

	fMsMaxRotSpeed = 2.5f;
	fMsRotAccelerate = 9.5f;

	fMouseUpdate = 0;
	iMouseTimes = 0;
	fCurMouseValueV = 0;
	fCurMouseValueH = 0;
	fMouseFilteredV = 0;
	fMouseFilteredH = 0;

	for (int i=0;i<4;i++) targets[i] = null;

	wasActivated = 1;

	zoomOn = false;
	fovChgCurTime = -1.0f;

	init_active_state = true;

	pBonusGreedy = null;

	//m_fSightAng = 0.0f;
	m_fSightSize = 0.f;
	m_fSightOffset = 0.f;
	m_pSightGUI_Hit = m_pSightGUI_Miss = null;

	m_fShootAnimationSpeed = 1.f;

	nRepeatableShootQuantity = 0;
	nRepeatableShootLeft = 0;

	traceparam.fTraceDistance = 1000.f;
	traceparam.fTraceDistanceDiv = 0.001f;
	//traceparam.fTraceLifeTime = 0.4f;
	//traceparam.fTraceLifeTimeDiv = 2.5f;
	//traceparam.fMinSpeed = 200.f;
	traceparam.fSpeed = 100.f;
	traceparam.fMinShowDist = 2.f;
	traceparam.length = 1.f;
	traceparam.lengthAdd = 10.f;
	traceparam.width = 0.1f;
	traceparam.widthAdd = 1.f;

	traceparam.vTex = null;

	traceparam.pcTraceTexture = "";
	traceparam.pcBallTexture = "";
	traceparam.pTraceTexture = null;
	traceparam.pBallTexture = null;
	traceparam.pvbTraceBuffer = null;
	traceparam.pvbBallBuffer = null;
	traceparam.nTraceQuantity = 0;

	fShootTime = 0.f;
	fShootInterval = 1.f;
	m_fBulletAnimationSpeed = 1.f;

	fDispersionAngle = 0.f;

	useSwingCorrection = true;

	m_pExplosionPatch = NULL;
}

//Деструктор
Minigun::~Minigun()
{
	Release();

	#ifndef NO_CONSOLE

	if (!EditMode_IsOn())
	{	
		Console().UnregisterCommand("ShooterDebug");
	}

	#endif
}


//Создание объекта
bool Minigun::Create(MOPReader & reader)
{
	bPointerBuilded = false;
/*
	m_fWideScreenAspectDividerConst = Render().GetWideScreenAspectFovMultipler();

	if( m_fWideScreenAspectDividerConst < 0.01f )
		m_fWideScreenAspectDividerConst = 1.f;
	else
		m_fWideScreenAspectDividerConst = 1.f / m_fWideScreenAspectDividerConst;
	m_fWideScreenAspectDividerConst *= (180.f/PI);
*/

	if (pAnim)
	{
		blender.UnregistryBlendStage();
		blender.SetAnimation(NULL);
		if( Listener )
			Listener->DelAnimation(pAnim);
	}
	DELETE(Listener);
	Listener = NEW AniMinigunEvents(this);
	Listener->Init(&Sound(), &Particles(), &Mission());

	if (pLoaderListener && pAnimLoader)
		pLoaderListener->DelAnimation(pAnimLoader);
	DELETE(pLoaderListener);
	//pLoaderListener = NEW AnimationStdEvents();
	pLoaderListener = NEW AniLoaderEvents();
	pLoaderListener->Init(&Sound(), &Particles(), &Mission());

	EditMode_Update (reader);

	codeWeapon_Fire = Mission().Controls().FindControlByName("Weapon_Fire");
	codeWeapon_Exit = Mission().Controls().FindControlByName("Weapon_Exit");

	codeWeapon_Turn_Left = Mission().Controls().FindControlByName("Weapon_Turn_Left");
	codeWeapon_Turn_Right = Mission().Controls().FindControlByName("Weapon_Turn_Right");
	codeWeapon_Turn_Up = Mission().Controls().FindControlByName("Weapon_Turn_Up");
	codeWeapon_Turn_Down = Mission().Controls().FindControlByName("Weapon_Turn_Down");

	codeWeapon_TurnMouse_H = Mission().Controls().FindControlByName("Weapon_TurnMouse_H");
	codeWeapon_TurnMouse_V = Mission().Controls().FindControlByName("Weapon_TurnMouse_V");	

	traceparam.vTex = Render().GetTechniqueGlobalVariable("tMinigunTexture", _FL_);

	if( traceparam.pcTraceTexture )
		traceparam.pTraceTexture = Render().CreateTexture( _FL_, "%s", traceparam.pcTraceTexture );
	if( traceparam.pcBallTexture )
		traceparam.pBallTexture = Render().CreateTexture( _FL_, "%s", traceparam.pcBallTexture );
	traceparam.nTraceQuantity = TraceBufferBlockQnt;
	traceparam.pvbTraceBuffer = Render().CreateVertexBuffer( sizeof(Vertex)*6*traceparam.nTraceQuantity, sizeof(Vertex), _FL_, USAGE_WRITEONLY | USAGE_DYNAMIC, POOL_DEFAULT );
	traceparam.pvbBallBuffer = Render().CreateVertexBuffer( sizeof(Vertex)*6*traceparam.nTraceQuantity, sizeof(Vertex), _FL_, USAGE_WRITEONLY | USAGE_DYNAMIC, POOL_DEFAULT );

	if (Weapon && Listener)
	{
		Listener->SetScene(Weapon, Weapon->GetTransform());
	}

	if (WeaponLoader && pLoaderListener)
	{
		pLoaderListener->SetScene(WeaponLoader, WeaponLoader->GetTransform());
	}

	MissionObject::Activate(false);
	return true;
}

void Minigun::PostCreate()
{
	MOSafePointer mo;

	// TiedCamera
	if( m_pTiedCameraID.NotEmpty() )
		MissionObject::FindObject( m_pTiedCameraID, m_pTiedCamera );

	// получим данные профайла
	m_fShoterSensitivity = api->Storage().GetFloat("Options.ShoterSensitivity",1.f);
	m_bShoterInverse = api->Storage().GetFloat("Options.ShoterInverse",0.f) > 0.5f;
	m_bShoterAIM = api->Storage().GetFloat("Options.ShoterAIM",1.f) > 0.5f;

	const char* pcName = api->Storage().GetString("Profile.Global.Difficulty");
	if( !pcName ) pcName = "Normal";
	for( long n=0; n<m_aComplexityData; n++ )
		if( m_aComplexityData[n].name && string::IsEqual(m_aComplexityData[n].name,pcName) )
		{
//#ifdef _XBOX
			//m_fSightAng = m_aComplexityData[n].autoaimAngle * (PI / 180.f);
			m_fSightOffset = -NearPlane;
			m_fSightSize = m_aComplexityData[n].autoaimSize * 0.01f;
			//m_fSightOffset = m_aComplexityData[n].autoaimOffset;
//#endif
			// если в профайле отключено автонаведение, то выключаем его в шутере
			if( !m_bShoterAIM )
				//m_fSightAng = 0.f;
				m_fSightSize = 0.f;

			MissionObject::FindObject( m_aComplexityData[n].pcSightName_Hit, mo );
			static const ConstString tid("IGUIElement");
			if( mo.Ptr() && mo.Ptr()->Is(tid) )
			{
				m_pSightGUI_Hit = (IGUIElement*)mo.Ptr();
				if( m_pSightGUI_Hit ) m_pSightGUI_Hit->Show(false);
			}

			MissionObject::FindObject( m_aComplexityData[n].pcSightName_Miss, mo );
			if( mo.Ptr() && mo.Ptr()->Is(tid) )
			{
				m_pSightGUI_Miss = (IGUIElement*)mo.Ptr();
				if( m_pSightGUI_Miss ) m_pSightGUI_Miss->Show(false);
			}
		}

	shooterEffect.PostCreate();

	Activate (init_active_state);
}

void Minigun::ClampAngles ()
{
	if (vViewAng.x < HeightLimitMinRad)
	{
		vViewAng.x = HeightLimitMinRad;
	}
	if (vViewAng.x > HeightLimitMaxRad)
	{
		vViewAng.x = HeightLimitMaxRad;
	}
	
	if (vViewAng.y < RotateLimitMinRad)
	{
		vViewAng.y = RotateLimitMinRad;
	}
	if (vViewAng.y > RotateLimitMaxRad)
	{
		vViewAng.y = RotateLimitMaxRad;
	}

	vViewAng.z = 0.0f;
}

//Обновление параметров
bool Minigun::EditMode_Update(MOPReader & reader)
{
	// удаляем старую модель шутера с анимацией
	if (pAnim)
	{
		if (Listener)
			Listener->DelAnimation(pAnim);
		pAnim->Release();
		pAnim = NULL;
	}
	RELEASE(Weapon);
	// создаем новую модель шутера
	IGMXScene* old_model = Weapon;
	Weapon = Geometry().CreateGMX(reader.String().c_str(), &Animation(), &Mission().Particles(), &Mission().Sound());
	RELEASE(old_model);
	// ставим освещение на модель
	bool WeaponDL = reader.Bool();
	if (Weapon)
	{
		Weapon->SetDynamicLightState(WeaponDL);
	}
	// ставим анимацию на модель шутера
	const char* weaponAnim = reader.String().c_str();
	if (Weapon)
	{
		Weapon->SetAnimationFile(weaponAnim);
		pAnim = Weapon->GetAnimation();
		if (!pAnim) api->Trace("Shoter %s: can't load anx - %s",GetObjectID().c_str(),weaponAnim);
	}
	if (pAnim)
	{
		Listener->AddAnimation(pAnim);
		blender.Init(pAnim);
	}

	// удаляем старую модель магазина
	RELEASE(pAnimAddition);
	// создаем новую модель магазина
	old_model = WeaponAddition;
	WeaponAddition = Geometry().CreateGMX(reader.String().c_str(), &Animation(), &Mission().Particles(), &Mission().Sound());
	RELEASE(old_model);
	// ставим освещение на модель
	if (WeaponAddition)
	{
		WeaponAddition->SetDynamicLightState(WeaponDL);
	}
	// ставим анимацию на модель магазина
	weaponAnim = reader.String().c_str();
	if (WeaponAddition)
	{
		WeaponAddition->SetAnimationFile(weaponAnim);
		pAnimAddition = WeaponAddition->GetAnimation();
		if (!pAnimAddition) api->Trace("Shoter %s: can't load anx - %s",GetObjectID().c_str(),weaponAnim);
	}
	if (pAnimAddition)
	{
		pAnimAddition->SetPlaySpeed( m_fBulletAnimationSpeed );
		pAnimAddition->Pause( true );
	}

	// руки для зарядки минигана
	if( pAnimLoader )
	{
		if( pLoaderListener )
			pLoaderListener->DelAnimation( pAnimLoader );
		pAnimLoader->Release();
		pAnimLoader = NULL;
	}
	old_model = WeaponLoader;
	WeaponLoader = Geometry().CreateGMX(reader.String().c_str(), &Animation(), &Mission().Particles(), &Mission().Sound());
	RELEASE(old_model);
	// ставим освещение на модель
	if (WeaponLoader)
	{
		WeaponLoader->SetDynamicLightState(WeaponDL);
	}
	// ставим анимацию на модель
	weaponAnim = reader.String().c_str();
	if (WeaponLoader)
	{
		WeaponLoader->SetAnimationFile(weaponAnim);
		pAnimLoader = WeaponLoader->GetAnimation();
		if (!pAnimLoader) api->Trace("Shoter %s: can't load anx - %s",GetObjectID().c_str(),weaponAnim);
	}
	if (pAnimLoader)
	{
		const char* params[2];
		params[0] = "collapse";
		params[1] = "saber_joint";
		if( pLoaderListener ) pLoaderListener->BoneCollapse(pAnimLoader,"collapse",params,2);
		//pAnimLoader->Pause( true );
	}

	m_fShootAnimationSpeed = reader.Float();
	nRepeatableShootQuantity = reader.Long();
	nRepeatableShootLeft = nRepeatableShootQuantity;

	fShootInterval = reader.Float();
	m_fBulletAnimationSpeed = reader.Float();

	m_nShootLocatorsQuantity = 0;
	if( Weapon )
	{
		CameraLocator = Weapon->FindEntity(GMXET_LOCATOR,"camera");
		BulletLocator = Weapon->FindEntity(GMXET_LOCATOR,"bullet");
		ShootLocator[0] = Weapon->FindEntity(GMXET_LOCATOR,"shoot");
		AdditionalLocator = Weapon->FindEntity(GMXET_LOCATOR,"additional");
	}
	if( ShootLocator[0].isValid() )
	{
		char pcTmpLocName[16];
		crt_strcpy(pcTmpLocName,sizeof(pcTmpLocName),"shoot1");
		for( char l=1; l<sizeof(ShootLocator) / sizeof(GMXHANDLE); l++ )
		{
			pcTmpLocName[5] = '0' + l;
			if( Weapon )
				ShootLocator[l] = Weapon->FindEntity(GMXET_LOCATOR,pcTmpLocName);
			else
				ShootLocator[l].reset();
			if( !ShootLocator[l].isValid() )
				break;
		}
		m_nShootLocatorsQuantity = l;
	}

	vPos = reader.Position();
	vViewAng = reader.Angles();
	if( vViewAng.x > PI ) vViewAng.x -= 2*PI;
	if( vViewAng.y > PI ) vViewAng.y -= 2*PI;

	HeightLimitMinRad = Deg2Rad(reader.Float());
	HeightLimitMaxRad = Deg2Rad(reader.Float());
	RotateLimitMinRad = Deg2Rad(reader.Float());
	RotateLimitMaxRad = Deg2Rad(reader.Float());

	// проверка для лохов, которые не знаю что мин должен быть меньбше макса
	if (HeightLimitMinRad > HeightLimitMaxRad)
	{
		float tmp = HeightLimitMinRad;
		HeightLimitMinRad = HeightLimitMaxRad;
		HeightLimitMaxRad = tmp;
	}

	if (RotateLimitMinRad > RotateLimitMaxRad)
	{
		float tmp = RotateLimitMinRad;
		RotateLimitMinRad = RotateLimitMaxRad;
		RotateLimitMaxRad = tmp;
	}	

	Matrix m;
	m.SetIdentity();
	m.RotateX((HeightLimitMaxRad + HeightLimitMinRad) * 0.5f);
	m.RotateY((RotateLimitMaxRad + RotateLimitMinRad) * 0.5f);

	nearFOV = reader.Float()*(PI/180.0f);
	farFOV = reader.Float()*(PI/180.0f);

	fovChgTime = reader.Float();
	init_zoomOn = reader.Bool();


	zoomOn = init_zoomOn;
	fovChgCurTime = -1.0f;

	if (zoomOn)
	{
		fov = nearFOV;
	}
	else
	{
		fov = farFOV;
	}


	NearPlane = reader.Float();
	FarPlane = reader.Float();


	fJoyMaxRotSpeed = reader.Float();
	fJoyRotAccelerate = reader.Float();
	fMsMaxRotSpeed = reader.Float();
	fMsRotAccelerate = reader.Float();


	fJoyMaxRotSpeed_zoomed = reader.Float();
	fJoyRotAccelerate_zoomed = reader.Float();
	fMsMaxRotSpeed_zoomed = reader.Float();
	fMsRotAccelerate_zoomed = reader.Float();

	m_fMaxShootDist = reader.Float();
	m_fMinShootDist = reader.Float();

	EffectReadMOPs( fxShoot, reader );

	EffectReadMOPs( fxWaterSplash, reader );
	EffectReadMOPs( fxCharacterSplash, reader );
	EffectReadMOPs( fxRagdollSplash, reader );
	EffectReadMOPs( fxColliderSplash, reader );

	ReloadSound = reader.String();
	RotateSound = reader.String();

	triger_onActivate.Init(reader);
	triger_onDeactivate.Init(reader);

	triger_onZoomIn.Init(reader);
	triger_onZoomOut.Init(reader);

	for (int i=0;i<4;i++)
	{
		targets[i] = null;
		MOSafePointer mo;
		MissionObject::FindObject(reader.String(), mo);

		if (mo.Ptr())
		{
			MO_IS_IF(tid, "ShooterTargets", mo.Ptr())
			{
				targets[i] = (ShooterTargets*)mo.Ptr();
			}
		}
	}

	bShadowCast = reader.Bool();
	
	bExplodeBullets = reader.Bool();

	radius = reader.Float();
	damage = reader.Float();
	power = reader.Float();

	fRagdollBulletImpulse = reader.Float();

	//m_fSightAng = 0.f;
	//m_fSightOffset = 0.f;
	m_aComplexityData.DelAll();
	long q = reader.Array();
	if( q>0 )
	{
		m_aComplexityData.AddElements( q );
		for( long j=0; j<q; j++ )
		{
			m_aComplexityData[j].name = reader.String().c_str();
			m_aComplexityData[j].autoaimSize = reader.Float();
			//m_aComplexityData[j].autoaimAngle = reader.Float();
			//m_aComplexityData[j].autoaimOffset = reader.Float();
			m_aComplexityData[j].pcSightName_Hit = reader.String();
			m_aComplexityData[j].pcSightName_Miss = reader.String();
		}
	}

	fDispersionAngle = reader.Float() * PI / 180.f;

	traceparam.vStartTraceOffset = reader.Position();
	traceparam.fTraceDistance = reader.Float();
	traceparam.fTraceDistanceDiv = (traceparam.fTraceDistance>1.f) ? (1.f/traceparam.fTraceDistance) : 1.f;
	//traceparam.fTraceLifeTime = reader.Float();
	//traceparam.fTraceLifeTimeDiv = (traceparam.fTraceLifeTime > 0.01f) ? (1.f / traceparam.fTraceLifeTime) : 0.f;
	//traceparam.fMinSpeed = reader.Float();
	traceparam.fSpeed = reader.Float();
	traceparam.fMinShowDist = reader.Float();
	traceparam.length = reader.Float();
	traceparam.lengthAdd = reader.Float() - traceparam.length;
	traceparam.width = reader.Float();
	traceparam.widthAdd = reader.Float() - traceparam.width;
	traceparam.pcBallTexture = reader.String().c_str();
	traceparam.pcTraceTexture = reader.String().c_str();

	shooterEffect.ReadMOPs( reader );

	m_pTiedCameraID = reader.String();

	useSwingCorrection = reader.Bool();

	showColliders = reader.Bool();

	Show(reader.Bool());

	init_active_state = reader.Bool();

	bLogicDebug = reader.Bool();

	shooterEffect.Init( &Mission(), Weapon );

	//m_fTmpRotateSpeed = reader.Float();

	return true;
}


Matrix & Minigun::GetMatrix(Matrix & mtx)
{
	if( m_pTiedCameraID.NotEmpty() && m_pTiedCamera.Validate() )
	{
		if (CameraLocator.isValid())
		{
			Weapon->GetNodeWorldTransform( CameraLocator, mtx );
			mtx.Normalize();
		}
		return mtx;
	}

	mtx.Build(vViewAng, vPos);
	return mtx;
}

void Minigun::DrawLimiters()
{
	if (!Mission().EditMode_IsAdditionalDraw()) return;
	Matrix m;
	
	
	m.SetIdentity();
	m.RotateY((RotateLimitMaxRad + RotateLimitMinRad) * 0.5f);
	m.pos = vPos;

	float x1 = cosf(HeightLimitMinRad);
	float y1 = sinf(HeightLimitMinRad);

	dword dwColor = 0xFFFF0000;

	Vector v1, v2;
	v1 = Vector(0.0f, -y1, x1) * m;
	Render().Print(v1, 100.0f, 0.0f, dwColor, "%3.2f", Rad2Deg(HeightLimitMinRad));

	float step = Deg2Rad(1.0f);

	for (float i = HeightLimitMinRad+step; i < HeightLimitMaxRad; i+=step)
	{
		float x2 = cosf(i);
		float y2 = sinf(i);

		v1 = Vector(0.0f, -y1, x1) * m;
		v2 = Vector(0.0f, -y2, x2) * m;


		Render().DrawLine(v1, dwColor,v2, dwColor);

		x1 = x2;
		y1 = y2;
	}

	Render().Print(v2, 100.0f, 0.0f, dwColor, "%3.2f", Rad2Deg(HeightLimitMaxRad));


	m.SetIdentity();
	m.RotateX((HeightLimitMaxRad + HeightLimitMinRad) * 0.5f);
	m.RotateY((RotateLimitMaxRad + RotateLimitMinRad) * 0.5f);	
	m.pos = vPos;

	float RotateLimitBorder = fabs(RotateLimitMaxRad - RotateLimitMinRad) * 0.5f;

	x1 = sinf(-RotateLimitBorder);
	float z1 = cosf(-RotateLimitBorder);

	dwColor = 0xFF0000FF;

	v1 = Vector(x1, 0.0f, z1) * m;
	Render().Print(v1, 100.0f, 0.0f, dwColor, "%3.2f", Rad2Deg(RotateLimitMinRad));
	
	for (i = -RotateLimitBorder+step; i < RotateLimitBorder; i+=step)
	{
		float x2 = sinf(i);
		float z2 = cosf(i);

		v1 = Vector(x1, 0.0f, z1) * m;
		v2 = Vector(x2, 0.0f, z2) * m;

		Render().DrawLine(v1, dwColor,v2, dwColor);

		z1 = z2;
		x1 = x2;

	}

	Render().Print(v2, 100.0f, 0.0f, dwColor, "%3.2f", Rad2Deg(RotateLimitMaxRad));
/*
	Render().Print(0, 0, 0xFFFFFF00, "%f %f", vViewAng.x, vViewAng.y);
	Render().Print(0, 24, 0xFFFFFF00, "%d", Bullets.Size());
	Render().Print(0, 48, 0xFFFFFF00, "state - %d", State);
*/

	// отрисуем конус для автоприцела
	if( EditMode_IsSelect() && CameraLocator.isValid() )
	{
		struct POLYGON_VERTEX
		{
			Vector		vPos;
			dword		dwColor;
		};

		POLYGON_VERTEX aV[26];
		Vector vEnd = Vector(0.f,0.f,m_fMaxShootDist+m_fSightOffset);
		Vector vBeg = Vector(0.f,0.f,m_fSightOffset);

		// определим текущий угол автоприцела
		const char* pcName = api->Storage().GetString("Profile.Global.Difficulty");
		if( !pcName ) pcName = "Normal";
		for( long n=0; n<m_aComplexityData; n++ )
			if( m_aComplexityData[n].name && string::IsEqual(m_aComplexityData[n].name,pcName) )
			{
				m_fSightSize = m_aComplexityData[n].autoaimSize * 0.01f;
				//m_fSightAng = m_aComplexityData[n].autoaimAngle * (PI / 180.f);
				//m_fSightOffset = m_aComplexityData[n].autoaimOffset;
			}
		// повернем на угол автоприцеливания
		//vEnd.Rotate(m_fSightAng); vEnd.z -= m_fSightOffset;
		//vBeg.Rotate(m_fSightAng); vBeg.z -= m_fSightOffset;
		vEnd.Rotate(m_fSightSize*fov); vEnd.z -= m_fSightOffset;
		vBeg.Rotate(m_fSightSize*fov); vBeg.z -= m_fSightOffset;

		// покрутим вокруг оси Z (сформируем конус)
		aV[0].vPos = vBeg;
		aV[0].dwColor = 0xFFFF0000;
		aV[1].vPos = vEnd;
		aV[1].dwColor = 0xFFFF0000;
		aV[24].vPos = vBeg;
		aV[24].dwColor = 0xFFFF0000;
		aV[25].vPos = vEnd;
		aV[25].dwColor = 0xFFFF0000;
		float fang = PIm2 * (1.f/12.f);
		float fac = cosf(fang);
		float fas = sinf(fang);
		for(int i=1; i<12; i++)
		{
			aV[i*2].vPos = vBeg.RotateXY( fac, fas );
			aV[i*2].dwColor = 0xFFFF0000;
			aV[i*2+1].vPos = vEnd.RotateXY( fac, fas );
			aV[i*2+1].dwColor = 0xFFFF0000;
		}

		Matrix mtxCam;
		Weapon->GetNodeWorldTransform(CameraLocator,mtxCam);
		Render().SetWorld(mtxCam);
		ShaderId shader_id;
		Render().GetShaderId("stdPolygon", shader_id);
		Render().DrawPrimitiveUP(shader_id, PT_TRIANGLESTRIP, 24, &aV[0], sizeof(POLYGON_VERTEX));
	}
}

void _cdecl Minigun::RealizeControl(float fDeltaTime, long level)
{
	UpdateColliders();

	if(Mission().Controls().GetControlStateType("Weapon_Fire") == CST_ACTIVE)
	{
		bWantToShoot = true;
		//Fire();
	}

	
	if(Mission().Controls().GetControlStateType("Weapon_Exit") == CST_INACTIVATED)
	{
		wasActivated--;

		if (wasActivated <= 0)
		{
			Activate(false);
			return;
		}
	}
	
	if(Mission().Controls().GetControlStateType("Weapon_ZoomOn") == CST_ACTIVATED)
	{
		ZoomEnable(!zoomOn);

		if (zoomOn)
		{
			triger_onZoomIn.Activate(Mission(), false);
			api->Trace("Minigun %s: ZoomIn triger was activated", GetObjectID().c_str());
		}
		else
		{
			triger_onZoomOut.Activate(Mission(), false);
			api->Trace("Minigun %s: ZoomOut triger was activated", GetObjectID().c_str());
		}
	}


	// Filter Mouse
	
	fMouseUpdate += fDeltaTime;

	fCurMouseValueV += Mission().Controls().GetControlStateFloat(codeWeapon_TurnMouse_V);
	fCurMouseValueH += Mission().Controls().GetControlStateFloat(codeWeapon_TurnMouse_H);
	iMouseTimes++;

	if (fMouseUpdate>0.05f)
	{
		fMouseUpdate = 0.0f;

		fMouseFilteredV = fCurMouseValueV/(float)iMouseTimes;
		fMouseFilteredH = fCurMouseValueH/(float)iMouseTimes;

		iMouseTimes = 0;
		fCurMouseValueV = 0;
		fCurMouseValueH = 0;		
	}

	// Read Control
	float fCurRotSpeed;
	//float k = Mission().Controls().GetControlStateFloat(codeWeapon_Turn_H);


	float   fMaxRotSpeed;
	float   fRotAccelerate;	

	float turn_h = Clampf(Mission().Controls().GetControlStateFloat(codeWeapon_Turn_Left)) -
				   Clampf(Mission().Controls().GetControlStateFloat(codeWeapon_Turn_Right));

	float turn_v = Clampf(Mission().Controls().GetControlStateFloat(codeWeapon_Turn_Up)) -
				   Clampf(Mission().Controls().GetControlStateFloat(codeWeapon_Turn_Down));

	// применяем настройки профайла
	turn_h *= m_fShoterSensitivity;
	turn_v *= m_fShoterSensitivity * (m_bShoterInverse ? -1.f : 1.f);

	if (fabs(turn_h)>0.05f || fabs(turn_v)>0.05f)
	{	
		if (zoomOn)
		{
			fMaxRotSpeed = fJoyMaxRotSpeed_zoomed;
			fRotAccelerate = fJoyRotAccelerate_zoomed;
		}
		else
		{
			fMaxRotSpeed = fJoyMaxRotSpeed;
			fRotAccelerate = fJoyRotAccelerate;
		}		

		fCurRotSpeed = turn_h * fMaxRotSpeed;	
		ChangeValue(fRotSpeedH, fCurRotSpeed, fDeltaTime, fRotAccelerate);
		//vViewAng.y += fDeltaTime * fRotSpeedH;

		fCurRotSpeed = turn_v * fMaxRotSpeed;	
		ChangeValue(fRotSpeedV, fCurRotSpeed, fDeltaTime, fRotAccelerate);
		//vViewAng.x -= fDeltaTime * fRotSpeedV;
	}
	else
	{	
		if (zoomOn)
		{
			fMaxRotSpeed = fMsMaxRotSpeed_zoomed;
			fRotAccelerate = fMsRotAccelerate_zoomed;
		}
		else
		{
			fMaxRotSpeed = fMsMaxRotSpeed;
			fRotAccelerate = fMsRotAccelerate;
		}

		fCurRotSpeed = SIGN(fMouseFilteredH) * Clampf( fabs(fMouseFilteredH), 0,10) / 10.0f * fMaxRotSpeed;
		ChangeValue(fRotSpeedH, fCurRotSpeed, fDeltaTime, fRotAccelerate);
		//vViewAng.y += fDeltaTime * fRotSpeedH;
	
		//Render().Print(0, 0, 0xFFFFFFFF, "fH %f", k);
		//Render().Print(0, 16, 0xFFFFFFFF, "Horiz vel %f, max %f", fRotSpeedH, fCurRotSpeed);
		
		fCurRotSpeed = SIGN(fMouseFilteredV) * Clampf( fabs(fMouseFilteredV), 0,10)/10.0f * fMaxRotSpeed;			
		ChangeValue(fRotSpeedV, fCurRotSpeed, fDeltaTime, fRotAccelerate);
		//vViewAng.x -= fDeltaTime * fRotSpeedV;

		//Render().Print(0, 32, 0xFFFFFFFF, "fV %f", k);
		//Render().Print(0, 50, 0xFFFFFFFF, "Vert vel %f, max %f", fRotSpeedV, fCurRotSpeed);
	}

	// если автоприцеливание, то притягиваемся к цели
/*	if( m_autoaim.locksight )
	{
		//if( m_autoaim.time > 0.f )

		// камера
		Matrix mCam(true);
		if (ShootLocator)
			mCam = ShootLocator->GetWorldTransform();
		else mCam.SetIdentity();

		// точка куда надо нацелиться
		Vector vTargPnt = GetAimTarget();
		// переводим ее в локальную систему камеры
		Vector vTargLoc = mCam.MulVertexByInverse( vTargPnt );

		// угол доворота по Y
		float fNeedY = Clampf( vTargLoc.GetAY(0.f), -m_autoaim.autoAimSpeed, m_autoaim.autoAimSpeed );
		// угол доворота по X
		vTargLoc.x = vTargLoc.y;
		float fNeedX = Clampf( vTargLoc.GetAY(0.f), -m_autoaim.autoAimSpeed, m_autoaim.autoAimSpeed );

		float fZoneSpeed = fDeltaTime * fRotAccelerate;

		if ( fRotSpeedH < 0.f )
			fRotSpeedH = fNeedY < 0.f ? fNeedY : -fZoneSpeed;

		if ( fRotSpeedH > 0.f )
			fRotSpeedH = fNeedY > 0.f ? fNeedY : fZoneSpeed;

		if ( fRotSpeedV < 0.f )
			fRotSpeedV = fNeedX < 0.f ? fNeedX : -fZoneSpeed;

		if ( fRotSpeedV > 0.f )
			fRotSpeedV = fNeedX > 0.f ? fNeedX : fZoneSpeed;
	}*/

	vViewAng.y += fDeltaTime * fRotSpeedH;
	vViewAng.x -= fDeltaTime * fRotSpeedV;

	ClampAngles();

	// обновление виджета прицела
	UpdateSight();
}

void _cdecl Minigun::RealizeDraw(float fDeltaTime, long level)
{
	if(!EditMode_IsVisible()) return;
	
	SetWeaponTransform ();

	if (EditMode_IsOn()) DrawLimiters();

	if (Listener && Weapon)
	{
		Listener->Update(Weapon->GetTransform(), fDeltaTime);
	}

	if( pLoaderListener && WeaponLoader )
	{
		pLoaderListener->Update(WeaponLoader->GetTransform(), fDeltaTime);
	}

	DrawModel();
	//DrawTrails(fDeltaTime);

	if (showColliders)
	{
		for (dword i = 0; i < 4; i++)
		if (targets[i])
		{
			targets[i]->DrawColliders();
		}
	}	

	//Render().Print(10,10,0xff00ff00,"%4.3f %4.3f %4.3f",blender.cur_rot, blender.angular_speed, blender.angular_force);
}

void Minigun::UpdateColliderPositions ()
{
	for (dword i = 0; i < 4; i++)
	{
		if (targets[i])
		{
			targets[i]->UpdateColliderPositions (false);
		}
	}
}

void Minigun::AddTrace(float deltaTime)
{
	// определяем направление стрельбы
	if( !CameraLocator.isValid() ) return;
	Matrix mWorld = mtxShootDirection;
	Vector vDir = !mWorld.vz;

	TraceDescr tp;
	tp.fLifeTime = 0.f;
	//tp.vBeg = mWorld * traceparam.vStartTraceOffset;
	if (m_nCurShootLocatorIndex >= 0 && ShootLocator[m_nCurShootLocatorIndex].isValid() )//ShootLocator)
		tp.vBeg = GetLocatorPos( Weapon, ShootLocator[m_nCurShootLocatorIndex] );
	else
		tp.vBeg = mWorld * traceparam.vStartTraceOffset;
	if( m_targetDescr.type == TargetDescribe::tt_none )
	{
		tp.vDir = vDir;
		tp.maxDist = traceparam.fTraceDistance;
	}
	else
	{
		tp.vDir = m_targetDescr.pos - tp.vBeg;
		tp.maxDist = Min(tp.vDir.Normalize(), traceparam.fTraceDistance);
	}

	/*tp.speed = tp.maxDist * traceparam.fTraceLifeTimeDiv;
	if( tp.speed < traceparam.fMinSpeed )
		tp.speed = traceparam.fMinSpeed;*/
	tp.speed = traceparam.fSpeed;

	aBulletTrace.Add(tp);

	if( aBulletTrace.Size() > traceparam.nTraceQuantity )
	{
		while( aBulletTrace.Size() > traceparam.nTraceQuantity )
			traceparam.nTraceQuantity += TraceBufferBlockQnt;
		RELEASE( traceparam.pvbTraceBuffer );
		traceparam.pvbTraceBuffer = Render().CreateVertexBuffer( sizeof(Vertex)*6*traceparam.nTraceQuantity, sizeof(Vertex), _FL_, USAGE_WRITEONLY | USAGE_DYNAMIC, POOL_DEFAULT );
		RELEASE( traceparam.pvbBallBuffer );
		traceparam.pvbBallBuffer = Render().CreateVertexBuffer( sizeof(Vertex)*6*traceparam.nTraceQuantity, sizeof(Vertex), _FL_, USAGE_WRITEONLY | USAGE_DYNAMIC, POOL_DEFAULT );
	}
}

void Minigun::DrawTrails(float fDeltaTime, long level)
{
	Matrix oldview = Render().GetView();
	Render().SetView((Mission().GetInverseSwingMatrix()*Matrix(oldview).Inverse()).Inverse());

	// дебаг отрисовка сферы в котором происходит автодоводка
	if( api->DebugKeyState(VK_SHIFT,VK_CONTROL,'Z') )
	{
		Matrix mtxTmp;
		Weapon->GetNodeWorldTransform(CameraLocator,mtxTmp);
		Vector vpostmp = mtxTmp.pos;
		Vector vDir = m_targetDescr.pos - vpostmp;
		float fdtmp = vDir.Normalize();
		float fsintmp = abs(GetAutoAimSin());
		float fcostmp = sqrtf(1.f - fsintmp*fsintmp);
		Render().DrawSphere( vpostmp + fdtmp * vDir, (fdtmp - m_fSightOffset)*fsintmp/fcostmp, 0xFF00FF00 );
	}

	long nDrawQnt = 0;
	//if( traceparam.fTraceLifeTime > 0 && traceparam.pvbTraceBuffer )
	if( traceparam.pvbTraceBuffer || traceparam.pvbBallBuffer )
	{
		Vertex * vrt = traceparam.pvbBallBuffer ? (Vertex *)traceparam.pvbBallBuffer->Lock() : 0;
		Vertex * vrt2 = traceparam.pvbTraceBuffer ? (Vertex *)traceparam.pvbTraceBuffer->Lock() : 0;//(0, 12*sizeof(Vertex)*aBulletTrace.Size(), 0);
		if( vrt || vrt2 )
		{
			for( long n=0; n<aBulletTrace; n++ )
			{
				// время идет
				aBulletTrace[n].fLifeTime += fDeltaTime;

				// текущая дистанция
				float dist = aBulletTrace[n].fLifeTime * aBulletTrace[n].speed;
				// пока дистанция меньше минимальной - не рисуем трейл
				if( dist < traceparam.fMinShowDist )
					continue;

				// если кончилось время жизни или трейл долетел до цели, то убиваем трейл
				if( dist > aBulletTrace[n].maxDist )
				{
					aBulletTrace.DelIndex( n );
					n--;
					continue;
				}

				float fk = dist * traceparam.fTraceDistanceDiv;
				float length = traceparam.length + fk * traceparam.lengthAdd;
				float width = traceparam.width + fk * traceparam.widthAdd;

				// отсечение хвоста
				if( dist-length < traceparam.fMinShowDist )
					length = dist - traceparam.fMinShowDist;
				if( length <= 0.f )
					continue;

				Vector vb = aBulletTrace[n].vBeg + aBulletTrace[n].vDir * dist;
				Vector ve = aBulletTrace[n].vBeg + aBulletTrace[n].vDir * (dist-length);

				long i = nDrawQnt * 3;

				// bullet
				Matrix tmpview = Render().GetView();
				Vector vleft = tmpview.MulNormalByInverse(Vector(1.f,0.f,0.f)) * 0.5f * width;
				Vector vup = tmpview.MulNormalByInverse(Vector(0.f,1.f,0.f)) * 0.5f * width;

				if( vrt )
				{
					vrt[i+3].p = vrt[i].p = vb - vleft - vup;
					vrt[i].color = 0xFFFFFFFF;
					vrt[i].u = 0.f;
					vrt[i].v = 1.f;
					vrt[i+1].p = vb - vleft + vup;
					vrt[i+1].color = 0xFFFFFFFF;
					vrt[i+1].u = 0.f;
					vrt[i+1].v = 0.f;
					vrt[i+4].p = vrt[i+2].p = vb + vleft + vup;
					vrt[i+2].color = 0xFFFFFFFF;
					vrt[i+2].u = 1.f;
					vrt[i+2].v = 0.f;

					vrt[i+3].color = 0xFFFFFFFF;
					vrt[i+3].u = 0.f;
					vrt[i+3].v = 1.f;
					vrt[i+4].color = 0xFFFFFFFF;
					vrt[i+4].u = 1.f;
					vrt[i+4].v = 0.f;
					vrt[i+5].p = vb + vleft - vup;
					vrt[i+5].color = 0xFFFFFFFF;
					vrt[i+5].u = 1.f;
					vrt[i+5].v = 1.f;
				}

				// trail
				if( vrt2 )
				{
					vrt2[i+3].p = vrt2[i].p = ve - vup;
					vrt2[i].color = 0xFFFFFFFF;
					vrt2[i].u = 0.f;
					vrt2[i].v = 1.f;
					vrt2[i+1].p = vb - vup;
					vrt2[i+1].color = 0xFFFFFFFF;
					vrt2[i+1].u = 0.f;
					vrt2[i+1].v = 0.f;
					vrt2[i+4].p = vrt2[i+2].p = vb + vup;
					vrt2[i+2].color = 0xFFFFFFFF;
					vrt2[i+2].u = 1.f;
					vrt2[i+2].v = 0.f;

					vrt2[i+3].color = 0xFFFFFFFF;
					vrt2[i+3].u = 0.f;
					vrt2[i+3].v = 1.f;
					vrt2[i+4].color = 0xFFFFFFFF;
					vrt2[i+4].u = 1.f;
					vrt2[i+4].v = 0.f;
					vrt2[i+5].p = ve + vup;
					vrt2[i+5].color = 0xFFFFFFFF;
					vrt2[i+5].u = 1.f;
					vrt2[i+5].v = 1.f;
				}

				nDrawQnt += 2;

				//Render().DrawLine( aBulletTrace[n].vBeg, 0xFFFFFFFF, aBulletTrace[n].vBeg + aBulletTrace[n].vDir * aBulletTrace[n].maxDist, 0xFFFFFFFF );
			}
			if( vrt2 ) traceparam.pvbTraceBuffer->Unlock();
			if( vrt ) traceparam.pvbBallBuffer->Unlock();
		}
	}

	if( nDrawQnt > 0 )//&& traceparam.pTraceTexture && traceparam.pvbTraceBuffer )
	{
		Render().SetWorld( Matrix() );
		ShaderId id;

		// bullet
		if( traceparam.pBallTexture )
		{
			Render().GetShaderId("Minigun_Bullet", id);
			traceparam.vTex->SetTexture( traceparam.pBallTexture );
			Render().SetStreamSource(0, traceparam.pvbBallBuffer, sizeof(Vertex));
			Render().DrawPrimitive(id, PT_TRIANGLELIST, 0, nDrawQnt);
		}
		// trail
		if( traceparam.pTraceTexture )
		{
			Render().GetShaderId("Minigun_Trail", id);
			traceparam.vTex->SetTexture( traceparam.pTraceTexture );
			Render().SetStreamSource(0, traceparam.pvbTraceBuffer, sizeof(Vertex));
			Render().DrawPrimitive(id, PT_TRIANGLELIST, 0, nDrawQnt);
		}
	}

	shooterEffect.Process( fDeltaTime );

	Render().SetView(oldview);
}

int Minigun::CheckCollision (const Vector& from, const Vector& to, float damage)
{
	float fFarDist = 1.9f;
	int idx = -1;

	for (dword i = 0; i < 4; i++)
	{
		if (targets[i])
		{
			targets[i]->UpdateColliderPositions (true);
		}
	}

	IPhysicsScene::RaycastResult HitDetail;
	IPhysBase* result = Physics().Raycast(from, to, phys_mask(COLLIDERS_GROUP), &HitDetail);
		
	if (HitDetail.mtl == pmtlid_air) result = NULL;

	if (result == NULL)
	{				
		{			
			Vector v_min = from;
			v_min.Min(to);
			Vector v_max = from;
			v_max.Max(to);

			//Пройдёмся по демедж-объектам
			dword count = QTFindObjects(MG_DAMAGEACCEPTOR, v_min, v_max);

			for(dword j = 0; j < count; j++)
			{
				DamageReceiver & mo = (DamageReceiver &)QTGetObject(j)->GetMissionObject();
				if (mo.Attack(this, DamageReceiver::ds_shooter, damage, from, to))
				{
					return -4;
				}
			}			
		}

		result = Physics().Raycast(from, to, phys_mask(phys_ragdoll), &HitDetail);
		if (result != NULL)
		{
			if (result->UserNumber() == 666) return -5;
		}

		return -2;
	}	


	for (dword i = 0; i < 4; i++)
	{
		if (targets[i])
		{
			if(targets[i]->CheckHit(result,HitDetail.position,HitDetail.normal))
			{
				return -3;
			}
		}
	}

	
	return -1;
}

void Minigun::SetWeaponTransform ()
{
	if (!Weapon) return;
	//Matrix mInitAng(vStartAng);
	Matrix mtx(false);

	mtx = Matrix(vViewAng);	

	mtx = mtx;// * mInitAng;
	mtx.pos = vPos;
	Weapon->SetTransform(mtx);

	if( AdditionalLocator.isValid() )
	{
		Matrix mtxTmp;
		Weapon->GetNodeWorldTransform(AdditionalLocator, mtxTmp);
		if( WeaponAddition )
			WeaponAddition->SetTransform( mtxTmp );
		if( WeaponLoader )
			WeaponLoader->SetTransform( mtxTmp );
	}
}

void Minigun::DrawModel()
{
	if (!Weapon) return;
	Weapon->Draw();
	if( WeaponAddition )
		WeaponAddition->Draw();
	if( WeaponLoader && IsActive() )
		WeaponLoader->Draw();
}

void Minigun::Fire (float deltaTime)
{
	if (GetState() == WEAPON_READY_TO_FIRE)
	{
		if (pAnim)
		{
			pAnim->ActivateLink("action");
		}
		if( pAnimLoader )
		{
			pAnimLoader->ActivateLink("start");
		}
	}

	if( GetState() != WEAPON_LOADING )
		blender.AddForce(5.f);

	if (GetState() != WEAPON_FIRING) return;

	if( pAnimLoader )
	{
		pAnimLoader->ActivateLink("fire");
	}

	if( !bShootMoment ) return;
	bShootMoment = false;
/*	fShootTime -= deltaTime;
	if( fShootTime > 0.f ) return;
	fShootTime = fShootInterval;*/

	// обновляем косточку вращения ствола при стрельбе
	blender.AddForce(5.f);

	if (bLogicDebug)
	{
		const char* animNode = pAnim->CurrentNode();
		const char* shooterState = "unknown";
		
		switch (GetState())
		{
			case WEAPON_EMPTY: shooterState = "Пустой";
			case WEAPON_LOADING: shooterState = "Заряжается";
			case WEAPON_READY_TO_FIRE: shooterState = "Готов к стрельбе";
			case WEAPON_FIRING: shooterState = "Стреляет";
		}

		Console().Trace(COL_ALL, "Fire from node : '%s', status : '%s'", animNode, shooterState);
	}


	m_nCurShootLocatorIndex++;
	if( m_nCurShootLocatorIndex >= m_nShootLocatorsQuantity )
		m_nCurShootLocatorIndex = 0;
	if (m_nCurShootLocatorIndex >= 0 && ShootLocator[m_nCurShootLocatorIndex].isValid() )//ShootLocator)
	{
		Matrix mShoot;
		Weapon->GetNodeWorldTransform(ShootLocator[m_nCurShootLocatorIndex],mShoot);

		if (fxShoot.sound.NotEmpty())
		{
			Sound().Create3D(fxShoot.sound, mShoot.pos, _FL_);
		}

		IParticleSystem* pSys = Particles().CreateParticleSystem( fxShoot.particle.c_str() );
		if (pSys)
		{
			pSys->SetTransform(mShoot);
			pSys->AttachTo(Weapon,ShootLocator[m_nCurShootLocatorIndex],false);
			pSys->AutoDelete(true);
		}
	}

	// добавить трейс
	AddTrace(deltaTime);

	// запуск эффектов
	shooterEffect.Start();

	// наносим дамаг цели
	RememberTargetDamage();
}

void Minigun::Reload ()
{
/*	if (GetState() == WEAPON_READY_TO_FIRE) return;
	//WeaponStates ws = GetState();
	//if ( ws != WEAPON_EMPTY && ws != WEAPON_READY_TO_FIRE ) return;

	if (ShootLocator)
	{
		Matrix mShoot = ShootLocator->GetWorldTransform();
		if (!string::IsEmpty(ReloadSound))
		{
			Sound().Create3D(ReloadSound, mShoot.pos, _FL_);
		}
	}


	if (pAnim)
	{
		DumpStateToConsole("Activate link ACTION");
		bool bCan = pAnim->ActivateLink("action");
		if( pAnim )
			pAnim->SetPlaySpeed(1.f);
	} else
	{
		api->Trace("Shooter animation not assigned !!!");
	}*/
}

void Minigun::Release()
{
	shooterEffect.Release();

	RELEASE(pSoundRotate);

	if( pAnimLoader )
	{
		if( pLoaderListener )
			pLoaderListener->DelAnimation(pAnimLoader);
		pAnimLoader->Release();
		pAnimLoader = NULL;
	}
	DELETE( pLoaderListener );
	RELEASE(WeaponLoader);

	RELEASE(pAnimAddition);
	RELEASE(WeaponAddition);

	if (pAnim)
	{
		blender.UnregistryBlendStage();
		blender.SetAnimation(NULL);
		if (Listener)
		{
			Listener->DelAnimation(pAnim);
		}
		RELEASE(pAnim);
	}
	DELETE (Listener);
	RELEASE(Weapon);

	RELEASE( pBonusGreedy );

	traceparam.vTex = NULL;

	RELEASE( traceparam.pTraceTexture );
	RELEASE( traceparam.pBallTexture );
	RELEASE( traceparam.pvbTraceBuffer );
	RELEASE( traceparam.pvbBallBuffer );

	for( long n=0; n<m_aWaitingTargets; n++ )
	{
		if( m_aWaitingTargets[n].targtype == TargetDescribe::tt_Ragdoll )
			m_aWaitingTargets[n].phys->Release();
	}
	// освободим регдол если он был
	if( m_targetDescr.type == TargetDescribe::tt_Ragdoll )
	{
		RELEASE(m_targetDescr.phys);
	}
}

void _cdecl Minigun::RealizeProcess(float fDeltaTime, long level)
{
	UpdateColliderPositions();
	// выстрелы обрабатываются даже если выйти из шутера
	UpdateTargetDamage(fDeltaTime);

	if (!IsActive())
	{
		bool bDelRealize = true;
		if (aBulletTrace.Size() != 0 )
			bDelRealize = false;
		if (m_aWaitingTargets.Size() != 0 )
			bDelRealize = false;
		if (pAnimAddition)
		{
			const char* st = pAnimAddition->GetConstString("Shooter state");
			if( st && st[0]=='f' )
				pAnimAddition->Pause(true);
			else
				bDelRealize = false;
		}
		if( bDelRealize )
			DelUpdate(&Minigun::RealizeProcess);
		return;
	}

	// обновляем звук вращения ствола
	if( pSoundRotate )
	{
		float fRelAngSpeed = blender.GetRelativeAngularSpeed();
		pSoundRotate->SetVolume( fRelAngSpeed );
		if( fRelAngSpeed==0.f )
		{
			if( pSoundRotate->IsPlay() )
				pSoundRotate->Stop();
		}
		else
		{
			if( !pSoundRotate->IsPlay() )
				pSoundRotate->Play();
		}
	}

	if(!EditMode_IsVisible()) return;

	//if (GetState() == WEAPON_EMPTY) Reload();

	if (fovChgCurTime>0.0f)
	{
		fovChgCurTime -= fDeltaTime;

		if (fovChgCurTime<0.0f)
		{
			fovChgCurTime = -1.0f;


			if (zoomOn)
			{
				fov = nearFOV;
			}
			else
			{
				fov = farFOV;
			}
		}
		else
		{
			if (zoomOn)
			{
				fov = farFOV + (nearFOV - farFOV)*(1.0f - fovChgCurTime/fovChgTime);
			}
			else
			{
				fov = nearFOV + (farFOV - nearFOV)*(1.0f - fovChgCurTime/fovChgTime);
			}
		}
	}

	if (bLogicDebug && pAnim)
	{
		const char* animNode = pAnim->CurrentNode();
		const char* shooterState = "unknown";
		
		switch (GetState())
		{
			case WEAPON_EMPTY: shooterState = "Пустой";
			case WEAPON_LOADING: shooterState = "Заряжается";
			case WEAPON_READY_TO_FIRE: shooterState = "Готов к стрельбе";
			case WEAPON_FIRING: shooterState = "Стреляет";
		}

		Render().Print(0, 40, 0xFFFFFFFFL, "Anim node: %s, Shooter state : %s, Хочу стрелять: %d", animNode, shooterState, bWantToShoot);
		//Render().Print(0, 60, 0xFFFFFFFFL, "Активных пуль: %d, Не перезаряжать пока активные пули : %d", Bullets.Size(), bDontReloadWhileActiveBullets);
	}	

	SetWeaponTransform();

	if (GetState() != WEAPON_READY_TO_FIRE && !bWantToShoot)
	{
		StopShooter();
	}

	if( pAnimAddition && WeaponAddition )
	{
		if( pAnimAddition->IsPause() )
		{
			if( GetState() == WEAPON_FIRING )
			{
				pAnimAddition->Pause( false );
				pAnimAddition->SetPlaySpeed( m_fBulletAnimationSpeed );
			}
		}
		else
		{
			const char * st = pAnimAddition->GetConstString("Shooter state");
			if(st && st[0]=='l')
			{
				if( pAnim ) pAnim->ActivateLink("reload");
				if( pAnimLoader ) pAnimLoader->ActivateLink("reload");
				pAnimAddition->SetPlaySpeed( 1.f );
				/*if( m_pTiedCameraID && m_pTiedCameraID[0] )
				{
					if( m_pTiedCamera.Validate() )
					{
						MOSafePointer mocam;
						MissionObject::FindObject("Camera Controller",mocam);
						if (mocam.Ptr())
						{
							const char* params[3];
							params[0] = "0.3";
							params[1] = "0.2";
							params[2] = "30";
							mocam.Ptr()->Command("shockPolar",3,params);
						}
					}
				}*/
			}
			else
				if( GetState() != WEAPON_FIRING )
					pAnimAddition->Pause( true );
		}
	}

	if( bWantToShoot )
	{
		Fire(fDeltaTime);
		bWantToShoot = false;
	}

	if (!IsActive()) return;

	//Обычная камера с орудия...
	if( m_pTiedCameraID.IsEmpty() || !m_pTiedCamera.Validate() )
	{
		float fWidth = (float)Render().GetViewport().Width;
		float fHeight = (float)Render().GetViewport().Height;
		float fCamFov = recalculateFov(fWidth, fHeight, Render().GetWideScreenAspectWidthMultipler(), fov);

		Matrix mProjection;
		mProjection.BuildProjection(fCamFov, fWidth, fHeight, NearPlane, FarPlane);
		Render().SetProjection(mProjection);

		Matrix m;
		if (CameraLocator.isValid())
		{
			Weapon->GetNodeWorldTransform(CameraLocator,m);
			m.Normalize ();
			m.Inverse();
		}
		Render().SetView(m);

		// смещение конуса автоприцела так что бы вершина совпадала с вершиной фруструма камеры
		// (тогда величина прицела будет константной)
		m_fSightOffset = -NearPlane;
	}
	else
	{
		char par1[24];
		crt_snprintf( par1, sizeof(par1), "%f", (float)(fov * (180.f/PI) /** m_fWideScreenAspectDividerConst*/) );
		const char* param[2];
		param[0] = par1;
		param[1] = "0";
		m_pTiedCamera.Ptr()->Command( "setFov", 2, param );

		// получаем ближний план из матрицы проекции
		m_fSightOffset = Render().GetProjection().m[3][2] / Render().GetProjection().m[2][2];
	}

	// сброс перезарядки (так что бы сначала было заданное количество зацикленных выстрелов)
	if( nRepeatableShootLeft==0 && pAnim )
	{
		const char* pcRep = pAnim->CurrentNode();
		if( pcRep && pcRep[0]=='r' &&
			string::IsEqual( pcRep, "reload" ) )
			nRepeatableShootLeft = nRepeatableShootQuantity;
	}
}

void Minigun::StopShooter()
{
	if (pAnim)
	{
		DumpStateToConsole("Activate link ACTION");
/*		if (pAnim->ActivateLink("stopshoot"))
			blender.AddForce(5.0f);*/
		pAnim->ActivateLink("stopshoot");
	}
	if( pAnimLoader )
	{
		pAnimLoader->ActivateLink("stop");
	}
	fShootTime = 0.f;
}

void Minigun::EditMode_GetSelectBox(Vector & min, Vector & max)
{
	min = Vector(-1.0f);
	max = Vector(1.0f);
}

void Minigun::Activate(bool isActive)
{
	if( IsActive() != isActive )
	{
		RELEASE( pSoundRotate );
		if (isActive)
		{
			zoomOn = init_zoomOn;
			fovChgCurTime = -1.0f;

			if (zoomOn)
			{
				fov = nearFOV;
			}
			else
			{
				fov = farFOV;
			}

			BuildPointers();
			//enter game
			if (!CameraLocator.isValid())
			{
				api->Trace("Minigun: Camera locator not found !");
			}

			if (!BulletLocator.isValid())
			{
				api->Trace("Minigun: Bullet locator not found !");
			}

			if (!ShootLocator)
			{
				api->Trace("Minigun: Shoot locator not found !");
			}

			
			if (!Weapon)
			{
				api->Trace("Minigun: Weapon model not found !");
			}

			ControlStateType cst = Mission().Controls().GetControlStateType("Weapon_Exit");
			if(cst == CST_ACTIVATED || cst == CST_ACTIVE )
			{
				wasActivated = 2;
			}
			else
			{
				wasActivated = 1;
			}

			if(IsShow())
			{
				SetUpdate(&Minigun::RealizeControl, ML_FIRST);
				SetUpdate(&Minigun::RealizeProcess, ML_CAMERAMOVE + 5);
				SetUpdate(&Minigun::DrawTrails, ML_PARTICLES5);
			}
			else
			{
				DelUpdate(&Minigun::RealizeControl);
				DelUpdate(&Minigun::RealizeProcess);
				DelUpdate(&Minigun::DrawTrails);
			}
			
			triger_onActivate.Activate(Mission(), false);
			api->Trace("Minigun %s: Activate triger was activated", GetObjectID().c_str());

			//Reload();		

			//Registry(MG_BONUSPICKUPER);
			if( !pBonusGreedy && Mission().Player()!=null )
			{
				pBonusGreedy = BonusesManager::CreateGreedy( Mission().Player() );
				if( pBonusGreedy )
					pBonusGreedy->SetPosition( vPos );
			}

			// прицел включен
			if( m_pSightGUI_Miss ) m_pSightGUI_Miss->Show( true );
			if( m_pSightGUI_Hit ) m_pSightGUI_Hit->Show( false );

			// ставим подсказку что в шутере
			api->Storage().SetString( "Profile.Runtime.GameHint", "Shooter" );

			// включаем камеру
			if( m_pTiedCameraID.NotEmpty() && m_pTiedCamera.Validate() )
			{
				const char* param[1];
				param[0] = GetObjectID().c_str();
				m_pTiedCamera.Ptr()->Command("Target",1,param);
				m_pTiedCamera.Ptr()->Activate(true);
			}

			// заводим звук вращения ствола
			if( RotateSound.NotEmpty() )
			{
				Vector vSoundPos;
				if( Weapon && CameraLocator.isValid() )
				{
					Matrix mtxTmp;
					Weapon->GetNodeWorldTransform(CameraLocator,mtxTmp);
					vSoundPos = mtxTmp.pos;
				}
				else
					vSoundPos = vPos;
				pSoundRotate = Sound().Create3D( RotateSound, vSoundPos, _FL_, false, false );
			}
		}
		else
		{
			triger_onDeactivate.Activate(Mission(), false);
			api->Trace("Minigun %s: Exit triger was activated", GetObjectID().c_str());		

			DelUpdate(&Minigun::RealizeControl);
			if (pAnim)
			{
				pAnim->Start();
			}
			if (pAnimAddition)
			{
				pAnimAddition->Start();
			}
			if (pAnimLoader)
			{
				pAnimLoader->Start();
			}

			RELEASE( pBonusGreedy );

			// прицел выключен
			if( m_pSightGUI_Miss ) m_pSightGUI_Miss->Show( false );
			if( m_pSightGUI_Hit ) m_pSightGUI_Hit->Show( false );

			// убираем подсказку что в шутере
			api->Storage().SetString( "Profile.Runtime.GameHint", "none" );

			// выключаем камеру
			if( m_pTiedCameraID.NotEmpty() && m_pTiedCamera.Validate() )
				m_pTiedCamera.Ptr()->Activate(false);
		}
	}
	MissionObject::Activate(isActive);
}

void Minigun::DumpStateToConsole (const char* mess)
{
	if (!bLogicDebug) return;
	const char* animNode = pAnim->CurrentNode();
	const char* shooterState = "unknown";

	switch (GetState())
	{
		case WEAPON_EMPTY: shooterState = "Пустой";
		case WEAPON_LOADING: shooterState = "Заряжается";
		case WEAPON_READY_TO_FIRE: shooterState = "Готов к стрельбе";
		case WEAPON_FIRING: shooterState = "Стреляет";
	}

	Console().Trace(COL_ALL, "%s : '%s', status : '%s'", mess, animNode, shooterState);
}

Minigun::WeaponStates Minigun::GetState()
{	
	if (!pAnim) return WEAPON_EMPTY;
	//Доступиться до константы типа string
	const char * st = pAnim->GetConstString("Shooter state");
	
	if(!st) return WEAPON_EMPTY;

	switch (st[0])
	{
		case 'e': return WEAPON_EMPTY;
		break;
		case 'l': return WEAPON_LOADING;
		break;
		case 'r': return WEAPON_READY_TO_FIRE;
		break;
		case 'f': return WEAPON_FIRING;
		break;
	}

	return WEAPON_EMPTY;
}

void Minigun::BuildPointers ()
{
	if (bPointerBuilded && !EditMode_IsOn()) return;

	for (dword i = 0; i < 4; i++)
	if (targets[i])
	{
		targets[i]->BuildPointers();
	}

	bPointerBuilded = true;
}

void Minigun::GetBox(Vector & min, Vector & max)
{
	if(Weapon)
	{
		min = Weapon->GetLocalBound().vMin;
		max = Weapon->GetLocalBound().vMax;
		return;
	}
	else
	{
		min = 0.01f;
		max = 0.01f;
	}	
}

void _cdecl Minigun::ShadowDraw ()
{
	if(!EditMode_IsVisible()) return;
	if (Weapon)
	{
		Weapon->Draw();
	}
	if( WeaponAddition )
	{
		WeaponAddition->Draw();
	}
	if( WeaponLoader && IsActive() )
	{
		WeaponLoader->Draw();
	}
}

void Minigun::Show(bool bShow)
{
	if(bShow)
	{
		SetUpdate(&Minigun::RealizeDraw, ML_GEOMETRY1);

		if(bShadowCast)
		{
			Registry(MG_SHADOWCAST, (MOF_EVENT)&Minigun::ShadowDraw, ML_GEOMETRY1);
			Registry(MG_SHADOWRECEIVE, (MOF_EVENT)&Minigun::ShadowDraw, ML_GEOMETRY1);
		}
		else
		{
			Unregistry(MG_SHADOWCAST);
			Unregistry(MG_SHADOWRECEIVE);
		}

		MissionObject::Show(bShow);
		//Activate(IsActive());
	}
	else
	{
		Unregistry(MG_SHADOWCAST);
		Unregistry(MG_SHADOWDONTRECEIVE);
		DelUpdate(&Minigun::RealizeDraw);

		bool isAct = IsActive();
		//Activate(false);
		//MissionObject::Activate(isAct);
		MissionObject::Show(bShow);
	}
}


void Minigun::UpdateColliders()
{
	for (dword i = 0; i < 4; i++)
	if (targets[i])
	{
		targets[i]->UpdateColliders();
	}	
}

void Minigun::ChangeValue(float &value, float targetValue, float dltTime, float accel)
{
	if (targetValue>value)
	{
		value += dltTime * accel;

		if (targetValue<value)
		{
			value = targetValue;
		}
	}
	else
	{
		value -= dltTime * accel;

		if (targetValue>value)
		{
			value = targetValue;
		}
	}

}

bool Minigun::CheckDirection(const Vector& pos)
{
	Matrix mtx(vViewAng, vPos);
	return ((pos - vPos) ^ mtx.vz) > 0.f;
}

void Minigun::ZoomEnable(bool enable)
{
	if (zoomOn == enable) return;

	zoomOn = enable;

	if (fovChgCurTime<0.0f)
	{
		fovChgCurTime = fovChgTime;
	}
	else
	{
		fovChgCurTime = fovChgTime - fovChgCurTime;
	}
}

//Обработчик команд для объекта
void Minigun::Command(const char * id, dword numParams, const char ** params)
{
	if (string::IsEqual(id,"LocPrt"))
	{
		if( Listener )
			Listener->LocParticles(pAnim,"LocPrt",params,numParams);
	}
	else
	if (string::IsEqual(id,"DelLocPrt"))
	{
		if( Listener )
			Listener->LocParticles(pAnim,"DelLocParticles",params,numParams);
	}
}

void Minigun::Restart()
{
	for (dword i = 0; i < 4; i++)
	if (targets[i])
	{
		targets[i]->RestoreLife();
	}

	Release();

	ReCreate();

	if( m_pTiedCameraID.NotEmpty() )
		MissionObject::FindObject(m_pTiedCameraID,m_pTiedCamera);

	Activate (init_active_state);
}


void Minigun::UpdateSight()
{
	// определяем направление стрельбы
	if( !CameraLocator.isValid() ) return;
	// обновляем матрицу разброса
	if( fDispersionAngle == 0.f )
		Weapon->GetNodeWorldTransform(CameraLocator,mtxShootDirection);
	else
	{
		Vector vrand;
		vrand.RandXZ();
		float fsindisp = sinf( FRAND(fDispersionAngle) );
		Vector vto = Vector(vrand.x * fsindisp, vrand.z * fsindisp, 1.f);

		vto.Normalize();
		mtxShootDirection.vz = vto;
		mtxShootDirection.vx = Vector(0.f,1.f,0.f) ^ vto;
		mtxShootDirection.vx.Normalize();
		mtxShootDirection.vy = vto ^ mtxShootDirection.vx;
		mtxShootDirection.pos = 0.f;

		Matrix mtxTmp;
		Weapon->GetNodeWorldTransform(CameraLocator,mtxTmp);
		mtxShootDirection *= mtxTmp;
		//mtxShootDirection *= CameraLocator->GetWorldTransform();
	}
	Matrix mWorld(true);
	if( useSwingCorrection )
		mWorld = Mission().GetInverseSwingMatrix() * mtxShootDirection;
	else
		mWorld = mtxShootDirection;
	Vector vDir = mWorld.vz;

	// освободим регдол если он был заюзан
	if( m_targetDescr.type == TargetDescribe::tt_Ragdoll )
	{
		RELEASE(m_targetDescr.phys);
	}
	// находим цель для стрельбы
	bool bYesTarget = FindNearTarget( mWorld.pos + m_fMinShootDist*vDir, vDir, m_fMaxShootDist-m_fMinShootDist, GetAutoAimSin(), m_targetDescr );
	// рандомизируем точку попадания
	if( bYesTarget )
	{
		RandomizeTargetPoint( m_targetDescr );
		// для регдола добавляем референс (что бы неотрелизился регдол без нашего ведома
		if( m_targetDescr.type == TargetDescribe::tt_Ragdoll )
			m_targetDescr.phys->AddRef();
	}

	// смена прицела (попадаем или нет по цели)
	if( bYesTarget && m_targetDescr.type >= TargetDescribe::tt_AutoAimedTarget )
	{
		if( m_pSightGUI_Hit && !m_pSightGUI_Hit->IsShow() )
			m_pSightGUI_Hit->Show( true );
		if( m_pSightGUI_Miss && m_pSightGUI_Miss->IsShow() )
			m_pSightGUI_Miss->Show( false );
	}
	else
	{
		if( m_pSightGUI_Hit && m_pSightGUI_Hit->IsShow() )
			m_pSightGUI_Hit->Show( false );
		if( m_pSightGUI_Miss && !m_pSightGUI_Miss->IsShow() )
			m_pSightGUI_Miss->Show( true );
	}
}

float Minigun::GetAutoAimSin()
{
//	float ang = m_fSightAng;
//	if( nearFOV > 0.01f && nearFOV != fov )
//		ang *= fov / nearFOV;
//	return sinf(ang);
	return sinf(m_fSightSize*fov*0.5f);
}

void Minigun::RandomizeTargetPoint( TargetDescribe & td )
{
	// для рандомизации подходят только спец таргеты для шутера или дамаг акцепторы
	if( td.type != TargetDescribe::tt_DamageAcceptor &&
		td.type != TargetDescribe::tt_ShooterTarget )
		return;

	// сфера в которой возможна рандомизация (что бы трейл не выходил за пределы прицела)
	Sphere randSphere;
	// матрица положения камеры
	Matrix mView = Render().GetView();
	mView.Inverse();
	// направление стрельбы
	Vector vTargDir = td.pos - mView.pos;
	// направление центра камеры (там находится центр прицела)
	Vector vCenterDir = mView.vz;
	// расстояние до плоскости через цель
	float fdist = vCenterDir | vTargDir;
	// радиус сферы рандомизации точки попадания
	//float ang = m_fSightAng;
	//if( nearFOV > 0.01f && nearFOV != fov )
	//	ang *= fov / nearFOV;
	//randSphere.r = fabs(fdist * tanf(ang));
	randSphere.r = fabs(fdist * tanf(m_fSightSize*fov*0.5f));
	// центр сферы рандомизации
	randSphere.p = mView.pos + vCenterDir * fdist;

	if( td.type == TargetDescribe::tt_ShooterTarget )
		td.pos = ((ShooterTargets*)td.mo)->GetRandomPoint( td.pos, randSphere );
	else
		td.pos = GetRandomPointFromMO( mView.pos, td.pos, randSphere, (DamageReceiver*)td.mo );
}

Vector Minigun::GetRandomPointFromMO( const Vector & from, const Vector & to, const Sphere & sph, DamageReceiver* mo )
{
	if( !mo ) return to;

	// рандомная точка в сфере
	Vector v;
	v.Rand();
	v *= rand()*(sph.r*(1.0f/RAND_MAX));
	v += sph.p;

	// приближение от рандомной точки к стартовой точке,
	// что бы новое попадание не вышло за пределы цели
	Vector vdelta = (to - v) * 0.2f;
	for( long n=0; n<5; n++ )
	{
		// проверяем находится ли текущая точка в объекте
		if ( mo->Attack( this, DamageReceiver::ds_check, 0.f, from, v ) )
			return v;
		v += vdelta;
	}
	return to;
}

bool Minigun::FindNearTarget( const Vector& pos, const Vector& dir, float fmaxdist, float fApproachSin, TargetDescribe & td )
{
	dword i;

	// до куда трейсим
	float fcurdist = fmaxdist;
	Vector to = pos + dir * fmaxdist;

	//Render().DrawLine( pos, 0xFFFFFFFF, to, 0xFFFFFFFF );

	// по умолчанию мы никуда не попадаем
	td.type = TargetDescribe::tt_none;
	td.mo = null;
	td.pos = to;
	td.phys = null;

	// обновим позиции коллидеров для целей шутера
	for (i = 0; i < 4; i++)
		if (targets[i])
			targets[i]->UpdateColliderPositions (true);

	// проверим на пресечение трейса с уровнем воды (попадаем в воду)
	//------------------------------------------------------------------------
	float fWaterLevel = IWaterLevel::GetWaterLevel(Mission(), waterLevel);
	if( to.y < fWaterLevel && pos.y > fWaterLevel )
	{
		Plane WaterPlane(Vector(0.f,1.f,0.f), fWaterLevel);

		Vector vres = to;
		if( WaterPlane.Intersection( pos, to, vres ) )
		{
			to = vres;
			fcurdist = (to-pos).GetLength();
			td.pos = Vector(vres.x, fWaterLevel, vres.z );
			td.normal = Vector( 0.f, 1.f, 0.f );
			td.mo = null;
			td.phys = null;
			td.type = TargetDescribe::tt_waterlevel;
		}
	}

	// смотрим коллидер в который упирается луч трейса
	//------------------------------------------------------------------------
	IPhysicsScene::RaycastResult HitDetail;
	IPhysBase* result = Physics().Raycast(pos + dir*DONT_SHOOT_DISTANCE, to, phys_mask(COLLIDERS_GROUP), &HitDetail);
	// коллидер в воздухе не считаем за пересечение
	if (HitDetail.mtl == pmtlid_air) result = NULL;
	// если воткнулись во что то, то считаем эту точку конечной для трейса
/*	if( result != NULL )
	{
		to = HitDetail.position;
		fcurdist = (to-pos).GetLength();
		td.type = TargetDescribe::tt_Collider;
		td.mo = null;
		td.pos = HitDetail.position;
		td.normal = HitDetail.normal;
		td.phys = result;
	}*/
	// проверяем попадание в специальные цели для шутера
	//------------------------------------------------------------------------
	if( result != NULL )
	{
		for (i = 0; i < 4; i++)
			if (targets[i])
			{
				bool bPassiveTarget = false;
				if( targets[i]->IsMyCollider( result, bPassiveTarget ) )
				{
					to = HitDetail.position;
					fcurdist = (to-pos).GetLength();
					td.type = bPassiveTarget ? TargetDescribe::tt_PassiveShooterTarget : TargetDescribe::tt_ShooterTarget;
					td.mo = targets[i];
					td.pos = HitDetail.position;
					td.normal = HitDetail.normal;
					td.phys = result;
					// если мы не стреляем сейчас, то можно выходить подсветка будет и так
					if( !bWantToShoot && !bPassiveTarget ) return true;
				}
			}
	}

	// проверим попадание в регдолы
	//------------------------------------------------------------------------
	Vector vadd = 0.f;
	if( bWantToShoot && td.type == TargetDescribe::tt_PassiveShooterTarget )
		vadd = dir * 0.5f;
/*	result = Physics().Raycast(pos, to+vadd, phys_mask(phys_ragdoll), &HitDetail);
	if (result != NULL && result->UserNumber() == 666 && fRagdollBulletImpulse>0.f )
	{
		to = HitDetail.position;
		fcurdist = (to-pos).GetLength();
		td.type = TargetDescribe::tt_Ragdoll;
		td.mo = null;
		td.pos = HitDetail.position;
		td.normal = HitDetail.normal;
		td.phys = result;
	}*/

	// проверим попадание в дамаг ресиверы
	//------------------------------------------------------------------------
	Vector v_min = pos;	v_min.Min(to);
	Vector v_max = pos;	v_max.Max(to);

	//Пройдёмся по демедж-объектам
	dword count = QTFindObjects(MG_DAMAGEACCEPTOR, v_min, v_max);
	for(dword j = 0; j < count; j++)
	{
		DamageReceiver & mo = (DamageReceiver &)QTGetObject(j)->GetMissionObject();

		if( !mo.IsShow() )
			continue;
		if( mo.IsDead() )
			continue;

		Matrix mtx(true);
		mo.GetMatrix(mtx);

		if ( mo.Attack( this, DamageReceiver::ds_check, 0.f, pos, to ) )
		{
			fcurdist = dir | (mtx.pos-pos);
			td.type = TargetDescribe::tt_DamageAcceptor;
			td.mo = &mo;
/*			if( mo.Is("Ship") )
			{
				Vector minsize, maxsize;
				mo.GetBox( minsize, maxsize );
				to = pos + (fcurdist+maxsize.z) * dir;
				td.pos = to;
			}
			else
			{
				to = pos + fcurdist * dir;
				td.pos = mtx.pos;
			}*/
			Vector minsize, maxsize;
			mo.GetBox( minsize, maxsize );
			to = pos + ( fcurdist + Max(maxsize.y,Max(maxsize.z,maxsize.x)) ) * dir;
			td.pos = to;

			td.normal = !(pos-to);
			td.phys = null;

			// если мы не стреляем сейчас, то можно выходить подсветка будет и так
			if( !bWantToShoot ) return true;
		}
	}

	// если мы не попали ни в кого, то попробуем вторым лучем с автодоводкой
	if( td.type < TargetDescribe::tt_AutoAimedTarget && fApproachSin > 0.f )
	{
		// проверим попадание в дамаг ресиверы вторым лучем
		//count = QTFindObjects(MG_DAMAGEACCEPTOR, v_min, v_max);
		for ( j = 0; j < count; j++ )
		{
			DamageReceiver & mo = (DamageReceiver &)QTGetObject(j)->GetMissionObject();

			if( !mo.IsShow() )
				continue;
			if( mo.IsDead() )
				continue;

			Matrix mtx(true);
			mo.GetMatrix(mtx);

			Vector dir2targ = mtx.pos - pos;
			// дистанция до цели по направлению трейса
			float fdist = dir2targ | dir;
			// будет трейс только если цель ближе конца трейса
			if( fdist > fcurdist ) continue;
			// совсем ближние цели не трейсим
			if( fdist < 1.f ) continue;

			// находим второй вектор для трейса который приближен на расстояние автодоводки
			Vector targOnTrace = pos + dir * fdist;				// позиция на трейсе на той же дистанции что и цель
			float foffset = (mtx.pos-targOnTrace).GetLength();	// расстояние от позиции цели до позиции на трейсе
			// если растояние коррекции меньше минимального, то просто трейсим до цели
			Vector to2;
			if( foffset <= (fdist+m_fSightOffset) * fApproachSin )
				to2 = mtx.pos;
			// если растояние коррекции больше минимального, то ищем точку до которой надо трейсить
			else
				to2 = targOnTrace + (mtx.pos - targOnTrace) * ((fdist+m_fSightOffset)*fApproachSin / foffset);

			// собстно трейс
			if ( mo.Attack( this, DamageReceiver::ds_check, 0.f, pos, to2 ) )
			{
				to = targOnTrace;
				fcurdist = (to-pos).GetLength();
				td.type = TargetDescribe::tt_DamageAcceptor;
				td.mo = &mo;
				td.pos = to2;
				td.normal = !(pos-to2);
				td.phys = null;

				// если мы не стреляем сейчас, то можно выходить подсветка будет и так
				if( !bWantToShoot ) return true;
			}
		}

		// проверим корабельные таргеты
		IPhysBase* findedPhys = null;
		Vector findedPos;
		float fFindedSin = fApproachSin; // fixme! тут нужен настоящий синус а мы подставляем тангенс (на малых углах практически без разницы)
		for (i = 0; i < 4; i++)
		{
			if (targets[i])
			{
				IPhysBase* pCurPhys;
				Vector curPos;
				float curOffset;
				if( targets[i]->FindNearestObject( pos, dir, fcurdist, pCurPhys, curPos, curOffset ) )
				{
					float fDist = (curPos - pos).GetLength();
					// ближе метра не палим
					if( fDist < DONT_SHOOT_DISTANCE ) continue;

					float curSin = curOffset / (fDist+m_fSightOffset);
					// выбираем ближайший подходящий (ближайший не по дистанции, а по углу к направлению стрельбы)
					if( curSin < fFindedSin )
					{
						fFindedSin = curSin;
						findedPhys = pCurPhys;
						findedPos = curPos;
					}
				}
			}
		}
		if( findedPhys != null )
		{
			result = Physics().Raycast(pos, findedPos, phys_mask(COLLIDERS_GROUP), &HitDetail);
			// коллидер в воздухе не считаем за пересечение
			if (HitDetail.mtl == pmtlid_air) result = NULL;
			// если воткнулись во что то, то считаем эту точку конечной для трейса
			/*if( result != NULL )
			{
				to = HitDetail.position;
				fcurdist = (to-pos).GetLength();
				td.type = TargetDescribe::tt_Collider;
				td.mo = null;
				td.pos = HitDetail.position;
				td.normal = HitDetail.normal;
				td.phys = result;
			}*/
			// проверяем попадание в специальные цели для шутера
			//------------------------------------------------------------------------
			if( result != NULL )
			{
				for (i = 0; i < 4; i++)
					if (targets[i])
					{
						bool bPassiveTarget = false;
						if( targets[i]->IsMyCollider( result, bPassiveTarget ) )
						{
							if( !bPassiveTarget )
							{
								to = HitDetail.position;
								fcurdist = (to-pos).GetLength();
								td.type = TargetDescribe::tt_ShooterTarget;
								td.mo = targets[i];
								td.pos = HitDetail.position;
								td.normal = HitDetail.normal;
								td.phys = result;
							}
						}
					}
			}
		}
	}

	return td.type != TargetDescribe::tt_none;
}

void Minigun::EffectReadMOPs(Effect & effect, MOPReader & reader)
{
	effect.particle = reader.String();
	effect.sound = reader.String();
}

void Minigun::EffectStart(const Effect & effect, const Vector& pos, const Vector& normal)
{
	if (effect.sound.NotEmpty()) Sound().Create3D(effect.sound, pos, _FL_);

	IParticleSystem* pSys = Particles().CreateParticleSystem(effect.particle.c_str());
	if (pSys)
	{
		Matrix mtx(true);
		mtx.BuildOrient( normal, Vector(0.f,1.f,0.f) );
		mtx.pos = pos;
		pSys->SetTransform(mtx);
		pSys->AutoDelete(true);
	}
}


#define MOP_EFFECT(name) \
	MOP_STRING(name##" particle","") \
	MOP_STRING(name##" sound","")

MOP_BEGINLIST(Minigun, "Minigun", '1.00', 100)

	MOP_STRINGC("WeaponModel", "", "Моделька орудия\nGMX файл");
	MOP_BOOL("WeaponModel Dynamic light", true);
	MOP_STRINGC("WeaponAnimation", "m2crossbow", "Анимация орудия");

	MOP_STRINGC("WeaponAdditionModel", "", "Моделька магазина\nGMX файл");
	MOP_STRINGC("WeaponAdditionAnimation", "", "Анимация магазина");

	MOP_STRINGC("WeaponLoaderModel", "", "Моделька рук\nGMX файл");
	MOP_STRINGC("WeaponLoaderAnimation", "", "Анимация рук");

	MOP_FLOATC("ShootAnimationSpeed", 1.f, "Скорость проигрывания анимации выстрела");

	MOP_LONGC("ShootRepeatQuantity", 0, "Количество зацикленных анимаций выстрелов");

	MOP_FLOAT("Shoot interval", 0.2f);
	MOP_FLOAT("BulletAnimationSpeed", 1.f);

	MOP_POSITIONC("Position", Vector(0.0f), "Позиция");
	MOP_ANGLESEXC("Angles", Vector(0.0f, 0.0f, 0.0f), Vector(0.f, 0.f, 0.0f), Vector(2*PI, 2*PI, 0.0f), "Угол вращения орудия");

	MOP_FLOATEXC("HeightLimitMin", -60.0f, -89.0f, 89.0f, "Верхний предел");
	MOP_FLOATEXC("HeightLimitMax", 60.0f, -89.0f, 89.0f, "Нижний предел");
	MOP_FLOATC("RotateLimitMin", -60.0f, "Левый предел");
	MOP_FLOATC("RotateLimitMax", 60.0f, "Правый предел");

	MOP_FLOATEXC("near FOV", 1.25f*180.0f/PI, 10.0f, 140.0f, "FOV камеры установленной на орудии и на патроне в режи зуминга");
	MOP_FLOATEXC("far FOV", 1.25f*180.0f/PI, 10.0f, 140.0f, "FOV камеры установленной на орудии и на патроне");	
	
	MOP_FLOATEXC("zoom Time", 1, 0.01f, 40.0f, "скорость смены фова камеры");
	MOP_BOOLC("Zoom enbaled", false,"Зоом включен при старте шутера");


	MOP_FLOATEXC("NearPlane", 0.01f, 0.0f, 10.0f, "Ближняя плоскость клипирования камеры");
	MOP_FLOATEXC("FarPlane", 4000.0f, 10.0f, 8000.0f, "Дальняя плоскость клипирования камеры");

	MOP_FLOATEX("MaxJoyRotSpeed", 1.5f, 0.05f, 10.0f)
	MOP_FLOATEX("RotJoyAccelerate", 4.5f, 0.05f, 50.0f)	
	MOP_FLOATEX("MaxMsRotSpeed", 4.5f, 0.05f, 20.0f)
	MOP_FLOATEX("RotMsAccelerate", 15.5f, 0.05f, 50.0f)

	MOP_FLOATEX("MaxJoyRotSpeed zoomed", 0.5f, 0.05f, 10.0f)
	MOP_FLOATEX("RotJoyAccelerate zoomed", 2.5f, 0.05f, 50.0f)
	MOP_FLOATEX("MaxMsRotSpeed zoomed", 2.0f, 0.05f, 20.0f)
	MOP_FLOATEX("RotMsAccelerate zoomed", 7.5f, 0.05f, 50.0f)

	MOP_GROUPBEG ("Shoot distance")
		MOP_FLOATC("Max distance", 1000.f, "Дальность стрельбы")
		MOP_FLOATC("Min distance", 10.f, "Дистанция от шутера ближе которой не попадаем по объекту")
	MOP_GROUPEND ()

	MOP_GROUPBEG ("Effects")
		MOP_EFFECT("Shoot")

		MOP_EFFECT("WaterSplash")
		MOP_EFFECT("Char")
		MOP_EFFECT("Ragdoll")
		MOP_EFFECT("Collider")
	MOP_GROUPEND ()

	MOP_STRINGC("ReloadSound", "", "3D Звук проигрываемый при перезарядке\nИмя ALIASA");
	MOP_STRINGC("RotateSound", "", "3D Звук проигрываемый при кручении ствола\nИмя ALIASA");

	MOP_MISSIONTRIGGERG("onActivate trigger", "onActivate")
	MOP_MISSIONTRIGGERG("onDeactivate trigger", "onDeactivate")

	MOP_MISSIONTRIGGERG("onZoomIn trigger", "onZoomIn")
	MOP_MISSIONTRIGGERG("onZoomOut trigger", "onZoomOut")

	MOP_STRING("Target List 1", "");
	MOP_STRING("Target List 2", "");
	MOP_STRING("Target List 3", "");
	MOP_STRING("Target List 4", "");

	MOP_BOOLC ("Shadow cast", true, "Отбрасывать тень");

	MOP_BOOL ("Explode bullets", false);

	MOP_FLOATEX("Explode Radius", 2.0f, 0.1f, 1000000.0f)
	MOP_FLOAT("Explode Damage", 100.0f)
	MOP_FLOATEX("Explode Power", 1.0f,0.1f,100.0f)

	MOP_FLOATEX("Ragdoll impulse power", 2.0f,0.1f,50.0f)

	MOP_GROUPBEG ("Sight")
		MOP_ARRAYBEG("Complexity", 1, 5)
			MOP_STRINGC("Name", "Normal", "Уровень сложности")
			MOP_FLOATEXC("Autotarget size", 4.f, 0.f, 100.f, "размер автоприцела в процентах (ширина виджета прицела)")
			//MOP_FLOATEXC("Autotarget angle", 2.f, 0.f, 90.f, "угол автонацеливания в градусах")
			//MOP_FLOATEXC("Autotarget offset", 0.f, 0.f, 1000.f, "смещение конуса автонацеливания")
			MOP_STRINGC ("Active widget name", "", "Имя виджета с прицелом который попадает в цель");
			MOP_STRINGC ("Miss widget name", "", "Имя виджета с прицелом который промахивается по цели");
		MOP_ARRAYEND
	MOP_GROUPEND ()

	MOP_FLOATEX("Dispersion angle", 0.f, 0.f, 45.0f)

	MOP_GROUPBEG ("Trail params")
		MOP_POSITIONC ( "Trace begin offset", Vector(0.f,-1.f,0.f), "Смещение начала видимого трейса пули если ноль, то луча не увидим т.к. он будет перпендикулярен нашему взгляду в центре экрана" )
		MOP_FLOAT ("Trail distance", 1000.f)
		//MOP_FLOAT ("Trail life time", 0.4f)
		//MOP_FLOATEX ("Trail minimal speed", 1000.f, 1.f, 10000.f)
		MOP_FLOATEX ("Trail speed", 100.f, 1.f, 10000.f)
		MOP_FLOATEX ("Trail minimal show distance", 2.f, 0.f, 100.f)
		MOP_FLOATEXC ("Trail length min", 1.f, 0.01f, 10.f, "Trail length into minimal distance")
		MOP_FLOATEXC ("Trail length max", 10.f, 0.01f, 1000.f, "Trail length by maximal distance")
		MOP_FLOATEXC ("Trail width min", 0.1f, 0.01f, 10.f, "Trail width into minimal distance")
		MOP_FLOATEXC ("Trail width max", 1.f, 0.01f, 1000.f, "Trail width by maximal distance")
		MOP_STRING ("Ball texture", "")
		MOP_STRING ("Trail texture", "")
	MOP_GROUPEND ()

	MOP_GROUPBEG ("Shooter Effect")
		MOP_SHOOTEREFFECT("")
	MOP_GROUPEND ()

	MOP_STRINGC ("Camera","","Mission Object StaticCamera");

	MOP_BOOL ("Make swing correction", true);
	MOP_BOOL ("Show Colliders", false);

	MOP_BOOL ("Visible", true);
	MOP_BOOL ("Active", false);

	MOP_BOOL ("Show Logic debug", false);

	//MOP_FLOAT ("Temp parameter - trail rotate speed",50.f)

MOP_ENDLIST(Minigun)
