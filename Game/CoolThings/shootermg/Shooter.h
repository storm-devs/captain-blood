#ifndef SHOOTER_MINIGAME_OBJECT
#define SHOOTER_MINIGAME_OBJECT

#include "..\..\..\Common_h\IShooter.h"
#include "ShooterTargets.h"
#include "AnimListener.h"
#include "AnimHandler.h"
#include "AnimBlender.h"

#include "..\..\..\Common_h\ITrail.h"

class IGreedy;

class Shooter : public IShooter
{

	struct BulletParam
	{
		Matrix mInitial;
		Vector vOldPos;
		Vector vPos;
		Vector Velocity;
		Vector Force;
		float fLifeTime;
		Matrix mTrans;
		ITrail *trail;
	};

	IPhysicsScene::RaycastResult HitDetail; 

	enum WeaponStates
	{
		WEAPON_EMPTY = 0,
		WEAPON_LOADING = 1,
		WEAPON_READY_TO_FIRE = 2,
		WEAPON_FIRING = 3,

		WEAPON_FORCEDWORD = 0x7fffffff
	};

	struct TargetTrackVertex
	{
		Vector pos;
		dword color;
		float u,v;
	};

	struct TargetTrackParams
	{
		IVBuffer* pvbTargTrack;			// вертекс буфер для трека выстрела (режим прицеливания)
		IBaseTexture* ptexTargTrack;	// текстура для трека выстрела (режим прицеливания)
		IBaseTexture* ptexTargAim;		// текстура для прицела выстрела (режим прицеливания)
		IVariable* varTargTrack;		// переменная в шейдере для хранения текстуры трека (режим прицеливания)
		ShaderId shidTargTrack;

		IGMXScene* pAimModel;			// моделька для прицела
		Color colAimModel;				// цвет модельки прицела
		float fAimModelScaleMin;		// минимальный масштаб для модельки прицела
		float fAimModelScaleMax;		// максимальный масштаб для модельки прицела
		float fAimModelScaleSpeed;		// скорость изменения масштаба для модельки прицела
		float fAimModelScaleCurTime;	// фаза изменения масштаба для модельки прицела

		float fAimTargetSize;
		float fAimTrackWidth;
		float fAimTrackWidthTimeK;
		float fAimTrackTimeStep;
		float fUVSpeed;
		float fAimTrackUV;
		Color colAimTrack;
		Color colAimTrackHit;
		long nTrackVertexMax;

		float fFadeTime;
		float fFadeMaxTime;
		bool bShow;
		bool bFadeIn;
	};

	TargetTrackParams m_targtrack;

	bool bPointerBuilded;

	float radius;
	float damage;
	float power;


	bool bWantToShoot;
	bool bLogicDebug;	

	array<BulletParam> Bullets;	
	ShooterTargets* targets[4];

	bool bDontReloadWhileActiveBullets;	

	bool bShadowCast;
	bool bCameraAttachedToBullet;	

	ConstString WaterSplashParticles;
	ConstString WaterSplashSound;

	ConstString ShootParticles;
	ConstString ShootSound;
	ConstString ReloadSound;	
	
	bool  showColliders;
	float fAccelerateTime;
	float fBrakingTime;
	
	float nearFOV;
	float farFOV;	
	
	bool  init_zoomOn;
	float fovChgTime;

	float fov;
	bool  zoomOn;	
	float fovChgCurTime;	

	float NearPlane;
	float FarPlane;
	
	float fWaitAfterDead;
	float fMaxCamDistance;


	float fTimeMultipler;

	bool bCameraDeteched;
	bool bWatchToBullet;

	Quaternion qRot;

	Vector vCameraVector;

	float HorizForce;
	float VertForce;

	float HorizVelocity;
	float VertVelocity;
	float fGravity;
	float fCameraFriction;
	float fCameraHitFriction;


	bool bVisible;

	
	struct THitRes
	{
		Vector  pos;
		dword   color;
		char    id[128];
	};

	int  iNumHits;
	int  iCurHit;

	#define NUM_MAXHITS 5
	THitRes  hitPoints[NUM_MAXHITS];

	char  dr_id[128];
	
	int  iNumSegments;
	int  iCurSegment;

	struct Segment
	{
		Vector beg, end;
		bool intrsection;
	};

	#define NUM_MAXSEGMENT 1024
	Segment segments[NUM_MAXSEGMENT];
	bool bDebugDraw;


	IGMXScene* Weapon;
	IGMXScene* WeaponGlow;
	IGMXScene* Bullet;
	IGMXScene* Loader;

	IAnimation* pAnim;
	IAnimation* pAnimLoader;


	ShooterAnimEventsHandler* Listener;
	BulletAnimEventsHandler* BulletAnimHandler;	

	AnimBlender blender;

	GMXHANDLE CameraLocator;
	GMXHANDLE BulletLocator;
	GMXHANDLE ShootLocator;
	GMXHANDLE BulletCameraLocator;


	Matrix mLastCamAttachedToBulletMatrix;	
	Vector vPos;
	


	Vector vViewAng;
	Vector vStartViewAng;

	float fDeadTime;

	float HeightLimitMinRad;
	float HeightLimitMaxRad;

	float RotateLimitMinRad;
	float RotateLimitMaxRad;

	float fBulletStartVelocity;
	float fBulletMaxLifeTime;

	bool bExplodeBullets;	

	ConstString char_particle;
	ConstString char_sound;

	ConstString ragdoll_particle;
	ConstString ragdoll_sound;

	bool bTraceEnbled;
	float fTraceInitSize;
	float fTraceFinalSize;
	float fTraceMinFadeoutTime;
	float fTraceMaxFadeoutTime;
	float fTraceOffsetStrength;
	Color cTraceColor;	

	float  fTraceWindVel;
	Vector vTraceWindDir;

	MOSafePointerType<ITrailManager> tm;

	int wasActivated;
	

	MissionTrigger triger_onActivate;
	MissionTrigger triger_onDeactivate;

	MissionTrigger triger_onZoomIn;
	MissionTrigger triger_onZoomOut;

	MissionTrigger m_triggerReloadStart;
	MissionTrigger m_triggerReloadEnd;

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

	Vector  vViewCenter;

	bool    was_activated;	
	bool    init_active_state;

	float	m_fCurReloadTimer;
	float	m_fMaxReloadTimer;

	ConstString m_pcReloadingGUI;
	MOSafePointer m_ReloadingGUI;

	float	m_fGlowPow;
	Color	m_colMinGlowColor;
	Color	m_colMaxGlowColor;

	IGreedy* pBonusGreedy;

	struct WeaponPlatform
	{
		MOSafePointer sptr;
		ConstString pcPlatformID;
	};
	array<WeaponPlatform> m_WeaponPlatforms;

//	float m_fWideScreenAspectDividerConst;
	ConstString m_pTiedCameraID;
	MOSafePointer m_pTiedCamera;

	Vector m_vAimTargetPos;
	Vector m_vTargetPos;

	MOSafePointerType<IShooterKickOut> m_ShooterKickOut;

public:
	//Конструктор - деструктор
	Shooter();
	virtual ~Shooter();

	//Создание объекта
	virtual bool Create(MOPReader & reader);

	virtual void PostCreate();

	//Обновление параметров
	virtual bool EditMode_Update(MOPReader & reader);	

	void _cdecl RealizeProcess(float fDeltaTime, long level);
	void _cdecl RealizeDraw(float fDeltaTime, long level);
	void _cdecl RealizeControl(float fDeltaTime, long level);

	void _cdecl RealizeDebugDraw(float fDeltaTime, long level);

	void EditMode_GetSelectBox(Vector & min, Vector & max);

	virtual void Activate(bool isActive);
	virtual void Show(bool bShow);

	virtual Matrix & GetMatrix(Matrix & mtx);

	virtual void GetBox(Vector & min, Vector & max);
	virtual void _cdecl ShadowDraw ();

protected:
	
	virtual void UpdateColliders ();

	virtual void Command(const char * id, dword numParams, const char ** params);
    
	
	virtual void Restart();

	void DumpStateToConsole (const char* mess);


	WeaponStates GetState();
		
	void CalcBulletTransform (int BulletIndex);
	void BuildPointers ();

	int CheckCollision (const Vector& from, const Vector& to, float damage);
	bool TestCollision (const Vector& from, const Vector& to);

	void UpdateColliderPositions ();

	void DrawLimiters();	

	void ChangeValue(float &value, float targetValue, float dltTime, float accel);	

	void ClampAngles ();

	void SetWeaponTransform ();
	void DrawModel();
	void DrawBullets (float fDeltaTime);

	void ProcessBullets (float fDeltaTime);

	void Fire ();

	// получить объект по которому стреляем
	MissionObject* GetTargetMO();

	void Reload ();

	void Release();

	virtual float GetRadius() { return radius; };
	virtual float GetDamage() { return damage; };
	virtual float GetPower() { return power; };

	virtual const ConstString & GetCharParticle() { return char_particle; };
	virtual const ConstString & GetCharSound() { return char_sound; };

	virtual bool CheckDirection(const Vector& pos);

	void DrawPreviewTrail(float fDeltaTime);
	void FadePreviewTrail(bool bFadeIn);

	void    ZoomEnable(bool enable);
	#ifndef NO_CONSOLE

	void _cdecl Console_EnableDebug(const ConsoleStack & params);

	#endif
};

#endif