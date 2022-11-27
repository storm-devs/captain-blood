//===========================================================================================================================
// Spirenkov Maxim, 2004
//===========================================================================================================================//	
// Character
//===========================================================================================================================
// Character
//============================================================================================

#ifndef _Character_h_
#define _Character_h_


#include "..\..\Common_h\ICharacter.h"
#include "CharacterPattern.h"
#include "Components\BodyParts\CharacterBodyParts.h"
#include "Components\AnimationsBlender.h"
#include "..\auxiliary objects\EffectTable\EffectTable.h"
#include "..\Auxiliary objects\ActorTimeLine\ActorTimeLine.h"
#include "..\Auxiliary objects\CharacterEffect\CharacterEffect.h"
#include "components\characterAchievements.h"
#include "Components\CharacterPhysics.h"
#include "..\..\common_h\bonuses.h"
#include "..\utils\SafePointerEx.h"

#define FATALITY_TIME 5.0f
#define CoinChangeTime 0.3f

class CharactersArbiter;
class CharacterArmor;
class CharactersLogic;
class CharacterController;
class CharacterControllerParams;
class CharacterButtonMiniGame;
class CharacterLogic;
class CharacterEffect;
class CharacterAchievements;
class CharacterInitAI;
class ButtonMiniGameParams;

class CharacterAnimation;
class CharacterAnimationEvents;
class CharacterItems;

struct MiniGame_Desc;


class Character : public ICharacter
{	
	friend CharacterPhysics;
	friend CharacterLogic;
	friend CharacterItems;
	friend CharacterAnimation;
	friend CharacterArmor;
	friend CharacterAnimationEvents;
	friend CharacterHair;

	typedef void (_cdecl Character::* CharCommand)(const char * id, dword numParams, const char ** params);

	struct CommandRecord
	{
		void Init(const char * sid, CharCommand cmd, CommandRecord ** entry, dword mask);
		ConstString id;				//Идентификатор команды
		CharCommand command;		//Указатель на функцию команды
		CommandRecord * next;		//Следующий в цепочке хэша
	};

	struct CommandsTable
	{
		CommandsTable();

		bool ExecuteCommand(Character * chr, const char * id, dword numParams, const char ** params);

	private:
		CommandRecord table[48];
		CommandRecord * entryTable[64];
	};
	friend CommandsTable;
	


	struct Model
	{
		Model()
		{
			scene = null;
		};
		~Model()
		{
			if(scene)
			{
				scene->Release();
				scene = null;
			}
		};

		IGMXScene * scene;		
	};

	struct TPartOfMe
	{
		bool isBox;

		IPhysBox* pPhysBox;
		IPhysCapsule* pPhysCapsule;
		
		Model PartModel;
		ConstString LocatorName;
		ConstString ID;	
		IParticleSystem* pParticale;		

		TPartOfMe()
		{
			isBox = true;
			pPhysBox = NULL;
			pPhysCapsule = NULL;
		};
	};	


	ConstString pattern_name;
	MOSafePointerTypeEx<CharacterPattern> pattern_ptr;
	CharacterPattern* pattern;

	bool statDismembered;

	Vector init_pos;
	float  init_ay;	

	const char* init_actorani;
	ConstString init_actorTimeLineName;
	bool init_actrorPreview;
	bool init_EnableAniBlend;
	bool init_ColideWhenActor;
	ConstString init_aiInit;
	bool init_Show;
	bool init_Active;
	ConstString aiType;
	float fBloodDropDamage;
	bool init_VanishBody;
	bool init_bActorMoveColider;	
	bool useRootBone;

	bool   set_new_ay;
	float  new_ay;
	Vector new_pos;

	float  fTimeToFreeze;

	float  fDistShadow;

	IGreedy* greedy;

	float	attackSpeedModifier;

public:
	bool	isMoneyFatality;

	bool	auto_spawnpoint_saved;
	bool	auto_spawnpoint;
	Vector	init_spawnpoint;

	bool	need_cntl_shoot_call;

	Character();
	virtual ~Character();

//--------------------------------------------------------------------------------------------
//Интерфейс миссионого объекта
//--------------------------------------------------------------------------------------------
public:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);

	//Вызываеться, когда все объекты созданны но ещё не началось исполнение миссии
	virtual void PostCreate();

	//Пересоздать объект
	virtual void Restart();

	//Освободить ресурсы
	virtual void Release();

	//Обработчик команд для объекта
	virtual void Command(const char * id, dword numParams, const char ** params);

	//Инициализировать объект в режиме редактирования
	virtual bool EditMode_Create(MOPReader & reader);
	//Обновить параметры в режиме редактирования
	virtual bool EditMode_Update(MOPReader & reader);
	//Получить размеры описывающего ящика
	virtual void EditMode_GetSelectBox(Vector & min, Vector & max);

	//Отрисовка в выделеном режиме
	virtual void _cdecl EditModeDrawSelected(float dltTime, long level);

#ifndef MIS_STOP_EDIT_FUNCS
	//Выделить объект
	virtual void EditMode_Select(bool isSelect);
#endif

	//Получить матрицу объекта
	virtual Matrix & GetMatrix(Matrix & mtx);

	//Получить матрицу объекта
	virtual Matrix & GetColiderMatrix(Matrix & mtx);

	virtual const ConstString & GetKillerId() { return killerId; };

	//Получить бокс, описывающий объект
	virtual void GetBox(Vector & min, Vector & max);
	//Показать/скрыть объект
	virtual void Show(bool isShow);
	//Активировать/деактивировать объект
	virtual void Activate(bool isActive);
	//Умирает
	virtual bool IsDie();
	//Мёртв
	virtual bool IsDead();	
	//Получить текущее значение жизни
	virtual float GetHP();
	//Получить максимальное значение жизни
	virtual float GetMaxHP();

	inline bool IsShotImmuneWhenRoll() { return bShotImmuneWhenRoll; }

	//Плеейр ли счар
	bool IsPlayer() { return isPlayer; };	

	//Добавить отладочное сообщение
	void _cdecl DebugMessage(dword color, const char * format, ...);

	float GetIndividualAttackSpeedModifier() { return attackSpeedModifier; }

//--------------------------------------------------------------------------------------------
//Команды персонажа
//--------------------------------------------------------------------------------------------
private:
	void _cdecl Cmd_enableactor(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_disableactor(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_actorset(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_goto(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_additem(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_delitem(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_sethp(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_sethpLimit(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_changehp(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_teleport(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_gizmorespawn(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_respawn(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_show(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_hide(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_activate(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_deactivate(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_instant_kill(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_EnableActorColider(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_DisableActorColider(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_EnableAniBlend(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_DisableAniBlend(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_setwaypoint(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_setFParams(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_settarget(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_setbombtarget(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_movelocator(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_drawon(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_drawoff(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_show_weapon(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_grab_weapon(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_take_weapon(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_ragdoll(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_pair_minigame(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_pair_minigame_add(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_collapse(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_actor_attach(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_actor_deattach(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_hairenable(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_legsenable(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_UseRootBone(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_controller(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_Achievement(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_ResetAI(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_RedirectSoundEvent(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_GodMode(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_postdraw(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_damageweapon(const char * id, dword numParams, const char ** params);
	void _cdecl Cmd_dropweapon(const char * id, dword numParams, const char ** params);



//--------------------------------------------------------------------------------------------
//Внутренний интерфейс
//--------------------------------------------------------------------------------------------
public:
	//Установить цель для бомбы
	void SetBombTarget(const Vector & boomPosition);
	//Состояние бомбы
	bool IsBombBusy();
	//Получить позицию бомбы в заданный момент времени (time = 0..1)
	static Vector GetBombPosition(const Vector & from, const Vector & to, float time);		

	void SetUseBombPhysics(bool enable) { useBombPhysic = enable; };

	void SetBombSpeed(float bmbSpeed) { bombSpeed = bmbSpeed; };
	float GetBombSpeed() { return bombSpeed; };

//--------------------------------------------------------------------------------------------
//Имплементация
//--------------------------------------------------------------------------------------------
private:	
	//Базовая инициализация
	bool Init(MOPReader & reader);

	bool InitFromPattern();
	void InitCharData();
	bool InitDefault();

	void _cdecl EditMode_Work(float dltTime, long level);

	//Установить модельку
	bool SetModel(Model & model, const char * modelName);
		
	//Установить анимацию
	//bool SetAnimation(const char * aniName);
	//Установить анимацию
	//void SetAnimation(IAnimation* anim);

	//Установить контролер
	bool SetController(const ConstString & ctrlName);
	//Установить арбитра персонажей
	bool SetArbiter();
	//Установить сторну персонажа
	void SetSide(const ConstString & side);

	// удалить мушек
	void ReleaseFlyes();

	void DrawShadowModel(bool recive_pass);

	//Работа логики персонажа
	void _cdecl Work(float dltTime, long level);

	//Работа логики персонажа
	void _cdecl UpdatePhysics(float dltTime, long level);

	//Нарисовать модельку персонажа
	void _cdecl Draw(float dltTime, long level);
	//Нарисовать полупрозрачную модельку персонажа
	void _cdecl DrawTransparency(float dltTime, long level);
	//Нарисовать модельку персонажа для тени
	void _cdecl ShadowInfo(const char * group, MissionObject * sender);
	//Нарисовать модельку персонажа для тени
	void _cdecl ShadowDraw(const char * group, MissionObject * sender);
	//Нарисовать модельку персонажа для прохода получения тени
	void _cdecl ShadowReciveDraw(const char * group, MissionObject * sender);
	//Нарисовать модельку персонажа для тени
	void _cdecl ReflectionDraw(const char * group, MissionObject * sender);
	//Нарисовать след за шпагой
	void _cdecl DrawTrail(float dltTime, long level);
	//Нарисовать фларии от оружия
	void _cdecl DrawFlares(float dltTime, long level);
	//Нарисовать модельку персонажа в режиме редактирования

	// отрисовка HUD для данного персонажа
	void _cdecl DrawHUD(float dltTime, long level);

	void _cdecl EditMode_Draw(float dltTime, long level);
	//Нарисовать модельку и навеску
	void DrawModel(const Matrix & mtx, IGMXScene * mdl, bool drawCoin = true, const Color * userColor = null);

	//Счётчик лежания трупа
	void _cdecl DeadTimer(float dltTime, long level);	

	//Полёт бомбы
	void _cdecl BulletFly(float dltTime, long level);		

	//Переинициализировать состояние актёра
	void ResetActor();
	
	//Отложеная установка актёра после инициализации
	void _cdecl PostInitSetActor(float dltTime, long level);
    
	void _cdecl GetUp(float dltTime, long level);

	float GetDeathTime();

public:

	void SelfBlowBombardier();

	//Обновить состояние актёра
	void UpdateActor(float dltTime);

	//Активировать/деактивировать рэгдол
	void ActivateRagdoll(bool isAct, float blendTime, float ImpulsePow = 1.0f, bool autorelease_ragdoll = true);

	void ArmRagdoll();

	void FlyBodyPart(const ConstString & ID ,const ConstString & ParticalName, float fImpulseVertPow, float fImpulseHorzPow, float torque,const char * bone);
	
	void ApplyAnimation();
	void _cdecl ApplyAnimationQuery(float dltTime, long level = ML_FIRST);

	void _cdecl OneFrameHide(float dltTime, long level);

	//Бросить предметы
	void DropItems();

	void DropItem(const ConstString & item_id);

	//Бросить бомбу
	void DropBomb(float bomblietime = 0.0f);

	//Стрельнуть
	void Shoot(float dmg);

	//Стрельнуть по множеству целям
	void ShootTargets(float dmg, const char* hit_react,bool flyAway,float fly_dist,float damage_radius, bool damage_once);

	//Убить перса
	void Kill();	

	void ReCalcMatrices();

	// растолкать соседних чаров
	void PushCharcters(float dltTimem,float fRadius, bool isCircleSearch, bool only_ally);

	void GetPosFromRootBone(Vector& pos);

	MO_IS_FUNCTION(Character, DamageReceiver);

	void DrawCharacterInPosition(const Matrix & mtx, const Color * userColor = null);
	//Загрузить рэгдол
	void LoadRagdoll();


	float GetTimeInFatality();

	// Вано добавил, проверяет матрицу на скалирование, и если чего то делает 3x3 единичной
	Matrix & FixMtx(Matrix & mtx);

	void BornParticle(const char* LocatorName,const char* ParticleName);

	bool FindLocator(const char* LocatorName,Vector &pos);	
	bool FindLocator(const char* LocatorName,Matrix &loc_mat);

	// получить матрицу кости
	Matrix GetBoneMatrix(long index);	

	virtual IAnimation* GetAnimation();	

//--------------------------------------------------------------------------------------------
public:
	//Арбитр для всех персонажей мисии
	CharactersArbiter* arbiter;	

	//Логическая часть персонажа
	CharacterLogic* logic;
	
	//Предметы персонажа	
	CharacterItems* items;

	//Физическая часть персонажа
	CharacterPhysics chr_physics;	
	CharacterPhysics* physics;
	//Текущий контролер персонажа
	CharacterController* controller;
	
	//Обработчик событий	
	CharacterAnimationEvents* events;
	//Кнопочный минигейм
	CharacterButtonMiniGame* buttonMG;

	//класс для рабоыт с анимацией	
	CharacterAnimation* animation;

	//броня персонажа
	CharacterArmor* armor;

	float killsPerSecond;

private:

	bool bShotImmuneWhenRoll;

	bool bWaterSpalshBorned;
	//Модели персонажа	
	Model model;						//Основная моделька персонажа	

public:
	
	IGMXScene *GetModel()
	{
		return model.scene;
	}
	
private:
	bool oneFrameHide;

	Model shadowModel;					//Моделька для тени

	float fTimeInFatality;


public:
	IFlysCloud* flyCloud;
	IPhysRagdoll * ragdoll;				//Рэгдол персонажа	
	ActorTimeLine::ActorData actorData;	//Параметры для MovieCtrParams указывающие текущее положение

private:
	
	IGMXScene * currentModel;			//Текущая моделька	
	bool showCollider;					//Показать колидер
	bool isHitLight;					//Было попадание, рисуем его
	float hitLight;						//Сиграл попадания			
	Vector angles;						//Углы вращения бомбы
	Vector anglesRot;					//Скорости вращения бомбы
	bool   useBombPhysic;
	Vector bombStart;
	Vector bombEnd;
	float bombSpeed;                    //Скорость полета бомбы
	float flySpeed;						//Относительная скорость полёта
	float bombTime;						//Текущая позиция бомбы
	float bombLieTime;					//Время которое лежит бомба на земле	
public:
	CharacterBodyParts		bodyparts;		//Процедурная анимация частей тела
	EffectTable*			step_effectTable;
	EffectTable*			shoot_effectTable;
	CharacterEffect*		chrEffect;
	CharacterAchievements*	chrAchievements;
	CharacterInitAI*		chrInitAI;
	ButtonMiniGameParams*	mg_params;
	ButtonMiniGameParams*	common_mg_params;
	
	ConstString		bombExplosionPattern;
	MOSafePointer	bombExplosionMO;
	
	bool	isDeadBodyInFrustum;

	void VanishBody(float dltTime);

	// возвращает null если нету, или указатель на MissionObject *
	MissionObject * GetBombExplosionPattern();

private:
	CharacterControllerParams * ccp;	//Объект инициализации
	ConstString actorTimeLineName;		//Имя текущего таймлайна
	ActorTimeLine * actorTimeLine;		//Параметры для актёра			
	bool dynamicLighting;				//Динамическое освещение
	bool shadowCast;					//Отбрасывает тень
	bool shadowReceive;					//Принимает тень
	bool seaReflection;					//Отражается в море
	bool seaRefraction;					//Преломление в море
	bool isPlayer;						//Данный персонаж является плеером
	
	bool bColideWhenActor;

	ConstString FatalityParamsId;
	char FatalityParamsData[64];
	ConstString AttackParams;

	bool bEnableAniBlend;

	bool bVanishBody;

	bool bActorMoveColider;

	int  debugInfoType;

	ConstString bonusTable; // имя таблицы бонусов

	// дополнительный цвет модельки
	Color add_color;

	bool  bodyparts_inited;

	bool bShadowGrRegistred;
	bool bReflectGrRegistred;
	bool bRefractGrRegistred;
	
	void CheckRenderState();

	IAnimation** PrepareAnim();

#ifdef CHARACTER_STARFORCE
	float scaleSpike;			// Направление и скорость изменения перса
	float scaleSpikeTime;		// время после которого перс начинает изменяться
	float curScaleSpike;		// Коээф масштабирования, в норм режиме = 1.0
#endif

public:
	
	MissionObject* hpBar;

	float fHitDmgMultiplayer;
	float fBombDmgMultiplayer;
	float fShootDmgMultiplayer;
	float fRageDmgMultiplayer;

	float fBlockProbabilityPerHit, fBlockProbabilityPerShoot;
	float block_cooldown;

	float fTotalDmgMultiplier;

	array<TPartOfMe> BodyParts;
	float fTimeToStandUp;
	float fTotalTimeToStandUp;
	Vector vLastRagdollPos;

	bool  bDeadConterOn;
	float deadTime;			//Счетчик времени лежания в смерти (работает тока когда не виден труп, или труп насильно растворяется)
	float deadBodyTime;		//Сколько пролежал в смерти(всего)

	bool  bAutoReleaseRagdollOn;
	float fTimeToReleaseRagDoll;
	float fRotDlt;

	MOSafePointer waterLevel;

	struct BltVertex
	{
		Vector p;
		dword color;
		float u, v;		
	};

	struct TBltTrace
	{
		bool Show;
		float fTime;
		float fMaxTime;
		float fSpeed;

		Vector BulletStart;
		Vector BulletEnd;

		float  fPitchAngle;
		float  fAngle;

		float  damage;

		float scaleV;
		float scaleH;
	};
	
	TBltTrace BltTrace;

	enum TCharacterCoin
	{		
		chrcoin_gun_bomb = 0,
		chrcoin_nogun_bomb = 1,
		chrcoin_gun_nobomb = 2,
		chrcoin_nogun_nobomb = 3,
		chrcoin_fatality = 4,
		chrcoin_none = 5
	};	

	float fBloodDamage;

	float fCoinTime;
	float fCoindDltHgt;
	float fCointAltetude;
	bool  bShowCoinED;

	TCharacterCoin CharCoin;
	TCharacterCoin LastCharCoin;

	int coinModelIndex;
	float fCoinAngle;
	void SetCharacterCoin(TCharacterCoin coin);

	long hips_boneIdx;
	long spine_boneIdx;
	
	bool bDraw;
	
	bool bSelfDestructRagdoll;
	Vector rd_last_pos;
	Vector rd_last_pos2;
	Vector rd_last_dir;
	float  rd_last_speed;
	float  rd_last_timeout;
	bool   rd_inited;	

	Vector last_frame_pos;
	float  last_dist2camera;		//Квадрат дистанции до камеры

	Vector last_bone_pos;
	Vector last_pos;
	float  last_bone_ay;

	bool showProcess;
	bool activeProcess;
	
	Character* pair_minigame[20];

	MissionObject* actor_attach;

	struct TBomb
	{
		Vector pos;								//Позиция бонуса
		Vector velocity;						//Скорость и направление полёта
		float dltTime;							//Остаток dltTime для физики
		bool isSleep;							//Перемещается бонус или остановился
		float radius;
	};

	TBomb bomb;

	bool need_post_grab_weapon;	
	int pgw_wp_index;
	const char* pgw_start_node;
	float pgw_blend_time;
	Character* pgw_victim;

	bool graph_changed;
	int  act_node;

	
	//Бросить бомбу
	void _cdecl DrawCoin(float dltTime);

	void SetDebugInfo(int type);
	void _cdecl DrawDebugInfo(float dltTime);

	void StartButtonMG(MiniGame_Desc& desc, bool auto_win);
	void UpdateMG(float dltTime);
	void EndButtonMG();

	//Воздействовать на объект сферой
	virtual bool Attack(MissionObject * obj, dword source, float hp, const Vector & center, float radius);
	
	//Воздействовать на объект линией
	virtual bool Attack(MissionObject * obj, dword source, float hp, const Vector & from, const Vector & to);
	
	//Воздействовать на объект выпуклым чехырёхугольником
	virtual bool Attack(MissionObject * obj, dword source, float hp, const Vector vrt[4]);
	
	virtual void Hit(dword source, float dmg, const char* reaction, float block_dmg, const char* blockReaction, const char * deathLink);

	void  SetAddColor(Color color) { add_color = color; };
	Color GetAddColor() { return add_color; };	

	void  ShowDebugInfo(int type);
	
	void  ShowControllerDebugInfo(bool show);
	void _cdecl DrawControllerDebugInfo(float dltTime);

	void  BloodStain(float dmg, Character * offender = null);	

	bool CollisionLine(const Vector & start, const Vector & end, Vector & p, Vector & n);		
	void _cdecl FlyBomb(float dltTime);

	void _cdecl  UpdatePhysObj(float dltTime, long level);

	float GetAlpha();

	void DisableTimeEvent();
	void DeattachChar();

private:

	bool was_showed;

#ifndef NO_CONSOLE

	struct DebugMessageInfo
	{
		dword color;
		char str[256];
	};

	array<DebugMessageInfo> debugMessages;
	float debugMessageShowTime;
	
#endif

public:
	bool isRealFatalityStarted;

public:
	static bool g_bFreezeRagdolls;
	ConstString killerId;
	static CommandsTable commandsTable;
	static const char * comment;
};


#endif


