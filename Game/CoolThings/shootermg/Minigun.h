#ifndef MINIGUN_MINIGAME_OBJECT
#define MINIGUN_MINIGAME_OBJECT

#include "..\..\..\Common_h\IShooter.h"
#include "ShooterTargets.h"
#include "AnimBlender.h"
#include "..\..\..\Common_h\AnimationStdEvents.h"

#include "ShooterEffect.h"

class IGreedy;
class IGUIElement;

class Minigun : public IShooter
{
	struct Effect
	{
		ConstString particle;
		ConstString sound;
	};

	// описание снаряда (трейла выстрела)
	struct TraceDescr
	{
		float fLifeTime;
		float speed;
		Vector vBeg, vDir;
		float maxDist;
	};

	struct Vertex
	{
		Vector p;
		dword color;
		float u, v;		
	};

	struct TraceParam
	{
		Vector vStartTraceOffset;

		float fTraceDistance;
		float fTraceDistanceDiv;

		float fSpeed;
		//float fTraceLifeTime;
		//float fTraceLifeTimeDiv;

		//float fMinSpeed;
		float fMinShowDist;

		float length, lengthAdd;
		float width, widthAdd;

		IVariable* vTex;

		const char* pcTraceTexture;
		const char* pcBallTexture;
		IBaseTexture* pTraceTexture;
		IBaseTexture* pBallTexture;
		IVBuffer * pvbTraceBuffer;
		IVBuffer * pvbBallBuffer;
		dword nTraceQuantity;
	};

	// состояние оружия
	enum WeaponStates
	{
		WEAPON_EMPTY = 0,
		WEAPON_LOADING = 1,
		WEAPON_READY_TO_FIRE = 2,
		WEAPON_FIRING = 3,

		WEAPON_FORCEDWORD = 0x7fffffff
	};

	// описание цели
	struct TargetDescribe
	{
		// тип цели
		enum TargetType
		{
			tt_none,

			tt_waterlevel,
			tt_Collider,
			tt_Ragdoll,
			tt_PassiveShooterTarget,

			tt_AutoAimedTarget, // метка для целей которые нужны автодоводке

			tt_DamageAcceptor = tt_AutoAimedTarget,
			tt_ShooterTarget
		};

		TargetType type;	// тип цели

		MissionObject* mo;	// объект цели
		IPhysBase* phys;	// физический объект

		Vector pos;			// позиция цели
		Vector normal;		// нормаль поверхности попадания

		// конструктор
		TargetDescribe() {type=tt_none; mo=null;}
		// нанести дамаг
		void ReceiveDamage(float fRadius, float fDamage, float fPower);
	};

	struct WaitingTarget
	{
		float time;
		TargetDescribe::TargetType targtype;
		MOSafePointer mosp;
		IPhysBase* phys;
		Vector pos;			// позиция цели
		Vector normal;		// нормаль поверхности попадания
	};
	array<WaitingTarget> m_aWaitingTargets;

	ConstString m_pTiedCameraID;
	MOSafePointer m_pTiedCamera;

	float m_fShoterSensitivity;
	bool m_bShoterInverse;
	bool m_bShoterAIM;

	//float m_fWideScreenAspectDividerConst;

	// построить указатели для таргетов шутера
	bool bPointerBuilded;
	// таргеты шутера
	ShooterTargets* targets[4];

	// параметры повреждения от пули
	float radius;
	float damage;
	float power;

	// сила воздействия на регдолл
	float fRagdollBulletImpulse;

	// выводить дебаг инфу
	bool bLogicDebug;
	// для дебаг инфы состояние я хочу стрелять
	bool bWantToShoot;
	// флаг момента стрельбы
	bool bShootMoment;
	// показывать коллидеры таргетов
	bool showColliders;

	// трейсы пуль
	array<TraceDescr> aBulletTrace;
	TraceParam traceparam;

	// отбрасывать тень
	bool bShadowCast;

	float m_fMaxShootDist;
	float m_fMinShootDist;

	// эффект попадания снаряда в воду
	Effect fxWaterSplash;
	// эффект попадания снаряда в коллидер
	Effect fxColliderSplash;
	// эффект попадания снаряда в персонажа
	Effect fxCharacterSplash;
	// эффект попадания снаряда в регдол
	Effect fxRagdollSplash;

	// эффект выстрела
	Effect fxShoot;

	// эффект шутера
	ShooterEffect shooterEffect;

	// звук перезарядки
	ConstString ReloadSound;
	// звук кручения ствола
	ConstString RotateSound;
	ISound3D* pSoundRotate;

	// активировать шутер с включенным зумом
	bool  init_zoomOn;
	// перспектива при зуме/нормальном просмотре из шутера
	float nearFOV;
	float farFOV;
	// время на зум шутера
	float fovChgTime;
	// текущий фов
	float fov;
	// включено приближения для шутера
	bool  zoomOn;
	// текущее время смены зума
	float fovChgCurTime;

	// ближний/дальний план для камеры шутера
	float NearPlane;
	float FarPlane;

	// геометрия шутера
	IGMXScene* Weapon;
	// анимация шутера
	IAnimation* pAnim;
	// обработчик евентов анимации для шутера
	AnimationStdEvents* Listener;

	// геометрия магазина (заряды)
	IGMXScene* WeaponAddition;
	// анимация магазина (перезарядка)
	IAnimation* pAnimAddition;
	GMXHANDLE AdditionalLocator;

	// геометрия рук
	IGMXScene* WeaponLoader;
	// анимация рук
	IAnimation* pAnimLoader;
	// обработчик евентов анимации для шутера
	AnimationStdEvents* pLoaderListener;

	// процедурная анимация для шутера
	AnimBlender blender;

	// локаторы в геометрии шутера
	GMXHANDLE CameraLocator;			// локатор камеры
	GMXHANDLE BulletLocator;			// локатор снаряда
	GMXHANDLE ShootLocator[10];			// локатер выстрела
	long m_nShootLocatorsQuantity;		// количество локаторов для выстрела
	long m_nCurShootLocatorIndex;


	// позиция шутера
	Vector vPos;
	Vector vViewAng;

	float fDispersionAngle;
	Matrix mtxShootDirection;

	// ограничения поворота шутера
	float HeightLimitMinRad;
	float HeightLimitMaxRad;
	float RotateLimitMinRad;
	float RotateLimitMaxRad;

	// количество выстрелов до перехода на анимационный граф с заканчивающимися патронами
	long nRepeatableShootQuantity;
	// оставшееся количество выстрелов до перехода на граф с конечными патронами
	long nRepeatableShootLeft;

	// взрывать точку попадания шутера
	bool bExplodeBullets;

	int wasActivated;

	MissionTrigger triger_onActivate;
	MissionTrigger triger_onDeactivate;

	MissionTrigger triger_onZoomIn;
	MissionTrigger triger_onZoomOut;

	MOSafePointer waterLevel;

	float   fRotSpeedH;
	float   fRotSpeedV;

	long    codeWeapon_Fire;
	long 	codeWeapon_Exit;
	long    codeWeapon_Turn_Left;
	long    codeWeapon_Turn_Right;
	long    codeWeapon_Turn_Up;
	long    codeWeapon_Turn_Down;

	long    codeWeapon_TurnMouse_H;
	long    codeWeapon_TurnMouse_V;

	float   fMsMaxRotSpeed;
	float   fMsRotAccelerate;

	float   fJoyMaxRotSpeed;
	float   fJoyRotAccelerate;



	float   fMsMaxRotSpeed_zoomed;
	float   fMsRotAccelerate_zoomed;

	float   fJoyMaxRotSpeed_zoomed;
	float   fJoyRotAccelerate_zoomed;


	float   fMouseUpdate;
	int     iMouseTimes;
	float   fCurMouseValueV;
	float   fCurMouseValueH;
	float   fMouseFilteredV;
	float   fMouseFilteredH;

	float	fShootInterval;
	float	fShootTime;

	// при инициализации шутера активировать его
	bool    init_active_state;

	// проводить коррекцию для свинг машины
	bool	useSwingCorrection;

	// пожиратель бонусов
	IGreedy* pBonusGreedy;

	// текущая цель куда мы попадаем из шутера
	TargetDescribe	m_targetDescr;

	// виджеты для прицела
	IGUIElement*	m_pSightGUI_Hit;
	IGUIElement*	m_pSightGUI_Miss;

	// положение прицела
	float m_fSightSpeed;
	//float m_fSightAng;
	float m_fSightSize;
	float m_fSightOffset;
	struct ComplexityData
	{
		const char* name;
		float autoaimSize;
		//float autoaimAngle;
		//float autoaimOffset;
		ConstString pcSightName_Hit;
		ConstString pcSightName_Miss;
	};
	array<ComplexityData> m_aComplexityData;

	// ускорение анимации стрельбы
	float m_fShootAnimationSpeed;
	float m_fBulletAnimationSpeed;

	//float m_fTmpRotateSpeed;

	void* m_pExplosionPatch;

public:
	//Конструктор - деструктор
	Minigun();
	virtual ~Minigun();

	//Создание объекта
	virtual bool Create(MOPReader & reader);

	virtual void PostCreate();

	//Обновление параметров
	virtual bool EditMode_Update(MOPReader & reader);	

	void _cdecl RealizeProcess(float fDeltaTime, long level);
	void _cdecl RealizeDraw(float fDeltaTime, long level);
	void _cdecl RealizeControl(float fDeltaTime, long level);
	void _cdecl DrawTrails(float fDeltaTime, long level);

	void EditMode_GetSelectBox(Vector & min, Vector & max);

	virtual void Activate(bool isActive);
	virtual void Show(bool bShow);

	virtual Matrix & GetMatrix(Matrix & mtx);

	virtual void GetBox(Vector & min, Vector & max);
	virtual void _cdecl ShadowDraw ();

	void SetShootMoment() {bShootMoment=true;}

protected:

	virtual void UpdateColliders ();

	virtual void Command(const char * id, dword numParams, const char ** params);

	virtual void Restart();

	void DumpStateToConsole (const char* mess);


	WeaponStates GetState();

	void BuildPointers ();

	int CheckCollision (const Vector& from, const Vector& to, float damage);

	void UpdateColliderPositions ();

	void AddTrace(float deltaTime);

	void DrawLimiters();

	void ChangeValue(float &value, float targetValue, float dltTime, float accel);

	void ClampAngles ();

	void SetWeaponTransform ();
	void DrawModel();

	void Fire (float deltaTime);

	void Reload ();
	void StopShooter();

	void Release();

	virtual float GetRadius() { return radius; };
	virtual float GetDamage() { return damage; };
	virtual float GetPower() { return power; };

	virtual const ConstString & GetCharParticle() { return fxCharacterSplash.particle; };
	virtual const ConstString & GetCharSound() { return fxCharacterSplash.sound; };

	virtual bool CheckDirection(const Vector& pos);

	void    ZoomEnable(bool enable);

	void UpdateSight();
	bool FindNearTarget( const Vector& pos, const Vector& dir, float fmaxdist, float fApproachSin, TargetDescribe & td );
	void AutoAimTarget( const TargetDescribe & td );
	float GetAutoAimSin();

	void RandomizeTargetPoint( TargetDescribe & td );
	Vector GetRandomPointFromMO( const Vector & from, const Vector & to, const Sphere & sph, DamageReceiver* mo );

	void TargetDamage(WaitingTarget & targ);
	void RememberTargetDamage();
	void UpdateTargetDamage(float fDeltaTime);

	void EffectReadMOPs(Effect & effect, MOPReader & reader);
	void EffectStart(const Effect & effect, const Vector& pos, const Vector& normal);

	Vector GetLocatorPos(IGMXScene* pScene,const GMXHANDLE& hloc) {if(pScene) {Matrix m; return pScene->GetNodeWorldTransform(hloc,m).pos;} return 0.f;}
};

#endif