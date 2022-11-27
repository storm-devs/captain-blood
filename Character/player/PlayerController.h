//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// Character
//===========================================================================================================================
// PlayerController	
//============================================================================================

#ifndef _PlayerController_h_
#define _PlayerController_h_

#include "..\Character\CharacterController.h"
#include "..\Character\Character.h"
#include "..\Character\Components\CharacterItems.h"
#include "..\..\common_h\IMissionPhysObject.h"
#include "..\..\common_h\IGUIElement.h"

class  Character;
class  QuickEvent;


class PlayerController : public CharacterController
{
	class AnimListener : public IAnimationListener
	{
	public:
		PlayerController* pPlayerController;

		AnimListener()
		{
			pPlayerController = NULL;
		}

		void _cdecl KickBody(IAnimation * ani, const char * name, const char ** params, dword numParams)
		{
			pPlayerController->KickBody();
		}

		void _cdecl PickupWeapon(IAnimation * ani, const char * name, const char ** params, dword numParams)
		{
			pPlayerController->PickUpWeapon();
		}

		void _cdecl MoneyBeatUp(IAnimation * ani, const char * name, const char ** params, dword numParams)
		{
			pPlayerController->MoneyBeatUp();
		}

		void _cdecl DropWeapon(IAnimation * ani, const char * name, const char ** params, dword numParams)
		{
			if (numParams<1)
			{
				return;
			}

			if (params[0][0] == 'd')
			{
				pPlayerController->DropWeapon(true);
			}
			else
			if (params[0][0] == 'b')
			{	
				pPlayerController->chr.items->DamageWeapon(false);
				pPlayerController->DropWeapon(false);				
			}
			else
			{
				pPlayerController->chr.items->DamageWeapon(false);
				pPlayerController->BrokeWeapon(true);
			}

		}		
	};

	AnimListener animListener;

	
	
	
	enum ChrCodes
	{
		ChrLeftRight,
		ChrForwBack,
		ChrMouseLeftRight,
		ChrMouseForwBack,
		ChrStrafeLeftRight,
		ChrStrafeForwBack,
		ChrStrafe,		
		ChrA,
		ChrB,
		ChrBlock,		
		ChrInstanceShoot,		
		ChrInstanceDropBomb,
		ChrRage,
		ChrAccept,
		ChrGotoHell
	};	

	struct TMissonObjRef
	{		
		ConstString objectID;
		MOSafePointer object;

		TMissonObjRef()
		{
			object.Reset();
		};

		void SetName(const ConstString & name,MissionObject* mo)
		{
			objectID = name;
			Validate(mo);
		}

		void Validate(MissionObject* mo)
		{
			if(!object.Validate())
			{							
				mo->FindObject(objectID, object);	
			}
		};

		void Show(MissionObject* mo,bool show, bool alwaysShow = false)
		{
			Validate(mo);

			if (object.Ptr())
			{
				if (object.Ptr()->IsShow() != show || alwaysShow) object.Ptr()->Show(show);
			}
		};

		bool IsShow(MissionObject* mo)
		{
			Validate(mo);

			if (object.Ptr())
			{
				return object.Ptr()->IsShow();
			}

			return false;
		};

		void Command(MissionObject* mo,const char * id, dword numParams, const char ** params)
		{
			Validate(mo);

			if (object.Ptr())
			{
				object.Ptr()->Command(id, numParams, params);
			}
		};

		void SetPercentage(MissionObject* mo,float percent)
		{
			Validate(mo);

			if (!object.Ptr()) return;			

			const char* params[2];

			params[0] = "1000";
			object.Ptr()->Command("SetMax",1,params);			

			char str[32];			

			crt_snprintf(str, sizeof(str),"%f", percent * 1000.0f);
			params[0] = str;

			object.Ptr()->Command("SetHP",1,params);
		};

		bool IsPlayingAnimation(MissionObject* mo)
		{
			Validate(mo);
		
			if (object.Ptr())
			{
				MO_IS_IF(id_GUIWidget, "GUIWidget", object.Ptr())
				{
					return ((IGUIElement*)object.Ptr())->IsPlayingAnimation();	
				}
			}

			return false;
		};
	};

public:

	struct wp_Cache
	{
		ConstString id;
		TWeaponType weapon_type;		
		const char* model_name;
		IGMXScene*  model;
		const char* model_part1_name;
		IGMXScene*  model_part1;
		const char* model_part2_name;
		IGMXScene*  model_part2;
		ConstString tip_id;

		wp_Cache()
		{
			model = null;
			model_part1 = null;
			model_part2 = null;
		}

		~wp_Cache()
		{
			RELEASE(model);
			RELEASE(model_part1);
			RELEASE(model_part2);		
		}
	};

	array<wp_Cache> wp_cache;

	struct wpHP_Params
	{
		ConstString diff;
		float       life;		
	};

	struct TWeaponStyle
	{		
		TWeaponStyle()
		{
			trail_tex = null;
		}

		~TWeaponStyle()
		{
			RELEASE(trail_tex);
		}

		int cur_wphp_params;
		int num_wphp_params;
		wpHP_Params wpHP_params[5];

		ConstString anim_id;
		TWeaponType weapon_type;
		ConstString locator_name;		
		ConstString particale_name;
		const char* trail_name;
		const char* trail_technique;
		float		trail_power;
		ConstString break_sound;	
		ConstString effectTable;		
		IBaseTexture* trail_tex;

		TMissonObjRef widgetTake;
		TMissonObjRef widgetDamaged;
		TMissonObjRef widgetDrop;
	};	

private:
	bool curGunNumber;
	array<TWeaponStyle> wp_styles;	
public:
	
	enum MoveState
	{
		ms_stop,
		ms_walk,
		ms_run,
		ms_strafe
	};	
			
	float fBombCooldown;	
	bool  bRestoreBomb;
	float fPistolCooldown;	

	bool isSlomoHit;
	int SlomoChecks;

	ICoreStorageFloat*  gsPistolTimeReload;
	ICoreStorageFloat*  gsPistolDamage;
	ICoreStorageFloat*  gsNumBombs;
	ICoreStorageFloat*  gsMaxBombs;
	ICoreStorageFloat*  gsHPLive;
	ICoreStorageFloat*  gsCurHPLive;
	ICoreStorageFloat*  gsCurRage;
	ICoreStorageFloat*  gsRageTimeAction;
	ICoreStorageString* gsPlayerShoot;
	ICoreStorageFloat*  gsControlType;
	ICoreStorageString* gsGameHint;
	ICoreStorageFloat*  gsTotalMoney;
	ICoreStorageFloat*  gsMouseSens;

	bool isDropStarted;
	bool watchCollisionWithPhysObject;
	Vector prevPhysObjPos;

	float startGold;

	bool  vanoHackDropPhysObjectWhenActor;

	float fDamageInGroundHit;
	float fDamageInPhysObjHit;
	float fDamageInPhysObjRadius;
	float fPhysObjDetectRadius;

	ICoreStorageFolder* gsPlayerAttackA;
	ICoreStorageFolder* gsPlayerAttackB;

	struct RestoreHP_Params
	{
		ConstString diff;
		float when_resore_HP;
		float restore_rate_HP;
		float time_to_restore_HP;
		float restore_rate_freq_HP;
	};

	int cur_reshp_params;
	int num_reshp_params;
	RestoreHP_Params restorehp_params[5];


	long lKeyCodes[17];

	int picked_item;
	IMissionPhysObject* phys_object;
	Vector last_pho_pos;

	struct TComboMoto
	{
		int iHits;
		const char* moto;
	};

	array<TComboMoto> comboMoto;
	array<TComboMoto> comboRageMoto;

	bool		   itemTaken;
	QuickEvent*    qevent;

	TWeaponStyle* cur_weapon_style;	

	int time_to_attack_notify;

private:

	Vector dir;	
	MoveState move;

	bool bIfTehereLag;	
	Matrix mLastViewMatrix;
	Vector vLastDir;

	float  fStrafeCooldown;	

	MGIterator * bombTargets;

	

	Vector delta_pair_v;
	Vector last_pair_v;

	bool   bAllowShoot;
	bool   bAllowBomb;

	int    NumAtackers;	

	bool	isShow;

	MissionObject* bomb_trg;	
	float          bomb_damage;
	float          bomb_radius;

	float fPistolDamage;
	float fRifleDamage;
	
	TMissonObjRef  pistolbar;
	float		   pistolBarBeg;
	float		   pistolBarEnd;
	
	TMissonObjRef  pistolwidget;
	TMissonObjRef  pistolfullwidget;

	TMissonObjRef  bombbar;
	float		   bombBeg;
	float		   bombSz;

	TMissonObjRef  ragebar;
	TMissonObjRef  rageaction;	

	TMissonObjRef  weaponlifeBar;	

	int            iPlayerNum;
	char           sPlayerNum[4];	
	bool           traceBox;
	float          timeTotraceBox;
	float          fPhysObj_PushZ;
	float          fPhysObj_PushY;

	float combo_cur_timespawn;	
	float combo_timespawn;
	int   combo_times;	
	int   combo_times_max;	
	float combo_cur_time_dec;
	float combo_time_dec;

	int   combo_rage_times_max;

	TMissonObjRef  combo_widget;	
	TMissonObjRef  combo_moto_widget;	

	int            need_to_add_combo;	
	int            combo_add_tick;
	float          combo_ticked;
	int            cur_combo_add_tick;

	Character*	   chr_showed_HUD;
	Character*	   chr_need_show_HUD;
	float		   chr_HUD_cooldown;
	MissionObject* hpBar;

	ICoreStorageFloat* gsMoney;
	float              need_to_add_money;	
	float              money_add_tick;
	float              money_ticked;
	float              cur_money_add_tick;

	float              money_txt_cooldown;
	TMissonObjRef      money_widget;	

	float          slomo_time;
	MissionObject* mission_time;
	ConstString    missiontime_name;

	float          cooldown_to_dropweapon;
	float          slomo_cooldown;	

	int            cur_mombo;
	float          time_to_hide_mombo;
	float          cur_time_to_hide_mombo;

	float          cur_time_to_restore_HP;
	float          cur_restore_rate_HP;

	struct TSloMoSubMarker
	{		
		int id;
		Vector s;
		Vector e;		
	};

	struct TSloMoMarker
	{		
		int id;

		array<TSloMoSubMarker> markers;

		TSloMoMarker():markers(_FL_)
		{
			id = -1;
		}
	};

	int cur_marker;
	array<TSloMoMarker> slomo_markers;

	ISound* snd_Slow_Motion_In;
	ISound* snd_Slow_Motion_Loop;
	ISound* snd_Slow_Motion_Out;
	MOSafePointer snd_Slow_Motion_Env;

	TMissonObjRef  widgetMainWeapon;	

	float          delayShowCurWpWidget;
	float          dmgMoneyBeatUp;	

	bool  bSlowDownTime;

	int   iHitsToRage;	
	float fCurRageTime;	

	MissionTrigger* triger_RageOn;
	MissionTrigger* triger_RageAttack;
	MissionTrigger* triger_RageOff;
	MissionTrigger* triger_RageFull;

	float time_to_run_fast;

	bool  useGameState;	
	bool  isBlood;

	float mouse_dir_time;
	float mouse_dir_x;
	float mouse_dir_z;
	Vector vLastMsDir;

	float   fMouseUpdate;
	int     iMouseTimes;
	float   fCurMouseValueV;
	float   fCurMouseValueH;
	float   fMouseFilteredV;
	float   fMouseFilteredH;

	float   ms_dead_zone;
	float   ms_scale;
	float   ms_inertion;
	Vector  last_ms_dir;
	
//--------------------------------------------------------------------------------------------
public:
	PlayerController(Character & ch, const char * name);
	virtual ~PlayerController();

	//Vano: выбрасывает оружие, физ. объекты и гренадиров из Блада
	void DropWeaponBoxSlave();

	//Обновить состояние контролера
	virtual void Update(float dltTime);	
		
	//Взрыв брошеной персонажем бомбы
	virtual void Boom(const Vector & pos);

	virtual void Shoot(float dm);

	virtual bool AllowToBeBlowed() { return false; };

	virtual void Reset();

	virtual void Show(bool show);

	virtual void Death();

	virtual int  GetNumAtackers() { return NumAtackers; };
	virtual void SetNumAtackers(int _NumAtackers) { NumAtackers = Max(0, _NumAtackers); };
	
	virtual void Attack(Character* _target);

	virtual void Hit(float dmg, Character* _offender, const char* reaction, DamageReceiver::DamageSource source);

	virtual bool IsWeaponOrPhysicsBoxEquiped();

	virtual void PickUpWeapon();
	virtual void DropWeapon(bool add_impulse);
	virtual void BrokeWeapon(bool need_anim_blend);
	virtual void AnimBlend();

	virtual bool TakeWeapon(TWeaponType wp_type,const char* weapon_model, 
							const char* broken_part1,const char* broken_part2,
							const ConstString & tip_id,float blend_time, const char* node = null);


	virtual bool TakeChachedWeapon(const ConstString & chaced_wp_id, 
							       float blend_time, const char* node = null);


	virtual void DrawHUD(float dltTime);

	virtual void AddMoney(float add_money);

	virtual void AddBomb(int bombs);
	virtual bool IsMaxBombs();
	virtual void ChargePistol();
	virtual void QuadDamage(float work_time);

	virtual void AddRage(int rage_tick);
	virtual bool IsFullRage();

	float GetLastWeaponLive();

	__forceinline int GetPlayerNum() { return iPlayerNum; }

	void ShowCurWeaponStateWidget();
	void ShowCurWeaponWidget(bool show);
	void MoneyBeatUp();

	bool IsSlowmoReady() const;

	void CheckSlowmo(const CharacterController::SlowmoParams & smp);

	void BeginSloMo(int id,float time);
	void AddSloMoMarker(int id, int sub_id, const ConstString & wp_id);	
	void StartSloMo(float time, const char * timeScale = "0.3");
	void EndSloMo(bool instance = false);

	void EndQuickEvent();
	void RestoreSecodWeapon();

	void GiveMeAllINeed();

	bool CanTakeWeapon() { return useGameState; };

	void ActivateTriger(MissionTrigger* triger);

	void  RemoveSecondWeapon();
	bool  AllowAccpet();

	void  UpdatePhysObject();

	void  HideWidgets();

	void NotifyCharAboutAttack();

	void BeginRage();
	void EndRage();

private:
	//Получить направление и скорость перемещения
	void GetMove(float timedelta);
	//Переместиться
	void Move(float dltTime);
	//Бой холодным оружием
	void Fight();
	//Парные анимации
	void Pair(float dltTime, bool doKick, int filter, bool weaponDropped = false);
	//Применить
	bool Accept();
	//Выстрелить
	void Fire();
	//Бросание бомбы
	void Bomb();

	// Взорвать ящик, бочку и т.п. кинутое оружие
	void BlowPhysObject(const Vector & pos, bool splashDamage = true, bool isAddAchievement = true);

	//Подобранное оружие
	bool Weapons(float dltTime, bool & weaponDropped);	

	void KickBody();		

	//Можно ли перемещатся
	bool IsCanMove();
	//Найти цель среди персонажей в массиве find
	Character * FindCharacterTarget();
	//Найти цель для бомбы
	MissionObject * FindBombTarget(bool CheckBombDistance=true);
	//Получить коэфициент веса
	float GetTargetWeight(float trgCos, float trgDist);		

	void FillControlCode(int code,const char* btn_name);

	virtual const ConstString &  GetAIParamsName(){ RETURN_CONSTSTRING("PlayerParams"); }

	virtual void   SetParams(CharacterControllerParams * params);	
	virtual void   Command(dword numParams, const char ** params);	

	virtual const ConstString & GetControllerState() { RETURN_CONSTSTRING("Player control"); }

	virtual void ShowDebugInfo();	

	void EnableRageMode(bool enable);	

	IMissionPhysObject* FindClosestPO(float& cur_dy, bool check_angle = true);
	const char* GetAttackLinkName(ICoreStorageFolder* gsPlayerAttack, const char* nodeName, const char* defLink);	

	void  HUDWork(float dltTime);

	char* CorrectPlayerBDKey(const char* key);	

	void FindCurHPParam();
	
	void FindWpHPParam();
	float GetWpHPParam();

	void  SetGameHint();

private:

	ITipsManager* tip_manager;

	static float fBombTimeReload;
	static float fStrafeWaitTime;

	static float thresholdToWalk;
	static float thresholdFromWalk;
	static float thresholdToRun;
	static float thresholdFromRun;

	static float pairDistance;	
};

#endif

