//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// Character
//===========================================================================================================================
// PlayerController	
//============================================================================================
	 		

#include "PlayerController.h"
#include "..\Character\Components\CharacterLogic.h"
#include "..\Character\Components\CharacterPhysics.h"
#include "..\Character\Components\CharacterAnimationEvents.h"
#include "..\Character\Components\CharacterAnimation.h"
#include "..\Character\Components\CharacterAchievements.h"

#include "..\auxiliary objects\bombtarget\BombTarget.h"
#include "..\auxiliary objects\Quick Events\QuickEvent.h"

#include "..\auxiliary objects\arbiter\CharactersArbiter.h"

#include "..\Utils\StorageWork.h"


DeclareCharacterController(PlayerController, "Player", 1)

//============================================================================================

float PlayerController::thresholdToWalk = 0.4f;
float PlayerController::thresholdFromWalk = 0.3f;
float PlayerController::thresholdToRun = 0.6f;
float PlayerController::thresholdFromRun = 0.5f;
float PlayerController::pairDistance = 1.5f;

float PlayerController::fBombTimeReload = 5.0f;

float PlayerController::fStrafeWaitTime = 1.0f;

const ConstString str_secondId("second");
const ConstString str_2gunId("2gun");
static const ConstString str_locator17Id("locator17");
static const ConstString str_locator9Id("locator9");
static const ConstString str_rapireId("rapire");

//============================================================================================

class PlayerParams : public CharacterControllerParams
{

public:

	struct TWpType
	{
		int num_wphp_params;
		PlayerController::wpHP_Params wpHP_params[5];

		ConstString weapon_type;		
		const char* animation;

		ConstString locator_name;		

		const char* trail_name;
		const char* trail_technique;
		float		trail_power;

		ConstString particale_name;
		ConstString break_sound;

		ConstString effectTable;

		ConstString widgetTakeName;
		ConstString widgetDamagedName;
		ConstString widgetDropName;
	};

	array<TWpType> wp_types;

	array<PlayerController::wp_Cache> wp_cache;

	int   iPlayerNum;	
	float money_add_tick;	
	float bomb_damage;
	float bomb_radius;
	bool  bRestoreBomb;
	float slomo_cooldown;
	ConstString missiontime_name;	
	
	int num_reshp_params;
	PlayerController::RestoreHP_Params restorehp_params[5];

	ConstString money_widget_name;
	ConstString rage_widget_name;
	ConstString rage_action_widget_name;	
	ConstString combo_widget_name;
	ConstString combo_moto_widget_name;

	ConstString pistol_widget_name;
	ConstString pistol_full_widget_name;
	ConstString weapon_life_name;


	array<PlayerController::TComboMoto> comboMoto;
	array<PlayerController::TComboMoto> comboRageMoto;

	ConstString	widgetMainWeaponName;

	bool  useGameState;
	bool  isBlood;
		
	ConstString pistolBarName;
	float pistolBarBeg;
	float pistolBarEnd;
	
	ConstString	bombName;
	float bombBeg;
	float bombSz;

	float fPistolDamage;
	float fRifleDamage;

	float fDamageInGroundHit;
	float dmgMoneyBeatUp;

	float fDamageInPhysObjHit;
	float fDamageInPhysObjRadius;
	float fPhysObjDetectRadius;

	int   iHitsToRage;
	float fRageAttackPow;
	float fRageHitPow;	

	MissionTrigger triger_RageOn;
	MissionTrigger triger_RageAttack;
	MissionTrigger triger_RageOff;
	MissionTrigger triger_RageFull;
	
	float combo_timespawn;	
	float combo_time_dec;

	float fPhysObj_PushZ;
	float fPhysObj_PushY;

	float autoZoomLerpSpeed;
	float autoZoomAngSpeed;

	PlayerParams::PlayerParams() : wp_types(_FL_, 1), comboMoto(_FL_, 1), comboRageMoto(_FL_, 1), wp_cache(_FL_, 1)
	{

	}

	virtual bool IsControllerSupport(const ConstString & controllerClassName)
	{
		CHECK_CONTROLLERSUPPORT("PlayerParams")
	}

	bool Create(MOPReader & reader)
	{
		Init(reader);
		return true;
	}

	bool EditMode_Create(MOPReader & reader)
	{
		Init(reader);
		return true;
	}

	bool EditMode_Update(MOPReader & reader)
	{
		Init(reader);
		return true;
	}


	void Init (MOPReader & reader)
	{	
		ConstString player = reader.Enum();

		iPlayerNum=1;

		static const ConstString p2Id("Player2");
		static const ConstString p3Id("Player3");
		static const ConstString p4Id("Player4");

		if ( player == p2Id)
		{
			iPlayerNum=2;
		}
		else
		if ( player == p3Id )
		{
			iPlayerNum=3;
		}
		else
		if (player == p4Id)
		{
			iPlayerNum=4;
		}

		money_add_tick = reader.Float();	
		
		bomb_damage = reader.Float();
		bomb_radius = reader.Float();
		bRestoreBomb = reader.Bool();
		slomo_cooldown = reader.Float();
		missiontime_name = reader.String();		

		useGameState = reader.Bool();
		isBlood = reader.Bool();

		num_reshp_params = reader.Array();

		for (int i=0;i<num_reshp_params;i++)
		{
			restorehp_params[i].diff = reader.String();
			restorehp_params[i].when_resore_HP = reader.Float();
			restorehp_params[i].restore_rate_HP = reader.Float();
			restorehp_params[i].restore_rate_freq_HP = reader.Float();
			restorehp_params[i].time_to_restore_HP = reader.Float();		
		}
		
		money_widget_name = reader.String();
		rage_widget_name = reader.String();
		rage_action_widget_name = reader.String();
		combo_widget_name = reader.String();
		combo_moto_widget_name = reader.String();
		
		pistol_widget_name = reader.String();
		pistol_full_widget_name = reader.String();
		weapon_life_name = reader.String();

		int num_motos = reader.Array();
		comboMoto.DelAll();
		comboMoto.AddElements(num_motos);

		for (int i=0;i<num_motos;i++)
		{
			PlayerController::TComboMoto & moto = comboMoto[i];
			moto.iHits = reader.Long();
			moto.moto = reader.LocString();

			//comboMoto.Add(moto);
		}

		num_motos = reader.Array();
		comboRageMoto.DelAll();
		comboRageMoto.AddElements(num_motos);

		for (int i=0;i<num_motos;i++)
		{
			PlayerController::TComboMoto & moto = comboRageMoto[i];
			moto.iHits = reader.Long();
			moto.moto = reader.LocString();

			//comboRageMoto.Add(moto);
		}		

		widgetMainWeaponName = reader.String();

		pistolBarName = reader.String();
		pistolBarBeg = reader.Float();
		pistolBarEnd = reader.Float();

		bombName = reader.String();
		bombBeg = reader.Float();
		bombSz = reader.Float();

		fRifleDamage = reader.Float();
		fPistolDamage = reader.Float();		

		fDamageInGroundHit = reader.Float();
		dmgMoneyBeatUp = reader.Float();

		fDamageInPhysObjHit = reader.Float();
		fDamageInPhysObjRadius = reader.Float();
		fPhysObjDetectRadius = reader.Float();


		iHitsToRage = reader.Long();
		fRageAttackPow = reader.Float();
		fRageHitPow = reader.Float();

		triger_RageOn.Init(reader);
		triger_RageAttack.Init(reader);
		triger_RageOff.Init(reader);
		triger_RageFull.Init(reader);

		combo_timespawn = reader.Float();		
		combo_time_dec = reader.Float();

		fPhysObj_PushZ = reader.Float();
		fPhysObj_PushY = reader.Float();

		autoZoomLerpSpeed = reader.Float();
		autoZoomAngSpeed = reader.Float();

		int num_types = reader.Array();
		wp_types.DelAll();
		wp_types.AddElements(num_types);

		for (int i=0;i<num_types;i++)
		{
			TWpType & wp_type = wp_types[i];	

			wp_type.weapon_type = reader.Enum();
			wp_type.animation = reader.String().c_str();
			wp_type.locator_name = reader.String();

			wp_type.num_wphp_params = reader.Array();

			for (int j=0;j<wp_type.num_wphp_params;j++)
			{
				wp_type.wpHP_params[j].diff = reader.String();
				wp_type.wpHP_params[j].life = reader.Float();
			}

			wp_type.particale_name = reader.String();
			wp_type.break_sound = reader.String();

			wp_type.trail_name = reader.String().c_str();

			wp_type.trail_technique = reader.Enum().c_str();
			wp_type.trail_power = reader.Float();
			
			wp_type.effectTable = reader.String();

			wp_type.widgetTakeName = reader.String();
			wp_type.widgetDamagedName = reader.String();
			wp_type.widgetDropName = reader.String();
		}

		int num_ch_wp = reader.Array();
		wp_cache.DelAll();
		wp_cache.AddElements(num_ch_wp);

		for (int i=0;i<num_ch_wp;i++)
		{
			PlayerController::wp_Cache& wp_c = wp_cache[i];//wp_cache.Add()];

			wp_c.id = reader.String();
			wp_c.weapon_type = GetWeaponType(reader.Enum().c_str());
			wp_c.model_name = reader.String().c_str();
			wp_c.model_part1_name = reader.String().c_str();
			wp_c.model_part2_name = reader.String().c_str();
			wp_c.tip_id = reader.String();
		}
	}
};

MOP_BEGINLISTCG(PlayerParams, "Player params", '1.00', 90, "Player params", "Character")

	MOP_ENUMBEG("Player")
		MOP_ENUMELEMENT("Player1")
		MOP_ENUMELEMENT("Player2")
		MOP_ENUMELEMENT("Player3")
		MOP_ENUMELEMENT("Player4")
	MOP_ENUMEND

	MOP_ENUMBEG("WeaponType")		
		MOP_ENUMELEMENT("Sword")
		MOP_ENUMELEMENT("Sword2")
		MOP_ENUMELEMENT("Sword3")
		MOP_ENUMELEMENT("Axe")
		MOP_ENUMELEMENT("Axe2")
		MOP_ENUMELEMENT("Axe3")
		MOP_ENUMELEMENT("Twohanded")
		MOP_ENUMELEMENT("Twohanded2")
		MOP_ENUMELEMENT("Twohanded3")
		MOP_ENUMELEMENT("Rifle")
		MOP_ENUMELEMENT("Gun")
		MOP_ENUMELEMENT("Rapire")
		MOP_ENUMELEMENT("Box")
		MOP_ENUMELEMENT("Bombardier Body")
		MOP_ENUMELEMENT("Broken1")
		MOP_ENUMELEMENT("Broken2")
		MOP_ENUMELEMENT("Broken3")
		MOP_ENUMELEMENT("Broken4")
		MOP_ENUMELEMENT("Broken5")
		MOP_ENUMELEMENT("Broken6")
		MOP_ENUMELEMENT("Broken7")
		MOP_ENUMELEMENT("Broken8")
		MOP_ENUMELEMENT("Broken9")
	MOP_ENUMEND

	MOP_ENUMBEG("TrailTechnique")
		MOP_ENUMELEMENT("Blend")
		MOP_ENUMELEMENT("Add")
		MOP_ENUMELEMENT("Multiply")
	MOP_ENUMEND

	MOP_ENUM("Player", "Player")

	MOP_FLOATEXC("Tick rate of adding money", 100, 1,100000,"Tick rate per second of adding money")

	MOP_FLOATEXC("Bomb Damage", 25, 1,100000,"Damage of player bomb")
	MOP_FLOATEXC("Bomb Radius", 1.4f, 1,100000,"Damage Radius of player bomb")
	MOP_BOOLC("Restore Bombs", true, "Restore Players Bomb")
	MOP_FLOATEXC("Slomo Cooldown | v3 |", 22, 1, 100000,"Slomo Cooldown")
	MOP_STRINGC("Mission Time", "Mission Time", "Name of mission object of 'mission time' type")	

	MOP_BOOL("Use Gamestate", true)
	MOP_BOOL("Is Blood", true)


	MOP_ARRAYBEGC("HP Restore Params", 1, 5, "HP Restore")
		MOP_STRING("Difficulty", "")
		MOP_FLOATEXC("When Restore HP", 20, 1,100,"HP limit after restore HP")
		MOP_FLOATEXC("Restore HP", 20, 1,100,"How mach restore HP per second")
		MOP_FLOATEXC("Restore HP time", 3, 0.01,1000,"How much time must pass to add HP")
		MOP_FLOATEXC("Time to restore HP", 5, 1,1000,"How much time must pass before restore HP")
	MOP_ARRAYEND

	
	

	MOP_STRING("Money widget", "")
	MOP_STRING("Rage bar", "")
	MOP_STRING("RageAction bar", "")
	MOP_STRING("Combo widget", "")
	MOP_STRING("Combo moto", "")

	MOP_STRING("Pistol widget", "")
	MOP_STRING("Pistol Full widget", "")
	MOP_STRING("WeaponLife bar", "")

	MOP_ARRAYBEGC("Combo Motos", 0, 100, "Motos")
		MOP_LONGEX("Num Hits", 1, 1,1000)
		MOP_LOCSTRING("Moto")
	MOP_ARRAYEND

	MOP_ARRAYBEGC("Combo Rage Motos", 0, 100, "Motos")
		MOP_LONGEX("Num Hits", 1, 1,1000)
		MOP_LOCSTRING("Moto")
	MOP_ARRAYEND

	MOP_STRING("Main Weapon Widget", "")

	MOP_STRING("PistolBar Name", "PistolBar")
	MOP_FLOAT("Pistol Bar Begin", 3)
	MOP_FLOAT("Pistol Bar End", 97)	
	
	MOP_STRING("BombBar Name", "BombBar")
	MOP_FLOAT("Bomb Bar Begin", 38)
	MOP_FLOAT("Bomb Bar End", 11)


	MOP_FLOATEX("Rifle Damage", 120.0f, 0.0f,1000.0f)
	MOP_FLOATEX("Gun Damage", 30.0f, 0.0f,1000.0f)

	MOP_FLOATEX("Damage In Ground Hit", 10.0f, 0.0f,1000.0f)

	MOP_FLOATEX("Damge in Money BeatUp", 5,1.0f,100.0f)

	MOP_FLOATEX("Damage by PhysObj", 30,1.0f,10000.0f)
	MOP_FLOATEX("Damage Radius of PhysObj", 2.5,0.5f,100.0f)
	MOP_FLOATEX("PhysObj Detect Radius", 0.65f,0.01f,10.0f)

	MOP_LONGEX("Num ComboHits to Rage", 25, 1, 100204)	
	MOP_FLOATEX("Damage in Rage Percent", 150,0.1f,1000.0f)
	MOP_FLOATEX("Hit Damage in Rage Percent", 50,0.1f,1000.0f)

	MOP_MISSIONTRIGGERG("Rage On Triger", "Rage On")
	MOP_MISSIONTRIGGERG("Attack in Rage", "Attack in Rage")
	MOP_MISSIONTRIGGERG("Rage Off Triger", "Rage Off")
	MOP_MISSIONTRIGGERG("On Full Rage Triger", "Full Rage")

	MOP_FLOATEX("Time To DecCombo", 3,0.1f,1000.0f)	
	MOP_FLOATEX("ComboTimeDecrease", 0.3f,0.001f,1000.0f)

	MOP_FLOATEX("PhysObj PushZ", 33.0f,0.01f,1024.0f)	
	MOP_FLOATEX("PhysObj PushY",  7.0f,0.01f,1024.0f)

	MOP_FLOATEX("AutoZoom LerpSpeed", 6.0f,0.01f, 64.0f)
	MOP_FLOATEX("AutoZoom AngSpeed", 1.0f,0.01f, 64.0f)

	MOP_ARRAYBEGC("Weapons", 0, 100, "Weapon types")		
	    
		MOP_ENUM("WeaponType","Type")		
		MOP_STRINGC("Animation", "", "Animation for this type of weapon")		
		MOP_STRINGC("Locator", "locator9", "Name of Weapon Locator")

		MOP_ARRAYBEGC("Weapon HP", 1, 5, "Weapon HP")
			MOP_STRING("Difficulty", "")			
			MOP_FLOATC("HP", 10, "Number of Hits needed to break")
		MOP_ARRAYEND
		
		MOP_STRINGC("Break Particale", "sparksdamagefull.xps", "Particale showed when weapon became broke")
		MOP_STRINGC("Break Sound", "", "Sound played when weapon breaks")
		MOP_STRINGC("Trail Texture", "", "Texture of trail")
		MOP_ENUM("TrailTechnique", "Trail technique")
		MOP_FLOATC("Trail power", 1.0f, "")
		MOP_STRING("Effect Table", "")

		MOP_STRING("widget on Take", "")
		MOP_STRING("widget on Damaged", "")
		MOP_STRING("widget on Drop", "")
	
	MOP_ARRAYEND	

	MOP_ARRAYBEG("Chached Weapon", 0, 100)
		MOP_STRING("ID", "")
		MOP_ENUM("WeaponType","Type")
		MOP_STRINGC("Model", "", "Model name")
		MOP_STRINGC("Broken Part1", "", "Model name for first broken part")
		MOP_STRINGC("Broken Part2", "", "Model name for second broken part")
		MOP_STRINGC("TipID", "Weapon", "Identifier of tip")
	MOP_ARRAYEND

MOP_ENDLIST(PlayerParams)


PlayerController::PlayerController(Character & ch, const char * name) : CharacterController(ch, name), wp_styles(_FL_), slomo_markers(_FL_,32), comboMoto(_FL_), comboRageMoto(_FL_), wp_cache(_FL_)
{
	isShow = true;
	bomb_damage = 25.0f;
	bomb_radius = 1.4f;
	slomo_cooldown = 1.0;

	vanoHackDropPhysObjectWhenActor = false;
	watchCollisionWithPhysObject = false;
	isDropStarted = false;

	curGunNumber = true;

	fStrafeCooldown = 0.0f;
	dir = 0.0f;
	move = ms_stop;
	bombTargets = &chr.GroupIterator(GroupId('b','m','b','t'), _FL_);

	bomb_trg = NULL;

	NumAtackers = 0;

	delayShowCurWpWidget = -1.0f;

	animListener.pPlayerController = this;	
	chr.animation->SetEventHandler(&animListener, (AniEvent)(&AnimListener::KickBody), "KickBody");	
	chr.animation->SetEventHandler(&animListener, (AniEvent)(&AnimListener::MoneyBeatUp), "MoneyBeatUp");	
	
	lKeyCodes[ChrLeftRight] = chr.Controls().FindControlByName("ChrLeftRight1"); 
	lKeyCodes[ChrForwBack] = chr.Controls().FindControlByName("ChrForwBack1"); 


	lKeyCodes[ChrMouseLeftRight] = chr.Controls().FindControlByName("ChrMouseLeftRight1"); 
	lKeyCodes[ChrMouseForwBack] = chr.Controls().FindControlByName("ChrMouseForwBack1"); 			

	lKeyCodes[ChrStrafeLeftRight] = chr.Controls().FindControlByName("ChrStrafeLeftRight1"); 
	lKeyCodes[ChrStrafeForwBack] = chr.Controls().FindControlByName("ChrStrafeForwBack1");	

	lKeyCodes[ChrStrafe] = chr.Controls().FindControlByName("ChrStrafe1");	
	lKeyCodes[ChrA] = chr.Controls().FindControlByName("ChrA1"); 
	lKeyCodes[ChrB] = chr.Controls().FindControlByName("ChrB1"); 
	lKeyCodes[ChrBlock] = chr.Controls().FindControlByName("ChrBlock1"); 
	lKeyCodes[ChrRage] = chr.Controls().FindControlByName("ChrRage1"); 
	lKeyCodes[ChrAccept] = chr.Controls().FindControlByName("ChrAccept1"); 	
	lKeyCodes[ChrInstanceShoot] = chr.Controls().FindControlByName("ChrInstanceShoot1"); 	
	lKeyCodes[ChrInstanceDropBomb] = chr.Controls().FindControlByName("ChrInstanceDropBomb1"); 	
	lKeyCodes[ChrGotoHell] = chr.Controls().FindControlByName("ChrGotoHell1"); 	


	iPlayerNum = 1;
	sPlayerNum[0] = '1';
	sPlayerNum[1] = 0;
	
	fBombCooldown = 0.0f;
	fPistolCooldown = 0.0f;		

	static const ConstString pistolBarId("PistolBar");
	pistolbar.SetName(pistolBarId,&chr);
	pistolBarBeg = 0;
	pistolBarEnd = 100.0f;
	
	bombBeg = 257.0f/670.0f;
	bombSz = 74.0f/670.0f;

	static const ConstString bombBarId("BombBar");
	bombbar.SetName(bombBarId,&chr);	

	qevent = NULL;	

	itemTaken = false;

	picked_item = -1;
	phys_object = NULL;
	last_pho_pos = 0.0f;

	bRestoreBomb = true;
	
	need_to_add_money = 0;
	money_add_tick = 100;	
	money_ticked = 0.0f;

	cooldown_to_dropweapon = -1.0f;	

	dmgMoneyBeatUp = 0.05f;
	
	fDamageInGroundHit = 10.0f;

	need_to_add_combo = 0;
	combo_add_tick = 7;	
	combo_ticked = 0.0f;	
	combo_times = 0;	
	cur_combo_add_tick = 0;

	combo_timespawn = 3.0f;
	combo_times_max = 0;

	slomo_time = -10.0f;
	mission_time = null;

	snd_Slow_Motion_In = chr.Sound().Create("Slow_Motion_In",_FL_,false,false);
	snd_Slow_Motion_Loop = chr.Sound().Create("Slow_Motion_Loop",_FL_,false,false);
	snd_Slow_Motion_Out = chr.Sound().Create("Slow_Motion_Out",_FL_,false,false);	

	static const ConstString misSndEnv("MissionSoundEnvironmentManager");
	//chr.Mission().CreateObject(snd_Slow_Motion_Env,"MissionSoundEnvironmentManager", misSndEnv);
	chr.Mission().FindObject(misSndEnv, snd_Slow_Motion_Env);

	fDamageInPhysObjHit = 30;
	fDamageInPhysObjRadius = 2.5f;
	fPhysObjDetectRadius = 1.0f;

	cur_weapon_style = NULL;

	traceBox = false;
	timeTotraceBox = -1.0f;

	cur_marker = -1;

	bSlowDownTime = false;

	triger_RageOn = null;
	triger_RageAttack = null;
	triger_RageOff = null;
	triger_RageFull = null;

	combo_cur_time_dec = -1;

	chr_showed_HUD = null;
	chr_need_show_HUD = null;
	chr_HUD_cooldown = 0;
	hpBar = null;

	num_reshp_params = 1;
	cur_reshp_params = 0;
	restorehp_params[0].diff.Empty();
	restorehp_params[0].when_resore_HP = 0.2f;
	restorehp_params[0].restore_rate_HP = 0.02f;
	restorehp_params[0].time_to_restore_HP = 5.0f;
	restorehp_params[0].restore_rate_freq_HP = 1.0f;	

	cur_time_to_restore_HP = 0;
	cur_restore_rate_HP = 0;
	

	cur_mombo = -1;
	time_to_hide_mombo = 2;
	cur_time_to_hide_mombo = 0;

	gsPlayerAttackA = null;
	gsPlayerAttackB = null;
	gsPlayerShoot = null;

	time_to_run_fast = 0.0f;

	iHitsToRage = 10;
	fCurRageTime = 0;

	fPhysObj_PushZ = 33.0f;
	fPhysObj_PushY = 7.0f;

	gsPistolTimeReload = null;
	gsPistolDamage = null;
	gsNumBombs = null;
	gsMaxBombs = null;
	gsHPLive = null;
	gsCurHPLive = null;
	gsMoney = null;
	gsCurRage = null;
	gsRageTimeAction = null;
	gsTotalMoney = null;
	gsMouseSens = null;
											
	delta_pair_v = 0.0f;
	last_pair_v = 0.0f;

	bAllowShoot = false;
	bAllowBomb = false;

	combo_cur_timespawn = 0;
	combo_times = 0;
	combo_times_max = 0;
	combo_rage_times_max = 0;
				
	combo_time_dec = 0.02f;	

	fPistolDamage = 30.0f;
	fRifleDamage = 120.0f;

	useGameState = true;
	isBlood = true;

	mouse_dir_time = -1;
	mouse_dir_x = 0.0f;
	mouse_dir_z = 0.0f;

	ms_dead_zone = 0.3f;
	ms_scale = 5;
	ms_inertion = 0.2f;

	last_ms_dir = Vector(0.0f,0.0f,0.0f);	

	tip_manager = null;

	gsGameHint = null;
	gsControlType = null;

	time_to_attack_notify = 0;	
}

PlayerController::~PlayerController()
{
	RELEASE(gsPistolTimeReload);
	RELEASE(gsPistolDamage);
	RELEASE(gsNumBombs);
	RELEASE(gsMaxBombs);
	RELEASE(gsHPLive);
	RELEASE(gsCurHPLive);
	RELEASE(gsCurRage);
	RELEASE(gsRageTimeAction);
	RELEASE(gsPlayerShoot);
	RELEASE(gsControlType);
	RELEASE(gsGameHint);
	RELEASE(gsTotalMoney);
	RELEASE(gsMouseSens);
	RELEASE(gsPlayerAttackA);
	RELEASE(gsPlayerAttackB);
	RELEASE(gsMoney);

	RELEASE(bombTargets);
	RELEASE(snd_Slow_Motion_In);
	RELEASE(snd_Slow_Motion_Loop);
	RELEASE(snd_Slow_Motion_Out);

	/*for (int i=0;i<(int)wp_cache.Size();i++)
	{		
		RELEASE(wp_cache[i].model);
		RELEASE(wp_cache[i].model_part1);
		RELEASE(wp_cache[i].model_part2);		
	}*/

	/*for (int i=0;i<(int)wp_styles.Size();i++)
	{
		if (wp_styles[i].trail_tex) wp_styles[i].trail_tex->Release();
	}*/
}

void PlayerController::FillControlCode(int code,const char* btn_name)
{
	char ctrl_name[32];

	crt_snprintf(ctrl_name,31,"%s%s",btn_name,sPlayerNum);
	lKeyCodes[code] = chr.Controls().FindControlByName(ctrl_name); 
}

const char* PlayerController::GetAttackLinkName(ICoreStorageFolder* gsPlayerAttack, const char* nodeName, const char* defLink)
{
	if (gsPlayerAttack)
	{
		const char* link = gsPlayerAttack->GetString(nodeName,defLink);

		if (!string::IsEmpty(link))	return link;
	}

	return defLink;
}

char* PlayerController::CorrectPlayerBDKey(const char* key)
{
	static char cr_key[128];
	
	crt_strcpy(cr_key,sizeof(cr_key),"Player");

	if (iPlayerNum!=1)
	{
		crt_strcat(cr_key,sizeof(cr_key),sPlayerNum);
	}
	
	crt_strcat(cr_key,sizeof(cr_key),".");
	crt_strcat(cr_key,sizeof(cr_key),key);
	
	return chr.arbiter->CorrectStateId(cr_key);
}

void PlayerController::FindCurHPParam()
{
	cur_reshp_params = 0;

	const ConstString & diff = chr.arbiter->GetDifficulty();
	
	for (int i=0;i<num_reshp_params;i++)
	{
		if (diff == restorehp_params[i].diff)
		{
			cur_reshp_params = i;
			return;
		}
	}
}

void PlayerController::FindWpHPParam()
{
	if (!cur_weapon_style) return;

	cur_weapon_style->cur_wphp_params = 0;	

	const ConstString & diff = chr.arbiter->GetDifficulty();
	
	for (int i=0;i<cur_weapon_style->num_wphp_params;i++)
	{
		if (diff == cur_weapon_style->wpHP_params[i].diff)
		{
			cur_weapon_style->cur_wphp_params = i;
			return;
		}
	}

	
}

float PlayerController::GetWpHPParam()
{
	if (!cur_weapon_style) return -1;
	return cur_weapon_style->wpHP_params[cur_weapon_style->cur_wphp_params].life;
}

void PlayerController::RemoveSecondWeapon()
{
	if (itemTaken)
	{			
		if (phys_object)
		{
			phys_object->Activate(true);
			phys_object = null;
		}
		else
		{					
			chr.items->DelItems(str_secondId);	
		}		

		itemTaken = false;
		ShowCurWeaponWidget(false);
		cur_weapon_style = null;
	}
}

void PlayerController::SetParams(CharacterControllerParams * params)
{
	if (!params) return;
	
	PlayerParams* pl_params = (PlayerParams*)params;

	iPlayerNum = ((PlayerParams*)params)->iPlayerNum;
	crt_snprintf(sPlayerNum,sizeof(sPlayerNum),"%i",iPlayerNum);
	
	FillControlCode(ChrLeftRight,"ChrLeftRight");
	FillControlCode(ChrForwBack,"ChrForwBack");
	FillControlCode(ChrMouseLeftRight,"ChrMouseLeftRight");
	FillControlCode(ChrMouseForwBack,"ChrMouseForwBack");
	FillControlCode(ChrStrafeLeftRight,"ChrStrafeLeftRight");
	FillControlCode(ChrStrafeForwBack,"ChrStrafeForwBack");

	FillControlCode(ChrStrafe,"ChrStrafe");
	FillControlCode(ChrA,"ChrA");
	FillControlCode(ChrB,"ChrB");
	FillControlCode(ChrBlock,"ChrBlock");
	FillControlCode(ChrRage,"ChrRage");
	FillControlCode(ChrAccept,"ChrAccept");
	FillControlCode(ChrInstanceShoot,"ChrInstanceShoot");
	FillControlCode(ChrInstanceDropBomb,"ChrInstanceDropBomb");
	FillControlCode(ChrGotoHell,"ChrGotoHell");	

	money_add_tick = pl_params->money_add_tick;
	
	bomb_radius = pl_params->bomb_radius;
	bomb_damage = pl_params->bomb_damage;
	bRestoreBomb = pl_params->bRestoreBomb;
	slomo_cooldown = pl_params->slomo_cooldown;
	missiontime_name = pl_params->missiontime_name;	
			
	useGameState = pl_params->useGameState;
	isBlood = pl_params->isBlood;
	chr.logic->check_fatality_state = isBlood;

	num_reshp_params = pl_params->num_reshp_params;

	for (int i=0;i<num_reshp_params;i++)
	{
		restorehp_params[i].diff = pl_params->restorehp_params[i].diff;
		restorehp_params[i].when_resore_HP = pl_params->restorehp_params[i].when_resore_HP * 0.01f;		
		restorehp_params[i].restore_rate_freq_HP = pl_params->restorehp_params[i].restore_rate_freq_HP;
		
		restorehp_params[i].restore_rate_HP = pl_params->restorehp_params[i].restore_rate_HP / (pl_params->restorehp_params[i].restore_rate_freq_HP / restorehp_params[i].restore_rate_freq_HP) * 0.01f;
		
		restorehp_params[i].time_to_restore_HP = pl_params->restorehp_params[i].time_to_restore_HP;
	}	

	pistolwidget.SetName(pl_params->pistol_widget_name,&chr);
	pistolfullwidget.SetName(pl_params->pistol_full_widget_name,&chr);

	money_widget.SetName(pl_params->money_widget_name,&chr);
	ragebar.SetName(pl_params->rage_widget_name,&chr);
	rageaction.SetName(pl_params->rage_action_widget_name,&chr);

	weaponlifeBar.SetName(pl_params->weapon_life_name,&chr);

	combo_widget.SetName(pl_params->combo_widget_name,&chr);
	combo_moto_widget.SetName(pl_params->combo_moto_widget_name,&chr);

	comboMoto.Empty();

	for (int i=0;i<pl_params->comboMoto;i++)
	{
		comboMoto.Add(pl_params->comboMoto[i]);
	}

	for (int i=0;i<pl_params->comboRageMoto;i++)
	{
		comboRageMoto.Add(pl_params->comboRageMoto[i]);
	}

	widgetMainWeapon.SetName(pl_params->widgetMainWeaponName,&chr);
	

	pistolbar.SetName(pl_params->pistolBarName,&chr);
	pistolBarBeg = pl_params->pistolBarBeg;
	pistolBarEnd = pl_params->pistolBarEnd;

	bombbar.SetName(pl_params->bombName,&chr);
	bombBeg = pl_params->bombBeg;
	bombSz = (pl_params->bombSz - bombBeg) * 0.25f;

	fRifleDamage = pl_params->fRifleDamage;
	fPistolDamage = pl_params->fPistolDamage;

	fDamageInGroundHit = pl_params->fDamageInGroundHit;
	dmgMoneyBeatUp = pl_params->dmgMoneyBeatUp * 0.01f;

	fDamageInPhysObjHit = pl_params->fDamageInPhysObjHit;
	fDamageInPhysObjRadius = pl_params->fDamageInPhysObjRadius;
	fPhysObjDetectRadius = pl_params->fPhysObjDetectRadius;

	iHitsToRage = pl_params->iHitsToRage;
	chr.logic->fRageAttackPow = pl_params->fRageAttackPow * 0.01f;
	chr.logic->fRageHitPow = pl_params->fRageHitPow * 0.01f;

	combo_timespawn = pl_params->combo_timespawn;
    combo_time_dec = pl_params->combo_time_dec;

	triger_RageOn = &pl_params->triger_RageOn;
	triger_RageAttack = &pl_params->triger_RageAttack;
	triger_RageOff = &pl_params->triger_RageOff;
	triger_RageFull = &pl_params->triger_RageFull;

	fPhysObj_PushZ = pl_params->fPhysObj_PushZ;
	fPhysObj_PushY = pl_params->fPhysObj_PushY;

	if (chr.logic->autoZoom.Ptr())
	{
		((AutoZoom*)chr.logic->autoZoom.Ptr())->SetLerpSpeed(pl_params->autoZoomLerpSpeed);
		((AutoZoom*)chr.logic->autoZoom.Ptr())->SetAngSpeed(pl_params->autoZoomAngSpeed);
	}

	slomo_time = -slomo_cooldown;

	wp_styles.DelAll();	

	cur_weapon_style = null;
	wp_styles.AddElements(pl_params->wp_types.Size());

	for (int i=0;i<(int)pl_params->wp_types.Size();i++)
	{
		TWeaponStyle & wp_style = wp_styles[i];

		wp_style.anim_id = pl_params->wp_types[i].weapon_type;
		wp_style.weapon_type = GetWeaponType(wp_style.anim_id.c_str());
		IAnimation* wp_anim = chr.animation->AddAnimation(pl_params->wp_types[i].weapon_type.c_str(),pl_params->wp_types[i].animation);
		

		wp_style.locator_name = pl_params->wp_types[i].locator_name;

		wp_style.num_wphp_params = pl_params->wp_types[i].num_wphp_params;
		wp_style.cur_wphp_params = 0;

		for (int j=0;j<wp_style.num_wphp_params;j++)
		{
			wp_style.wpHP_params[j].diff = pl_params->wp_types[i].wpHP_params[j].diff;
			wp_style.wpHP_params[j].life = pl_params->wp_types[i].wpHP_params[j].life;
		}

		wp_style.particale_name = pl_params->wp_types[i].particale_name;
		wp_style.break_sound = pl_params->wp_types[i].break_sound;

		wp_style.trail_name = pl_params->wp_types[i].trail_name;

		wp_style.trail_tex = chr.Render().CreateTexture(_FL_, wp_style.trail_name);	

		wp_style.trail_technique = pl_params->wp_types[i].trail_technique;
		wp_style.trail_power = pl_params->wp_types[i].trail_power;

		wp_style.effectTable = pl_params->wp_types[i].effectTable;

		wp_style.widgetTake.SetName(pl_params->wp_types[i].widgetTakeName,&chr);
		wp_style.widgetDamaged.SetName(pl_params->wp_types[i].widgetDamagedName,&chr);
		wp_style.widgetDrop.SetName(pl_params->wp_types[i].widgetDropName,&chr);		

		if (wp_anim)
		{
			wp_anim->SetEventHandler(&animListener, (AniEvent)(&AnimListener::PickupWeapon), "WeaponPickUp");
			wp_anim->SetEventHandler(&animListener, (AniEvent)(&AnimListener::DropWeapon), "WeaponDrop");
		}		

		//wp_styles.Add(wp_style);
	}

	wp_cache.DelAll();
	wp_cache.AddElements(pl_params->wp_cache.Size());

	for (int i=0;i<(int)pl_params->wp_cache.Size();i++)
	{
		PlayerController::wp_Cache& wp_c = wp_cache[i];

		wp_c.id = pl_params->wp_cache[i].id;
		wp_c.weapon_type = pl_params->wp_cache[i].weapon_type;
		wp_c.model_name = pl_params->wp_cache[i].model_name;
		wp_c.model = chr.Geometry().CreateScene(wp_c.model_name, &chr.Animation(), &chr.Particles(), &chr.Sound(), _FL_);	 
		wp_c.model_part1_name = pl_params->wp_cache[i].model_part1_name;
		wp_c.model_part1 = chr.Geometry().CreateScene(wp_c.model_part1_name, &chr.Animation(), &chr.Particles(), &chr.Sound(), _FL_);	 
		wp_c.model_part2_name = pl_params->wp_cache[i].model_part2_name;
		wp_c.model_part2 = chr.Geometry().CreateScene(wp_c.model_part2_name, &chr.Animation(), &chr.Particles(), &chr.Sound(), _FL_);	 
		wp_c.tip_id = pl_params->wp_cache[i].tip_id;
	}

	GetStateFloatVar(CorrectPlayerBDKey("recharge"),gsPistolTimeReload);
	GetStateFloatVar(CorrectPlayerBDKey("pistoldamage"),gsPistolDamage);
	GetStateFloatVar(CorrectPlayerBDKey("bomb.slots"),gsMaxBombs);
	GetStateFloatVar(CorrectPlayerBDKey("bomb.count"),gsNumBombs);

	if (ReadGameState(gsMaxBombs,4.0f)<ReadGameState(gsNumBombs,4.0f))
	{
		WriteGameState(gsNumBombs,ReadGameState(gsMaxBombs,4.0f));
	}

	GetStateFloatVar(CorrectPlayerBDKey("live"),gsHPLive);
	GetStateFloatVar(CorrectPlayerBDKey("curHP"),gsCurHPLive);	
	GetStateFloatVar(CorrectPlayerBDKey("gold"),gsMoney);
	
	money_ticked = ReadGameState(gsMoney,0.0f);

	GetStateFloatVar(CorrectPlayerBDKey("rage.value"),gsCurRage);
	GetStateFloatVar(CorrectPlayerBDKey("rage.TimeAction"),gsRageTimeAction);
	GetStateFolder(chr.arbiter->CorrectStateId("Player.Attack.A"),gsPlayerAttackA);
	GetStateFolder(chr.arbiter->CorrectStateId("Player.Attack.B"),gsPlayerAttackB);
	
	GetStateStringVar(chr.arbiter->CorrectStateId("Player.Attack.Shoot"),gsPlayerShoot);
	GetStateStringVar("Profile.Runtime.GameHint",gsGameHint);
	GetStateFloatVar("Runtime.Land", gsControlType);
	GetStateFloatVar(chr.arbiter->CorrectStateId("EarnedMoney"),gsTotalMoney);
	GetStateFloatVar("Options.CharacterSensitivity",gsMouseSens);

	startGold = (gsMoney && gsMoney->IsValidate()) ? gsMoney->Get() : 0.0f;

	tip_manager = ITipsManager::GetManager(&chr.Mission());

	/*IFileService * pFS = (IFileService*)api->GetService("FileService");

	IIniFile * pIniFl = pFS->OpenIniFile("Ini/CharacterMouse.ini",_FL_);
			
	if (pIniFl)
	{	
		ms_dead_zone = pIniFl->GetFloat("CharacterMouse", "MouseDeadZone", 0.4f);
		ms_scale = pIniFl->GetFloat("CharacterMouse", "MouseScale", 5);
		ms_inertion = pIniFl->GetFloat("CharacterMouse", "MouseInertion", 0.2f);
		
		pIniFl->Release();
	}*/	
}

void PlayerController::Reset()
{
	/*DropWeapon();

	if (itemTaken)
	{
		chr.animation->SetLastAnimation();
		
		chr.items->DelItems(str_secondId);		

		itemTaken = false;
	}
	
	itemTaken = false;

	phys_object = NULL;	
	*/

	fStrafeCooldown = 0.0f;
	fPistolCooldown = 0.01f;	

	combo_cur_timespawn = 0;
	combo_times = 0;
	combo_times_max = 0;
	combo_rage_times_max = 0;

	need_to_add_combo = 0;
	combo_ticked = 0.0f;	
	cur_combo_add_tick = 0;

	EnableRageMode(false);

	chr_showed_HUD = null;
	chr_need_show_HUD = null;
	chr_HUD_cooldown = 0.0f;

	cur_mombo = -1;	
	cur_time_to_hide_mombo = 0;	
	
	cur_money_add_tick = money_add_tick;
	money_txt_cooldown = 0.0f;	
	money_ticked = ReadGameState(gsMoney, 0.0f);

	mission_time = null;

	MOSafePointer mo;
	chr.FindObject(missiontime_name,mo);
	mission_time = mo.Ptr();

	EndSloMo(true);

	slomo_time = -slomo_cooldown;

	chr.bodyparts.SetRotation(0.0f,true);

	if (chr.logic->GetHP() > 0.1f && chr.logic->GetHP()/chr.logic->GetMaxHP() < restorehp_params[cur_reshp_params].when_resore_HP)
	{
		chr.logic->SetHP(chr.logic->GetMaxHP() * restorehp_params[cur_reshp_params].when_resore_HP);
	}	

	HideWidgets();

	hpBar = null;

	FindCurHPParam();

	fMouseUpdate = 0.0f;
	iMouseTimes = 0;
	fCurMouseValueV = 0.0f;
	fCurMouseValueH = 0.0f;
	fMouseFilteredV = 0.0f;
	fMouseFilteredH = 0.0f;

	qevent = null;

	time_to_attack_notify = 0;
}

void PlayerController::HideWidgets()
{
	money_widget.Show(&chr,false);
	combo_widget.Show(&chr,false);
	combo_moto_widget.Show(&chr,false);

	if (hpBar)
	{					
		hpBar->Show(false);
	}
}

void PlayerController::Show(bool show)
{
	isShow = show;

	if (!chr.IsPlayer())
	{
		return;
	}

	if (phys_object && itemTaken)
	{
		phys_object->Show(show);
	}	

	pistolbar.Show(&chr,show);
	bombbar.Show(&chr,show);
	ragebar.Show(&chr,show);	

	pistolwidget.Show(&chr,false);
	pistolfullwidget.Show(&chr,false);

	weaponlifeBar.Show(&chr,false);

	if (cur_weapon_style) cur_weapon_style->widgetDrop.Show(&chr,false);	

	if (itemTaken)
	{
		widgetMainWeapon.Show(&chr,false);

		if (cur_weapon_style) cur_weapon_style->widgetTake.Show(&chr,show);		
	}
	else
	{
		widgetMainWeapon.Show(&chr,true);

		if (cur_weapon_style) cur_weapon_style->widgetTake.Show(&chr,false);		
	}

	HideWidgets();

	EnableRageMode(false);
}

void PlayerController::Death()
{
	if (hpBar)
	{					
		hpBar->Show(false);
	}

	DropWeapon(true);

	if (itemTaken)
	{
		chr.animation->SetLastAnimation();

		chr.items->DelItems(str_secondId);	
		ShowCurWeaponWidget(false);

		itemTaken = false;
		cur_weapon_style = null;
	}

	HideWidgets();
}

void PlayerController::NotifyCharAboutAttack()
{
	chr.arbiter->FindCircle(chr, 6.25f);

	Matrix mtx = chr.physics->GetMatrixWithBoneOffset(mtx);		
	Vector pos = mtx.pos;

	float damage = chr.animation->GetConstFloat("damage");

	if(chr.arbiter->find > 0)
	{			
		array<CharacterFind> & find = chr.arbiter->find;

		for(long i = 0; i < find; i++)
		{	
			if (find[i].chr->logic->IsPairMode()) continue;
			if(find[i].chr->logic->GetState() == CharacterLogic::state_die) continue;
			if(find[i].chr->logic->GetState() == CharacterLogic::state_dead) continue;
			if(find[i].chr->logic->GetHP() <= 0.0f) continue;
			if(find[i].chr->ragdoll) continue;

			find[i].chr->controller->NotifyAboutAttack(&chr, damage);			
		}
	}
}

void PlayerController::ShowDebugInfo()
{	
	//chr.Render().Print(10,10,0xffffffff,"%f",chr.logic->ReadGameStateFloat(gsNumBombs));

	if (slomo_markers.Size()<=0) return;


	if (api->DebugKeyState('N'))
	{
		cur_marker++;

		if (cur_marker>=(int)slomo_markers.Size()) cur_marker = 0;
		Sleep(100);
	}

	if (api->DebugKeyState('M'))
	{
		cur_marker--;

		if (cur_marker<0) cur_marker = slomo_markers.Size()-1;
		Sleep(100);
	}

	if (cur_marker == -1) return;	

	chr.Render().Print(10,10,0xff00ff00,"%i",slomo_markers[cur_marker].id);

	float dmg = chr.animation->GetConstFloat("damage");

	const char * react = chr.animation->GetConstString("hit");

	if (react)
	{
		if (string::IsEqual(react,"Blow")||
		string::IsEqual(react,"HeadOff")||
		string::IsEqual(react,"Legcut"))
		{
			dmg = 10000;
		}
	}
		
	bool show_markers = false;

	Matrix mat;
	chr.physics->GetModelMatrix(mat);

	//Vector dir(0,0,0.2f);
	//dir.Rotate(chr.physics->GetAy());
	//mat.pos += dir;

	chr.arbiter->FindCircle(chr,3.5f);

	array<CharacterFind> & find = chr.arbiter->find;						

	for (int k=0;k<(int)slomo_markers.Size();k++)
	{
		//if (slomo_markers[k].id == id)
		if (k == cur_marker)
		{
			for (int j=1;j<(int)slomo_markers[k].markers.Size();j++)
			{
				static Vector quadrangle[4];
				quadrangle[0] = mat * slomo_markers[k].markers[j].s;
				quadrangle[1] = mat * slomo_markers[k].markers[j].e;
				quadrangle[2] = mat * slomo_markers[k].markers[j-1].e;
				quadrangle[3] = mat * slomo_markers[k].markers[j-1].s;

				for (int i = 0; i < find; i++)
				{			
					if (find[i].chr == &chr) continue;

					if (chr.items->CheckHit(find[i].chr,quadrangle)/*&&		
						find[i].chr->GetHP()>0 && //find[i].chr->GetHP()<dmg &&
						chr.logic->IsEnemy(*find[i].chr) &&!find[i].chr->logic->IsActor() &&
						!find[i].chr->logic->IsPairMode()*/)
					{
						show_markers = true;
					}
				}
	
			}
		}
	}

	
	{
		for (int i=0;i<(int)slomo_markers.Size();i++)
		{
			//if (slomo_markers[i].id != id) continue;
			if (i != cur_marker) continue;

			array<TSloMoSubMarker> &markers = slomo_markers[i].markers;

			dword fst_color = 0xffff0000;
			if (show_markers) fst_color = 0xffff00ff;

			dword scnd_color = 0xff00ff00;
			if (show_markers) scnd_color = 0xffff00ff;

			if (markers.Size()>0)
			{
				chr.Render().DrawLine(markers[0].s * mat, fst_color,
									  markers[0].e * mat, scnd_color);

				for (int j=1;j<(int)markers.Size();j++)
				{				
					chr.Render().DrawLine(markers[j].s * mat, fst_color,
									      markers[j].e * mat, scnd_color);

					chr.Render().DrawLine(markers[j].s * mat, fst_color,
									      markers[j-1].s * mat, fst_color);

					chr.Render().DrawLine(markers[j].e * mat, scnd_color,
									      markers[j-1].e * mat, scnd_color);
				}
			}

			break;
		}
	}	
}

void PlayerController::HUDWork(float dltTime)
{	
	{				
		pistolbar.Show(&chr, !itemTaken );
		//pistolbar.Show(&chr,(!itemTaken && !trace_weapondownwidget));

		weaponlifeBar.Show(&chr,itemTaken);

		if (itemTaken)
		{			
			pistolwidget.Show(&chr,false);
			pistolfullwidget.Show(&chr,false);

			ShowCurWeaponStateWidget();
			
			float item_life = Clampf(GetLastWeaponLive());
			weaponlifeBar.SetPercentage(&chr,(pistolBarBeg + item_life * (pistolBarEnd - pistolBarBeg)) * 0.01f);
		}
		else
		{			
			pistolbar.SetPercentage(&chr,(pistolBarBeg + (1.0f - fPistolCooldown/ReadGameState(gsPistolTimeReload,4.0f)) * (pistolBarEnd - pistolBarBeg)) * 0.01f);

			if (fPistolCooldown<=0.0f)
			{
				pistolwidget.Show(&chr,false);

				if (!pistolfullwidget.IsShow(&chr))
				{
					pistolfullwidget.Command(&chr,"PlayEffect",0,null);
				}

				pistolfullwidget.Show(&chr,true);				
			}
			else
			{
				pistolwidget.Show(&chr,true);
				pistolfullwidget.Show(&chr,false);
			}			
		}
	}	

	//if (fBombCooldown>0.0f || !bRestoreBomb)
	{
		if (bRestoreBomb)
		{
			fBombCooldown -= dltTime;

			if (fBombCooldown<=0.0f)
			{
				if (ReadGameState(gsNumBombs,4.0f)<ReadGameState(gsMaxBombs,4.0f)-1)
				{				
					fBombCooldown = fBombTimeReload;
				}
				else
				{
					fBombCooldown = 0.0f;
				}

				WriteGameState(gsNumBombs,ReadGameState(gsNumBombs,4.0f)+1);

				if (ReadGameState(gsNumBombs,4.0f)>ReadGameState(gsMaxBombs,4.0f))
				{
					WriteGameState(gsNumBombs,ReadGameState(gsMaxBombs,4.0f));
				}
			}
		}
				
		bombbar.SetPercentage(&chr,(bombBeg + bombSz * (float)(ReadGameState(gsNumBombs,4.0f))) * 0.01f);
	}	
	
	{	
		{
			ragebar.SetPercentage(&chr,ReadGameState(gsCurRage,0.0f) / (float)iHitsToRage);			
		}

		if (chr.logic->IsRageMode())
		{						
			rageaction.SetPercentage(&chr,fCurRageTime / ReadGameState(gsRageTimeAction,10.0f));
		}
		
		
	}
}

void PlayerController::DropWeaponBoxSlave()
{
	if (chr.logic->slave)
	{
		Character * bomba = chr.logic->slave;
		chr.logic->slave = null;

		if (bomba->logic)
		{
			bomba->logic->SetOffender(&chr);
			bomba->DeattachChar();
			bomba->SelfBlowBombardier();
		}
	}
	else if (itemTaken && phys_object)
	{
		Vector min, max;
		phys_object->GetBox(min, max);
		Vector curPos = (min + max) * 0.5f;

		phys_object->Activate(true);
		BlowPhysObject(curPos, false, false);
		BrokeWeapon(true);
	}
	else if (cur_weapon_style)
	{
		BrokeWeapon(true);
	}
}

//Управление персонажем
void PlayerController::Update(float dltTime)
{
	if (itemTaken && phys_object)
	{
		if (!phys_object->IsShow())
			phys_object->Show(true);
	}

	if (watchCollisionWithPhysObject && ((itemTaken && phys_object) || chr.logic->slave))
	{
		Vector curPos = prevPhysObjPos;
		Vector min, max;

		if (phys_object)
		{
			phys_object->GetBox(min, max);
			curPos = (min + max) * 0.5f;
		}
		else if (chr.logic->slave)
			curPos = chr.logic->slave->physics->GetPos(true);

		if (phys_object)
		{
			Vector dir = curPos - prevPhysObjPos;
			float dist = dir.GetLength();
			if (dist > 1e-8f)
			{
				dir = dir * (1.0f / dist);
				if (chr.Physics().Raycast(prevPhysObjPos, curPos + dir * 0.20f))
				{
					phys_object->Activate(true);
					BlowPhysObject(curPos, false);
					BrokeWeapon(true);
				}
			}
		}
		else if (chr.logic->slave)
		{
			Vector dir2D = (curPos - prevPhysObjPos);
			dir2D.NormalizeXZ();
			if (chr.Physics().Raycast(prevPhysObjPos, curPos) || chr.Physics().Raycast(curPos, curPos + dir2D * 0.85f, phys_mask(phys_ragdoll)))
			{
				Character * bomba = chr.logic->slave;
				chr.logic->slave = null;
				
				if (bomba->logic)
				{
					bomba->logic->SetOffender(&chr);
					bomba->DeattachChar();
					bomba->SelfBlowBombardier();
				}
			}
		}
	
		prevPhysObjPos = curPos;
	}


	if (itemTaken)
	{
		if (cur_weapon_style)
		{
			if (cur_weapon_style->weapon_type == wp_rifle || cur_weapon_style->weapon_type == wp_gun)
			{
				chr.logic->OrientBodyPart(false);
			}
		}
		else
		{
			chr.bodyparts.SetRotation(0.0f,false);
		}		
	}
	else
	{
		chr.bodyparts.SetRotation(0.0f,false);
	}

	
	if (api->DebugKeyState('I'))
	{
		chr.BloodStain(api->DebugKeyState(VK_CONTROL) ? 200.0f : -1.0f);
		Sleep(100);
	}

	/*if (api->DebugKeyState('V'))
	{
		AddComboHits(10);
		Sleep(300);
	}

	/*if (api->DebugKeyState('V'))
	{
		EnableRageMode(!chr.logic->IsRageMode());
		Sleep(500);
	}*/


	if(api->DebugKeyState('D', 'T', 'H'))
	{
		chr.logic->SetHP(0);
		return;
		//Death();
		//chr.items->DropItems();
	}

	/*if (mission_time)
	{
		if (bSlowDownTime)
		{
			chr.animation->SetPlaySpeed(3.0f);
			chr.logic->SetTimeScale(3.0);

			const char* params[3];
			params[0] = "0.333";
			params[1] = "0.001";
			mission_time->Command("set",2,params);
			chr.events->SetParticlesPlaySpeed(2.0f);
		}
		else
		{
			chr.animation->SetPlaySpeed(1.0f);
			chr.logic->SetTimeScale(1.0);

			const char* params[3];
			params[0] = "1.0";
			params[1] = "0.001";
			mission_time->Command("set",2,params);
			chr.events->SetParticlesPlaySpeed(1.0f);
		}
	}*/

	if (useGameState)
	{
		chr.logic->SetHPMultipler(Clampf(ReadGameState(gsHPLive,100.0f) * 0.01f,0.5f,256.0f));	
	}
	else
	{
		chr.logic->SetHPMultipler(1.0f);	
	}

	if (ReadGameState(gsMaxBombs,4.0f)<ReadGameState(gsNumBombs,4.0f))
	{
		WriteGameState(gsNumBombs,ReadGameState(gsMaxBombs,4.0f));
	}
	

	{	
		bool was_positive = false;
		
		if (slomo_time>0.0f)
		{
			was_positive = true;
		}

		slomo_time -= api->GetDeltaTime();

		if (slomo_time<0 && was_positive)
		{
			EndSloMo();
			slomo_time = -RRnd(0.0f, slomo_cooldown * 0.4f);
		}

		if (slomo_time<-slomo_cooldown)
		{
			slomo_time=-slomo_cooldown;
		}
	}
	


	if (chr.logic->GetHP()/chr.logic->GetMaxHP() < restorehp_params[cur_reshp_params].when_resore_HP)
	{
		if (cur_time_to_restore_HP>0)
		{
			cur_time_to_restore_HP -= dltTime;
		}
		else
		{
			cur_restore_rate_HP -= dltTime;

			if (cur_restore_rate_HP<0)
			{
				int nums = -(int)(cur_restore_rate_HP / restorehp_params[cur_reshp_params].restore_rate_freq_HP) + 1;				
				cur_restore_rate_HP += restorehp_params[cur_reshp_params].restore_rate_freq_HP * nums;
				chr.logic->SetHP(chr.logic->GetHP() + chr.logic->GetMaxHP() * restorehp_params[cur_reshp_params].restore_rate_HP * nums);
			}
		}		
	}

	if (money_txt_cooldown>0.0f)
	{
		money_txt_cooldown -= dltTime;

		if (money_txt_cooldown<0.0f) money_txt_cooldown = 0.0f;
	}	

	//if (need_to_add_money>0)
	{
		float delta = ReadGameState(gsMoney, 0.0f) - money_ticked;
		if (delta > 1e-5f)
		{
			money_txt_cooldown = 3.0f;
			money_ticked += Min(delta, 3.0f * dltTime * money_add_tick);
		}

		/*if (money_ticked>need_to_add_money)
		{
			cur_money_add_tick = money_add_tick;

			money_ticked = 0;
			need_to_add_money=0;			
		}*/		
	}

	if (need_to_add_combo>0)
	{		
		combo_cur_timespawn = combo_timespawn;
		combo_ticked += dltTime * cur_combo_add_tick;

		/*if (!chr.logic->IsRageMode())
		{
			if ((int)(combo_times + combo_ticked)>=iHitsToRage)
			{
				EnableRageMode(true);
			}
		}*/

		combo_widget.Command(&chr,"PlayEffect",0,null);		

		if (combo_ticked>need_to_add_combo)
		{
			combo_times += need_to_add_combo;
			cur_combo_add_tick = combo_add_tick;

			combo_ticked = 0;
			need_to_add_combo=0;			
		}		
	}


	if (chr_HUD_cooldown>0)
	{
		chr_HUD_cooldown -= dltTime;

		if (chr_HUD_cooldown<0.0f || chr_showed_HUD->GetHP()<=0.01f)
		{
			chr_HUD_cooldown = 0.0f;

			if (chr_showed_HUD)
			{
				if (hpBar)
				{					
					hpBar->Show(false);					
				}				

				chr_showed_HUD = null;
				hpBar = null;
			}

			if (chr_need_show_HUD)
			{
				if (chr_need_show_HUD->hpBar)
				{
					hpBar = chr_need_show_HUD->hpBar;

					if (hpBar)
					{		
						hpBar->Show(true);

						const char* params[2];
						params[0] = chr_need_show_HUD->GetObjectID().c_str();

						hpBar->Command("SetMO",1,params);						

						chr_showed_HUD = chr_need_show_HUD;
						chr_need_show_HUD = null;

						chr_HUD_cooldown = 2.0f;
					}
				}
			}
		}
	}

	if(chr.Controls().GetControlStateType(lKeyCodes[ChrGotoHell]) == CST_ACTIVATED)
	{
		const array<Character *> & active_chars = chr.arbiter->GetActiveCharacters();

		for (int i=0;i<(int)active_chars.Size();i++)
		{
			if (active_chars[i]->logic->IsActor()) continue;
			if (active_chars[i]->logic->IsDead()) continue;
			if (!active_chars[i]->logic->IsEnemy(&chr)) continue;

			if (chr.arbiter->IsSphereInFrustrum(active_chars[i]->physics->GetPos(), 0.75f))
			{
				active_chars[i]->logic->SetHP(0.0f);
			}
		}
	}

	if (qevent)
	{	
		if (qevent->bQuickEventEnabled && qevent->AllowInterruptByButton())
		{
			bool skip_mouse = false;

			if (string::IsEqual(qevent->GetName(),"Taran"))
			{
				skip_mouse = true;
			}


			if (ReadGameState(gsControlType,0)>0.5f)
			{				
				ChrCodes codes[] = {ChrBlock, ChrA, ChrB,
									ChrInstanceShoot, ChrInstanceDropBomb};

				
				for (int i=0;i<(int)(sizeof(codes)/sizeof(ChrCodes));i++)
				{
					if (!skip_mouse)					
					{
						if (i>0) continue;
					}					

					if ( fabs(chr.Controls().GetControlStateFloat(lKeyCodes[codes[i]])) > 0.7f)
					{
						EndQuickEvent();	
						break;
					}
				}
			}
			else
			{
				for (int i=0;i<13;i++)				
				{
					if (skip_mouse)
					{
						if (i==0 || i==1) continue;
					}

					if ( fabs(chr.Controls().GetControlStateFloat(lKeyCodes[i])) > 0.7f)
					{
						EndQuickEvent();
	
						break;
					}
				}
			}
		}
	}

	if (fCurRageTime > 0)
	{
		fCurRageTime -= dltTime;

		if (fCurRageTime <0)
		{
			EndRage();
			//EnableRageMode(false);
		}		
	}

	HUDWork(dltTime);	

	if (fPistolCooldown>0.0f)
	{	
		fPistolCooldown -= dltTime;

		if (fPistolCooldown<0.0f)
		{
			fPistolCooldown = 0.0f;
		}
	}
		
	if (delayShowCurWpWidget>0.0f)
	{
		delayShowCurWpWidget -= dltTime;

		//if (delayShowCurWpWidget<0.0f)
		{
			delayShowCurWpWidget = -1.0f;
			ShowCurWeaponWidget(true);
		}
	}

	if (fStrafeCooldown>0.0f)
	{
		fStrafeCooldown -= dltTime;

		if (fStrafeCooldown<0.0f)
		{
			fStrafeCooldown = 0.0f;
		}
	}

	/*if (trace_weapondownwidget && cur_weapon_style)
	{
		if (!cur_weapon_style->widgetDrop.IsPlayingAnimation(&chr))
		{
			trace_weapondownwidget = false;

			cur_weapon_style->widgetDrop.Show(&chr,false);

			widgetMainWeapon.Show(&chr,true);			
		}		
	}*/

	if (time_to_attack_notify>0)
	{
		time_to_attack_notify--;

		if (time_to_attack_notify<=0)
		{
			time_to_attack_notify = 0;
			NotifyCharAboutAttack();
		}
	}

	chr.UpdateMG(dltTime);

	WriteGameState(gsCurHPLive,chr.logic->GetHP());
		
	if (chr.logic->IsActor()) return;

	if (qevent) return;

	if (chr.logic->IsPairMode())
	{
		Pair(dltTime,false,0);
	}
	else
	{	
		//if (!itemTaken)
		Pair(dltTime,false, 1);

		if (chr.logic->IsPairMode())
		{
			return;
		}

		
		Fight();
		GetMove(dltTime);
		chr.logic->OrientBodyPart(true);
		
		if(IsCanMove())
		{
			Move(dltTime);
		}

		if(chr.Controls().GetControlStateType(lKeyCodes[ChrBlock]) == CST_ACTIVE)
		{
			if(chr.logic->GetState() != CharacterLogic::state_block)
			{
				if (!itemTaken || (itemTaken && GetLastWeaponLive()>0.0f)) chr.animation->ActivateLink("toBlock",true);
			}
		}
		else
		{
			if(chr.logic->GetState() == CharacterLogic::state_block)
			{
				chr.animation->ActivateLink("fromBlock");			
			}
		}

		//if (!itemTaken) 
		Bomb();

		//if (!itemTaken) Pair(dltTime,true);

		if (Accept()) return;

		// Vano: добавил, если player выключен после нажатия Accept и входа в какой либо QE или Minigun/Shooter, чтобы не выбросилось оружие
		if (!isShow) return;

		if (chr.logic->IsActor()) return;

		bool weaponDropped = false;
		if (!qevent && !chr.logic->IsPairMode())
		{
			if (!Weapons(dltTime, weaponDropped)) return;
		}
		
		if (chr.physics->dltPosSpeed.z > 0.1f)
		{
			chr.PushCharcters(dltTime,1.75f,true,!(chr.logic->GetState() == CharacterLogic::state_roll));
		}

		Pair(dltTime, false, 2, weaponDropped);
	}

	SetGameHint();
	//chr.Render().Print(10,10,0xff00ff00,"%s",ReadGameState(gsGameHint,"failed read"));

	//chr.Render().Print(10, 10, 0xFF00FF00, "hit = %d/%d, cdwn = %.1f/%.1f", isSlomoHit, SlomoChecks, fabsf(slomo_time), fabsf(slomo_cooldown));
	isSlomoHit = false;
	SlomoChecks = 0;
	
}

//Получить направление и скорость перемещения
void PlayerController::GetMove(float timedelta)
{	
	dir.x = chr.Controls().GetControlStateFloat(lKeyCodes[ChrStrafeLeftRight]);
	dir.z = chr.Controls().GetControlStateFloat(lKeyCodes[ChrStrafeForwBack]);			
	
	//chr.Render().Print(chr.physics->GetPos() + Vector(0.0f, 2.0f, 0.0f), 100.0f, 1.0f, 0xffffffff, "State: %f", dir.x);
	//chr.Render().Print(chr.physics->GetPos() + Vector(0.0f, 2.0f, 0.0f), 100.0f, 2.0f, 0xffffffff, "State: %f", dir.z);

	bool is_strafe = false;

	fMouseUpdate += timedelta;

	fCurMouseValueH += chr.Mission().Controls().GetControlStateFloat(lKeyCodes[ChrMouseLeftRight]) * ReadGameState(gsMouseSens,1.0f);
	fCurMouseValueV += chr.Mission().Controls().GetControlStateFloat(lKeyCodes[ChrMouseForwBack]) * ReadGameState(gsMouseSens,1.0f);
	
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
	
	/*chr.Render().Print(10,10,0xff00ff00,"ms_dead_zone = %4.3f ms_scale = %4.3f", ms_dead_zone, ms_scale);

	if (api->DebugKeyState('V'))
	{
		ms_dead_zone += 0.01f;
	}

	if (api->DebugKeyState('C'))
	{
		ms_dead_zone -= 0.01f;
		if (ms_dead_zone<0.05f) ms_dead_zone = 0.05f;
	}

	if (api->DebugKeyState('N'))
	{
		ms_scale += 0.01f;
	}

	if (api->DebugKeyState('B'))
	{
		ms_scale -= 0.01f;
		if (ms_scale<1.05f) ms_scale = 1.05f;
	}*/

	chr.physics->SetTurnSpeed(10.0f);

	if (fabs(dir.x)<0.7f && fabs(dir.z)<0.7f)
	{
		//Получим состояние контролов
		dir.x = chr.Controls().GetControlStateFloat(lKeyCodes[ChrLeftRight]);
		dir.z = chr.Controls().GetControlStateFloat(lKeyCodes[ChrForwBack]);
				
		Vector ms_dir = Vector( fMouseFilteredH, 0.0f, fMouseFilteredV);

		last_ms_dir.MoveByStep(ms_dir,timedelta*40.0f);								
		ms_dir.x = last_ms_dir.x;
		ms_dir.z = last_ms_dir.z;		

		ms_dir.x /= ms_scale;
		ms_dir.z /= ms_scale;		

		if (fabs(dir.x)<0.25f && fabs(dir.z)<0.25f)
		{							
			if (fabs(ms_dir.x)>ms_dead_zone || fabs(ms_dir.z)>ms_dead_zone)
			{
				chr.physics->SetTurnSpeed(4.5f);

				dir.x = ms_dir.x;
				dir.z = ms_dir.z;

				mouse_dir_time = ms_inertion;
				mouse_dir_x = ms_dir.x;
				mouse_dir_z = ms_dir.z;
			}
			else
			if (mouse_dir_time>0.0f)
			{
				mouse_dir_time -= timedelta;
				dir.x = mouse_dir_x;
				dir.z = mouse_dir_z;
			}			
		}
		else
		{
			if (fabs(ms_dir.x)>ms_dead_zone || fabs(ms_dir.z)>ms_dead_zone)
			{
				chr.physics->SetTurnSpeed(4.5f);

				ms_dir.Normalize();

				vLastMsDir.x = dir.x;
				vLastMsDir.z = dir.z;

				dir.x = ms_dir.x;
				dir.z = ms_dir.z;				

				mouse_dir_time = ms_inertion;
				mouse_dir_x = ms_dir.x;
				mouse_dir_z = ms_dir.z;
			}
			else
			if (mouse_dir_time>0.0f)
			{
				if (fabs(vLastMsDir.x-dir.x)>0.25f||
					fabs(vLastMsDir.z-dir.z)>0.25f)
				{
					mouse_dir_time = -1.0f;
				}
				else
				{
					dir.x = mouse_dir_x;
					dir.z = mouse_dir_z;
				}
			}
		}
	}
	else
	{
		//chr.Render().Print(chr.physics->GetPos() + Vector(0.0f, 2.0f, 0.0f), 100.0f, 3.0f, 0xffffffff, "Fuck!!!", dir.z);		
		if (!itemTaken || (itemTaken && GetLastWeaponLive()>0.0f)) chr.animation->ActivateLink("strafe forward",true);

		is_strafe = true;

		if (mouse_dir_time>0.0f)
		{
			if (fabs(vLastMsDir.x-dir.x)<0.25f&&fabs(vLastMsDir.z-dir.z)<0.25f)
			{
				dir.x = mouse_dir_x;
				dir.z = mouse_dir_z;		
			}
		}
	}	
			
	//chr.Render().Print(10,50,0xff00ff00,"%4.3f %4.3f", fMouseFilteredH, fMouseFilteredV);	
	//chr.Render().Print(10,70,0xff00ff00,"%4.3f %4.3f", dir.x, dir.z);	
	//chr.animation->ActivateLink("strafe forward",true);
	//chr.animation->ActivateLink("strafe forward");

	//Переведём в систему камеры
	Matrix view = chr.Render().GetView();
	view.Inverse();
	

	if (dir.x==0&&dir.z==0)
	{		
		mLastViewMatrix = view;

		vLastDir = dir;
	}
	else
	if (fabs(dir.x - vLastDir.x)<0.2f && fabs(dir.z - vLastDir.z)<0.2f)
	{
		view = mLastViewMatrix;
	}
	else
	{
		mLastViewMatrix = view;

		vLastDir = dir;
	}
	
	{	
		Matrix mtx;
		mtx.vz = view.vz;
		mtx.vz.y = 0.0f;
		if(mtx.vz.NormalizeXZ() < 1e-20f)
		{
			mtx.vz = view.vy;
			mtx.vz.y = 0.0f;
			if(mtx.vz.NormalizeXZ() < 1e-20f)
			{
				//Сюда не должны попадать
				return;
			}
		}
	
		mtx.vy = Vector(0.0f, 1.0f, 0.0f);
		mtx.vx = mtx.vy ^ mtx.vz;
		dir = mtx*dir;		
	}	

	if (is_strafe)
	{
		move = ms_strafe;
		return;
	}
	else
	{
		move = ms_run;
	}

	//Анализируем перемещение
	float moveValue = coremax(fabsf(dir.x), fabsf(dir.z));
	switch(move)
	{
		case ms_stop:
		if(moveValue >= thresholdToRun)
		{
			move = ms_run;
		}else
		if(moveValue >= thresholdToWalk)
		{
			move = ms_run;
		}
		break;
		case ms_walk:
		if(moveValue >= thresholdToRun)
		{
			move = ms_run;
		}else
		if(moveValue <= thresholdFromWalk)
		{
			move = ms_stop;
		}
		break;
		case ms_run:
		if(moveValue <= thresholdFromWalk)
		{
			move = ms_stop;
		}else
		if(moveValue <= thresholdFromRun)
		{
			move = ms_run;
		}
		break;
	}
}


//Переместиться
void PlayerController::Move(float dltTime)
{
	//Текущий режим перемещения
	CharacterLogic::State st = chr.logic->GetState();	

	switch(move)
	{
		case ms_stop:
		if(st != CharacterLogic::state_idle)
		{			
			time_to_run_fast = 0.0f;
			chr.animation->ActivateLink("idle");
		}
		break;
		case ms_walk:
		if(st != CharacterLogic::state_walk)
		{
			time_to_run_fast = 0.0f;
			chr.animation->ActivateLink("walk");
		}
		break;
		case ms_run:
		{
			if (chr.logic->slave)
			{
				CharacterLogic::State slave_st = chr.logic->slave->logic->GetState();
				if (st != CharacterLogic::state_run || slave_st != CharacterLogic::state_run)
				{
					time_to_run_fast = 0.0f;
					chr.animation->ActivateLink("run");
				}
			}
			else
			{
				if(st != CharacterLogic::state_run)
				{
					time_to_run_fast = 0.0f;
					chr.animation->ActivateLink("run");
				}
				else
				{
					time_to_run_fast += dltTime;

					if (time_to_run_fast>2.0f)
					{
						time_to_run_fast = 2.0f;

						chr.animation->ActivateLink("run fast");
					}
				}
			}
		}
		break;
	}
	//Устанавливаем угол
	if(~dir > 1e-1f &&
	   st != CharacterLogic::state_shoot&& st != CharacterLogic::state_dropbomb&& st != CharacterLogic::state_hit &&
	   chr.logic->AllowOrient() && !qevent)
	{
		chr.physics->Turn(dir.GetAY(chr.physics->GetAy()));
	}
}

//Бой холодным оружием
void PlayerController::Fight()
{		
	bool isRifle = false;
	bool isGun = false;
	if(cur_weapon_style)
	{
		isRifle = (cur_weapon_style->weapon_type == wp_rifle);
		isGun = (cur_weapon_style->weapon_type == wp_gun);
	}

	bool isChrA = chr.Controls().GetControlStateType(lKeyCodes[ChrA]) == CST_ACTIVATED;
	bool isChrAccept = chr.Controls().GetControlStateType(lKeyCodes[ChrAccept]) == CST_ACTIVATED;

	if (isChrA || 
	   (isChrAccept && itemTaken && phys_object) ||
	   (itemTaken && chr.Controls().GetControlStateType(lKeyCodes[ChrInstanceShoot]) ==  CST_ACTIVATED && isRifle) ||
	   (itemTaken && chr.Controls().GetControlStateType(lKeyCodes[ChrInstanceShoot]) ==  CST_ACTIVATED && isGun) ||
	   (itemTaken && chr.Controls().GetControlStateType(lKeyCodes[ChrB]) == CST_ACTIVATED && isGun))
	{	
		/*const char* link = "A";

		const char* node = chr.animation->CurrentNode();
		if (node[0] == 'B' || node[0] == 'b')
		{
			link = "A_Sp1";
		}
		
		
		if (chr.animation->ActivateLink(link))*/
		//const char * ann = GetAttackLinkName(gsPlayerAttackA, chr.animation->CurrentNode(), "A");
		if (chr.animation->ActivateLink(GetAttackLinkName(gsPlayerAttackA, chr.animation->CurrentNode(), "A")))
		{
			//if (chr.swordEffect) chr.swordEffect->MakeSwordEffect(&chr,"attackA");
			//Vano: если выбрасываем физ объект
			if (isChrAccept || isChrA)
			{
				if ((itemTaken && phys_object) || chr.logic->slave)
				{
					watchCollisionWithPhysObject = true;

					if (phys_object)
					{
						Vector min, max;
						phys_object->GetBox(min, max);
						prevPhysObjPos = (min + max) * 0.5f;
						isDropStarted = true;
					}
					else
						prevPhysObjPos = chr.logic->slave->physics->GetPos(true);
				}

				if ((itemTaken && phys_object) || 
					(cur_weapon_style && cur_weapon_style->weapon_type == wp_bombardier_body))
						vanoHackDropPhysObjectWhenActor = true;
			}
	
			time_to_attack_notify = 1;			
			
			// Hard code I hate Vladmir Vladimorovich
			if (itemTaken && (isRifle || isGun))				
			{
				chr.items->DamageWeapon(false);
				chr.logic->AimToTarget(7.0,PI*0.5f);

				if (chr.logic->GetState()==CharacterLogic::state_run||
					chr.logic->GetState()==CharacterLogic::state_walk)
				{
					chr.logic->time_to_return = chr.logic->GetCrossLinkTime();

					MOSafePointer mo;
					static const ConstString ccId("Camera Controller");
					chr.FindObject(ccId,mo);

					if (mo.Ptr())
					{
						const char* params[3];
						params[0] = "0.1";
						params[1] = "0.2";
						params[2] = "30";
						mo.Ptr()->Command("shockPolar",3,params);
					}

					Matrix mtx(true);
					if (chr.items->FindLocatorinItems("end",mtx))
					{					
						ISound3D * snd = chr.Sound().Create3D("gun3", mtx.pos, _FL_, true, true);
			
						if(snd)
						{
							snd->Play();
						}
					}
					static const ConstString fxMusket_Shot_Light("Musket_Shot_Light");
					chr.chrEffect->MakeEffect(&chr,fxMusket_Shot_Light);					
								
					if (chr.chrEffect && isRifle)
					{						
						chr.ShootTargets(fRifleDamage,"HitA4",true,3.0f,10.0f, false);						

						if (isRifle) chr.items->StartParticlesOnItems(str_secondId, "fire", "shotmusket.xps", true, chr.GetMatrix(mtx), 1.0f, false);
					
						if (chr.logic->GetState()==CharacterLogic::state_run)
						{
							// Vano убрал по просьбе Вити Темного
							//chr.chrEffect->MakeEffect(&chr,"Musket_Shot_Run_Geometry");
						}
						else
						if (chr.logic->GetState()==CharacterLogic::state_walk)
						{
							// Vano убрал по просьбе Вити Темного
							//chr.chrEffect->MakeEffect(&chr,"Musket_Shot_Start_Run_Geometry");
						}
					}				
					else
					if (chr.chrEffect && isGun)
					{
						static const ConstString fxOfficer_Shot_Light("Officer_Shot_Light");
						chr.chrEffect->MakeEffect(&chr,fxOfficer_Shot_Light);
						chr.items->StartParticlesOnItems((curGunNumber) ? str_secondId : str_2gunId, "fire", "shotshortgunblood.xps", true, chr.GetMatrix(mtx), 0.7f, true);
						chr.ShootTargets(fPistolDamage,"GunHit",false,1.5f,15.0f, true );
						curGunNumber ^= 1;
					}
				}
			}			
		}		
	}
	else
	if(chr.Controls().GetControlStateType(lKeyCodes[ChrB]) == CST_ACTIVATED)
	{
		//if (chr.swordEffect) chr.swordEffect->MakeSwordEffect(&chr,"attackB");

		/*const char* link = "B";

		const char* node = chr.animation->CurrentNode();
		if (node[0] == 'A' || node[0] == 'a')
		{
			link = "B_Sp1";
		}


		if (chr.animation->ActivateLink(link))*/
		if (chr.animation->ActivateLink(GetAttackLinkName(gsPlayerAttackB, chr.animation->CurrentNode(), "B")))		
		{	
			time_to_attack_notify = 1;			
		}
	}
	/*else
	if(chr.Controls().GetControlStateType(lKeyCodes[ChrCrush]) == CST_ACTIVATED)
	{
		if (chr.animation->TestActivateLink("Crush"))
		{
			chr.animation->ActivateLink("Crush");
			
			if ( (chr.logic->GetHP() - chr.logic->GetMaxHP()*0.05f)>0.0f)
			{
				chr.logic->SetHP(chr.logic->GetHP() - chr.logic->GetMaxHP()*0.05f);
			}

		}		
	}*/

	if(chr.Controls().GetControlStateType(lKeyCodes[ChrRage]) == CST_ACTIVATED && !chr.logic->IsRageMode())
	{
		//if (IsFullRage())
		if (ReadGameState(gsCurRage,0.0f)>0.0f)
		{
			EnableRageMode(true);
		}		
	}
	
}

//Парные анимации
void PlayerController::Pair(float dltTime, bool doKick, int filter, bool weaponDropped)
{
	CharacterLogic::State state = chr.logic->GetState();

	Character* pair = chr.logic->GetPairCharacter();

	if(state != CharacterLogic::state_findpair)
	{		
		if(chr.Controls().GetControlStateType(lKeyCodes[ChrAccept]) == CST_ACTIVATED)
		{						
			/*bool kick_body = false;

			chr.arbiter->FindCircle(chr, 1.5f);

			if(chr.arbiter->find > 0)
			{			
				array<CharacterFind> & find = chr.arbiter->find;

				for(long i = 0; i < find; i++)
				{								
					if (find[i].chr->ragdoll && find[i].chr->GetHP()>0.01f)
					{
						kick_body = true;
						break;
					}					
				}
			}
			else
			{
				kick_body = false;
			}

			if (kick_body)
			{
				chr.animation->ActivateLink("Kick Body");
			}
			else*/
			{				
				//if (doKick)
				bool activateC = true;
				if (weaponDropped)
				{
					char pairLink[128];
					FatalityParams::TFatalityType type;

					chr.logic->FindCharacterForPair(chr, false, pairLink, type, 1, &activateC);
				}

				if (activateC)
					chr.animation->ActivateLink("C");
			}
		}
	}

	if(pair && chr.logic->IsPairMode())
	{		
		bool isMoneyFatality = false;

		if(chr.logic->GetState() == CharacterLogic::state_topair)
		{
			//Анализируем тип врага, и в соответствии с этим выбираем захват
			if(pair->controller)
			{
				const char * achieve = null;
				const char * action = null;

				if(chr.Controls().GetControlStateType(lKeyCodes[ChrA]) == CST_ACTIVATED &&
				   chr.arbiter->IsFatalityAllowed(0))
				{
					action = "pair A";
					ACHIEVEMENT_REACHED(&chr, WEAPON_MASTER);
					//chr.AchievementReached(GP::Weaponmaster);
					//achieve = "16 Weapon master";
				}
				else
				if(chr.Controls().GetControlStateType(lKeyCodes[ChrB]) == CST_ACTIVATED &&
				   chr.arbiter->IsFatalityAllowed(1))
				{
					action = "pair B";
					ACHIEVEMENT_REACHED(&chr, RAGE);
					//chr.AchievementReached(GP::Rage);
					//achieve = "15 Rage";
				}
				else
				if(chr.Controls().GetControlStateType(lKeyCodes[ChrBlock]) == CST_ACTIVATED &&
				   chr.arbiter->IsFatalityAllowed(2))
				{
					ACHIEVEMENT_REACHED(&chr, MURDERER);
					//chr.AchievementReached(GP::Killa);
					action = "pair C";
					//achieve = "14 Murderer";
				}
				else
				if(chr.Controls().GetControlStateType(lKeyCodes[ChrAccept]) == CST_ACTIVATED &&
				   chr.arbiter->IsFatalityAllowed(3))
				{
					isMoneyFatality = true;
					action = "pair D";
				}

				if(action)
				{
					//Запускаем анимацию, если есть для данного типа противника
					bool testSelf = chr.animation->TestActivateLink(action);
					bool testPair = chr.logic->pair->animation->TestActivateLink(action);
					if(testSelf && testPair)
					{
						pair->animation->ActivateLink(action,true);
						chr.animation->ActivateLink(action,true);
						chr.isRealFatalityStarted = true;
					}

					if(testSelf != testPair || !testSelf)
					{
						//if(api->DebugKeyState(VK_NUMPAD0))
						{
							api->Trace("Can't continue pair animation \"%s\":", action);
							Character * c = pair;
							if(testSelf)
							{
								c = &chr;
							}
							api->Trace("    obect: \"%s\"", c->GetObjectID().c_str());
							api->Trace("    animation: \"%s\"", c->animation->AnimationName());
							api->Trace("    current node: \"%s\"", c->animation->CurrentNode());
						}
					}
					else
					{
						pair->isMoneyFatality = isMoneyFatality;
						//ACHIEVEMENT_REACHED(&chr, MERCILESS);

						/*if (isBlood)
						{
							AddRage(10);							
						}*/
					}
				}
			}
		}
	}	

	if (!doKick) chr.logic->PairProcess(dltTime,filter);
}

bool PlayerController::AllowAccpet()
{
	if ((itemTaken && phys_object) || chr.logic->slave) return false;
	
	chr.arbiter->FindCircle(chr,2.95f);

	//if (chr.logic->GetState() == CharacterLogic::state_findpair)
	{
		for (int i = 0; i < chr.arbiter->find; i++)
		{
			if (chr.arbiter->find[i].chr->logic->IsEnemy(&chr) && chr.arbiter->find[i].chr->GetHP()>0
				&& !chr.arbiter->find[i].chr->logic->IsActor() && !chr.arbiter->find[i].chr->logic->IsPairMode())
			{				
				if(chr.arbiter->find[i].chr->logic->PairModeAllowed(false,true))
				{
					return false;
				}
			}			
		}
	}

	return true;
}

//Применить
bool PlayerController::Accept()
{
	if ((itemTaken && phys_object) || chr.logic->slave) return false;

	if(chr.Controls().GetControlStateType(lKeyCodes[ChrAccept]) != CST_ACTIVATED)
	{
		return false;
	}

	chr.arbiter->FindCircle(chr,2.95f);

	// смотрим если есть истинные враги(без НПС которым можно свернуть голову)
	for (int i = 0; i < chr.arbiter->find; i++)
	{
		Character * fchr = chr.arbiter->find[i].chr;
		if (fchr->logic->IsEnemy(&chr, false) && fchr->GetHP()>0 &&
			!fchr->logic->IsActor() && !fchr->logic->IsPairMode())
		{
			if (fchr->logic->PairModeAllowed(false, true))
				return false;
		}			
	}

	MGIObject it(chr.GroupIterator(MG_ACTIVEACCEPTOR, _FL_));
	while(!it.IsDone())
	{
		AcceptorObject * ao = (AcceptorObject *)it.Get();

		const char* id = ao->GetObjectType();

		/*if (id[0] == 'Q' &&id[1] == 'u' &&id[2] == 'c')
		{
			if (!isEnemyThere)
			{
				if(ao->Accept(&chr)) return;
			}
		}
		else
		{
			if(ao->Accept(&chr)) return;
		}*/

		if(ao->Accept(&chr))
		{
			if (hpBar)
			{					
				hpBar->Show(false);
			}

			//BrokeWeapon(false);

			chr.items->ShowWeapon(false,str_secondId);
		
			if (cur_weapon_style)
			{
				if (cur_weapon_style->weapon_type == wp_gun) chr.items->SetLogicLocator(str_2gunId, ConstString::EmptyObject());
			}

			return true;		
		}
		
		it.Next();
	}

	// смотрим если есть враги(включая НПС которым можно свернуть голову)
	for (int i = 0; i < chr.arbiter->find; i++)
	{
		Character * fchr = chr.arbiter->find[i].chr;
		if (fchr->logic->IsEnemy(&chr, true) && fchr->GetHP()>0 &&
			!fchr->logic->IsActor() && !fchr->logic->IsPairMode())
		{
			if (fchr->logic->PairModeAllowed(false, true))
				return false;
		}			
	}

	return false;
}

//Взрыв брошеной персонажем бомбы
void PlayerController::Boom(const Vector & pos)
{
	chr.arbiter->Boom(&chr, DamageReceiver::ds_bomb, pos, 3.5f, bomb_damage, bomb_radius);

	//Рождаем эффекты
	/*IParticleSystem * p = null;	
	
	p = chr.Particles().CreateParticleSystem("ExplosionBomb");
	chr.Sound().Create3D("bmb_blast", pos, _FL_);
	
	if(p)
	{
		p->Teleport(Matrix(Vector(0.0f), pos));
		p->AutoDelete(true);
	}*/
}

void PlayerController::Shoot(float dmg)
{
	bool pistol_cooldown = true;

	if (itemTaken)
	{
		if (cur_weapon_style)
		{
			if (cur_weapon_style->weapon_type == wp_rifle || cur_weapon_style->weapon_type == wp_gun)
			{
				pistol_cooldown = false;
			}
		}
	}
	
	if (pistol_cooldown) fPistolCooldown=ReadGameState(gsPistolTimeReload,0.1f);

	dmg *= ReadGameState(gsPistolDamage,100.0f) * 0.01f;

	NotifyCharAboutAttack();
}

//Выстрелить
void PlayerController::Fire()
{	
	/*
	//chr.arbiter->FindDraw(chr, 0.5f, 12.0f, -PI*0.03f, PI*0.03f, 6.0f);
	if(chr.Controls().GetControlStateType("ChrShoot") != CST_ACTIVATED) return;
	//Ищем перед собой цель
	chr.arbiter->Find(chr, 0.5f, 12.0f, false, -PI*0.03f, PI*0.03f, 6.0f);
	if(chr.arbiter->find <= 0)
	{
		chr.logic->ShootTarget(null);
		return;
	}
	//Выбираем цель
	Character * trg = FindCharacterTarget();
	//Стреляем по ней
	chr.logic->ShootTarget(trg);*/
}

//Бросание бомбы
void PlayerController::Bomb()
{
	//chr.arbiter->FindDraw(chr, 0.05f, 7.55f, -PI*0.2f, PI*0.2f, 0.0f);
	/*chr.arbiter->Find(chr, 0.05f, 7.55f, false, -PI*0.2f, PI*0.2f, 0.0f);
	chr.arbiter->FindDraw(chr, 0.05f, 7.55f, -PI*0.2f, PI*0.2f, 0.0f);
	array<CharacterFind> & find = chr.arbiter->find;

	MissionObject * trg = null;
	Vector dir = Vector().Make2D(chr.physics->GetAy());	

	for(long i = 0; i < find; i++)
	{
		Matrix mat;

		Character* chr = (Character*)find[i].chr;

		chr->Render().DrawSphere(chr->physics->GetPos(),0.2f,0xffffffff);		
	}*/


	CharacterLogic::State st = chr.logic->GetState();

	if(st != CharacterLogic::state_aimbomb)
	{		
		if(chr.Controls().GetControlStateFloat(lKeyCodes[ChrInstanceDropBomb]) > 0.7f &&		
		   ReadGameState(gsNumBombs,4.0f)>0)
		{
			//Ищем цель среди персонажей
			MissionObject * trg = FindBombTarget(false);
			
			if (chr.animation->TestActivateLink("Drop bomb"))
			{
				NotifyCharAboutAttack();

				//bomb_trg = trg;
				Matrix mtx(true);
				if (trg)
				{
					trg->GetMatrix(mtx);
					chr.physics->Orient(mtx.pos, false, true);
				}
				else
				{
					// Vano: все равно бросаем бомбу, даже если никого нет рядом
					chr.GetMatrix(mtx);
					Vector dir = !Vector(mtx.vz.x, 0.0f, mtx.vz.z);
					mtx.pos = mtx.pos + dir * 6.0f;
				}
				
				chr.SetBombTarget(mtx.pos);
				
				chr.animation->ActivateLink("Drop bomb");
				
				if (ReadGameState(gsNumBombs,4.0f)==ReadGameState(gsMaxBombs,4.0f))
				{
					fBombCooldown=fBombTimeReload;
				}				

				WriteGameState(gsNumBombs,ReadGameState(gsNumBombs,4.0f)-1);

				if (bomb_trg)
				{
					MO_IS_IF(is_Character, "Character", bomb_trg)
					{
						Character* chr = (Character*)bomb_trg;

						/*if (chr->logic->PairModeAllowed(true))
						{
						chr->SetCharacterCoin(Character::chrcoin_fatality);
						}
						else*/
						{
							chr->SetCharacterCoin(Character::chrcoin_none);
						}
					}
				}
			}			
		}		
	
		if (chr.Controls().GetControlStateType(lKeyCodes[ChrInstanceShoot]) == CST_ACTIVE && fPistolCooldown < 0.001f)
		{			
			if (itemTaken && cur_weapon_style && (cur_weapon_style->weapon_type == wp_rifle || cur_weapon_style->weapon_type == wp_gun))
			{
			}
			else
			{
				if (chr.animation->ActivateLink(ReadGameState(gsPlayerShoot, "shoot")))
				{
					chr.logic->AimToTarget(7.0f,Deg2Rad(75.0f));
				}
			}
		}
	}
}

void PlayerController::UpdatePhysObject()
{
	if (phys_object &&  cur_weapon_style && !chr.graph_changed)
	{			
		Matrix mat;				
		chr.FindLocator(cur_weapon_style->locator_name.c_str(),mat);

		Matrix pivot_matrix;
		phys_object->GetPartPivot(0,pivot_matrix);
		phys_object->SetPartMatrix(0,pivot_matrix * mat);			
	}
}

void PlayerController::BlowPhysObject(const Vector & pos, bool splashDamage, bool isAddAchievement)
{
	if (isAddAchievement)
		ACHIEVEMENT_REACHED(&chr, CATAPULT);

	phys_object->ShowTips(true);
	phys_object->BrokeObject();

	if (splashDamage)
		chr.arbiter->SplashDamage(htds_item, &chr, pos, fDamageInPhysObjRadius,fDamageInPhysObjHit,true,null,true,1.0f,true,"PhysObjHit",null,phys_object);

	phys_object = null;

	traceBox = false;
	timeTotraceBox = -1.0f;

	itemTaken = false;
}

bool PlayerController::Weapons(float dltTime, bool & weaponDropped)
{
	if (cooldown_to_dropweapon>0.0f)
	{
		cooldown_to_dropweapon-=dltTime;

		if (cooldown_to_dropweapon<0.0f)
		{
			cooldown_to_dropweapon = -1.0f;
		}
	}
	

	if (phys_object && traceBox)
	{				
		timeTotraceBox -= dltTime;

		if (timeTotraceBox<=0.0f)
		{
			phys_object->ShowTips(true);

			timeTotraceBox = -1.0f;
			phys_object = null;
			traceBox = false;
		}
		else		
		{			
			Matrix mat;
			phys_object->GetPartMatrix(0, mat);

			float speed = (mat.pos - last_pho_pos).GetLength()/dltTime;
		
			Vector chr_pos;

			if (speed > 0.75f && chr.arbiter->IfAnyCharacer(&chr, mat.pos, fPhysObjDetectRadius, chr_pos) && traceBox)
			{	
				BlowPhysObject(mat.pos);
				BrokeWeapon(true);

				return false;
			}
	
			last_pho_pos = mat.pos;
		}
	}

	int item_to_take = -1;
	if (!itemTaken)
	{		
		item_to_take = chr.arbiter->FindClosesetItem(&chr);

		if (item_to_take==-1)
		{
			float cur_dy;
			IMissionPhysObject* box = FindClosestPO(cur_dy);

			if (box)
			{
				if (tip_manager)
				{
					if (!tip_manager->IsAnyActiveTip())	tip_manager->SetTipState(ITip::active,box);	
				}
			}
		}
	}


	if (itemTaken)
	{
		if (!phys_object)		
		{
			if (GetLastWeaponLive()<=0.0f)
			{
				chr.animation->ActivateLink("Break Weapon");
				return false;
			}
		}
	}


	if(chr.Controls().GetControlStateType(lKeyCodes[ChrAccept]) == CST_ACTIVATED)// && !itemTaken)
	{	
		// VANO
		weaponDropped = false;
		if (itemTaken)
		{
			if (!phys_object)		
			{
				if (GetLastWeaponLive()<=0.0f)
				{
					chr.animation->ActivateLink("Break Weapon");
				}
				else
				{
					if(chr.Controls().GetControlStateType(lKeyCodes[ChrAccept]) == CST_ACTIVATED && 
					   !chr.logic->slave && cooldown_to_dropweapon<0.0f)
					{	
						BrokeWeapon(true);

						weaponDropped = true;
						//return true;
					}
				}
			}
		}

		// VANO

		if (!itemTaken || weaponDropped)
		{
			chr.arbiter->FindCircle(chr, 1.5f);		

			for (int i = 0; i < chr.arbiter->find; i++)
				if (chr.logic->IsEnemy(chr.arbiter->find[i].chr) && chr.arbiter->find[i].chr->GetHP()>0
					&& !chr.arbiter->find[i].chr->logic->IsActor() && !chr.arbiter->find[i].chr->logic->IsPairMode())
				{
					return true;
				}

			picked_item = item_to_take;

			if (picked_item != -1)
			{		
				for (int i=0;i<(int)wp_styles.Size();i++)
				{				
					if (wp_styles[i].weapon_type == chr.arbiter->GetDropedWpType(picked_item))
					{
						cur_weapon_style = &wp_styles[i];

						chr.animation->SetAnimation(cur_weapon_style->anim_id.c_str());					
						chr.animation->Goto("pick up",0.0f);

						itemTaken = true;

						cooldown_to_dropweapon = 0.25f;

						break;
					}
				}		
			}		

			phys_object = null;		

			if (!itemTaken)
			{	
				float cur_dy;

				IMissionPhysObject* box = FindClosestPO(cur_dy);

				if (box)
				{
					for (int i=0;i<(int)wp_styles.Size();i++)
					{
						if (wp_styles[i].weapon_type == wp_box)	
						{
							cur_weapon_style = &wp_styles[i];

							chr.animation->SetAnimation(cur_weapon_style->anim_id.c_str(),-1.0f);						

							if (cur_dy>0.6f)
							{
								chr.animation->Goto("pick up up",0.0f);
							}
							else
							{
								chr.animation->Goto("pick up",0.0f);
							}

							chr.graph_changed = true;

							itemTaken = true;							
						
							cooldown_to_dropweapon = 0.25f;

							phys_object = box;
							phys_object->Activate(false);
							phys_object->ShowTips(false);
							
							if (tip_manager)
							{
								tip_manager->SetTipState(ITip::inactive,phys_object);	
							}

							Matrix mat;
							box->GetMatrix(mat);
							last_pho_pos = mat.pos;

							traceBox = false;

							delayShowCurWpWidget = 1.0f;

							break;
						}
					}
				}			
			}

			/*if (!itemTaken)
			{
				chr.arbiter->FindCircle(chr, 1.5f);
					
				Matrix mtx = chr.physics->GetMatrixWithBoneOffset(mtx);		
				Vector pos = mtx.pos;

				if(chr.arbiter->find > 0)
				{			
					array<CharacterFind> & find = chr.arbiter->find;

					for(long i = 0; i < find; i++)
					{			
						if (find[i].chr == &chr) continue;

						if (find[i].chr->logic->IsPairMode()) continue;
						if (!find[i].chr->ragdoll) continue;
						

						if (TakeWeapon(find[i].chr->animation->GetCharName(), "", 0.2f))
						{						
							find[i].chr->ActivateRagdoll(false,0.01f,0,false);					

							find[i].chr->logic->is_attached = true;
							find[i].chr->logic->master = &chr;
							find[i].chr->physics->Activate(false);
		
							find[i].chr->animation->Goto("Grab",0.2f);
		
							chr.logic->slave = find[i].chr;

							return true;
						}										
					}
				}
			}*/
		}
	}
	/*else
	if (itemTaken)
	{
		if (!phys_object)		
		{
			if (GetLastWeaponLive()<=0.0f)
			{
				chr.animation->ActivateLink("Break Weapon");
			}
			else
			{
				if(chr.Controls().GetControlStateType(lKeyCodes[ChrAccept]) == CST_ACTIVATED && 
				   !chr.logic->slave && cooldown_to_dropweapon<0.0f)
				{	
					BrokeWeapon(true);

					weaponDropped = true;
					return true;
				}
			}
		}
	}*/

	return true;
}

//Можно ли перемещатся
bool PlayerController::IsCanMove()
{
	CharacterLogic::State st = chr.logic->GetState();
	switch(st)
	{
		case CharacterLogic::state_block:
		case CharacterLogic::state_aimbomb:
		case CharacterLogic::state_dropbomb: return false;
	}

	return true;
}

//Найти цель среди персонажей в массиве find
inline Character * PlayerController::FindCharacterTarget()
{
	array<CharacterFind> & find = chr.arbiter->find;
	if(!find)
	{
		return null;
	}
	float sort;
	Character * trg = null;
	Vector dir = Vector().MakeXZ(chr.physics->GetAy());
	for(long i = 0; i < find; i++)
	{
		Vector & dv = find[i].dv;
		float dist = dv.NormalizeXZ();
		float cs = dir | dv;
		if(cs < 0.0f) continue;
		float k = GetTargetWeight(cs, dist);
		if(!trg || k < sort)
		{
			trg = find[i].chr;
			sort = k;
		}
	}
	return trg;
}

void PlayerController::RestoreSecodWeapon()
{
	if (itemTaken)
	{		
		if (phys_object)
		{
			phys_object->Show(true);
		}
		else
		{
			chr.items->ShowWeapon(true,str_secondId);
			if (cur_weapon_style)
			{				
				if (cur_weapon_style->weapon_type == wp_gun) chr.items->SetLogicLocator(str_2gunId, str_locator17Id);
			}
		}

		if (traceBox)
		{
			itemTaken = false;
			traceBox = false;
			cur_weapon_style = null;

			if (phys_object)
			{
				phys_object->Activate(true);
			}

			phys_object = null;
		}
		else
		{
			if (cur_weapon_style)
				chr.animation->SetAnimation(cur_weapon_style->anim_id.c_str());					

			chr.animation->Goto("Idle",0.0f);
		}

		//if (phys_object)
		{
			//phys_object->Activate(true);
			//phys_object->ShowTips(true);
			//phys_object = null;
			//itemTaken = false;
		}
		//else
		
	}
}

//Найти цель для бомбы
MissionObject * PlayerController::FindBombTarget(bool CheckBombDistance)
{
	bAllowBomb = false;
	bAllowShoot = false;

	//chr.arbiter->Find(chr, 2.5f, 12.0f, false, -PI*0.4f, PI*0.4f, 0.0f);
	chr.arbiter->Find(chr, 0.5f, 12.0f, false, -PI*0.5f, PI*0.5f, 0.0f);
	//chr.arbiter->FindDraw(chr, 2.5f, 12.0f, -PI*0.4f, PI*0.4f, 0.0f);
	array<CharacterFind> & find = chr.arbiter->find;
	float sort;
	MissionObject * trg = null;
	Vector dir = Vector().MakeXZ(chr.physics->GetAy());
	for(long i = 0; i < find; i++)
	{
		if(find[i].chr->logic->GetHP()<0.1f) continue;

		if(find[i].chr->logic->GetState() == CharacterLogic::state_dead) continue;
		if(find[i].chr->logic->GetState() == CharacterLogic::state_die) continue;

		if(find[i].chr->ragdoll) continue;
		
		if(!find[i].chr->logic->IsEnemy(&chr)) continue;

		if(find[i].chr->logic->IsPairMode()) continue;

		if (chr.logic->InBombSafe(find[i].chr->physics->GetPos())) continue;

		if (!chr.arbiter->IsSphereInFrustrum(find[i].chr->physics->GetPos(), 0.75f)) continue;

		Vector & dv = find[i].dv;
		float dist = dv.NormalizeXZ();
		float cs = dir | dv;
		if(cs < 0.0f) continue;		

		float k = GetTargetWeight(cs, dist);
		if(!trg || k < sort)
		{
			bAllowShoot = true;
			trg = find[i].chr;

			Matrix mat;			
			trg->GetMatrix(mat);

			float dist = (mat.pos - chr.physics->GetPos()).GetLengthXZ2();

			if (dist < Sqr(2.5f) && CheckBombDistance)
			{
				bAllowBomb = false;
			}
			else
			{
				bAllowBomb = true;
			}

			sort = k;
		}
	}
	//Ищем цель среди бомбоцелей		
	const Vector & chrPos = chr.physics->GetPos();
	static const float minCos = cosf(PI*0.4f);
	for(bombTargets->Reset(); !bombTargets->IsDone(); bombTargets->Next())
	{
		if (!chr.arbiter->IsSphereInFrustrum(bombTargets->Get()->GetMatrix(Matrix(true)).pos, 0.75f)) continue;

		Vector dv = bombTargets->Get()->GetMatrix(Matrix(true)).pos - chrPos;
		float dist = dv.NormalizeXZ();

		BombTarget* bomb_targey = (BombTarget*)bombTargets->Get();

		if (dist>bomb_targey->fAimDistance) continue;

		float cs = dir | dv;
		if(cs < minCos) continue;
		float k = GetTargetWeight(cs, dist);
		if(!trg || k < sort)
		{
			bAllowShoot = false;
			trg = bombTargets->Get();
			sort = k;

			bAllowBomb = true;
		}
	}
	return trg;
}

//Получить коэфициент веса
inline float PlayerController::GetTargetWeight(float trgCos, float trgDist)
{
	float k = trgDist*(1.05f - trgCos);	

	return k;
}

void PlayerController::Attack(Character* _target)
{
	if (chr.logic->GetHP()<1.0f) return;

	if (_target)
	{
		if (_target->logic->GetState() != CharacterLogic::state_block && _target->logic->IsEnemy(&chr))
		{
			if (combo_cur_timespawn<0)
			{
				combo_times = 1;
				//iCurHitsToRage = 0;
			}
			else
			{			
				combo_times++;
			}
		
			combo_cur_timespawn = combo_timespawn;
			combo_cur_time_dec = combo_time_dec;
	

			/*if (!chr.logic->IsRageMode())
			{
				AddRage(1);				
			}*/

			if (chr.logic->IsRageMode())
			{
				if (combo_rage_times_max<combo_times) combo_rage_times_max = combo_times;					
				ActivateTriger(triger_RageAttack);
			}
			else
			{
				if (combo_times_max<combo_times) combo_times_max = combo_times;
			}

			combo_widget.Command(&chr,"PlayEffect",0,null);			
		}
	}
	
	if (_target == chr_showed_HUD)
	{
		chr_HUD_cooldown = 2.0f;
	}

	if (chr_HUD_cooldown<=1.01f)
	{		
		if (hpBar)
		{					
			hpBar->Show(false);
		}	
		
		hpBar = _target->hpBar;
		
		if (hpBar)
		{			
			hpBar->Show(true);

			const char* params[2];
			params[0] = _target->GetObjectID().c_str();

			hpBar->Command("SetMO",1,params);

			chr_HUD_cooldown = 2.0f;
			chr_showed_HUD = _target;
		}
	}
	else
	{
		chr_need_show_HUD = _target;
	}
}

void PlayerController::DrawHUD(float dltTime)
{		
	{
		money_widget.Show(&chr,(money_txt_cooldown>0.01f));			

		if (money_widget.IsShow(&chr))
		{
			const char* params[2];		

			char str[32];			
			crt_snprintf(str, sizeof(str),"%i",(int)(/*ReadGameState(gsMoney,0.0f)+*/money_ticked));	

			params[0] = str;

			money_widget.Command(&chr,"SetString",1,params);
		}
	}

	combo_cur_timespawn -= dltTime;

	if (combo_cur_timespawn<0 && combo_times>0)
	{		
		if (chr.logic->IsPairMode())
		{
			combo_cur_timespawn = combo_timespawn;
		}
		else
		{
			combo_cur_time_dec -= dltTime;

			if (combo_cur_time_dec<0)
			{
				int times = 1 - (int)(combo_cur_time_dec/combo_time_dec);
				
				combo_times-=times;

				if (combo_times<0) combo_times = 0;

				/*if (iCurHitsToRage>0 && !chr.logic->IsRageMode())
				{
					//iCurHitsToRage = 0;

					iCurHitsToRage-=times;
					if (iCurHitsToRage<0) iCurHitsToRage = 0;
				}*/

				if (combo_times>0)
				{
					combo_cur_time_dec = combo_time_dec;
				}
				else
				{
					combo_times_max = 0;
					combo_rage_times_max = 0;
					cur_mombo = -1;
					cur_time_to_hide_mombo = 0;
				}
			}
		}		
	}

	{				
		if (combo_cur_timespawn<0.01f && combo_times<=0)
		{
			combo_widget.Show(&chr,false);
		}
		else
		{
			const char* params[2];		

			char str[32];

			if (chr.logic->IsRageMode())
			{					
				crt_snprintf(str, sizeof(str),"x `%i",(int)(combo_times + combo_ticked));	
			}
			else
			{
				crt_snprintf(str, sizeof(str),"x `%i",(int)(combo_times + combo_ticked));	
			}				

			params[0] = str;
				
			combo_widget.Show(&chr,true);

			combo_widget.Command(&chr,"SetString",1,params);
		}
	}

	{				
		if (combo_cur_timespawn<0.01f && combo_times<=0)
		{
			combo_moto_widget.Show(&chr,false);
		}
		else
		{
			const char* combo_str = "";

			int cmb_times = -1;
			int index;

			//if (!chr.logic->IsRageMode())
			{
				for (int i=0;i<(int)comboMoto.Size();i++)
				{
					if (comboMoto[i].iHits<=combo_times_max && cmb_times<comboMoto[i].iHits)
					{
						cmb_times = comboMoto[i].iHits;
						combo_str = comboMoto[i].moto;
						index = i;
					}
				}
			}
			/*else
			{
				for (int i=0;i<(int)comboRageMoto.Size();i++)
				{
					if (comboRageMoto[i].iHits<=combo_rage_times_max && cmb_times<comboRageMoto[i].iHits)
					{
						cmb_times = comboRageMoto[i].iHits;
						combo_str = comboRageMoto[i].moto;
						index = i;
					}
				}
			}*/

			if (cur_mombo != index || chr.logic->IsRageMode())
			{
				if (!chr.logic->IsRageMode()) cur_mombo = index;

				const char* params[2];				
									
				params[0] = combo_str;

				combo_moto_widget.Show(&chr,true);
				combo_moto_widget.Command(&chr,"SetString",1,params);
				combo_moto_widget.Command(&chr,"PlayEffect",0,null);

				cur_time_to_hide_mombo = time_to_hide_mombo;					
			}			
			else
			{
				cur_time_to_hide_mombo -= dltTime;

				if (cur_time_to_hide_mombo<0)
				{
					cur_time_to_hide_mombo = 0.0f;
					combo_moto_widget.Show(&chr,false);
				}
			}
		}
	}
}

void PlayerController::AddBomb(int bombs)
{	
	WriteGameState(gsNumBombs,ReadGameState(gsNumBombs,4.0f)+bombs);

	if (ReadGameState(gsNumBombs,4.0f)>ReadGameState(gsMaxBombs,4.0f))
	{
		WriteGameState(gsNumBombs,ReadGameState(gsMaxBombs,4.0f));
		fBombCooldown = 0.0f;
	}
}

bool PlayerController::IsMaxBombs()
{
	if (ReadGameState(gsNumBombs,4.0f)==ReadGameState(gsMaxBombs,4.0f)) return true;

	return false;
}

void PlayerController::ChargePistol()
{
	fPistolCooldown = 0.0f;	
}

void PlayerController::QuadDamage(float work_time)
{

}

void PlayerController::Hit(float dmg, Character* _offender, const char* reaction, DamageReceiver::DamageSource source)
{
	if (chr.logic->GetHP()/chr.logic->GetMaxHP() < restorehp_params[cur_reshp_params].when_resore_HP)
	{
		cur_time_to_restore_HP = restorehp_params[cur_reshp_params].time_to_restore_HP;
	}

	if (qevent)
	{
		if (qevent->InteruptOnHit())
		{
			qevent->Interupt(false);		
			//RestoreSecodWeapon();
		}
	}
}

void PlayerController::PickUpWeapon()
{
	if (picked_item!=-1)
	{
		CharacterPattern::TItemDescr descr;

		CharactersArbiter::TDropedItemInitData* item_data = chr.arbiter->GetDropedWpItemData(picked_item);

		descr.id = str_secondId;
		descr.modelName = item_data->model_name;
		descr.boneName = "";
		Assert(cur_weapon_style);
		descr.locatorName = cur_weapon_style->locator_name;
		descr.weapon_type = cur_weapon_style->weapon_type;

		descr.broken_part1 = item_data->broken_part1;
		descr.broken_part2 = item_data->broken_part2;

		descr.tip_id = item_data->tip_id;
		descr.fProbality = 100;
		descr.time_lie = 10;

		if (item_data->item_life>0)
		{
			descr.itemLife = item_data->item_life;
		}
		else
		{
			FindWpHPParam();
			descr.itemLife = GetWpHPParam();
		}		

		descr.uniqTexture = cur_weapon_style->trail_name;		
		descr.trailTechnique = cur_weapon_style->trail_technique;
		descr.trailPower = cur_weapon_style->trail_power;
		descr.useTrail = !(!descr.uniqTexture || descr.uniqTexture[0]==0);
		descr.showFlare = false;
		descr.locatortoattache = "";
		descr.attachedobject.Empty();
		descr.objectoffset = Vector(0.0f);
		descr.effectTable = cur_weapon_style->effectTable;

		chr.items->AddItem(&descr);

		chr.arbiter->TakeItem(picked_item);

		delayShowCurWpWidget = 1.0f;

		picked_item = -1;

		if (cur_weapon_style->weapon_type == wp_rapire) chr.items->SetLogicLocator(str_rapireId, ConstString::EmptyObject());
		if (cur_weapon_style->weapon_type == wp_gun) chr.items->SetLogicLocator(str_2gunId, str_locator17Id);
	}
}

void PlayerController::BrokeWeapon(bool need_anim_blend)
{
	vanoHackDropPhysObjectWhenActor = false;
	watchCollisionWithPhysObject = false;
	isDropStarted = false;

	ShowCurWeaponWidget(false);

	if (need_anim_blend) AnimBlend();

	if (cur_weapon_style)
	{
		if (cur_weapon_style->weapon_type == wp_rapire) chr.items->SetLogicLocator(str_rapireId, str_locator9Id);
		if (cur_weapon_style->weapon_type == wp_gun) chr.items->SetLogicLocator(str_2gunId, ConstString::EmptyObject());

		chr.BornParticle(cur_weapon_style->locator_name.c_str(),cur_weapon_style->particale_name.c_str());				

		Vector pos;
		chr.FindLocator(cur_weapon_style->locator_name.c_str(),pos);
		chr.Sound().Create3D(cur_weapon_style->break_sound, pos, _FL_);
	}

	chr.items->DropItem(str_secondId, GetLastWeaponLive()<=0);	
	chr.items->DelItems(str_secondId);	
	
	itemTaken = false;
	cur_weapon_style = NULL;

	//ShowCurWeaponWidget(false);
}

bool PlayerController::IsWeaponOrPhysicsBoxEquiped()
{
	return (cur_weapon_style || phys_object);
}

void PlayerController::DropWeapon(bool add_impulse)
{
	if (!phys_object)
	{
		BrokeWeapon(false);

		return;
	}

	phys_object->Activate(true);

	if (add_impulse && cur_weapon_style)
	{		
		Matrix mat;

		chr.FindLocator(cur_weapon_style->locator_name.c_str(),mat);
	
		Vector impulse = Vector(sin(chr.physics->GetAy()),0,cos(chr.physics->GetAy()));
		impulse  *= fPhysObj_PushZ;
		impulse.y = fPhysObj_PushY * 1.2f;

		phys_object->Impulse(impulse * 0.3f);

		traceBox = true;

		timeTotraceBox = 2.5f;
	}
	else
	{
		phys_object->ShowTips(true);
		//phys_object->BrokeObject();
	}
}

void PlayerController::AnimBlend()
{
	itemTaken = false;

	if (chr.animation->GetCurAnimation())
	{
		chr.events->DelAllParticles(chr.animation->GetCurAnimation(),null,null,0);

		if (cur_weapon_style)
		{
			if (cur_weapon_style->weapon_type != wp_box) chr.animation->GetCurAnimation()->Start();
		}		
	}

	if (cur_weapon_style)
	{
		if (cur_weapon_style->weapon_type == wp_rapire) chr.items->SetLogicLocator(str_rapireId, str_locator9Id);
		if (cur_weapon_style->weapon_type == wp_gun) chr.items->SetLogicLocator(str_2gunId, ConstString::EmptyObject());
	}

	//ShowCurWeaponWidget(false);

	chr.animation->SetLastAnimation();	
	chr.animation->Goto("Idle",0.0f);
	
	//Vano - убрал тут иначе все уничтожалось оружие когда бросаешь его
	//cur_weapon_style = null;	
}

bool PlayerController::TakeWeapon(TWeaponType wp_type,const char* weapon_model,								  
								  const char* broken_part1,const char* broken_part2,
								  const ConstString & tip_id,float blend_time, const char* node)
{
	if (itemTaken)
	{
		//chr.animation->ActivateLink("Break Weapon");
		//chr.animation->SetAnimation("base", 0.0f);
		chr.items->ShowWeapon(true, str_secondId);
		BrokeWeapon(false);
		itemTaken = false;
		chr.items->ShowWeapon(false, str_secondId);
	}

	for (int i=0;i<(int)wp_styles.Size();i++)
	{
		if (wp_styles[i].weapon_type == wp_type)
		{
			cur_weapon_style = &wp_styles[i];
			
			chr.animation->SetAnimation(cur_weapon_style->anim_id.c_str(),blend_time);

			if (node)
			{
				chr.animation->Goto(node,0.0f);
			}
			else
			{
				chr.animation->Goto("Idle",0.0f);
			}

			itemTaken = true;


			CharacterPattern::TItemDescr descr;

			descr.id = str_secondId;
			descr.modelName = weapon_model;
			descr.boneName = "";
			descr.broken_part1 = broken_part1;
			descr.broken_part2 = broken_part2;					
			descr.locatorName = cur_weapon_style->locator_name;
			descr.useTrail = true;
			descr.weapon_type = cur_weapon_style->weapon_type;

			descr.tip_id = tip_id;
			descr.fProbality = 100;
			descr.time_lie = 10;
			FindWpHPParam();
			descr.itemLife = GetWpHPParam();
			descr.uniqTexture = cur_weapon_style->trail_name;						
			descr.trailTechnique = cur_weapon_style->trail_technique;
			descr.trailPower = cur_weapon_style->trail_power;
			if (!descr.uniqTexture || descr.uniqTexture[0]==0) descr.useTrail = false;
			descr.showFlare = false;
			descr.locatortoattache = "";
			descr.attachedobject.Empty();
			descr.objectoffset = Vector(0.0f);
			descr.effectTable = cur_weapon_style->effectTable;
						
			delayShowCurWpWidget = 1.0f;			

			chr.items->AddItem(&descr);

			picked_item = -1;

			if (wp_type == wp_rapire) chr.items->SetLogicLocator(str_rapireId, ConstString::EmptyObject());
			if (wp_type == wp_gun) chr.items->SetLogicLocator(str_2gunId, str_locator17Id);

			return true;
		}
	}

	return false;
}

bool PlayerController::TakeChachedWeapon(const ConstString & chaced_wp_id, 
							             float blend_time, const char* node)
{
	for (int i=0;i<(int)wp_cache.Size();i++)
	{
		if (wp_cache[i].id == chaced_wp_id)
		{
			TakeWeapon(wp_cache[i].weapon_type,
					   wp_cache[i].model_name,
					   wp_cache[i].model_part1_name,wp_cache[i].model_part2_name,
					   wp_cache[i].tip_id, blend_time, node);

		}
	}

	return false;
}

void PlayerController::KickBody()
{
	chr.arbiter->FindCircle(chr, 1.5f);

	if(chr.arbiter->find > 0)
	{			
		array<CharacterFind> & find = chr.arbiter->find;

		for(long i = 0; i < find; i++)
		{			
			if (!find[i].chr->ragdoll) continue;
			if (find[i].chr->GetHP()<0.01f) continue;

			Vector dv = find[i].dv;
			dv.Normalize();

			//dv.x *=1.5f * 10.0f;
			//dv.y  = 7.0f;
			//dv.z *=1.5f * 10.0f;

			dv.x *= 0.0f;
			dv.y  = 3.0f;
			dv.z *= 0.0f;

			find[i].chr->ragdoll->ApplyImpulse( dv );			
			find[i].chr->logic->SetHP(find[i].chr->logic->GetHP() - find[i].chr->items->CalcDamage(fDamageInGroundHit));

			if (find[i].chr->logic->IsDead())
			{
				find[i].chr->DropItems();
			}

			chr.physics->Orient(chr.physics->GetPos()+find[i].dv);

			IParticleSystem * p = chr.Particles().CreateParticleSystem("Blood_Offic.xps");

			if(p)
			{			
				Matrix mt;
				find[i].chr->physics->GetMatrixWithBoneOffset(mt);

				Matrix mat;
				mat.RotateX(PI);
				mat.pos = mt.pos;
				p->Teleport(mat);
				p->AutoDelete(true);
			}

			Attack(find[i].chr);

			break;
		}	
	}
}

void PlayerController::ShowCurWeaponStateWidget()
{
	if (!cur_weapon_style) return;

	if (phys_object)
	{
		cur_weapon_style->widgetTake.Show(&chr,true);

		return;
	}

	float item_life = GetLastWeaponLive();

	if (delayShowCurWpWidget<0.0f && item_life>0.0f)
	{
		cur_weapon_style->widgetTake.Show(&chr,item_life>0.3f);
		cur_weapon_style->widgetDamaged.Show(&chr,!(item_life>0.3f));
	}
}

float PlayerController::GetLastWeaponLive()
{
	if (cur_weapon_style)
	{
		if (cur_weapon_style->weapon_type == wp_bombardier_body)
			return 1.0f;
	}

	return chr.items->GetLastWeaponLive() / GetWpHPParam();
}

void PlayerController::ShowCurWeaponWidget(bool show)
{
	if (!cur_weapon_style) return;

	if (show)
	{		
		ShowCurWeaponStateWidget();
					
		cur_weapon_style->widgetTake.Command(&chr,"Reset",0,NULL);
		cur_weapon_style->widgetTake.Command(&chr,"Play",0,NULL);		

		widgetMainWeapon.Show(&chr,false);
	}
	else
	{		
		cur_weapon_style->widgetTake.Show(&chr,false,true);	
		cur_weapon_style->widgetDamaged.Show(&chr,false,true);
		cur_weapon_style->widgetDrop.Show(&chr,true);	

		cur_weapon_style->widgetDrop.Command(&chr,"Reset",0,NULL);
		cur_weapon_style->widgetDrop.Command(&chr,"Play",0,NULL);
	}

}

void PlayerController::MoneyBeatUp()
{
	if (!chr.logic->IsPairMode()) return;

	//Attack(chr.logic->pair);

	chr.logic->pair->logic->SetHP(coremax(chr.logic->pair->logic->GetHP() - chr.logic->pair->logic->GetMaxHP()*dmgMoneyBeatUp,1.0f));
}

void PlayerController::Command(dword numParams, const char ** params)
{
	if (numParams<1) return;

	if (string::IsEqual(params[0],"drop_second"))
	{
		DropWeapon(true);

		if (itemTaken)
		{
			chr.animation->SetLastAnimation();

			chr.items->DelItems(str_secondId);		

			ShowCurWeaponWidget(false);

			itemTaken = false;
			cur_weapon_style = null;
		}		
	}
	else
	if (string::IsEqual(params[0],"restoreHP"))
	{
		char * pos = null;
		float value = (float)strtod(params[1], &pos) * 0.01f;
		value = Clampf(value);		

		if (useGameState)
		{
			chr.logic->SetHPMultipler(Clampf(ReadGameState(gsHPLive,100.0f) * 0.01f,0.5f,256.0f));	
		}

		chr.logic->SetHP(chr.logic->GetMaxHP() * value );
	}
	else
	if (string::IsEqual(params[0],"show_second"))
	{
		bool show = false;
		
		if (params[1][0] == 'y' || params[1][0] == 'Y')
		{
			show = true;
		}

		chr.items->ShowWeapon(show,str_secondId);
	}
	else
	if (string::IsEqual(params[0],"goto"))
	{
		float value = 0.2f;
		
		if (numParams>=3)
		{
			char * pos = null;
			value = (float)strtod(params[2], &pos) * 0.01f;
		}
		
		chr.animation->Goto(params[1],value);
	}
	else
	if (string::IsEqual(params[0],"beginrage"))
	{
		BeginRage();
	}
}

void PlayerController::CheckSlowmo(const CharacterController::SlowmoParams & smp)
{
	// если slowmo включено
	if (slomo_time > -slomo_cooldown) 
		return;

	chr.arbiter->FindCircle(chr, smp.radius);
	array<CharacterFind> & find = chr.arbiter->find;						

	for (int i = 0; i < find; i++)
	{			
		Character * fchr = find[i].chr;
		
		if (fchr == &chr) continue;
		if (fchr->logic->IsActor() || fchr->logic->IsPairMode()) continue;
		if (fchr->logic->GetState() == CharacterLogic::state_block) continue;
		if (!chr.logic->IsEnemy(find[i].chr)) continue;
		if (chr.IsDead()) continue;

		float hp = find[i].chr->GetHP();
		if (hp <= 0.1f) continue;

		// FIX-ME: сделать функцию у чара, которая скажет сколько по нему пройдет дамага
		float killCoeff = (hp <= smp.damage * find[i].chr->fHitDmgMultiplayer) ? 3.0f : 1.0f;
		float probability = smp.probability * killCoeff;

		if (Rnd() < probability)
		{
			StartSloMo(smp.duration, smp.timeScale);
			break;
		}
	}
}

void PlayerController::BeginSloMo(int id, float time)
{	
	if (!mission_time) return;

	time *= 1.1f;

	if (chr.logic->IsPairMode() || chr.logic->IsActor())
	{
		StartSloMo(time);

		return;
	}	

	if (IsSlowmoReady())
	{
		float dmg = chr.animation->GetConstFloat("damage");
		
		const char * react = chr.animation->GetConstString("hit");

		if (react)
		{
			if (string::IsEqual(react,"Blow")||
				string::IsEqual(react,"HeadOff")||
				string::IsEqual(react,"Legcut"))
			{
				dmg = 10000;
			}
		}

		Matrix mat;
		chr.physics->GetModelMatrix(mat);				

		if (dmg>0.0f)
		{
			chr.arbiter->FindCircle(chr,3.5f);

			array<CharacterFind> & find = chr.arbiter->find;						

			for (int k=0;k<(int)slomo_markers.Size();k++)
			{
				if (slomo_markers[k].id == id)
				{
					for (int j=1;j<(int)slomo_markers[k].markers.Size();j++)
					{
						static Vector quadrangle[4];
						quadrangle[0] = mat * slomo_markers[k].markers[j].s;
						quadrangle[1] = mat * slomo_markers[k].markers[j].e;
						quadrangle[2] = mat * slomo_markers[k].markers[j-1].e;
						quadrangle[3] = mat * slomo_markers[k].markers[j-1].s;
												
						for (int i = 0; i < find; i++)
						{							
							if (find[i].chr == &chr) continue;
							if (find[i].chr->logic->GetState() == CharacterLogic::state_block) continue;

							SlomoChecks++;
							isSlomoHit |= chr.items->CheckHit(find[i].chr, quadrangle);

							if (find[i].chr->GetHP()>0 && find[i].chr->GetHP()<=dmg * find[i].chr->fHitDmgMultiplayer &&
								chr.items->CheckHit(find[i].chr,quadrangle)&&
								chr.logic->IsEnemy(find[i].chr) &&!find[i].chr->logic->IsActor() &&
								!find[i].chr->logic->IsPairMode())
							{
								StartSloMo(time, "0.2");
								
								break;
							}
						}
					}
					break;
				}
			}			
		}

	}
}

bool PlayerController::IsSlowmoReady() const
{
	return slomo_time <= -slomo_cooldown && chr.logic->GetHP() >= 1.0f;
}

void PlayerController::StartSloMo(float time, const char * timeScale)
{
	if (!mission_time) return;
	if (!IsSlowmoReady()) return;

	slomo_time = time;

	const char* params[3];
	params[0] = timeScale;
	params[1] = "0.3";
	mission_time->Command("set", 2, params);

	chr.logic->SetAnimSlowDown(false);

	if (snd_Slow_Motion_In)
	{
		snd_Slow_Motion_In->Play();
	}

	if (snd_Slow_Motion_Loop)
	{
		snd_Slow_Motion_Loop->FadeIn(0.6f);
	}

	params[0] = "5";
	params[1] = "slomo";
	params[2] = "0.3";
	
	if (snd_Slow_Motion_Env.Validate())
		snd_Slow_Motion_Env.Ptr()->Command("set", 3, params);

}

void PlayerController::EndSloMo(bool instance)
{
	if (mission_time)
	{
		const char* params[2];
		params[0] = "1.0";

		if (instance)
		{		
			params[1] = "0.01";
		}
		else
		{		
			params[1] = "1.25";
		}

		mission_time->Command("set",2,params);

		chr.logic->SetAnimSlowDown(true);

		if (snd_Slow_Motion_Loop)
		{
			snd_Slow_Motion_Loop->FadeOut(0.6f);
		}
		
		if (snd_Slow_Motion_Out)
		{
			if (instance)
			{
				snd_Slow_Motion_Out->Stop();
			}
			else
			{
				snd_Slow_Motion_Out->Play();
			}
		}

		params[0] = "5";
		params[1] = "0.3";
		if (snd_Slow_Motion_Env.Validate())
			snd_Slow_Motion_Env.Ptr()->Command("reset", 1, params);
	}
}

void PlayerController::AddSloMoMarker(int id, int sub_id, const ConstString & wp_id)
{	
	for (int i=0;i<(int)slomo_markers.Size();i++)
	{
		if (slomo_markers[i].id == id)
		{
			for (int j=0;j<(int)slomo_markers[i].markers.Size();j++)
			{
				if (slomo_markers[i].markers[j].id == sub_id)
				{
					cur_marker = i;

					return;
				}
			}		

			break;
		}
	}

	TSloMoMarker* slomo_marker;

	if (i<(int)slomo_markers.Size())
	{
		slomo_marker = &slomo_markers[i];
		cur_marker = i;
	}
	else
	{
		slomo_marker = &slomo_markers[slomo_markers.Add()];
		slomo_marker->id = id;

		cur_marker = slomo_markers.Size()-1;
	}

	TSloMoSubMarker marker;
	
	marker.id = sub_id;
	chr.items->GetWeaponTrail(marker.s,marker.e,wp_id);	

	slomo_marker->markers.Add(marker);	
}

void PlayerController::EndQuickEvent()
{
	if (qevent) qevent->Interupt(false);	
}

void PlayerController::BeginRage()
{
	fCurRageTime = ReadGameState(gsRageTimeAction,10.0f) * ReadGameState(gsCurRage,0.0f) / (float)iHitsToRage;
	ActivateTriger(triger_RageOn);
	if (chr.chrAchievements) 
		chr.chrAchievements->StartCountDeathInRage(true);

	rageaction.Show(&chr, true);
	ragebar.Show(&chr, false);
	
	chr.logic->EnableRageMode(true);

	WriteGameState(gsCurRage, 0.0f);
}

void PlayerController::EndRage()
{
	chr.animation->ActivateLink("EndRage");

	EnableRageMode(false);
}

void PlayerController::EnableRageMode(bool enable)
{
	if (enable == chr.logic->IsRageMode()) return;
	 
	if (enable)
	{		
		chr.animation->ActivateLink("BeginRage");
	}
	else
	{
		// Vano: восстанавливаем непотраченную ярость, а то игроку будет обидно!
		if (fCurRageTime > 0.1f)
		{
			float curRage = fCurRageTime * float(iHitsToRage) / ReadGameState(gsRageTimeAction, 10.0f);
			WriteGameState(gsCurRage, curRage);
		}

		fCurRageTime = 0;
		ActivateTriger(triger_RageOff);
		combo_moto_widget.Show(&chr,false);
		if (chr.chrAchievements) 
			chr.chrAchievements->StartCountDeathInRage(false);
		chr.logic->EnableRageMode(false);
		rageaction.Show(&chr, false, true);
		ragebar.Show(&chr, true, true);
	}
}

void PlayerController::AddMoney(float add_money)
{
	WriteGameState(gsTotalMoney,ReadGameState(gsTotalMoney,0.0f)+add_money);

	if (add_money > 10000)
	{
		WriteGameState(gsMoney, ReadGameState(gsMoney,0.0f) + add_money);
		money_ticked = ReadGameState(gsMoney, 0.0f) - 1000;
	}
	else
	{
		WriteGameState(gsMoney,ReadGameState(gsMoney,0.0f)+add_money);

		//need_to_add_money+=add_money;
		//cur_money_add_tick += money_add_tick;
	}

	/*float total = gsMoney->Get();
	float earned = total - startGold;*/
	chr.arbiter->StatsMoneyAdd(add_money);
}

void PlayerController::AddRage(int rage_tick)
{
	if (chr.logic->IsRageMode())
	{		
		fCurRageTime += ReadGameState(gsRageTimeAction,10.0f) * (float)rage_tick / (float)iHitsToRage;

		if (fCurRageTime>ReadGameState(gsRageTimeAction,10.0f))
		{
			fCurRageTime=ReadGameState(gsRageTimeAction,10.0f);
		}		
	}
	
	if (!IsFullRage())
	{
		WriteGameState(gsCurRage, ReadGameState(gsCurRage,0.0f) + rage_tick);

		if (ReadGameState(gsCurRage,0.0f)>iHitsToRage)
		{
			WriteGameState(gsCurRage, (float)iHitsToRage);
			ActivateTriger(triger_RageFull);			
		}
	}
}

bool PlayerController::IsFullRage()
{
	if (ReadGameState(gsCurRage,0.0f) == iHitsToRage)
	{
		return true;
	}

	return false;
}

IMissionPhysObject* PlayerController::FindClosestPO(float& cur_dy, bool check_angle)
{
	float radius = 0.05f;	
	float cur_dist = 1.75f;
	cur_dy = -0.3f;

	IMissionPhysObject* box = NULL;

	float pl_chr_angle = -1000.0f;
	Vector dir = Vector (0,0,1);
	dir.Rotate(chr.physics->GetAy());
			
			

	dword count = chr.QTFindObjects(MG_DAMAGEACCEPTOR,
									chr.physics->GetPos() - Vector(radius, 0.0f, radius),
									chr.physics->GetPos() + Vector(radius, 0.0f, radius));

	for(dword i = 0; i < count; i++)
	{
		MissionObject& mo = chr.QTGetObject(i)->GetMissionObject();

		MO_IS_IF_NOT(id_IMissionPhysObject, "IMissionPhysObject", &mo)		
		{
			continue;
		}

		IMissionPhysObject* po = (IMissionPhysObject*)&mo;

		if (po->Attack(&chr, IMissionPhysObject::ds_pickup, 0, chr.physics->GetPos(), 3))
		{
			Matrix mat;
			po->GetPartMatrix(0,mat);					

			Vector chr_dir;
			chr_dir = mat.pos - chr.physics->GetPos();	
			chr_dir.Normalize();

			float dt_angle = chr_dir | dir;				
			
			if (check_angle)
			{
				if (dt_angle < 0.3f) continue;			
				if (dt_angle < pl_chr_angle) continue;
			}

			float dist = (chr.physics->GetPos() - mat.pos).GetLength();
			float dy = mat.pos.y - chr.physics->GetPos().y;

			if (dy<-0.3f || dy>1.2f) continue;
	
			if (dy>cur_dy)
			{					
				if (dist<cur_dist)
				{
					if (!check_angle) return po;

					box = po;
					cur_dist = dist;
					cur_dy = dy;
					pl_chr_angle = dt_angle;					
				}
			}
		}
	}

	return box;
}


void PlayerController::GiveMeAllINeed()
{
	AddMoney(1000000);

	WriteGameState(gsMaxBombs,4.0f);
	WriteGameState(gsNumBombs,4.0f);
	WriteGameState(gsHPLive,300.0f);	

	if (gsPlayerAttackA)
	{
		gsPlayerAttackA->SetString("B1","A_Sp3");
		gsPlayerAttackA->SetString("B2","A_Sp3");
		gsPlayerAttackA->SetString("B3","A_Sp3");
	}

	if (gsPlayerAttackB)
	{
		gsPlayerAttackB->SetString("A1","B_Sp3");
		gsPlayerAttackB->SetString("A2","B_Sp3");
		gsPlayerAttackB->SetString("A3","B_Sp3");
		gsPlayerAttackB->SetString("A4","B_Sp3");
		gsPlayerAttackB->SetString("B3","B_Sp3");
	}

	if (gsPlayerShoot)
	{
		WriteGameState(gsPlayerShoot,"triple shot");		
	}

	for (int i=0;i<4;i++) chr.arbiter->SetFatalityAllowe(i,1.0f);	

	WriteGameState(gsPistolTimeReload,7.0f);
	WriteGameState(gsPistolDamage,160.0f);

	AddRage(iHitsToRage);
}

void PlayerController::ActivateTriger(MissionTrigger* triger)
{
	if (triger) triger->Activate(chr.Mission(),false);
}

void PlayerController::SetGameHint()
{
	if (!chr.IsShow()) return;

	WriteGameState(gsGameHint,"none");

	if (qevent)
	{
		WriteGameState(gsGameHint,qevent->GetName());
		return;
	}

	if (itemTaken && phys_object)
	{
		WriteGameState(gsGameHint,"phys object");
		return;
	}

	if (chr.logic->GetState() == CharacterLogic::state_topair)
	{
		WriteGameState(gsGameHint,"topair");
		return;
	}

	int act = chr.logic->GetFightAct();	

	if (act == 1)
	{
		WriteGameState(gsGameHint,"fight");
	}
	else
	if (act == 2)
	{
		WriteGameState(gsGameHint,"fatality");
		return;
	}		
	else
	if (act == 3)
	{
		WriteGameState(gsGameHint,"instant fatality");
		return;
	}
	

	MGIterator* iter = &chr.Mission().GroupIterator(MG_QE, _FL_);

	for (;!iter->IsDone();iter->Next())
	{
		MissionObject* mo = iter->Get();		
				
		if (mo->IsActive())
		{
			Vector dir = chr.physics->GetPos() - mo->GetMatrix(Matrix()).pos;

			if (dir.GetLength2() < 25.0f)
			{
				MO_IS_IF(id_QuckEvent, "QuckEvent", mo)
				{
					QuickEvent* qe = (QuickEvent*)mo;

					const char* name=qe->GetName();

					if (string::IsEqual(name,"Chest"))
					{
						WriteGameState(gsGameHint,"Open Chest");
						iter->Release();
						return;
					}
					else
					if (string::IsEqual(name,"Door"))
					{
						WriteGameState(gsGameHint,"Open Door");
						iter->Release();
						return;
					}
				}
			}
		}
	}

	iter->Release();


	if (!itemTaken)
	{		
		int item_to_take = chr.arbiter->FindClosesetItem(&chr, 3.0f);

		if (item_to_take==-1)
		{
			float cur_dy;
			IMissionPhysObject* box = FindClosestPO(cur_dy,false);

			if (box)
			{
				WriteGameState(gsGameHint,"take phys object");
			}			
		}
		else
		{
			WriteGameState(gsGameHint,"take weapon");
		}
	}
}