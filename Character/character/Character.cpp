
#include "Character.h"
#include "CharacterController.h"
#include "Components\ButtonMG.h"
#include "..\Auxiliary objects\ButtonMiniGame\ButtonMiniGame.h"
#include "Components\CharacterArmor.h"
#include "..\..\Common_h\AnimationBlendLevels.h"
#include "..\..\Common_h\IShooter.h"
#include "Components\CharacterItems.h"
#include "Components\CharacterAnimation.h"
#include "Components\CharacterAnimationEvents.h"
#include "Components\CharacterLogic.h"
#include "Components\CharacterInitAI.h"
#include "Components\CharacterAchievements.h"
#include "..\player\PlayerController.h"
#include "..\..\common_h\IStainManager.h"

#define CHARACTER_DEADTIME	5.0f
#define CHARACTER_SHOWMSGTIME	5.0f
#define CHARACTER_BODYDISAPEAR	2.0f
#define DEF_CAM_FOV tanf(71.619f*PI/180.0f*0.5f)

INTERFACE_FUNCTION

Character::CommandsTable Character::commandsTable;

bool Character::g_bFreezeRagdolls = true;

void Character::CommandRecord::Init(const char * sid, CharCommand cmd, CommandRecord ** entry, dword mask)
{
	id.Set(sid);
	command = cmd;
	next = null;
	CommandRecord ** cr = &entry[id.Hash() & mask];
	while(*cr) cr = &((*cr)->next);
	*cr = this;
}

Character::CommandsTable::CommandsTable()
{
	for(dword i = 0; i < ARRSIZE(entryTable); i++)
	{
		entryTable[i] = null;
	}
	const dword mask = ARRSIZE(entryTable) - 1;
	table[0].Init("enableactor", &Character::Cmd_enableactor, entryTable, mask);
	table[1].Init("disableactor", &Character::Cmd_disableactor, entryTable, mask);
	table[2].Init("actorset", &Character::Cmd_actorset, entryTable, mask);
	table[3].Init("goto", &Character::Cmd_goto, entryTable, mask);
	table[4].Init("additem", &Character::Cmd_additem, entryTable, mask);
	table[5].Init("delitem", &Character::Cmd_delitem, entryTable, mask);
	table[6].Init("sethp", &Character::Cmd_sethp, entryTable, mask);
	table[7].Init("sethpLimit", &Character::Cmd_sethpLimit, entryTable, mask);
	table[8].Init("changehp", &Character::Cmd_changehp, entryTable, mask);
	table[9].Init("teleport", &Character::Cmd_teleport, entryTable, mask);
	table[10].Init("gizmorespawn", &Character::Cmd_gizmorespawn, entryTable, mask);
	table[11].Init("respawn", &Character::Cmd_respawn, entryTable, mask);
	table[12].Init("show", &Character::Cmd_show, entryTable, mask);
	table[13].Init("hide", &Character::Cmd_hide, entryTable, mask);
	table[14].Init("activate", &Character::Cmd_activate, entryTable, mask);
	table[15].Init("deactivate", &Character::Cmd_deactivate, entryTable, mask);
	table[16].Init("instant kill", &Character::Cmd_instant_kill, entryTable, mask);
	table[17].Init("EnableActorColider", &Character::Cmd_EnableActorColider, entryTable, mask);
	table[18].Init("DisableActorColider", &Character::Cmd_DisableActorColider, entryTable, mask);
	table[19].Init("EnableAniBlend", &Character::Cmd_EnableAniBlend, entryTable, mask);
	table[20].Init("DisableAniBlend", &Character::Cmd_DisableAniBlend, entryTable, mask);
	table[21].Init("setwaypoint", &Character::Cmd_setwaypoint, entryTable, mask);
	table[22].Init("setFParams", &Character::Cmd_setFParams, entryTable, mask);
	table[23].Init("settarget", &Character::Cmd_settarget, entryTable, mask);
	table[24].Init("setbombtarget", &Character::Cmd_setbombtarget, entryTable, mask);
	table[25].Init("movelocator", &Character::Cmd_movelocator, entryTable, mask);
	table[26].Init("drawon", &Character::Cmd_drawon, entryTable, mask);
	table[27].Init("drawoff", &Character::Cmd_drawoff, entryTable, mask);
	table[28].Init("show_weapon", &Character::Cmd_show_weapon, entryTable, mask);
	table[29].Init("grab weapon", &Character::Cmd_grab_weapon, entryTable, mask);
	table[30].Init("take weapon", &Character::Cmd_take_weapon, entryTable, mask);
	table[31].Init("ragdoll", &Character::Cmd_ragdoll, entryTable, mask);
	table[32].Init("pair_minigame", &Character::Cmd_pair_minigame, entryTable, mask);
	table[33].Init("pair_minigame_add", &Character::Cmd_pair_minigame_add, entryTable, mask);
	table[34].Init("collapse", &Character::Cmd_collapse, entryTable, mask);
	table[35].Init("actor_attach", &Character::Cmd_actor_attach, entryTable, mask);
	table[36].Init("actor_deattach", &Character::Cmd_actor_deattach, entryTable, mask);
	table[37].Init("hairenable", &Character::Cmd_hairenable, entryTable, mask);
	table[38].Init("legsenable", &Character::Cmd_legsenable, entryTable, mask);
	table[39].Init("UseRootBone", &Character::Cmd_UseRootBone, entryTable, mask);
	table[40].Init("controller", &Character::Cmd_controller, entryTable, mask);
	table[41].Init("Achievement", &Character::Cmd_Achievement, entryTable, mask);
	table[42].Init("ResetAI", &Character::Cmd_ResetAI, entryTable, mask);
	table[43].Init("RedirectSoundEvent", &Character::Cmd_RedirectSoundEvent, entryTable, mask);
	table[44].Init("GodMode", &Character::Cmd_GodMode, entryTable, mask);
	table[45].Init("postdraw", &Character::Cmd_postdraw, entryTable, mask);
	table[46].Init("damageweapon", &Character::Cmd_damageweapon, entryTable, mask);
	table[47].Init("dropweapon", &Character::Cmd_dropweapon, entryTable, mask);
}

bool Character::CommandsTable::ExecuteCommand(Character * chr, const char * id, dword numParams, const char ** params)
{
	if(string::IsEmpty(id)) return false;
	ConstString strId(id);
	static const dword mask = ARRSIZE(entryTable) - 1;
	Character::CommandRecord * rec = entryTable[strId.Hash() & mask];
	for(; rec; rec = rec->next)
	{
		if(rec->id == strId)
		{
			(chr->*(rec->command))(id, numParams, params);
			return true;
		}		
	}
	return false;
}


Character::Character() : 
	BodyParts(_FL_, 1)
#ifndef NO_CONSOLE
	,debugMessages(_FL_)
#endif
{
	isRealFatalityStarted = false;
	killerId.Empty();

	isMoneyFatality = false;

	statDismembered = false;

	fBloodDamage = 0.0f;
	fBloodDropDamage = 20.0f;

	fRotDlt = 0.0f;
	fCoinTime = 0.0f;
	showProcess = false;
	activeProcess = false;
	fTimeInFatality = 0.0f;
	actorData.chr = this;
	bWaterSpalshBorned = false;
	arbiter = null;
	logic = null;
	items = null;
	physics = null;	
	ragdoll = null;
	controller = null;
	animation = null;
	showCollider = false;
	events = null;
	currentModel = null;
	isHitLight = false;
	hitLight = 0.0f;
	deadTime = -1.0f;
	bombTime = -1.0f;
	ccp = null;
	actorTimeLine = null;
	isPlayer = false;

	fCointAltetude = 2.0f;

	oneFrameHide = false;

	fCoindDltHgt = 0.0f;

	LastCharCoin = chrcoin_none;
	CharCoin = chrcoin_none;

	//rdData = null;

	bColideWhenActor = false;

	bEnableAniBlend = false;

	fTimeToStandUp = 0.0f;
	fTimeToFreeze = 2.0f;

	fTotalTimeToStandUp = 0.0f;

	bDeadConterOn = false;
	deadBodyTime = 0.0f;

	bAutoReleaseRagdollOn = false;
	fTimeToReleaseRagDoll = 0.0f;

	bShotImmuneWhenRoll = false;

	debugInfoType = -1;	

	bShowCoinED = false;

	fTimeInFatality = FATALITY_TIME;

	fHitDmgMultiplayer = 1.0f;
	fBombDmgMultiplayer = 1.0f;
	fShootDmgMultiplayer = 1.0f;
	fRageDmgMultiplayer = 1.0f;

	waterLevel.Reset();

	bombSpeed = 7.0f;

	buttonMG = NULL;

	bDraw = true;

	pattern = null;
	pattern_ptr.Reset();

	animation = null;

	FatalityParamsData[0] = 0;

	BltTrace.fTime = 0.0f;
	BltTrace.fSpeed = 35.0f;
	BltTrace.Show  = true;

	BltTrace.scaleV = 0.05f;
	BltTrace.scaleH = 1.5f;

	BltTrace.BulletStart = Vector(0,0,0);
	BltTrace.BulletEnd = Vector(0,0,10);	

	add_color.r = add_color.g = add_color.b = 0.0f;

	bSelfDestructRagdoll = false;

	hpBar = null;

	bVanishBody = false;

	flyCloud = null;

	bActorMoveColider = false;	

	for (int i=0;i<20;i++)
	{
		pair_minigame[i] = null;
	}
	

	step_effectTable = null;
	shoot_effectTable = null;
	
	chrEffect = null;
	chrAchievements = null;
	chrInitAI = null; 
	mg_params = null;
	common_mg_params = null;

	actorData.ani = null;

	debugInfoType = -1;

	bodyparts_inited = false;

	useBombPhysic = true;

	bomb.dltTime = 0;
	bomb.isSleep = true;
	bomb.pos = 0.0f;
	bomb.radius = 0.1f;
	bomb.velocity = 0.0f;

	bShadowGrRegistred = false;
	bReflectGrRegistred = false;
	bRefractGrRegistred = false;

	seaReflection = false;
	seaRefraction = false;

	init_actrorPreview = false;

	auto_spawnpoint = true;
	init_spawnpoint = 0.0f;

	actor_attach = null;

	armor = null;

	useRootBone = false;


	set_new_ay = false;
	new_ay = 0.0f;

	attackSpeedModifier = 1.0f;

	need_post_grab_weapon = false;
	pgw_wp_index = -1;
	pgw_start_node = "";
	pgw_blend_time = 0.2f;
	pgw_victim = null;


	fDistShadow = 0.0f;

	graph_changed = false;
	act_node = -1;
	was_showed = false;

	need_cntl_shoot_call = true;

	greedy = null;

#ifndef NO_CONSOLE
	debugMessageShowTime = 0.0f;
#endif

#ifdef CHARACTER_STARFORCE
	scaleSpike = RRnd(0.999f, 1.001f);
	scaleSpikeTime = RRnd(120.0f, 170.0f);
	curScaleSpike = 1.0f;
#endif
}

Character::~Character()
{
	Release();
}

//============================================================================================

//Инициализировать объект
bool Character::Create(MOPReader & reader)
{
	Registry(GroupId('C','h','G','R'));

	bDraw = true;

	if(!DamageReceiver::Create(reader))
	{
		LogicDebugError("Object not created");
		return false;
	}
	if(!Init(reader)) return false;
	
	if (arbiter->IsCharacterDebug())
	{
		if (!shadowModel.scene && pattern)
		{		
			api->Trace ("Shadow Model for %s not set ",GetObjectID().c_str());
		}
	}

	return true;
}

//Вызываеться, когда все объекты созданны но ещё не началось исполнение миссии
void Character::PostCreate()
{
	if (chrAchievements)
		chrAchievements->PostCreate();

	if (FindObject(bombExplosionPattern, bombExplosionMO))
	{
		static const ConstString bombExplosionPatternId = ConstString("BombExplosionPattern");
		if (!bombExplosionMO.Ptr()->Is(bombExplosionPatternId))
			bombExplosionMO.Reset();
	}
}

// возвращает null если нету, или указательн на MissionObject
MissionObject * Character::GetBombExplosionPattern()
{
	if (bombExplosionMO.Validate())
		return bombExplosionMO.Ptr();

	if (pattern && pattern->bombExplosionMO.Validate())
		return pattern->bombExplosionMO.Ptr();

	return null;
}

float Character::GetDeathTime()
{
	float k = 1.0f;

	if (isPlayer)
	{
		k = 2.0f;
	}

	return CHARACTER_DEADTIME * k;
}

void Character::Kill()
{
	//animation->Goto("Idle",0.0f);
	logic->SetHP(0.0f,false);
	//logic->Update(0.0f);
	if(controller)
	{
		controller->Death();
	}
	
	bWaterSpalshBorned = false;

	if (ragdoll)
	{		
		//ActivateRagdoll(false, 0.0f);
		ragdoll->Release();
		ragdoll = NULL;

		bAutoReleaseRagdollOn = false;
		fTimeToReleaseRagDoll = 0.0f;
	}

	for (dword i=0;i<BodyParts.Size();i++)
	{
		if (BodyParts[i].pPhysBox)
		{
			BodyParts[i].pPhysBox->Release();
			BodyParts[i].pPhysBox=NULL;
		}

		if (BodyParts[i].pPhysCapsule)
		{
			BodyParts[i].pPhysCapsule->Release();
			BodyParts[i].pPhysCapsule=NULL;
		}

		if (BodyParts[i].pParticale)
		{
			BodyParts[i].pParticale->Release();
			BodyParts[i].pParticale=NULL;
		}
	}

	deadTime = GetDeathTime();

	if (bDeadConterOn && !bVanishBody)
		arbiter->DelDeadBody(*this);

	bDeadConterOn = false;
	DelUpdate(&Character::DeadTimer);
	
	set_new_ay = false;
	//Activate(IsActive());
	//Show(IsShow());


//	Unregistry(MG_DOF);
	Unregistry(MG_SHADOWCAST);
	Unregistry(MG_SHADOWRECEIVE);
	Unregistry(MG_SEAREFLECTION);
	Unregistry(MG_SEAREFRACTION);
	Unregistry(MG_SHADOWDONTRECEIVE);

	DelUpdate(&Character::DrawTrail);
	DelUpdate(&Character::DrawFlares);
	DelUpdate(&Character::Draw);
	DelUpdate(&Character::DrawCoin);


	if (controller)
	{
		controller->Reset();
		controller->SetChrTarget(NULL);
	}


	DelUpdate(&Character::Work);
	DelUpdate(&Character::UpdatePhysics);

	if(!logic->IsActor())
	{		
		physics->Activate(false);
		//arbiter->Activate(this, false);
	}
	//else
	//{
	//	physics->Activate(bColideWhenActor);
		//arbiter->Activate(this, isActive);
	//}

}

void _cdecl Character::Cmd_enableactor(const char * id, dword numParams, const char ** params)
{
	DisableTimeEvent();

	if(logic->IsDead())
	{
		LogicDebugError("Command <enableactor>. Skip command, character is dead.");
		return;
	}
	bool isActor = logic->IsActor();		

	float blend_time = -1.0f;

	if(bEnableAniBlend && !isActor)
	{
		blend_time = 0.3f;
	}		

	if (was_showed)
	{
		Show(false);
	}

	graph_changed = true;

	if(controller)
	{
		controller->Reset();
	}	

	if (IsPlayer())
	{
		PlayerController * pc = (PlayerController*)controller;
		pc->EndQuickEvent();
		if (pc->cur_weapon_style)
		{
			static const ConstString gunId("2Gun");
			if (pc->cur_weapon_style->weapon_type == wp_gun) items->SetLogicLocator(gunId, ConstString::EmptyObject());
		}

		if (pc->vanoHackDropPhysObjectWhenActor)
		{
			pc->BrokeWeapon(false);
		}
	}

	animation->SetAnimation("actor",blend_time);

	if (logic->IsPairMode())
	{
		logic->EndPairProcess();
	}		
	logic->SetActor(true);		

	if(IsPlayer())
	{
		PlayerController * pc = (PlayerController*)controller;
		MissionObject* mo = pc->phys_object;		

		if (mo)
		{
			mo->Show(false);
		}
	}	

	bodyparts.SetRotation(0,true);
	bodyparts.SetRotation(0,false);

	ResetActor();
	//UpdateActor(0);		

	LogicDebug("Command <enableactor>. Character now is actor.");		

	//ApplyAnimationQuery(0.1f);
}

void _cdecl Character::Cmd_disableactor(const char * id, dword numParams, const char ** params)
{
	if (bActorMoveColider && logic->IsActor())
	{
		Vector pos;
		GetPosFromRootBone(pos);
		physics->SetPos(pos);
	}

	bool isActor = logic->IsActor();
	bool isStartDead = logic->IsDead();

	float blend_time = -1.0f;

	if(bEnableAniBlend && isActor)
	{
		blend_time = 0.3f;
	}

	bool animStateAlive = logic->GetState() != CharacterLogic::state_die && logic->GetState() != CharacterLogic::state_dead;

	if (animStateAlive)
	{
		animation->SetAnimation("base", blend_time);
		animation->Goto("idle",0.0f);
	}

	if (was_showed)
	{
		Show(false);
	}

	//graph_changed = true;

	if(deadTime >= 0.0f) isStartDead = false;
	if(isStartDead)
	{
		logic->SetHP(1.0f);
		logic->SetActor(false);
		logic->SetHP(0.0f);
	}
	else
	{
		logic->SetActor(false);
	}		



	if(controller)
	{
		controller->Reset();
		controller->SetChrTarget(NULL);
	}		

	if (ragdoll)
	{
		if (logic->GetHP()>0.0f &&				
			logic->GetState() != CharacterLogic::state_die &&
			logic->GetState() != CharacterLogic::state_dead &&
			!logic->IsPairMode())
		{
			animation->Goto("Lie",0.2f);
		}
		else
		{
			animation->ActivateLink("Lie",true);
		}
	}

	if(IsPlayer())
	{
		PlayerController * pc = (PlayerController*)controller;
		if (pc->vanoHackDropPhysObjectWhenActor)
		{
			pc->DropWeapon(true);
			pc->BrokeWeapon(false);
		}
		pc->RestoreSecodWeapon();			
	}	

	logic->SetSpawnPoint(physics->GetPos());

	LogicDebug("Command <disableactor>. Character now is not actor. AI & Physics activating.");
}

void _cdecl Character::Cmd_actorset(const char * id, dword numParams, const char ** params)
{
	if(numParams < 1)
	{
		LogicDebugError("Command <ActorSet> error. Invalidate parameters...");
		return;
	}

	MOSafePointerTypeEx<ActorTimeLine> mo;
	static const ConstString strTypeId("ActorTimeLine");
	mo.FindObject(&Mission(),ConstString(params[0]),strTypeId);

	if(!mo.Ptr())
	{
		actorTimeLine = null;
		LogicDebugError("Command <ActorSet>. Time line object \"%s\" not found or incorrect", params[0]);
		return;
	}

	actorTimeLine = mo.Ptr();

	ResetActor();
	if(logic->IsActor())
	{
		//UpdateActor(0.0f);
	}

	LogicDebug("Command <ActorSet>. Set new actor's time line \"%s\"", params[0]);
}

void _cdecl Character::Cmd_goto(const char * id, dword numParams, const char ** params)
{
	if(numParams < 1)
	{
		LogicDebugError("Command <Goto> error. Invalidate parameters...");
		return;
	}
	if(!actorTimeLine)
	{
		LogicDebugError("Command <Goto> error. Time line not set...");
		return;
	}
	long pointIndex = atol(params[0]);

	if (graph_changed)
	{
		act_node = pointIndex;
	}
	else
	{			
		actorTimeLine->Goto(actorData, pointIndex);
		UpdateActor(0.0f);
	}

	LogicDebug("Command <Goto>. Current time line position moved to %i", actorData.GetCurrentPosition());
}

void _cdecl Character::Cmd_additem(const char * id, dword numParams, const char ** params)
{
	if(numParams < 3)
	{
		LogicDebugError("Command <additem> error. Invalidate parameters...");
		return;
	}
	bool isTrail = false;
	if(numParams > 3)
	{
		isTrail = string::IsEqual(params[3], "trail");
	}

	CharacterPattern::TItemDescr descr;

	descr.id.Set(params[0]);
	descr.modelName = params[1];
	descr.locatorName.Set(params[2]);
	descr.useTrail = isTrail;
	descr.weapon_type = wp_undropable;
	descr.tip_id.Empty();
	descr.fProbality = 100.0f;
	descr.time_lie = 10.0f;
	descr.itemLife = -1;				
	descr.uniqTexture = numParams > 4 ? params[4] : null;
	descr.trailTechnique = "Blend";
	descr.trailPower = 1.0f;
	descr.showFlare = false;
	descr.locatortoattache = "";
	descr.attachedobject.Empty();
	descr.objectoffset = Vector(0.0f);		

	if(items->AddItem(&descr))
	{
		LogicDebug("Command <additem>. Add item \"%s\" with model \"%s\" to locator \"%s\" %s", params[0], params[1], params[2], isTrail ? "with trail" : "without trail");
	}else{
		LogicDebugError("Command <additem>. Can't add item \"%s\" with model \"%s\" to locator \"%s\"", params[0], params[1], params[2]);
	}
}

void _cdecl Character::Cmd_delitem(const char * id, dword numParams, const char ** params)
{
	if(numParams < 1)
	{
		LogicDebugError("Command <delitem> error. Invalidate parameters...");
		return;
	}
	long count = items->DelItems(ConstString(params[0]));
	LogicDebug("Command <delitem>. Delete items with id = \"%s\", count = %i", params[0], count);
}

void _cdecl Character::Cmd_sethp(const char * id, dword numParams, const char ** params)
{
	if(numParams < 1)
	{
		LogicDebugError("Command <sethp> error. Invalidate parameters...");
		return;
	}
	float count = (float)atof(params[0]);
	logic->SetHP(count);

	if (logic->GetHP()<=0.0f)
	{
		logic->Update(0.0f);

		if(controller)
		{
			controller->Death();
		}

		if (isMoneyFatality)
			ACHIEVEMENT_REACHED(arbiter->GetPlayer(), RACKETEER);
	}
	else
	{
		if(numParams >= 2)
		{
			animation->ActivateLink(params[1]);
		}
	}

	isMoneyFatality = false;

	LogicDebug("Command <sethp>. Set new HP for character by %f, current HP = %f", count, logic->GetHP());
}

void _cdecl Character::Cmd_sethpLimit(const char * id, dword numParams, const char ** params)
{
	if(numParams < 1)
	{
		LogicDebugError("Command <sethpLimit> error. Invalidate parameters...");
		return;
	}
	float count = (float)atof(params[0]);
	logic->SetMinHPLimitter(count * 0.01f);			
	logic->CorrectHP();

	LogicDebug("Command <sethpLimit>. Set new HPLimit to %f", count);
}

void _cdecl Character::Cmd_changehp(const char * id, dword numParams, const char ** params)
{
	if(numParams < 1)
	{
		LogicDebugError("Command <changehp> error. Invalidate parameters...");
		return;
	}
	float count = (float)atof(params[0]);
	logic->SetHP(logic->GetHP() + count);

	if(numParams >= 2 && !logic->IsDead())
	{
		animation->ActivateLink(params[1]);
	}

	LogicDebug("Command <changehp>. Character change HP by %f, current HP = %f", count, logic->GetHP());
}

void _cdecl Character::Cmd_teleport(const char * id, dword numParams, const char ** params)
{
	if(numParams < 1)
	{
		LogicDebugError("Command <teleport> error. Invalidate parameters...");
		return;
	}

	MOSafePointer obj;
	FindObject(ConstString(params[0]),obj);

	if(obj.Ptr())
	{
		Matrix mtx(true);
		obj.Ptr()->GetMatrix(mtx);
		physics->SetPos(mtx.pos);
		physics->SetAy(mtx.vz.GetAY(physics->GetAy()));

		LogicDebug("Command <teleport>. Character teleported to \"%s\" (%4.3f,%4.3f,%4.3f)", params[0], mtx.pos.x, mtx.pos.y, mtx.pos.z);
	}
	else
	{
		LogicDebugError("Command <teleport> error. Can't find object <%s>.",params[0]);
	}
}

void _cdecl Character::Cmd_gizmorespawn(const char * id, dword numParams, const char ** params)
{
	if(numParams < 1)
	{
		LogicDebugError("Command <gizmorespawn> error. Invalidate parameters...");
		return;
	}

	MOSafePointer obj;
	FindObject(ConstString(params[0]),obj);

	if(obj.Ptr())
	{
		char* resawnparams[1];

		resawnparams[0] = (char*)GetObjectID().c_str();

		obj.Ptr()->Command("respawn",1,(const char **)resawnparams);			

		LogicDebug("Command <gizmorespawn>. Character resawned at \"%s\"", params[0]);
	}
	else
	{
		LogicDebugError("Command <gizmorespawn> error. Can't find object <%s>.",params[0]);
	}
}

void _cdecl Character::Cmd_respawn(const char * id, dword numParams, const char ** params)
{
	//if (logic->GetHP()<=0.0f && !IsDead()) return;
	killerId.Empty();

	Kill();
	statDismembered = false;
	logic->deathPass = false;		
	fTimeToFreeze = 2.0f;
	deadTime = -1.0f;
	deadBodyTime = 0.0f;
	if (bDeadConterOn && !bVanishBody) 
	{
		arbiter->DelDeadBody(*this);
	}
	bDeadConterOn = false;
	DelUpdate(&Character::DeadTimer);
	logic->Reset();
	physics->Reset();
	controller->Reset();
	controller->SetChrTarget(null);		
	items->Reset();
	events->ResetCollapser();
	events->ResetParticles();
	events->ResetSounds();
	currentModel = model.scene;

	ReleaseFlyes();

	if(animation)
	{
		animation->Start(true);
	}

	if(numParams == 0)
	{
		//physics->SetPos(init_pos);
		//physics->SetAy(init_ay);
	}

	if(actorData.ani)
	{
		actorData.ani->Start(null, true);
	}		

	if(numParams >= 1)
	{
		MOSafePointer obj;
		FindObject(ConstString(params[0]),obj);

		if(obj.Ptr())
		{
			Matrix mtx(true);
			obj.Ptr()->GetMatrix(mtx);
			physics->SetPos(mtx.pos);
			physics->SetAy(mtx.vz.GetAY(physics->GetAy()));

			logic->SetSpawnPoint(mtx.pos);
		}
	}
	Activate(true);
	Show(true);

	LogicDebug("Command <respawn>. Character is respawn now.");

	if(numParams > 1)
	{			
		MOSafePointer obj;
		FindObject(ConstString(params[0]),obj);

		if(obj.Ptr())
		{
			Matrix mtx(true);
			obj.Ptr()->GetMatrix(mtx);
			physics->SetPos(mtx.pos);
			physics->SetAy(mtx.vz.GetAY(physics->GetAy()));	

			LogicDebug("                  respawned at %s",params[0]);
		}
	}

	oneFrameHide = true;
	SetUpdate(&Character::OneFrameHide, ML_SCENE_END);
}

void _cdecl Character::OneFrameHide(float dltTime, long level)
{
	DelUpdate(&Character::OneFrameHide);
	oneFrameHide = false;
}

void _cdecl Character::Cmd_show(const char * id, dword numParams, const char ** params)
{
	Show(true);
	LogicDebug("Command <show>. Character was showed");
}

void _cdecl Character::Cmd_hide(const char * id, dword numParams, const char ** params)
{
	Show(false);
	LogicDebug("Command <hide>. Character was hided");
}

void _cdecl Character::Cmd_activate(const char * id, dword numParams, const char ** params)
{
	Activate(true);
	LogicDebug("Command <activate>. Character was activated");
}

void _cdecl Character::Cmd_deactivate(const char * id, dword numParams, const char ** params)
{
	Activate(false);
	LogicDebug("Command <deactivate>. Character was deactivated");
}

void _cdecl Character::Cmd_instant_kill(const char * id, dword numParams, const char ** params)
{
	Kill();

	LogicDebug("Command <instant kill>. Character was killed");
}

void _cdecl Character::Cmd_EnableActorColider(const char * id, dword numParams, const char ** params)
{
	bColideWhenActor = true;

	Activate(IsActive());

	LogicDebug("Command <DisableActorColider>. ActorColider was enabled");
}

void _cdecl Character::Cmd_DisableActorColider(const char * id, dword numParams, const char ** params)
{
	bColideWhenActor = false;

	Activate(IsActive());

	LogicDebug("Command <DisableActorColider>. ActorColider was disabled");
}

void _cdecl Character::Cmd_EnableAniBlend(const char * id, dword numParams, const char ** params)
{
	if(animation && actorData.ani && !EditMode_IsOn())
	{
		bEnableAniBlend = true;
		LogicDebug("Command <EnableAniBlend>. AniBlend was enabled");
	}
}

void _cdecl Character::Cmd_DisableAniBlend(const char * id, dword numParams, const char ** params)
{
	if(animation && actorData.ani && !EditMode_IsOn())
	{
		bEnableAniBlend = false;
		LogicDebug("Command <DisableAniBlend>. AniBlend was disabled");
	}
}

void _cdecl Character::Cmd_setwaypoint(const char * id, dword numParams, const char ** params)
{
	if(numParams < 1)
	{
		LogicDebugError("Command <setwaypoint> error. Invalidate parameters...");
		return;
	}

	if (controller)
	{
		controller->SetWayPointsName(ConstString(params[0]));
	}

	LogicDebug("Command <setwaypoint>. Waypoints was changed to \"%s\"", params[0]);
}

void _cdecl Character::Cmd_setFParams(const char * id, dword numParams, const char ** params)
{
	if(numParams < 1)
	{
		LogicDebugError("Command <setFParams> error. Invalidate parameters...");
		return;
	}

	crt_strncpy(FatalityParamsData,sizeof(FatalityParamsData),params[0], sizeof(FatalityParamsData));
	FatalityParamsData[sizeof(FatalityParamsData) - 1] = 0;
	FatalityParamsId.Set(FatalityParamsData);
	logic->FParams.Reset();
	logic->UpdateFatalityParams();

	LogicDebug("Command <setFParams>. Fatalaty Params was changed to \"%s\"", params[0]);
}

void _cdecl Character::Cmd_settarget(const char * id, dword numParams, const char ** params)
{
	if(numParams < 1)
	{
		LogicDebugError("Command <settarget> error. Invalidate parameters...");
		return;
	}

	if (controller)
	{
		controller->SetTarget(ConstString(params[0]));
	}

	LogicDebug("Command <settarget>. Waypoints chabged to \"%s\"", params[0]);
}

void _cdecl Character::Cmd_setbombtarget(const char * id, dword numParams, const char ** params)
{
	if(numParams < 1)
	{
		LogicDebugError("Command <setbombtarget> error. Invalidate parameters...");
		return;
	}

	MOSafePointer obj;
	FindObject(ConstString(params[0]),obj);

	if(obj.Ptr())
	{
		Matrix mtx(true);
		obj.Ptr()->GetMatrix(mtx);
		SetBombTarget(mtx.pos);

		LogicDebug("Command <setbombtarget>. Bomb Target set as %s", params[0]);
	}
	else
	{
		LogicDebugError("Command <setbombtarget> error. Can't find object <%s>.",params[0]);
	}
}

void _cdecl Character::Cmd_movelocator(const char * id, dword numParams, const char ** params)
{
	if(numParams >= 2)
	{
		items->SetLogicLocator(ConstString(params[0]), ConstString(params[1]));

		LogicDebug("Command <movelocator>. Logic locator %s moved to %s", params[0], params[1]);
	}
	else
	{
		LogicDebugError("Command <movelocator>. Invalidate parameters...");
	}
}

void _cdecl Character::Cmd_drawon(const char * id, dword numParams, const char ** params)
{
	bDraw = true;

	LogicDebug("Command <drawon>. Character Draw was enabled.");
}

void _cdecl Character::Cmd_drawoff(const char * id, dword numParams, const char ** params)
{
	bDraw = false;

	LogicDebug("Command <drawoff>. Character Draw was disabled.");
}

void _cdecl Character::Cmd_show_weapon(const char * id, dword numParams, const char ** params)
{
	if(numParams >= 2)
	{
		bool show = false;
		if (params[0][0]=='Y'||params[0][0]=='y')
		{
			show = true;
			LogicDebug("Command <show_weapon>. Weapon %s was showed...",params[1]);
		}
		else
		{
			LogicDebug("Command <show_weapon>. Weapon %s was hided...",params[1]);
		}

		items->ShowWeapon(show,ConstString(params[1]));


	}
	else
	{
		LogicDebug("Command <show_weapon>. Invalidate parameters...");
	}
}

void _cdecl Character::Cmd_grab_weapon(const char * id, dword numParams, const char ** params)
{
	if(numParams >= 1 && IsPlayer())
	{			
		pgw_start_node = null;

		if(numParams >= 2)
		{
			pgw_start_node = params[1];
		}

		pgw_blend_time = 0.2f;

		pgw_victim = null;

		if (numParams >= 3)
		{
			pgw_blend_time = (float)strtod(params[2], null);	
		}

		if (logic->pair)
		{
			pgw_victim = logic->pair;
		}
		else
			if(numParams >= 4)
			{				
				MOSafePointerTypeEx<Character> trgt;
				static const ConstString strTypeId("Character");
				trgt.FindObject(&Mission(),ConstString(params[3]),strTypeId);

				pgw_victim = trgt.Ptr();				
			}

			if (pgw_victim)
			{
				int index = pgw_victim->items->FindItemIndex(ConstString(params[0]));

				if (index !=-1)
				{	
					pgw_wp_index = index;
					need_post_grab_weapon = true;					
					LogicDebug("Command <grab weapon>. Weapon with ID %s was taken.", params[0]);
				}else{
					LogicDebugError("Command <grab weapon> skip. Weapon with ID %s not found.", params[0]);
				}
			}
			else
			{
				if (string::IsEqual(params[3],"self"))
				{
					LogicDebugError("Command <grab weapon>. Self mode.");
					((PlayerController*)controller)->TakeChachedWeapon(ConstString(params[0]), pgw_blend_time,params[1]);

				}else{
					LogicDebugError("Command <grab weapon> skip. No victim, no self.");
				}
			}
	}
	else
	{
		LogicDebugError("Command <grab weapon>. Invalidate parameters...");
	}
}

void _cdecl Character::Cmd_take_weapon(const char * id, dword numParams, const char ** params)
{
	if(numParams >= 4 && IsPlayer())
	{
		const char* start_node = null;

		if(numParams >= 3)
		{
			start_node = params[2];
		}

		const char* broken_part1 = null;
		const char* broken_part2 = null;

		if(numParams >= 5)
		{
			broken_part1 = params[3];
			broken_part2 = params[4];
		}

		((PlayerController*)controller)->TakeWeapon(GetWeaponType(params[0]), params[1],
			broken_part1,broken_part2,ConstString(),0.2f,start_node);

		LogicDebug("Command <take weapon>. Weapon %s with model %s was taken.", params[0], params[1]);
	}
	else
	{
		LogicDebugError("Command <take weapon>. Invalidate parameters...");
	}
}

void _cdecl Character::Cmd_ragdoll(const char * id, dword numParams, const char ** params)
{
	ActivateRagdoll(true,0.2f);
	LogicDebugError("Command <ragdoll>. Ragdoll was activated.");
}

void _cdecl Character::Cmd_pair_minigame(const char * id, dword numParams, const char ** params)
{
	for (dword i=0;i<20;i++)
	{
		if (i<numParams)
		{
			MOSafePointerTypeEx<Character> trgt;
			static const ConstString strTypeId("Character");
			trgt.FindObject(&Mission(),ConstString(params[i]),strTypeId);

			pair_minigame[i] = trgt.Ptr();				
			//pair_minigame[i]->events->SetTimeEvent(false);
		}
		else
		{
			pair_minigame[i] = null;
		}			
	}

	DisableTimeEvent();
	LogicDebugError("Command <pair_minigame>. Pairs was set.");
}

void _cdecl Character::Cmd_pair_minigame_add(const char * id, dword numParams, const char ** params)
{
	if (numParams>0)
	{
		dword index = 0;

		for (dword i=0;i<20;i++)		
		{
			if (!pair_minigame[i])
			{
				MOSafePointerTypeEx<Character> trgt;
				static const ConstString strTypeId("Character");
				trgt.FindObject(&Mission(),ConstString(params[index]),strTypeId);

				if (trgt.Ptr())
				{						
					pair_minigame[i] = trgt.Ptr();
					index++;

					if (index>=numParams)
					{
						break;
					}
				}					
			}								
		}
	}

	DisableTimeEvent();
	LogicDebugError("Command <pair_minigame_add>. Pairs was set.");
}

void _cdecl Character::Cmd_collapse(const char * id, dword numParams, const char ** params)
{
	events->BoneCollapse(animation->GetCurAnimation(),"collapse",params,numParams);
	LogicDebugError("Command <collapse>. Pairs was set.");
}

void _cdecl Character::Cmd_actor_attach(const char * id, dword numParams, const char ** params)
{
	if(numParams >= 1)
	{
		MOSafePointer obj;
		FindObject(ConstString(params[0]),obj);

		actor_attach = obj.Ptr();

		if (actor_attach)
		{
			LogicDebug("Command <actor_attach>. Character %s was attached to %s.", GetObjectID().c_str(), params[0]);
		}
		else
		{
			LogicDebug("Command <actor_attach>. Character %s was attached to %s.", GetObjectID().c_str(), params[0]);
		}
	}
	else
	{
		LogicDebugError("Command <actor_attach>. Character %s was deattached.", GetObjectID().c_str());
	}
}

void _cdecl Character::Cmd_actor_deattach(const char * id, dword numParams, const char ** params)
{
	actor_attach = null;
	LogicDebugError("Command <actor_deattach>. Charater was deattached...");
}

void _cdecl Character::Cmd_hairenable(const char * id, dword numParams, const char ** params)
{
	if(numParams >= 1)
	{
		bool enable = false;

		if (params[0][0] == 'Y' || params[0][0] == 'y')
		{
			enable = true;
			LogicDebug("Command <hairenable>. Hairblender was enabled.");
		}
		else
		{
			LogicDebug("Command <hairenable>. Hairblender was disebled.");
		}

		bodyparts.ActivateHair(enable);

		return;
	}
}

void _cdecl Character::Cmd_legsenable(const char * id, dword numParams, const char ** params)
{
	if(numParams >= 1)
	{
		bool enable = false;

		if (params[0][0] == 'Y' || params[0][0] == 'y')
		{
			enable = true;
			LogicDebug("Command <legsenable>. Hairblender was enabled.");
		}
		else
		{
			LogicDebug("Command <legsenable>. Hairblender was disebled.");
		}

		bodyparts.ActivateLegs(enable);

		return;
	}
}

void _cdecl Character::Cmd_UseRootBone(const char * id, dword numParams, const char ** params)
{
	if(numParams >= 1)
	{
		if (params[0][0] == 'Y' || params[0][0] == 'y')
		{
			useRootBone = true;
			LogicDebug("Command <UseRootBone>. Character use root bone.");
		}
		else
		{
			useRootBone = false;
			LogicDebug("Command <UseRootBone>. Character dont use root bone.");			
		}			
	}
}

void _cdecl Character::Cmd_controller(const char * id, dword numParams, const char ** params)
{
	if (controller)
	{
		controller->Command(numParams, params);
		LogicDebug("Command <controller>. Command to %s controller was send.",params[0]);
	}
}

void _cdecl Character::Cmd_Achievement(const char * id, dword numParams, const char ** params)
{
	if(numParams >= 1)
	{
		if (chrAchievements)
			chrAchievements->AchievementReachedByID(params[0]);			
		LogicDebug("Command <Achievement>. Some conditions of achievement %s was satisfied.",params[0]);
	}
}

void _cdecl Character::Cmd_ResetAI(const char * id, dword numParams, const char ** params)
{
	if (controller)
	{
		controller->Reset();
		controller->SetChrTarget(NULL);
	}
}

void _cdecl Character::Cmd_RedirectSoundEvent(const char * id, dword numParams, const char ** params)
{
	if(events)
	{			
		events->PlaySound(animation->GetCurAnimation(), "Snd", params, numParams);
	}
}

void _cdecl Character::Cmd_dropweapon(const char * id, dword numParams, const char ** params)
{
	if (IsPlayer())
	{
		PlayerController * pc = (PlayerController*)controller;
		pc->DropWeapon(false);
		pc->BrokeWeapon(true);
		if (numParams && params[0] && params[0][0])
			animation->ActivateLink(params[0], true);
	}
}

void _cdecl Character::Cmd_damageweapon(const char * id, dword numParams, const char ** params)
{
	float damage = (numParams >= 1) ? (float)atof(params[0]) : 1.0f;
	if (items)
		items->DamageWeapon(false, damage);
}

void _cdecl Character::Cmd_postdraw(const char * id, dword numParams, const char ** params)
{
	Draw(0.0f, 0);
}

void _cdecl Character::Cmd_GodMode(const char * id, dword numParams, const char ** params)
{
	if (IsPlayer())
	{
		arbiter->SwitchGodMode();
	}
}


//Обработчик команд для объекта
void Character::Command(const char * id, dword numParams, const char ** params)
{	
	if(!commandsTable.ExecuteCommand(this, id, numParams, params))
	{
		LogicDebugError("Unknown command \"%s\".", id);
	}
}

//Инициализировать объект
bool Character::EditMode_Create(MOPReader & reader)
{
	if(!DamageReceiver::Create(reader))
	{
		LogicDebugError("Object not created");
		return false;
	}
	EditMode_Update(reader);

	if (arbiter->IsCharacterDebug())
	{
		if (!shadowModel.scene  && pattern)
		{		
			api->Trace ("Shadow Model for %s not set ",GetObjectID().c_str());
		}
	}

	SetUpdate(&Character::EditMode_Work,ML_EXECUTE1);

	return true;
}

//Обновить параметры
bool Character::EditMode_Update(MOPReader & reader)
{
	Release();
	
	if(!Init(reader)) return false;

	return true;
}

//Получить бокс, описывающий объект
void Character::GetBox(Vector & min, Vector & max)
{
	IGMXScene* mdl = shadowModel.scene;

	if (!mdl) mdl = currentModel;

	if(mdl)
	{		
		min = mdl->GetLocalBound().vMin;
		max = mdl->GetLocalBound().vMax;

		if (useRootBone)
		{
			Matrix mtx;
			physics->GetModelMatrix(mtx);

			Matrix mtx2;
			physics->GetMatrixWithBoneOffset(mtx2);
			mtx2.Inverse();

			// Vano: тут поменял немного, хотя все равно неправильно работает.
			Vector center = ((min + max) * 0.5f);
			center = center * mtx * mtx2;
			Vector extents = ((max - min) * 0.5f) + 0.2f;	// магическое 0.2f

			/*min = min * mtx * mtx2;
			max = max * mtx * mtx2;*/

			min = center - extents;
			max = center + extents;
		}
		
		return;
	}
	else
	{
		min = 0.01f;
		max = 0.01f;
	}
}

//Получить размеры описывающего ящика
void Character::EditMode_GetSelectBox(Vector & min, Vector & max)
{
	if(physics)
	{
		float r = physics->GetRadius();
		float h = physics->GetHeight();
		
		min = Vector(-r, 0.0f, -r) - 0.1f;
		max = Vector(r, h, r) + 0.1f;		
	}
	else
	{
		min = Vector(-0.8f, -0.2f, -0.8f);
		max = Vector(0.8f, 2.0f, 0.8f);
	}
}

#ifndef MIS_STOP_EDIT_FUNCS
//Выделить объект
void Character::EditMode_Select(bool isSelect)
{
	MissionObject::EditMode_Select(isSelect);
	
	if(!EditMode_IsOn()) return;

	if(isSelect)
	{		
		SetUpdate(&Character::EditModeDrawSelected, ML_ALPHA5);
	}
	else
	{
		DelUpdate(&Character::EditModeDrawSelected);
	}
}
#endif

//Отрисовка в выделеном режиме
void _cdecl Character::EditModeDrawSelected(float dltTime, long level)
{
	if (!auto_spawnpoint)
	{
		Render().DrawSphere(init_spawnpoint,0.25f,0xff00ffff);
	}

	if (chrInitAI)
	{		
		for (int i=0;i<(int)chrInitAI->patrol_points.Size();i++)
		{
			Vector p1 = chrInitAI->patrol_points[i].pos;
			Vector p2;

			if (i==(int)chrInitAI->patrol_points.Size()-1)
			{
				p2 = chrInitAI->patrol_points[0].pos;
			}
			else
			{
				p2 = chrInitAI->patrol_points[i+1].pos;
			}

			Render().DrawLine(p1,0xff00ffff,p2,0xff00ffff);			
			Render().DrawSphere(p1,0.25f,0xff00ffff);

			if (chrInitAI->patrol_points[i].wait_time>0.0f)
			{
				Render().DrawVector(chrInitAI->patrol_points[i].pos,
									chrInitAI->patrol_points[i].pos + chrInitAI->patrol_points[i].dir,0xff00ffff);
				Render().Print(chrInitAI->patrol_points[i].pos,10.0f,0,0xffffffff,"Node - %s, time - %4.1f",
							   chrInitAI->patrol_points[i].wait_node,
							   chrInitAI->patrol_points[i].wait_time);
			}
		}
	}
}

//Получить матрицу объекта
Matrix & Character::GetMatrix(Matrix & mtx)
{	
	if (useRootBone)
	{
		return physics->GetMatrixWithBoneOffset(mtx);
	}
	
	return physics->GetModelMatrix(mtx);
}

Matrix& Character::GetColiderMatrix(Matrix & mtx)
{
	return physics->GetModelMatrix(mtx);
}

//Показать/скрыть объект
void Character::Show(bool isShow)
{
	if(showProcess)
	{
		LogicDebugError("Show process already started");
		return;
	}

	if (logic->master && logic->master->IsPlayer() && !isShow && IsShow())
	{
		LogicDebugError("This character is slave and cannot be hided, but he can be deactivated");
		return;
	}

	showProcess = true;
	
	if (!IsShow() && isShow)
	{
		was_showed = true;

		/*if (graph_changed)
		{
			showProcess = false;
			return;
		}*/
	}

	logic->UpdateFatalityParams();
	DamageReceiver::Show(isShow);
	if(IsDead() && isShow)
	{
		LogicDebug("Warning: Character is dead and can't be showing...");
		isShow = false;
	}

	if(isShow)
	{
//		Registry(MG_DOF, &Character::ShadowDraw, ML_DYNAMIC2);
		
		if(shadowCast)
		{
			Registry(MG_SHADOWCAST, &Character::ShadowInfo, ML_GEOMETRY2);
		}
		else
		{
			Unregistry(MG_SHADOWCAST);
		}
		
		if(shadowReceive)
		{
			Unregistry(MG_SHADOWDONTRECEIVE);
			Registry(MG_SHADOWRECEIVE, &Character::ShadowDraw, ML_GEOMETRY2);
		}
		else
		{
			Registry(MG_SHADOWDONTRECEIVE, (MOF_EVENT)&Character::ShadowDraw, ML_GEOMETRY2);
			Unregistry(MG_SHADOWRECEIVE);
		}
		
		if(seaReflection)
		{
			Registry(MG_SEAREFLECTION, &Character::ReflectionDraw, ML_GEOMETRY2);
		}
		else
		{
			Unregistry(MG_SEAREFLECTION);
		}		

		if(seaRefraction)
		{
			Registry(MG_SEAREFRACTION, &Character::ReflectionDraw, ML_GEOMETRY2);
		}
		else
		{
			Unregistry(MG_SEAREFRACTION);
		}		

		bShadowGrRegistred = true;
		bReflectGrRegistred = true;
		bRefractGrRegistred = true;
	}
	else
	{
		if (flyCloud)
		{
			flyCloud->Release(false);
			flyCloud = null;
		}

		if(events)
		{
			events->ResetParticles();
			events->ResetSounds();
		}

//		Unregistry(MG_DOF);
		Unregistry(MG_SHADOWCAST);
		Unregistry(MG_SHADOWRECEIVE);
		Unregistry(MG_SEAREFLECTION);
		Unregistry(MG_SEAREFRACTION);
		Unregistry(MG_SHADOWDONTRECEIVE);
	}


	if(!EditMode_IsOn())
	{
		if(isShow)
		{
			/*if (logic->PairModeAllowed(true))
			{
				SetCharacterCoin(Character::chrcoin_fatality);
			}
			else*/
			{
				SetCharacterCoin(Character::chrcoin_none);
			}

			LogicDebug("Show");
			SetUpdate(&Character::DrawTrail, ML_ALPHA3 + 1);
			SetUpdate(&Character::DrawFlares, ML_ALPHA3 + 10);
			
			if (IsPlayer()) SetUpdate(&Character::DrawHUD, ML_GUI1);

			if(deadTime < 0.0f)
			{
				SetUpdate(&Character::Draw, ML_GEOMETRY3);
				
				if (!IsPlayer()) SetUpdate(&Character::DrawCoin, ML_ALPHA4);				
			}
			else
			{
				DelUpdate(&Character::DrawCoin);				
			}		

			if(animation)
			{
				animation->Pause(logic->IsActor());
			}
			if(actorData.ani)
			{
				actorData.ani->Pause(!logic->IsActor());
			}
		}
		else
		{
			LogicDebug("Hide");
			
			if(animation)
			{
				animation->Pause(true);				
			}
			if(actorData.ani)
			{
				actorData.ani->Pause(true);
			}
			
			DelUpdate(&Character::DrawTrail);
			DelUpdate(&Character::DrawFlares);
			DelUpdate(&Character::Draw);
			DelUpdate(&Character::DrawCoin);
			if (IsPlayer()) DelUpdate(&Character::DrawHUD);
		}

		Activate(IsActive());

		controller->Show(isShow);
	}
	else
	{
		if(isShow)
		{
			SetUpdate(&Character::EditMode_Draw, ML_ALPHA5);
			SetUpdate(&Character::DrawCoin, ML_ALPHA2);
			SetUpdate(&Character::DrawFlares, ML_ALPHA5);

			DelUpdate(&Character::DrawTransparency);			
		}
		else
		{
			DelUpdate(&Character::EditMode_Draw);

			DelUpdate(&Character::DrawCoin);

			SetUpdate(&Character::DrawFlares);

			SetUpdate(&Character::DrawTransparency, ML_ALPHA5);
		}
	}

	showProcess = false;
}

//Активировать/деактивировать объект
void Character::Activate(bool isActive)
{	
	if (IsPlayer())
	{
		PlayerController * pc = (PlayerController*)controller;
		if (pc->itemTaken && pc->phys_object && pc->isDropStarted)
		{
			pc->DropWeapon(false);
			pc->BrokeWeapon(true);
		}
	}

	if (logic->IsActor() && !isActive && IsShow())
	{
		LogicDebugError("Actor Cannot be deactivated");
		return;
	}

	if(activeProcess)
	{
		LogicDebugError("Activate process already started");
		return;
	}
	activeProcess = true;
	DamageReceiver::Activate(isActive);
	if(EditMode_IsOn()) isActive = false;
	if((IsDead() && isActive))
	{
		LogicDebug("Warning: Character is dead and can't be activate now...");
		isActive = false;
	}
	if(isActive)
	{
		LogicDebug("Activate");
	}else{
		LogicDebug("Deactivate");
	}

	if (controller)
	{
		controller->Reset();
		controller->SetChrTarget(NULL);
	}

	//if(!logic->IsActor())
	{
		if(!IsShow()) isActive = false;
		if(IsDie()) isActive = false;
		if(isActive && logic->GetHP()>0)// && logic->GetState() != CharacterLogic::state_die && logic->GetState() != CharacterLogic::state_dead)
		{
			SetUpdate(&Character::Work, ML_DYNAMIC3);
			SetUpdate(&Character::UpdatePhysics, ML_DYNAMIC2);
			
			if (IsPlayer()) SetUpdate(&Character::UpdatePhysObj, ML_EXECUTE1);

			if (controller->AllowRestartAnim())
			{
				if(animation && !ragdoll)
				{
					animation->Start();
				}
			}
		}
		else
		{
			DelUpdate(&Character::Work);
			DelUpdate(&Character::UpdatePhysics);

			if (IsPlayer()) DelUpdate(&Character::UpdatePhysObj);
		}		
	}

	if(!logic->IsActor())
	{
		physics->Activate(isActive);		
	}
	else
	{
		physics->Activate(bColideWhenActor&&isActive);
		//ResetActor();
		//UpdateActor(0);
	}

	if (isActive && !logic->IsActor())
	{
		controller->Tick();
	}

	arbiter->Activate(this, isActive);
		
	bodyparts.Activate(isActive, &Sound());

	finder->Activate(isActive);	

	activeProcess = false;
}

//Умирает
bool Character::IsDie()
{
	return logic->IsDead();
}

//Мёртв
bool Character::IsDead()
{
	return deadTime >= GetDeathTime() || deadBodyTime >= GetDeathTime();
}

//Получить текущее значение жизни
float Character::GetHP()
{
	return logic->GetHP();
}

//Получить максимальное значение жизни
float Character::GetMaxHP()
{
	return logic->GetMaxHP();
}


//Добавить отладочное сообщение
void _cdecl Character::DebugMessage(dword color, const char * format, ...)
{
#ifndef NO_CONSOLE
	if(debugMessages >= 8)
	{
		for(long i = 0; i < debugMessages - 1; i++)
		{
			debugMessages[i] = debugMessages[i + 1];
		}
	}else{
		debugMessages.Add();
	}
	DebugMessageInfo & dm = debugMessages[debugMessages - 1];
	dm.color = color;
	crt_vsnprintf(dm.str, sizeof(dm.str), format, (char *)((&format) + 1));
	dm.str[sizeof(dm.str) - 1] = 0;
	debugMessageShowTime = CHARACTER_SHOWMSGTIME;
#endif
}

void Character::CheckRenderState()
{
	if (!seaReflection && !shadowReceive && !shadowCast && !seaRefraction) return;
	if (!pattern) return;


	if (!IsShow())
	{
		if (seaReflection) Unregistry(MG_SEAREFLECTION);
		if (seaRefraction) Unregistry(MG_SEAREFRACTION);
		if (shadowCast) Unregistry(MG_SHADOWCAST);
		if (shadowReceive) Unregistry(MG_SHADOWDONTRECEIVE);		
	}

	Vector pos;

	if (hips_boneIdx == -1)
	{
		if (!currentModel) return;

		pos = (currentModel->GetLocalBound().vMin + currentModel->GetLocalBound().vMax) * 0.5f;
	}
	else
	{
		pos = physics->GetPos(true);
	}

	float dist_to_cam = (Render().GetView().GetCamPos() - pos).GetLength();

	if (seaReflection)
	{
		if (bReflectGrRegistred)
		{
			if (pattern->fDistReflect + 1 < dist_to_cam)
			{
				Unregistry(MG_SEAREFLECTION);
				bReflectGrRegistred = false;
			}
		}
		else
		{
			if (pattern->fDistReflect > dist_to_cam)
			{
				Registry(MG_SEAREFLECTION, &Character::ReflectionDraw, ML_GEOMETRY2);
				bReflectGrRegistred = true;
			}
		}
	}

	if (seaRefraction)
	{		
		if (bRefractGrRegistred)
		{
			if (pattern->fDistReflect + 1 < dist_to_cam)
			{
				Unregistry(MG_SEAREFRACTION);
				bRefractGrRegistred = false;
			}
		}
		else
		{
			if (pattern->fDistReflect > dist_to_cam)
			{
				Registry(MG_SEAREFRACTION, &Character::ReflectionDraw, ML_GEOMETRY2);
				bRefractGrRegistred = true;
			}
		}

	}

	float fDstDhd = pattern->fDistShadow;
	if (fDistShadow>0.01f) fDstDhd = fDistShadow;
		
	/*if (shadowReceive || shadowCast)
	{
		if (bShadowGrRegistred)
		{
			if (fDstDhd + 1 < dist_to_cam)
			//if (35 + 1 < dist_to_cam)
			{
				if (shadowReceive)
				{
					Registry(MG_SHADOWDONTRECEIVE, (MOF_EVENT)&Character::ShadowDraw, ML_GEOMETRY2);
					Unregistry(MG_SHADOWRECEIVE);
				}

				if (shadowCast) Unregistry(MG_SHADOWCAST);
				bShadowGrRegistred = false;
			}
		}
		else
		{
			if (fDstDhd > dist_to_cam)
			//if (35 > dist_to_cam)
			{
				if (shadowReceive)
				{
					Unregistry(MG_SHADOWDONTRECEIVE);
					Registry(MG_SHADOWRECEIVE, &Character::ShadowDraw, ML_GEOMETRY2);
				}
				if (shadowCast) Registry(MG_SHADOWCAST, &Character::ShadowInfo, ML_GEOMETRY2);
				bShadowGrRegistred = true;
			}
		}
	}*/
}

//Работа персонажа
void _cdecl Character::Work(float dltTime, long level)
{
	killsPerSecond = Max(0.0f, killsPerSecond - dltTime * 0.25f);

	if (dltTime<0.0001f)
	{
		if (IsPlayer())
		{
			controller->Update(0.0f);
		}

		return;
	}

	if (IsPlayer())
	{
		/*arbiter->Find(*this, 0.05f, 15.55f, false, -PI*0.3f, PI*0.3f, 0.5f);	
		array<CharacterFind> & find = arbiter->find;
			
		for(long i = 0; i < find; i++)
		{
			Render().DrawSphere(find[i].chr->physics->GetPos(), 1.0f, 0xff00ffff);			
		}*/
	
		if (api->DebugKeyState('M','G') && mg_params)
		{
			if (buttonMG->IfMissionTime())
			{
				buttonMG->SetMissionTime(null);
			}
			else
			{
				buttonMG->SetMissionTime(mg_params->GetMissionTime(true));
			}
	
			Sleep(200);
		}
	}

#ifdef CHARACTER_STARFORCE
	if (arbiter->timeInSecFromStart->Get(0.0f) > 1777.0)
	{
		double guarding = 73.0 - arbiter->GuardingValueDamage<double>(1);
		if (arbiter->GetTime().GetTimeFloat() >= scaleSpikeTime && guarding > 1.0)
		{
			curScaleSpike *= powf(scaleSpike, 0.4f);
			curScaleSpike = Max(0.1f, Min(4.0f, curScaleSpike));
		}
	}
#endif

	/*if (logic->IsActor() && IsPlayer())
	{
		Sleep(20);

		IAnimation* anim = animation->GetCurAnimation();

		const char* node1 = anim->CurrentNode();

		if (api->DebugKeyState('V'))
		{
			api->Trace("ID Player : Node %s",node1);
		}

		if (pair_minigame[0])
		{			
			anim = pair_minigame[0]->animation->GetCurAnimation();
			
			const char* node2 = anim->CurrentNode();

			if (api->DebugKeyState('V'))
			{
				api->Trace("          : Node %s",node2);
			}

			if (!string::IsEqual(node1,node2))
			{
				if (events->IsEnableTimeEvent())
				{
					api->Trace("ID Player : Node %s, TimeEvent Enabled",node1);
				}
				else
				{
					api->Trace("ID Player : Node %s, TimeEvent Disabled",node1);
				}
				
				
				if (pair_minigame[0]->events->IsEnableTimeEvent())
				{
					api->Trace("        : Node %s, TimeEvent Enabled",node2);
				}
				else
				{
					api->Trace("        : Node %s, TimeEvent Disabled",node2);
				}
			}

			Assert(string::IsEqual(node1,node2));			
		}		
	}*/

	was_showed = false;

	CheckRenderState();	

	/*if (IsPlayer())
	{
		if (api->DebugKeyState('B'))
		{
			logic->autoZoom->Activate(true);			
		}
		
		if (api->DebugKeyState('V'))
		{
			logic->autoZoom->Activate(false);			
		}
	}*/

	
	if (IsPlayer())
	{
		dltTime *= logic->GetTimeScale();		
	}

	if (flyCloud)
	{
		Matrix mat;
		physics->GetMatrixWithBoneOffset(mat);

		flyCloud->SetPosition(mat.pos);
	}


	if (logic->is_attached>0)
	{
		if (logic->is_attached>1) logic->is_attached--;		
		return;
	}	

	if (shadowModel.scene)
	{
		Matrix mtx1(true);
		physics->GetMatrixWithBoneOffset(mtx1);		

		currentModel = model.scene;

		Vector cam_pos = Render().GetView().GetCamPos();

		Matrix mProj = Render().GetProjection();		

		float k = 1 / (mProj.m[0][0] * DEF_CAM_FOV);

		if (pattern)
		{
			if ((cam_pos-mtx1.pos).GetLengthXZ2()*k*k>pattern->fLODDist)
			{
				currentModel = shadowModel.scene;
			}
		}
	}

	if (logic->PairModeAllowed(false,false))
	{
		fTimeInFatality += dltTime;
	}	

	if (!logic->IsActor())
	{		
		Matrix boneMatrix(true);
		physics->GetModelMatrix(boneMatrix);

		if (ragdoll)
		{
			physics->GetMatrixWithBoneOffset(boneMatrix);
		}

		//Render().DrawMatrix(boneMatrix);

		float fWaterLevel = IWaterLevel::GetWaterLevel(Mission(), waterLevel);		

		if (boneMatrix.pos.y < -2.0f+fWaterLevel || boneMatrix.pos.y < -900.0f)
		{
			physics->SetPos(boneMatrix.pos);

			physics->GetModelMatrix(boneMatrix);
			model.scene->SetTransform(boneMatrix);
			
			if (!arbiter->IsMultiplayer() && logic && logic->GetHP() > 0.0f)
				ACHIEVEMENT_REACHED(arbiter->GetPlayer(), FISH_FOOD);

			Kill();
			bWaterSpalshBorned = true;
			Show(false);

			ReleaseFlyes();

			return;
		}		

		//if (IsPlayer()) Render().Print(10,10,0xff00ff00,"%f %f",boneMatrix.pos.y,fWaterLevel);

		if (boneMatrix.pos.y < fWaterLevel && !bWaterSpalshBorned && ragdoll)
		{
			bWaterSpalshBorned = true;

			boneMatrix.pos.y = fWaterLevel;			

			IParticleSystem * p = null;	

			p = Particles().CreateParticleSystem("CharSplash");		

			if(p)
			{
				p->Teleport(Matrix(Vector(0.0f), Vector(boneMatrix.pos.x,fWaterLevel,boneMatrix.pos.z)));
				p->AutoDelete(true);
			}

			ReleaseFlyes();
		}
	}

	if (IsPlayer() && !logic->IsActor() && !logic->IsMinigameStarted())
	{
		Vector cam_pos = Render().GetView().GetCamPos();

		Matrix mtx;
		physics->GetMatrixWithBoneOffset(mtx);
		Vector pos = mtx.pos;

		Matrix mat;
		mat.BuildView(cam_pos,pos,Vector(0,1,0));
		mat.Inverse();
		mat.pos = pos;
		Sound().SetListenerMatrix(mat);
	}
	
	//if (IsPlayer()) PushCharcters(dltTime,2.5f,true);

	//if(controller && IsPlayer())
	//{
	//	controller->Update(dltTime);			
	//}	

	//ProfileTimer timer;

	/*
	if (bAutoReleaseRagdollOn)
	{
		fTimeToReleaseRagDoll -= dltTime;
		
		if (fTimeToReleaseRagDoll<0.0f)
		{
			bAutoReleaseRagdollOn = false;
			fTimeToReleaseRagDoll = 0.0f;

			ragdoll->Release();
			ragdoll = NULL;
		}
	}		

	if (bSelfDestructRagdoll&&ragdoll)
	{	
		if (rd_last_timeout>0)
		{
			rd_last_timeout -= dltTime;
		}
		else
		{
			Matrix mtx1(true);
			physics->GetModelMatrix(mtx1);

			Matrix joint_mat = GetBoneMatrix(spine_boneIdx);		

			mtx1 = joint_mat * mtx1;

			Vector cur_rd_pos = mtx1.pos;

			Vector dir = cur_rd_pos - rd_last_pos2;
			float speed = dir.Normalize()/ (1/60.0f);			
					
			Vector chr_pos;

			if ((arbiter->IfAnyCharacer(this,cur_rd_pos,1.5f,chr_pos) && speed>2.0f)||			
				(((speed>0.25f && speed<3.5f) || (dir|rd_last_dir)<0.95f) && (rd_last_speed - speed)>0 && rd_inited))
			{	
				items->DropArmor(true);

				//Рождаем эффекты
				IParticleSystem * p = null;	

				p = Particles().CreateParticleSystem("ExplosionBomb");
				Sound().Create3D("Bomb boom", cur_rd_pos, _FL_);

				if(p)
				{
					p->Teleport(Matrix(Vector(0.0f), cur_rd_pos));
					p->AutoDelete(true);
				}

				arbiter->Boom(logic->GetOffender(),cur_rd_pos,3.0f,400.0f,1.0f);
				bSelfDestructRagdoll = false;
			}		

			float tst_speed = (cur_rd_pos - rd_last_pos).GetLength()/ (1/60.0f);		

			if (tst_speed>0.85f)
			{
				rd_last_pos2 = rd_last_pos;
				rd_last_pos = mtx1.pos;			

				rd_last_dir = dir;

				rd_last_speed = speed;

				rd_inited = true;
			}
		}
	}	

	if (ragdoll && !logic->IsActor() && !ragdoll->IsFreezed())
	{		
		Matrix mtx1(true);
		physics->GetModelMatrix(mtx1);

		Matrix joint_mat = GetBoneMatrix(spine_boneIdx);		

		mtx1 = joint_mat * mtx1;
		
		Vector vDelta = vLastRagdollPos - mtx1.pos;
		
		vLastRagdollPos = mtx1.pos;

		float speed = vDelta.Normalize()/ (1/60.0f);

		//fTotalTimeToStandUp -= dltTime;		
		
		//if (fTotalTimeToStandUp>0.0f)
		//{
			//speed = 0.0f;
		//}

		if (speed<3.0f)
		{
			fTimeToStandUp-=dltTime;

			if (fTimeToStandUp<=0)
			{				
				if (logic->GetHP()>0.0f &&					
					logic->GetState() != CharacterLogic::state_die &&
					logic->GetState() != CharacterLogic::state_dead)
				{
					ActivateRagdoll(false,0.0f);
					
					if (joint_mat.vz.y>0)
					{
						animation->ActivateLink("StandUp",0.0f);
						fRotDlt=PI;
					}
					else
					{
						animation->ActivateLink("StandUp2",0.0f);					
						fRotDlt=0;
					}
								
					set_new_ay = true;
					new_ay += fRotDlt;

					//physics->SetAy(physics->GetAy()+fRotDlt);
				}
				else
				{
					ragdoll->Freeze();
				}
			}
		}
		else
		{	
			if (logic->GetHP()>0.0f)
			{
				fTimeToStandUp = 2.0f;
			}
			else
			{
				fTimeToStandUp = 3.0f;
			}			
		}		
	}*/
		
	if(!logic->IsActor())
	{
		if(!logic->IsPairMode())
		{
			//Обновление логических параметров состояния
			if(controller &&
			   /*logic->GetState() != CharacterLogic::state_die &&
			   logic->GetState() != CharacterLogic::state_dead &&*/
			   logic->GetHP()>0 && !ragdoll)
			{				
				controller->Update(dltTime);
			}

			//Если мертвы, запускаем счётчик смерти и деактивируемся
			if((/*logic->GetState() == CharacterLogic::state_die ||
			    logic->GetState() == CharacterLogic::state_dead ||*/
			    logic->GetHP()<=0.0f) && !bDeadConterOn)
			{
				SetCharacterCoin(Character::chrcoin_none);
				
				DelUpdate(&Character::Draw);
				SetUpdate(&Character::Draw, ML_ALPHA1);

				SetUpdate(&Character::DeadTimer, ML_EXECUTE2);
				if (!bVanishBody)
					arbiter->AddDeadBody(*this);
				LogicDebug("Death process");

				if (controller)
				{
					controller->SetChrTarget(null);
				}

				if (arbiter->fly_manager.Ptr() && !IsPlayer())
				{
					flyCloud = arbiter->fly_manager.Ptr()->CreateFlys(1.2f,(int)RRnd(10,20));
				}
				
				bDeadConterOn = true;
				// Vano: было 0.0f, сделал хак, иначе после переключений персов не было, а тени были
				deadTime = -0.0001f;
				deadBodyTime = 0.0f;

				//Unregistry(MG_SHADOWCAST);
				//Unregistry(MG_SHADOWRECEIVE);	
				//Registry(MG_SHADOWDONTRECEIVE, (MOF_EVENT)&Character::ShadowDraw);

				//Show(IsShow());
			}			
		}
		else
		{
			PushCharcters(dltTime,1.7f,true,false);			
			
			//physics->Move(dltTime);

			//Обновление логических параметров состояния
			if(controller &&
			   logic->GetState() != CharacterLogic::state_die &&
			   logic->GetState() != CharacterLogic::state_dead &&
			   logic->GetHP()>0 && !ragdoll)
			{
				//if (strcmp(controller->Name(),"Player")==0)
				{
					controller->Update(dltTime);
				}				
			}

			//Если ме
			if((logic->GetState() == CharacterLogic::state_die||
				logic->GetState() == CharacterLogic::state_dead ||
				logic->GetHP()<=0.0f) && !bDeadConterOn && !logic->is_attached)
			{
				SetUpdate(&Character::DeadTimer, ML_EXECUTE2);
				if (!bVanishBody)
					arbiter->AddDeadBody(*this);
				LogicDebug("Death process");
				
				SetCharacterCoin(Character::chrcoin_none);

				DelUpdate(&Character::Draw);
				SetUpdate(&Character::Draw, ML_ALPHA2);

				if (controller)
				{
					controller->SetChrTarget(null);
				}

				if (arbiter->fly_manager.Ptr() && !IsPlayer())
				{
					flyCloud = arbiter->fly_manager.Ptr()->CreateFlys(1.2f,(int)RRnd(10,20));
				}

				bDeadConterOn = true;
				// Vano: было 0.0f, сделал хак, иначе после переключений персов не было, а тени были
				deadTime = -0.0001f;
				deadBodyTime = 0.0f;

				//Unregistry(MG_SHADOWCAST);
				//Unregistry(MG_SHADOWRECEIVE);
				//Registry(MG_SHADOWDONTRECEIVE, (MOF_EVENT)&Character::ShadowDraw);				

				//Show(IsShow());
			}
		}

		logic->Update(dltTime);
	}
	else
	{
		if(controller && IsPlayer())
		{
			controller->Update(dltTime);			
		}		
	}


	Matrix mtx(true);
	physics->GetModelMatrix(mtx);
	items->Update(dltTime);
	events->Update(mtx, dltTime);	
	
	//Моргание глазами
	bodyparts.SetDead(IsDead());	

	finder->SetMatrix(mtx);		

	//timer.AddToCounter("Char Work");


	//hairblender.SetDelltaPos(physics->dltPos);
	//hairblender.SetMatrix(mtx);


	Matrix mt = mtx;
	//mt.pos = 0.0f;
	bodyparts.SetMatrix(mt);
	bodyparts.SetDeltaPos(physics->dltPos);


	if (IsPlayer())
	{
		if (greedy)
		{
			greedy->SetPosition( physics->GetPos() + Vector(0.0f,1.0f,0.0f));
			//Render().DrawSphere( greedy->GetPosition(),0.5f,0xff00ff00);
		}
	}
}

void _cdecl Character::UpdatePhysics(float dltTime, long level)
{
	if (dltTime<1e-30f)
	{
		physics->dltPos = 0.0f;
		return;
	}

	if(!logic->IsAttack())
	{
		physics->TurnUpdate(dltTime);
	}

	if(!logic->IsActor())
	{		
		if (!ragdoll) physics->Move(dltTime);		
	}
	else
	{		
		if (bColideWhenActor)
		{
			PushCharcters(dltTime,1.7f,true,false);

			if (!actorTimeLine)
			{
				physics->Move(dltTime);
			}
			else
			{
				//Перемещение в режиме актёра
				UpdateActor(dltTime);
			}
		}
		else
		{
			UpdateActor(dltTime);
		}
	}

	if (logic->slave)
	{
		if (!logic->slave->IsShow())
		{
			if (IsPlayer())
			{
				((PlayerController*)controller)->BrokeWeapon(true);
			}

			logic->slave = null;
			return;
		}

		logic->slave->physics->SetPos(physics->GetPos());
		logic->slave->physics->SetAy(physics->GetAy());
		logic->slave->physics->SetAz(physics->GetAz());
	}
}

void _cdecl Character::UpdatePhysObj(float dltTime, long level)
{
	if (IsPlayer() && controller)
	{
		((PlayerController*)controller)->UpdatePhysObject();
	}
}

void Character::ReCalcMatrices()
{
	bool reCalc = false;

	if(!logic->IsActor())
	{		
		reCalc = true;
	}
	else
	{		
		if (bColideWhenActor)
		{			
			if (!actorTimeLine)
			{
				reCalc = true;
			}			
		}		
	}

	if (reCalc) physics->ReCalcMatrices();
}

//Нарисовать модельку персонажа
void _cdecl Character::Draw(float dltTime, long level)
{	
	if (oneFrameHide)
		return;

	/*if (IsPlayer())
	{
		Render().Print(10.0f, 20.0f, 0xFF00FF00, "killsPerSecond = %.1f", killsPerSecond);
	}*/

	Matrix mtx(true);
	if(isHitLight)
	{
		hitLight += dltTime*22.0f;
		if (hitLight > 1.0f)
		{
			hitLight = 1.0f;
			isHitLight = false;
		}
	}
	else
	{
		hitLight -= dltTime*8.0f;
		if (hitLight < 0.0f) hitLight = 0.0f;
	}	

	//Рисуем персонажа
	if(currentModel && bDraw)	
	{
		physics->GetModelMatrix(mtx);
		float a = 1.0f;
		
		if (!IsPlayer() && logic->GetSide()!=CharacterLogic::s_boss)
		{		
			if (deadTime > 0.0f && GetDeathTime()-deadTime<CHARACTER_BODYDISAPEAR)
			{
				a = GetAlpha();

				if (a<0.525f)
				{
					Unregistry(MG_SHADOWRECEIVE);
				}			
			}
		}

		Color userColor(hitLight*0.6f+add_color.r, hitLight*0.4f+add_color.g, hitLight*0.3f+add_color.b, a);

		if (bAutoReleaseRagdollOn && !set_new_ay)
		{		
			Matrix joint_mat = GetBoneMatrix(hips_boneIdx);			

			Matrix mtx2;

			mtx2.pos = -joint_mat.pos;
			mtx2.pos.y = 0;

			Matrix mtx3;			
			mtx3.RotateY(-joint_mat.vy.GetAY()+fRotDlt);

			mtx = mtx2 * mtx3 * mtx;
		}

		if (currentModel!=model.scene)
		{
			model.scene->SetTransform(mtx);
		}
			
		//Render().DrawMatrix(mtx);
		DrawModel( mtx, currentModel, true, &userColor);		

		currentModel->SetUserColor(Color(0xff000000L));
		//Render().DrawSphere(mtx.pos,0.15f);		
	}


	if(showCollider)
	{
		physics->GetColliderMatrix(mtx);
		mtx.Scale3x3(physics->GetRadius(), physics->GetHeight()*0.5f, physics->GetRadius());
		Render().DrawSphere(mtx, 0x9fc0c0ff);
		Render().Print(mtx.pos + Vector(0.0f, 1.6f, 0.0f), 10.0f, 1.0f, 0xffffffff, "Id: %s", GetObjectID().c_str());
	}

	if( pattern && pattern->cHairParams.NotEmpty() )
	{
		bodyparts.Draw(dltTime);
	}	

	for (dword i=0;i<BodyParts.Size();i++)
	{
		Color userColor(0.0f, 0.0f, 0.0f, GetAlpha());

		if (BodyParts[i].pPhysBox || BodyParts[i].pPhysCapsule)
		{
			const GMXBoundBox & volume = BodyParts[i].PartModel.scene->GetLocalBound();
		
			Vector size = volume.vMax - volume.vMin;		
			Vector center = volume.vMin + (size * 0.5f);

			Matrix mat;
			mat.pos=-center;

			if (BodyParts[i].pPhysBox)
			{
				BodyParts[i].pPhysBox->GetTransform(mtx);
			}
			else
			if (BodyParts[i].pPhysCapsule)
			{
				BodyParts[i].pPhysCapsule->GetTransform(mtx);
			}

			mtx = mat * mtx;

			BodyParts[i].PartModel.scene->SetUserColor(userColor);
			BodyParts[i].PartModel.scene->SetTransform(mtx);
			BodyParts[i].PartModel.scene->Draw();
			BodyParts[i].PartModel.scene->SetUserColor(Color(0xff000000L));
		}
	}

#ifndef NO_CONSOLE
	if(debugMessages)
	{
		float alpha = 1.0f;
		debugMessageShowTime -= dltTime;
		if(debugMessageShowTime < 0.0f)
		{
			if(debugMessageShowTime <= -1.0f)
			{
				debugMessages.DelIndex(0);
				debugMessageShowTime = CHARACTER_SHOWMSGTIME;
			}else{
				alpha = 1.0f + debugMessageShowTime;
			}
		}
		Vector pos = physics->GetPos() + Vector(0.0f, physics->GetHeight() + 0.2f, 0.0f);
		for(long i = 0; i < debugMessages; i++)
		{
			dword color = debugMessages[i].color;
			if(i == 0)
			{
				Color c(color);
				c.alpha *= alpha;
				color = c.GetDword();
			}
			Render().Print(pos, 100.0f, (float)(-i), color, "%s", debugMessages[i].str);
		}
	}	

#endif	

	if (need_post_grab_weapon)
	{
		need_post_grab_weapon = false;	

		((PlayerController*)controller)->TakeWeapon(pgw_victim->items->items[pgw_wp_index].weapon_type,
													pgw_victim->items->items[pgw_wp_index].model_name,										
													pgw_victim->items->items[pgw_wp_index].model_part1_name,
													pgw_victim->items->items[pgw_wp_index].model_part2_name,
													pgw_victim->items->items[pgw_wp_index].tip_id, pgw_blend_time,pgw_start_node);
	}		

	GetUp( dltTime, level);
}

void Character::SelfBlowBombardier()
{
	Matrix mtx1(true);
	physics->GetModelMatrix(mtx1);

	Matrix joint_mat = GetBoneMatrix(spine_boneIdx);		

	mtx1 = joint_mat * mtx1;

	Vector cur_rd_pos = mtx1.pos;

	items->DropArmor(true);

	//Рождаем эффекты
	IParticleSystem * p = null;	

	p = Particles().CreateParticleSystem("ExplosionBomb");
	Sound().Create3D("bmb_blast", cur_rd_pos, _FL_);

	if(p)
	{
		p->Teleport(Matrix(Vector(0.0f), cur_rd_pos));
		p->AutoDelete(true);
	}

	arbiter->Boom(logic->GetOffender(),DamageReceiver::ds_bomb, cur_rd_pos, 3.0f, 600.0f, 1.0f, this);
	bSelfDestructRagdoll = false;
}		

void _cdecl Character::GetUp(float dltTime, long level)
{
	if (bAutoReleaseRagdollOn)
	{
		fTimeToReleaseRagDoll -= dltTime;

		if (fTimeToReleaseRagDoll<0.0f)
		{
			bAutoReleaseRagdollOn = false;
			fTimeToReleaseRagDoll = 0.0f;

			ragdoll->Release();
			ragdoll = NULL;
		}
	}		

	if (bSelfDestructRagdoll&&ragdoll)
	{	
		if (rd_last_timeout>0)
		{
			rd_last_timeout -= dltTime;
		}
		else
		{
			Matrix mtx1(true);
			physics->GetModelMatrix(mtx1);

			Matrix joint_mat = GetBoneMatrix(spine_boneIdx);		

			mtx1 = joint_mat * mtx1;

			Vector cur_rd_pos = mtx1.pos;

			Vector dir = cur_rd_pos - rd_last_pos2;
			float speed = dir.Normalize()/ (1/60.0f);			

			Vector chr_pos;

			// VANO: было if ((arbiter->IfAnyCharacer(this,cur_rd_pos,1.5f,chr_pos) && speed>2.0f)||			
			if ((arbiter->IfAnyCharacer(this,cur_rd_pos,2.5f,chr_pos))||			
				(((speed>0.25f && speed<3.5f) || (dir|rd_last_dir)<0.95f) && (rd_last_speed - speed)>0 && rd_inited))
			{	
				SelfBlowBombardier();
			}

			float tst_speed = (cur_rd_pos - rd_last_pos).GetLength()/ (1/60.0f);		

			if (tst_speed>0.85f)
			{
				rd_last_pos2 = rd_last_pos;
				rd_last_pos = mtx1.pos;			

				rd_last_dir = dir;

				rd_last_speed = speed;

				rd_inited = true;
			}
		}
	}	

	if (ragdoll && !logic->IsActor() && !ragdoll->IsFreezed())
	{
		Matrix mtx1(true);
		physics->GetModelMatrix(mtx1);

		Matrix joint_mat = GetBoneMatrix(spine_boneIdx);		

		mtx1 = joint_mat * mtx1;

		Vector vDelta = vLastRagdollPos - mtx1.pos;

		vLastRagdollPos = mtx1.pos;

		float speed = vDelta.Normalize()/ (1/60.0f);

		//fTotalTimeToStandUp -= dltTime;		

		//if (fTotalTimeToStandUp>0.0f)
		//{
		//speed = 0.0f;
		//}

		if (speed<3.0f)
		{
			fTimeToStandUp-=dltTime;

			if (fTimeToStandUp<=0)
			{				
				if (logic->GetHP()>0.0f &&					
					logic->GetState() != CharacterLogic::state_die &&
					logic->GetState() != CharacterLogic::state_dead)
				{
					ActivateRagdoll(false,0.0f);

					if (joint_mat.vz.y>0)
					{
						animation->ActivateLink("StandUp",0.0f);
						fRotDlt=PI;
					}
					else
					{
						animation->ActivateLink("StandUp2",0.0f);					
						fRotDlt=0;
					}

					set_new_ay = true;
					new_ay += fRotDlt;

					//physics->SetAy(physics->GetAy()+fRotDlt);
				}
				else
				{
					fTimeToFreeze -= dltTime;
					if (fTimeToFreeze <= 0.0f)
					{
						fTimeToFreeze = 2.0f;

						if (g_bFreezeRagdolls)
							ragdoll->Freeze();
					}
				}
			}
		}
		else
		{	
			float standUpTimes[10] = { 0.75f, 1.2f, 2.1f, 0.9f, 1.5f, 1.0f, 1.8f, 1.35f, 2.3f, 1.6f };
			static int standUpIndex = 0;
			// Vano: время вставания с пола
			if (logic->GetHP()>0.0f)
			{
				fTimeToStandUp = standUpTimes[standUpIndex % ARRSIZE(standUpTimes)];//RRnd(0.8f, 2.2f);
			}
			else
			{
				fTimeToStandUp = standUpTimes[standUpIndex % ARRSIZE(standUpTimes)];//RRnd(0.8f, 3.0f);
			}			
			standUpIndex++;
		}		
	}	

	if (set_new_ay)
	{
		set_new_ay = false;

		if (!physics->IsActive())
		{
			//physics->SetPos(new_pos);
			physics->Activate(true, &new_pos);
		}

		physics->SetAy(new_ay);
	}
}

//Нарисовать полупрозрачную модельку персонажа
void _cdecl Character::DrawTransparency(float dltTime, long level)
{
	if(!EditMode_IsVisible()) return;
	//Рисуем персонажа
	if(currentModel)
	{
		Matrix mtx(true);
		physics->GetModelMatrix(mtx);
		DrawModel(mtx, currentModel, false, &Color(0xa0000000L));
	}
}

//Нарисовать модельку персонажа для тени
void _cdecl Character::ShadowInfo(const char * group, MissionObject * sender)
{
	if(sender)
	{
		//Матрица модельки
		Matrix mtx(true);
		physics->GetModelMatrix(mtx);
		//Моделька
		IGMXScene * mdl = shadowModel.scene;
		if(!mdl) mdl = model.scene;
		if(!mdl) return;		

		if (bAutoReleaseRagdollOn)
		{				
			Matrix joint_mat = GetBoneMatrix(hips_boneIdx);		

			Matrix mtx2(true);		

			mtx2.pos = -joint_mat.pos;
			mtx2.pos.y = 0;

			Matrix mtx3;
			mtx3.RotateY(-joint_mat.vy.GetAY()+fRotDlt);

			mtx = mtx2 * mtx3 * mtx;
		}

		mdl->SetTransform(mtx);
		const Vector & vMin = mdl->GetBound().vMin;
		const Vector & vMax = mdl->GetBound().vMax;
		((MissionShadowCaster *)sender)->AddObject(this, &Character::ShadowReciveDraw, vMin, vMax);
	}
}

void Character::DrawShadowModel(bool recive_pass)
{
	Matrix mtx(true);	

	IGMXScene * mdl;
	
	if (recive_pass)
	{
		mdl = model.scene;
	}
	else
	{
		mdl = shadowModel.scene;
		if(!mdl) mdl = model.scene;
	}

	//Рисуем персонажа
	if(bDraw)		
	{
		physics->GetModelMatrix(mtx);				

		if (bAutoReleaseRagdollOn)
		{		
			Matrix joint_mat = GetBoneMatrix(hips_boneIdx);			

			Matrix mtx2;

			mtx2.pos = -joint_mat.pos;
			mtx2.pos.y = 0;

			Matrix mtx3;			
			mtx3.RotateY(-joint_mat.vy.GetAY()+fRotDlt);

			mtx = mtx2 * mtx3 * mtx;
		}		

		DrawModel( mtx, mdl, true);

		if (recive_pass)
		{
			DrawCoin(0.0f);
		}
	}
}

//Нарисовать модельку персонажа для тени
void _cdecl Character::ShadowDraw(const char * group, MissionObject * sender)
{
	DrawShadowModel(true);

	return;
}

//Нарисовать модельку персонажа для прохода получения тени
void _cdecl Character::ShadowReciveDraw(const char * group, MissionObject * sender)
{
	DrawShadowModel(false);

	return;
}

//Нарисовать модельку персонажа для тени
void _cdecl Character::ReflectionDraw(const char * group, MissionObject * sender)
{
	DrawShadowModel(false);
}


//Нарисовать след за шпагой
void _cdecl Character::DrawTrail(float dltTime, long level)
{
	if(EditMode_IsOn())
	{
		if(!EditMode_IsSelect()) return;
		if(!EditMode_IsVisible()) return;		
	}
	Matrix mtx(true);
	physics->GetModelMatrix(mtx);
	items->DrawTrails(dltTime, mtx);
}

void _cdecl Character::DrawFlares(float dltTime, long level)
{
	if(EditMode_IsOn())
	{		
		if(!EditMode_IsVisible()) return;		
	}
	else
	{
		if (!IsShow()) return;		
		if (!IsActive()) return;
	}
		
	Matrix mtx(true);
	physics->GetModelMatrix(mtx);
	items->DrawFlares(dltTime, mtx);
}

void _cdecl Character::DrawHUD(float dltTime, long level)
{
	if (controller)
	{
		controller->DrawHUD(dltTime);
	}

	if (chrAchievements)
	{
		//chrAchievements->DrawReached(&Render(),dltTime);
	}

}

//Полёт бомбы
void _cdecl Character::BulletFly(float dltTime, long level)
{
	BltTrace.fTime += dltTime;

	if (BltTrace.fTime>BltTrace.fMaxTime)
	{
		DelUpdate(&Character::BulletFly);	

		logic->ShootHit(BltTrace.damage);

		if (logic->shootTargets.Size()>0)
		{
			if (logic->shootTargets[0].target->IsPlayer() &&
				logic->shootTargets[0].target->logic->GetState() == CharacterLogic::state_block)
			{
				if (Rnd()>0.85f)
				{
					logic->shootTargets[0].target->logic->SetShootTarget(this,"Hit");
				}
				else
				{
					logic->shootTargets[0].target->logic->SetShootTarget(NULL,"Hit");
				}
				
				logic->shootTargets[0].target->need_cntl_shoot_call = false;
				logic->shootTargets[0].target->Shoot(10.0f);
			}
		}

		return;
	}


	Vector pos = Vector(0,0,0);	

	pos.Lerp(BltTrace.BulletStart,BltTrace.BulletEnd, BltTrace.fTime/BltTrace.fMaxTime);

	arbiter->DrawBulletTrace(Matrix(Vector(BltTrace.fPitchAngle,BltTrace.fAngle,0.0f),pos),1.0f);	
}

//Нарисовать сферу в режиме редактирования
void _cdecl Character::EditMode_Draw(float dltTime, long level)
{
	if(!EditMode_IsVisible()) return;	

	if (shadowModel.scene)
	{
		Matrix mtx1(true);
		physics->GetMatrixWithBoneOffset(mtx1);		
		
		currentModel = model.scene;

		Vector cam_pos = Render().GetView().GetCamPos();

		if (pattern)
		{
			if ((Render().GetView().GetCamPos()-mtx1.pos).GetLengthXZ2()>pattern->fLODDist)
			{
				currentModel = shadowModel.scene;
			}
		}
	}


	Matrix mtx(true);
	physics->GetModelMatrix(mtx);
	if(currentModel)
	{
		DrawModel(mtx, currentModel);
	}
	else
	{
		mtx.Scale3x3(physics->GetRadius(), physics->GetHeight()*0.5f, physics->GetRadius());
		mtx.pos.y += physics->GetHeight()*0.5f;
		Render().DrawSphere(mtx, 0x4fff0000);
	}
	
	if( pattern && pattern->cHairParams.NotEmpty() )
	{
		bodyparts.Draw(dltTime);
	}	

	/*if(!logic->IsActor())
	{
		if(controller)
		{
			if(!ccp && !aiInit.IsEmpty())
			{
				MissionObject * obj = Mission().FindObject(aiInit);
				if(obj && obj->Is("CharacterControllerParams"))
				{
					ccp = (CharacterControllerParams *)obj;
					if(ccp->IsControllerSupport(controller->Name()))
					{
						controller->SetParams(ccp);
						controller->Reset();
					}else{
						ccp = null;
					}
				}
			}
		}
	}else*/
	if(logic->IsActor())
	{
		if(!actorTimeLine)
		{
			MOSafePointerTypeEx<ActorTimeLine> mo;
			static const ConstString strTypeId("ActorTimeLine");
			mo.FindObject(&Mission(),actorTimeLineName,strTypeId);
			
			if(mo.Ptr() != actorTimeLine)
			{
				actorTimeLine = mo.Ptr();
				ResetActor();
			}
		}
		if(actorTimeLine)
		{
			actorTimeLine->SetDrawCharacter(this);
		/*
			ResetActor();
			physics->SetPos(actorData.pos);
			physics->SetAy(actorData.ay);
		*/
		}
	}	

	if(showCollider)
	{
		physics->GetColliderMatrix(mtx);
		mtx.Scale3x3(physics->GetRadius(), physics->GetHeight()*0.5f, physics->GetRadius());
		Render().DrawSphere(mtx, 0x9fc0c0ff);
		Render().Print(mtx.pos + Vector(0.0f, 1.6f, 0.0f), 10.0f, 1.0f, 0xffffffff, "Id: %s", GetObjectID().c_str());
	}	

	if (bShowCoinED) Render().DrawSphere(mtx.pos+Vector(0,fCointAltetude,0), 0.15f, 0x9900ffff);
}

Matrix & Character::FixMtx(Matrix & mtx)
{
	if (mtx.vx.GetLength2() < 0.9f)
		mtx.SetIdentity3x3();

	return mtx;
}

void Character::BornParticle(const char* LocatorName,const char* ParticleName)
{
	GMXHANDLE loc = model.scene->FindEntity(GMXET_LOCATOR, LocatorName);

	if (loc.isValid())
	{					
		Matrix mtx = model.scene->GetNodeLocalTransform(loc);

		mtx = FixMtx(mtx);

		Matrix mat(true);
		physics->GetModelMatrix(mat);		

		IParticleSystem* pParticale = Particles().CreateParticleSystem(ParticleName);

		if (pParticale)
		{
			pParticale->Teleport(mtx * mat);
			pParticale->AutoDelete(true);
			pParticale->AttachTo(model.scene, loc, true);
			pParticale->Restart(rand());							
		}		
	}

}


bool Character::FindLocator(const char* LocatorName,Vector &pos)
{
	Matrix mat(true);

	if (FindLocator(LocatorName,mat))
	{
		pos = mat.pos;

		return true;
	}	

	return false;
}

bool Character::FindLocator(const char* LocatorName,Matrix &loc_mat)
{
	if(model.scene)
	{
		GMXHANDLE loc = model.scene->FindEntity(GMXET_LOCATOR, LocatorName);

		if (loc.isValid())
		{
			Matrix mtx = model.scene->GetNodeLocalTransform(loc);
			mtx = FixMtx(mtx);

			Matrix mat(true);
			physics->GetModelMatrix(mat);

			loc_mat = mtx * mat;

			return true;
		}
	}	

	return false;
}

// получить матрицу кости
Matrix Character::GetBoneMatrix(long index)
{
	if (index<0 || !animation) return Matrix();

	IAnimation* anim = animation->GetCurAnimation();

	if (anim)
	{
		return anim->GetBoneMatrix(index);	
	}
	
	return Matrix();
}

IAnimation* Character::GetAnimation()
{
	if (animation)
	{
		return animation->GetCurAnimation();
	}

	return null;

};

float Character::GetTimeInFatality()
{
	return fTimeInFatality;
}

void Character::ShowDebugInfo(int type)
{
	/*if (type<0)
	{
		debugInfoType = -1;
		DelUpdate(&Character::DrawDebugInfo);

		return;
	}

	if (debugInfoType<0) debugInfoType = 0;*/
	
	

	int pow = 1;	

	for (int i=0;i<type;i++)
	{
		pow *= 2;
	}

	if (debugInfoType & pow)
	{
		debugInfoType = debugInfoType ^ pow;
	}
	else
	{
		debugInfoType = debugInfoType | pow;
	}	
	
	SetUpdate(&Character::DrawDebugInfo, ML_DEBUG1);	
}

void Character::SetDebugInfo(int type)
{
	debugInfoType = type;

	if (type<0)
	{		
		DelUpdate(&Character::DrawDebugInfo);
	}
	else
	{
		SetUpdate(&Character::DrawDebugInfo, ML_DEBUG1);	
	}
}

void _cdecl Character::DrawDebugInfo(float dltTime)
{
	/*if (IsPlayer())
	{
		logic->AimToTarget(12,PI * 0.5f);
	}

	Render().Print(10,30,0xffffffff,"%4.3f %4.3f %4.3f",physics->GetPos().x,physics->GetPos().y,physics->GetPos().z);
	
	/*if (IsPlayer())
	{
		Render().DrawBox(currentModel->GetLocalBound().vMin,currentModel->GetLocalBound().vMax);

		Vector vMin, vMax;
		
		Matrix mObj;
		GetBox(vMin, vMax);
		GetMatrix(mObj);

		Render().DrawBox(vMin+Vector(0,0.1f,0.0f),vMax+Vector(0,0.1f,0.0f),mObj,0xff0000ff);
		Render().DrawMatrix(mObj);

		Vector min, max;
		Box::FindABBforOBB(mObj, vMin, vMax, min, max, false);

		Render().DrawBox(min,max,Matrix(),0xff00ff00);
	}*/
	
	if (!pattern) return;

	Matrix mtx(true);
	physics->GetColliderMatrix(mtx);
		
	int pow = 1;
	int index = 1;	
	
	Vector pos = mtx.pos + Vector(0.0f, physics->GetHeight() * 0.75f, 0.0f);

	arbiter->PrintTextShadowed(pos, 0, 0xffffffff, "Id: %s", GetObjectID().c_str());
	
	for (int i=0;i<7;i++)
	{
		if (debugInfoType & pow)
		{
			switch (pow)
			{
				case 1:
				{	
					arbiter->PrintTextShadowed(pos, index, 0xffffffff, "Active: %s", IsActive() ? "Yes" : "No");
					arbiter->PrintTextShadowed(pos, index+1, 0xffffffff, "Visible: %s", IsShow() ? "Yes" : "No"); 
															
					if (logic->GetHP()>0)
					{
						arbiter->PrintTextShadowed(pos, index+2, 0xffffffff, "State: Alive");
					}
					else
					{
						if (bDeadConterOn)
						{
							if (IsDead())
								arbiter->PrintTextShadowed(pos, index+2, 0xffffffff, "State: Dead");
							else
								arbiter->PrintTextShadowed(pos, index+2, 0xffffffff, "State: Dying");
						}
						else
						{
							arbiter->PrintTextShadowed(pos, index+2, 0xffffffff, "State: Dead");
						}
					}

					Vector ps = physics->GetPos();

					arbiter->PrintTextShadowed(pos, index+3, 0xffffffff, "Anim State: %s", logic->GetStateString());

					arbiter->PrintTextShadowed(pos, index+4, 0xffffffff,
											   "Position: %3.2f %3.2f %3.2f",
											   ps.x,ps.y,ps.z);

					index+=5;
				}
				break;
				case 2:
				{
					if (animation->GetCurAnimation())
					{
						arbiter->PrintTextShadowed(pos, index, 0xffffffff, "Animation: %s", animation->GetCurAnimation()->GetName());
						arbiter->PrintTextShadowed(pos, index+1, 0xffffffff, "Node: %s", animation->GetCurAnimation()->CurrentNode()); 
						index+=2;
					}
					else
					{
						arbiter->PrintTextShadowed(pos, index, 0xffffffff, "Animation not set");
						index++;
					}
				}
				break;
				case 4:
				{
					if (animation->GetCurAnimation() && pattern)
					{
						arbiter->PrintTextShadowed(pos, index, 0xffffffff, "Dynamic Light: %s", pattern->dynamicLighting ? "yes" : "no");
						arbiter->PrintTextShadowed(pos, index+1, 0xffffffff, "Shadow Cast: %s", pattern->shadowCast ? "yes" : "no"); 
						arbiter->PrintTextShadowed(pos, index+2, 0xffffffff, "Shadow Receive: %s", pattern->shadowReceive ? "yes" : "no");
						arbiter->PrintTextShadowed(pos, index+3, 0xffffffff, "Sea Reflection: %s", pattern->seaReflection ? "yes" : "no"); 

						index+=4;
					}		
					break;
				}
				case 8:
				{
					arbiter->PrintTextShadowed(pos, index, 0xffffffff, "HP: %4.1f", logic->GetHP());
					arbiter->PrintTextShadowed(pos, index+1, 0xffffffff, "Max HP: %4.1f", logic->GetMaxHP());
					arbiter->PrintTextShadowed(pos, index+2, 0xffffffff, "Armor absorb damage: %3.1f of 100", 100.0f - items->CalcDamage(100.0f));

					index+=3;
				}
				break;
				case 16:
				{			
					if (logic->IsActor())
					{
						arbiter->PrintTextShadowed(pos, index, 0xffffffff, "Character is Actor");

						if (actorTimeLine)
						{
							arbiter->PrintTextShadowed(pos, index+1, 0xffffffff, "TimeLine: %s", actorTimeLine->GetObjectID().c_str());
							arbiter->PrintTextShadowed(pos, index+2, 0xffffffff, "Current Node: %4.1f", actorData.currentPosition);
							if (animation->GetCurAnimation())
							{
								arbiter->PrintTextShadowed(pos, index+3, 0xffffffff, "Current Anim Node: %s", animation->GetCurAnimation()->CurrentNode());
							}
							else
							{
								arbiter->PrintTextShadowed(pos, index+3, 0xffffffff, "Actor Animation not set");
							}

							index+=4;
						}
						else
						{
							arbiter->PrintTextShadowed(pos, index+1, 0xffffffff, "TimeLine not set");
							index+=2;
						}
					}	
					else
					{
						arbiter->PrintTextShadowed(pos, index, 0xffffffff, "Character under AI control");
						arbiter->PrintTextShadowed(pos, index+1, 0xffffffff, "AI type: %s", controller->Name());

						index+=2;

						if (!IsPlayer())
						{
							arbiter->PrintTextShadowed(pos, index, 0xffffffff, "AI thought: %s", controller->GetControllerState().c_str());

							if (controller->GetChrTarget())
							{
								arbiter->PrintTextShadowed(pos, index+1, 0xffffffff, "AI target: %s", controller->GetChrTarget()->GetObjectID().c_str());
								arbiter->PrintTextShadowed(pos, index+2, 0xffffffff, "Dist to target: %4.1f", (controller->GetChrTarget()->physics->GetPos()-physics->GetPos()).GetLength());
								index+=3;
							}
							else
							{
								arbiter->PrintTextShadowed(pos, index+1, 0xffffffff, "AI target: absent");
								index+=2;
							}							

							arbiter->PrintTextShadowed(pos, index, 0xffffffff, "Way: %s", controller->GetWayPointsName().c_str());
							arbiter->PrintTextShadowed(pos, index+1, 0xffffffff, "Zone: %s", controller->GetAIZoneName().c_str()); 

							arbiter->PrintTextShadowed(pos, index+2, 0xffffffff, "Dist to Spawn: %4.1f", (logic->GetSpawnPoint()-physics->GetPos()).GetLength()); 

							index+=3;
						}

						int num_attackers = 0;

						const array<Character*> &chars = arbiter->GetActiveCharacters();

						for (int i=0; i< chars; i++)
						{
							if (chars[i]->controller->GetChrTarget() == this)
							{
								num_attackers++;
							}
						}

						arbiter->PrintTextShadowed(pos, index, 0xffffffff, "Num atackers: %i", num_attackers);
						arbiter->PrintTextShadowed(pos, index+1, 0xffffffff, "Num active atackers: %i", controller->GetNumAtackers());

						index+=2;
					}
				}
				break;
				case 32:
				{						
					for (int i=0; i<items->items;i++)
						if (items->items[i].weapon_type != wp_armor && items->items[i].weapon_type != wp_bomb)
						{
							if (items->items[i].trail)
							{
								arbiter->PrintTextShadowed(pos, index, 0xffffffff, "Item %s, attack - %s, trail - %s", items->items[i].id.c_str(), items->items[i].isActive ? "yes" : "no", items->items[i].trail->IsActive() ? "yes" : "no");
							}
							else
							{
								arbiter->PrintTextShadowed(pos, index, 0xffffffff, "Item %s, attack - %s, trail absent", items->items[i].id.c_str(), items->items[i].isActive ? "yes" : "no");
							}

							index++;
						}			
				}
				break;
				case 64:
				{	
					if (logic->IsActor() && IsShow() && IsActive())
					{
						arbiter->PrintTextShadowed(0xffffff00, "Actor : %s",GetObjectID().c_str());		
						if (actorTimeLine)
						{
							arbiter->PrintTextShadowed(0xffffffff, "TimeLine: %s", actorTimeLine->GetObjectID().c_str());
							arbiter->PrintTextShadowed(0xffffffff, "Current Node: %4.1f", actorData.currentPosition);

							if (animation->GetCurAnimation())
							{
								arbiter->PrintTextShadowed(0xffffffff, "Current Anim Node: %s", animation->GetCurAnimation()->CurrentNode());
							}
							else
							{
								arbiter->PrintTextShadowed(0xffffffff, "Actor Animation not set");
							}							
						}
						else
						{
							arbiter->PrintTextShadowed(0xffffffff, "TimeLine: not set");
						}												
					}							
				}
				break;
			}
		}
		
		pow *= 2;
	}	
}

void Character::ShowControllerDebugInfo(bool show)
{
	if (show)
	{
		SetUpdate(&Character::DrawControllerDebugInfo, ML_ALPHA5);
	}
	else
	{				
		DelUpdate(&Character::DrawControllerDebugInfo);
	}
}

void _cdecl Character::DrawControllerDebugInfo(float dltTime)
{
	if (controller)
	{
		controller->ShowDebugInfo();
	}
}

void Character::BloodStain(float dmg, Character * offender)
{
	if (!arbiter->stainManager) return;

	Vector dir;
	dir.Rand();
	dir.y = -fabs(dir.y);

	int dropCount = 1;		// если dmg < 0.0 дропаем 1 штучку крови(из анимаций и т.п.)
	if (dmg >= 0.0f)
	{
		if (offender && offender != arbiter->GetPlayer() && offender->logic)
			dmg *= 1.0f / Max(1.0f, offender->logic->attack_mult);
		
		fBloodDamage += powf(dmg, 0.85f);
		dropCount = int(fBloodDamage / fBloodDropDamage);
		dropCount = Min(5, dropCount);
		fBloodDamage -= dropCount * fBloodDropDamage;
	}

	if (dropCount)
	{
		Vector pos = physics->GetPos(true);
		pos.y -= physics->GetHeight() * 0.5f;

		float d = 0.5f;
		for (int i=0; i<dropCount; i++)
			arbiter->stainManager->AddStain(pos + Vector(RRnd(-d, d), 0.0f, RRnd(-d, d)), dir);
	}
}

void _cdecl Character::DrawCoin(float dltTime)
{	
	if (logic->IsActor()) return;
	if (!IsShow()) return;

	//if (coinModelIndex<0) return;

	// HARD HACK!!!!
	if (logic->GetSide() != CharacterLogic::s_boss)
	{			
		MissionObject* player = Mission().Player();

		if (player)
		{		
			MO_IS_IF(is_Character, "Character", player)
			{
				Character* chr = (Character*)player;

				Vector pos1 = chr->physics->GetPos();
				Vector pos2 = physics->GetPos();
				
				if(fabsf(pos1.y - pos2.y) > 0.3f) return;
			}
		}						
	}
	

	fCoinTime -= dltTime;

	float ang_k = 1.0f;
	
	if (FATALITY_TIME - fTimeInFatality<2.0f)
	{
		ang_k = 2.0f - (FATALITY_TIME - fTimeInFatality) + 1.0f;
	}

	fCoinAngle += dltTime * 7.0f * ang_k;

	coinModelIndex = -1;

	if (fCoinTime<0.0f)
	{
		fCoinTime = 0.0f;
		//fCoinAngle = 0.0f;

		if (CharCoin != chrcoin_none)
		{
			coinModelIndex = CharCoin;
		}
	}
	else
	if (fCoinTime<CoinChangeTime)
	{
		//fCoinAngle = -fCoinTime/CoinChangeTime * PI/2;

		if (CharCoin != chrcoin_none)
		{
			coinModelIndex = CharCoin;
		}
	}
	else		
	{
		//fCoinAngle = (CoinChangeTime * 2.0f - fCoinTime)/CoinChangeTime * PI/2;

		if (LastCharCoin != chrcoin_none)
		{
			coinModelIndex = LastCharCoin;
		}
	}

	fCoindDltHgt += dltTime * 2.0f;

	if (fCoindDltHgt > PI * 2) fCoindDltHgt -= PI * 2;



	if (fTimeInFatality > FATALITY_TIME && coinModelIndex == 4) return;

	// Test Link
	{
		const char * charName = "blood";		

		const char * victimName = logic->GetCharName();

		if (!victimName || !victimName[0])
		{
			SetCharacterCoin(chrcoin_none);			

			return;
		}

		FatalityParams::TFatalityType type;
		const char * topairLink = logic->GetToPairLink(type);

		char pairLink[128];

		if (!topairLink || !topairLink[0])
		{				
			crt_snprintf(pairLink, 127, "%s %s pair",charName,victimName);
		}
		else
		{
			if (string::IsEmpty(topairLink))
			{
				crt_snprintf(pairLink,127, "%s %s pair",charName,victimName);
			}
			else
			{					
				crt_snprintf(pairLink,127,"%s %s pair %s",charName,victimName,topairLink);
			}
		}			

		if(!animation->TestActivateLink(pairLink))
		{
			///SetCharacterCoin(chrcoin_none);			

			return;
		}
	}



	Matrix mInvView = Render().GetView();

	mInvView.Inverse();

	Vector dir(0.0,0.0,-1.0f);

	mInvView.pos = 0.0f;		

	dir = dir * mInvView;

	dir.y=0;
	dir.Normalize();

	Vector up(0.0,0.0,1.0f);

	float angle = up.GetAngleXZ(dir) + fCoinAngle;

	Matrix mtx;

	mtx.RotateY(angle);

	Matrix mtx1(true);
	physics->GetMatrixWithBoneOffset(mtx1);

	mtx.pos = mtx1.pos + Vector(0.0f, fCointAltetude - 1.0f + cos (fCoindDltHgt) *0.105f,0.0f);

	float a = 1.0f;

	if (FATALITY_TIME - fTimeInFatality<2.0f)
	{
		a = (FATALITY_TIME - fTimeInFatality) / 2.0f;
	}

	if (a>0.25f) arbiter->DrawCoin(coinModelIndex,mtx, a);	
}

void Character::SetCharacterCoin(TCharacterCoin coin)
{
	if (isPlayer) return;

	if (logic->GetSide() == CharacterLogic::s_ally)
	{
		coin = chrcoin_none;
	}

	if (IsDead())
	{
		coin = chrcoin_none;
	}
		
	if (coin == chrcoin_fatality)
	{
		if (logic->GetHP()>0)
		{		
			fTimeInFatality = 0.0f;
		}
		else
		{
			coin = chrcoin_none;
		}
	}


	if (coin == CharCoin) return;	

	if (CharCoin == chrcoin_none)
	{
		fCoinTime = CoinChangeTime;		
	}
	else
	{
		fCoinTime = CoinChangeTime * 2.0f;
	}

	LastCharCoin = CharCoin;
	CharCoin = coin;	

	if (CharCoin == chrcoin_none)
	{	
		fTimeInFatality = FATALITY_TIME + 0.1f;
	}	
}

//Нарисовать модельку и навеску
inline void Character::DrawModel(const Matrix & mtx, IGMXScene * mdl, bool drawCoin, const Color * userColor)
{
	if(!mdl) return;
	if(!userColor)
	{
		static Color empty(0.0f, 0.0f, 0.0f, 1.0f);
		userColor = &empty;
	}

	Matrix mat = mtx;
		
	items->Draw(mat, *userColor);			

#ifdef CHARACTER_STARFORCE
	AssertStarForce(fabsf(curScaleSpike - 1.0f) < 1e-5f);
	mdl->SetTransform(Matrix().BuildScale(curScaleSpike) * mat);	
#else
	mdl->SetTransform(mat);	
#endif

	mdl->SetUserColor(*userColor);
	mdl->Draw();	

	if (armor)
	{
		armor->Draw(mat);
	}
	

	//fCoinAngle = 0.0f;
	//coinModelIndex = 1;

	if (coinModelIndex != -1 && drawCoin)
	{				
		if (!logic->PairModeAllowed(true,true))
		{
			SetCharacterCoin(chrcoin_none);			

			return;
		}

		// Test Link
		/*{
			const char * charName = "blood";		

			const char * victimName = logic->GetCharName();

			if (!victimName || !victimName[0])
			{
				SetCharacterCoin(chrcoin_none);			

				return;
			}
	
			FatalityParams::TFatalityType type;
			const char * topairLink = logic->GetToPairLink(type);

			char pairLink[128];

			if (!topairLink || !topairLink[0])
			{				
				crt_snprintf(pairLink, 127, "%s %s pair",charName,victimName);
			}
			else
			{
				if (string::IsEmpty(topairLink))
				{
					crt_snprintf(pairLink,127, "%s %s pair",charName,victimName);
				}
				else
				{					
					crt_snprintf(pairLink,127,"%s %s pair %s",charName,victimName,topairLink);
				}
			}			
		
			if(!animation->TestActivateLink(pairLink))
			{
				///SetCharacterCoin(chrcoin_none);			

				return;
			}
		}

		{			
			MissionObject* player = Mission().Player();

			if (player)
			{
				Matrix mat;
				player->GetMatrix(mat);

				if(fabsf(mat.pos.y - physics->GetPos().y) > 0.3f) return;
			}						
		}*/
	}
}

//Счётчик лежания трупа
void _cdecl Character::DeadTimer(float dltTime, long level)
{
	deadBodyTime += dltTime;

	if (logic->is_attached>0)
	{		
		return;
	}	

	if (logic->slave)
	{
		logic->slave->physics->SetPos(physics->GetPos());
		logic->slave->physics->SetAy(physics->GetAy());
		logic->slave->physics->SetAz(physics->GetAz());
	}


	if (physics->IsActive())
	{
		if (logic->GetState() == CharacterLogic::state_dead)
		{
			physics->Activate(false);
		}
	}

	if (!bVanishBody)
	{
		Matrix mtx(true);
		physics->GetMatrixWithBoneOffset(mtx);		

		const Plane* frustum = Render().GetFrustum();

		isDeadBodyInFrustum = arbiter->IsSphereInFrustrum(mtx.pos, 1.1f);
		/*if (isDeadBodyInFrustum)
			return;
		else
			deadTime = GetDeathTime();*/
		return;
	}

	deadTime += dltTime;		
	
	if (deadTime >= GetDeathTime())
	{			
		VanishBody(dltTime);
	}
}

void Character::VanishBody(float dltTime)
{
	if (!bVanishBody)
		arbiter->DelDeadBody(*this);

	api->Trace("VanishBody = %s", GetObjectID().c_str());

	// Vano: поганый хак, для того чтобы персонаж считался мертвым
	deadTime = GetDeathTime() + 1.0f;

	statDismembered = false;
	bDeadConterOn = false;
	ActivateRagdoll(false, 0.0f);

	DelUpdate(&Character::DeadTimer);			

	ReleaseFlyes();

	static const ConstString chaimanId("Chainman");
	if (!IsPlayer() && (logic->GetSide()!=CharacterLogic::s_boss || aiType == chaimanId)) 
	{
		if(animation)
		{
			animation->Start();
		}

		Show(false);
		Activate(false);
	}
	else
	{
		//hack!!!!
		logic->Update(dltTime);

		DelUpdate(&Character::Work);
		DelUpdate(&Character::UpdatePhysics);
	}

	if (ragdoll)
	{		
		Matrix boneMatrix(true);
		physics->GetModelMatrix(boneMatrix);
		physics->SetPos(boneMatrix.pos);


		ragdoll->Release();
		ragdoll = NULL;

		bAutoReleaseRagdollOn = false;
		fTimeToReleaseRagDoll = 0.0f;
		
		physics->GetModelMatrix(boneMatrix);
		model.scene->SetTransform(boneMatrix);
	}

	for (dword i=0;i<BodyParts.Size();i++)
	{
		if (BodyParts[i].pPhysBox)
		{
			BodyParts[i].pPhysBox->Release();
			BodyParts[i].pPhysBox=NULL;
		}

		if (BodyParts[i].pPhysCapsule)
		{
			BodyParts[i].pPhysCapsule->Release();
			BodyParts[i].pPhysCapsule=NULL;
		}

		if (BodyParts[i].pParticale)
		{
			BodyParts[i].pParticale->Release();
			BodyParts[i].pParticale=NULL;
		}
	}		
}

void Character::DrawCharacterInPosition(const Matrix & mtx, const Color * userColor)
{
	IGMXScene* mdl = model.scene;
	
	Vector cam_pos = Render().GetView().GetCamPos();
	
	if (pattern)
	{
		if ((cam_pos-physics->GetPos()).GetLengthXZ2()>pattern->fLODDist)
		{
			mdl = shadowModel.scene;
		}
	}

	DrawModel(mtx, mdl, false, userColor);
}

//Установить цель для бомбы
void Character::SetBombTarget(const Vector & boomPosition)
{
	if(bombTime < 0.0f)
	{
		bombEnd = boomPosition;
		bombTime = -10.0f;
	}
}

//Состояние бомбы
bool Character::IsBombBusy()
{
	return bombTime >= 0.0f;
}

//Бросить бомбу
void Character::DropBomb(float bomblietime)
{		
	if(bombTime > -5.0f)
	{
		bombLieTime = 0.0f;

		bombTime = -1.0f;
		DelUpdate(&Character::FlyBomb);

		if(controller)
		{
			controller->Boom(Vector(bomb.pos.x, bomb.pos.y - 0.1f, bomb.pos.z));
		}
	}

	bombLieTime = bomblietime;

	//Vector bombStart;	

	//Ищем локатор для бомбы
	bool isSet = false;
	GMXHANDLE loc = model.scene->FindEntity(GMXET_LOCATOR, "lefthand");
	if (loc.isValid())
	{
		bombStart = model.scene->GetNodeLocalTransform(loc).pos;
		bombStart = bombStart * GetMatrix(Matrix());
		isSet = true;
	}
	if(!isSet)
	{
		bombStart = physics->GetPos() + Vector(0.0f, 1.0f, 0.0f);
	}	

	bombTime = 0.0f;
	angles = 0.0f;
	anglesRot.Rand(Vector(-1.0f), Vector(1.0f));
	SetUpdate(&Character::FlyBomb, ML_ALPHA1);
	flySpeed = (bombEnd - bombStart).GetLength();
	if(flySpeed > 1e-3f)
	{
		flySpeed = GetBombSpeed()/flySpeed;
	}else{
		flySpeed = 1000.0f;
		bombTime = 1.0f;
	}

	bomb.dltTime = 0;
	bomb.isSleep = false;
	bomb.pos = bombStart;
	bomb.radius = 0.1f;
	Vector dir = (bombEnd - bombStart);
	float dist = dir.Normalize();
	dir *= dist * 3.33f;
	dir.y = dist * 0.45f;	

	bomb.velocity = dir;
}

//Высстрелить
void Character::ShootTargets(float dmg,const char* hit_react,bool flyAway,float fly_dist,float damage_radius, bool damage_once)
{		
	damage_radius *= damage_radius;
	fly_dist *= fly_dist;

	logic->SetShootTarget(null,hit_react);

	array<CharacterFind> & find = arbiter->find;

	if (damage_once)
	{		
		CharacterFind fnd;
		fnd.chr = logic->FindTarget(15.55f, PI*0.3f);

		find.DelAll();

		if (fnd.chr)
			find.Add(fnd);
	}
	else
	{	
		arbiter->Find(*this, 0.05f, 15.55f, true, -PI*0.3f, PI*0.3f, 0.5f, 2.5f);	
	}
		
	Character* trg = null;	

	for(long i = 0; i < find; i++)
	{		
		if(!find[i].chr->logic->IsEnemy(this)) continue;

		if(find[i].chr->logic->IsPairMode()) continue;

		if(find[i].chr->ragdoll) continue;

		if(find[i].chr->logic->IsDead()) continue;

		{			
			trg = find[i].chr;

			Matrix mat(true);	
			trg->GetMatrix(mat);			

			float dist = (mat.pos - physics->GetPos()).GetLengthXZ2();			

			if (trg)
			{			
				trg->logic->SetOffender(this);

				if (dist<damage_radius)
				{
					float fDamageKoef = 1.0f;
					bool fly = flyAway;

					if (trg->logic->GetSide() == CharacterLogic::s_boss)
					{
						fly = true;
					}

					/*if (dist>fly_dist)
					{
						fDamageKoef = 1.0f;
						fly = false;
					}*/
					
					logic->SetShootTarget(trg,hit_react,fDamageKoef,fly, true);

					if (trg)
					{
						MO_IS_IF(is_Character, "Character", trg)
						{
							Character* chr = (Character*)trg;

							chr->SetCharacterCoin(Character::chrcoin_none);
						}
					}
				}
			}
		}
	}

	Shoot(dmg);
}

//Высстрелить
void Character::Shoot(float dmg)
{
	DelUpdate(&Character::BulletFly);

	if (controller && need_cntl_shoot_call)
	{
		controller->Shoot(dmg);
	}
	
	need_cntl_shoot_call = true;

	if (logic->shootTargets.Size()>1 || (IsPlayer() && logic->GetState() != CharacterLogic::state_block))
	{
		logic->ShootHit(dmg);
		return;
	}

	Matrix mtx(true);

	if (IsPlayer() && logic->GetState() == CharacterLogic::state_block)
	{
		if (items->FindLocatorinItems("flare",mtx))
		{
			BltTrace.BulletStart = mtx.pos;
		}
		else
		{
			BltTrace.BulletStart = physics->GetPos()+Vector(0.0f,1.2f,0.0f);
		}
	}
	else
	if (items->FindLocatorinItems("fire",mtx))
	{
		BltTrace.BulletStart = mtx.pos;
	}		
	else
	{		
		GMXHANDLE loc = model.scene->FindEntity(GMXET_LOCATOR, "gun_fire");

		if (loc.isValid())
		{				
			model.scene->GetNodeWorldTransform(loc, mtx);

			BltTrace.BulletStart = mtx.pos;
		}
		else
		{		
			BltTrace.BulletStart = physics->GetPos()+Vector(0.0f,1.2f,0.0f);
		}
	}	

	if (logic->shootTargets.Size()>0)
	{	
		BltTrace.BulletEnd = logic->shootTargets[0].target->physics->GetPos()+Vector(0.0f,1.2f,0.0f);		
	}
	else
	{							
		Vector dr = 0.0f;
		
		float angle = physics->GetAy();

		if (IsPlayer() && logic->GetState() == CharacterLogic::state_block)
		{
			Vector dir = 0.0f;
			float v_angle = RRnd(0,PI * 0.75f);
			
			dir.z = cos(v_angle);
			dir.y = sin(v_angle);

			dir.Rotate(angle);
			dir.Rotate(Rnd() * PI * 0.8f);

			BltTrace.BulletEnd = BltTrace.BulletStart + (Rnd(5.0f)+15.0f)*dir;
		}
		else		
		{						
			BltTrace.BulletEnd = BltTrace.BulletStart + (Rnd(5.0f)+15.0f)*Vector(sin(angle),0,cos(angle)) + dr;
		}		
	}		

	Vector delta = (BltTrace.BulletEnd - BltTrace.BulletStart);
	float dst = delta.Normalize();
	dst -= BltTrace.scaleH;

	if (dst<0)	
	{
		logic->ShootHit(dmg);
		return;
	}

	BltTrace.BulletEnd = BltTrace.BulletStart + delta * dst;

	BltTrace.fTime = 0.0f;
	BltTrace.fMaxTime = ((BltTrace.BulletStart - BltTrace.BulletEnd).GetLength())/BltTrace.fSpeed;		

	BltTrace.damage = dmg;

	SetUpdate(&Character::BulletFly, ML_DYNAMIC5);
		

	Vector vZAxis = Vector (0.0f, 0.0f, 1.0f);
	Vector vLookTo = BltTrace.BulletEnd - BltTrace.BulletStart;
	BltTrace.fAngle = vZAxis.GetAngleXZ(vLookTo);

	BltTrace.fPitchAngle = -atan(vLookTo.y/(sqrt(vLookTo.z*vLookTo.z+vLookTo.x*vLookTo.x)));	
}

// растолкать соседних чаров
void Character::PushCharcters(float dltTime,float fRadius, bool isCircleSearch, bool only_ally)
{
	if (isCircleSearch)
	{
		arbiter->FindCircle(*this, fRadius);
	}
	else
	{	
		arbiter->Find(*this, 0.2f, fRadius, false, -PI*0.5f, PI*0.5f, 0);
	}	

	Matrix mtx = physics->GetMatrixWithBoneOffset(mtx);		
	Vector pos = mtx.pos;

	if(arbiter->find > 0)
	{			
		array<CharacterFind> & find = arbiter->find;

		for(long i = 0; i < find; i++)
		{			
			if (find[i].chr->logic->IsPairMode()) continue;

			if (only_ally && find[i].chr->logic->GetSide()!=CharacterLogic::s_npc)
			{
				if (find[i].chr->logic->IsEnemy(this)) continue;
			}
			
			Vector vDir = find[i].dv;			
			float dist = vDir.Normalize();
							
			vDir*=dltTime*2.75f*(fRadius-dist);

			find[i].chr->physics->Move(vDir);			
		}
	}
}

void Character::GetPosFromRootBone(Vector& pos)
{
	Matrix mtx(true);
	physics->GetModelMatrix(mtx);
	mtx.pos = last_pos;

	//Моделька
	IGMXScene * mdl = shadowModel.scene;
	if(!mdl) mdl = model.scene;
	if(!mdl) return;

	if (bAutoReleaseRagdollOn)
	{
		Matrix joint_mat = GetBoneMatrix(hips_boneIdx);

		Matrix mtx2(true);
		
		mtx2.pos = -joint_mat.pos;
		mtx2.pos.y = 0;

		Matrix mtx3;
		mtx3.RotateY(-joint_mat.vy.GetAY()+fRotDlt);

		mtx = mtx2 * mtx3 * mtx;
	}
			
	mdl->SetTransform(mtx);	

	Matrix mat;
	physics->GetMatrixWithBoneOffset(mat);
	mtx.pos = last_bone_pos;

	//pos.x = (mdl->GetBound().vMin.x + mdl->GetBound().vMin.x) * 0.5f;
	pos.x = mtx.pos.x;
	pos.y = mtx.pos.y-0.8f;//mdl->GetBound().vMin.y;
	//pos.z = (mdl->GetBound().vMin.z + mdl->GetBound().vMin.z) * 0.5f;
	pos.z = mtx.pos.z;

	physics->SetAy(last_bone_ay);
}

//Переинициализировать состояние актёра
void Character::DropItems()
{
	if (items)
	{
		items->DropItems();
	}
}

//Переинициализировать состояние актёра
void Character::DropItem(const ConstString & item_id)
{
	if (items)
	{
		items->DropItem(item_id,false);
	}
}

//Переинициализировать состояние актёра
void Character::ResetActor()
{
	actorData.playerPosition = physics->GetPos();
	actorData.playerAngle = physics->GetAy();
	if(actorTimeLine)
	{
		Matrix mt;
		physics->GetModelMatrix(mt);
		actorTimeLine->Reset(actorData,mt);
	}
}

//Обновить состояние актёра
void Character::UpdateActor(float dltTime)
{
	{
		Matrix mt;

		physics->GetModelMatrix(mt);
		last_pos = mt.pos;

		physics->GetMatrixWithBoneOffset(mt);
		last_bone_pos = mt.pos;

		last_bone_ay = mt.vz.GetAY();

		//Render().Print(10,10,0xff00ff00,"%f %f %f",mt.pos.x,mt.pos.y,mt.pos.z);
	}

	if(!actorTimeLine) return;	

	if(!IsActive())// || EditMode_IsOn())
	{
		return;
	}

	actorTimeLine->Move(actorData, dltTime);
	
	if (!logic->IsActor() && bActorMoveColider) return;
	

	physics->SetPos(actorData.pos);
	physics->SetAy(actorData.ay);

	if (actor_attach)
	{		
		Matrix mat;
		actor_attach->GetMatrix(mat);

		Matrix mat2;
		physics->GetModelMatrix(mat2);

		mat = mat2 * mat;

		physics->SetPos(mat.pos);
		physics->SetAy(mat.vz.GetAY());		
	}	
}

//Отложеная установка актёра после инициализации
void _cdecl Character::PostInitSetActor(float dltTime, long level)
{
	const char * tmp[1];
	tmp[0] = "";
	Command("enableactor", 0, tmp);
	DelUpdate(&Character::PostInitSetActor);
}

//Активировать/деактивировать рэгдол
void Character::ActivateRagdoll(bool isAct, float blendTime, float ImpulsePow, bool autorelease_ragdoll)
{
	if (logic->is_attached>0 && isAct)
	{		
		return;
	}

	if (isAct && IsDead()) return;	

	if(ragdoll == NULL && isAct == true)
	{
		if (!EditMode_IsOn())
		{			
			blendTime = 0.01f;

			fTimeToStandUp = RRnd(3.0f, 10.0f);
			fTotalTimeToStandUp = 5.0f;
			
			physics->Activate(false, null, true);

			if (logic->GetHP()>0.0f &&				
				logic->GetState() != CharacterLogic::state_die &&
				logic->GetState() != CharacterLogic::state_dead &&
				!logic->IsPairMode())
			{
				animation->Goto("Lie",0.2f);
			}
			else
			{
				animation->ActivateLink("Lie",true);
			}

			if (pattern->rdData)
			{
				ragdoll = Physics().CreateRagdoll(_FL_, pattern->rdData->Buffer(), pattern->rdData->Size());
				if (ragdoll)
				{
					ragdoll->SetUserNumber(666);
					ragdoll->SetUserPointer(this);
				}
			}

			if (physics->IsUseFakeModelPosition())
			{
				physics->SetUseFakeModelPosition(false);
			}

			if (logic->IsActor())
			{
				if(ragdoll && actorData.ani)
				{
					Vector dltPos(0.0f);
					float dltAng = 0.0f;

					dltPos = physics->dltPosSpeed * 5.0f;

					float speed = dltPos.Normalize();
					speed *= ImpulsePow;
					speed = Clampf(speed,0.0f,10.0f);
					dltPos *= speed;

					dltPos = Vector(0,2.0f,4.0f);					

					ragdoll->SetBlendStage(actorData.ani, aminationBlendLevel_ragdoll);					
					ragdoll->Activate(blendTime, physics->GetModelMatrix(Matrix()));
					
					Matrix mat(true);
					physics->GetModelMatrix(mat);

					mat.pos = Vector(0,0,0);

					dltPos.y += Rnd(0.2f);

					dltPos = dltPos * mat;

					float fPow = Rnd(0.6f) + 0.4f;

					ragdoll->ApplyImpulse( dltPos * fPow);
				}
			}
			else
			{
				if(ragdoll && animation->GetCurAnimation())
				{
					ragdoll->SetBlendStage(animation->GetCurAnimation(), aminationBlendLevel_ragdoll);					
					ragdoll->Activate(blendTime, physics->GetModelMatrix(Matrix()));					

					if (physics->vFlyDir.x==0&&
						physics->vFlyDir.y==0&&
						physics->vFlyDir.z==0)
					{
						Vector dltPos(0.0f);
						float dltAng = 0.0f;
					
						dltPos = physics->dltPosSpeed * 5.0f;
						
						float speed = dltPos.Normalize();
						speed *= ImpulsePow;
						speed = Clampf(speed,0.0f,10.0f);
						dltPos *= speed;

						Matrix mat(true);
						physics->GetModelMatrix(mat);

						mat.pos = Vector(0,0,0);

						dltPos.y += Rnd(0.2f);

						dltPos = dltPos * mat;

						float fPow = Rnd(0.6f) + 0.4f;

						ragdoll->ApplyImpulse( dltPos * fPow );
					}
					else
					{
						Vector dltPos(0.0f);						
						dltPos = physics->vFlyDir * 5.0f;

						float speed = dltPos.Normalize();
						speed *= ImpulsePow;
						//speed = Clampf(speed,0.0f,10.0f);
						dltPos *= speed;

						float fPow = Rnd(0.8f) + 0.5f;

						dltPos *= fPow;

						float pw = dltPos.Normalize();
						pw = Clampf(pw,0.0f,15.0f);

						ragdoll->ApplyImpulse( dltPos * pw);

						physics->vFlyDir = 0.0f;
					}

					SetCharacterCoin(Character::chrcoin_none);
				}
			}

			if (!logic->IsActor())
			{
				Matrix mtx1(true);
				physics->GetMatrixWithBoneOffset(mtx1);				

				vLastRagdollPos = mtx1.pos;
			}			
		}
		else
		{
			ragdoll = NULL;
		}
	}

	if (ragdoll && isAct == false)
	{
		Matrix mtx1(true);
		physics->GetMatrixWithBoneOffset(mtx1);		


		IPhysicsScene::RaycastResult pRayCastRes;

		IPhysBase* pRayCastedBody=NULL;

		pRayCastedBody = Physics().Raycast(mtx1.pos,
										   mtx1.pos+Vector(0,-5.0f,0),
										   phys_mask(phys_world),
										   &pRayCastRes);

		if (pRayCastedBody)
		{
			mtx1.pos+=Vector(0,-pRayCastRes.distance+0.01f,0);
		}

		Matrix moveTo(true);
		
		blendTime = 0.15f;		

		physics->GetModelMatrix(moveTo);

		SetCharacterCoin(Character::chrcoin_none);

		ragdoll->Deactivate(blendTime, mtx1 );

		new_pos = mtx1.pos;

		//physics->SetPos(new_pos);
		physics->Activate(true, &new_pos, true);

		Matrix joint_mat = GetBoneMatrix(hips_boneIdx);		

		mtx1 = joint_mat * moveTo;
		//physics->SetAy(mtx1.vy.GetAY());		

		set_new_ay = true;
		new_ay = mtx1.vy.GetAY();

		bAutoReleaseRagdollOn = true;
		fTimeToReleaseRagDoll = blendTime;
		//fTimeToReleaseRagDoll = 0.75f;

		
		if (!autorelease_ragdoll)
		{
			bAutoReleaseRagdollOn = false;
			fTimeToReleaseRagDoll = 0.0f;
			ragdoll->Release();
			ragdoll = NULL;
		}

		bSelfDestructRagdoll = false;
	}
}

void Character::ArmRagdoll()
{
	if (!ragdoll) return;

	bSelfDestructRagdoll = true;

	Matrix mtx1(true);
	physics->GetModelMatrix(mtx1);

	Matrix joint_mat = GetBoneMatrix(spine_boneIdx);		

	mtx1 = joint_mat * mtx1;

	rd_last_pos = mtx1.pos;		
	rd_last_pos2 = rd_last_pos;
	rd_last_speed = 0.0f;

	// VANO: поменял с 0.15 на -0.15, потому что часто за 0.15 секунд труп успевает улететь далеко от врагов и никого не взрывает
	rd_last_timeout = -0.15f;

	rd_last_dir = 0.0f;

	rd_inited = false;
}

void Character::FlyBodyPart(const ConstString & ID ,const ConstString & ParticalName, float fImpulseVertPow, float fImpulseHorzPow, float torque,const char * bone)
{
	if (fImpulseVertPow==0) fImpulseVertPow = 1.0f;
	if (fImpulseHorzPow==0) fImpulseHorzPow = 1.0f;

	static const ConstString headId("Head");
	static const ConstString helemId("helmet");
	static const ConstString emptyId("");

	bool isHead = (ID == headId);

	//Vano: добавил выброс шляпы, а то она часто оставалась прибитой
	if (isHead && (!logic || !logic->IsActor()))
	{
		if (!items->DropItem(helemId, false))
			if (!items->HideItem(helemId))
			{
				api->Trace("ERROR: Can't drop or hide HELMET item on character \"%s\"", GetObjectID().c_str());

				// скрываем все пустые 
				items->HideItem(emptyId);
			}
	}

	for (int i=0;i<(int)BodyParts.Size();i++)
	{
		if (BodyParts[i].ID != ID) continue;
	
		if (BodyParts[i].pPhysBox || BodyParts[i].pPhysCapsule) break;	
		
		Matrix mtx;

		if (BodyParts[i].PartModel.scene)
		{
			//AABB
			const GMXBoundBox & volume = BodyParts[i].PartModel.scene->GetLocalBound();
			Vector size = volume.vMax - volume.vMin;

			if(size.x >= 0.0001f && size.y >= 0.0001f && size.z >= 0.0001f)
			{
				GMXHANDLE loc = model.scene->FindEntity(GMXET_LOCATOR, BodyParts[i].LocatorName.c_str());

				if (!loc.isValid())
				{
					LogicDebugError("BodyPart: Cant find locator - %s in %s",BodyParts[i].LocatorName.c_str(),GetObjectID().c_str());
					return;
				}

				if (loc.isValid())
				{
					mtx = model.scene->GetNodeLocalTransform(loc);
					mtx = FixMtx(mtx);

					Vector center = volume.vMin + (size * 0.51f);

					Matrix mt;					
					mt.pos=center;

					Matrix mat(true);
					physics->GetModelMatrix(mat);

					Matrix world(mtx, mat);

					world = world * mt;					

					float radius = (size.x+size.z) * 0.25f;
					float height = size.y-radius*2;

					if (height<0) height = 0.01f;

					Matrix rot(true);
					rot.BuildRotateX(PI/2);
					
					if (BodyParts[i].isBox)
					{
						BodyParts[i].pPhysBox = Physics().CreateBox(_FL_, volume.vMax-volume.vMin, world, true);
					}
					else
					{
						BodyParts[i].pPhysCapsule = Physics().CreateCapsule(_FL_, radius, height, rot * world, true);
					}					

					float mass = 0.25f;
					
					if (BodyParts[i].pPhysBox || BodyParts[i].pPhysCapsule)
					{
						if (BodyParts[i].pPhysBox)
						{
							BodyParts[i].pPhysBox->SetMass(mass);
							BodyParts[i].pPhysBox->SetGroup(phys_charitems);
						}
						
						if (BodyParts[i].pPhysCapsule)
						{
							BodyParts[i].pPhysCapsule->SetMass(mass);
							BodyParts[i].pPhysCapsule->SetGroup(phys_charitems);
						}
						
						//float impulsePower = RRnd(-1.0f, 1.0f);

						Vector impulse;

						impulse.Rand();
						impulse.y =0.0f;
						impulse.Normalize();


						impulse.x *= fImpulseHorzPow;
						impulse.y = 0.5f+fImpulseVertPow;
						impulse.z *= fImpulseHorzPow;
												
						Vector Offset = 0.0f;//-size * 0.5f;// + size * Rnd();						
						//Vector torque = 150.0f;

						if (BodyParts[i].pPhysBox)
						{
							BodyParts[i].pPhysBox->ApplyImpulse( impulse, Offset);
							BodyParts[i].pPhysBox->ApplyLocalTorque(torque);
							BodyParts[i].pPhysBox->SetCenterMass(Offset * 0.25f);
						}
							
						if (BodyParts[i].pPhysCapsule)
						{
							BodyParts[i].pPhysCapsule->ApplyImpulse( impulse, Offset);
							BodyParts[i].pPhysCapsule->ApplyLocalTorque(torque);
							BodyParts[i].pPhysCapsule->SetCenterMass(Offset * 0.25f);
						}

						IGMXScene * partScene = BodyParts[i].PartModel.scene;
						GMXHANDLE loc = partScene->FindEntity(GMXET_LOCATOR, "blood");
					
						if (loc.isValid())
						{
							BodyParts[i].pParticale = Particles().CreateParticleSystem(ParticalName.c_str());
							
							if (BodyParts[i].pParticale)
							{
								BodyParts[i].pParticale->Teleport(partScene->GetNodeLocalTransform(loc) * world);
								BodyParts[i].pParticale->AutoDelete(false);
								BodyParts[i].pParticale->AttachTo(partScene, loc, true);
								BodyParts[i].pParticale->Restart(rand());							
							}
						}

						if (bone)
						{
							const char* params[2];
							params[0] = "c";
							params[1] = bone;

							events->BoneCollapse(animation->GetCurAnimation(),"colapse",params,2);								
						}

						/*//Vano: добавил выброс шляпы, а то она часто оставалась прибитой
						if (!logic || !logic->IsActor())
						{
							static const ConstString helemId("helmet");
							items->DropItem(helemId, false);
						}*/
					}

					Character* offender = logic->GetOffender();

					if (offender && offender->IsPlayer() && logic->IsRealEnemy(offender, false))
					{
						if (!statDismembered)
						{
							statDismembered = true;
							arbiter->StatsDismemberAdd();

							ACHIEVEMENT_REACHED(offender, RIPPER);

							if (offender->chrAchievements && isHead)
								offender->chrAchievements->CountHead();
						}
					}
					
					return;
				}
			}
		}
	}

	LogicDebugError("BodyPart: Cant find part - %s in %s",ID.c_str(),GetObjectID().c_str());	
}

//============================================================================================
//Элементы инициализации
//============================================================================================

void _cdecl Character::EditMode_Work(float dltTime, long level)
{		
	bool need_update = false;
	
	if (!pattern_ptr.Validate())				
	{
		static const ConstString strTypeId("CharacterPattern");
		if (!pattern_ptr.FindObject(&Mission(),pattern_name,strTypeId))
		{
			if (pattern)
			{
				pattern = null;
				pattern_ptr.Reset();
				need_update = true;
			}			
		}
		else
		{
			need_update = true;
			pattern = pattern_ptr.Ptr();			
		}		
	}
	else
	{
		need_update = pattern->IsNeedUpdate();
	}

	if (need_update)
	{
		Release();
		
		if (!InitFromPattern())
		{
			InitDefault();
		}
		
		physics->SetPos(init_pos);
		physics->SetAy(init_ay);

		InitCharData();		

		if (!shadowModel.scene && pattern)
		{		
			api->Trace ("Shadow Model for %s not set ",GetObjectID().c_str());
		}
	}

	CheckRenderState();

	
	if (EditMode_IsOn() && init_actrorPreview && actorTimeLine)
	{	
		UpdateActor(dltTime);

		if (actorTimeLine->IsTimeLineFinished(actorData))
		{
			animation->SetAnimation("actor",0.0f);

			ResetActor();
			UpdateActor(0);
		}		
	}	
}


//Базовая инициализация
bool Character::Init(MOPReader & reader)
{
	Registry(MG_CHARACTER);

	// Создаем арбитер
	if(!SetArbiter())
	{
		LogicDebugError("Character not created, CharacterArbiter not found");
		return false;
	}

	pattern = null;
	pattern_name = reader.String();
	
	init_pos = reader.Position();
	init_ay = reader.Angles().y;	

	last_frame_pos = init_pos;

	auto_spawnpoint = reader.Bool();
	auto_spawnpoint_saved = auto_spawnpoint;
	init_spawnpoint = reader.Position();

	init_actorani = reader.String().c_str();
	init_actorTimeLineName = reader.String();
	init_actrorPreview = reader.Bool();
	init_EnableAniBlend = reader.Bool();
	init_ColideWhenActor = reader.Bool();	
	init_bActorMoveColider = reader.Bool();	
	useRootBone = reader.Bool();

	fDistShadow = reader.Float();
	init_aiInit = reader.String();

	if (chrInitAI)
	{
		delete chrInitAI;
		chrInitAI = null;
	}

	chrInitAI = NEW CharacterInitAI(this);
	chrInitAI->Init(reader);

	{		
		static const ConstString strTypeId("ButtonMiniGameParams");
		MOSafePointerTypeEx<ButtonMiniGameParams> mo;

		mg_params = null;
		ConstString mg_params_name = reader.String();
		if (mg_params_name.NotEmpty())
		{
			mo.FindObject(&Mission(), mg_params_name, strTypeId);
			mg_params = mo.Ptr();
		}
			
		common_mg_params = null;
		ConstString common_mg_params_name = reader.String();
		if (common_mg_params_name.NotEmpty())
		{
			mo.FindObject(&Mission(), common_mg_params_name, strTypeId);
			common_mg_params = mo.Ptr();		
		}
	}

	init_VanishBody = reader.Bool();

	bombExplosionPattern = reader.String();

	seaReflection = reader.Bool();
	seaRefraction = reader.Bool();

	init_Show = reader.Bool();
	init_Active = reader.Bool();	

	if (InitFromPattern())
	{
		InitCharData();		

		if (isPlayer && !EditMode_IsOn())
		{
			bool isMultiplayer = string::IsEqual("survival", api->Storage().GetString("Profile.Global.Difficulty", ""));
			dword playerNum = ((PlayerController*)controller)->GetPlayerNum();
			chrAchievements = NEW CharacterAchievements(this, playerNum, isMultiplayer);
		}

		return true;
	}

	return InitDefault();
}

bool Character::InitFromPattern()
{		
	pattern = null;	
	static const ConstString strTypeId("CharacterPattern");
	if (!pattern_ptr.FindObject(&Mission(),pattern_name,strTypeId))
	{		
		if (!EditMode_IsOn())
		{
			LogicDebugError("Pattern for %s not set or invalid",GetObjectID().c_str());
		}

		return false;
	}	
	pattern = pattern_ptr.Ptr();

	static const ConstString playerId("Player");

	if (pattern->aiType.NotEmpty() && pattern->aiType.c_str()[0] == 'P' && pattern->aiType.c_str()[1] == 'l')
	{
		isPlayer = true;
	}
	else
	{
		isPlayer = false;
	}

	if (!isPlayer && GetObjectID() == playerId)
	{
		LogicDebugError("SetController: Player should have Player controller...");
		api->Trace("ERROR: SetController: Player should have Player controller");
		return false;
	}

	//Модельки		
	if(!SetModel(model, pattern->modelName))
	{		
		if(!EditMode_IsOn())
		{
			LogicDebugError("Character not created, model \"%s\" not loaded...", pattern->modelName);
			return false;
		}
	}
		
	currentModel = model.scene;

	//Рэгдол	
	LoadRagdoll();

	//Теневая моделька	
	SetModel(shadowModel, pattern->shadowName);	

	Assert(!events);
	events = NEW CharacterAnimationEvents(this);	

	Assert(!animation);
	animation = NEW CharacterAnimation(this);

	//Анимация	
	animation->AddAnimation("base",pattern->animationName);
	animation->SetAnimation("base");	
	ApplyAnimationQuery(-1.0f);

	//Assert(!physics);	
	//physics = NEW CharacterPhysics(this);
	chr_physics.SetOwner(this);
	physics = &chr_physics;

	Assert(!logic);
	logic = NEW CharacterLogic(this);
	
	Assert(!items);
	items = NEW CharacterItems(this);
	
		
	//Физические парметры	
	physics->Reset();		
	physics->SetRadius(pattern->fRad);
	physics->SetHeight(pattern->fHeight);	
	physics->SetPos(Vector(0.0f));	

	finder->SetBoxSize(Vector(physics->GetRadius() * 2, physics->GetHeight(), physics->GetRadius() * 2));

	//Логический
	//logic->Init();
	if (IsPlayer())
	{
		logic->Init();
	}

	fBloodDropDamage = pattern->fBloodDropDamage;

	logic->Reset();
	aiType = pattern->aiType;
	SetSide(pattern->side);
	logic->InitMaxHP(pattern->hp);
	logic->InitHP(logic->GetMaxHP());

	MOSafePointer obj;
	FindObject(pattern->hpBar,obj);

	hpBar = obj.Ptr();

	bShotImmuneWhenRoll = pattern->bShotImmuneWhenRoll;

	fHitDmgMultiplayer = pattern->fHitDmgMultiplayer;
	fBombDmgMultiplayer = pattern->fBombDmgMultiplayer;
	fShootDmgMultiplayer = pattern->fShootDmgMultiplayer;
	fRageDmgMultiplayer = pattern->fRageDmgMultiplayer;

	attackSpeedModifier = pattern->attackSpeedModifier;

	block_cooldown = pattern->block_cooldown;

	fBlockProbabilityPerHit = pattern->fBlockProbabilityPerHit;
	fBlockProbabilityPerShoot = pattern->fBlockProbabilityPerShoot;

	fTotalDmgMultiplier = pattern->fTotalDmgMultiplier;

	fCointAltetude = pattern->fCointAltetude;	

	bShowCoinED = pattern->bShowCoinED;	

	bonusTable = pattern->bonusTable;

	if(!EditMode_IsOn())
	{		
		events->Init(&Sound(), &Particles(), &Mission());
		//if(animation) events->SetAnimation(animation->GetCurAnimation());
		//if(actorData.ani) events->SetAnimation(actorData.ani);
		if(model.scene)
		{
			Matrix mtx;
			events->SetScene(model.scene, GetMatrix(mtx));
		}
	}

	if (!SetController(aiType))
		return false;

	for(long i = 0; i < pattern->logic_loactors; i++)
	{		
		items->SetLogicLocator(pattern->logic_loactors[i].id, pattern->logic_loactors[i].locator);
	}

	items->max_armor_damage_absorb = pattern->max_armor_damage_absorb;



	{		
		IGMXScene* armor_model = Geometry().CreateGMX(pattern->armorName, &Animation(), &Particles(), &Sound());

		if (armor_model)
		{
			armor_model->SetDynamicLightState(pattern->dynamicLighting);
			
			armor = NEW CharacterArmor(this);			
			armor->Init(armor_model,&Animation());
		}
	}


	items->ReserveElements(pattern->item_descr.Size());
	for(long i = 0; i < pattern->item_descr; i++)
	{
/*
		pattern->item_descr
		CharacterItems::TItemDescr descr;

		descr.id = pattern->item_descr[i].id;
		descr.modelName = pattern->item_descr[i].modelName;
		descr.locatorName = pattern->item_descr[i].locatorName;
		descr.useTrail = pattern->item_descr[i].useTrail;
		descr.weapon_type = pattern->item_descr[i].weapon_type;
		descr.fProbality = pattern->item_descr[i].fProbality;
		descr.time_lie = pattern->item_descr[i].time_lie;
		descr.itemLife = -1;				
		descr.uniqTexture = pattern->item_descr[i].uniqTexture;
		descr.showFlare = pattern->item_descr[i].showFlare;
		descr.locatortoattache = pattern->item_descr[i].locatortoattache;
		descr.attachedobject = pattern->item_descr[i].attachedobject;
		descr.objectoffset = pattern->item_descr[i].objectoffset;
*/
		items->AddItem(&pattern->item_descr[i]);
	}

	items->armor_hp_drop = pattern->armor_hp_drop;
	items->armor_drop_prop = pattern->armor_drop_prop * 0.01f;

	items->Reset();
			
	{
		MOSafePointerTypeEx<EffectTable> mo;
		static const ConstString strTypeId("EffectTable");
		mo.FindObject(&Mission(),pattern->step_effectTable,strTypeId);
		step_effectTable = mo.Ptr();
	
		mo.FindObject(&Mission(),pattern->shoot_effectTable,strTypeId);
		shoot_effectTable = mo.Ptr();		
	}

	{
		MOSafePointerTypeEx<CharacterEffect> mo;
		static const ConstString strTypeId("CharacterEffect");
		mo.FindObject(&Mission(),pattern->sword_effectTable,strTypeId);
		chrEffect = mo.Ptr();		
	}	

	//Параметры рендера
	dynamicLighting = pattern->dynamicLighting;
	shadowCast = pattern->shadowCast;
	shadowReceive = pattern->shadowReceive;
	seaReflection |= pattern->seaReflection;
	seaRefraction |= pattern->seaRefraction;

	Assert(pattern->FatalityParams.Len() < sizeof(FatalityParamsData));
	crt_strncpy(FatalityParamsData, sizeof(FatalityParamsData), pattern->FatalityParams.c_str(), sizeof(FatalityParamsData));
	FatalityParamsData[sizeof(FatalityParamsData) - 1] = 0;
	FatalityParamsId.Set(FatalityParamsData, pattern->FatalityParams.Hash(), pattern->FatalityParams.Len());
	AttackParams = pattern->AttackParams;

	//Параметры контроллера
	ccp = null;
	
	if(model.scene)
	{
		model.scene->SetDynamicLightState(dynamicLighting);
		items->SetDynamicLightingFlag(dynamicLighting);
	}

	if (shadowModel.scene)
	{
		shadowModel.scene->SetDynamicLightState(dynamicLighting);
	}

	if (isPlayer && !EditMode_IsOn())
	{
		buttonMG = NEW CharacterButtonMiniGame(this);		
		buttonMG->Init();

		if (mg_params)
		{
			buttonMG->SetMissionTime(mg_params->GetMissionTime());
		}			
	}

	// Вынес в Init, иначе playerNum был всегда равен 1
	/*if (isPlayer && !EditMode_IsOn())
	{
		bool isMultiplayer = string::IsEqual("survival", api->Storage().GetString("Profile.Global.Difficulty", ""));
		dword playerNum = ((PlayerController*)controller)->GetPlayerNum();
		chrAchievements = NEW CharacterAchievements(this, playerNum, isMultiplayer);
		chrAchievements->Init();		
	}*/
		

	coinModelIndex = -1;
	fCoinAngle = Rnd(3.0f);	

	BodyParts.Empty();
	BodyParts.AddElements(pattern->body_parts.Size());
	for(long i = 0; i < pattern->body_parts; i++)
	{
		//TPartOfMe Part;
		//BodyParts.Add(Part);

		BodyParts[i].pPhysBox = NULL;
		BodyParts[i].pPhysCapsule = NULL;
		BodyParts[i].pParticale = NULL;

		BodyParts[i].ID = pattern->body_parts[i].id;
		
		SetModel(BodyParts[i].PartModel, pattern->body_parts[i].modelName);	

		if(BodyParts[i].PartModel.scene)
		{
			BodyParts[i].PartModel.scene->SetDynamicLightState(dynamicLighting);
		}

		BodyParts[i].LocatorName = pattern->body_parts[i].locator;

		BodyParts[i].isBox = pattern->body_parts[i].isBox;	
	}	

	return true;
}

void Character::InitCharData()
{	
	if (!IsPlayer())
	{
		logic->SetHPMultipler(Clampf(arbiter->npc_hp->Get(100.0f) * 0.01f, 0.5f, 256.0f));
		logic->SetAttackMultipler(Clampf(arbiter->npc_damage->Get(100.0f) * 0.01f,0.5f,256.0f));		
	}
	else
	{		
		logic->SetHPMultipler(Clampf(arbiter->player_live->Get(100.0f) * 0.01f,0.5f,256.0f));
	}

	logic->InitHP(logic->GetMaxHP());
	logic->SetHP(logic->GetMaxHP());

	physics->SetPos(init_pos);
	physics->SetAy(init_ay);

	logic->SetSpawnPoint(init_pos);

	actorData.ani = animation->AddAnimation("actor",init_actorani);	
	//if(actorData.ani) events->SetAnimation(actorData.ani);

	//Таймлайн		
	actorTimeLine = null;
	actorTimeLineName = init_actorTimeLineName;

	if (actorTimeLineName.NotEmpty())
	{
		MOSafePointerTypeEx<ActorTimeLine> mo;
		static const ConstString strTypeId("ActorTimeLine");
		mo.FindObject(&Mission(),actorTimeLineName,strTypeId);	
		actorTimeLine = mo.Ptr();	
	}

	bEnableAniBlend  = init_EnableAniBlend;
	bColideWhenActor = init_ColideWhenActor;
	bActorMoveColider = init_bActorMoveColider;

	if(controller && !EditMode_IsOn())
	{
		if(init_aiInit.NotEmpty())
		{
			MOSafePointerTypeEx<CharacterControllerParams> mo;		
			static const ConstString strTypeId("CharacterControllerParams");
			mo.FindObject(&Mission(),init_aiInit,strTypeId);			
			
			if(mo.Ptr())
			{
				ccp = mo.Ptr();

				if(ccp->IsControllerSupport(controller->GetAIParamsName()))
				{
					controller->SetParams(ccp);					
				}
				else
				{
					LogicDebugError("Can't set init params object \"%s\" for AI, object not support that AI type", init_aiInit.c_str());
				}
			}
			else
			{
				LogicDebugError("Can't set init params object \"%s\" for AI, %s", init_aiInit.c_str(), mo.Ptr() ? "invalide object type" : "object not found");
			}
		}		
		
		controller->Init();	
		controller->Reset();
	}

	//init_actrorPreview = false;		

	if (IsPlayer())
	{
		if (!greedy) greedy = BonusesManager::CreateGreedy(this);
	}

	if (EditMode_IsOn() && init_actrorPreview && actorTimeLine)
	{		
		animation->SetAnimation("actor",0.0f);
		ResetActor();
		UpdateActor(0);		
	}


	bVanishBody = init_VanishBody;

	Activate(init_Active);
	Show(init_Show);
}

bool Character::InitDefault()
{	
	if(!EditMode_IsOn())
	{	
		return false;	
	}

	model.scene = null;
	currentModel = model.scene;	

	//Assert(!physics);	
	//physics = NEW CharacterPhysics(this);	
	chr_physics.SetOwner(this);
	physics = &chr_physics;

	Assert(!logic);
	logic = NEW CharacterLogic(this);	
	

	Assert(!events);
	events = NEW CharacterAnimationEvents(this);	


	Assert(!animation);
	animation = NEW CharacterAnimation(this);	

	Assert(!items);
	items = NEW CharacterItems(this);		


	physics->Reset();	
	physics->SetAy(init_ay);
	physics->SetRadius(0.4f);
	physics->SetHeight(1.8f);
	physics->SetPos(init_pos);

	finder->SetBoxSize(Vector(physics->GetRadius() * 2, physics->GetHeight(), physics->GetRadius() * 2));
	finder->SetBoxCenter(Vector(0.0, physics->GetHeight() * 0.5f, 0.0f));

	//Логический
	//logic->Init();	
	logic->Reset();
	logic->InitMaxHP(100.0f);
	logic->InitHP(logic->GetMaxHP());	
			
	Activate(init_Active);
	Show(init_Show);	

	FatalityParamsId.Empty();
	FatalityParamsData[0] = 0;
	AttackParams.Empty();

	return true;
}

void Character::ReleaseFlyes()
{
	if (!flyCloud)
		return;

	flyCloud->Release(false);
	flyCloud=null;
}

//Пересоздать объект
void Character::Restart()
{
#ifdef CHARACTER_STARFORCE
	scaleSpike = RRnd(0.999f, 1.001f);
	scaleSpikeTime = RRnd(120.0f, 160.0f);
	curScaleSpike = 1.0f;
#endif

	killerId.Empty();
	statDismembered = false;
	fBloodDamage = 0.0f;
	fTimeToFreeze = 2.0f;

	if (isPlayer)
	{
		arbiter->DestroyAllWeapon();
		Show(false);
	}

	if (ragdoll)
	{		
		ActivateRagdoll(false, 0.0f);
	}

	if (bDeadConterOn && !bVanishBody)
		arbiter->DelDeadBody(*this);

	bDeadConterOn = false;
	DelUpdate(&Character::DeadTimer);

	if(ragdoll)
	{
		ragdoll->Release();
		ragdoll = null;	
	}

	ReleaseFlyes();

	for(long i = 0; i < (int)BodyParts.Size(); i++)
	{
		if (BodyParts[i].pPhysBox)
		{
			BodyParts[i].pPhysBox->Release();
			BodyParts[i].pPhysBox = NULL;
		}

		if (BodyParts[i].pPhysCapsule)
		{
			BodyParts[i].pPhysCapsule->Release();
			BodyParts[i].pPhysCapsule = NULL;
		}

		if (BodyParts[i].pParticale)
		{
			BodyParts[i].pParticale->Release();
			BodyParts[i].pParticale=NULL;
		}		
	}

	fTimeInFatality = 0.0f;
	actorData.chr = this;
	bWaterSpalshBorned = false;
	
	showCollider = false;	
	isHitLight = false;
	hitLight = 0.0f;
	deadTime = -1.0f;
	deadBodyTime = 0.0f;
	bombTime = -1.0f;	
	actorTimeLine = null;	

	fCoindDltHgt = 0.0f;

	LastCharCoin = chrcoin_none;
	CharCoin = chrcoin_none;	

	bColideWhenActor = false;
	bEnableAniBlend = false;
	fTimeToStandUp = 0.0f;
	fTotalTimeToStandUp = 0.0f;
	bDeadConterOn = false;
	bAutoReleaseRagdollOn = false;
	fTimeToReleaseRagDoll = 0.0f;
	fTimeInFatality = FATALITY_TIME;
	/*fHitDmgMultiplayer = 1.0f;
	fBombDmgMultiplayer = 1.0f;
	fShootDmgMultiplayer = 1.0f;	
	fRageDmgMultiplayer = 1.0f;*/
	debugInfoType = false;

	bDraw = true;

	for (int i=0;i<20;i++)
	{
		pair_minigame[i] = null;
	}

	bodyparts_inited = false;
	bShadowGrRegistred = false;
	bReflectGrRegistred = false;
	bRefractGrRegistred = false;
	auto_spawnpoint = auto_spawnpoint_saved;
	init_spawnpoint = 0.0f;
	useRootBone = false;
	set_new_ay = false;
	
	//fCointAltetude = 2.0f;

	if (isPlayer && controller)
	{
		((PlayerController*)controller)->RemoveSecondWeapon();
	}

	logic->Reset();
	items->Reset();
	physics->Reset();
	animation->Reset();
	InitCharData();	

	/*if (isPlayer)
	{
		arbiter->DestroyAllWeapon();
	}

	Release();
	
	fTimeInFatality = 0.0f;
	actorData.chr = this;
	bWaterSpalshBorned = false;
	arbiter = null;
	logic = null;
	items = null;
	physics = null;	
	ragdoll = null;
	controller = null;
	animation = null;
	actorData.ani = null;
	buttonMG = null;
	showCollider = false;
	events = null;
	currentModel = null;
	isHitLight = false;
	hitLight = 0.0f;
	deadTime = -1.0f;
	bombTime = -1.0f;
	ccp = null;
	actorTimeLine = null;
	isPlayer = false;

	fCoindDltHgt = 0.0f;

	LastCharCoin = chrcoin_none;
	CharCoin = chrcoin_none;

	if(rdData)
	{
		rdData->Release();
		rdData = null;
	}

	bColideWhenActor = false;

	bEnableAniBlend = false;

	fTimeToStandUp = 0.0f;
	fTotalTimeToStandUp = 0.0f;

	bDeadConterOn = false;

	bAutoReleaseRagdollOn = false;
	fTimeToReleaseRagDoll = 0.0f;

	fTimeInFatality = FATALITY_TIME;

	fHitDmgMultiplayer = 1.0f;
	fBombDmgMultiplayer = 1.0f;
	fShootDmgMultiplayer = 1.0f;

	fCointAltetude = 2.0f;

	debugInfoType = false;

	bDraw = true;

	for (int i=0;i<5;i++)
	{
		pair_minigame[i] = null;
	}

	bodyparts_inited = false;

	bShadowGrRegistred = false;
	bReflectGrRegistred = false;

	auto_spawnpoint = true;
	init_spawnpoint = 0.0f;

	useRootBone = false;

	set_new_ay = false;

#ifndef NO_CONSOLE
	debugMessageShowTime = 0.0f;
#endif	

	ReCreate();*/
}

//Освободить ресурсы
void Character::Release()
{	
	bWaterSpalshBorned = false;

	if (ragdoll)
	{		
		ActivateRagdoll(false, 0.0f);
	}
	
	if (bDeadConterOn && !bVanishBody)
		arbiter->DelDeadBody(*this);

	statDismembered = false;
	bDeadConterOn = false;
	DelUpdate(&Character::DeadTimer);

	Unregistry(MG_SHADOWCAST);
	Unregistry(MG_SHADOWRECEIVE);
	Unregistry(MG_SEAREFLECTION);
	Unregistry(MG_SEAREFRACTION);
	Unregistry(MG_SHADOWDONTRECEIVE);

	DelUpdate(&Character::DrawTrail);
	DelUpdate(&Character::DrawFlares);
	DelUpdate(&Character::Draw);
	DelUpdate(&Character::DrawCoin);	

	if (logic)
	{
		if(!logic->IsActor())
		{
			DelUpdate(&Character::Work);
			DelUpdate(&Character::UpdatePhysics);
			physics->Activate(false);
		}
	}

	if (armor)
	{
		delete armor;
		armor = null;
	}

	if(controller)
	{				
		delete controller;
		controller = null;
	}

	/*if (rdData)
	{
		rdData->Release();
		rdData = null;
	}*/

	if (!EditMode_IsOn())
	{
		if(arbiter)
		{
			arbiter->Activate(this, false);
			arbiter = null;
		}
	}

	if (logic)
	{
		delete logic;
		logic = null;
	}

	if (items)
	{		
		delete items;
		items = null;
	}

	if (physics)
	{
		physics->Release();		
	}

	if (greedy)
	{
		greedy->Release();
		greedy = null;
	}

	bodyparts.UnregistryBlendStage();
	bodyparts.SetAnimation(null);

	if (ragdoll)
	{
		ragdoll->Release();
		ragdoll = null;	
	}	

	
	if (animation)
	{
		delete animation;
		animation = null;
	}

	ReleaseFlyes();

	if (model.scene)
	{
		model.scene->SetAnimation(null);
		model.scene->Release();
		model.scene = null;
	}
	
	if (shadowModel.scene)
	{
		shadowModel.scene->SetAnimation(null);
		shadowModel.scene->Release();
		shadowModel.scene = null;
	}	

	if (events)
	{
		events->ResetParticles();
		events->ResetSounds();
		delete events;
		events = null;
	}	

	if (buttonMG)
	{
		delete buttonMG;
		buttonMG = null;
	}

	if (chrAchievements)
	{
		delete chrAchievements;
		chrAchievements = null;
	}

	if (chrInitAI)
	{
		delete chrInitAI;
		chrInitAI = null;
	}

	for(long i = 0; i < (int)BodyParts.Size(); i++)
	{
		if (BodyParts[i].pPhysBox)
		{
			BodyParts[i].pPhysBox->Release();
			BodyParts[i].pPhysBox = NULL;
		}
		
		if (BodyParts[i].pPhysCapsule)
		{
			BodyParts[i].pPhysCapsule->Release();
			BodyParts[i].pPhysCapsule = NULL;
		}

		if (BodyParts[i].pParticale)
		{
			BodyParts[i].pParticale->Release();
			BodyParts[i].pParticale=NULL;
		}

		if (BodyParts[i].PartModel.scene)
		{
			BodyParts[i].PartModel.scene->Release();
			BodyParts[i].PartModel.scene = null;
		}		
	}

	//actorAnimationName = "";
	BodyParts.DelAll();
}

//Установить модельку
bool Character::SetModel(Model & model, const char * modelName)
{	
	//Удаляем модельку
	if(model.scene) 
	{
		model.scene->SetAnimation(null);
		model.scene->Release();
	}
	model.scene = null;	

	IGMXScene* old_model = model.scene;
	model.scene = Geometry().CreateGMX(modelName, &Animation(), &Particles(), &Sound());
	RELEASE(old_model);

	if(model.scene == null) return false;
	
	return true;
}

void Character::ApplyAnimation()
{
	if (logic)
		logic->NeedStateUpdate();

	//ApplyAnimationQuery(0.01f);
	SetUpdate(&Character::ApplyAnimationQuery, ML_DYNAMIC1);
}

void _cdecl Character::ApplyAnimationQuery(float dltTime, long level)
{	
	if (logic)
		logic->NeedStateUpdate();

	IAnimation* anim = animation->GetCurAnimation();

	if(model.scene)
	{
		model.scene->SetAnimation(anim);
	}
	if(shadowModel.scene)
	{
		shadowModel.scene->SetAnimation(anim);
	}

	if (events)
	{
		if (animation->GetLastAnimation())
		{
			events->DelAnimation(animation->GetLastAnimation());
		}

		if (animation->GetCurAnimation())
		{
			events->SetAnimation(animation->GetCurAnimation());
		}

		events->ResetCollapser();
		events->SetTimeEvent(false);
	}	

	if (anim)
	{										
		hips_boneIdx = anim->FindBone("|Hips_Joint");
		spine_boneIdx = anim->FindBone("|Hips_Joint|Spine1_joint|Spine_joint");

		if (bodyparts_inited)
		{
			bodyparts.ResetAnimation(anim);

			if (pattern)
			{
				bodyparts.SetLimit(pattern->fHeadRotateLimit * PI / 180.0f,true);
				bodyparts.SetOffset(pattern->fHeadRotateOffset * PI / 180.0f,true);
			}
		}
		else
		{
			CharacterBlender::InitData data;
			data.animation = anim;
			data.model = model.scene;
			data.chr = this;

			bodyparts_inited = true;

			bodyparts.Init(data);

			if (pattern)
			{
				bodyparts.SetLimit(pattern->fHeadRotateLimit * PI / 180.0f,true);
				bodyparts.SetOffset(pattern->fHeadRotateOffset * PI / 180.0f,true);
			}
		}

		if (IsPlayer())
		{
			bool hide = true;

			if (controller)
			{
				if (((PlayerController*)controller)->cur_weapon_style &&
					((PlayerController*)controller)->itemTaken &&
					!((PlayerController*)controller)->qevent)
				{					
					if (((PlayerController*)controller)->cur_weapon_style->weapon_type == wp_rapire)
					{
						hide = false;
					}
				}
			}

			if (hide)
			{
				const char* params[2];
				params[0] = "collapse";
				params[1] = "saber_joint";

				events->BoneCollapse(anim,"collapse",params,2);
			}
		}
	}
	else
	{
		hips_boneIdx = -1;
		spine_boneIdx = -1;
	}	

	graph_changed = false;

	if (dltTime>=0.0f)
	{
		if (was_showed)
		{
			Show(true);
		}
	}

	if (logic)
	{
		if (logic->IsActor())
		{
			if (act_node != -1)
			{				
				actorTimeLine->Goto(actorData, act_node);
				UpdateActor(0.0f);

				act_node = -1;
			}
			else
			{
				ResetActor();
				UpdateActor(0);
			}
		}
	}	

	DelUpdate(&Character::ApplyAnimationQuery);
}

//Установить контролер
bool Character::SetController(const ConstString & ctrlName)
{
	static const ConstString playerId("Player");
	if (ctrlName == playerId) 
	{
		Registry(MG_BONUSPICKUPER);
	}
	else if (GetObjectID() == playerId) 
	{
		api->Trace("ERROR: SetController: Player should have Player controller");
		return false;
	}


	Assert(!controller);
	if(ctrlName.IsEmpty())
	{
		Assert(false);
		return false;
	}
	for(CharacterControllerDeclarant * decl = CharacterControllerDeclarant::GetFirst(); decl; decl = decl->GetNext())
	{		
		if(decl->Name() == ctrlName)
		{
			controller = decl->Create(*this);			
			controller->Reset();

			return true;
		}
	}
	Assert(false);
	return false;
}

//Установить арбитра персонажей
bool Character::SetArbiter()
{
	MOSafePointer obj;
	static const ConstString objectId("CharactersArbiter");
	Mission().CreateObject(obj,"CharactersArbiter", objectId);
	arbiter = (CharactersArbiter*)obj.Ptr();
	Assert(arbiter);	

	return true;
}

//Установить сторну персонажа
void Character::SetSide(const ConstString & side)
{
	CharacterLogic::Side sd = CharacterLogic::s_npc;

	struct SideId
	{
		ConstString id;
		CharacterLogic::Side side;
	};

	static const SideId sides[] = 
	{
		{ConstString("Enemy"), CharacterLogic::s_enemy},
		{ConstString("Ally"), CharacterLogic::s_ally},
		{ConstString("Ally FrFire"), CharacterLogic::s_ally_frfire},
		{ConstString("Boss"), CharacterLogic::s_boss},
		{ConstString("NPC"), CharacterLogic::s_npc}
	};

	for(dword i = 0; i < ARRSIZE(sides); i++)
	{
		if(side == sides[i].id)
		{
			sd = sides[i].side;
			break;
		}
	}

	logic->SetSide(sd);
}

//Загрузить рэгдол
void Character::LoadRagdoll()
{
	if(ragdoll)
	{
		ragdoll->Release();
		ragdoll = null;
	}

	// загружаем паттерный регдолл
	pattern->LoadRagdoll();
}

void Character::StartButtonMG(MiniGame_Desc& desc, bool auto_win)
{
	if (buttonMG)
	{		
		buttonMG->Start(desc, auto_win);
	}
}

IAnimation** Character::PrepareAnim()
{
	static IAnimation* anims[23];

	anims[0] = animation->GetCurAnimation();	

	Character* pair = logic->GetPairCharacter();

	if (pair)
	{
		anims[1] = pair->animation->GetCurAnimation();
	}
	else
	{
		anims[1] = null;
	}

	if (logic->pair_mg)
	{
		anims[2] = logic->pair_mg->animation->GetCurAnimation();
	}
	else
	{
		anims[2] = null;
	}	


	for (int i=0;i<20;i++)
	{
		if (pair_minigame[i])
		{
			anims[i+3] = pair_minigame[i]->animation->GetCurAnimation();
		}
		else
		{
			anims[i+3] = null;
		}		
	}

	return anims;
}

void Character::UpdateMG(float dltTime)
{	
	if (!buttonMG) return;
	
	buttonMG->Update(dltTime,PrepareAnim(), 23);
}

void Character::EndButtonMG()
{
	if (!buttonMG) return;

	buttonMG->ShowWidget(false);	
	if (buttonMG->wait_end_mg) buttonMG->ActivateTriger();
	if (!buttonMG->right_pressed)
	{
		if (buttonMG->lose_triger && !buttonMG->lose_triggered)
		{
			buttonMG->lose_triggered = true;
			LogicDebug("Lose Triger in '%s' ButtonMG Activated:", buttonMG->id);
			buttonMG->lose_triger->Activate(true);
		}
	}
	buttonMG->mode = mg_disabled;
}


bool Character::Attack(MissionObject * obj, dword source, float hp, const Vector & center, float radius)
{	
	if (ragdoll || GetHP()<0.1f || IsDead() || !IsActive() || logic->IsActor() || logic->IsPairMode()) return false;

	Matrix mat(true);
	physics->GetModelMatrix(mat);

	mat.pos.y += physics->GetHeight()*0.5f;

	if(Box::OverlapsBoxSphere(mat, Vector(physics->GetRadius(),physics->GetHeight()*0.5f,physics->GetRadius()), center, radius))
	{
		logic->Hit(source, hp , "hit", 0.0f, hp, "hit", NULL, null, false);	

		return true;
	}
	
	return false;
}

//Воздействовать на объект линией
bool Character::Attack(MissionObject * obj, dword source, float hp, const Vector & from, const Vector & to)
{		
	if (ragdoll || GetHP()<0.1f || IsDead() || !IsActive() || logic->IsActor() || logic->IsPairMode()) return false;		

	Matrix mat(true);
	physics->GetModelMatrix(mat);

	Vector pos = mat.pos;

	mat.pos.y += physics->GetHeight()*0.5f;

	if(Box::OverlapsBoxLine(mat, Vector(physics->GetRadius(),physics->GetHeight()*0.5f,physics->GetRadius()), from,to))
	{		
		if (source == DamageReceiver::ds_shooter)
		{
			IShooter* shooter = (IShooter*)obj;

			pos += Vector(RRnd(-0.3f, 0.3f), 0.0f, RRnd(-0.3f, 0.3f));
			arbiter->Boom(null, source, pos, shooter->GetRadius(), hp, shooter->GetPower() * RRnd(0.5f, 1.5f));

			Sound().Create3D(shooter->GetCharSound(), mat.pos, _FL_);

			IParticleSystem* pSys = Particles().CreateParticleSystem(shooter->GetCharParticle().c_str());
			if (pSys)
			{
				Matrix mt(true);
				Vector dir = from - to;
				dir.Normalize();

				mt.BuildOrient(dir,Vector(0.0f,1.0f,0.0f));
				mt.pos = mat.pos;

				pSys->Teleport(mt);				
				pSys->AutoDelete(true);
			}
		}
		else
		if (source != DamageReceiver::ds_check)
		{
			logic->Hit(source, hp, "hit", 0.0f, hp, "hit", NULL, null, false);	
		}

		if (source == DamageReceiver::ds_check && !logic->IsEnemy(arbiter->GetPlayer()))
			return false;

		return true;
	}

	return false;
}

//Воздействовать на объект выпуклым чехырёхугольником
bool Character::Attack(MissionObject * obj, dword source, float hp, const Vector vrt[4])
{		
	if (ragdoll || GetHP()<0.1f || IsDead() || !IsActive() || logic->IsActor() || logic->IsPairMode()) return false;	

	// обработка
	if (source != DamageReceiver::ds_sword) return false;

	Matrix mat(true);
	physics->GetModelMatrix(mat);

	mat.pos.y += physics->GetHeight()*0.5f;

	if(Box::OverlapsBoxPoly(mat, Vector(physics->GetRadius(),physics->GetHeight()*0.5f,physics->GetRadius()), vrt))
	{
		logic->Hit(source, hp , "hit", 0.0f, hp, "hit", NULL, null, false);	

		return true;
	}

	return false;
}

void Character::Hit(dword source, float dmg, const char* reaction, float block_dmg, const char* blockReaction, const char * deathLink)
{	
	logic->Hit(source, dmg, reaction, 0.0f, block_dmg, blockReaction, deathLink, NULL, false);	
}

//Найти пересечение колижена и отрезка, если нет пересечения вернуть false
bool Character::CollisionLine(const Vector & start, const Vector & end, Vector & p, Vector & n)
{
	IPhysicsScene::RaycastResult res;

	if (Physics().Raycast(start, end, phys_mask(phys_character),&res))
	{
		//if (res.mtl != pmtlid_air)
		{
			p = res.position;
			n = res.normal;

			return true;
		}
	}

	return false;		
}

Vector Character::GetBombPosition(const Vector & from, const Vector & to, float time)
{
	time = Clampf(time);
	Vector dir = to - from;
	float h = dir.GetLengthXZ()*0.2f;
	dir = from + (to - from)*time + Vector(0.0f, h*sinf(time*PI), 0.0f);
	return dir;
}

//Переместить бонус
void _cdecl Character::FlyBomb(float dltTime)
{	
	bombTime += flySpeed*dltTime;
	angles += anglesRot*dltTime;

	if(bombTime >= 1.0f)
	{
		bombTime = 1.0f;

		bombLieTime -= dltTime;

		if (bombLieTime<=0)		
		{
			bombLieTime = 0.0f;

			bombTime = -1.0f;
			DelUpdate(&Character::FlyBomb);

			if(controller)
			{
				Vector pos = bomb.pos;
				
				if (!useBombPhysic)
				{
					pos = bombEnd;
				}

				controller->Boom(pos);
			}

			return;
		}
	}

	Matrix mtx;

	if (useBombPhysic)
	{				
		//Отрезок перемещения
		Vector delta = bomb.velocity*dltTime;
		
		//Точка на краю сферы в направлении движения в системе бонуса
		Vector edge = bomb.velocity;
		edge.Normalize();
		edge *= bomb.radius;
		//Пересчитываем скорости для следующего кадра
		bomb.velocity.FrictionXZ(5.0f*dltTime, 0.8f);
		bomb.velocity.y -= 18.8f*dltTime;
		//Точка, до которой проверяем наличие препятствий
		Vector newPos = bomb.pos + edge + delta;
		//Render().DrawLine(bonus.pos, 0xff0000ff, newPos, 0xffff0000);
		
		Vector cp(0.0f), cn(0.0f);
		bool isCollision = false;
		if(CollisionLine(bomb.pos, bomb.pos - Vector(0.0f, bomb.radius, 0.0f), cp, cn))
		{
			isCollision = true;
			bomb.pos = cp + Vector(0.0f, bomb.radius + 0.01f, 0.0f);
		}
		if(isCollision || CollisionLine(bomb.pos, newPos, cp, cn))
		{
			//В зависимости от скорости решаем двигатся дальше или заснуть
			cn.Normalize();
			if(cn.y > 0.5f && bomb.velocity.GetLengthXZ2() < 0.045f && fabsf(bomb.velocity.y) < 0.8f)
			{			
				bomb.isSleep = true;

				/*if(controller)
				{
					controller->Boom(bomb.pos);
				}*/
			}
			else
			{
				//Отскакиваем
				bomb.velocity.Reflection(cn);
				if(cn.y > 0.0f && bomb.velocity.y < 0.0f)
				{
					bomb.velocity.y = -bomb.velocity.y;
				}
				if(cn.y > 0.5f)
				{
					bomb.velocity.y *= (1.5f - cn.y)*0.8f;
				}			
			}
		}
		else
		{
			//Перемещаемся в выбраном направлении
			bomb.pos += delta;
		}	

		mtx = Matrix(angles, bomb.pos);
	}
	else
	{
		mtx = Matrix(angles, GetBombPosition(bombStart, bombEnd, bombTime));
	}

	if (bombTime>0 || (bombTime<=0 && bombLieTime>0)) arbiter->DrawBomb(mtx);
}

float Character::GetAlpha()
{
	return Clampf((GetDeathTime() - deadTime)/CHARACTER_BODYDISAPEAR);
}

void  Character::DisableTimeEvent()
{	
	if (!IsPlayer()) return;

	events->SetTimeEvent(false);

	if (logic->GetPairCharacter())
	{
		logic->GetPairCharacter()->events->SetTimeEvent(false);
	}

	if (logic->pair_mg)
	{
		logic->pair_mg->events->SetTimeEvent(false);
	}

	for (int i=0;i<20;i++)
	{
		if (pair_minigame[i])
		{
			pair_minigame[i]->events->SetTimeEvent(false);
		}		
	}	
}

void Character::DeattachChar()
{
	if (logic && logic->is_attached)
	{
		logic->is_attached = false;

		if (logic->master)
			logic->master->logic->slave = null;

		logic->master = null;
	}
}



//============================================================================================
//Параметры инициализации
//============================================================================================

const char * Character::comment = 
"This object representation character in game\n"
"  Aviable commands list:\n"
"    EnableActor - включить режим актера\n"
"    DisableActor - выключить режим актера\n"
"    ActorSet timeLineObject - указать актеру таймлайн\n"    
"    Goto timeLinePointIndex - перейти на указанную точку таймлайна\n"
"    AddItem itemID modelName locatorName [trail] - добавить предмет\n"
"    DelItem itemID - удалить предмет\n"
"    sethp HP [reaction] - задать HP (не в процентах)\n"
"    sethpLimit HPLimit - задать предел в процентах ниже которого HP опуститься не может\n"
"    changehp count [reaction] - изменить HP (не в процентах) на заданное значение\n"
"    teleport Target - переместить персонажа в позицию цели\n"
"    gizmorespawn Gizmo - отреспамить персонажа через указанную гизмо\n"
"    respawn [Target] - отреспамить персонажа \n"
"    show - показать персонажа\n"
"    hide - спрятать персонажа\n"
"    activate - активировать персонажа\n"
"    deactivate - деактивировать персонажа\n"
"    instant kill - убить персонажа, при этом происходит прятание и деактивация персонажа\n"
"    EnableActorColider - включить колижен в режиме актера\n"
"    DisableActorColider - выключить колижен в режиме актера\n"
"    EnableAniBlend - включить блендинг при смене актерского графов на боевой и обратно\n"
"    DisableAniBlend  - выключить блендинг при смене актерского графов на боевой и обратно\n"
"    setwaypoint WaypointsName - задать вейпоинты\n"
"    setFParams FatalityParams - задать фаталити парметры\n"
"    settarget target - задать цель для атаки AI персонажу\n"
"    setbombtarget target - задать цель для бомбометания\n"
"    movelocator logic_name locator_name - переместить логический локатор\n"
"    drawon - включить отрисовку персонажа\n"
"    drawoff - выключить отрисовку персонажа\n"
"    show_weapon Yes/No wp_id - показать/спрятать оружие\n"
"    grab weapon weapon_id start_node blend_time victim - включить подбор оружия, часть данных береться из оружия жертвы\n"
"    take weapon Wp_Type wp_model Broken_Wp_Type Broken_wp_mode start_node br_prt1 br_prt2 - включить подбор оружия\n"
"    ragdoll - включить регдол персонажу\n"
"    pair_minigame Character_Name,... - перечислить персов учавсвтующих в минигейме, старый список будет очищен\n"
"    pair_minigame_add Character_Name,...\n - добавить в текущий список персов учавсвтующих в минигейме\n"
"    collapse Colapse/Restore bone_name	- сколапсить/восстановить кость\n"
"    actor_attach Character_Name - прикрепить персонажа (позиции будут сопадать, будут идти синхронно по графам)\n"
"    actor_deattach - отцепить персонажа\n"
"    hairenable Yes/No - вкл/выкл анимационный блендер волос\n"
"    legsenable Yes/No - вкл/выкл IK ног\n"
"    UseRootBone Yes/No - вкл/выкл использование рутовой кости при отпределении положения персонажа\n"
"    controller [params] - послать команду AI контроллеру\n"
"    Achievement id - отметить, что условия для ачивмента с именим id были выполнены\n"
"    ResetAI - сбросить мысли и цель персонажу\n"
"    GodMode - вкл/выкл режим бесмертия\n"
" ";
	






MOP_BEGINLISTCG(Character, "Character", '1.00', 100, Character::comment, "Character")

	MOP_ENUMBEG("InitState")
		MOP_ENUMELEMENT("none")
		MOP_ENUMELEMENT("waiting")
		MOP_ENUMELEMENT("wait and alarm")
		MOP_ENUMELEMENT("patrolling")
		MOP_ENUMELEMENT("patrolling and alarm")
	MOP_ENUMEND

	//Паттерн	
	MOP_STRING("Pattern Name", "")
	MOP_POSITION("Position", Vector(0.0f))
	MOP_ANGLESEX("Angle", Vector(0.0f), Vector(0.0f, -4.0f*PI, 0.0f), Vector(0.0f, 4.0f*PI, 0.0f))	
	
	MOP_BOOL("Auto spawn point", true)
	MOP_POSITION("Spawn point", Vector(0.0f))	

	MOP_STRING("Actor animation", "")
	MOP_STRING("Actor time line", "")
	MOP_BOOL("Preview Actor timeline", false)
	MOP_BOOL("Blend AI and Act Ani", false)
	MOP_BOOL("Colide when Actor", false)
	MOP_BOOL("Move colider when Actor", false)
	MOP_BOOL("Use root bone", false)
		
	MOP_FLOATEX("Distance to drop shadow", 0.0f, 0.0f, 1000.0f);

	MOP_STRING("AI params", "")

	MOP_ENUM("InitState", "AI InitState")	
	MOP_STRING("AI InitNode", "")
	MOP_FLOATEX("AI Argo Dist", 7.0f, 1.0f, 1024.0f)
	MOP_FLOATEX("AI Alarm Dist", 7.0f, 1.0f, 1024.0f)
	MOP_FLOATEX("AI Time To Alarm", 0.5f, 0.01f, 64.0f)	
	
	MOP_FLOATEX("AI Hear Dist", 4.0f, 0.01f, 64.0f)	
	MOP_FLOATEX("AI Sector Angle", 45.0f, 0.01f, 90.0f)	
	
	MOP_ARRAYBEG("AI Patrol Point", 0, 1000)
	    MOP_POSITION("Point",0.0f)
		MOP_FLOATEX("Wait Time", -1.0f, -1.0f, 64.0f)
		MOP_STRING("Wait Node", "")
		MOP_ANGLES("Angels",0.0f)		
	MOP_ARRAYEND		

	MOP_STRING("MiniGame Params", "")
	MOP_STRING("Common MiniGame Params", "")
	MOP_BOOL("Vanish Body", false)

	MOP_STRING("Bomb explosion pattern", "")

	MOP_BOOLC("Sea reflection", false, "Character can reflect in sea")
	MOP_BOOLC("Sea refraction", false, "Character can refract in sea")
				
	MOP_BOOL("Show", true)
	MOP_BOOL("Active", true)
	
MOP_ENDLIST(Character)