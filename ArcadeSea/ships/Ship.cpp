#include "..\pch.h"
#include "Ship.h"
#include "BrokenSystemMaker.h"
#include "AttackHandler.h"
#include "PartsClassificator.h"
#include "..\weapons\Cannon.h"
#include "..\weapons\Flamethrower.h"
#include "..\weapons\Mine.h"
#include "..\weapons\CannonBallBatcher.h"
#include "..\weapons\CannonBallBatcher.h"

//#include "..\Bonuses\BonusManager.h"
#include "ShipsAI\ShipsAIService.h"

#include "..\asspecial\clothburns.h"

#include "ShipTouch.h"

#include "..\TargetPoints.h"
#include "..\SeaMissionParams.h"

#ifndef _XBOX
	#define SHIP_PROFILING
#endif



const char * Ship::SHIP_CONTROLLER_SERVICE = "ShipAIService";
const float Ship::SHIP_AFTERDEATH_TIME = 30.0f;
const ConstString Ship::playerName("Player");

INTERFACE_FUNCTION

IPhysCombined* ShipSailOwner::GetPhysView()
{
	return m_pShip->GetPhysView();
}

IPhysCloth* ShipSailOwner::CreateCloth(const char* file, long line, IClothRenderInfo & renderInfo, IClothMeshBuilder & builder, IPhysCloth::SimulationData & simData)
{
	return m_pShip->Physics().CreateCloth(file,line, renderInfo, builder, simData);
}

bool ShipSailOwner::EditMode_IsOn()
{
	return m_pShip->EditMode_IsOn();
}

Matrix& ShipSailOwner::GetMatrix(Matrix& mtx)
{
	return m_pShip->GetMatrix(mtx);
}

float ShipSailOwner::GetAlpha()
{
	return m_pShip->GetAlpha();
}

float ShipSailOwner::GetLastDeltaTime()
{
	return m_pShip->GetLastDeltaTime();
}

IClothBurns* ShipSailOwner::GetClothBurns()
{
	return m_pShip->GetClothBurns();
}

void ShipSailOwner::PartDestroyFlagSet(DWORD dwFlag)
{
	m_pShip->PartDestroyFlagSet(dwFlag);
}

const char* ShipSailOwner::GetSoundNameForEvent(dword dwIdx)
{
	return m_pShip->GetSoundNameForEvent(dwIdx);
}

void ShipSailOwner::AcceptVisitor(IPartVisitor& clsf)
{
	m_pShip->AcceptVisitor(clsf);
}


void WeaponLightManager::SetLightObjects( MissionObject* mo, array<ShipContent::WeaponLight> & aLights )
{
	m_aWeaponLights.DelAll();
	if( aLights.Size() > 0 && mo )
	{
		m_aWeaponLights.AddElements( aLights.Size() );
		for( long n=0; n<m_aWeaponLights; n++ )
		{
			mo->FindObject( aLights[n].pcLightObjID, m_aWeaponLights[n].obj );
			static const ConstString typeId("LightPoint");
			if( m_aWeaponLights[n].obj.Ptr() && !m_aWeaponLights[n].obj.Ptr()->Is(typeId) )
			{
				api->Trace("Warning! WeaponLightManager::SetLightObjects() : Mission object \"%s\" is not LightPoint object", aLights[n].pcLightObjID.c_str() );
				m_aWeaponLights[n].obj.Reset();
			}
			m_aWeaponLights[n].pWeapon = NULL;
			m_aWeaponLights[n].fTime = 0.f;
			m_aWeaponLights[n].fLifeTime = aLights[n].fLifeTime;
		}
	}
}

void WeaponLightManager::Update(float fDeltaTime)
{
	for( long n=0; n<m_aWeaponLights; n++ )
	{
		if( !m_aWeaponLights[n].pWeapon || !m_aWeaponLights[n].obj.Ptr() )
			continue;
		m_aWeaponLights[n].fTime += fDeltaTime;
		// выключить источник света (догорел?)
		if( m_aWeaponLights[n].fTime >= m_aWeaponLights[n].fLifeTime )
		{
			if( m_aWeaponLights[n].obj.Validate() )
				m_aWeaponLights[n].obj.Ptr()->Activate( false );
			m_aWeaponLights[n].pWeapon = NULL;
		}
		// продолжает светить?
		else
		{
			SetPos( n, m_aWeaponLights[n].pWeapon->GetParentTransform().MulVertex(m_aWeaponLights[n].pWeapon->GetPosition()) );
		}
	}
}

void WeaponLightManager::AddLight(Weapon* pWeapon)
{
	Assert( pWeapon );
	Vector pos = pWeapon->GetParentTransform().MulVertex( pWeapon->GetPosition() + m_fOffset * pWeapon->GetDirection2D() );

	long n;
	// смотрим попадает ли источник света в зону куда уже не надо ничего добавлять
	for( n=0; n<m_aWeaponLights; n++ )
	{
		if( !m_aWeaponLights[n].pWeapon ) continue;
		if( ~(m_aWeaponLights[n].pos - pos) < m_fExcludeRadius * m_fExcludeRadius )
			return;
	}
	// ищем свободный источник света
	for( n=0; n<m_aWeaponLights; n++ )
		if( !m_aWeaponLights[n].pWeapon && m_aWeaponLights[n].obj.Validate() && !m_aWeaponLights[n].obj.Ptr()->IsActive() ) break;
	// нет свободных источников
	if( n>=m_aWeaponLights ) return;
	// занимаем этот источник
	m_aWeaponLights[n].pWeapon = pWeapon;
	m_aWeaponLights[n].fTime = 0.f;
	SetPos(n,pos);
	m_aWeaponLights[n].obj.Ptr()->Activate(true);
}

void WeaponLightManager::SetPos( long n, const Vector& pos )
{
	m_aWeaponLights[n].pos = pos;
	char param[3][64];
	crt_snprintf( param[0], sizeof(param[0]), "%f", pos.x );
	crt_snprintf( param[1], sizeof(param[1]), "%f", pos.y );
	crt_snprintf( param[2], sizeof(param[2]), "%f", pos.z );
	const char* p[3]; p[0]=param[0]; p[1]=param[1]; p[2]=param[2];
	m_aWeaponLights[n].obj.Ptr()->Command("moveto",3,p);
}


//Конструктор
Ship::Ship() :
	littleMen_(_FL_),
	m_aSailorHandhold(_FL_),
	m_Lods(_FL_)
{
	m_bNoPhysicsSails = false;
	fakeSails_ = true;

	rotationMotion_ = 0.0f;
	forwardMotion_ = 0.0f;
	offsetMotion_ = 0.f;
	patternObj_ = NULL;
	rigidBody_ = NULL;
	vPos_ = 0.0f;
	shipController_ = NULL;

	m_SailOwner.SetShip(this);

	m_connectToObj.Reset();
	m_pCBurns.Reset();
	m_pTouch.Reset();

	m_bNoSwing = false;
	m_fLastDeltaTime = 0.f;
	realForwardMotion_ = 0.0f;
	realOffsetMotion_ = 0.f;
	realRotationMotion_ = 0.0f;
	m_vCurSpeed = 0.f;
	alpha_ = 1.0f;
	releaseTimer_ = SHIP_AFTERDEATH_TIME;
	m_fShipMoveSoundTime = 0.f;
	m_fShipMoveFadeTime = 0.f;
	m_pSoundFireHull = NULL;
	m_pSoundFireSail = NULL;
	m_pSoundTouch = NULL;
	m_pSoundMoving = NULL;
	globalTime_ = 0.f;
	m_dieType = DieType_none;
	m_bPause = false;
	m_fGUIProgressMinimizeK = 0.2f;

	m_pAniBlender = NULL;
	m_pcBreakAnimation = "";
	m_pAniListener = NULL;
	m_pAniDestruct = NULL;

	PartDestroyFlagReset();
	finder_ = NULL;
	m_bCannonVolley = false;

	m_nMineQuantity = m_nMineQuantityMax = -1;

	m_bIsActivePhysObject = false;
	m_bRelativeShipPos = false;

	m_bNoTearedSails = true;

	SetOwner(this,this);

	m_bInitShow = m_bInitActive = false;
	m_bNotCreated = true;

	m_debugInfoData.infoSwitch = 0;

	m_bFrameUpdated = false;
}

//Деструктор
Ship::~Ship()
{
	Destroy();
}

void Ship::Destroy()
{
	// выгружаем объект для QT поиска
	RELEASE( finder_ );

	// отписываем объект из системы горения парусов
	if( m_pCBurns.Validate() )
		m_pCBurns.Ptr()->ExcludeObjects( (MissionObject*)this );

	// уничтожим звуки
	RELEASE( m_pSoundFireHull );
	RELEASE( m_pSoundFireSail );
	RELEASE( m_pSoundTouch );
	RELEASE( m_pSoundMoving );

	// уничтожаем физ тело
	if (rigidBody_)
	{
		rigidBody_->Release();
		rigidBody_ = NULL;
	}

	// отрегиваемся из паттерна
	if (patternObj_)
		patternObj_->UnregisterShipObject(this);

	// отгружаем объект управления кораблем
	if ( shipController_ )
	{
		IShipControlService * ctrlService = (IShipControlService *)api->GetService( SHIP_CONTROLLER_SERVICE );
		if( ctrlService ) ctrlService->ReleaseShipController(shipController_);
		shipController_ = NULL;
	}

	// убиваем детей
	for (unsigned int i = 0; i < GetChildren().Size(); ++i)
		delete GetChildren()[i];
	GetChildren().DelAll();

	// убиваем людей
	for (unsigned int i = 0; i < littleMen_.Size(); ++i)
		delete littleMen_[i];
	littleMen_.DelAll();

	// уничтожаем места для людей
	m_aSailorHandhold.DelAll();

	// отгружаем лоды
	ReleaseModelLods();

	// убиваем анимацию разрушения
	RELEASE(m_pAniDestruct);

	// партиклы
	m_particescontainer.ReleaseAll();

	m_bNotCreated = true;

	//Rebuild();
}

void Ship::OnNotify(ShipPattern::Notification type)
{
	if (!patternObj_)
		return;

	switch(type)
	{
	case ShipPattern::ParamChanged:
		CopyPattern();
		break;
	case ShipPattern::PatternRemoved:
		{
			for (unsigned int i = 0; i < GetChildren().Size(); ++i)
				delete GetChildren()[i];
			GetChildren().DelAll();

			patternObj_ = NULL;
		}
		break;
	}
}

bool Ship::Init()
{
	return true;
}

void Ship::CopyPattern()
{
	m_bNotCreated = false;

	Assert(patternObj_);

	// удаляем старую модель (со всеми лодами)
	ReleaseModelLods();

	// получаем параметры из паттерна
	patternObj_->CopyContent(content_);

	m_pcBreakAnimation = content_.pcBreakAnimation;

	// создаем массив лодов
	CreateModelLods();

	IAnimationProcedural* pAni = NULL;
	IAnimationProcedural* pAniRefl = NULL;

	// Создаем новую сцену(корабль)
	IGMXScene *pModelMain, *pModelRefl, *pModelRefr;
	GetLodModels(0,pModelMain,pModelRefl,pModelRefr);

	// ставим анимацию разрушения
	RELEASE(m_pAniDestruct);
	if( !string::IsEmpty(content_.pcBreakAnimation) )
		m_pAniDestruct = Animation().Create( content_.pcBreakAnimation, _FL_ );

	// блендер анимации
	m_pAniBlender = NEW ShipAniBlendStage();
	Assert(m_pAniBlender);
	m_pAniBlender->Init(this,m_pAniDestruct);

	// прописаваем моделькам анимацию
	SetAnimationToLods(m_pAniDestruct);//pAni,pAniRefl);

	if (rigidBody_)
	{
		rigidBody_->Release();
		rigidBody_ = NULL;
	}

	if (!EditMode_IsOn())
	{
		rigidBody_ = Physics().CreateCombined(_FL_, Matrix());
		rigidBody_->AddBox(	content_.massBoxBounds_, Matrix().BuildPosition(content_.massBoxCenter_));
		rigidBody_->SetMass(0, content_.fMass_);
		rigidBody_->EnableCollision(0, false);
		rigidBody_->EnableResponse(0, false);
		rigidBody_->EnableRaycast(0, false);
	}

	for (unsigned int i = 0; i < GetChildren().Size(); ++i)
		delete GetChildren()[i];
	GetChildren().DelAll();

	for (unsigned int i = 0; i < littleMen_.Size(); ++i)
		delete littleMen_[i];
	littleMen_.DelAll();
	
	GetShapes().DelAll();

	for (unsigned int i = 0; i < 4; ++i)
		content_.sides_[i].SetOwner(this);

	for (unsigned int i = 0; i < content_.hull_.Size(); ++i)
	{
		HullPart * part = NEW HullPart(content_.hull_[i]);

		part->SetParent(this);
		part->SetOwner(this,this);
		UpdatePhysForm(content_.hull_[i].form_, *part, true);
		m_pAniBlender->AttachPart(part,content_.hull_[i].modelDamagedName_);
		GetChildren().Add(part);
	}

	unsigned int firstMastIndex = GetChildren().Size();
	for (unsigned int i = 0; i < content_.masts_.Size(); ++i)
	{
		Mast * mast = NEW Mast(content_.masts_[i]);

		mast->SetParent(this);
		mast->SetOwner(this,this);
		GetChildren().Add(mast);
		
		ShipPart * prevPart = mast;
		for (unsigned int j = 0; j < content_.masts_[i].partsDesc_.Size(); ++j)
		{
			MastPart * part = NEW MastPart(*mast, content_.masts_[i].partsDesc_[j]);

			part->SetParent(mast);
			part->SetOwner(this,this);
			UpdatePhysForm(content_.masts_[i].partsDesc_[j].form_, *part);
			m_pAniBlender->AttachPart(part,content_.masts_[i].partsDesc_[j].modelName_);
			mast->AddPart(part);
			prevPart->GetChildren().Add(part);
			prevPart = part;

			for (unsigned int k = 0; k < content_.masts_[i].partsDesc_[j].yards_.Size(); ++k)
			{
				Yard * yard = NEW Yard(content_.masts_[i].partsDesc_[j].yards_[k]);

				yard->SetParent(part);
				yard->SetOwner(this,this);
				UpdatePhysForm(content_.masts_[i].partsDesc_[j].yards_[k].form_, *yard);
				m_pAniBlender->AttachPart(yard,content_.masts_[i].partsDesc_[j].yards_[k].modelName_);
				part->GetChildren().Add(yard);
			}
		}
	}

	SetupPhysicParams();

	// установить связи дети-родитель для костей в блендере анимации
	m_pAniBlender->SetupChilds();

	unsigned int sailNum = 0;
	for (unsigned int i = 0; i < content_.masts_.Size(); ++i)
	{
		Mast * mast = (Mast*)GetChildren()[firstMastIndex+i];

		for (unsigned int j = 0; j < content_.masts_[i].sails_.Size(); ++j)
		{
			if( content_.cookedSails_[sailNum] )
			{
				Sail * sail = NEW Sail(content_.masts_[i].sails_[j], string().Format("%s_sail%d%d", patternObj_->GetObjectID().c_str(), i,j), &m_SailOwner);

				sail->SetParent(mast);
				sail->SetOwner(this,this);
				sail->Make(fakeSails_, content_.cookedSails_[sailNum], m_bNoPhysicsSails);
				mast->GetChildren().Add(sail);
			}
			++sailNum;
		}
	}

	for (unsigned int i = 0; i < content_.ropes_.Size(); ++i)
	{
		Rope * rope = NEW Rope(content_.ropes_[i]);

		rope->SetParent(this);
		rope->SetOwner(this,this);
		rope->Make();
		GetChildren().Add(rope);
	}

	for (unsigned int i = 0; i < content_.littleMen_.Size(); ++i)
	{
		if( content_.littleMen_[i].patternName.IsEmpty() ) continue;
		LittleManImpl * man = NEW LittleManImpl(this, content_.littleMen_[i], Mission());
		littleMen_.Add(man);
	}

	RELEASE(pAni);
	RELEASE(pAniRefl);

	TieSailorsToHandholds();

	statistic.ResetStatistics();
}

void _cdecl Ship::InGameInit(float fDeltaTime, long level)
{
	if( m_nInitPhase == 2 )
	{
		IShipControlService * ctrlServ = (IShipControlService *)api->GetService(SHIP_CONTROLLER_SERVICE);
		if ( ctrlServ )
		{
			if( shipController_ )
				ctrlServ->ReleaseShipController(shipController_);
			shipController_ = ctrlServ->CreateShipController(ctrlParamsObjectName_, *this, Mission());
		}

		if( m_connectToName.NotEmpty() )
			FindObject( m_connectToName, m_connectToObj );
		else
			m_connectToObj.Reset();

		static const ConstString id_ClothBurns("ClothBurns");
		FindObject(id_ClothBurns, m_pCBurns.GetSPObject());
		if( m_pCBurns.Ptr() && !m_pCBurns.Ptr()->Is(id_ClothBurns) )
			m_pCBurns.Reset();

		static const ConstString id_ShipTouch("ShipTouch");
		FindObject(id_ShipTouch, m_pTouch.GetSPObject());
		if( m_pTouch.Ptr() && !m_pTouch.Ptr()->Is(id_ShipTouch) )
			m_pTouch.Reset();
	}
	else {
		// подсчитаем относительную позицию (что бы в самом начеле корабль мог быть привязан к треку)
		if( m_connectToObj.Ptr() )
		{
			Matrix transf(true);
			m_connectToObj.Ptr()->GetMatrix(transf);
			m_mtxInitRelativeShipTransform = m_mtxRelativeShipTransform = Matrix(vAngles_,vPos_) * transf.Inverse();
			m_bRelativeShipPos = !IsActive() || !IsShow();
			//api->Trace("ship connect init: pos(%.2f,%.2f) mtx.pos(%.2f,%.2f) mtx.ang(x:%.2f,%.2f z:%.2f,%.2f)", vPos_.x,vPos_.z, transf.pos.x,transf.pos.z, transf.vx.x,transf.vx.z, transf.vz.x,transf.vz.z);
		}
	}

	m_nInitPhase--;

	if( m_nInitPhase<=0 )
		DelUpdate(&Ship::InGameInit);
}

void _cdecl Ship::EditorInit(float fDeltaTime, long level)
{
	if (!patternObj_)
	{
		// ищем шаблон, регистрируемся, читаем с него параметры
		MOSafePointer safeptr;
		FindObject(patternName_, safeptr);
		static const ConstString typeId("ShipPattern");
		if (safeptr.Ptr() && safeptr.Ptr()->Is(typeId))
		{
			if (patternObj_)
				patternObj_->UnregisterShipObject(this);

			patternObj_ = (ShipPattern*)safeptr.Ptr();

			patternObj_->RegisterShipObject(this);
			CopyPattern();
		}
	}
}

void Ship::ReadMOPs(MOPReader & reader)
{
	patternName_ = reader.String();
	vInitPos_ = vPos_ = reader.Position();
	vInitAngles_ = vAngles_ = reader.Angles();
	fMaxSpeed_ = reader.Float();
	fMinSpeed_ = reader.Float();
	fMaxRotateSpeed_ = Deg2Rad(reader.Float());

	startHP_ = hp_ = reader.Float();
	ctrlParamsObjectName_ = reader.String();
	m_connectToName = reader.String();

	m_pcGUIProgressName = reader.String();
	m_fGUIProgressUpper = reader.Float();
	m_fGUIProgressMinimizeK = reader.Float();
	m_fGUIProgressMaxShowDistance = reader.Float();
	DisableGUIProgressBar();

	m_nMineQuantityMax = reader.Long();
	m_nMineQuantity = m_nMineQuantityMax;
	triger_mineBeOver.Init(reader);

	struct EnumSide
	{
		ConstString id;
		ShipParty party;
	};

	static EnumSide sides[] =
	{
		{ConstString("Friend"), SHIP_SIDE_FRIEND},
		{ConstString("Enemy"), SHIP_SIDE_ENEMY},
		{ConstString("Player1"), SHIP_SIDE_PLAYER_1},
		{ConstString("Player2"), SHIP_SIDE_PLAYER_2},
		{ConstString("Player3"), SHIP_SIDE_PLAYER_3},
		{ConstString("Player4"), SHIP_SIDE_PLAYER_4}
	};

	ConstString pcShipParty = reader.Enum();
	shipParty_ = SHIP_SIDE_NEUTRAL;

	for(dword i = 0; i < ARRSIZE(sides); i++)
	{
		if(sides[i].id == pcShipParty)
		{
			shipParty_ = sides[i].party;
			break;
		}
	}

	m_bNoTearedSails = reader.Bool();

	m_cModelColor = reader.Colors();

	m_bCannonVolley = reader.Bool();

	m_bNoSwing = reader.Bool();
	bSeaReflection_ = reader.Bool();
	bSeaRefraction_ = reader.Bool();
	fakeSails_ = reader.Bool();
	m_bNoPhysicsSails = reader.Bool();
	m_bShowWhileSink = reader.Bool();
	
	// ищем шаблон, регистрируемся, читаем с него параметры
	MOSafePointer mo;
	FindObject(patternName_, mo);
	static const ConstString typeId("ShipPattern");
	if (mo.Ptr() && mo.Ptr()->Is(typeId))
	{
		if (patternObj_)
			patternObj_->UnregisterShipObject(this);

		patternObj_ = (ShipPattern*)mo.Ptr();

		patternObj_->RegisterShipObject(this);
		CopyPattern();
	}

	m_bInitShow = reader.Bool();
	m_bInitActive = reader.Bool();

	Show(m_bInitShow);
	Activate(m_bInitActive);
}

// Рестарт объекта
void Ship::Restart()
{
	// восстанавливаем позицию
	vPos_ = vInitPos_;
	vAngles_ = vInitAngles_;
	m_bFrameUpdated = true;

	Respawn();

	bool bReactivatePhysicActor = true;
	if ( IsShow() != m_bInitShow )
	{
		Show(m_bInitShow);
		bReactivatePhysicActor = false;
	}
	if ( IsActive() != m_bInitActive )
	{
		Activate(m_bInitActive);
		bReactivatePhysicActor = false;
	}

	if( bReactivatePhysicActor )
		EnablePhysicActor(IsActive() && IsShow());

	m_bRelativeShipPos = !IsActive() || !IsShow();
	m_mtxRelativeShipTransform = m_mtxInitRelativeShipTransform;

/*	Destroy();
	Rebuild();

	// апдейт физики для того, что бы грохнуть старые буферы удаленных парусов
	// т.к. иначе может память переполниться
	Physics().UpdateEndFrame(m_fLastDeltaTime);
	Physics().UpdateBeginFrame(0.f);

	ReCreate();*/
	//patternObj_->
}

//Создание объекта
bool Ship::Create(MOPReader & reader)
{
	alpha_ = 1.0f;

	Controls().EnableControlGroup("Mission.Sea", true);

	ReadMOPs(reader);

	finder_ = QTCreateObject(MG_DAMAGEACCEPTOR, _FL_);
	if( finder_ )
	{
		finder_->SetBox(content_.boundsCenter_-content_.boundsSize_*0.5f,
						content_.boundsCenter_+content_.boundsSize_*0.5f);
		finder_->Activate(true);
	}
	Registry( MG_SHIP );

	if (EditMode_IsOn())
		SetUpdate(&Ship::EditorInit, ML_GEOMETRY1);
	else {
		m_nInitPhase = 2;
		SetUpdate(&Ship::InGameInit, ML_GEOMETRY1);
	}

	// внесем поправки от сложности
	float k = 1.f;
	const char* pDifficulty = api->Storage().GetString("Profile.Global.Difficulty","");
	if( pDifficulty )
	{
		char pcFolder[256];
		crt_snprintf(pcFolder,sizeof(pcFolder),"Profile.%s.Ship.hp", pDifficulty);
		k = api->Storage().GetFloat(pcFolder,100.f) * 0.01f;
	}
	startHP_ = hp_ * k;
	hp_ = startHP_;

	// установим обработку для вывода инфы о корабле
	if( !EditMode_IsOn() )
		SetUpdate(&Ship::DebugInfoWork, ML_DEBUG);

	return true;
}

void Ship::PostCreate()
{
	if( m_pcGUIProgressName.NotEmpty() )
	{
		FindObject(m_pcGUIProgressName, m_GUIProgress);
		static const ConstString typeId("IGUIElement");
		if( !m_GUIProgress.Ptr() || !m_GUIProgress.Ptr()->Is(typeId) )
			m_GUIProgress.Reset();
		if( m_GUIProgress.Ptr() )
			((IGUIElement*)m_GUIProgress.Ptr())->GetSize( m_fGUIProgressWidth, m_fGUIProgressHeight );
		DisableGUIProgressBar();
	}
	else
		m_GUIProgress.Reset();

	// список источников света для стрельбы из пушек
	m_WeaponLightManager.SetExcludeRadius( content_.fExcludeLightRadius );
	m_WeaponLightManager.SetLightOffset( content_.fLightOffset );
	m_WeaponLightManager.SetLightObjects( this, content_.aWeaponLights );

	// ишем объекты ачивментов
	static const ConstString strSeaWolf = ConstString("ACHIEVEMENT_SEA_WOLF");
	static const ConstString strGunmaster = ConstString("ACHIEVEMENT_GUNMASTER");
	static const ConstString strSeaMisParams = ConstString("SeaMissionParams");

	if (!FindObject(strSeaWolf, achievSeawolf.GetSPObject()))
		api->Error("ERROR: Achievements: Can't find achievement object (ship): %s", strSeaWolf.c_str());

	if (!FindObject(strGunmaster, achievGunmaster.GetSPObject()))
		api->Error("ERROR: Achievements: Can't find achievement object (ship): %s", strGunmaster.c_str());

	hp_ = startHP_;
}

void Ship::UpdatePhysForm(const PhysForm& form, ShipPart& part, bool enableCollision)
{
	if (!rigidBody_) return;

	Matrix m;
	unsigned int count = form.boxes_.Size();
	if( count!= 1 )
	{
		long k = 2;
	}
	for (unsigned int j = 0 ; j < count; ++j)
	{
		m.Build(form.boxes_[j].angles, form.boxes_[j].pos);
		rigidBody_->AddBox(form.boxes_[j].size, m);
		unsigned int index = rigidBody_->GetCountForBuild()-1;

		rigidBody_->SetMass(index, 0.01f);
		rigidBody_->EnableCollision(index, enableCollision);
		//rigidBody_->EnableCollision(index, false);

		part.GetShapes().Add(index);
		part.GetInitialPoses().Add(m);
	}
}

// установка физ. параметров
void Ship::SetupPhysicParams()
{
	if (!rigidBody_)
		return;

	rigidBody_->Build();
	rigidBody_->SetGroup(phys_ship);

	rigidBody_->EnableCollision(m_bIsActivePhysObject);
	rigidBody_->Activate(m_bIsActivePhysObject);
	rigidBody_->EnableGravity(false);

	// наоборот понижаем до минимума, нечего там лишние итерации плодить - ибо все равно у нас жесткая схема крепления шейпов
	rigidBody_->SetSolverIterations(2);

	// отключаем все лишнее у бокса распределения массы
	rigidBody_->EnableRaycast(0, false);
	rigidBody_->EnableCollision(0, false);
	rigidBody_->EnableResponse(0, false);
		
	rigidBody_->SetTransform(Matrix().Build(vAngles_, vPos_));

	// делаем трение о воду
	rigidBody_->SetMotionDamping(content_.fLinearFrictionForce_, content_.fAngularFrictionForce_);

	// повышаем устойчивость - центр масс смещаем вниз
	rigidBody_->SetMass(content_.fMass_);
	rigidBody_->SetCenterMass(content_.vCenterMass_);
	
/*	IPhysMaterial * mtl = Physics().GetMaterial(mtl_ships);

	mtl->SetRestitution(0.0f);
	mtl->SetDynamicFriction(0.0f);
	mtl->SetStaticFriction(0.02f);

	rigidBody_->SetMaterial(mtl);*/
}

//Обновление параметров
bool Ship::EditMode_Update(MOPReader & reader)
{
	ReadMOPs(reader);

	return true;
}


void Ship::EditMode_GetSelectBox(Vector & min, Vector & max)
{
	min = content_.boundsCenter_ - 0.5f*content_.boundsSize_;
	max = content_.boundsCenter_ + 0.5f*content_.boundsSize_;
}

bool Ship::IsLimitedParticles(const Vector & pos)
{
	return (~Render().GetView().MulVertex(pos)) < content_.m_fSFXLimitationDistance * content_.m_fSFXLimitationDistance;
}

float ComputeForestalledTargetTime(const Vector& shootPos, Ship* targetShip, float ballSpeed)
{
	// матрица цели
	Matrix targetMtx;
	targetShip->GetMatrix(targetMtx);

	// расстояние и направление до цели в начальный момент времени
	Vector vDir0 = targetMtx.pos - shootPos;
	float fDist0 = vDir0.Normalize();

	// скорость цели (скалярная величина)
	float fShipSpeed = targetShip->GetCurrentVelocity();
	// при достаточно больших скоростях учитываем ошибку расчета времени по начальной позиции
	if( fShipSpeed > 1.f )
	{
		// направление движения цели
		Vector vTargMoveDir = Vector(0,0,1);
		float angle = targetShip->GetCurrentDirectionAngle();
		vTargMoveDir.Rotate(angle);

		// косинус угла между направлением на цель и направлением движения цели
		float ca = vTargMoveDir.x * vDir0.x + vTargMoveDir.z * vDir0.z;
		// квадрат синуса
		float saq = (ca*ca<1.f) ? (1.f - ca*ca) : 0.f;

		// посчитаем критические данные. Выражение под корнем - не должно быть отрицательным, но математически может, ибо
		// это случай, когда корабль со скоростью выше ядра постоянно уносится от нас,
		// т.е. выстрел никогда до него не достигнет
		float fTmp = ballSpeed*ballSpeed - fShipSpeed*fShipSpeed * saq;
		// в случае описанном выше не мудрим и возвращаем обычную точку с погрешностями - ибо все равно не попадем
		if( fTmp <= 0.f )
			return fDist0 / ballSpeed;

		// еще одна проверка промежуточных данных
		fTmp = sqrtf( fTmp ) - fShipSpeed*ca;
		if( fTmp < 0.01f )
			return fDist0 / ballSpeed;

		return fDist0 / fTmp;
	}

	// при маленьких скоростях ошибку не учитываем
	return fDist0 / ballSpeed;
}

Vector ComputeForestalledTargetPos(const Vector& shootPos, Ship* targetShip, float ballSpeed)
{
	Assert(	targetShip );

	// время полета
	float ballFlyTime = ComputeForestalledTargetTime(shootPos, targetShip, ballSpeed);

	// матрица цели
	Matrix targetMtx;
	targetShip->GetMatrix(targetMtx);

	// опережение
	Vector forestall = Vector(0,0,1);
	float angle = targetShip->GetCurrentDirectionAngle();
	float vel = targetShip->GetCurrentVelocity();
	forestall.Rotate(angle);
	forestall *= vel;
	forestall *= ballFlyTime*1.25f;

	return targetMtx.pos + forestall;
}

long Ship::GetCannonWeaponSide(MissionObject * fireTarget, float& fTargDist, float fReadyProcent)
{
	// положение цели
	Matrix mtxTarg(true);
	fireTarget->GetMatrix(mtxTarg);
	// положение собственно корабля
	Matrix mtxSelf(true);
	GetMatrix(mtxSelf);
	// позиция цели относительно корабля
	Vector vRelTargPos = mtxSelf.MulVertexByInverse(mtxTarg.pos);
	// дистанция до цели
	fTargDist = vRelTargPos.GetLength();

	// проходим по всем орудийным сторонам и решаем что делать
	for( long i = 0; i < 4; i++ )
	{
		unsigned int q = content_.sides_[i].GetWeaponCount();
		if( q == 0 )
			continue;

		// проверяем, та ли это сторона, которая нам нужна
		Weapon* pW = content_.sides_[i].GetWeapon(0).GetWeapon();
		if( !pW ) continue;
		if( pW->GetDirectionAngle() < 0.f || pW->GetDirectionAngle() > PI ) {
			if( vRelTargPos.x > 0.f ) continue;
		} else {
			if( vRelTargPos.x < 0.f ) continue;
		}

		// подсчитаем процент готовых к стрельбе пушек
		unsigned int nready = 0;
		unsigned int nquantity = 0;
		for( unsigned int k=0; k<q; k++ )
		{
			Weapon * pW = content_.sides_[i].GetWeapon(k).GetWeapon();
			// не пушка - пропускаем
			if( !pW || pW->GetType() != Weapon::WeaponType_cannon )
				continue;
			// не может стрелять - закончим поиск
			nquantity++;
			if( pW->CanFire() )
				nready++;
		}

		// если не все пушки готовы к стрельбе, то ждем до полной готовности (переходим к следущей стороне)
		if( fReadyProcent < 1.f )
			nquantity = (unsigned int)(nquantity * fReadyProcent);
		if( nready>0 && nready>=nquantity ) return i;
	}

	return -1;
}

void Ship::FinishWeaponSideCannons(long nWeaponSide, float fDistance)
{
	for( unsigned int k=0; k<content_.sides_[nWeaponSide].GetWeaponCount(); k++ )
	{
		Weapon * pW = content_.sides_[nWeaponSide].GetWeapon(k).GetWeapon();
		// не пушка - пропускаем
		if( !pW || pW->GetType() != Weapon::WeaponType_cannon )
			continue;
		//
		if( pW->CanFire() )
		{
			// отстрел даже если не можем попадать в цель
			Vector vtarg = pW->GetParentTransform().MulVertex( pW->GetPosition() ) +
				pW->GetParentTransform().MulNormal( pW->GetDirection2D() ) * fDistance;
			if( pW->Fire(vtarg,false) )
				m_WeaponLightManager.AddLight( pW );
		}
	}
}

// стрельнуть по объекту
void Ship::FireCannons( MissionObject * fireTarget, float fFireLevel )
{
	Assert(fireTarget);

	// стрельба не по другому кораблю: используем таргет поинты
	static const ConstString tid("Ship");
	TargetPoints* pTargPoints = TargetPoints::GetNextTargetPoint(&Mission());
	if( !fireTarget->Is(tid) )
	{
		// стрельба только по тагрет поинтам
		if( pTargPoints )
		{
			float fTargDist;
			long nWeaponSide = GetCannonWeaponSide(fireTarget,fTargDist,1.f);
			if( nWeaponSide >= 0 )
			{
				// сколькими надо стрелять
				unsigned int q = content_.sides_[nWeaponSide].GetWeaponCount();
				// сколько есть точек для стрельбы
				array<long> aPoints(_FL_);
				long nTargQnt = pTargPoints->GetPointsArray(q, aPoints);
				long n = 0;

				for( unsigned int k=0; k<q && n<nTargQnt; k++ )
				{
					Weapon * pW = content_.sides_[nWeaponSide].GetWeapon(k).GetWeapon();
					// не пушка - пропускаем
					if( !pW || pW->GetType() != Weapon::WeaponType_cannon )
						continue;

					// стрельба
					pW->FireByPoint( pTargPoints, aPoints[n] );
					// добавляем свет от выстрела
					m_WeaponLightManager.AddLight( pW );

					n++;
				}
				// добъем цель из остатков пушек
				//FinishWeaponSideCannons(nWeaponSide, fTargDist);

				// занимаем таргет поинты
				TargetPoints::BorrowTargetPoints(pTargPoints,this);

				content_.cannonFireBase.Activate(Mission(),false);
			}
		}
		return;
	}

	// стрельба по другим кораблям
	float fTargDist;
	long nWeaponSide = GetCannonWeaponSide(fireTarget,fTargDist, m_bCannonVolley ? 1.f : 0.3f);
	if( nWeaponSide >= 0 )
	{
		unsigned int q = content_.sides_[nWeaponSide].GetWeaponCount();

		Matrix mtxSelf(true);
		GetMatrix(mtxSelf);
		Matrix mtxTarg(true);
		fireTarget->GetMatrix(mtxTarg);

		m_debugInfoData.cannonFire.DelAll();

		// стрельба
		bool bFireStarted = false;
		WeaponTargetZone wtz_(GetPosition(),fireTarget,false);
		//WeaponTargetZone wtz_(mtxTarg.pos);
		for( unsigned int k=0; k<q; k++ )
		{
			Weapon * pW = content_.sides_[nWeaponSide].GetWeapon(k).GetWeapon();
			// не пушка - пропускаем
			if( !pW || pW->GetType() != Weapon::WeaponType_cannon )
				continue;
			//
			if( pW->Fire( wtz_, false ) )
			{
				m_WeaponLightManager.AddLight( pW );
				bFireStarted = true;
			}
			if( m_debugInfoData.infoSwitch & sdim_cannon_info )
			{
				long idxCannon = m_debugInfoData.cannonFire.Add();
				m_debugInfoData.cannonFire[idxCannon].from = mtxSelf * pW->GetPosition();
				m_debugInfoData.cannonFire[idxCannon].to = mtxTarg.pos;
				m_debugInfoData.cannonFire[idxCannon].speed = ((Cannon*)pW)->GetShotSpeed();
			}
		}
		// если был хотя бы один выстрел, то стреляем всеми оставшимися пушками
		if( m_bCannonVolley && bFireStarted )
			FinishWeaponSideCannons(nWeaponSide, fTargDist);
		if( bFireStarted )
		{
			content_.cannonFireBase.Activate(Mission(),false);
		}
	}
}

// стрельнуть огнеметами
void Ship::FireFlamethrower()
{
	// хелпер для стрельбы из огнеметов
	class FlameThrowerHelper : public IWeaponInspector
	{
	public:
		FlameThrowerHelper() {bDoFireCheck=true;}
		virtual void Visit(FlameThrower& ft)
		{
			ft.SetCheckFire(bDoFireCheck);
			ft.Fire(Vector(0.f), 1.f);
		}
		virtual void Visit(Cannon&) {}
		virtual void Visit(Mine&) {}

		bool bDoFireCheck;
	};

	FlameThrowerHelper helper;
	// для игрока не делать проверку на стрельбу из огнемета
	// стреляем тогда когда нажали стрельбу
	helper.bDoFireCheck = (GetObjectID() != playerName);

	RunWeaponsInspector(helper);
}

// стрельнуть минами
void Ship::FireMines()
{
	// не стреляем если нет мин
	if( m_nMineQuantity == 0 )
	{
		triger_mineBeOver.Activate(Mission(), false);
		return;
	}

	// хелпер для стрельбы минами
	class MineHelper : public IWeaponInspector
	{
	public:
		long count;

		MineHelper(long q) {count = q;}
		virtual void Visit(Mine& mine)
		{
			if( count!=0 )
			{
				if( mine.Fire(Vector(0.f),1.f) )
				{
					if( count > 0 ) count--;
				}
			}
		}
		virtual void Visit(Cannon&) {}
		virtual void Visit(FlameThrower&) {}
	};

	MineHelper mh(m_nMineQuantity);
	RunWeaponsInspector( mh );
	m_nMineQuantity = mh.count;
}

void Ship::GetSizes( float & fLength, float & fWidth ) const
{
	if (m_Lods.Size()>0 && m_Lods[0].pModelMain)
	{
		GMXBoundBox box = m_Lods[0].pModelMain->GetBound();
		fLength = (box.vMax.z - box.vMin.z);
		fWidth = (box.vMax.x - box.vMin.x);
	}
	else
	{
		fLength = content_.boundsSize_.z;
		fWidth = content_.boundsSize_.x;
	}
}

void Ship::GetWeaponSidesProgress( float & fLCannonProgress, float & fRCannonProgress, float & fFlamethrowerProgress, float & fMineProgress )
{
	fLCannonProgress = content_.sides_[0].GetReloadProgress( Weapon::WeaponType_cannon );
	fRCannonProgress = content_.sides_[1].GetReloadProgress( Weapon::WeaponType_cannon );
	fFlamethrowerProgress = content_.sides_[2].GetReloadProgress( Weapon::WeaponType_flamethrower );
	fMineProgress = content_.sides_[3].GetReloadProgress( Weapon::WeaponType_mine );

	content_.sides_[0].SetDamageMultiply( 1.f );
	content_.sides_[1].SetDamageMultiply( 1.f );
	return;
}

void Ship::ApplyPhysics(float deltaTime)
{
	if (deltaTime == 0.0f) // пока миссия на паузе - ничего не делаем
		return;

	globalTime_ += deltaTime;

	Matrix m;
	GetTransform(m);

	Vector boatPos = m.pos;
	Vector dir(0,0,1);
	Vector side(1,0,0);
	Vector top(0,1,0);
	dir		= m.MulNormal(dir);
	side	= m.MulNormal(side);
	top		= m.MulNormal(top);

	float sailHealthCoef = 0.0f;
	m_partClassificator.Reset();
	AcceptVisitor(m_partClassificator);
    for (unsigned int i = 0; i < m_partClassificator.GetSails().Size(); ++i)
		sailHealthCoef += m_partClassificator.GetSails()[i]->GetHealth();
	if (m_partClassificator.GetSails().Size())
		sailHealthCoef /= m_partClassificator.GetSails().Size();
	else
		sailHealthCoef = 1.0f; // если парусов нет вообще - скорость максимальная

	float speed = fMinSpeed_+(fMaxSpeed_-fMinSpeed_)*sailHealthCoef;
	float moveForce = speed*100.0f;
	float rotateForce = (2*PI*10.0f*Rad2Deg(fMaxRotateSpeed_) + content_.fAngularFrictionForce_);

	// морская качка
	// продольная
	const float ZWaveFreq = 1.0f;
	const float XWaveFreq = 1.0f;
	const float ZWaveScale = 100.0f;
	const float XWaveScale = 500.0f;
	rigidBody_->ApplyLocalTorque(Vector(0,0,cos(ZWaveFreq*globalTime_)*ZWaveScale*content_.fWaveness_));
	// поперечная
	rigidBody_->ApplyLocalTorque(Vector(sin(XWaveFreq*globalTime_)*XWaveScale*content_.fWaveness_,0,0));

	float fMaxTime = PI*128.f / (ZWaveFreq * XWaveFreq);
	if( globalTime_ >= fMaxTime )
		globalTime_ -= fMaxTime;

	// возврат на уровень ватерлинии
	float delta = boatPos.y + content_.fWaterLineHeight_;
	rigidBody_->ApplyForce(Vector(0, -content_.fFloatForce_*delta*content_.fMass_, 0), content_.vCenterMass_);

	// применяем боковую силу для крена при повороте
	rigidBody_->ApplyForce(	side*rotationMotion_*content_.fForwardSlopeForce_*
							(realForwardMotion_ + 0.1f), // FIX: magic numbers
							content_.vCenterMass_+Vector(0, -2.0f, 0));

	// стабилизация от крена набок
	Vector	s(-content_.massBoxBounds_.x, 0, 0);
	Vector	v;
	v = GetTransform(m).MulNormal(s);
	rigidBody_->ApplyForce(Vector(0, -v.y*content_.fBackwardSlopeForce_, 0), s);
	Vector xAxis(1,0,0);
	xAxis = GetTransform(m).MulNormal(xAxis);
	if (xAxis.y > cos(PI/4) || xAxis.y < -cos(PI/4))
		rigidBody_->ApplyLocalTorque(Vector(0,0,-sign(xAxis.y)*7500.0f));


	// стабилизация от продольного крена
	Vector	p(0, 0, -content_.massBoxBounds_.z);
	v = GetTransform(m).MulNormal(p);
	rigidBody_->ApplyForce(Vector(0, -v.y*content_.fBackwardSlopeForce_*content_.fBackAlongSlopeScale_, 0), p);
	

	// реакция на управление
	realForwardMotion_ += (forwardMotion_ - realForwardMotion_)*deltaTime*0.5f;
	realOffsetMotion_ += (offsetMotion_ - realOffsetMotion_)*deltaTime*0.5f;
	rigidBody_->ApplyForce(moveForce*(dir*realForwardMotion_+side*realOffsetMotion_), content_.vCenterMass_);//+Vector(0, 0.0f, sign(realForwardMotion_)*content_.massBoxBounds_.z/2));
	rigidBody_->ApplyLocalTorque(Vector(0, rotateForce*rotationMotion_/(2*fabs(realForwardMotion_)+1.0f), 0)); // FIX magic numbers
}

void Ship::Simulate(float deltaTime)
{
	if (!patternObj_ || !rigidBody_ || IsDead())
		return;

	ApplyPhysics(deltaTime);

	Matrix m;
	vPos_ = GetTransform(m).pos;
	vAngles_ = m.GetAngles();
}

// разрушить корабль
void Ship::Destruct(DieType dt)
{
	Matrix mtx;

	// берем скорость физ тела
	if( rigidBody_ )
	{
		m_vCurSpeed = rigidBody_->GetLinearVelocity();
		rigidBody_->GetTransform(mtx);
	}
	else
		m_vCurSpeed = 0.f;

	// запускаем анимацию смерти:
	IGMXScene *pModel, *pRefl, *pRefr;
	GetLodModels(0,pModel,pRefl,pRefr);
	if( pModel )
	{
		// евенты анимации
		if( !m_pAniListener )
			m_pAniListener = NEW ShipAniEvents(this);//AnimationStdEvents();
		if( m_pAniListener )
		{
			m_pAniListener->Init(&Sound(), &Particles(), &Mission());
			m_pAniListener->SetScene(pModel,mtx);
			m_pAniListener->AddAnimation(m_pAniDestruct);
		}

		m_pAniBlender->TurnOnAnimation();

		if( m_pAniDestruct )
			m_pAniDestruct->ActivateLink("Action");
	}

	m_dieType = dt;

	BrokenSystemMaker maker(*rigidBody_, Physics(), true);
	maker.SetStartNode(this);
	AcceptVisitor(maker);

	maker.Make();

	SetBrokenSystem(maker.GetSystem());

	PartsClassificator classificator;
	AcceptVisitor(classificator);
	for( unsigned int n=0; n<classificator.GetSails().Size(); n++ )
	{
		// фейковый парус гасим
		if( classificator.GetSails()[n] && classificator.GetSails()[n]->IsFake() )
		{
			classificator.GetSails()[n]->FadeIn(0.7f);
		}
	}

	CreateAutoParticle(content_.shipExplosionParticles_.GetSFXName(IsLimitedParticles(mtx.pos)), mtx);

	// отключаем работу физ тела
	if( rigidBody_ )
		rigidBody_->Activate(false);
	//rigidBody_->Release();
	//rigidBody_ = NULL;

	// звук взрыва корабля
	if( content_.shipExplosionSound_ )
		Sound().Create3D( content_.shipExplosionSound_, mtx.pos, _FL_ );

	vAngles_.x = vAngles_.z = 0.f;
	
	releaseTimer_ = SHIP_AFTERDEATH_TIME;

	// записать корабль в статистику
	if( content_.bUsedByStatistics && !content_.bIsBoat )
	{
		float fSinkCount = 1.f + api->Storage().GetFloat("Runtime.Statistic.ShipsSunk",0.f);
		api->Storage().SetFloat("Runtime.Statistic.ShipsSunk",fSinkCount);
	}

	// записать как ачивку на подбитие лодки
	if( content_.bIsBoat )
	{
		if (achievGunmaster.Validate())
			achievGunmaster.Ptr()->Increment(1.0f);
	}
	// записать как ачивку на подбитие корабля
	else
	{
		if (achievSeawolf.Validate())
			achievSeawolf.Ptr()->Increment(1.0f);
	}
}

//Воздействовать на объект сферой
bool Ship::Attack(MissionObject * obj, dword source, float damage, const Vector & center, float radius)
{
	// не активный корабль нельзя атаковать
	if( !IsActive() )
		return false;

	Matrix mtx;

	if (EditMode_IsOn() || 
		!Box::OverlapsBoxSphere(	Matrix().BuildPosition(content_.boundsCenter_)*GetTransform(mtx),
									content_.boundsSize_*0.5f, center, radius) )
		return false;

	m_attackHandler.Init(center, radius, damage, Physics());
	AcceptVisitor(m_attackHandler);
	if( m_attackHandler.HitDetected() )
	{
		hp_ -= m_attackHandler.TransferedDamage() > damage ? damage : m_attackHandler.TransferedDamage();
	}

	if (m_attackHandler.HitDetected())
		for (unsigned int i = 0; i < littleMen_.Size(); ++i)
			littleMen_[i]->AttackMine(center, radius);

	if (hp_ <= 0.0f && !IsBroken())
		Destruct( DieType_mine );

	return m_attackHandler.HitDetected();
}

//Воздействовать на объект линией
bool Ship::Attack(MissionObject * obj, dword source, float damage, const Vector & from, const Vector & to)
{
	// не активный корабль нельзя атаковать
	if( !IsActive() )
		return false;
	// в режиме редактора, или если не прошел трейс по боксу, то выходим сразу - не попали
	if (EditMode_IsOn() ||  
		!Box::OverlapsBoxLine(	Matrix().BuildPosition(content_.boundsCenter_)*GetTransform(Matrix()),
								content_.boundsSize_*0.5f, from, to) )
		return false;

	// сторона к которой принадлежит атакующий
	bool bObjIsShip = false;
	static const ConstString typeId("Ship");
	if( obj && obj->Is(typeId) )
	{
		// если атакующий на стороне этого корабля, то не считаем повреждения
		if( ((Ship*)obj)->GetParty() == GetParty() )
			return false;
		bObjIsShip = true;
	}

	// работа в режиме предварительной проверки попадания (только трейс, без повреждений)
	if( source == DamageReceiver::ds_check )
	{
		m_attackHandler.Init(from, to, Physics());
		AcceptVisitor(m_attackHandler);
		return m_attackHandler.HitDetected();
	}

	// создаем визитер для атаки
	m_attackHandler.Init(from, to, damage, Physics(), source == DamageReceiver::ds_flame);

	// ставим параметры визитера для стрельбы из шутера
	if( source == DamageReceiver::ds_shooter ) {
		// повреждение нескольких частей корпуса одним выстрелом
		m_attackHandler.SetChildRecursionLevel( content_.nDamageHullRecurseLevel );
		// попадание в парус не останавливает ядро
		m_attackHandler.SetSailsSkip(true);
	}

	// все корабли не могут повреждать паруса (если установлен соответствующий параметр)
	if( bObjIsShip && IsNoTeared() )
		m_attackHandler.SetNoTear(true);

	// пошли работать
	AcceptVisitor(m_attackHandler);
	hp_ -= m_attackHandler.TransferedDamage();

	// при попадании
	if( m_attackHandler.HitDetected() )
	{
		// дамаг заносим в статистику
		statistic.AddStatistic( ShipStatistics::stt_receivedDamage, m_attackHandler.TransferedDamage() );

		// расшвиривание человечков
		bool bThrowSailors = source == DamageReceiver::ds_cannon ||
							source == DamageReceiver::ds_shooter ||
							source == DamageReceiver::ds_bomb;
		for (unsigned int i = 0; i < littleMen_.Size(); ++i)
			littleMen_[i]->AttackBall( m_attackHandler.GetLastHitPos() );

		// повреждаем паруса от взрыва
		if( bObjIsShip && !IsNoTeared() )
			SailTearByExplosion( m_attackHandler.GetLastHitPos() );
	}

	// корабль потоплен?
	if (hp_ <= 0.0f && !IsBroken())
		Destruct( source == DamageReceiver::ds_flame ? DieType_flamethrower : DieType_cannon );

	return m_attackHandler.HitDetected();
}

//Воздействовать на объект выпуклым чехырёхугольником
bool Ship::Attack(MissionObject * obj, dword source, float damage, const Vector vrt[4])
{
	// не активный корабль нельзя атаковать
	if( !IsActive() )
		return false;

	if (EditMode_IsOn() || 
		!Box::OverlapsBoxPoly(	GetTransform(Matrix().BuildPosition(content_.boundsCenter_)),
								content_.boundsSize_, vrt) )
		return false;

	m_attackHandler.Init(vrt, damage, Physics());
	AcceptVisitor(m_attackHandler);
	hp_ -= m_attackHandler.TransferedDamage();

	if (hp_ <= 0.0f && !IsBroken())
		Destruct( DieType_unknown );

	return m_attackHandler.HitDetected();
}

void Ship::DrawGeometry()
{
	Matrix m;
	GetTransform(m);

	// погашенный корабль не показываем
	if( alpha_ <= 0.f ) return;

	float ship2powDist = ~(m.pos-Render().GetView().GetCamPos());
	IGMXScene* pModel = GetLodMainModel( ship2powDist );
	if( !pModel ) return;

	// определяем цвет с учетом гашения видимости по альфе от дальности
	Color col = m_cModelColor + content_.m_cModelColor;
	col.a = alpha_;

	// рисуем части (и обновляем их матрицы для отрисовки геометрии)
	DrawPart(m);

	// рисуем геометрию как анимационную модель
	pModel->SetUserColor( col );
	pModel->SetTransform(m);
	pModel->Draw();
}

void Ship::Activate(bool isActive)
{
	MissionObject::Activate(isActive);

	EnablePhysicActor(isActive && IsShow());
}

void Ship::Show(bool isShow)
{
	if( isShow && IsDead() && (!m_bShowWhileSink || releaseTimer_<=0.f) )
		isShow = false;

	bool bSeaRefl = isShow ? bSeaReflection_ : false;
	bool bSeaRefr = isShow ? bSeaRefraction_ : false;

	MissionObject::Show(isShow);

	DelUpdate(&Ship::Work);
	DelUpdate(&Ship::Draw);
	DelUpdate(&Ship::FrameEnd);
	if (isShow)
	{
		SetUpdate(&Ship::Work, ML_EXECUTE_END);
		SetUpdate(&Ship::Draw, ML_ALPHA4);
		SetUpdate(&Ship::FrameEnd, ML_LAST);
	}
	else
		DisableGUIProgressBar();


	if( bSeaRefl )
	{
		Registry(MG_SEAREFLECTION, &Ship::DrawRefl, ML_GEOMETRY5);
	}
	else
	{
		Unregistry(MG_SEAREFLECTION);
	}

	if(bSeaRefr)
	{
		Registry(MG_SEAREFRACTION, &Ship::DrawRefr, ML_GEOMETRY5);
	}else{
		Unregistry(MG_SEAREFRACTION);
	}

	EnablePhysicActor(isShow && IsActive());
}

void Ship::GetBox(Vector & min, Vector & max)
{
	min = content_.boundsCenter_ - 0.5f*content_.boundsSize_;
	max = content_.boundsCenter_ + 0.5f*content_.boundsSize_;
}

bool Ship::IsDead()
{
	return hp_ <= 0.0f;//
}

void _cdecl Ship::FrameEnd(float fDeltaTime, long level)
{
	m_bFrameUpdated = false;
}

void _cdecl Ship::Work(float fDeltaTime, long level)
{
	StartFrame();

	ProfileTimer timer;
	m_fLastDeltaTime = fDeltaTime;

	Matrix geoMtx, m;
	GetTransform(geoMtx);

	// сохраняем движение тонущего корабля
	if( IsDead() )
	{
		vPos_ += m_vCurSpeed * fDeltaTime;
		m_vCurSpeed *= (1.f - Clampf(0.25f*fDeltaTime));
		geoMtx.pos = vPos_;
	}

	if( m_pAniListener )
		m_pAniListener->Update( geoMtx, fDeltaTime );

	if( finder_ )
		finder_->SetMatrix(geoMtx);

	if( fDeltaTime > 0.f )
		m_bPause = false;

	if( !m_bPause )
		ShipPart::Work(geoMtx);

	if( fDeltaTime <= 0.f )
		m_bPause = true;

	fadeHelper_.Work(fDeltaTime);
	float peopleAlpha = alpha_ = 1.0f;
	if (fadeHelper_.IsActive())
	{
		if (!fadeHelper_.IsFadeFinished())
		{
			float fadeValue = fadeHelper_.GetFadeValue();
			Geometry().SetUserColor(Color(0.0f, 0.0f, 0.0f, fadeValue));
			alpha_ = peopleAlpha = fadeValue;
		}
		else
		{
			if (!fadeHelper_.IsFadeIn())
			{
				Geometry().SetUserColor(Color (0.0f, 0.0f, 0.0f, 0.0f));
				alpha_ = peopleAlpha = 0.0f;
				Show(false);
			}
			else
			{
				Geometry().SetUserColor(Color (0.0f, 0.0f, 0.0f, 1.0f));
				alpha_ = peopleAlpha = 1.0f;
			}
			fadeHelper_.SetActive(false);
		}
	}

	// подсчитаем альфу (гашение по дальности)
	float ship2powDist = ~(geoMtx.pos-Render().GetView().GetCamPos());
	if( ship2powDist > content_.fShipLodFadeDist * content_.fShipLodFadeDist )
	{
		float fDist = content_.fShipLodFadeDist + content_.fShipLodFadeLength;
		// расстояние больше того на котором мы еще что то видим - выходим ничего не рисуем
		if( content_.fShipLodFadeLength < 0.25f || ship2powDist >= fDist * fDist )
			alpha_ = 0.f;
		else
		{
			float fK = 1.f - (sqrtf(ship2powDist) - content_.fShipLodFadeDist) / content_.fShipLodFadeLength;
			alpha_ = Clampf(fK);
		}
	}

	timer.Start();

#ifdef SHIP_PROFILING
	timer.AddToCounter("Ship geometry");
#endif SHIP_PROFILING

	if( m_pAniBlender )
		m_pAniBlender->CreateWaterSplashParticles(content_.m_pcShipBoneWaterSplash, this);

	// отработаем огни на пушках
	m_WeaponLightManager.Update( fDeltaTime );

	if (IsDead())
	{
		releaseTimer_ -= fDeltaTime;

		if (releaseTimer_ <= 0.0f)
		{
			CleanAllBrokenSystems();

/*			Destroy();
			Activate(false);
			Show(false);*/
			MissionObject::Activate(false);
			MissionObject::Show(false);
			DelUpdate(&Ship::Work);
			DelUpdate(&Ship::Draw);
			DelUpdate(&Ship::FrameEnd);
			Unregistry(MG_SEAREFLECTION);
			Unregistry(MG_SEAREFRACTION);
		}
	}
	else
	{
		if (IsActive())
		{
			timer.Start();
			if (shipController_ && !EditMode_IsOn())
				shipController_->Work(fDeltaTime);
#ifdef SHIP_PROFILING
			timer.AddToCounter("Ship AI");
#endif

			timer.Start();
			Simulate(fDeltaTime);
#ifdef SHIP_PROFILING
			timer.AddToCounter("Ship Physic Model");
#endif
		}
	}

	//--------------------------------------------------------------
	// работа с звуковыми евентами
	if( !EditMode_IsOn() )
	{
		// обработка столкновения
		bool bTouch = false;
		ShipTouch::TouchData td;
		if( m_pTouch.Ptr() )
			bTouch = m_pTouch.Ptr()->CheckShip(this,td);

		// было столкновение и это первый удар
		if( rigidBody_ && bTouch && td.knock && !IsDead() )
		{
			Matrix mtxTouch;
			rigidBody_->GetGlobalTransform( td.shape, mtxTouch );
			Vector vTouchPos;
			rigidBody_->GetBox( td.shape, vTouchPos );
			vTouchPos = mtxTouch.MulVertex( Vector(0.f,vTouchPos.y,0.f) );

			// добавм дамаг
			hp_ -= td.part->HandleDamage( mtxTouch.pos, td.fDamage * td.part->GetTouchDamageFactor() );
			if (hp_ <= 0.0f && !IsBroken())
				Destruct( DieType_ram );

			// родим звук столкновения
			if( content_.pSound_[ShipContent::se_ship_knock] )
				Sound().Create3D(content_.pSound_[ShipContent::se_ship_knock], vTouchPos, _FL_);

			// родим партикл от столкновения
			if( ((HullPart*)td.part)->GetTouchSFX() )
			{
				Matrix mtxShipPos;
				GetTransform(mtxShipPos);
				CreateAutoParticle( ((HullPart*)td.part)->GetTouchSFX(), Matrix().BuildPosition(vTouchPos) );
			}
		}

		// запустим звук на уничтожение, горение и т.п. части корабля
		// уничтожение части корпуса
		if( (PartDestroyFlag() & pdf_destroy_hull) && content_.pSound_[ShipContent::se_hull_destroy] )
			Sound().Create3D(content_.pSound_[ShipContent::se_hull_destroy], vPos_, _FL_);
		// горение паруса
		if( content_.pSound_[ShipContent::se_sail_fire] )
		{
			if( PartDestroyFlag() & pdf_fire_sail )
			{
				if( m_pSoundFireSail )
					m_pSoundFireSail->SetPosition( vPos_ + Vector(0.f,10.f,0.f) );
				else
					m_pSoundFireSail = Sound().Create3D(content_.pSound_[ShipContent::se_sail_fire], vPos_ + Vector(0.f,10.f,0.f), __FILE__, __LINE__, true, false);
			}
			else
			{
				RELEASE( m_pSoundFireSail );
			}
		}
		// горение корпуса
		if( content_.pSound_[ShipContent::se_ship_fire] )
		{
			if( PartDestroyFlag() & pdf_fire_hull )
			{
				if( m_pSoundFireHull )
					m_pSoundFireHull->SetPosition( vPos_ + Vector(0.f,2.f,0.f) );
				else
					m_pSoundFireHull = Sound().Create3D(content_.pSound_[ShipContent::se_ship_fire], vPos_ + Vector(0.f,2.f,0.f), __FILE__, __LINE__, true, false);
			}
			else
			{
				RELEASE( m_pSoundFireHull );
			}
		}
		PartDestroyFlagReset();

		// звук движения корабля
		if( !m_pSoundMoving )
		{ // если нет звука, то смотрим, какой надо заводить и запускаем его с фейдом
			if( !IsBroken() )
			{
				if( realForwardMotion_ < 0.5f )
				{ // корабль стоит
					if( content_.pSound_[ShipContent::se_ship_not_moved] )
						m_pSoundMoving = Sound().Create3D( content_.pSound_[ShipContent::se_ship_not_moved], vPos_, __FILE__, __LINE__, true, false );
					m_bShipMoveCurrent = false;
				}
				else
				{ // корабль движется
					if( content_.pSound_[ShipContent::se_ship_moved] )
						m_pSoundMoving = Sound().Create3D( content_.pSound_[ShipContent::se_ship_moved], vPos_, __FILE__, __LINE__, true, false );
					m_bShipMoveCurrent = true;
				}
				// включаем плавное нарастание звука
				if( m_pSoundMoving )
				{
					m_fShipMoveFadeTime = 0.f;
					m_pSoundMoving->SetVolume( 0.f );
					m_bShipMoveFadeIn = true;
				}
			}
		}
		else
		{
			if( IsBroken() )
			{
				RELEASE( m_pSoundMoving );
			}
			else
			{
				// идет фейд
				if( m_fShipMoveFadeTime < content_.fShipMoveFadeTime_ )
				{
					m_fShipMoveFadeTime += fDeltaTime;
					if( m_fShipMoveFadeTime < content_.fShipMoveFadeTime_ )
					{
						if( m_bShipMoveFadeIn )
							m_pSoundMoving->SetVolume( m_fShipMoveFadeTime / content_.fShipMoveFadeTime_ );
						else
							m_pSoundMoving->SetVolume( 1.f - m_fShipMoveFadeTime / content_.fShipMoveFadeTime_ );
					}
					else
					{
						m_fShipMoveFadeTime = content_.fShipMoveFadeTime_;
						if( m_bShipMoveFadeIn )
						{
							m_pSoundMoving->SetVolume( 1.f );
							//m_fShipMoveSoundTime = content_.fShipMoveSoundPeriod_ * (1.f + FRAND(0.5f));
						}
						else
						{
							RELEASE( m_pSoundMoving );
						}
					}
				}
				else
				{
					if( realForwardMotion_ > 0.5f && !m_bShipMoveCurrent )
					{ // корабль движется, а раньше стоял - гасим текущий звук
						m_fShipMoveFadeTime = 0.f;
						m_bShipMoveFadeIn = false;
					}
					if( realForwardMotion_ < 0.5f && m_bShipMoveCurrent )
					{ // корабль стоит, а раньше двигался - гасим текущий звук
						m_fShipMoveFadeTime = 0.f;
						m_bShipMoveFadeIn = false;
					}
				}

				if( m_pSoundMoving )
					m_pSoundMoving->SetPosition(vPos_);
			}
		}

		// звук столкновения
		if( content_.pSound_[ShipContent::se_ship_touched] )
		{
			// проверять на столкновение будем только если звука нет или он уже отыгрался
			if( m_pSoundTouch && m_pSoundTouch->IsPlay() )
			{
				m_pSoundTouch->SetPosition( vPos_ );
			}
			else
			{
				// есть столкновение - тогда заводим звук
				if( bTouch )
				{
					if( m_pSoundTouch )
					{
						m_pSoundTouch->Play();
						m_pSoundTouch->SetPosition( vPos_ );
					}
					else
					{
						m_pSoundTouch = Sound().Create3D( content_.pSound_[ShipContent::se_ship_touched],
							vPos_, __FILE__, __LINE__, true, false );
					}
				}
				// нет столкновения - убиваем звук
				else
				{
					RELEASE( m_pSoundTouch );
				}
			}
		}
	}

	m_particescontainer.Update( Mission(), Render() );

	UpdateGUIProgressBar();

	statistic.Update( fDeltaTime );
}

void _cdecl Ship::Draw(float fDeltaTime, long level)
{
	// Enable swing machine?
	Matrix oldview(true);
	if(m_bNoSwing)
	{
		oldview = Render().GetView();
		Render().SetView((Mission().GetInverseSwingMatrix()*Matrix(oldview).Inverse()).Inverse());
	}

	Matrix geoMtx;
	GetTransform(geoMtx);

	DrawGeometry();

	// человечки
	for (unsigned int i = 0; i < littleMen_.Size(); ++i)
	{
		littleMen_[i]->SetUserAlpha(alpha_);
		littleMen_[i]->Work(geoMtx, Render().GetView().GetCamPos(), fDeltaTime);
	}
	Geometry().SetUserColor(Color (0.0f, 0.0f, 0.0f, 0.0f));

	// орудия
	if (!IsDead() && !IsBroken())
	{
		for (unsigned int i = 0; i < 4; ++i)
		{
			content_.sides_[i].FrameUpdate( fDeltaTime, geoMtx );
		}
	}

	if(m_bNoSwing)
	{
		Render().SetView(oldview);
	}
}

void _cdecl Ship::DebugInfoWork(float fDeltaTime, long level)
{
	Info_Show();
}

void _cdecl Ship::DrawRefl(GroupId, MissionObject*)
{
	Matrix m;
	GetTransform(m);

	float ship2powDist = ~(m.pos-Render().GetView().GetCamPos());
	IGMXScene* pModel = GetLodReflModel( ship2powDist );
	if( !pModel ) return;

	// определяем цвет с учетом гашения видимости по альфе от дальности
	Color col = m_cModelColor + content_.m_cModelColor;
	// проверяем расстояние до корабля что бы определить его видимость
	if( ship2powDist > content_.fShipLodFadeDist * content_.fShipLodFadeDist )
	{
		float fDist = content_.fShipLodFadeDist + content_.fShipLodFadeLength;
		// расстояние больше того на котором мы еще что то видим - выходим ничего не рисуем
		if( content_.fShipLodFadeLength < 0.25f || ship2powDist >= fDist * fDist )
			return;
		float fK = 1.f - (sqrtf(ship2powDist) - content_.fShipLodFadeDist) / content_.fShipLodFadeLength;
		col.a = Clampf(fK);
	}

	// рисуем геометрию как анимационную модель
	pModel->SetUserColor( col );
	pModel->SetTransform(m);
	pModel->Draw();
}

void _cdecl Ship::DrawRefr(GroupId, MissionObject*)
{
	Matrix m;
	GetTransform(m);

	float ship2powDist = ~(m.pos-Render().GetView().GetCamPos());
	IGMXScene* pModel = GetLodRefrModel( ship2powDist );
	if( !pModel ) return;

	// определяем цвет с учетом гашения видимости по альфе от дальности
	Color col = m_cModelColor + content_.m_cModelColor;
	// проверяем расстояние до корабля что бы определить его видимость
	if( ship2powDist > content_.fShipLodFadeDist * content_.fShipLodFadeDist )
	{
		float fDist = content_.fShipLodFadeDist + content_.fShipLodFadeLength;
		// расстояние больше того на котором мы еще что то видим - выходим ничего не рисуем
		if( content_.fShipLodFadeLength < 0.25f || ship2powDist >= fDist * fDist )
			return;
		float fK = 1.f - (sqrtf(ship2powDist) - content_.fShipLodFadeDist) / content_.fShipLodFadeLength;
		col.a = Clampf(fK);
	}

	// рисуем геометрию как анимационную модель
	pModel->SetUserColor( col );
	pModel->SetTransform(m);
	pModel->Draw();
}

Matrix& Ship::GetTransform(Matrix& mtx) const
{
	if( m_bFrameUpdated )
	{
		mtx = m_mtxTransform;
	}
	else
	{
		if (rigidBody_ && !((Ship*)this)->IsDead())
		{
			rigidBody_->GetTransform(mtx);
		}
		else
			mtx = Matrix (vAngles_, vPos_);

		// применяем трансформацию от внешнего объекта
		if( m_connectToName.NotEmpty() && !EditMode_IsOn() && m_bRelativeShipPos )
		{
			if( ((Ship*)this)->m_connectToObj.Validate() )
			{
				// трансформация на момент действий
				Matrix transf(true);
				mtx = m_mtxRelativeShipTransform * ((Ship*)this)->m_connectToObj.Ptr()->GetMatrix(transf);
			}
		}

		// меняем позицию и углы прописаные в корабле
		((Ship*)this)->m_mtxTransform = mtx;
		((Ship*)this)->vPos_ = mtx.pos;
		((Ship*)this)->vAngles_ = mtx.GetAngles();
		((Ship*)this)->m_bFrameUpdated = true;
	}

	return mtx;
}

void Ship::Command(const char * id, dword numParams, const char ** params)
{
	if( !id ) return;
	// воскрешение корабля (пересоздание)
	if (string::IsEqual(id, "respawn"))
	{
		if (!patternObj_)
		{
			LogicDebugError("Command ship::<respawn> error. No pattern is set. Can't respawn...");
			return;
		}

		// позиция воскрешения
		if(numParams >= 1)
		{
			MOSafePointer obj;
			FindObject(ConstString(params[0]), obj);
			if(obj.Ptr())
			{
				Matrix mtx;
				vPos_ = obj.Ptr()->GetMatrix(mtx).pos;
				vAngles_ = mtx.GetAngles();
			}
		}
		Respawn();

		if (!IsShow())
			Show(true);
		if (!IsActive())
			Activate(true);
	}
	else
	// изменить АИ корабля
	if (string::IsEqual(id, "changeAI"))
	{
		if(numParams < 1)
		{
			LogicDebugError("Command ship::<changeAI> error. New AI pattern name not present...");
			return;
		}
		IShipControlService * ctrlServ = (IShipControlService *)api->GetService(SHIP_CONTROLLER_SERVICE);
		if ( shipController_ )
			ctrlServ->ReleaseShipController(shipController_);
		shipController_ = ctrlServ->CreateShipController(ConstString(params[0]), *this, Mission());
	}
	else
	// стрелять по цели
	if (string::IsEqual(id, "shoot"))
	{
		if(numParams < 1)
		{
			LogicDebugError("Command ship::<shoot> error. Target not present...");
			return;
		}
		MOSafePointer target;
		FindObject(ConstString(params[0]), target);
		if (!target.Ptr())
		{
			LogicDebugError("Command ship::<shoot> error. Invalid target...");
			return;
		}

		FireCannons(target.Ptr(), 0.f);
	}
	else
	// сломать мачту
	if (string::IsEqual(id, "mastBreak"))
	{
		if( numParams < 1 )
		{
			LogicDebugError("Command ship::<mastBreak> error. Mast index not present...");
			return;
		}

		unsigned int mastIndex = (unsigned int)atoi(params[0]);
		m_partClassificator.Reset();
		AcceptVisitor(m_partClassificator);

		if ( mastIndex >= m_partClassificator.GetMasts().Size() )
		{
			LogicDebugError("Command ship::<mastBreak> error. Mast index is invalid...");
			return;
		}

		m_partClassificator.GetMasts()[mastIndex]->Break();
	}
	else
	if (string::IsEqual(id, "showpeople"))
	// "showpeople <group name> [fadein={0,1}] - показать людей в группе с опциональным фэйдом\n"
	{
		if (numParams < 1)
		{
			LogicDebugError("Command ship::<showpeople> error. Too few arguments");
			return;
		}

		bool useFade = false;
		if (numParams > 1)
			useFade = string::IsEqual("1", params[1]) || string::IsEqual("true", params[1]);

		for (unsigned int i = 0; i < littleMen_.Size(); ++i)
			littleMen_[i]->Hide(params[0], false, useFade);
		
		return;
	}
	else
	if (string::IsEqual(id, "hidepeople"))
	// "hidepeople <group name> [fadeout={0,1}] - скрыть людей в группе с опциональным фэйдом\n"
	{
		if (numParams < 1)
		{
			LogicDebugError("Command ship::<hidepeople> error. Too few arguments");
			return;
		}

		bool useFade = false;
		if (numParams > 1)
			useFade = string::IsEqual("1", params[1]) || string::IsEqual("true", params[1]);

		for (unsigned int i = 0; i < littleMen_.Size(); ++i)
			littleMen_[i]->Hide(params[0], true, useFade);

		return;
	}
	else
	if (string::IsEqual(id, "behaviour"))
	// "behaviour <group name> <behaviour name> - поменять поведение людей в группе",
	{
		if (numParams < 2)
		{
			LogicDebugError("Command ship::<behaviour> error. Too few arguments");
			return;
		}

		for (unsigned int i = 0; i < littleMen_.Size(); ++i)
			littleMen_[i]->SetBehaviour(params[0], params[1]);

		return;
	}
	else
	if (string::IsEqual(id, "setHP"))
	// setHP <HP> - set ship HP
	{
		if (numParams < 1)
		{
			LogicDebugError("Command ship::<setHP>. Too few agruments");
			return;
		}

		if ( IsDead() )
		{
			LogicDebugError("Command ship::<setHP>. Ship is dead. Operation has no effect.");
			return;
		}

		hp_ = (float)atof(params[0]);
		if (hp_ <= 0.0f && !IsBroken())
			Destruct( DieType_unknown );
		
		return;
	}
	
	else
	if (string::IsEqual(id, "changeHP"))
	// changeHP <HP>
	{
		if (numParams < 1)
		{
			LogicDebugError("Command ship::<changeHP>. Too few agruments");
			return;
		}

		if ( IsDead() )
		{
			LogicDebugError("Command ship::<changeHP>. Ship is dead. Operation has no effect.");
			return;
		}

		AddHP((float)atof(params[0]));
		if (hp_ <= 0.0f && !IsBroken())
			Destruct( DieType_unknown );

		return;
	}
	else
	if (string::IsEqual(id, "fadein"))
	// fadein [time] - fades ship in with optional fade time
	{
		if (IsShow())
			return;

		Show(true);

		fadeHelper_.SetActive(true);
		if (numParams > 0)
		{
			float time = (float)atof(params[0]);
			fadeHelper_.BeginFade(true, time);
		}
		else
			fadeHelper_.BeginFade(true);

		return;
	}
	else
	if (string::IsEqual(id, "fadeout"))
	// fadeout [time] - fades ship out with optional fade time
	{
		if (!IsShow())
			return;

		fadeHelper_.SetActive(true);
		if (numParams > 0)
		{
			float time = (float)atof(params[0]);
			fadeHelper_.BeginFade(false, time);
		}
		else
			fadeHelper_.BeginFade(false);

		return;
	}
	else
	if (string::IsEqual(id, "repair hull"))
	// repair hull - repair all hull parts
	{
		RepairHull();
	}
	else
	if (string::IsEqual(id, "repair sails"))
	// repair sails [particles] - repair all sails
	{
		if (numParams > 0)
			RepairSails(params[0]);
		else
			RepairSails("");

	}
}


//////////////////////////////////////////////////////////////////////////
// функции для бонусов
//////////////////////////////////////////////////////////////////////////


void Ship::RepairSails(const char* sfxName)
{
	m_partClassificator.Reset();
	AcceptVisitor(m_partClassificator);
	
	// восстанавливаем парусную систему
	unsigned int curMast = 0;
	unsigned int curMastPart = 0;
	unsigned int curYard = 0;
	unsigned int curSail = 0;

	for (unsigned int i = 0; i < content_.masts_.Size(); ++i)
	{
		Mast * mast = m_partClassificator.GetMasts()[curMast++];
		mast->Rebuild(content_.masts_[i]);

		for (unsigned int j = 0; j < content_.masts_[i].partsDesc_.Size(); ++j)
		{
			MastPart * part = m_partClassificator.GetMastParts()[curMastPart++];
			part->Rebuild(content_.masts_[i].partsDesc_[j]);
			
			for (unsigned int k = 0; k < content_.masts_[i].partsDesc_[j].yards_.Size(); ++k)
			{
				Yard * yard = m_partClassificator.GetYards()[curYard++];
				yard->Rebuild(content_.masts_[i].partsDesc_[j].yards_[k]);
			}
		}

		for (unsigned int j = 0; j < content_.masts_[i].sails_.Size(); ++j)
		{
			Sail * sail = m_partClassificator.GetSails()[curSail++];

			sail->Rebuild();
			CreateAutoParticle( sfxName, Matrix().BuildPosition(content_.masts_[i].sails_[j].pos) );
		}
	}
}

void Ship::RepairHull()
{
	m_partClassificator.Reset();
	AcceptVisitor(m_partClassificator);

	for (unsigned int i = 0; i < content_.hull_.Size(); ++i)
		m_partClassificator.GetHullParts()[i]->Rebuild(content_.hull_[i]);
}

void Ship::AddHP(float hp)
{
	hp_ += hp;
	hp_ = Min(hp_, startHP_);
}

void Ship::RunWeaponsInspector(IWeaponInspector& inspector)
{
	for (unsigned int i = 0; i < 4; ++i)
	{
		inspector.SetWeaponSide( &content_.sides_[i] );
		for (unsigned int j = 0; j < content_.sides_[i].GetWeaponCount(); ++j)
			content_.sides_[i].GetWeapon(j).GetWeapon()->AcceptVisitor(inspector);
	}
}

void Ship::EnablePhysicActor(bool bEnable)
{
	if( m_bIsActivePhysObject == bEnable )
		return;
	m_bIsActivePhysObject = bEnable;

	// применяем трансформацию от внешнего объекта
	if( m_connectToName.NotEmpty() )
	{
		// ищем объект для получения внешней трансформации
		if( !m_connectToObj.Validate() )
			if( EditMode_IsOn() )
				FindObject( m_connectToName, m_connectToObj );

		if( m_connectToObj.Ptr() )
		{
			// при включении - формируем новую позицию в соответствии с трансформацией и переносимся туда
			if( bEnable )
			{
				m_bRelativeShipPos = false;
				rigidBody_->SetTransform(Matrix(vAngles_,vPos_));
			}
			// при выключении - сохраняем относительную позицию, что бы восстановить настоящую после включения
			else
			{
				// трансформация на момент действий
				Matrix transf(true);
				m_connectToObj.Ptr()->GetMatrix(transf);
				m_mtxRelativeShipTransform = Matrix(vAngles_,vPos_) * transf.Inverse();
				m_bRelativeShipPos = true;
				api->Trace("ship connect to: pos(%.2f,%.2f) transf(%.2f,%.2f)", vPos_.x,vPos_.z, transf.pos.x,transf.pos.z);
			}
		}
	}

	if (rigidBody_)
	{
		rigidBody_->Activate(bEnable);
		rigidBody_->EnableCollision(bEnable);
	}

	ShipPart_EnablePhysicActor(bEnable);
}

void Ship::TieSailorsToHandholds()
{
	class HandholdVisitor : public IPartVisitor
	{
		array<LittleManImpl*> &	m_littleMen;
		array<SailorHandhold> & m_aSailorHandhold;

		void VisitChildren(ShipPart& part)
		{
			for (unsigned int i = 0; i < part.GetChildren().Size(); ++i )
				part.GetChildren()[i]->AcceptVisitor(*this);
		}

		void AddHandhold( long nSailorIdx, ShipPart* pPart )
		{
			unsigned int n;
			for(n = 0; n<m_aSailorHandhold; n++ )
				if( m_aSailorHandhold[n].pPart == pPart )
					break;
			if( n==m_aSailorHandhold )
			{
				n = m_aSailorHandhold.Add();
				m_aSailorHandhold[n].pPart = pPart;
			}
			m_aSailorHandhold[n].aSailorIndex.Add(nSailorIdx);
		}

		void CheckHandhold(ShipPart& part)
		{
			if( part.GetShapes().Size() > 0 )
			{
				for( long i=0; i<part.GetShapes(); i++ )
				{
					if( !part.GetShipOwner() || !part.GetShipOwner()->GetPhysView() )
						continue;
					Matrix mtxLoc(true);
					Vector vSize;
					part.GetShipOwner()->GetPhysView()->GetLocalTransform( part.GetShapes()[i], mtxLoc );
					part.GetShipOwner()->GetPhysView()->GetBox( part.GetShapes()[i], vSize );
					for( long n=0; n<m_littleMen; n++ )
					{
						const Vector& vSailorPos = m_littleMen[n]->GetLocalPos();
						Vector v = vSailorPos - mtxLoc.pos;
						if( v.x > -vSize.x && v.x < vSize.x   &&   v.z > -vSize.z && v.z < vSize.z )
						{
							if( v.y > -vSize.y  &&  v.y < vSize.y + 1.f )
							{
								AddHandhold( n, &part );
							}
						}
					}
				}
			}
		}

	public:
		HandholdVisitor(array<LittleManImpl*> &	littleMen, array<SailorHandhold> & aSailorHandhold) :
		  m_littleMen(littleMen),
		  m_aSailorHandhold(aSailorHandhold) {}

		virtual void Visit(ShipPart& part) {VisitChildren(part);}

		virtual void Visit(Ship& part) {VisitChildren(part);}
		virtual void Visit(HullPart& part)
		{
			CheckHandhold(part);
			VisitChildren(part);
		}
		virtual void Visit(MastPart& part)
		{
			CheckHandhold(part);
			VisitChildren(part);
		}
		virtual void Visit(Mast& part) {VisitChildren(part);}
		virtual void Visit(Yard& part)
		{
			CheckHandhold(part);
			VisitChildren(part);
		}
		virtual void Visit(Rope& part) {VisitChildren(part);}
		virtual void Visit(Sail& part) {VisitChildren(part);}

		virtual void Reset() {}
	};

	HandholdVisitor hhv(littleMen_, m_aSailorHandhold);
	AcceptVisitor(hhv);
}

void Ship::DropSailors(ShipPart* pPart)
{
	for( long n=0; n<m_aSailorHandhold; n++ )
		if( m_aSailorHandhold[n].pPart == pPart )
		{
			for( long i=0; i<m_aSailorHandhold[n].aSailorIndex; i++ )
				littleMen_[ m_aSailorHandhold[n].aSailorIndex[i] ]->Kill();
			return;
		}
}

void Ship::ReleaseModelLods()
{
	// убиваем блендер анимации
	DELETE(m_pAniBlender);
	// евенты анимации туда же
	if( m_pAniListener )
	{
		if( m_Lods.Size()>0 && m_Lods[0].pModelMain )
		{
			IAnimation* pAni = m_Lods[0].pModelMain->GetAnimation();
			if( pAni ) {
				m_pAniListener->DelAnimation(pAni);
				pAni->Release();
			}
		}
		delete m_pAniListener;
		m_pAniListener = NULL;
	}

	for( dword n=0; n<m_Lods.Size(); n++ )
	{
		RELEASE(m_Lods[n].pModelMain);
		RELEASE(m_Lods[n].pModelRefl);
		RELEASE(m_Lods[n].pModelRefr);
	}
	m_Lods.DelAll();
}

void Ship::CreateModelLods()
{
	if( content_.m_Lods.Size()==0 )
		return;

	Assert(m_Lods.Size()==0);

	// массив лодов
	m_Lods.AddElements( content_.m_Lods.Size() );

	// создаем модельки в массиве
	for( dword n=0; n<m_Lods.Size(); n++ )
	{
		// основная моделька
		m_Lods[n].pModelMain = string::IsEmpty( content_.m_Lods[n].mainModelName ) ?
			NULL :
			Geometry().CreateScene( content_.m_Lods[n].mainModelName, &Animation(), &Particles(), &Sound(), _FL_);
		if( m_Lods[n].pModelMain )
			m_Lods[n].pModelMain->SetDynamicLightState(true);

		// моделька отражения
		m_Lods[n].pModelRefl = string::IsEmpty( content_.m_Lods[n].reflModelName ) ?
			NULL :
			Geometry().CreateScene( content_.m_Lods[n].reflModelName, &Animation(), &Particles(), &Sound(), _FL_);
		if( m_Lods[n].pModelRefl )
			m_Lods[n].pModelRefl->SetDynamicLightState(true);

		// моделька переломления
		m_Lods[n].pModelRefr = string::IsEmpty( content_.m_Lods[n].refrModelName ) ?
			NULL :
			Geometry().CreateScene( content_.m_Lods[n].refrModelName, &Animation(), &Particles(), &Sound(), _FL_);
		if( m_Lods[n].pModelRefr )
			m_Lods[n].pModelRefr->SetDynamicLightState(true);
	}
}

void Ship::SetAnimationToLods(IAnimationTransform* pAni)
{
	// анимация для лодов

	for( dword n=0; n<m_Lods.Size(); n++ )
	{
		if( m_Lods[n].pModelMain )
			m_Lods[n].pModelMain->SetAnimation(pAni);

		// моделька отражения
		if( m_Lods[n].pModelRefl )
			m_Lods[n].pModelRefl->SetAnimation(pAni);

		// моделька переломления
		if( m_Lods[n].pModelRefr )
			m_Lods[n].pModelRefr->SetAnimation(pAni);
	}
}

void Ship::SailTearByExplosion( const Vector& pos )
{
	// визитор для атаки паруса множеством лучей
	class SailAttackHandler : public IPartVisitor
	{
		Vector vSrc;
		Vector avDst[26];

		// передать визитера всем детям
		void VisitChildren(ShipPart& part)
		{
			for (unsigned int i = 0; i < part.GetChildren().Size(); ++i)
				part.GetChildren()[i]->AcceptVisitor(*this);
		}

	public:
		// конструктор
		SailAttackHandler(const Vector& pos,float radius)
		{
			vSrc = pos;
			for( dword n=0; n<26; n++ )
				avDst[n] = pos;

			// лучи вдоль каждой оси
			avDst[0].x += radius;
			avDst[1].x -= radius;
			avDst[2].y += radius;
			avDst[3].y -= radius;
			avDst[4].z += radius;
			avDst[5].z -= radius;

			// лучи между парой осей
			radius *= (1.f/1.41f);
			avDst[6].x += radius; avDst[6].z += radius;
			avDst[7].x += radius; avDst[7].z -= radius;
			avDst[8].x -= radius; avDst[8].z += radius;
			avDst[9].x -= radius; avDst[9].z -= radius;
			avDst[10].x += radius; avDst[10].y += radius;
			avDst[11].x += radius; avDst[11].y -= radius;
			avDst[12].x -= radius; avDst[12].y += radius;
			avDst[13].x -= radius; avDst[13].y -= radius;
			avDst[14].z += radius; avDst[14].y += radius;
			avDst[15].z += radius; avDst[15].y -= radius;
			avDst[16].z -= radius; avDst[16].y += radius;
			avDst[17].z -= radius; avDst[17].y -= radius;

			// лучи между всеми тремя осями
			radius *= (1.f/1.41f);
			avDst[18].x += radius; avDst[18].y += radius; avDst[18].z += radius;
			avDst[19].x += radius; avDst[19].y += radius; avDst[19].z -= radius;
			avDst[20].x += radius; avDst[20].y -= radius; avDst[20].z += radius;
			avDst[21].x += radius; avDst[21].y -= radius; avDst[21].z -= radius;
			avDst[22].x -= radius; avDst[22].y += radius; avDst[22].z += radius;
			avDst[23].x -= radius; avDst[23].y += radius; avDst[23].z -= radius;
			avDst[24].x -= radius; avDst[24].y -= radius; avDst[24].z += radius;
			avDst[25].x -= radius; avDst[25].y -= radius; avDst[25].z -= radius;
		}
		// деструктор
		~SailAttackHandler() {}

		// все части только передают детям визитера
		virtual void Visit(ShipPart& part) {VisitChildren(part);}
		virtual void Visit(Ship& part) {VisitChildren(part);}
		virtual void Visit(HullPart& part) {VisitChildren(part);}
		virtual void Visit(MastPart& part) {VisitChildren(part);}
		virtual void Visit(Mast& part) {VisitChildren(part);}
		virtual void Visit(Yard& part) {VisitChildren(part);}
		virtual void Visit(Rope& part) {}

		// парус трейсится
		virtual void Visit(Sail& part)
		{
			part.Attack(vSrc,10.f);
		}

		virtual void Reset() {}
	};

	SailAttackHandler sailTear(pos,10.f);
	AcceptVisitor(sailTear);
}

void Ship::UpdateGUIProgressBar()
{
	// мертвому  или невидимому не нужен прогресс бар
	if( IsDead() || !IsShow() )
	{
		if( m_GUIProgress.Ptr() )
			DisableGUIProgressBar();
		return;
	}

	// проверка/получение указателя на объект прогресс бара
	if ( !m_GUIProgress.Validate() )
	{
		if( m_pcGUIProgressName.NotEmpty() )
		{
			FindObject(m_pcGUIProgressName, m_GUIProgress);
			static const ConstString typeId("IGUIElement");
			if( !m_GUIProgress.Ptr() || !m_GUIProgress.Ptr()->Is(typeId) )
				m_GUIProgress.Reset();
			if( m_GUIProgress.Ptr() )
				((IGUIElement*)m_GUIProgress.Ptr())->GetSize( m_fGUIProgressWidth, m_fGUIProgressHeight );
		}
	}

	// ставим прогресс бар в позицию
	if( m_GUIProgress.Ptr() )
	{
		if( !m_GUIProgress.Ptr()->IsShow() )
		{
			((IGUIElement*) m_GUIProgress.Ptr())->SetAlpha( 1.f );
			m_GUIProgress.Ptr()->Show( true );
		}

		float x,y, w,h, k, alph;

		// дистанция
		float fDist = (GetPosition() - Render().GetView().GetCamPos()).GetLength();
		// позиция
		Matrix mVP(Render().GetView(), Render().GetProjection());
		Vector4 v = mVP.Projection( GetPosition() + Vector (0,m_fGUIProgressUpper,0), Render().GetViewport().Width * 0.5f, Render().GetViewport().Height * 0.5f );

		if( fDist > m_fGUIProgressMaxShowDistance || v.w <= 0.0f )
		{
			x = y = 0.5f;
			w = m_fGUIProgressWidth;
			h = m_fGUIProgressHeight;
			alph = 0.f;
		}
		else
		{
			// коэффициент масштабирования от дальности
			k = Clampf( fDist / m_fGUIProgressMaxShowDistance );
			k = Lerp( 1.f, m_fGUIProgressMinimizeK, k );
			// ширина и высота
			w = m_fGUIProgressWidth * k;
			h = m_fGUIProgressHeight * k;

			// расчет позиции
			x = v.x / Render().GetViewport().Width - w*0.5f;
			y = v.y / Render().GetViewport().Height + h*0.5f;

			if( x<=-w || x>=1.f || y<=-h || y>=1.f )
				alph = 0.f;
			else
				alph = 1.f;
		}

		((IGUIElement*)m_GUIProgress.Ptr())->SetPosition( x, y );
		((IGUIElement*)m_GUIProgress.Ptr())->SetSize( w, h );
		((IGUIElement*)m_GUIProgress.Ptr())->SetAlpha( alph );
	}
}

void Ship::DisableGUIProgressBar()
{
	if( m_GUIProgress.Validate() )
	{
		((IGUIElement*)m_GUIProgress.Ptr())->SetSize( m_fGUIProgressWidth, m_fGUIProgressHeight );
		((IGUIElement*)m_GUIProgress.Ptr())->SetAlpha( 0.f );
		((IGUIElement*)m_GUIProgress.Ptr())->Show( false );
		m_GUIProgress.Reset();
	}
}

void Ship::GetLodModels(long lodNum, IGMXScene* &pModelMain, IGMXScene* &pModelRefl, IGMXScene* &pModelRefr)
{
	if( lodNum<0 || lodNum >= m_Lods )
	{
		pModelMain = pModelRefl = pModelRefr = NULL;
		return;
	}
	pModelMain = m_Lods[lodNum].pModelMain;
	pModelRefl = m_Lods[lodNum].pModelRefl;
	pModelRefr = m_Lods[lodNum].pModelRefr;
}

// получить модельку лода для заданной дистанции (в квадрате)
IGMXScene* Ship::GetLodMainModel( float fDist2Pow )
{
	IGMXScene* pRetModel = NULL;
	for( dword n=0; n<content_.m_Lods.Size() && n<m_Lods.Size(); n++ )
	{
		if( content_.m_Lods[n].fDist2Pow > fDist2Pow )
			break;
		pRetModel = m_Lods[n].pModelMain;
	}
	return pRetModel;
}

// получить модельку лода отражения для заданной дистанции (в квадрате)
IGMXScene* Ship::GetLodReflModel( float fDist2Pow )
{
	IGMXScene* pRetModel = NULL;
	for( dword n=0; n<content_.m_Lods.Size() && n<m_Lods.Size(); n++ )
	{
		if( content_.m_Lods[n].fDist2Pow > fDist2Pow )
			break;
		pRetModel = m_Lods[n].pModelRefl;
	}
	return pRetModel;
}

// получить модельку лода переломления для заданной дистанции (в квадрате)
IGMXScene* Ship::GetLodRefrModel( float fDist2Pow )
{
	IGMXScene* pRetModel = NULL;
	for( dword n=0; n<content_.m_Lods.Size() && n<m_Lods.Size(); n++ )
	{
		if( content_.m_Lods[n].fDist2Pow > fDist2Pow )
			break;
		pRetModel = m_Lods[n].pModelRefr;
	}
	return pRetModel;
}

void Ship::Respawn()
{
	// Убъем все прежние партиклы
	m_particescontainer.ReleaseAll();

	if( m_bNotCreated )
		CopyPattern();
	else
	{
		// очищаем евенты анимации от партиклов, звуков и т.п.
		if( m_pAniListener )
			m_pAniListener->Stop();

		// перестраиваем все части
		TreeRebuild();
		rigidBody_->Activate(true);

		// восстановление человечков
		for (unsigned int i = 0; i < littleMen_.Size(); ++i)
			delete littleMen_[i];
		littleMen_.DelAll();
		for (unsigned int i = 0; i < content_.littleMen_.Size(); ++i)
		{
			if( content_.littleMen_[i].patternName.IsEmpty() ) continue;
			LittleManImpl * man = NEW LittleManImpl(this, content_.littleMen_[i], Mission());
			littleMen_.Add(man);
		}
		TieSailorsToHandholds();
	}

	if( rigidBody_ )
		rigidBody_->SetTransform( Matrix(vAngles_,vPos_) );

	// подсчитаем относительную позицию (что бы в самом начеле корабль мог быть привязан к треку)
	if( m_connectToObj.Validate() )
	{
		m_mtxRelativeShipTransform = m_mtxInitRelativeShipTransform;
		//Matrix transf(true);
		//m_connectToObj.Ptr()->GetMatrix(transf);
		//m_mtxRelativeShipTransform = Matrix(vAngles_,vPos_) * transf.Inverse();
		//m_bRelativeShipPos = true;
		m_bRelativeShipPos = !IsActive() || !IsShow();
	}
	else
		m_bRelativeShipPos = false;


	// пересоздаем АИ (он мог быть изменен)
	IShipControlService * ctrlServ = (IShipControlService *)api->GetService(SHIP_CONTROLLER_SERVICE);
	if ( shipController_ )
		ctrlServ->ReleaseShipController(shipController_);
	shipController_ = ctrlServ->CreateShipController(ctrlParamsObjectName_, *this, Mission());

	// восстанавливаем альфу (мог быть погашен после смерти)
	alpha_ = 1.0f;

	// восстанавливаем финдер (мог быть удален после смерти)
	if( !finder_ )
	{
		finder_ = QTCreateObject(MG_DAMAGEACCEPTOR, _FL_);
		if( finder_ )
		{
			finder_->SetBox(content_.boundsCenter_-content_.boundsSize_*0.5f,
							content_.boundsCenter_+content_.boundsSize_*0.5f);
			finder_->Activate(true);
		}
	}

	// восстанавливаем жизнь
	hp_ = startHP_;

	// часть не разбита
	SetBroken(false);

	// количество мин на борту
	m_nMineQuantity = m_nMineQuantityMax;

	// отключаем хелс бары
	DisableGUIProgressBar();

	// рестартим анимацию
	if( m_pAniDestruct )
		m_pAniDestruct->Start();

	// рестарт статистики
	statistic.ResetStatistics();

	// убиваем все партиклы, которые были на корабле
	m_particescontainer.ReleaseAll();
}

// debug section
void Ship::Info_SetMask(dword infoFlag)
{
	m_debugInfoData.infoSwitch |= infoFlag;
}
void Ship::Info_ResetMask(dword infoFlag)
{
	m_debugInfoData.infoSwitch &= ~infoFlag;
}
void Ship::Info_SwitchMask(dword infoFlag)
{
	m_debugInfoData.infoSwitch ^= infoFlag;
}
bool Ship::Info_GetMask(dword infoFlag)
{
	return (m_debugInfoData.infoSwitch & infoFlag) != 0;
}

void Ship::Info_Show()
{
	// общая инфа о корабле
	if( Info_GetMask(sdim_ship_info) )
	{
		Matrix geoMtx(true);
		GetTransform(geoMtx);
		Render().Print( geoMtx.pos + Vector(0.f,30.f,0.f), 0.0f, 0.0f, 0xFFFFFFFF,
			"Name: %s\n"
			"Pattern: %s\n"
			"Active: %s\n"
			"HP: %0.2f",
			GetObjectID().c_str(),
			patternName_.c_str(),
			IsActive() ? "true" : "false",
			GetHP() );
	}

	// инфа о физической форме корабля (боксы частей корабля)
	if( Info_GetMask(sdim_phys_form) )
	{
		// физические шейпы в основном теле
		Vector boxsize;
		Matrix boxmtx(true);
		long nq = rigidBody_->GetCount();
		for( long nn=0; nn<nq; nn++ )
		{
			rigidBody_->GetBox(nn,boxsize);
			rigidBody_->GetGlobalTransform(nn, boxmtx);
			Render().DrawBox(-boxsize, boxsize, boxmtx, 0xFFFFFFFF);
		}

		// показ информации о коллижене
		// группа коллижена
		const char* pcPhysCollisionGroup = "unknown";
		PhysicsCollisionGroup grp = rigidBody_->GetGroup();
		switch( grp )
		{
		case phys_world: pcPhysCollisionGroup = "world"; break;
		case phys_ragdoll: pcPhysCollisionGroup = "ragdoll"; break;
		case phys_character: pcPhysCollisionGroup = "character"; break;
		case phys_player: pcPhysCollisionGroup = "player"; break;
		case phys_particles: pcPhysCollisionGroup = "particles"; break;
		case phys_pair: pcPhysCollisionGroup = "pair"; break;
		case phys_ship: pcPhysCollisionGroup = "ship"; break;
		case phys_grp1: pcPhysCollisionGroup = "grp1"; break;
		case phys_grp2: pcPhysCollisionGroup = "grp2"; break;
		case phys_grp3: pcPhysCollisionGroup = "grp3"; break;
		case phys_grp4: pcPhysCollisionGroup = "grp4"; break;
		case phys_nocollision: pcPhysCollisionGroup = "nocollision"; break;
		case phys_playerctrl: pcPhysCollisionGroup = "playerctrl"; break;
		case phys_physobjects: pcPhysCollisionGroup = "physobjects"; break;
		case phys_charitems: pcPhysCollisionGroup = "charitems"; break;
		case phys_bloodpatch: pcPhysCollisionGroup = "bloodpatch"; break;
		case phys_enemy: pcPhysCollisionGroup = "enemy"; break;
		case phys_ally: pcPhysCollisionGroup = "ally"; break;
		case phys_boss: pcPhysCollisionGroup = "boss"; break;
		case phys_grass: pcPhysCollisionGroup = "grass"; break;
		}
		// материал с которым была коллизия
		dword dwCollisionMat = rigidBody_->GetContactReport();
		const char* pcMaterial = SeaMissionParams::GetMaterialName(dwCollisionMat);
		if( !pcMaterial )
			pcMaterial = "no collision";
		else
		{ // вывод точки коллизии
			Vector vCollisionPoint = rigidBody_->GetContactPoint();
			Render().DrawLine( vCollisionPoint, 0xFFFF0000, vCollisionPoint+Vector(0.f,10.f,0.f), 0xFFFF0000 );
			Render().DrawSphere( vCollisionPoint, 0.2f, 0xFFFF0000 );
		}
		// вывод
		Matrix geoMtx(true);
		GetTransform(geoMtx);
		Render().Print( geoMtx.pos + Vector(0.f,45.f,0.f), 0.0f, 0.0f, 0xFFFFFFFF,
			"Collision group: %s\n"
			"Collision material: %s", pcPhysCollisionGroup, pcMaterial );

		// показ отвалившихся частей
		DebugShowPhysForm();
	}

	// инфа об оружии
	if( Info_GetMask(sdim_cannon_info) )
	{
		for( long i=0; i<4; i++ )
		{
			dword q = content_.sides_[i].GetWeaponCount();
			for( dword n=0; n<q; n++ )
			{
				Weapon * pW = content_.sides_[i].GetWeapon(n).GetWeapon();
				if( !pW ) continue;
				pW->DrawDebugInfo();
			}
		}

		Render().SetWorld(Matrix());
		for( i=0; i<m_debugInfoData.cannonFire; i++ )
		{
			float angle;
			if( Ballistics::ComputeShootAngle(
				m_debugInfoData.cannonFire[i].from, m_debugInfoData.cannonFire[i].to,
				m_debugInfoData.cannonFire[i].speed, angle ) )
			{
				float dist = (m_debugInfoData.cannonFire[i].from - m_debugInfoData.cannonFire[i].to).GetLengthXZ();
				float time = Ballistics::ComputeFlightTime(dist,angle,m_debugInfoData.cannonFire[i].speed);
				Vector vPrevPos = m_debugInfoData.cannonFire[i].from;
				for( long k=1; k<=10; k++)
				{
					Vector vCurPos;
					Ballistics::ComputePosition( m_debugInfoData.cannonFire[i].from,
						m_debugInfoData.cannonFire[i].to,
						-9.8f, m_debugInfoData.cannonFire[i].speed, angle, k*0.1f*time, vCurPos );
					Render().DrawLine( vPrevPos, 0xFF00FF00, vCurPos, 0xFF00FF00 );
					vPrevPos = vCurPos;
				}
			}
			else
			{
				Render().DrawLine( m_debugInfoData.cannonFire[i].from, 0xFFFF0000, m_debugInfoData.cannonFire[i].to, 0xFFFF0000 );
			}
		}
	}

	// инфа об интеллекте
	if( Info_GetMask(sdim_ai_info) )
	{
		if( shipController_ )
			shipController_->ViewDebugInfo();
		else
		{
			Matrix geoMtx(true);
			GetTransform(geoMtx);
			Render().Print( geoMtx.pos + Vector(0.f,60.f,0.f), 0.0f, 0.0f, 0xFFFFFFFF, "AI not set" );
		}
	}
}



static char ShipDescription[] =
"-= Ship object for sea missions =-\n"
"Supported commands:\n"
"respawn [locator] - respawn a ship at specified locator or at its current position\n"
"changeAI <AI params object> - set new ship AI\n"
"shoot <target> - all weapons shoot to target\n"
"mastBreak <mast index={0...}> - break mast at specified index\n"
"showpeople <group name> [fadein={0,1}] - show people in group with optional fade\n"
"hidepeople <group name> [fadeout={0,1}] - hide people in group with optional fade\n"
"behaviour <group name> <behaviour name> - change behaviour of the people in group\n"
"setHP <HP> - set ship HP\n"
"changeHP <HP> - add/subtract ship HP\n"
"fadein [time] - fade ship in with optional fade time\n"
"fadeout [time] - fade ship out with optional fade time\n"

"Commands for bonuses:\n"
"repair hull - repair all hull parts\n"
"repair sails [particles] - repair all sails\n"
"megaballs <model, damageScale> - change cannon balls model and damage";

MOP_BEGINLISTCG(Ship, "Ship", '1.00', 101, ShipDescription, "Arcade Sea");
	MOP_STRING("Ship pattern name", "")
	MOP_POSITION("Position", Vector(0.0f))
	MOP_ANGLESEX("Orientation", Vector(0.0f), Vector(0.0f, 0.0f, 0.0f), Vector(0.0f, 360.0f, 0.0f))
	MOP_FLOAT("Max linear speed", 100.0f)
	MOP_FLOAT("Min linear speed", 50.0f)
	MOP_FLOAT("Max rotate speed", 300.0f)
	MOP_FLOAT("HP", 100.0f)
	MOP_STRING("AI params object", "")
	MOP_STRINGC("ConnectTo", "", "Connect to mission object for transform position while inactive or show off state of ship")
	MOP_GROUPBEG("ProgressBar")
		MOP_STRING("Object ID", "")
		MOP_FLOAT("Upper value", 40.f)
		MOP_FLOATEX("Minimize factor", 0.2f, 0.f, 1.f)
		MOP_FLOAT("Max show distance", 1500.f)
	MOP_GROUPEND()
	MOP_LONG("Mine quantity", -1)
	MOP_MISSIONTRIGGER("Mine be over trigger")

	MOP_ENUMBEG("ShipSide")
		MOP_ENUMELEMENT("Enemy")
		MOP_ENUMELEMENT("Neutral")
		MOP_ENUMELEMENT("Friend")
		MOP_ENUMELEMENT("Player1")
		MOP_ENUMELEMENT("Player2")
		MOP_ENUMELEMENT("Player3")
		MOP_ENUMELEMENT("Player4")
	MOP_ENUMEND
	MOP_ENUM("ShipSide", "Side")
	MOP_BOOLC("NoTearSail",true,"Only player can teared sails")

	MOP_COLOR("Model Color", Color(0.0f, 0.0f, 0.0f, 1.0f))

	MOP_BOOLC("CannonVolley",false,"Shoot from cannon only volley")

	MOP_BOOLC("No swing", true, "No swing ship in swing machine")
	MOP_BOOL("Sea reflection", true)
	MOP_BOOL("Sea refraction", true)
	MOP_BOOL("Fake sails", true)
	MOP_BOOL("No make physics sails", false)
	MOP_BOOL("Show while sink", true)
	MOP_BOOL("Visible", true)
	MOP_BOOL("Active", true)
MOP_ENDLIST(Ship)