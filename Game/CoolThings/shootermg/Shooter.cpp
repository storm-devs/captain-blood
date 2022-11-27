#include "Shooter.h"
#include "physic.h"
#include "..\..\..\Common_h\ICharactersArbiter.h"
#include "..\..\..\Common_h\bonuses.h"


#define MAX_HEIGHT 0.2f


//Конструктор
Shooter::Shooter() :
	Bullets(_FL_, 16),
	m_WeaponPlatforms(_FL_)
{

	bLogicDebug = false;
	bWantToShoot = false;
	radius = 2.0f;
	damage = 0.0f;
	power = 100.0f;


	bExplodeBullets = false;
	fAccelerateTime = 1.0f;
	fBrakingTime = 10.0f;


	fDeadTime = 1000.0f;
	fTimeMultipler = 1.0f;
	bPointerBuilded = false;
	HorizVelocity = 0.0f;
	VertVelocity = 0.0f;

	HorizForce = 0.0f;
	VertForce = 0.0f;

	Listener = NULL;
	pAnim = NULL;
	pAnimLoader = NULL;
	Weapon = NULL;
	WeaponGlow = NULL;
	Bullet = NULL;
	Loader = NULL;

	Listener = NULL;
	BulletAnimHandler = NULL;

	showColliders = false;

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

	fTraceMinFadeoutTime = 0.05f;
	fTraceMaxFadeoutTime = 0.5f;
	fTraceOffsetStrength = 1.0f;

	fTraceWindVel = 0.0f;
	vTraceWindDir = Vector(0.0f,1.0f,0.0f);

	for (int i=0;i<4;i++) targets[i] = null;

	wasActivated = 1;

	iNumSegments = 0;
	iCurSegment = 0;

	iNumHits = 0;
	iCurHit = 0;

	bDebugDraw = true;

	dr_id[0] = 0;

	zoomOn = false;
	fovChgCurTime = -1.0f;

	was_activated = false;
	init_active_state = true;

	m_fMaxReloadTimer = 5.f;
	m_fCurReloadTimer = 0.f;
	m_fGlowPow = 1.f;
	m_colMinGlowColor = Color((dword)0x01000000);
	m_colMaxGlowColor = Color((dword)0x01FFFFFF);

	pBonusGreedy = null;

	m_targtrack.pvbTargTrack = NULL;
	m_targtrack.ptexTargTrack = NULL;
	m_targtrack.ptexTargAim = NULL;
	m_targtrack.varTargTrack = NULL;
	m_targtrack.shidTargTrack = NULL;
	m_targtrack.pAimModel = NULL;
	m_targtrack.fAimModelScaleMin = 0.7f;
	m_targtrack.fAimModelScaleMax = 1.5f;
	m_targtrack.fAimModelScaleSpeed = 1.f;
	m_targtrack.fAimModelScaleCurTime = 0.f;
	m_targtrack.fAimTargetSize = 10.f;
	m_targtrack.fAimTrackWidth = 0.1f;
	m_targtrack.fAimTrackWidthTimeK = 0.1f;
	m_targtrack.fAimTrackTimeStep = 0.02f;
	m_targtrack.fUVSpeed = 1.f;
	m_targtrack.fAimTrackUV = 0.f;
	m_targtrack.fFadeTime = 0.f;
	m_targtrack.fFadeMaxTime = 1.f;
	m_targtrack.bShow = false;

	m_targtrack.nTrackVertexMax = 0;

	m_vAimTargetPos = 0.f;
	m_vTargetPos = 0.f;
}

//Деструктор
Shooter::~Shooter()
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
bool Shooter::Create(MOPReader & reader)
{
/*
	// коррекция аспекта экрана для камеры
	m_fWideScreenAspectDividerConst = Render().GetWideScreenAspectFovMultipler();

	if( m_fWideScreenAspectDividerConst < 0.01f )
		m_fWideScreenAspectDividerConst = 1.f;
	else
		m_fWideScreenAspectDividerConst = 1.f / m_fWideScreenAspectDividerConst;
	m_fWideScreenAspectDividerConst *= (180.f/PI);
*/

	bPointerBuilded = false;

	if (Listener)
	{
		delete Listener;
		Listener = NULL;
	}

	if (BulletAnimHandler)
	{
		delete BulletAnimHandler;
		BulletAnimHandler = NULL;
	}

	Listener = NEW ShooterAnimEventsHandler(this);
	Listener->Init(&Sound(), &Particles(), &Mission());
	BulletAnimHandler = NEW BulletAnimEventsHandler(this, &Mission());
	BulletAnimHandler->Init(&Sound(), &Particles(), &Mission());
	EditMode_Update (reader);

	codeWeapon_Fire = Mission().Controls().FindControlByName("Weapon_Fire");
	codeWeapon_Exit = Mission().Controls().FindControlByName("Weapon_Exit");

	codeWeapon_Turn_Left = Mission().Controls().FindControlByName("Weapon_Turn_Left");
	codeWeapon_Turn_Right = Mission().Controls().FindControlByName("Weapon_Turn_Right");
	codeWeapon_Turn_Up = Mission().Controls().FindControlByName("Weapon_Turn_Up");
	codeWeapon_Turn_Down = Mission().Controls().FindControlByName("Weapon_Turn_Down");

	codeWeapon_TurnMouse_H = Mission().Controls().FindControlByName("Weapon_TurnMouse_H");
	codeWeapon_TurnMouse_V = Mission().Controls().FindControlByName("Weapon_TurnMouse_V");

	static const ConstString objectId("TrailManager");
	Mission().CreateObject(tm.GetSPObject(), "TrailManager",objectId);

	m_targtrack.varTargTrack = Render().GetTechniqueGlobalVariable( "tMinigunTexture", _FL_ );

	if (Weapon)
	{
		Listener->SetScene(Weapon, Weapon->GetTransform());
	}

	if (Bullet)
	{
		BulletAnimHandler->SetScene(Bullet, Bullet->GetTransform());
	}

	#ifndef NO_CONSOLE

	if (!EditMode_IsOn())
	{
		Console().RegisterCommand("ShooterDebug", "show debug collision", this, (CONSOLE_COMMAND)&Shooter::Console_EnableDebug);
	}

	#endif

	//Render().GetShaderId("Minigun_Trail", m_targtrack.shidTargTrack);
	Render().GetShaderId("Minigun_Bullet", m_targtrack.shidTargTrack);

	MissionObject::Activate(false);
	return true;
}

void Shooter::PostCreate()
{
	if( m_pcReloadingGUI.NotEmpty() )
		MissionObject::FindObject(m_pcReloadingGUI,m_ReloadingGUI);

	for( dword n=0; n<m_WeaponPlatforms.Size(); n++ )
		MissionObject::FindObject(m_WeaponPlatforms[n].pcPlatformID, m_WeaponPlatforms[n].sptr);

	if( m_pTiedCameraID.NotEmpty() )
		MissionObject::FindObject(m_pTiedCameraID,m_pTiedCamera);
	else
		m_pTiedCamera.Reset();

	static const ConstString strShooterKickOut = ConstString("ShooterKickOut");
	if (!FindObject(strShooterKickOut, m_ShooterKickOut.GetSPObject()))
		api->Error("ERROR: Shooter: Can't find KickOut object ShooterKickOut");

	Activate (init_active_state);
}

void Shooter::ClampAngles ()
{
	if (vViewAng.x < HeightLimitMinRad)
	{
		VertVelocity = 0.0f;
		vViewAng.x = HeightLimitMinRad;
	}
	if (vViewAng.x > HeightLimitMaxRad)
	{
		VertVelocity = 0.0f;
		vViewAng.x = HeightLimitMaxRad;
	}

	if (vViewAng.y < RotateLimitMinRad)
	{
		HorizVelocity = 0.0f;
		vViewAng.y = RotateLimitMinRad;
	}
	if (vViewAng.y > RotateLimitMaxRad)
	{
		HorizVelocity = 0.0f;
		vViewAng.y = RotateLimitMaxRad;
	}

	vViewAng.z = 0.0f;
}

//Обновление параметров
bool Shooter::EditMode_Update(MOPReader & reader)
{
	RELEASE( m_targtrack.pvbTargTrack );
	RELEASE( m_targtrack.ptexTargTrack );
	RELEASE( m_targtrack.ptexTargAim );
	RELEASE( m_targtrack.pAimModel );
	m_targtrack.nTrackVertexMax = 0;

	RELEASE(Weapon);
	RELEASE(WeaponGlow);
	RELEASE(Bullet);
	RELEASE(Loader);

	if (pAnim)
	{
		blender.UnregistryBlendStage();
		blender.SetAnimation(NULL);

		if (Listener)
			Listener->DelAnimation(pAnim);

		pAnim->Release();
		pAnim = NULL;
	}


	RELEASE(pAnimLoader);

	Weapon = Geometry().CreateGMX(reader.String().c_str(), &Animation(), &Mission().Particles(), &Mission().Sound());
	WeaponGlow = Geometry().CreateGMX(reader.String().c_str(), &Animation(), &Mission().Particles(), &Mission().Sound());

	bool WeaponDL = reader.Bool();
	bool ShadowReceive = reader.Bool();
	if (Weapon)
	{
		Weapon->SetDynamicLightState(WeaponDL);
		Weapon->SetShadowReceiveState(ShadowReceive);
	}

	if( WeaponGlow )
	{
		WeaponGlow->SetDynamicLightState(false);
		WeaponGlow->SetShadowReceiveState(false);
	}

	const char* weaponAnim = reader.String().c_str();

	Bullet = Geometry().CreateGMX(reader.String().c_str(), &Animation(), &Mission().Particles(), &Mission().Sound());
	bool BulletDL = reader.Bool();
	if (Bullet)
		Bullet->SetDynamicLightState(BulletDL);

	const char* bulletAnim = reader.String().c_str();

	Loader = Geometry().CreateGMX(reader.String().c_str(), &Animation(), &Mission().Particles(), &Mission().Sound());
	bool LoaderDL = reader.Bool();
	if (Loader)
		Loader->SetDynamicLightState(LoaderDL);


	const char* loaderAnim = reader.String().c_str();


	if (Weapon)
	{
		Weapon->SetAnimationFile(weaponAnim);
	}
	if (Bullet)
	{
		Bullet->SetAnimationFile(bulletAnim);
	}
	if (Loader)
	{
		Loader->SetAnimationFile(loaderAnim);
	}

	if (Weapon)
	{
		pAnim = Weapon->GetAnimation();

		if (!pAnim) api->Trace("Shooter %s: can't load anx - %s",GetObjectID().c_str(),weaponAnim);
	}

	if (pAnimLoader) pAnimLoader->Release();
	pAnimLoader = NULL;
	if (Loader) pAnimLoader = Loader->GetAnimation();


	if (pAnim)
	{
		Listener->SetAnimation(pAnim);
		blender.Init(pAnim);
	}


	if (Bullet)
	{
		if (Bullet->GetAnimation())
		{
			BulletAnimHandler->SetAnimation(Bullet->GetAnimation());
			BulletAnimHandler->SetScene(Bullet, Bullet->GetTransform());
		}
	}

	if( Weapon )
	{
		CameraLocator = Weapon->FindEntity(GMXET_LOCATOR, "camera", GMXFINDMETHOD_FULL);
		BulletLocator = Weapon->FindEntity(GMXET_LOCATOR, "bullet", GMXFINDMETHOD_FULL);
		ShootLocator = Weapon->FindEntity(GMXET_LOCATOR, "shoot", GMXFINDMETHOD_FULL);
	}
	else
	{
		CameraLocator.reset();
		BulletLocator.reset();
		ShootLocator.reset();
	}
	if( Bullet )
		BulletCameraLocator = Bullet->FindEntity(GMXET_LOCATOR, "camera", GMXFINDMETHOD_FULL);
	else
		BulletCameraLocator.reset();


	vPos = reader.Position();
	vStartViewAng = vViewAng = reader.Angles();

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

	// Адаптируем угол  пушки под ограничения по углу поворота
	while( vViewAng.y < RotateLimitMinRad )
		vViewAng.y += PIm2;
	while( vViewAng.y > RotateLimitMaxRad )
		vViewAng.y -= PIm2;
	vStartViewAng = vViewAng;

	Matrix m;
	m.SetIdentity();
	m.RotateX((HeightLimitMaxRad + HeightLimitMinRad) * 0.5f);
	m.RotateY((RotateLimitMaxRad + RotateLimitMinRad) * 0.5f);
	vViewCenter = m.vz;

	fBulletStartVelocity = reader.Float();
	fGravity = reader.Float();
	fBulletMaxLifeTime = reader.Float();

	m_pTiedCameraID = reader.String();
	m_pTiedCamera.Reset();

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


	bTraceEnbled = reader.Bool();
	fTraceInitSize = reader.Float();
	fTraceFinalSize = reader.Float();
	fTraceMinFadeoutTime = reader.Float();
	fTraceMaxFadeoutTime = reader.Float();
	fTraceOffsetStrength = reader.Float();
	cTraceColor = reader.Colors();

	fTraceWindVel = reader.Float();
	vTraceWindDir = Matrix(reader.Angles()).vy;

	// preview for trail to target
	IBaseTexture* pOldTex = m_targtrack.ptexTargTrack;
	const char* pcTex = reader.String().c_str();
	m_targtrack.ptexTargTrack = string::NotEmpty(pcTex) ? Render().CreateTexture( _FL_, pcTex ) : NULL;
	RELEASE( pOldTex );
	pOldTex = m_targtrack.ptexTargAim;
	pcTex = reader.String().c_str();
	m_targtrack.ptexTargAim = string::NotEmpty(pcTex) ? Render().CreateTexture( _FL_, pcTex ) : NULL;
	RELEASE( pOldTex );
	m_targtrack.fAimTargetSize = reader.Float();
	m_targtrack.fAimTrackWidth = reader.Float();
	m_targtrack.fAimTrackWidthTimeK = reader.Float();
	m_targtrack.fAimTrackTimeStep = reader.Float();
	m_targtrack.fUVSpeed = reader.Float();
	m_targtrack.fAimTrackUV = 0.f;
	m_targtrack.colAimTrack = reader.Colors();
	m_targtrack.colAimTrackHit = reader.Colors();

	// грузим модельку для прицела
	const char* pcAimModelName = reader.String().c_str();
	IGMXScene* pOldAimModel = m_targtrack.pAimModel;
	m_targtrack.pAimModel = Geometry().CreateScene( pcAimModelName, &Animation(), &Particles(), &Sound(), _FL_ );
	RELEASE( pOldAimModel );
	if( m_targtrack.pAimModel ) {
		m_targtrack.pAimModel->SetDynamicLightState(false);
		m_targtrack.pAimModel->SetShadowReceiveState(false);
	}
	// ставим анимацию для модельки прицела
	const char* pcAimAnim = reader.String().c_str();
	if( m_targtrack.pAimModel && pcAimAnim )
		m_targtrack.pAimModel->SetAnimationFile( pcAimAnim );
	// цвет модельки
	m_targtrack.colAimModel = reader.Colors();
	// параметры изменения масштаба прицела
	m_targtrack.fAimModelScaleMin = reader.Float();
	m_targtrack.fAimModelScaleMax = reader.Float();
	m_targtrack.fAimModelScaleSpeed = reader.Float();
	m_targtrack.fAimModelScaleCurTime = 0.f;


	fTimeMultipler = reader.Float();

	WaterSplashParticles = reader.String();
	WaterSplashSound = reader.String();

	ShootParticles = reader.String();
	ShootSound = reader.String();

	ReloadSound = reader.String();

	triger_onActivate.Init(reader);
	triger_onDeactivate.Init(reader);

	triger_onZoomIn.Init(reader);
	triger_onZoomOut.Init(reader);

	m_triggerReloadStart.Init(reader);
	m_triggerReloadEnd.Init(reader);

	m_fMaxReloadTimer = reader.Float();

	m_pcReloadingGUI = reader.String();

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

	char_particle = reader.String();
	char_sound = reader.String();

	ragdoll_particle = reader.String();
	ragdoll_sound = reader.String();

	bCameraAttachedToBullet = reader.Bool();
	bWatchToBullet = reader.Bool();

	fWaitAfterDead = reader.Float();
	fMaxCamDistance = reader.Float();
	fCameraFriction = reader.Float() / 100.0f;
	fCameraHitFriction = reader.Float() / 100.0f;


	bDontReloadWhileActiveBullets = reader.Bool();

	bShadowCast = reader.Bool();

	bExplodeBullets = reader.Bool();

	radius = reader.Float();
	damage = reader.Float();
	power = reader.Float();


	showColliders = reader.Bool();

	bDebugDraw = reader.Bool();

	iNumSegments = 0;
	iCurSegment = 0;

	iNumHits = 0;
	iCurHit = 0;

	Show(reader.Bool());

	init_active_state = reader.Bool();

	bLogicDebug = reader.Bool();

	m_fGlowPow = reader.Float();
	m_colMinGlowColor = reader.Colors();
	m_colMaxGlowColor = reader.Colors();

	// зачитаем массив оружейных платформ для синхронной стрельбы вместе с нами
	dword q = reader.Array();
	m_WeaponPlatforms.DelAll();
	if( q>0 )
	{
		m_WeaponPlatforms.AddElements(q);
		for( dword n=0; n<q; n++ )
		{
			m_WeaponPlatforms[n].sptr.Reset();
			m_WeaponPlatforms[n].pcPlatformID = reader.String();
		}
	}

	return true;
}


Matrix & Shooter::GetMatrix(Matrix & mtx)
{
	if( m_pTiedCamera.Validate() )
	{
		if (Weapon && CameraLocator.isValid())
		{
			Weapon->GetNodeWorldTransform(CameraLocator,mtx);
			mtx.Normalize();
			return mtx;
		}
	}
	mtx.Build(vViewAng, vPos);
	return mtx;
}

void Shooter::DrawLimiters()
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
}

void _cdecl Shooter::RealizeControl(float fDeltaTime, long level)
{
	UpdateColliders();

	if(Mission().Controls().GetControlStateType("Weapon_Fire") == CST_ACTIVE)
	{
		Fire();
	}

	if(Mission().Controls().GetControlStateType("Weapon_Exit") == CST_INACTIVATED)
	{
		wasActivated--;

		if (wasActivated <= 0)
		{
			Activate(false);
		}
	}

	if(Mission().Controls().GetControlStateType("Weapon_ZoomOn") == CST_ACTIVATED)
	{
		ZoomEnable(!zoomOn);

		if (zoomOn)
		{
			triger_onZoomIn.Activate(Mission(), false);
			api->Trace("Shooter %s: ZoomIn triger was activated", GetObjectID().c_str());
		}
		else
		{
			triger_onZoomOut.Activate(Mission(), false);
			api->Trace("Shooter %s: ZoomOut triger was activated", GetObjectID().c_str());
		}
	}

	bool bNeedSetupCamera = true;
	if (bCameraAttachedToBullet && Bullets.Size() > 0) bNeedSetupCamera = false;

	if (CameraLocator.isValid())
	{
		if (!bNeedSetupCamera) return;
		if (bCameraAttachedToBullet && fDeadTime < fWaitAfterDead) return;
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
		vViewAng.y += fDeltaTime * fRotSpeedH;

		fCurRotSpeed = turn_v * fMaxRotSpeed;
		ChangeValue(fRotSpeedV, fCurRotSpeed, fDeltaTime, fRotAccelerate);
		vViewAng.x -= fDeltaTime * fRotSpeedV;
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
		vViewAng.y += fDeltaTime * fRotSpeedH;

		//Render().Print(0, 0, 0xFFFFFFFF, "fH %f", k);
		//Render().Print(0, 16, 0xFFFFFFFF, "Horiz vel %f, max %f", fRotSpeedH, fCurRotSpeed);

		fCurRotSpeed = SIGN(fMouseFilteredV) * Clampf( fabs(fMouseFilteredV), 0,10)/10.0f * fMaxRotSpeed;
		ChangeValue(fRotSpeedV, fCurRotSpeed, fDeltaTime, fRotAccelerate);
		vViewAng.x -= fDeltaTime * fRotSpeedV;

		//Render().Print(0, 32, 0xFFFFFFFF, "fV %f", k);
		//Render().Print(0, 50, 0xFFFFFFFF, "Vert vel %f, max %f", fRotSpeedV, fCurRotSpeed);
	}

	ClampAngles();
}

void _cdecl Shooter::RealizeDraw(float fDeltaTime, long level)
{
	if(!EditMode_IsVisible()) return;

	SetWeaponTransform ();

	if (EditMode_IsOn()) DrawLimiters();

	if (IsActive()==true)
	{
		if (Weapon)
		{
			if (Loader)
			{
				Loader->SetTransform(Weapon->GetTransform());
				Loader->Draw();
			}
		}
	}

	if (Weapon)
	{
		if (Bullet)
		{
			Listener->Update(Weapon->GetTransform(),fDeltaTime);
		}
	}

	if (BulletAnimHandler)
	{
		if (Bullet)
		{
			BulletAnimHandler->Update(Bullet->GetTransform(),fDeltaTime);
		}
	}

	// обновим модельку с анимацией перегрева ствола
	if( m_fCurReloadTimer > 0.f )
	{
		m_fCurReloadTimer -= fDeltaTime;
		if( m_fCurReloadTimer <= 0.f )
		{
			m_fCurReloadTimer = 0.f;
			m_triggerReloadEnd.Activate(Mission(), false);
			if( pAnim )
				pAnim->Start();
		}
		if( WeaponGlow )
		{
			Color clr;
			clr.Lerp( m_colMinGlowColor, m_colMaxGlowColor, powf(m_fCurReloadTimer/m_fMaxReloadTimer,m_fGlowPow) );
			WeaponGlow->SetUserColor( clr );
		}

		FadePreviewTrail(true);
	}
	else
		FadePreviewTrail(false);

	DrawModel();
	DrawBullets(fDeltaTime);

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

void _cdecl Shooter::RealizeDebugDraw(float fDeltaTime, long level)
{
	for (int i=0;i<iNumSegments;i++)
	{
		if (segments[i].intrsection)
		{
			Render().DrawBufferedLine(segments[i].beg,0xffffff00,segments[i].end,0xffff0000);
		}
		else
		{
			Render().DrawBufferedLine(segments[i].beg,0xff0000ff,segments[i].end,0xff00ff00);
		}
		Render().FlushBufferedLines();
	}

	for (int i=0;i<iNumHits;i++)
	{
		Render().DrawSphere(hitPoints[i].pos,0.25f,hitPoints[i].color);
		Render().Print(hitPoints[i].pos + Vector(0,0.25f,0.0f),15.0f,0,0xffffffff,hitPoints[i].id);
	}
}

void Shooter::UpdateColliderPositions ()
{
	for (dword i = 0; i < 4; i++)
	{
		if (targets[i])
		{
			targets[i]->UpdateColliderPositions (false);
		}
	}
}

int Shooter::CheckCollision (const Vector& from, const Vector& to, float damage)
{
	float fFarDist = 1.9f;
	int idx = -1;

	dr_id[0] = 0;

	for (dword i = 0; i < 4; i++)
	{
		if (targets[i])
		{
			targets[i]->UpdateColliderPositions (true);
		}
	}

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
				if( !mo.IsActive() )
					continue;
				if( mo.IsDead() )
					continue;
				if (mo.Attack(this, DamageReceiver::ds_shooter, damage, from, to))
				{
					crt_strcpy (dr_id,sizeof(dr_id),mo.GetObjectID().c_str());
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

bool Shooter::TestCollision (const Vector& from, const Vector& to)
{
/*	// проверка попадания в 
	IPhysBase* result = Physics().Raycast(from, to, phys_mask(COLLIDERS_GROUP), &HitDetail);

	if (HitDetail.mtl == pmtlid_air) result = NULL;
	if( result )
	{
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
	}*/

	// проверка попадания в дамаг объект
	Vector v_min = from;
	v_min.Min(to);
	Vector v_max = from;
	v_max.Max(to);

	//Пройдёмся по демедж-объектам
	dword count = QTFindObjects(MG_DAMAGEACCEPTOR, v_min, v_max);
	for(dword j = 0; j < count; j++)
	{
		DamageReceiver & mo = (DamageReceiver &)QTGetObject(j)->GetMissionObject();
		// неактивные объекты неучитываем
		if( !mo.IsActive() )
			continue;
		// мертвые тоже не считаем
		if( mo.IsDead() )
			continue;

		// проверяем попадание
		if (mo.Attack(this, DamageReceiver::ds_check, damage, from, to))
			return true;
	}

	return false;
}

void Shooter::SetWeaponTransform ()
{
	if (!Weapon) return;

	// вертикальный угол (задается
	blender.SetBarrelAngle( vViewAng.x );

	//
	Matrix mtx(false);
	mtx = Matrix( 0.f, vViewAng.y, vViewAng.z );

	mtx.pos = vPos;
	Weapon->SetTransform(mtx);

	if( WeaponGlow && BulletLocator.isValid() )
	{
		Matrix m;
		Weapon->GetNodeWorldTransform(BulletLocator,m);
		WeaponGlow->SetTransform( m );
	}
}

void Shooter::DrawModel()
{
	if (!Weapon) return;
	Weapon->Draw();
	if( WeaponGlow && m_fCurReloadTimer > 0.f )
		WeaponGlow->Draw();
}

void Shooter::CalcBulletTransform (int BulletIndex)
{
	Vector OldPos = Bullets[BulletIndex].vOldPos;
	Vector NowPos = Bullets[BulletIndex].vPos;
	float fDeltaY = NowPos.y - OldPos.y;
	Vector vRazn = NowPos - OldPos;
	float fDeltaX = vRazn.GetLengthXZ();
	float fAngle = atan2(fDeltaY, fDeltaX);

	Matrix mRotate(Vector(-fAngle, 0.0f, 0.0f));

	Matrix mtx = mRotate * Bullets[BulletIndex].mInitial;

	mtx.pos = Bullets[BulletIndex].vPos;

	Bullets[BulletIndex].mTrans = mtx;
}

void Shooter::DrawBullets (float fDeltaTime)
{
/*	if (GetState() <= WEAPON_READY_TO_FIRE && IsActive()==true)
	{
		if (BulletLocator)
		{
			Matrix mBulletReloading = BulletLocator->GetWorldTransform();

			if (Bullet)
			{
				Bullet->SetTransform(mBulletReloading);
				Bullet->Draw();
			}
		}
	}*/

	if( IsActive() && (!EditMode_IsOn() || EditMode_IsSelect()) )
		DrawPreviewTrail(fDeltaTime);

	for (dword i = 0; i < Bullets.Size(); i++)
	{
		if (Bullet)
		{
			Bullet->SetTransform(Bullets[i].mTrans);
			Bullet->Draw();

			if (Bullets[i].trail) Bullets[i].trail->Update(Bullets[i].mTrans);
		}
	}
}


void Shooter::ProcessBullets (float fDeltaTime)
{
	float DeltaTime = fDeltaTime;
	DeltaTime *= fTimeMultipler;

	for (dword n = 0; n < Bullets.Size(); n++)
	{
		Bullets[n].fLifeTime+=DeltaTime;
		if (Bullets[n].fLifeTime > fBulletMaxLifeTime)
		{
			Bullets.ExtractNoShift(n);
			n--;
			Reload();
			continue;
		}

		Bullets[n].vOldPos = Bullets[n].vPos;
//		Bullets[n].Force = 0.f;
//		AddGravityForce (Bullets[n].Force, fGravity, 1.0f);
		SolvePhysic (Bullets[n].vPos, Bullets[n].Velocity, Bullets[n].Force, 1.0f, DeltaTime);
		//Bullets[n].Force = Vector(0.0f);
		CalcBulletTransform(n);

		float fWaterLevel = IWaterLevel::GetWaterLevel(Mission(), waterLevel);

		if (Bullets[n].vPos.y < fWaterLevel)
		{
			Plane WaterPlane(
				Mission().GetSwingMatrix().vy,
				Render().GetView().GetCamPos() + Mission().GetSwingMatrix().vy*fWaterLevel
			);
			Vector vres = Bullets[n].vPos;
			WaterPlane.Intersection(Bullets[n].vOldPos, Bullets[n].vPos, vres);
			//vres.y = fWaterLevel;

			if (WaterSplashSound.NotEmpty()) Sound().Create3D(WaterSplashSound, vres, _FL_);
			IParticleSystem* pSys = Particles().CreateParticleSystem(WaterSplashParticles.c_str());
			if (pSys)
			{
				pSys->SetTransform(Matrix(Vector(0.0f), vres));
				pSys->AutoDelete(true);
			}

			//В воду попали...
			fDeadTime = 0.0f;
			Bullets.ExtractNoShift(n);
			n--;
			Reload();
			continue;
		}

		int objIndex = CheckCollision(Bullets[n].vOldPos, Bullets[n].vPos,damage);

		if (bDebugDraw)
		{
			segments[iCurSegment].beg = Bullets[n].vOldPos;
			segments[iCurSegment].end = Bullets[n].vPos;

			if (objIndex!=-2 && objIndex!=-1)
			{
				segments[iCurSegment].intrsection = true;

				if (objIndex == -5) // ragdoll
				{
					hitPoints[iCurHit].pos = Bullets[n].vPos;
					hitPoints[iCurHit].color = 0xffff0000;
				}
				else
				if (objIndex == -4) // damage reciver
				{
					hitPoints[iCurHit].pos = Bullets[n].vPos;
					hitPoints[iCurHit].color = 0xff00ff00;
				}
				else // Target from list
				{
					hitPoints[iCurHit].pos = HitDetail.position;
					hitPoints[iCurHit].color = 0xffffffff;
				}

				crt_strcpy(hitPoints[iCurHit].id,sizeof(hitPoints[iCurHit].id),dr_id);

				iCurHit++;

				if (iCurHit>=NUM_MAXHITS)
				{
					iCurHit = 0;
				}

				if (iNumHits<NUM_MAXHITS)
				{
					iNumHits++;
				}
			}
			else
			{
				segments[iCurSegment].intrsection = false;
			}

			iCurSegment++;

			if (iCurSegment>=NUM_MAXSEGMENT)
			{
				iCurSegment = 0;
			}

			if (iNumSegments<NUM_MAXSEGMENT)
			{
				iNumSegments++;
			}
		}


		if (objIndex==-2 || objIndex==-1)
		{
			continue;
		}

		if (Bullets[n].trail)
		{
			Bullets[n].trail->Reset();
			Bullets[n].trail->Show(false);
			Bullets[n].trail->Release();
		}

		if (objIndex==-4 || objIndex==-3)
		{
			Bullets.ExtractNoShift(n);
			n--;
			Reload();

			continue;
		}

		if (bExplodeBullets)
		{
			MOSafePointer arbiter;
			static const ConstString objectId("CharactersArbiter");
			MissionObject::FindObject(objectId, arbiter);
			if(arbiter.Ptr())
			{
				((ICharactersArbiter *)arbiter.Ptr())->Boom(this, DamageReceiver::ds_shooter, HitDetail.position, radius, damage, power);
			}
		}

		ConstString snd;
		ConstString particle;

		if (objIndex==-5)
		{
			snd = ragdoll_sound;
			particle = ragdoll_particle;
		}

		Sound().Create3D(snd, Bullets[n].vPos, _FL_);

		IParticleSystem* pSys = Particles().CreateParticleSystem(particle.c_str());
		if (pSys)
		{
			Matrix mat(true);
			mat.BuildOrient(HitDetail.normal,Vector(0.0f,1.0f,0.0f));
			mat.pos = HitDetail.position;

			pSys->Teleport(mat);
			//pSys->SetTransform(mat, Bullets[n].vPos));
			pSys->AutoDelete(true);
		}

		//В объект попали...
		Bullets.ExtractNoShift(n);
		n--;
		Reload();
	}
}


void Shooter::Fire ()
{
	bWantToShoot = true;
	if (GetState() != WEAPON_READY_TO_FIRE) return;
	if (!BulletLocator.isValid()) return;


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


	Matrix mWorld;
	Weapon->GetNodeWorldTransform(BulletLocator,mWorld);

	BulletParam Bullet;
	Bullet.vPos = mWorld.pos;
	Bullet.vOldPos = mWorld.pos;
	Bullet.Force = 0.f;
	AddGravityForce (Bullet.Force, fGravity, 1.0f);
	Bullet.Velocity = !mWorld.vz * fBulletStartVelocity;
	Bullet.fLifeTime = 0.0f;

//	Matrix mInitAng(Vector(0.0f, vStartAng.y, 0.0f));
	Matrix mtx(Vector(0.0f, vViewAng.y, 0.0f));
	mtx = mtx;// * mInitAng;

	Bullet.mInitial = mtx;

	if (tm.Ptr() && bTraceEnbled)
	{
		Bullet.trail = tm.Ptr()->Add();

		if (Bullet.trail)
		{
			Bullet.trail->SetParams(fTraceInitSize,fTraceFinalSize,
									fTraceMinFadeoutTime, fTraceMaxFadeoutTime,
									cTraceColor,fTraceOffsetStrength,
									fTraceWindVel,
									vTraceWindDir);
			Bullet.trail->Show(true);
			{
				Matrix m(mtx); m.pos = Bullet.vPos;
				Bullet.trail->Update(m);
			}
		}
	}
	else
	{
		Bullet.trail = NULL;
	}

	Bullets.Add(Bullet);

	if (ShootLocator.isValid())
	{
		Matrix mShoot;
		Weapon->GetNodeWorldTransform(ShootLocator,mShoot);

		if (ShootSound.NotEmpty())
		{
			Sound().Create3D(ShootSound, mShoot.pos, _FL_);
			//api->Trace ("shooter shoot sound: '%s', %f,%f,%f", ShootSound, mShoot.pos.x, mShoot.pos.y, mShoot.pos.z);
		}

		IParticleSystem* pSys = Particles().CreateParticleSystem(ShootParticles.c_str());
		if (pSys)
		{
			pSys->SetTransform(mShoot);
			pSys->AttachTo(Weapon,ShootLocator,false);
			pSys->AutoDelete(true);
		}

		// запустим показ геометрии разогрева ствола
		m_fCurReloadTimer = m_fMaxReloadTimer;

		m_triggerReloadStart.Activate(Mission(), false);
	}

	if (pAnim)
	{
		DumpStateToConsole("Activate link ACTION");
		if (pAnim->ActivateLink("action"))
		{
			blender.AddForce(5.0f);
		}
	}

	if (pAnimLoader) pAnimLoader->ActivateLink("action");

	bCameraDeteched = false;


	iNumSegments = 0;
	iCurSegment = 0;

	iNumHits = 0;
	iCurHit = 0;

	// найдем цель
	//MissionObject* pTargetMO = GetTargetMO();
	//const char* pcEnemyId = pTargetMO ? pTargetMO->GetObjectID().c_str() : null;
	// оповестим оружейные платформы о том, что надо стрелять
	for( dword n=0; n<m_WeaponPlatforms.Size(); n++ )
		if( m_WeaponPlatforms[n].sptr.Validate() )
			// передаем цель как объект
/*			if( pcEnemyId )
				m_WeaponPlatforms[n].sptr.Ptr()->Command("fire",1,&pcEnemyId);
			// передаем цель как позицию
			else*/
			{
				// создаем строки с координатами точки попадания
				char param[3][64];
				crt_snprintf(param[0],sizeof(param[0]), "%f", m_vTargetPos.x);
				crt_snprintf(param[1],sizeof(param[1]), "%f", m_vTargetPos.y);
				crt_snprintf(param[2],sizeof(param[2]), "%f", m_vTargetPos.z);
				const char* params[3] = {param[0],param[1],param[2]};
				m_WeaponPlatforms[n].sptr.Ptr()->Command("fireToPos",3,params);
			}
}

// получить объект по которому стреляем
MissionObject* Shooter::GetTargetMO()
{
	// получаем направление стрельбы
	Matrix mtxShoot;
	Weapon->GetNodeWorldTransform(BulletLocator,mtxShoot);
	Vector vdir = mtxShoot.vz;
	vdir.y = 0.f;
	vdir.Normalize();

	// считаем бокс в котором цели будут соответствовать искомому направлению
	Vector v_min = mtxShoot.pos + vdir*15.f - mtxShoot.vx*20.f;
	Vector v_max = mtxShoot.pos + vdir*1000.f + mtxShoot.vx*20.f;
	v_min.y = -10.f;
	v_max.z = 100.f;
	// придаем боксу объем, если по одной из сторон у него слишком маленький размер
/*	if( fabs(vdir.x)<0.01f )
	{
		v_min.x -= 2.f;
		v_max.x = v_min.x + 4.f;
	}
	if( fabs(vdir.z)<0.01f )
	{
		v_min.z -= 2.f;
		v_max.z = v_min.z + 4.f;
	}*/

	//Пройдёмся по демедж-объектам
	dword count = QTFindObjects(MG_DAMAGEACCEPTOR, v_min, v_max);

	float fMinDist = 4000.f * 4000.f;
	dword dwMinDistIdx = count;
	Matrix mtxTarg(true);
	for(dword j = 0; j < count; j++)
	{
		DamageReceiver & mo = (DamageReceiver &)QTGetObject(j)->GetMissionObject();
		mo.GetMatrix( mtxTarg );
/*		Vector vTargDir = mtxTarg.pos - mtxShoot.pos;
		float fTargDist = vTargDir | vdir;
		float fDist2Pow = (~vTargDir) - fTargDist*fTargDist;
		if( fDist2Pow < 0.f )
			continue;*/
		float fDist2Pow = ~(m_vAimTargetPos - mtxTarg.pos);

		if( fDist2Pow < fMinDist )
		{
			dwMinDistIdx = j;
			fMinDist = fDist2Pow;
		}
	}

	// нашли ближайший по дистанции до линии стрельбы корабль
	if( dwMinDistIdx < count )
		return & QTGetObject(dwMinDistIdx)->GetMissionObject();

	return null;
}

void Shooter::Reload ()
{
	// не пустое оружие не перезаряжаем
	if (GetState() != WEAPON_EMPTY) return;
	// перезагрузка должна быть только через определенное время
	if (m_fCurReloadTimer > 0.f ) return;
	if (Bullets.Size() > 0 && bDontReloadWhileActiveBullets) return;

	if (ShootLocator.isValid())
	{
		Matrix mShoot;
		Weapon->GetNodeWorldTransform(ShootLocator,mShoot);
		if (ReloadSound.NotEmpty())
		{
			Sound().Create3D(ReloadSound, mShoot.pos, _FL_);
			//api->Trace ("shooter reload: '%s', %f,%f,%f", ReloadSound, mShoot.pos.x, mShoot.pos.y, mShoot.pos.z);
		}
	}


	if (pAnim)
	{
		DumpStateToConsole("Activate link ACTION");
		bool bCan = pAnim->ActivateLink("action");
		//api->Trace("Shooter anim link action activation %d", bCan);
	} else
	{
		api->Trace("Shooter animation not assigned !!!");
	}

	if (pAnimLoader)
	{
		bool bCan = pAnimLoader->ActivateLink("action");
		//api->Trace("Shooter reloader anim link action activation %d", bCan);
	} else
	{
		api->Trace("Shooter reloader animation not assigned !!!");
	}
}

void Shooter::Release()
{
	RELEASE( m_targtrack.pvbTargTrack );
	RELEASE( m_targtrack.ptexTargTrack );
	RELEASE( m_targtrack.ptexTargAim );
	RELEASE( m_targtrack.pAimModel );

	RELEASE( Weapon );
	RELEASE( WeaponGlow );
	RELEASE( Bullet );
	RELEASE( Loader );

	if (pAnim)
	{
		blender.UnregistryBlendStage();
		blender.SetAnimation(NULL);

		if (Listener)
		{
			Listener->DelAnimation(pAnim);
		}

		pAnim->Release();
		pAnim = NULL;
	}

	RELEASE( pAnimLoader );
	DELETE( Listener );
	DELETE( BulletAnimHandler );

	was_activated = false;

	RELEASE( pBonusGreedy );
}

void _cdecl Shooter::RealizeProcess(float fDeltaTime, long level)
{
	UpdateColliderPositions();
	ProcessBullets(fDeltaTime);

	if (!IsActive())
	{
		if (Bullets.Size() == 0 ) DelUpdate(&Shooter::RealizeProcess);

		return;
	}

	if(!EditMode_IsVisible()) return;

	if (GetState() == WEAPON_EMPTY) Reload();

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
			case WEAPON_EMPTY: shooterState = "Пустой"; break;
			case WEAPON_LOADING: shooterState = "Заряжается"; break;
			case WEAPON_READY_TO_FIRE: shooterState = "Готов к стрельбе"; break;
			case WEAPON_FIRING: shooterState = "Стреляет"; break;
		}

		Render().Print(0, 40, 0xFFFFFFFFL, "Anim node: %s, Shooter state : %s, Хочу стрелять: %d", animNode, shooterState, bWantToShoot);
		Render().Print(0, 60, 0xFFFFFFFFL, "Активных пуль: %d, Не перезаряжать пока активные пули : %d", Bullets.Size(), bDontReloadWhileActiveBullets);

		bWantToShoot = false;
	}

	fDeadTime+=fDeltaTime;

	SetWeaponTransform();

	if (!IsActive()) return;

	bool bNeedSetupCamera = true;
	if (bCameraAttachedToBullet && Bullets.Size() > 0) bNeedSetupCamera = false;

	if ((bCameraAttachedToBullet == false) || (bNeedSetupCamera && CameraLocator.isValid() && fDeadTime > fWaitAfterDead))
	{
		//Обычная камера с орудия...
		if( m_pTiedCamera.Ptr() )
		{
			char par1[24];
			crt_snprintf( par1, sizeof(par1), "%f", (float)(fov  * (180.f/PI) /** m_fWideScreenAspectDividerConst*/) );
			const char* param[2];
			param[0] = par1;
			param[1] = "0";
			m_pTiedCamera.Ptr()->Command( "setFov", 2, param );
		}
		else
		{
			Matrix m;
			if( Weapon )
			{
				Weapon->GetNodeWorldTransform(CameraLocator,m);
				m.Normalize ();
			}
			else
				m.SetIdentity();

			mLastCamAttachedToBulletMatrix = m;
			vCameraVector = Vector(0.0f);
			m.Inverse();

/*
			float wideConst = Render().GetWideScreenAspectFovMultipler();
			float fWidth = (float)Render().GetViewport().Width;
			float fHeight = fWidth * ((3.f/4.f) / wideConst);
			Matrix mProjection;
			mProjection.BuildProjection(fov, fWidth, fHeight, NearPlane, FarPlane);
			Render().SetProjection(mProjection);
*/
			float fWidth = (float)Render().GetViewport().Width;
			float fHeight = (float)Render().GetViewport().Height;
			float fPerspective = recalculateFov(fWidth, fHeight, Render().GetWideScreenAspectWidthMultipler(), fov);
			Matrix mProjection;
			mProjection.BuildProjection(fPerspective, fWidth, fHeight, NearPlane, FarPlane);
			Render().SetProjection(mProjection);



			Render().SetView(m);
		}
	} else
	{
		bool bDistanceMoreThanNeed = false;
		bool bNeedPlaceCam = true;
		if (!BulletCameraLocator.isValid()) bNeedPlaceCam = false;
		if (Bullets.Size() == 0) bNeedPlaceCam = false;

		if (Bullets.Size() > 0)
		{
			if (Vector(Bullets[0].mTrans.pos - vPos).GetLengthXZ() > fMaxCamDistance)
			{
				bDistanceMoreThanNeed = true;
				bNeedPlaceCam = false;

				if (bCameraDeteched == false)
				{
					bCameraDeteched = true;

					qRot = Quaternion(mLastCamAttachedToBulletMatrix);
				}
			}
			if (Bullets[0].mTrans.pos.y < MAX_HEIGHT) bNeedPlaceCam = false;
		}

		if (bNeedPlaceCam)
		{
			Bullet->SetTransform(Bullets[0].mTrans);
			Matrix m;
			Bullet->GetNodeWorldTransform(BulletCameraLocator,m);
			m.Normalize ();
			mLastCamAttachedToBulletMatrix = m;

			m.Inverse();


			vCameraVector = Bullets[0].Velocity;

		} else
		{
			float fFric = fCameraFriction;
			if (Bullets.Size() == 0) fFric = fCameraHitFriction;


			float dt = api->GetDeltaTime();
			mLastCamAttachedToBulletMatrix.pos += (vCameraVector * dt);

			if (mLastCamAttachedToBulletMatrix.pos.y < MAX_HEIGHT)
			{
				mLastCamAttachedToBulletMatrix.pos.y = MAX_HEIGHT;
			}

			float k = dt * fCameraFriction * fFric;
			if (k > 1.0f) k = 1.0f;
			vCameraVector -= (vCameraVector * k);

			if (bDistanceMoreThanNeed && Bullets.Size() > 0 && bWatchToBullet)
			{
				Vector OldPos = mLastCamAttachedToBulletMatrix.pos;

				Matrix mViewTemp;
				mViewTemp.BuildView(OldPos, Bullets[0].vPos, Vector(0.0f, 1.0f, 0.0f));
				mViewTemp.Inverse();

				Quaternion qLoc(mViewTemp);
				Quaternion qNowRot;
				float lrp = dt * 5.0f;
				if (lrp > 1.0f) lrp = 1.0f;
				qNowRot.SLerp(qRot, qLoc, lrp);
				qRot = qNowRot;


				qNowRot.GetMatrix(mLastCamAttachedToBulletMatrix);
				mLastCamAttachedToBulletMatrix.pos = OldPos;
			}
		}


		Matrix mProjection;
		mProjection.BuildProjection(fov, (float)Render().GetViewport().Width, (float)Render().GetViewport().Height, NearPlane, FarPlane);
		Render().SetProjection(mProjection);

		Matrix mView = mLastCamAttachedToBulletMatrix;
		mView.Inverse();
		Render().SetView(mView);
	}

	// обновим интерфейс (прогресс бар зарядки)
	if( m_ReloadingGUI.Validate() )
	{
		char param[32];
		const char* pPar = param;
		if( m_fMaxReloadTimer > 0.f )
			crt_snprintf(param,sizeof(param),"%f",10000.f * Clampf(1.f - m_fCurReloadTimer/m_fMaxReloadTimer));
		else {
			param[0] = '0';
			param[1] = 0;
		}
		m_ReloadingGUI.Ptr()->Command( "Set", 1, &pPar );
	}
}

void Shooter::EditMode_GetSelectBox(Vector & min, Vector & max)
{
	min = Vector(-1.0f);
	max = Vector(1.0f);
}

void Shooter::Activate(bool isActive)
{
	if( IsActive() == isActive )
		return;
	if (isActive)
	{
		was_activated = true;

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
			api->Trace("Shooter: Camera locator not found !");
		}

		if (!BulletLocator.isValid())
		{
			api->Trace("Shooter: Bullet locator not found !");
		}

		if (!ShootLocator.isValid())
		{
			api->Trace("Shooter: Shoot locator not found !");
		}

		if (!Weapon)
		{
			api->Trace("Shooter: Weapon model not found !");
		}

		if (!Bullet)
		{
			api->Trace("Shooter: Bullet model not found !");
		}

		if (!Loader)
		{
			api->Trace("Shooter: Loader model not found !");
		}

		ControlStateType st = Mission().Controls().GetControlStateType("Weapon_Exit");
		if( st == CST_ACTIVATED || st == CST_ACTIVE )
		{
			wasActivated = 2;
		}
		else
		{
			wasActivated = 1;
		}

		if(IsShow())
		{
			SetUpdate(&Shooter::RealizeControl, ML_FIRST);
			//SetUpdate(&Shooter::RealizeProcess, ML_CAMERAMOVE_ANI);
			SetUpdate(&Shooter::RealizeProcess, ML_CAMERAMOVE - 1);
		}
		else
		{
			DelUpdate(&Shooter::RealizeControl);
			DelUpdate(&Shooter::RealizeProcess);
		}

		triger_onActivate.Activate(Mission(), false);
		api->Trace("Shooter %s: Activate triger was activated", GetObjectID().c_str());

		Reload();

		if( !pBonusGreedy && Mission().Player()!=null )
		{
			pBonusGreedy = BonusesManager::CreateGreedy( Mission().Player() );
			if( pBonusGreedy )
				pBonusGreedy->SetPosition( vPos );
		}

		if( m_ReloadingGUI.Validate() )
			m_ReloadingGUI.Ptr()->Show(true);

		if( m_ShooterKickOut.Validate() )
			m_ShooterKickOut.Ptr()->InShooter(this);

		// включаем камеру
		if( m_pTiedCameraID.NotEmpty() && m_pTiedCamera.Validate() )
		{
			const char* param[1];
			param[0] = GetObjectID().c_str();
			m_pTiedCamera.Ptr()->Command("Target",1,param);
			m_pTiedCamera.Ptr()->Activate(true);
		}
	}
	else
	{
		triger_onDeactivate.Activate(Mission(), false);
		api->Trace("Shooter %s: Exit triger was activated", GetObjectID().c_str());

		DelUpdate(&Shooter::RealizeControl);

		RELEASE( pBonusGreedy );

		if( m_ReloadingGUI.Validate() )
			m_ReloadingGUI.Ptr()->Show(false);

		if( m_ShooterKickOut.Validate() )
			m_ShooterKickOut.Ptr()->OutShooter(this);

		vViewAng = vStartViewAng;

		// выключаем камеру
		if( m_pTiedCameraID.NotEmpty() && m_pTiedCamera.Validate() )
			m_pTiedCamera.Ptr()->Activate(false);
	}

	MissionObject::Activate(isActive);
}

void Shooter::DumpStateToConsole (const char* mess)
{
	if (!bLogicDebug) return;
	const char* animNode = pAnim->CurrentNode();
	const char* shooterState = "unknown";

	switch (GetState())
	{
		case WEAPON_EMPTY: shooterState = "Пустой";  break;
		case WEAPON_LOADING: shooterState = "Заряжается";  break;
		case WEAPON_READY_TO_FIRE: shooterState = "Готов к стрельбе";  break;
		case WEAPON_FIRING: shooterState = "Стреляет";  break;
	}

	Console().Trace(COL_ALL, "%s : '%s', status : '%s'", mess, animNode, shooterState);
}

Shooter::WeaponStates Shooter::GetState()
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

void Shooter::BuildPointers ()
{
	if (bPointerBuilded && !EditMode_IsOn()) return;

	for (dword i = 0; i < 4; i++)
	if (targets[i])
	{
		targets[i]->BuildPointers();
	}

	bPointerBuilded = true;
}

void Shooter::GetBox(Vector & min, Vector & max)
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

void _cdecl Shooter::ShadowDraw ()
{
	if(!EditMode_IsVisible()) return;
	if (Weapon)
	{
		Weapon->Draw();
	}
}

void Shooter::Show(bool bShow)
{
	if(bShow)
	{
		SetUpdate(&Shooter::RealizeDraw, ML_ALPHA5+10);

		if (bDebugDraw) SetUpdate(&Shooter::RealizeDebugDraw, ML_DEBUG1);

		if(bShadowCast)
		{
			Registry(MG_SHADOWCAST, (MOF_EVENT)&Shooter::ShadowDraw, ML_GEOMETRY1);
			Registry(MG_SHADOWRECEIVE, (MOF_EVENT)&Shooter::ShadowDraw, ML_GEOMETRY1);
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
		DelUpdate(&Shooter::RealizeDraw);
		DelUpdate(&Shooter::RealizeDebugDraw);

		bool isAct = IsActive();
		//Activate(false);
		//MissionObject::Activate(isAct);
		MissionObject::Show(bShow);
	}
}


void Shooter::UpdateColliders()
{
	for (dword i = 0; i < 4; i++)
	if (targets[i])
	{
		targets[i]->UpdateColliders();
	}
}

void Shooter::ChangeValue(float &value, float targetValue, float dltTime, float accel)
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

void Shooter::ZoomEnable(bool enable)
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
void Shooter::Command(const char * id, dword numParams, const char ** params)
{
	if (string::IsEqual(id,"LocPrt"))
	{
		Listener->LocParticles(pAnim,"LocPrt",params,numParams);
	}
	else
	if (string::IsEqual(id,"DelLocPrt"))
	{
		Listener->LocParticles(pAnim,"DelLocParticles",params,numParams);
	}
}

void Shooter::Restart()
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

	for( dword n=0; n<m_WeaponPlatforms.Size(); n++ )
		MissionObject::FindObject(m_WeaponPlatforms[n].pcPlatformID, m_WeaponPlatforms[n].sptr);

	Activate (init_active_state);
}

bool Shooter::CheckDirection(const Vector& pos)
{
	Matrix mtx(vViewAng, vPos);
	return ((pos - vPos) ^ mtx.vz) > 0.f;
}

void Shooter::DrawPreviewTrail(float fDeltaTime)
{
	// фейдин/фейдоут
	float fKAlpha = 1.f;
	if( m_targtrack.fFadeTime > 0.f )
	{
		// считаем альфу от 0 до 1
		fKAlpha = m_targtrack.fFadeTime / m_targtrack.fFadeMaxTime;
		if( !m_targtrack.bFadeIn ) fKAlpha = 1.f - fKAlpha;
		fKAlpha = Clampf(fKAlpha);
		// время идет
		m_targtrack.fFadeTime -= fDeltaTime;
		if( m_targtrack.fFadeTime <= 0.f && m_targtrack.bFadeIn )
			m_targtrack.bShow = false;
	}

	if( BulletLocator.isValid() && m_targtrack.bShow )
	{
		// получаем параметры трека (начало трека и направление стрельбы)
		//Видовая матрица с включёной свинг матрицей
		Matrix view = Render().GetView();
		//Чистая видовая матрица
		Matrix realView = Mission().GetInverseSwingMatrix()*Matrix(view).Inverse();
		realView.Inverse();
		//Разносная матрица
		Matrix diff = Matrix(Matrix(realView), Matrix(view).Inverse());

		Matrix mtxStart;
		Weapon->GetNodeWorldTransform(BulletLocator,mtxStart);
		Vector vpos = mtxStart.pos;
		Vector velocity = fBulletStartVelocity * !mtxStart.vz;
		Vector offset = (!mtxStart.vx) * m_targtrack.fAimTrackWidth;
		array<Vector> aPos(_FL_);
		aPos.Add(vpos);

		// получаем ключевые точки трейла
		float fWaterLevel = IWaterLevel::GetWaterLevel(Mission(), waterLevel);
		Vector vForce = 0.f;
		AddGravityForce (vForce, fGravity, 1.0f);
		bool bShootTarget = false;
		for(long n=0; n<1000; n++)
		{
			Vector vold = vpos;
			SolvePhysic (vpos, velocity, vForce, 1.0f, m_targtrack.fAimTrackTimeStep);

			bool bIsBreak = false;

			Vector vres;
			Plane WaterPlane( diff.MulNormal(Vector(0.f,1.f,0.f)), diff.MulVertex(Vector(0,fWaterLevel,0)) );
			if( WaterPlane.Intersection(vold,vpos,vres) )
			{
				bIsBreak = true;
				vpos.x = vres.x;
				vpos.y = vres.y + 0.1f;//fWaterLevel + 0.2f;
				vpos.z = vres.z;
				m_vAimTargetPos = vres;
			}
			aPos.Add(vpos);

			// проверяем попадание в цель
			if( !bShootTarget )
			{
				long idx = aPos.Last();
				if( TestCollision (aPos[idx-1], aPos[idx]) )
					bShootTarget = true;
			}

			if( bIsBreak ) break;
		}

		// создаем буфер нужного размера
		long nVertexQuantity = (aPos.Size()-1)*6;
		if( nVertexQuantity > m_targtrack.nTrackVertexMax )
		{
			RELEASE(m_targtrack.pvbTargTrack);
			m_targtrack.nTrackVertexMax = ((nVertexQuantity>>6) + 1) << 6;
			m_targtrack.pvbTargTrack = Render().CreateVertexBuffer( sizeof(TargetTrackVertex)*m_targtrack.nTrackVertexMax,sizeof(TargetTrackVertex), _FL_, USAGE_WRITEONLY | USAGE_DYNAMIC, POOL_DEFAULT );
		}

		Color colCur;
		dword dwCurCol;

		// заполняем буфер
		TargetTrackVertex * vrt = m_targtrack.pvbTargTrack ? (TargetTrackVertex*)m_targtrack.pvbTargTrack->Lock() : 0;
		if(vrt)
		{
			colCur = bShootTarget ? m_targtrack.colAimTrackHit : m_targtrack.colAimTrack;
			colCur.alpha *= fKAlpha;
			dwCurCol = colCur.GetDword();
			for(long n=0; n<aPos-1; n++)
			{
				long i=n*6;
				Vector begoff = offset * (1.f + m_targtrack.fAimTrackWidthTimeK * n * m_targtrack.fAimTrackTimeStep);
				Vector endoff = offset * (1.f + m_targtrack.fAimTrackWidthTimeK * (n+1) * m_targtrack.fAimTrackTimeStep);
				float begV = m_targtrack.fAimTrackUV + 1.f;
				float endV = m_targtrack.fAimTrackUV;
				if( m_targtrack.fAimTrackUV >= 1.f ) m_targtrack.fAimTrackUV -= 1.f;
				vrt[i].pos = aPos[n] - begoff;
				vrt[i].color = dwCurCol;
				vrt[i].u = 0.f;
				vrt[i].v = begV;
				vrt[i+1].pos = aPos[n] + begoff;
				vrt[i+1].color = dwCurCol;
				vrt[i+1].u = 1.f;
				vrt[i+1].v = begV;
				vrt[i+2].pos = aPos[n+1] - endoff;
				vrt[i+2].color = dwCurCol;
				vrt[i+2].u = 0.f;
				vrt[i+2].v = endV;

				vrt[i+3].pos = aPos[n] + begoff;
				vrt[i+3].color = dwCurCol;
				vrt[i+3].u = 1.f;
				vrt[i+3].v = begV;
				vrt[i+4].pos = aPos[n+1] + endoff;
				vrt[i+4].color = dwCurCol;
				vrt[i+4].u = 1.f;
				vrt[i+4].v = endV;
				vrt[i+5].pos = aPos[n+1] - endoff;
				vrt[i+5].color = dwCurCol;
				vrt[i+5].u = 0.f;
				vrt[i+5].v = endV;
			}
			m_targtrack.pvbTargTrack->Unlock();
		}
		m_targtrack.fAimTrackUV += m_targtrack.fUVSpeed * fDeltaTime;

		// отрисовка
		// trail Mission().GetInverseSwingMatrix()
		Render().SetWorld(Matrix());
		if( m_targtrack.ptexTargTrack && m_targtrack.shidTargTrack )
		{
			Render().SetStreamSource(0, m_targtrack.pvbTargTrack, sizeof(TargetTrackVertex));
			m_targtrack.varTargTrack->SetTexture( m_targtrack.ptexTargTrack );
			Render().DrawPrimitive(m_targtrack.shidTargTrack, PT_TRIANGLELIST, 0, (aPos.Size()-1)*2);
		}
		// aim
		if( m_targtrack.ptexTargAim && m_targtrack.shidTargTrack )
		{
			dwCurCol = Color(1.f,fKAlpha).GetDword();
			// готовим вертексы для прицелы
			TargetTrackVertex vtxAim[4];
			vtxAim[0].pos = vpos + diff.MulNormal(Vector(-m_targtrack.fAimTargetSize,0.f,-m_targtrack.fAimTargetSize));
			vtxAim[0].color = dwCurCol;
			vtxAim[0].u = 0.f;
			vtxAim[0].v = 0.f;
			vtxAim[1].pos = vpos + diff.MulNormal(Vector(m_targtrack.fAimTargetSize,0.f,-m_targtrack.fAimTargetSize));
			vtxAim[1].color = dwCurCol;
			vtxAim[1].u = 1.f;
			vtxAim[1].v = 0.f;
			vtxAim[2].pos = vpos + diff.MulNormal(Vector(-m_targtrack.fAimTargetSize, 0.f, m_targtrack.fAimTargetSize));
			vtxAim[2].color = dwCurCol;
			vtxAim[2].u = 0.f;
			vtxAim[2].v = 1.f;
			vtxAim[3].pos = vpos + diff.MulNormal(Vector(m_targtrack.fAimTargetSize, 0.f, m_targtrack.fAimTargetSize));
			vtxAim[3].color = dwCurCol;
			vtxAim[3].u = 1.f;
			vtxAim[3].v = 1.f;
			// рисуем прицел
			m_targtrack.varTargTrack->SetTexture( m_targtrack.ptexTargAim );
			Render().DrawPrimitiveUP(m_targtrack.shidTargTrack, PT_TRIANGLESTRIP, 2, vtxAim, sizeof(TargetTrackVertex));
		}
		if( m_targtrack.pAimModel )
		{
			Matrix aimTransf(true);
			aimTransf.BuildPosition(vpos);
			float fCurScale = m_targtrack.fAimModelScaleMin + (0.5f*sinf(m_targtrack.fAimModelScaleCurTime)+0.5f)*(m_targtrack.fAimModelScaleMax-m_targtrack.fAimModelScaleMin);
			m_targtrack.fAimModelScaleCurTime += api->GetDeltaTime() * m_targtrack.fAimModelScaleSpeed;
			if( m_targtrack.fAimModelScaleCurTime > PIm2 )
				m_targtrack.fAimModelScaleCurTime -= PIm2;
			aimTransf.vx *= fCurScale;
			aimTransf.vy *= fCurScale;
			aimTransf.vz *= fCurScale;
			m_targtrack.pAimModel->SetTransform( aimTransf );
			colCur = m_targtrack.colAimModel;
			colCur.alpha *= fKAlpha;
			m_targtrack.pAimModel->SetUserColor( colCur );
			m_targtrack.pAimModel->Draw();
		}

		// запомним точку куда надо стрелять
		m_vTargetPos = vpos;
	}
}

void Shooter::FadePreviewTrail(bool bFadeIn)
{
	// мы и так не показываемся, так что - отбой
	if( bFadeIn && !m_targtrack.bShow ) return;

	// мы и так нормально показываемся, так что отбой
	if( !bFadeIn && m_targtrack.bShow ) return;

	// уже в процессе, так что не зачем запускать снова
	if( m_targtrack.fFadeTime>0.f && m_targtrack.bFadeIn==bFadeIn ) return;

	m_targtrack.bShow = true;
	m_targtrack.fFadeTime = m_targtrack.fFadeMaxTime;
	m_targtrack.bFadeIn = bFadeIn;
}



#ifndef NO_CONSOLE

void _cdecl Shooter::Console_EnableDebug(const ConsoleStack & params)
{
	bDebugDraw = !bDebugDraw;

	if (bDebugDraw) SetUpdate(&Shooter::RealizeDebugDraw, ML_DEBUG1);
	else DelUpdate(&Shooter::RealizeDebugDraw);

}

#endif

MOP_BEGINLIST(Shooter, "Shooter", '1.00', 100)

	MOP_STRINGC("WeaponModel", "", "Моделька орудия\nGMX файл");
	MOP_STRINGC("WeaponGlowModel", "", "Моделька орудия при перегреве ствола\nGMX файл");
	MOP_BOOL("WeaponModel Dynamic light", true);
	MOP_BOOL("WeaponModel shadow receive", true);
	MOP_STRINGC("WeaponAnimation", "m2crossbow", "Анимация орудия");

	MOP_STRINGC("BulletModel", "", "Моделька патрона, которе из этого орудия вылетает\nGMX файл");
	MOP_BOOL("BulletModel Dynamic light", true);
	MOP_STRINGC("BulletAnimation", "", "Анимация патрона");


	MOP_STRINGC("LoaderModel", "", "Моделька заряжающего\nВидна только в режиме игры\nGMX файл");
	MOP_BOOL("LoaderModel Dynamic light", true);
	MOP_STRINGC("LoaderAnimation", "m2hands", "Анимация рук, заряжающий шутер");


	MOP_POSITIONC("Position", Vector(0.0f), "Позиция");
	MOP_ANGLESEXC("Angles", Vector(0.0f, 0.0f, Deg2Rad(0.0f)), Vector(0.0f, 0.0f, 0.0f), Vector(2*PI, 2*PI, 0.0f), "Угол вращения орудия");
	//MOP_ANGLESC("Angles", Vector(0.0f, 0.0f, Deg2Rad(0.0f)), "Угол вращения орудия");
	
	MOP_FLOATEXC("HeightLimitMin", -60.0f, -89.0f, 89.0f, "Верхний предел");
	MOP_FLOATEXC("HeightLimitMax", 60.0f, -89.0f, 89.0f, "Нижний предел");
	MOP_FLOATC("RotateLimitMin", -60.0f, "Левый предел");
	MOP_FLOATC("RotateLimitMax", 60.0f, "Правый предел");
	MOP_FLOATC("BulletStartVelocity", 100.0f, "Начальная скорость патрона м/сек");
	MOP_FLOATC("Gravity", -9.8f, "Гравитация м/сек");
	MOP_FLOATC("MaxBulletLifeTime", 20.0f, "Время жизни патрона в секундах. Если она не куда не попала, умрет через это время");
	
	MOP_STRING("TieCamera","");
	MOP_FLOATEXC("near FOV", 1.25f*180.0f/PI, 10.0f, 140.0f, "FOV камеры установленной на орудии и на патроне в режи зуминга");
	MOP_FLOATEXC("far FOV", 1.25f*180.0f/PI, 10.0f, 140.0f, "FOV камеры установленной на орудии и на патроне");	
	
	MOP_FLOATEXC("zoom Time", 1, 0.01f, 40.0f, "скорость смены фова камеры");
	MOP_BOOLC("Zoom enabled", false,"Зоом включен при старте шутера");


	MOP_FLOATEXC("NearPlane", 0.1f, 0.0f, 10.0f, "Ближняя плоскость клипирования камеры");
	MOP_FLOATEXC("FarPlane", 4000.0f, 10.0f, 8000.0f, "Дальняя плоскость клипирования камеры");

	MOP_FLOATEX("MaxJoyRotSpeed", 1.5f, 0.05f, 10.0f)
	MOP_FLOATEX("RotJoyAccelerate", 4.5f, 0.05f, 50.0f)
	MOP_FLOATEX("MaxMsRotSpeed", 4.5f, 0.05f, 20.0f)
	MOP_FLOATEX("RotMsAccelerate", 15.5f, 0.05f, 50.0f)

	MOP_FLOATEX("MaxJoyRotSpeed zoomed", 0.5f, 0.05f, 10.0f)
	MOP_FLOATEX("RotJoyAccelerate zoomed", 2.5f, 0.05f, 50.0f)
	MOP_FLOATEX("MaxMsRotSpeed zoomed", 2.0f, 0.05f, 20.0f)
	MOP_FLOATEX("RotMsAccelerate zoomed", 7.5f, 0.05f, 50.0f)



	MOP_BOOL("Trace Enabled",true)
	MOP_FLOATEX("Trace init size", 0.05f, 0.01f, 20.0f)
	MOP_FLOATEX("Trace final size", 0.2f, 0.01f, 20.0f)
	MOP_FLOATEX("Trace min fadeout time", 0.1f, 0.01f, 20.0f)
	MOP_FLOATEX("Trace max fadeout time", 0.1f, 0.01f, 20.0f)
	MOP_FLOATEX("Trace offset strenght", 1.0f, 0.1f, 5.0f)
	MOP_COLORC("Trace color", 0x88ffffff, "Trace color")

	MOP_FLOATEX("Trace wind strenght", 0.0f, 0.0f, 5.0f)
	MOP_ANGLES("Trace wind direction", 0.0f)


	MOP_GROUPBEG("PreviewTrail")
	MOP_STRING("PreviewTrailTexture","");
	MOP_STRING("PreviewAimTexture","");
	MOP_FLOATEXC("PreviewAimSize", 10.f, 0.5f,500.f, "Размер прицела на воде")
	MOP_FLOATEXC("PreviewTrailWidth", 0.1f, 0.01f,20.f, "Ширина трейла на старте")
	MOP_FLOATEXC("PreviewTrailTimeWidthMul", 0.1f, 0.01f,10.f, "Коэффициент увеличения ширины трейла со временем")
	MOP_FLOATEXC("PreviewTrailTimeStep", 0.1f, 0.01f,1.f, "Временной промежуток (сек) между ключевыми кадрами трейла")
	MOP_FLOATEXC("PreviewTrailUVSpeed", 1.f, 0.1f, 50.f, "Скорость движения трейла")
	MOP_COLORC("PreviewTrailColorMiss", Color(1.f,1.f), "Цвет трейла")
	MOP_COLORC("PreviewTrailColorHit", Color(1.f,0.f,0.f,1.f), "Цвет трейла при попадании в цель")
	MOP_STRINGC("AimModel", "", "Моделька для прицела");
	MOP_STRINGC("AimAnimation", "", "Анимация прицела");
	MOP_COLORC("AimModelColor", Color(1.f,1.f), "Цвет для модельки прицела");
	MOP_FLOATEXC("AimScaleMin", 0.7f, 0.1f, 5.f, "Минимальный масштаб для модельки прицела");
	MOP_FLOATEXC("AimScaleMax", 1.5f, 0.1f, 5.f, "Максимальный масштаб для модельки прицела");
	MOP_FLOATEXC("AimScaleSpeed", 1.f, 0.1f, 10.f, "Скорость изменения масштаба модельки прицела");
	MOP_GROUPEND()


	MOP_FLOATC("Time multipler", 1.0f, "Множитель времени, если нужно визуально замедлить скорость снаряда\nНе влияет на физику полета");

	MOP_STRINGC("WaterSplashParticles", "", "Партиклы рождаемые при попадании в воду\nXPS файл");
	MOP_STRINGC("WaterSplashSound", "", "3D Звук проигрываемый при попадании в воду\nИмя ALIASA");

	MOP_STRINGC("ShootParticles", "", "Партиклы рождаемые при выстреле\nXPS файл");
	MOP_STRINGC("ShootSound", "", "3D Звук проигрываемый при выстреле\nИмя ALIASA");

	MOP_STRINGC("ReloadSound", "", "3D Звук проигрываемый при перезарядке\nИмя ALIASA");

	MOP_MISSIONTRIGGERG("onActivate trigger", "onActivate")
	MOP_MISSIONTRIGGERG("onDeactivate trigger", "onDeactivate")

	MOP_MISSIONTRIGGERG("onZoomIn trigger", "onZoomIn")
	MOP_MISSIONTRIGGERG("onZoomOut trigger", "onZoomOut")

	MOP_MISSIONTRIGGERG("startReloading trigger", "startReloading")
	MOP_MISSIONTRIGGERG("endReloading trigger", "endReloading")

	MOP_FLOAT("ReloadingTime", 5.f)

	MOP_STRING("ReloadingProgressGUI","")

	MOP_STRING("Target List 1", "");
	MOP_STRING("Target List 2", "");
	MOP_STRING("Target List 3", "");
	MOP_STRING("Target List 4", "");

	MOP_STRING("Char particle", "");
	MOP_STRING("Char sound", "");
	MOP_STRING("Ragdoll particle", "");
	MOP_STRING("Ragdoll sound", "");

	MOP_BOOLC ("Camera attached to bullet", true, "Камера прицеплена к летящему патрону");
	MOP_BOOLC ("Watch to bullet", true, "Следить за патроном, после того как камера отцепилась");
	MOP_FLOATC("Wait after bullet dead", 2.0f, "Сколько камера наблюдает сцену взрыва патрона в секундах");
	MOP_FLOATC("Max camera distance", 10.0f, "Максимальная дистанция на которую камера отлетает, после чего она\nотцепляется и плавно затормаживается");
	MOP_FLOATC("Camera friction", 98.9f, "Торможение камеры при превышении максимальной дистанции, при 100 затормозится за 1 секунду");
	MOP_FLOATC("Camera hit friction", 99.999f, "Торможение камеры при гибели снаряда, при 100 затормозится за 1 секунду");

	MOP_BOOLC ("Don't reload while bullet exist", true, "Не перезаряжать орудие пока существуют активные патроны");
	MOP_BOOLC ("Shadow cast", true, "Отбрасывать тень");

	MOP_BOOL ("Explode bullets", false);

	MOP_FLOATEX("Explode Radius", 2.0f, 0.1f, 1000000.0f)
	MOP_FLOAT("Explode Damage", 100.0f)
	MOP_FLOATEX("Explode Power", 1.0f,0.1f,100.0f)

	MOP_BOOL ("Show Colliders", false);

	MOP_BOOL ("Debug Draw", false);

	MOP_BOOL ("Visible", true);
	MOP_BOOL ("Active", false);

	MOP_BOOL ("Show Logic debug", false);

	MOP_FLOATEX ("GlowPow", 0.7f, 0.1f, 10.f);
	MOP_COLORC ("GlowColorMin", Color((dword)0x01000000), "Значение цвета при отключенной модели")
	MOP_COLORC ("GlowColorMax", Color((dword)0x01FFFFFF), "Значение цвета при максимальной яркости модели")

	MOP_ARRAYBEG("Weapon Platforms",0,10)
		MOP_STRING("Platform ID","")
	MOP_ARRAYEND

MOP_ENDLIST(Shooter)
