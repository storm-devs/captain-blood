//////////////////////////////////////////////////////////////////////////
// Корабль, лодка
//////////////////////////////////////////////////////////////////////////

#ifndef _Ship_h_
#define _Ship_h_

#include "..\..\Common_h\Mission.h"

#include "..\..\Common_h\IShip.h"
#include "..\..\Common_h\IShipControlService.h"
#include "..\..\Common_h\IGUIElement.h"
#include "..\..\Common_h\Achievements.h"

#include "ShipPart.h"
#include "ShipPattern.h"
#include "LittleManImpl.h"
#include "ShipAni.h"
#include "ShipStatistics.h"
#include "PartsClassificator.h"
#include "AttackHandler.h"


class IClothBurns;
class ShipTouch;
class AnimationStdEvents;
class Ship;


// менеджер плавного появления/изчезания корабля (команды fadein/fadeout)
//-------------------------------------------------------------------------
class FadeHelper
{
	bool m_isFadeIn;
	float m_curFadeTime;
	float m_FadeTime;
	bool m_Active;

public:
	FadeHelper() : m_curFadeTime(0.0f), m_FadeTime(0.0f), m_isFadeIn(true), m_Active(false) {}
	void BeginFade(bool isFadeIn, float fadeTime = 2.0f) { m_isFadeIn = isFadeIn; m_FadeTime = m_curFadeTime = fadeTime; }
	void Work(float deltaTime)
	{
		if (IsFadeFinished())
			return;

		m_curFadeTime -= deltaTime;
	}
	bool IsFadeFinished() const { return m_curFadeTime <= 0.0f; }
	bool IsFadeIn() const { return m_isFadeIn; }
	void SetActive(bool isActive) { m_Active = isActive; }
	bool IsActive() const { return m_Active; }

	float GetFadeValue() const { return m_isFadeIn ? (1.0f-m_curFadeTime / m_FadeTime) : m_curFadeTime / m_FadeTime; }
};


// менеджер управления партиклами корабля (учет swing  матрицы, исчезновение вместе с краблем)
//---------------------------------------------------------------------------------------------
class ShipParticlesContainer
{
	struct ParticlDescr
	{
		Matrix mtx;
		IParticleSystem* ps;
	};
	array<ParticlDescr> m_aParticles;
	bool m_bSwing;
public:
	ShipParticlesContainer() : m_aParticles(_FL_) {m_bSwing=false;}
	~ShipParticlesContainer()
	{
		ReleaseAll();
	}

	void ReleaseAll()
	{
		for(long n=0; n<m_aParticles; n++) { RELEASE(m_aParticles[n].ps); }
		m_aParticles.DelAll();
	}

	void TurnOnSwing(bool bf) {m_bSwing=bf;}

	void Update( IMission& mis, IRender& render )
	{
		for(long n=0; n<m_aParticles; n++)
			if( m_aParticles[n].ps->IsAlive() )
			{
				m_aParticles[n].ps->CancelHide();
				if( m_bSwing )
				{
					//Видовая матрица с включёной свинг матрицей
					Matrix view = render.GetView();
					//Чистая видовая матрица
					Matrix realView = mis.GetInverseSwingMatrix()*Matrix(view).Inverse();
					realView.Inverse();
					//Разносная матрица
					Matrix diff = Matrix(Matrix(realView), Matrix(view).Inverse());
					m_aParticles[n].ps->SetTransform( Matrix().EqMultiply(m_aParticles[n].mtx, diff) );
				}
			}
			else
			{
				RELEASE( m_aParticles[n].ps );
				m_aParticles.DelIndex( n );
				n--;
			}
	}
	void AddParticle(IParticleManager& pm, const char* pcName, const Matrix& mtx)
	{
		IParticleSystem* ps = pm.CreateParticleSystemEx2( pcName, mtx, false, _FL_ );
		if( ps ) {
			ps->AutoHide( true );
			dword n = m_aParticles.Add();
			m_aParticles[n].ps = ps;
			m_aParticles[n].mtx = mtx;
		}
	}
};


// менеджер источников света для подсветки стрельбы из пушек
//------------------------------------------------------------
class WeaponLightManager
{
	struct WeaponLight
	{
		MOSafePointer obj;
		Weapon* pWeapon;
		float fTime;
		float fLifeTime;
		Vector pos;
	};
	array<WeaponLight> m_aWeaponLights;
	float m_fExcludeRadius;
	float m_fOffset;

public:
	WeaponLightManager() : m_aWeaponLights(_FL_) {m_fExcludeRadius=0.f; m_fOffset=0.f;}
	~WeaponLightManager() {m_aWeaponLights.DelAll();}

	void SetExcludeRadius(float fExcludeRadius) {m_fExcludeRadius = fExcludeRadius;}
	void SetLightOffset(float fOffset) {m_fOffset = fOffset;}
	void SetLightObjects( MissionObject* mo, array<ShipContent::WeaponLight> & aLights );
	void Update(float fDeltaTime);
	void AddLight(Weapon* pWeapon);

protected:
	void SetPos( long n, const Vector& pos );
};


// хозяин паруса (класс управления парусом)
//-------------------------------------------
class ShipSailOwner : public SailOwner
{
	Ship* m_pShip;
public:
	ShipSailOwner() {m_pShip=NULL;}

	void SetShip(Ship* pShip) {m_pShip=pShip;}

	virtual IPhysCombined* GetPhysView();
	virtual IPhysCloth* CreateCloth(const char* file, long line, IClothRenderInfo & renderInfo, IClothMeshBuilder & builder, IPhysCloth::SimulationData & simData);
	virtual bool EditMode_IsOn();
	virtual Matrix& GetMatrix(Matrix& mtx);
	virtual float GetAlpha();
	virtual float GetLastDeltaTime();
	virtual IClothBurns* GetClothBurns();
	virtual void PartDestroyFlagSet(DWORD dwFlag);
	virtual const char* GetSoundNameForEvent(dword dwIdx);
	virtual void AcceptVisitor(IPartVisitor& clsf);
	virtual bool TieToPhysicBody(Sail* pSail,bool bTie,bool bTop) {return false;}
};


// корабль/лодка
//-------------------------------------------
class Ship : public IShip, public ShipPart 
{
public:
	// описание опоры для матроса
	struct SailorHandhold
	{
		ShipPart* pPart;
		array<long> aSailorIndex;

		SailorHandhold() : aSailorIndex(_FL_) {}
	};

	// флаги разрушения частей корабля
	enum PartDestroyFlags
	{
		pdf_destroy_none = 0,

		pdf_destroy_hull = 1,
		pdf_destroy_mast = 2,
		pdf_destroy_mastpart = 4,
		pdf_destroy_yard = 8,
		pdf_destroy_sail = 16,

		pdf_fire_sail = 32,
		pdf_fire_hull = 64
	};

	// набор лодов
	struct LodModels
	{
		IGMXScene* pModelMain; // основная модель
		IGMXScene* pModelRefl; // модель для отражения
		IGMXScene* pModelRefr; // модель для переломления
	};

protected:
	// шаблон корабля
	ShipContent				content_;				// шаблонные параметры корабля
	ConstString				patternName_;			// имя шаблона
	ShipPattern*			patternObj_;			// шаблон

	// геометрия корабля
	array<LodModels>		m_Lods;					// массив лодов корабля
	IAnimation*				m_pAniDestruct;			// анимация разрушения корабля
	Color					m_cModelColor;			// цвет корабля
	float					alpha_;					// текущее значение прозрачности
	bool					m_bNoSwing;				// не свинговать
	bool					bSeaReflection_;		// отрисовывать в отражении или нет
	bool					bSeaRefraction_;		// отрисовать в переломлении или нет

	// анимация
	const char*				m_pcBreakAnimation;		// анимация взрыва корабля
	ShipAniBlendStage*		m_pAniBlender;			// блендер анимации корабля (между взрывом и физическим разлетом частей)
	AnimationStdEvents*		m_pAniListener;			// обработчик евентов анимации корабля (всплески падения частей, партиклы)

	// менеджирование
	ShipSailOwner			m_SailOwner;			// хозяин парусов
	IMissionQTObject *		finder_;				// объект поиска QT
	MOSafePointerType<IClothBurns>	m_pCBurns;		// объект горения парусов
	MOSafePointerType<ShipTouch>	m_pTouch;		// объект обработки столкновений
	ShipParticlesContainer	m_particescontainer;	// набор самоудаляемых партиклов
	FadeHelper				fadeHelper_;			// фейдер для плавного появления исчезновения по команде
	WeaponLightManager		m_WeaponLightManager;	// менеджер источников света от выстрела корабельных пушек
	PartsClassificator		m_partClassificator;	// классификатор частей корабля
	AttackHandler			m_attackHandler;		// классификатор частей корабля для распределения дамага

	// прогресс бар, показывающий жизнь у корабля
	ConstString				m_pcGUIProgressName;	// имя прогресс бара
	MOSafePointer			m_GUIProgress;
	float					m_fGUIProgressUpper;	// высота поднятия прогресс бара над кораблем
	float					m_fGUIProgressMinimizeK;
	float					m_fGUIProgressMaxShowDistance;
	float					m_fGUIProgressWidth;
	float					m_fGUIProgressHeight;

	// параметры движения корабля
	float					fMaxSpeed_;				// максимальная линейная скорость
	float					fMinSpeed_;				// минимальная линейная скорость (все паруса повреждены)
	float					fMaxRotateSpeed_;		// максимальная скорость поворота
	float					rotationMotion_;		// кол-во поворота ([-1.0f .. 1.0f])
	float					forwardMotion_;			// кол-во движения ([0.0f .. 1.0f])
	float					offsetMotion_;			// кол-во смещения ([0.0f .. 1.0f])
	float					realOffsetMotion_;		// сглаженное значение скорости смещения
	float					realForwardMotion_;		// сглаженное значение скорости (плавный разгон/торможение)
	float					realRotationMotion_;	// сглаженное значение угловой скорости
	Vector					m_vCurSpeed;			// скорость движения корабля (берется при разрушении и учитывается пока корабль тонет)

	// положение корабля
	bool					m_bRelativeShipPos;
	Matrix					m_mtxRelativeShipTransform;
	Matrix					m_mtxInitRelativeShipTransform;
	Vector					vPos_,vInitPos_;		// позиция кораблика
	Vector					vAngles_,vInitAngles_;	// ориентация кораблика
	Matrix					m_mtxTransform;

	// управление кораблем
	ConstString				ctrlParamsObjectName_;	// имя объекта параметров контроллера
	IShipController *		shipController_;		// контроллер корабля
	ConstString				m_connectToName;		// имя объекта к которому мы присоединяемся в неактивном режиме
	MOSafePointer			m_connectToObj;			// объект миссии, к которому мы присоединяемся в неактивном режиме
	bool					m_bFrameUpdated;		// позиция уже обновлялась на данном кадре

	// состояние корабля
	float					hp_;					// здоровье
	float					startHP_;				// начальное здоровье
	ShipParty				shipParty_;				// на чьей стороне играем
	bool					m_bCannonVolley;		// стрелять из пушек залпами
	DWORD					m_dwPartDestroyFlag;	// флажок разрушения части корабля
	bool					m_bShowWhileSink;		// сделать видимым объект работает пока корабль не потонет

	// физика корабля
	IPhysCombined*			rigidBody_;				// физ. объект - тело корабля
	bool					fakeSails_;				// делать ли паруса изначально фейковые
	bool					m_bNoPhysicsSails;		// никогда не создавать физические паруса
	bool					m_bNoTearedSails;		// паруса не могут повреждаться от дамага идущего не от игрока
	bool					m_bIsActivePhysObject;	// включен/отключен физобъект

	// время
	float					m_fLastDeltaTime;		// дельта тайм на текущем кадре
	float					globalTime_;			// время, как параметр фаза для качки и других временных функций
	float					releaseTimer_;			// счетчик времени с момента смерти до удаления корабля из памяти

	// человечки на палубе
	array<SailorHandhold> m_aSailorHandhold;		// опоры для человечков - если разрушаются, то матросики с них должны упасть
	array<LittleManImpl*>	littleMen_;				// набор человечков на палубе

	// звуки
	float m_fShipMoveSoundTime;		// время оставшееся до заведения нового звука движения корабля
	float m_fShipMoveFadeTime;		// время появления, гашения звука движения корабля
	bool m_bShipMoveFadeIn;			// флаг усиления/понижения уровня звука движения корабля
	bool m_bShipMoveCurrent;		// флаг текущего звука движения/стояния корабля
	ISound3D* m_pSoundFireHull;		// звук горения корпуса
	ISound3D* m_pSoundFireSail;		// звук горения паруса
	ISound3D* m_pSoundTouch;		// звук столкновения
	ISound3D* m_pSoundMoving;		// звук движения/стояния на месте

	// мины
	long m_nMineQuantity;				// текущее количество мин
	long m_nMineQuantityMax;			// количество мин (если < 0, то бесконечное)
	MissionTrigger triger_mineBeOver;	// евент на то, что мины кончились

	// состояние Show & Activate на старте объекта
	bool m_bInitShow;
	bool m_bInitActive;
	bool m_bNotCreated;
	long m_nInitPhase;

	// ачивки
	MOSafePointerType<IAchievement> achievSeawolf;
	MOSafePointerType<IAchievement> achievGunmaster;

	// на текущем кадре есть пауза
	bool m_bPause;

	ShipStatistics statistic;

	MOSafePointer m_seaMisParams;

public:
	// Конструктор - деструктор
	Ship();
	virtual ~Ship();

	// обработчик оповещений от паттерна
	void OnNotify(ShipPattern::Notification type);

	void DropSailors(ShipPart* pPart);

	virtual const PatternParams& GetPatternParams() {return content_;}
	IPhysCombined* GetPhysView() { return rigidBody_; }
	IClothBurns* GetClothBurns() {return m_pCBurns.Ptr();}
	IAnimation* GetDestructAnimation() {return m_pAniDestruct;}

	float GetAlpha() const { return alpha_; }
	float GetStartHP() const { return startHP_; }
	float GetLastDeltaTime() {return m_fLastDeltaTime;}
	float GetDamageFromMastBroke() {return content_.m_fDamageFromMastBroke;}

	//////////////////////////////////////////////////////////////////////////
	// функции для бонусов
	void RepairSails(const char* sfxName);
	void RepairHull();
	void AddHP(float hp);
	void RunWeaponsInspector(IWeaponInspector&);
	//////////////////////////////////////////////////////////////////////////
	

	void _cdecl Work(float fDeltaTime, long level);
	void _cdecl Draw(float fDeltaTime, long level);
	void _cdecl DebugInfoWork(float fDeltaTime, long level);
	void _cdecl FrameEnd(float fDeltaTime, long level);
	// отрисовка геометрии корабля
	void DrawGeometry();
	void _cdecl DrawRefl(GroupId, MissionObject*);
	void _cdecl DrawRefr(GroupId, MissionObject*);

	const char* GetShipPartWaterSplashSFX() {return content_.m_pcShipPartWaterSplash;}
	float GetShipPartWaterSplashStep() {return content_.m_fShipPartWaterSplashStep;}

	//////////////////////////////////////////////////////////////////////////
	// ShipPart
	//////////////////////////////////////////////////////////////////////////
	virtual ShipPart* Clone() { return NULL; }
	virtual float HandleDamage(const Vector&, float) {return 0.0f;}

	// работа с флагом уничтоженных частей
	void PartDestroyFlagSet(DWORD dwFlag) {m_dwPartDestroyFlag = m_dwPartDestroyFlag | dwFlag;}
	void PartDestroyFlagReset() {m_dwPartDestroyFlag = pdf_destroy_none;}
	DWORD PartDestroyFlag() {return m_dwPartDestroyFlag;}

	bool IsLimitedParticles(const Vector & pos);

	// получить имя звука для звукового евента
	const char* GetSoundNameForEvent(dword dwIdx) {if (dwIdx<ShipContent::se_quantity) return content_.pSound_[dwIdx]; return "";}

	// получить скорость вращения
	float GetCurAngleSpeed() const { return rotationMotion_ * fMaxRotateSpeed_; }

	void CreateAutoParticle(const char* pcName, const Matrix& mtx) {m_particescontainer.AddParticle( Particles(), pcName, mtx );}

	//////////////////////////////////////////////////////////////////////////
	// IShip
	//////////////////////////////////////////////////////////////////////////
	// получить текущую позицию корабля
	virtual Vector	GetPosition() const	{ return vPos_;	}
	// задает угловую скорость вращения
	virtual void	SetRotate(float value) { rotationMotion_ = value/GetMaxAngleSpeed(); }
	// задать  скорость движения
	virtual void	SetVelocity( float fShipVelocity ) { forwardMotion_ = fShipVelocity/fMaxSpeed_; }
	// задать скорость смещения
	virtual void	SetSideVelocity( float fShipOffset ) { offsetMotion_ = fShipOffset/fMaxSpeed_; }
	// получить текущий курсовой угол
	virtual float	GetCurrentDirectionAngle() { Matrix m; Vector dir(0, 0, 1.0f); return GetMatrix(m).MulNormal(dir).GetAY(); }
	// получить текущую скорость
	virtual float	GetCurrentVelocity() const { return rigidBody_ != NULL ? rigidBody_->GetLinearVelocity().GetLength() : 0.0f; }
	// получить максимально возможную скорость корабля
	virtual float	GetMaxVelocity() const { return fMaxSpeed_; }
	// получить максимально возможную угловую скорость 
	virtual float	GetMaxAngleSpeed() const { return fMaxRotateSpeed_; }
	// стрельнуть пушками по цели
	virtual void	FireCannons ( MissionObject * pFireTarget, float fFireLevel );
	// стрельнуть огнеметами
	virtual void	FireFlamethrower();
	// стрельнуть минами
	virtual void	FireMines();
	// установить  сторону корабля
	virtual void	SetParty ( ShipParty eShipParty ) { shipParty_ = eShipParty; }
	// установить  сторону корабля
	virtual ShipParty GetParty () const { return shipParty_; }
	// получить размеры корабля ( длину и ширину )
	virtual void	GetSizes ( float & fLength, float & fWidth ) const;

	void GetWeaponSidesProgress( float & fLCannonProgress, float & fRCannonProgress, float & fFlamethrowerProgress, float & fMineProgress );
	long GetMineCount() {return m_nMineQuantity;}

	PartsClassificator & GetPartClassificator() {m_partClassificator.Reset(); return m_partClassificator;}

	//////////////////////////////////////////////////////////////////////////
	// DamageReceiver
	//////////////////////////////////////////////////////////////////////////

	//Воздействовать на объект сферой
	virtual bool Attack(MissionObject * obj, dword source, float hp, const Vector & center, float radius);
	//Воздействовать на объект линией
	virtual bool Attack(MissionObject * obj, dword source, float hp, const Vector & from, const Vector & to);
	//Воздействовать на объект выпуклым чехырёхугольником
	virtual bool Attack(MissionObject * obj, dword source, float hp, const Vector vrt[4]);

	//////////////////////////////////////////////////////////////////////////
	// MissionObject
	//////////////////////////////////////////////////////////////////////////
		
	// Создание объекта
	virtual bool Create(MOPReader & reader);
	virtual void PostCreate();
	// Рестарт объекта
	virtual void Restart();
	// Обновление параметров
	virtual bool EditMode_Update(MOPReader & reader);
	// Инициализация 
	virtual bool Init();
	void _cdecl EditorInit(float fDeltaTime, long level);
	void _cdecl InGameInit(float fDeltaTime, long level);

	virtual void EditMode_GetSelectBox(Vector & min, Vector & max);

	virtual Matrix & GetMatrix(Matrix & mtx) { return GetTransform(mtx); }
	//Получить бокс, описывающий объект в локальных координатах
	virtual void GetBox(Vector & min, Vector & max);

	virtual float GetHP() { return hp_; }
	virtual float GetMaxHP() { return startHP_;}
	virtual bool IsDead();
	virtual void Show(bool isShow);
	virtual void Activate(bool isActive);

	//Обработчик команд для объекта
	virtual void Command(const char * id, dword numParams, const char ** params);

	MO_IS_FUNCTION(Ship, IShip);

	enum DieType
	{
		DieType_none,
		DieType_flamethrower,
		DieType_cannon,
		DieType_mine,
		DieType_ram,
		DieType_unknown
	} m_dieType;
	DieType GetDieType() {return m_dieType;}

	ShipStatistics & GetStatistics() {return statistic;}

	bool IsNoSwing() {return m_bNoSwing;}

	bool IsNoTeared() {return m_bNoTearedSails;}

private:
	// принять визитера
	virtual void Accept(IPartVisitor& visitor) { visitor.Visit(*this); }

	// копирование параметров с шаблона
	void CopyPattern();
	// чтение параметров
	void ReadMOPs(MOPReader & reader);
	// симуляция корабля
	void Simulate(float deltaTime);
	// установка физ. параметров
	void SetupPhysicParams();
	// добавляет шейпы (для рэйтрейса оружия) к основному коллижн-боксу
	void UpdatePhysForm(const PhysForm& form, ShipPart& part, bool enableCollision = false);
	// приложение физ. сил
	void ApplyPhysics(float deltaTime);
	// построить систему физ. тел для процесса разрушения корабля
	void Destruct(DieType dt);
	// почистить за собой
	void Destroy();
	// включает/выключает показ физических актеров
	void EnablePhysicActor(bool bEnable);
	// найти опорные части для матросов
	void TieSailorsToHandholds();
	// выгрузить модели с лодами
	void ReleaseModelLods();
	// загрузить модели лодов
	void CreateModelLods();
	// поставить всем моделькам в лодах соответсвующую анимацию
	void SetAnimationToLods(IAnimationTransform* pAni);//, IAnimationTransform* pAniLod);
	// повреждаем паруса от взрыва
	void SailTearByExplosion( const Vector& pos );

	// обновить прогресс бар
	void UpdateGUIProgressBar();
	void DisableGUIProgressBar();

	// 
	Matrix& GetTransform(Matrix& mtx) const;

	void GetLodModels(long lodNum, IGMXScene* &pModelMain, IGMXScene* &pModelRefl, IGMXScene* &pModelRefr);
	IGMXScene* GetLodMainModel( float fDist2Pow );
	IGMXScene* GetLodReflModel( float fDist2Pow );
	IGMXScene* GetLodRefrModel( float fDist2Pow );

	void Respawn();

	// управление пушками
	// получить орудийную сторону, с которой нужно стрелять
	long GetCannonWeaponSide(MissionObject * fireTarget, float& fTargDist, float fReadyProcent);
	// дострел пушек с борта (чтоб был полный залп
	void FinishWeaponSideCannons(long nWeaponSide, float fDistance);

//---------------------------------------------------------------
// Дебажная секция
//---------------------------------------------------------------
private:
	struct ShipDebugInfo
	{
		dword infoSwitch;

		struct cannonFireData
		{
			Vector from;
			Vector to;
			float speed;
		};
		array<cannonFireData> cannonFire;

		ShipDebugInfo() :
			cannonFire(_FL_)
		{
			infoSwitch = 0;
		}
	} m_debugInfoData;

	void Info_Show();

public:
	enum InfoMask
	{
		sdim_ship_info = 1,
		sdim_phys_form = 2,
		sdim_cannon_info = 4,
		sdim_ai_info = 8
	};

	void Info_SetMask(dword infoFlag);
	void Info_ResetMask(dword infoFlag);
	void Info_SwitchMask(dword infoFlag);
	bool Info_GetMask(dword infoFlag);
//---------------------------------------------------------------


public:
	static const char * SHIP_CONTROLLER_SERVICE;
	static const float SHIP_AFTERDEATH_TIME;
	static const ConstString playerName;
};

#endif
