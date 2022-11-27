//===========================================================================================================================
// Spirenkov Maxim, 2004
//===========================================================================================================================
// Character
//===========================================================================================================================
// CharactersArbiter
//===========================================================================================================================


#include "CharactersArbiter.h"

#include "..\BombPattern\BombPattern.h"

#include "..\..\Character\Character.h"
#include "..\..\Character\Components\CharacterLogic.h"
#include "..\..\Character\Components\CharacterPhysics.h"
#include "..\..\Character\Components\CharacterItems.h"
#include "..\..\character\components\CharacterAnimation.h"
#include "..\..\Character\CharacterController.h"
#include "..\..\Character\Components\CharacterAchievements.h"

#include "..\..\Character\CharacterPattern.h"

#include "..\..\player\PlayerController.h"

#include "..\waypoints\aiPathFinder.h"
#include "..\CombatPoints\CombatPoints.h"
#include "..\AI Zones\AIZones.h"

#include "..\..\Utils\StorageWork.h"

#include "..\..\..\common_h\AnimationNativeAccess.h"

#include "..\..\..\Common_h\IStainManager.h"
#include "..\..\..\Common_h\IExplosionPatch.h"
#include "..\..\..\Common_h\ILiveService.h"


#define CharactersArbiterTickInterval	0.1f

#define MAX_DEAD_BODIES		7

void Draw2DLine (IRender* rs,float pX, float pY, float tX, float tY, unsigned long color)
{
	struct TVertex
	{
		Vector p;
		unsigned long color;
	};

	TVertex vrx[2];

	Vector From = Vector (pX,pY,0.0f);
	Vector To = Vector (tX,tY,0.0f);

	vrx[0].p = From;
	vrx[1].p = To;

	for (int r = 0; r < 2; r++) vrx[r].color = color;

	for (r = 0; r < 2; r++) vrx[r].p.z = 0.01f;

	ShaderId id;
	rs->GetShaderId("GUIBase", id);
	rs->DrawPrimitiveUP (id, PT_LINELIST, 1, vrx, sizeof (TVertex));	
}

//============================================================================================

dword dwCurrentCharThoughtIndex = 0;

bool CharactersArbiter::charStatsShow = false;

CharactersArbiter::CharactersArbiter() : 
	find(_FL_, 48), 
	characters(_FL_, 48), 
	all_characters(_FL_,128), 
	//distTable(_FL_, 128), 
	d_recivers(_FL_, 4),
	deadBodies(_FL_, 16),
	droppedWeapons(_FL_)
#ifdef CHARACTER_STARFORCE
	,spikeStars(_FL_, 512)
	,spikeBoomz(_FL_, 64)
#endif
{
	m_player = null;
	curFrame = 0;

	tickTime = CharactersArbiterTickInterval;
	dwCurrentCharIndex = 0;
	bCharacterDebug = false;

	dwCurrentCharThoughtIndex = 0;

	NeedCalcDistances = true;

	item_index = 0;

	showMap = false;
	showWayPointsOnMap = false;
	showCPtOnMap = false;
	showAIZnOnMap = false;

	showWayPoints = false;
	showCPt = false;
	showAIZn = false;

	crt_strcpy(charFilter,sizeof(charFilter),"*");
	debugInfoType = -1;

	map_scale = 100.0f;
	map_angle = 0.0f;

	show_beacon = false;

	bHairDebug = false;

	index2d = 0;
	
	gsFatalityA = null;
	gsFatalityB = null;
	gsFatalityC = null;
	gsFatalityD = null;

	playerIdleTime = null;

	npc_hp = null;
	npc_damage = null;
	player_live = null;
	gsGodMode = null;

	AnimDebugOn = false;

	stainManager = null;

	statKilledTotal = null;
	statKilledPlayer = null;
	statKilledDismember = null;
	statKilledFatalities = null;
	statKilledShot = null;
	statKilledBombed = null;
	statKilledByItems = null;
	statKilledShootered = null;

	statChestsTotal = null;
	statChestsOpened = null;

	statMoney = null;
	statShipsSunk = null;

	enemyAttackSpeedModifier = null;

	droppedWeapons.AddElements(TOTAL_DROPPED_ITEMS);

	isMultiplayer = false;

	explosionPatch = null;

#ifdef CHARACTER_STARFORCE
	timeInSecFromStart = null;
	gameTicks = null;
#endif
}

CharactersArbiter::~CharactersArbiter()
{
#ifdef CHARACTER_STARFORCE
	RELEASE(timeInSecFromStart);
	RELEASE(gameTicks);
#endif

	RELEASE(enemyAttackSpeedModifier);

	#ifndef NO_CONSOLE
	Console().UnregisterCommand("CharStats");
	Console().UnregisterCommand("CharState");	
	Console().UnregisterCommand("CharAnim");
	Console().UnregisterCommand("CharRenderParams");
	Console().UnregisterCommand("CharHP");
	Console().UnregisterCommand("CharAI");
	Console().UnregisterCommand("CharItems");
	Console().UnregisterCommand("CharActor");
	Console().UnregisterCommand("CharHide");

	Console().UnregisterCommand("CharFilter");

	Console().UnregisterCommand("AnimDebug");

	Console().UnregisterCommand("SwordDebug");

	Console().UnregisterCommand("showwaypoint");	
	Console().UnregisterCommand("showghost");	
	Console().UnregisterCommand("showalive");

	Console().UnregisterCommand("Map");	
	Console().UnregisterCommand("MapWaypoints");
	Console().UnregisterCommand("MapCombatPoints");
	Console().UnregisterCommand("MapAIZones");

	Console().UnregisterCommand("ShowWaypoints");
	Console().UnregisterCommand("ShowCombatPoints");
	Console().UnregisterCommand("ShowAIZones");

	Console().UnregisterCommand("showchars");	
	Console().UnregisterCommand("hidechars");
 
	Console().UnregisterCommand("charBeacon");

	Console().UnregisterCommand("GodMode");
	Console().UnregisterCommand("GiveMoney");
	Console().UnregisterCommand("GiveAmmo");
	Console().UnregisterCommand("GiveAll");

	Console().UnregisterCommand("Console_CharDraw");	

	Console().UnregisterCommand("CharDebug");
	Console().UnregisterCommand("HairDebug");	

	Console().UnregisterCommand("FreezeRagdolls");	

	Console().UnregisterCommand("CharAchieve");
	Console().UnregisterCommand("WatchAchieve");	
				
	#endif

	RELEASE(gmRData.bulletBuffer);
	RELEASE(gmRData.bulletTex);

	RELEASE(gmRData.flareBuffer);
	RELEASE(gmRData.flareTex1);
	RELEASE(gmRData.flareTex2);

	gmRData.vTex = NULL;
	gmRData.vPower = NULL;

	RELEASE(gsGodMode);

	RELEASE(gsFatalityA);
	RELEASE(gsFatalityB);
	RELEASE(gsFatalityC);
	RELEASE(gsFatalityD);

	RELEASE(npc_hp);
	RELEASE(npc_damage);
	RELEASE(player_live);

	RELEASE(playerIdleTime);

	if (coin_model) coin_model->Release();		
	if (bomb_model) bomb_model->Release();

	for (int i=0;i<TOTAL_DROPPED_ITEMS;i++)
	{
		TDropedItem& item = droped_items[i];

		RELEASE(item.model);
		RELEASE(item.pPhysicBox);
		RELEASE(item.tip);
	}

	stainManager = null;

	// Статистика текущей миссии
	RELEASE(statKilledTotal);
	RELEASE(statKilledPlayer);
	RELEASE(statKilledDismember);
	RELEASE(statKilledFatalities);
	RELEASE(statKilledShot);
	RELEASE(statKilledBombed);
	RELEASE(statKilledByItems);
	RELEASE(statKilledShootered);

	RELEASE(statChestsTotal);
	RELEASE(statChestsOpened);

	RELEASE(statMoney);
	RELEASE(statShipsSunk);
}

bool CharactersArbiter::Create(MOPReader & reader)
{
	SetStainManager();

	isMultiplayer = string::IsEqual("survival", api->Storage().GetString("Profile.Global.Difficulty", ""));

	if (!EditMode_IsOn()) 
	{
		//SetUpdate((MOF_UPDATE)&CharactersArbiter::Work, ML_FIRST);
		SetUpdate((MOF_UPDATE)&CharactersArbiter::Work, ML_EXECUTE1);		
		SetUpdate((MOF_UPDATE)&CharactersArbiter::EndWork, ML_EXECUTE_END);		
		SetUpdate((MOF_UPDATE)&CharactersArbiter::DeadBodiesWork, ML_EXECUTE3);		
		SetUpdate((MOF_UPDATE)&CharactersArbiter::RealizeDebug, ML_DEBUG);
	}

	tip_manager = ITipsManager::GetManager(&Mission());

	static const ConstString flyId("FlyManager");
	Mission().CreateObject(fly_manager.GetSPObject(),"FlyManager",flyId);

	for (int i = 0; i<TOTAL_DROPPED_ITEMS; i++)
	{
		droped_items[i].pPhysicBox = NULL;
		droped_items[i].model = null;

		droped_items[i].tip=null;		
		droped_items[i].fTimeLife = -1.0f;
	}

	SetUpdate((MOF_UPDATE)&CharactersArbiter::DrawDropedItems, ML_ALPHA1);

	#ifndef NO_CONSOLE		

	if (!EditMode_IsOn())
	{	
		
		Console().RegisterCommand("CharStats", "show statistic for current mission", this, (CONSOLE_COMMAND)&CharactersArbiter::Console_CharStats);
		Console().RegisterCommand("CharState", "show character id", this, (CONSOLE_COMMAND)&CharactersArbiter::Console_CharID);
		Console().RegisterCommand("CharAnim", "show character animation data", this, (CONSOLE_COMMAND)&CharactersArbiter::Console_CharAnim);
		Console().RegisterCommand("CharRenderParams", "hide character render params", this, (CONSOLE_COMMAND)&CharactersArbiter::Console_CharRenderParams);
		Console().RegisterCommand("CharHP", "show character HP", this, (CONSOLE_COMMAND)&CharactersArbiter::Console_CharHP);
		Console().RegisterCommand("CharAI", "hide character behavor", this, (CONSOLE_COMMAND)&CharactersArbiter::Console_CharAI);
		Console().RegisterCommand("CharItems", "show character item data", this, (CONSOLE_COMMAND)&CharactersArbiter::Console_CharItems);
		Console().RegisterCommand("CharActor", "show character item data", this, (CONSOLE_COMMAND)&CharactersArbiter::Console_CharActor);
		Console().RegisterCommand("CharHide", "hide character info", this, (CONSOLE_COMMAND)&CharactersArbiter::Console_CharHideInfo);

		Console().RegisterCommand("SwordDebug", "show swords debug info", this, (CONSOLE_COMMAND)&CharactersArbiter::Console_SwordDebug);

		Console().RegisterCommand("AnimDebug", "show chars anim info", this, (CONSOLE_COMMAND)&CharactersArbiter::Console_AnimDebug);

		Console().RegisterCommand("CharFilter", "Set Character Name filter for debug info", this, (CONSOLE_COMMAND)&CharactersArbiter::Console_CharSetFilter);		

		Console().RegisterCommand("Map", "show character mini map", this, (CONSOLE_COMMAND)&CharactersArbiter::Console_ShowMap);
		Console().RegisterCommand("MapWaypoints", "show/hide waypoints on min map", this, (CONSOLE_COMMAND)&CharactersArbiter::Console_ShowWaysOnMap);		
		Console().RegisterCommand("MapCombatPoints", "show/hide combatpoints on min map", this, (CONSOLE_COMMAND)&CharactersArbiter::Console_ShowCPtOnMap);
		Console().RegisterCommand("MapAIZones", "show/hide aizones on min map", this, (CONSOLE_COMMAND)&CharactersArbiter::Console_ShowAIZnOnMap);
		
		Console().RegisterCommand("ShowWaypoints", "show/hide waypoints", this, (CONSOLE_COMMAND)&CharactersArbiter::Console_ShowWays);
		Console().RegisterCommand("ShowCombatPoints", "show/hide combatpoints", this, (CONSOLE_COMMAND)&CharactersArbiter::Console_ShowCPt);		
		Console().RegisterCommand("ShowAIZones", "show/hide aizones", this, (CONSOLE_COMMAND)&CharactersArbiter::Console_ShowAIZn);				

		Console().RegisterCommand("charBeacon", "show/hide char beacon", this, (CONSOLE_COMMAND)&CharactersArbiter::Console_ShowBeacon);						

		Console().RegisterCommand("GodMode", "enable/disable GodMode", this, (CONSOLE_COMMAND)&CharactersArbiter::Console_GodMode);
		Console().RegisterCommand("GiveMoney", "GiveMoney to Greedy Player", this, (CONSOLE_COMMAND)&CharactersArbiter::Console_GiveMoney);
		Console().RegisterCommand("GiveAmmo", "Give bombs to Greedy Player", this, (CONSOLE_COMMAND)&CharactersArbiter::Console_GiveAmmo);
		Console().RegisterCommand("GiveAll", "GiveAll to Greedy Player", this, (CONSOLE_COMMAND)&CharactersArbiter::Console_GiveAll);
				
		Console().RegisterCommand("CharShowPair", "Show chars that in Pair MiniGame with Player", this, (CONSOLE_COMMAND)&CharactersArbiter::Console_ShowPairMiniGame);

		Console().RegisterCommand("CharDraw", "Enable/Disable char draw", this, (CONSOLE_COMMAND)&CharactersArbiter::Console_CharDraw);
		
		Console().RegisterCommand("CharDebug", "auto breakpoint on char tick", this, (CONSOLE_COMMAND)&CharactersArbiter::Console_CharDebug);		

		Console().RegisterCommand("HairDebug","Enable/Disable char hair debug", this,(CONSOLE_COMMAND)&CharactersArbiter::Console_CharHairDebug);

		Console().RegisterCommand("CharAchieve","Show character achievements", this,(CONSOLE_COMMAND)&CharactersArbiter::Console_ShowAchievements);

		Console().RegisterCommand("WatchAchieve","Watch state of achievement", this,(CONSOLE_COMMAND)&CharactersArbiter::Console_WatchAchieve);		

		Console().RegisterCommand("FreezeRagdolls", "Enable/Disable freezing ragdolls", this, (CONSOLE_COMMAND)&CharactersArbiter::Console_FreezeRagdolls);
	}

	#endif

	gmRData.vPower = GetRender().GetTechniqueGlobalVariable("fPower", _FL_);	
	gmRData.vTex = GetRender().GetTechniqueGlobalVariable("tBulletTexture", _FL_);	

	gmRData.flareBuffer = GetRender().CreateVertexBuffer( 4*sizeof(Vertex), sizeof(Vertex), _FL_, USAGE_WRITEONLY);

	if (gmRData.flareBuffer)
	{
		Vertex * vrt = (Vertex *)gmRData.flareBuffer->Lock(0, 4*sizeof(Vertex), 0);		

		vrt[0].p = Vector (-0.5,0.5,0.0);	
		vrt[0].u = 0.0f;
		vrt[0].v = 0.0f;

		vrt[1].p = Vector ( 0.5,0.5,0.0);	
		vrt[1].u = 1.0f;
		vrt[1].v = 0.0f;

		vrt[2].p = Vector (-0.5,-0.5,0.0);	
		vrt[2].u = 0.0f;
		vrt[2].v = 1.0f;

		vrt[3].p = Vector ( 0.5,-0.5,0.0);	
		vrt[3].u = 1.0f;
		vrt[3].v = 1.0f;

		vrt[0].color = vrt[1].color = vrt[2].color = vrt[3].color = 0xffffffff;

		gmRData.flareBuffer->Unlock();
	}

	gmRData.flareTex1 = GetRender().CreateTexture(_FL_, "fx_Flare.txx");
	gmRData.flareTex2 = GetRender().CreateTexture(_FL_, "fx_Flare2.txx");	
		
	gmRData.bulletBuffer = GetRender().CreateVertexBuffer( 12*sizeof(Vertex), sizeof(Vertex), _FL_, USAGE_WRITEONLY);

	if (gmRData.bulletBuffer)
	{
		float scaleV = 0.05f;
		float scaleH = 1.5f;

		Vertex * vrt = (Vertex*)gmRData.bulletBuffer->Lock(0, 12*sizeof(Vertex), 0);		

		vrt[0].p = Vector (0.0, scaleV,0.0);	
		vrt[0].u = 1.0f;
		vrt[0].v = 0.0f;

		vrt[1].p = Vector (0.0, scaleV,scaleH);
		vrt[1].u = 0.0f;
		vrt[1].v = 0.0f;

		vrt[2].p = Vector (0.0,-scaleV,scaleH);	
		vrt[2].u = 0.0f;
		vrt[2].v = 1.0f;

		vrt[3].p = Vector (0.0,-scaleV,scaleH);	
		vrt[3].u = 0.0f;
		vrt[3].v = 1.0f;

		vrt[4].p = Vector (0.0,-scaleV,0.0);
		vrt[4].u = 1.0f;
		vrt[4].v = 1.0f;

		vrt[5].p = Vector (0.0, scaleV,0.0);	
		vrt[5].u = 1.0f;
		vrt[5].v = 0.0f;	

		for (int i=0;i<6;i++)
		{
			vrt[i+6]=vrt[i];

			vrt[i+6].p.y=0.0f;
			vrt[i+6].p.x=vrt[i].p.y;
		}

		for (int i=0;i<12;i++)
		{
			vrt[i].color = 0xffffffff;
		}

		gmRData.bulletBuffer->Unlock();
		gmRData.bulletTex = GetRender().CreateTexture(_FL_, "gun_fire.txx");
	}


	IFileService* pFS = (IFileService*)api->GetService("FileService");

	IIniFile* pEngineIni = pFS->SystemIni();

	if (pEngineIni)
	{
		bCharacterDebug = pEngineIni->GetLong("Mission", "CharacterDebug", 1) > 0;
	}
	
	coin_model =  Geometry().CreateGMX("Signs\\i_Fatality_Scull.gmx", &Animation(), &Particles(), &Sound());

	if (coin_model) coin_model->SetDynamicLightState(true);	
	
	bomb_model =  Geometry().CreateGMX("Weapons\\w_bomb", &Animation(), &Particles(), &Sound());
	if (bomb_model) bomb_model->SetDynamicLightState(true);

	//Registry(MG_SHADOWRECEIVE, &CharactersArbiter::ReciveShadowDropedItems, ML_DYNAMIC2);

	ICoreStorageString* var = api->Storage().GetItemString("Profile.Global.Difficulty",_FL_);

	if (var)
	{
		if (var->IsValidate())
		{
			level_difficulty_name.Set(var->Get("Normal"));
		}

		var->Release();
	}
	else
	{
		level_difficulty_name.Set("Normal");
	}

	// достаем ускорение атаки врагов для разных режимов сложности
	char tmp[1024];
	crt_snprintf(tmp, sizeof(tmp), "Profile.%s.NPC.EnemyAttackSpeed", level_difficulty_name.c_str());
	enemyAttackSpeedModifier = api->Storage().GetItemFloat(tmp, _FL_);

#ifdef CHARACTER_STARFORCE
	gameTicks = api->Storage().GetItemFloat("Profile.Global.Ticks", _FL_);
#endif

	GetStateFloatVar(CorrectStateId("Player.fatality.A"),gsFatalityA);
	GetStateFloatVar(CorrectStateId("Player.fatality.B"),gsFatalityB);
	GetStateFloatVar(CorrectStateId("Player.fatality.C"),gsFatalityC);
	GetStateFloatVar(CorrectStateId("Player.fatality.D"),gsFatalityD);

	GetStateFloatVar(CorrectStateId("NPC.HP"),npc_hp);
	GetStateFloatVar(CorrectStateId("NPC.Damage"),npc_damage);
	GetStateFloatVar(CorrectStateId("Player.live"),player_live);

	GetStateFloatVar("Runtime.GodMode",gsGodMode);
	WriteGameState(gsGodMode, 0);

	// Статистика текущей миссии
	GetStatVar(statKilledTotal, "Killed.Total");
	GetStatVar(statKilledPlayer, "Killed.Player");
	GetStatVar(statKilledDismember, "Killed.Dismember");
	GetStatVar(statKilledFatalities, "Killed.Fatalities");
	GetStatVar(statKilledShot, "Killed.Shot");
	GetStatVar(statKilledBombed, "Killed.Bombed");
	GetStatVar(statKilledByItems, "Killed.ByItems");
	GetStatVar(statKilledShootered, "Killed.Shootered");

	GetStatVar(statChestsTotal, "Chests.Total");
	GetStatVar(statChestsOpened, "Chests.Opened");

	GetStatVar(statMoney, "Money");
	GetStatVar(statShipsSunk, "ShipsSunk");

	playerIdleTime = api->Storage().GetItemFloat("Runtime.Global.IdleTime", _FL_);
	playerIdleTime->Set(0.0f);

#ifdef CHARACTER_STARFORCE
	//guardingValueDamage = NEW float[5];
	nSpikeValues = 5;
	for (int i=0; i<5; i++)
		guardingValueDamage[i] = 73.0f;
#endif

	return true;
}

bool CharactersArbiter::SetStainManager()
{
	MOSafePointer obj;
	static const ConstString objectId("StainManager");
	Mission().CreateObject(obj,"StainManager", objectId);
	stainManager = (IStainManager*)obj.Ptr();
	Assert(stainManager);

#ifdef CHARACTER_STARFORCE
	pOceans = (IOcean3Service *)api->GetService("Ocean3Service"); Assert(pOceans);
	dwProtectValue = pOceans->ProtectValue();
	dword value = 128 * (dword(pOceans) - 1000 + 128);
	guardCloud = 2.0f;//((value == pOceans->ProtectValue() || ((rand() % 5) == 2))) ? 2.0f : 3.0f;

	RELEASE(timeInSecFromStart);
	const char* difficulty = api->Storage().GetString("profile.global.difficulty", NULL);
	if (difficulty)
	{
		char tmpBuff[512];
		crt_snprintf(tmpBuff, sizeof(tmpBuff), "Profile.%s.time.totalSec", difficulty);
		timeInSecFromStart = api->Storage().GetItemFloat(tmpBuff, _FL_);
	}

	if (!timeInSecFromStart)
		timeInSecFromStart = api->Storage().GetItemFloat("Profile.normal.time.totalSec", _FL_);
#endif

	return true;
}

void CharactersArbiter::PostCreate()
{
	MissionObject * mo = Mission().Player();
	if (mo)
	{
		MO_IS_IF(is_Character, "Character", mo)
		{
			m_player = (Character*)mo;
		}
		else
			m_player = null;
	}

	explosionPatch = IExplosionPatch::GetExplosionPatch(Mission());

	MGIterator* iter;
	iter = &Mission().GroupIterator(GroupId('C','h','G','R'), _FL_);
	for (;!iter->IsDone();iter->Next())
	{
		all_characters.Add((Character*)iter->Get());
	}
	iter->Release();
}

void CharactersArbiter::StatAdd(ICoreStorageFloat & var, float value)
{
	var.Set(var.Get() + value);
}

void _cdecl CharactersArbiter::ReciveShadowDropedItems(const char * group, MissionObject * sender)
{
	DrawDropedItems(0,0);	
}

__forceinline Vector CharactersArbiter::GetVector(Character * from, Character * to)
{
	return (to->last_frame_pos - from->last_frame_pos);
}

float CharactersArbiter::GetPowDistance(Character * from, Character * to)
{
	return (from->last_frame_pos - to->last_frame_pos).GetLength2();
}

/*float CharactersArbiter::GetPowDistance(Character* from, Character* to)
{
	float dist = (from->last_frame_pos - to->last_frame_pos).GetLength2();

	for (int i=0;i<(int)distTable.Size();i++)
	if (distTable[i].owner == from)
	{
		for (int j=0;j<(int)distTable[i].table.Size();j++)
		if (distTable[i].table[j].chr == to)
		{
			float dist2 = distTable[i].table[j].dist_pow;
			return dist2;
		}
	}

	return 0.0f;
}*/

void CharactersArbiter::CalcDistances()
{
	const Vector camPos = GetRender().GetView().GetCamPos();

	for (int i=0; i<characters.Len(); i++)
	{
		Matrix mtx = characters[i]->physics->GetMatrixWithBoneOffset(mtx);		
		characters[i]->last_frame_pos = mtx.pos;
		characters[i]->last_dist2camera = (camPos - mtx.pos).GetLength2();
	}

	/*for (int i=0;i<(int)distTable.Size();i++)
	{
		Matrix mtx = distTable[i].owner->physics->GetMatrixWithBoneOffset(mtx);		
		Vector pos = mtx.pos;

		distTable[i].owner->last_frame_pos = pos;

		distTable[i].table[i].dist_pow = 0.0f;

		for (int j=i+1;j<distTable[i].table;j++)
		{
			Matrix mtx =  distTable[i].table[j].chr->physics->GetMatrixWithBoneOffset(mtx);			
			Vector pos2 = mtx.pos;

			distTable[i].table[j].dv  = (pos2 - pos);
			distTable[i].table[j].dist_pow   = (pos2 - pos).GetLength2();
			//distTable[i].table[j].dist = distTable[i].table[j].dir.Normalize();


			//distTable[j].table[i].dir  = -distTable[i].table[j].dir;
			distTable[j].table[i].dv   = -distTable[i].table[j].dv;
			distTable[j].table[i].dist_pow=  distTable[i].table[j].dist_pow;
		}
	}*/

#ifdef CHARACTER_STARFORCE
	if (timeInSecFromStart->Get(0.0f) > 1812.0)
		SetSpikeData(dwProtectValue);
#endif

	NeedCalcDistances = false;
}

void _cdecl CharactersArbiter::EndWork(float dltTime, long level)
{
	curFrame++;

	for (long i = 0; i < characters; i++)
	{
		if (characters[i]->logic)
			characters[i]->logic->NeedStateUpdate();
	}

#ifdef CHARACTER_STARFORCE
	if (gameTicks->Get(0.0f) > 1812.0)
		SetSpikeData(pOceans->ProtectValue());
#endif
}

#ifdef CHARACTER_STARFORCE
__forceinline void CharactersArbiter::SetSpikeData(dword protectValue)
{
	dword value = 128 * (dword(pOceans) - 1000 + 128);
	int i;
	for (i=0; i<nSpikeValues; i++)
	{
		bool goodValue = (value == protectValue || ((rand() % 5) == 2));
		guardingValueDamage[i] = (goodValue) ? 73.0f : RRnd(15.0f, 70.0f);
		
		if (i==0)
			guardCloud = ((value == pOceans->ProtectValue() || ((rand() % 5) == 2))) ? 2.0f : 3.0f;
		}
	}
	nSpikeValues -= i;
}
#endif

void _cdecl CharactersArbiter::Work(float dltTime, long level)
{
#ifdef CHARACTER_STARFORCE
	gameTicks->Set(gameTicks->Get(0.0f) + dltTime * 1.0127f);
#endif

	time.Execute(dltTime);

	index2d = 0;

#ifdef CHARACTER_STARFORCE
	Matrix mtx(true);
	Vector playerPos = 0.0f;
	if (GetPlayer())
		playerPos = GetPlayer()->GetMatrix(mtx).pos;

	for (int i=0; i<spikeStars.Len(); i++)
	{
		SpikeStar & ss = spikeStars[i];
		ss.scale += dltTime * 0.03f;
		ss.time -= dltTime;
		ss.pos += (!(playerPos - ss.pos)) * ss.speed * dltTime * 0.5f;
		spikeDamagePos = ss.pos - Vector(0.0f, 0.5f * ss.scale, 0.0f);
		ss.ps->SetTransform(Matrix().BuildPosition(spikeDamagePos));
		ss.ps->SetScale(ss.scale);

		if (rand()%50 == 25)
		{
			spikeBoomz.Add(spikeDamagePos);
			SetUpdate((MOF_UPDATE)&CharactersArbiter::SpikeBoom, ML_EXECUTE1);
		}

		if (ss.time <= 0.0f)
		{
			ss.ps->Release();
			spikeStars.DelIndex(i);
			i--;
			continue;
		}
	}
#endif

	CalcDistances();

	DWORD dwStartTicks = GetTickCount();

	dword dwActiveChars = 0;			

	if (characters.Size() > 0)
	{		
		dword dwMaxCharsToProcess = 5;
		dwMaxCharsToProcess = coremin (dwMaxCharsToProcess, characters.Size());

		for (dword dwProcessedChars = 0; dwProcessedChars < dwMaxCharsToProcess; dwProcessedChars++)
		{
			//ProfileTimer timer;								

			//Если текущий за пределами массива сначала начать
			if (dwCurrentCharIndex >= characters.Size()) dwCurrentCharIndex = 0;

			//Берем текущего
			Character* chr = characters[dwCurrentCharIndex];

			Assert(chr);

			//Передвигаем индекс
			dwCurrentCharIndex++;

			//Подходит ли для исполнения
			if(chr->logic->IsPairMode()) continue;
			if(chr->logic->IsActor()) continue;
			if(chr->EditMode_IsOn()) continue;

			if(chr->logic->GetState() == CharacterLogic::state_die) continue;
			if(chr->logic->GetState() == CharacterLogic::state_dead) continue;
			if(chr->logic->GetHP() <= 0.0f) continue;
			if(chr->ragdoll) continue;				


			if (chr->controller->StopWhenDebug())
			{
				int k=0;

				k++;
			}


			//Исполняем
			dwActiveChars++;
			chr->controller->Tick();


			/*timer.Stop();
			unsigned __int64 i64 = timer.time64;

			if (i64>350000)
			{
				api->Trace("Warning: Chracter - %s is real pain in ass %i",chr->GetObjectID().c_str(),i64);
			}*/

			//Если прошло много времени выходим
			DWORD dwNowTicks = GetTickCount();
			if ((dwNowTicks - dwStartTicks) > 5) break;

		}
	}

/*
	tickTime += dltTime;
	if(tickTime >= CharactersArbiterTickInterval)
	{
		tickTime -= CharactersArbiterTickInterval;
		for(long i = 0; i < characters; i++)
		{
			Character * chr = characters[i];
			if(chr->logic->IsPairMode()) continue;
			if(chr->logic->IsActor()) continue;
			if(chr->EditMode_IsOn()) continue;

			string pName = characters[i]->GetObjectID();
			dwActiveChars++;
			characters[i]->controller->Tick();
		}
	}
*/

	//GetRender().PrintBuffered(0, 20, 0xFFFFFFFF, "Arbiter execute %d characters. Total %d", dwActiveChars, characters.Size());

	for(long i = 0; i < characters; i++)
	{
		AttackChrItems(characters[i]);		
	}

#ifdef CHARACTER_STARFORCE
	if (gameTicks->Get(0.0f) > 1900.0f && GetPlayer())
	{
		double guarding = 73.0 - GuardingValueDamage<double>(2);
		AssertStarForce(fabs(guarding) < 1e-5);
		if (rand()%500 == 251 && guarding > 1.0)
		{
			char slowmoK[256];
			crt_snprintf(slowmoK, sizeof(slowmoK), "%.2f", RRnd(0.05f, 0.2f));
			((PlayerController*)GetPlayer()->controller)->StartSloMo(RRnd(5.0f, 120.0f), slowmoK);
		}
	}

	if (timeInSecFromStart->Get(0.0f) > 1790.0f)
		SetSpikeData(dwProtectValue);
#endif

	// считаем время вне боя(и в режиме игры) для плеера

	Character * player = GetPlayer();
	bool isPlayerIdle = false;
	if (player && player->logic)
	{
		bool isQEvent = ((PlayerController*)(player->controller))->qevent != null;
		bool isActor = player->logic->IsActor();
		bool isPairMode = player->logic->IsPairMode();
		bool isActive = player->IsActive() && player->IsShow();

		if (isActive && !isActor && !isPairMode && !isQEvent)
		{
			isPlayerIdle = true;
			FindCircle(*player, 15.0f);
			for (int i=0; i<find.Len(); i++)
			{			
				if (!find[i].chr->IsActive()) continue;
				if (!find[i].chr->IsShow()) continue;
				if (find[i].chr->logic->IsDead()) continue;
				if (find[i].chr->logic->IsActor()) continue;
				if (!find[i].chr->logic->IsRealEnemy(player, false)) continue;
				isPlayerIdle = false;
				break;
			}
		}
	}

#ifdef CHARACTER_STARFORCE
	if (timeInSecFromStart->Get(0.0f) > 1900.0f)
	{
		float probFloat = float(int(GetTime().GetTimeFloat() / 90.0f));
		AssertStarForce(fabsf(guardCloud - 2.0f) < 1e-5f);
		int probability = Min(50, int(GetTime().GetTimeFloat() / 10.0f));
		if (rand()%500 < probability*probFloat*(guardCloud - 2.0f) && dltTime > 1e-4f)
		{
			SpikeStar & ss = spikeStars[spikeStars.Add()];
			ss.ps = Particles().CreateParticleSystemEx("rage", _FL_);

			ss.pos = playerPos;
			ss.pos += Vector().RandXZ() * RRnd(22.0f, 45.0f);
			ss.time = RRnd(20.0f, 40.0f);
			ss.scale = RRnd(0.1f, 0.3f);
			ss.speed = RRnd(1.0f, 3.0f);

			ss.ps->SetTransform(Matrix().BuildPosition(ss.pos));
			ss.ps->SetScale(ss.scale);
		}
	}
#endif

	if (isPlayerIdle)
		playerIdleTime->Set(playerIdleTime->Get() + dltTime);
	else
		playerIdleTime->Set(0.0f);
}

#ifdef CHARACTER_STARFORCE
void _cdecl CharactersArbiter::SpikeBoom()
{
	for (int i=0; i<spikeBoomz.Len(); i++)
		Boom(this, DamageReceiver::ds_cannon, spikeBoomz[i], 1.2f, 5.0f, 2.0f);

	spikeBoomz.DelAll();
	DelUpdate((MOF_UPDATE)&CharactersArbiter::SpikeBoom);
}
#endif

void CharactersArbiter::AttackChrItems(Character* chr)
{
	if (chr)
	{
		if (chr->IsActive())
		{
			chr->items->AttackProcess(characters);
		}		
	}	
}

void CharactersArbiter::DrawSprite(Vector p, float sz, dword color)
{
	RS_SPRITE spr[4];

	Character* player = GetPlayer();

	Vector pos = p;

	if (player)
	{
		pos = p - player->physics->GetPos();
	}

	float fPX =  pos.x/map_scale;
	float fPY =  pos.z/map_scale;

	float fPosX = cos(map_angle) * fPX - sin(map_angle)*fPY;
	float fPosY = sin(map_angle) * fPX + cos(map_angle)*fPY;

	spr[0].vPos = Vector (fPosX-sz*0.75f, fPosY+sz, 0.0f);
	spr[1].vPos = Vector (fPosX+sz*0.75f, fPosY+sz, 0.0f);
	spr[2].vPos = Vector (fPosX+sz*0.75f, fPosY-sz, 0.0f);
	spr[3].vPos = Vector (fPosX-sz*0.75f, fPosY-sz, 0.0f);

	spr[0].tv = 0.0f;
	spr[0].tu = 0.0f;

	spr[1].tv = 0.0f;
	spr[1].tu = 1.0f;

	spr[2].tv = 1.0f;
	spr[2].tu = 1.0f;

	spr[3].tv = 1.0f;
	spr[3].tu = 0.0f;

	for (int j=0;j<4;j++)
	{
		spr[j].dwColor = color;
	}

	GetRender().DrawSprites(NULL,spr, 1);
}

//Отладочная информация
void CharactersArbiter::DrawLine(Vector v1, Vector v2, dword color)
{
	Character* player = GetPlayer();

	Vector pos = v1;
	
	if (player)
	{
		pos = v1 - player->physics->GetPos();
	}

	float fPX =  pos.x/map_scale;
	float fPY =  pos.z/map_scale;

	float fPosX = cos(map_angle) * fPX - sin(map_angle)*fPY;
	float fPosY = sin(map_angle) * fPX + cos(map_angle)*fPY;	

	pos = v2;
	
	if (player)
	{
		pos = v2 - player->physics->GetPos();
	}

	float fPX2 =  pos.x/map_scale;
	float fPY2 =  pos.z/map_scale;

	float fPosX2 = cos(map_angle) * fPX2 - sin(map_angle)*fPY2;
	float fPosY2 = sin(map_angle) * fPX2 + cos(map_angle)*fPY2;	

	Draw2DLine(&Render(),fPosX,fPosY,fPosX2,fPosY2,color);
}

//Отладочная информация
void _cdecl CharactersArbiter::DrawMap(float dltTime, long level)
{		
	if (api->DebugKeyState('N'))
	{
		map_scale += 10.0f;

		if (map_scale>1000.0f)
		{
			map_scale = 1000.0f;
		}
	}

	if (api->DebugKeyState('M'))
	{
		map_scale -= 10.0f;

		if (map_scale<25.0f)
		{
			map_scale = 25.0f;
		}
	}

	Character* player = GetPlayer();

	RENDERVIEWPORT vp = GetRender().GetViewport();

	RENDERVIEWPORT new_vp;

	new_vp.Height = vp.Height/2;
	new_vp.Width = vp.Width/2;

	new_vp.X = vp.X + new_vp.Width;
	new_vp.Y = vp.Y + new_vp.Height;	

	new_vp.MaxZ = vp.MaxZ;	
	new_vp.MinZ = vp.MinZ;	
	
	GetRender().SetViewport(new_vp);


	Matrix m_view = GetRender().GetView();
	m_view.Inverse();
	Vector dir = Vector(1,0,0);
		
	map_angle = dir.GetAngleXZ(m_view.vx);;
	if (dir.x<0.0f) map_angle=2*PI-map_angle;	

	if (player)
	{
		DrawSprite( player->physics->GetPos(), 1.5f, 0xaaaaaaaa);
	}


	if (showWayPointsOnMap)
	{
		MGIterator& iter = Mission().GroupIterator(WAY_POINT_GROUP, _FL_);

		for (;!iter.IsDone();iter.Next())
		{
			IPathFinder* path = (IPathFinder*)iter.Get();

			for (int j = 0; j < (int)path->GetGraphSize(); j++)
			{
				if (path->IsPointDeleted(j)) continue;							

				for (int n = 0; n < (int)path->GetLinksSize(j); n++)
				{
					DrawLine(path->GetPointPos(j),
							 path->GetPointPos(path->GetLink(j, n)), 0xff0000ff);
				}
			}			
		}

		iter.Release();
	}	



	if (showCPtOnMap)
	{
		MGIterator& iter = Mission().GroupIterator(COMBAT_POINTS_GROUP, _FL_);

		for (;!iter.IsDone();iter.Next())
		{
			CombatPoints* cpoint = (CombatPoints*)iter.Get();

			for (int i=0;i<(int)cpoint->Points.Size();i++)
			{
				DrawSprite( cpoint->Points[i].position, 0.025f * cpoint->Points[i].radius, 0xFF0000AA);
			}
		}

		iter.Release();
	}

	if (showAIZnOnMap)
	{
		MGIterator& iter = Mission().GroupIterator(AIZones_GROUP, _FL_);

		for (;!iter.IsDone();iter.Next())
		{
			AIZones* aiZones = (AIZones*)iter.Get();

			for (int i=0;i<(int)aiZones->zones.Size();i++)
			{
				AIZones::Zone* zn = &aiZones->zones[i];
				
				DrawLine(Vector ( zn->max.x,0.0f, zn->max.z) * zn->mtx,
					     Vector ( zn->max.x,0.0f,-zn->max.z) * zn->mtx,0xff009900);
				DrawLine(Vector ( zn->max.x,0.0f,-zn->max.z) * zn->mtx,
					     Vector (-zn->max.x,0.0f,-zn->max.z) * zn->mtx,0xff009900);
				DrawLine(Vector (-zn->max.x,0.0f,-zn->max.z) * zn->mtx,
						 Vector (-zn->max.x,0.0f, zn->max.z) * zn->mtx,0xff009900);
				DrawLine(Vector (-zn->max.x,0.0f, zn->max.z) * zn->mtx,
						 Vector ( zn->max.x,0.0f, zn->max.z) * zn->mtx,0xff009900);								
			}
		}

		iter.Release();	
	}
	
	for(long i = 0; i < characters; i++)
	{
		if(characters[i]->IsActive())
		{
			{		
				dword color = 0xFFFFFFFF;

				if (!player)
				{
					color = 0xFF00FFFF;
				}
				else
				if (characters[i]->IsPlayer())
				{
						color = 0xFF00FF00;
				}
				else
				if (characters[i]->logic->IsActor())
				{
					if (!characters[i]->logic->IsEnemy(player))
					{
						color = 0xFF999900;
					}
					else
					{
						color = 0xFFFFFF00;
					}
				}
				else
				if (characters[i]->logic->IsEnemy(player))
				{
					if (characters[i]->controller->GetChrTarget() == player)
					{
						color = 0xFFFF0000;
					}
					else
					{
						color = 0xFFFF00FF;
					}						
				}
				else
				if (!characters[i]->logic->IsEnemy(player))
				{
					color = 0xFF00FFFF;
				}				

				DrawSprite( characters[i]->physics->GetPos(), 0.025f, color);				
			}
		}	
	}

	for(long i = 0; i < characters; i++)
	{
		if(characters[i]->IsActive())
		{
			if (characters[i]->controller->GetChrTarget())
			{				
				DrawLine(characters[i]->physics->GetPos(), characters[i]->controller->GetChrTarget()->physics->GetPos(), 0xff0000ff);
			}			
		}
	}

	GetRender().SetViewport(vp);
}

void _cdecl CharactersArbiter::RealizeDebug(float dltTime, long level)
{
	if (!charStatsShow)
		return;

	GetRender().Print(10.0f, 50.0f, 0xFFFFFFFF, "Total killed: %.0f", statKilledTotal->Get());
	GetRender().Print(10.0f, 70.0f, 0xFFFFFFFF, "Player killed: %.0f", statKilledPlayer->Get());
	GetRender().Print(10.0f, 90.0f, 0xFFFFFFFF, "Player shot: %.0f", statKilledShot->Get());
	GetRender().Print(10.0f, 110.0f, 0xFFFFFFFF, "Player bombed: %.0f", statKilledBombed->Get());
	GetRender().Print(10.0f, 130.0f, 0xFFFFFFFF, "Player items: %.0f", statKilledByItems->Get());
	GetRender().Print(10.0f, 150.0f, 0xFFFFFFFF, "Player shooter: %.0f", statKilledShootered->Get());
	GetRender().Print(10.0f, 170.0f, 0xFFFFFFFF, "Player dismember: %.0f", statKilledDismember->Get());
	GetRender().Print(10.0f, 190.0f, 0xFFFFFFFF, "Player fatalities: %.0f", statKilledFatalities->Get());

	GetRender().Print(10.0f, 230.0f, 0xFFFFFFFF, "Chest total: %.0f", statChestsTotal->Get());
	GetRender().Print(10.0f, 250.0f, 0xFFFFFFFF, "Chest opened: %.0f", statChestsOpened->Get());

	GetRender().Print(10.0f, 290.0f, 0xFFFFFFFF, "Money earned: %.0f", statMoney->Get());
}

void _cdecl CharactersArbiter::AnimDebug(float dltTime, long level)
{
	int index = 0;

	for (int i=0;i<(int)all_characters.Size();i++)
	{
		if (all_characters[i]->IsActive() && all_characters[i]->IsShow())
		{
			Vector pos = all_characters[i]->physics->GetPos(true);

			if (IsSphereInFrustrum(pos, 0.1f))
			{
				pos.y += 1.0f;
				PrintTextShadowed(pos, 0, 0xffffffff, "%i", i);

				AGNA_GetCurrentFrame frame;
				IAnimation* anim = all_characters[i]->animation->GetCurAnimation();

				if (anim)
				{
					anim->GetNativeGraphInfo(frame);				

					GetRender().Print(10,(float)10 + index*15.0f,0xffffffff,"%i - %s : %s, frame %4.1f",
								   i, anim->GetName(), anim->CurrentNode(),frame.currentFrame);

					index++;
				}
			}
		}
	}
}

//============================================================================================
//Управление персонажами
//============================================================================================

//Активировался-деактивировался персонаж
void CharactersArbiter::Activate(Character * chr, bool isActive)
{
	Assert(chr);
	dword index = characters.Find(chr);

	if(isActive)
	{		
		if(index == INVALID_ARRAY_INDEX)
		{
			/*for (int i=0;i<(int)distTable.Size();i++)			
			{
				TCharDist* charDist = &distTable[i].table[distTable[i].table.Add()];
				charDist->chr = chr;				
			}*/

			characters.Add(chr);
			

			/*TCharDistTable* pTable = &distTable[distTable.Add()];

			pTable->owner = chr;
			for (int i=0;i<(int)characters.Size();i++)			
			{
				TCharDist* charDist = &pTable->table[pTable->table.Add()];
				charDist->chr = characters[i];				
			}*/
		}

		NeedCalcDistances = true;
	}
	else
	{		
		if (index != INVALID_ARRAY_INDEX)
		{		
			//distTable.DelIndex(index);
			characters.Del(chr);

			/*for (int j=0;j<(int)distTable.Size();j++)
			{
				distTable[j].table.DelIndex(index);
			}*/
		}		
	}
}


//Найти персонажи в заданном секторе и дальности
void CharactersArbiter::Find(Character & chr, float rmin, float rmax, bool CheckHeight, float amin, float amax, float backMove, float maxHeightDiff)
{	
	if (NeedCalcDistances) CalcDistances();

	find.DelAll();

	Vector dir(0.0f,0.0f,1.0f);
	dir.Rotate(chr.physics->GetAy());

	float fObjectRadius = 0.2f;
	Matrix m;

	float cs = cosf(amax);

	rmin += 0.5f;
	rmax -= 0.5f;

	const array<Character *> &chars  = chr.arbiter->GetActiveCharacters();

	for (int i=0;i<(int)chars.Size();i++)
	{
		Character * pObject = chars[i];

		if (&chr==pObject) continue;
		if (!pObject->IsActive()) continue;
		if (pObject->logic->IsActor()) continue;
		if (pObject->logic->IsPairMode()) continue;

		//Если есть отсечение по высоте - то отсекаем
		if (fabsf(chr.physics->GetPos().y - pObject->physics->GetPos().y) > maxHeightDiff && CheckHeight)
			continue;

		Vector objPos = pObject->GetMatrix(m).pos - (chr.physics->GetPos()  - dir * backMove);

		float r = objPos.Normalize();

		if ((objPos | dir)<cs) continue;		

		if (rmin>r || r>rmax) continue;


		dword n = find.Add();
		CharacterFind & f = find[n];
		f.chr = pObject;
		f.dv = objPos;
		f.dist_pow = f.dv.GetLength2();		
	}

/*

	//Очищаем массив найденных
	find.DelAll();
	if(amin > amax || rmin > rmax) return;
	if(!chr.IsActive()) return;
	//Генерим массив векторов для проверки угла
	Vector vt[4];
	long vtCount = 0;
	float dAngle = amax - amin;
	if(dAngle < 2.0f*PI)
	{
		//Если ограничение
		dAngle *= 1.0f/(0.8f*PI);
		vtCount = long(dAngle) + 1;
		dAngle = (amax - amin)/float(vtCount);
		float a = amin + chr.physics->GetAy();
		for(long j = 0; j <= vtCount; j++)
		{
			vt[j].MakeXZ(a);
			a += dAngle;
		}
		vtCount++;
	}

	rmin *= rmin;
	rmax *= rmax;

	Matrix mat;
	chr.physics->GetModelMatrix(mat);
	
	
	Vector pos = mat.pos;

	if(backMove > 0.0f)
	{
		pos -= Vector().MakeXZ(chr.physics->GetAy())*backMove;
		rmin += backMove;
		rmax += backMove;
	}
	
	for(long i = 0; i < characters; i++)
	{
		Character * c = characters[i];
		if(&chr == c) continue;

		if(c->logic->IsActor()) continue;

		dword n = find.Add();
		CharacterFind & f = find[n];
		f.chr = c;
		f.dv = c->physics->GetPos() - pos;
		f.dist_pow = f.dv.GetLength2();
		
		//Проверяем по дистанции
		if(f.dist_pow < rmin || f.dist_pow > rmax)
		{
			find.DelIndex(n);
			continue;
		}
		//Проверяем по углам
		long j = 0;
		if(vt[j++].CrossXZ(f.dv) < 0.0f) j = vtCount;
		while(j < vtCount)
		{
			if(vt[j].CrossXZ(f.dv) <= 0.0f) break;
			j++;
		}
		if(j == vtCount)
		{
			find.DelIndex(n);
			continue;
		}
		
		//Если в области, то оставляем его
		if (fabs(chr.physics->GetPos().y - c->physics->GetPos().y)>0.1f && CheckHeight)
		{
			find.DelIndex(n);
			continue;
		}
	}*/
}

//Найти персонажей в заданном радиусе
void CharactersArbiter::FindCircle(Character & chr, float fRadius)
{
	fRadius *= fRadius;

	if (NeedCalcDistances) CalcDistances();

	//Очищаем массив найденных
	find.Empty();	

	//dword index = characters.Find(&chr);	

	//if(index == INVALID_ARRAY_INDEX) return;

	for(long i = 0; i < characters; i++)
	{
		Character * c = characters[i];
		if(&chr == c) continue;		

		float dist_pow = GetPowDistance(&chr, c);//distTable[index].table[i].dist_pow;
		
		//Проверяем по дистанции
		if( dist_pow < fRadius)
		{
			dword n = find.Add();
			CharacterFind* f = &find[n];		

			f->chr = c;
			f->dv = GetVector(&chr, c);//distTable[index].table[i].dv;	
			f->dist_pow = dist_pow;
			//f->dist = distTable[index].table[i].dist;
		}		
	}
}

//Найти персонажей в заданном радиусе
void CharactersArbiter::FindCircle(const Vector& pos, float fRadius)
{	
	fRadius *= fRadius;
	
	//Очищаем массив найденных
	find.Empty();	

	Vector dir;
	float dist_pow;

	for(long i = 0; i < characters; i++)
	{
		Character * c = characters[i];

		if (c->logic->IsActor()) continue;

		dir = pos - c->physics->GetPos();
		dir.y -= 1.0f;

		dist_pow = dir.GetLength2();
		
		if (dist_pow<fRadius)
		{			
			dword n = find.Add();
			CharacterFind* f = &find[n];			

			f->chr = c;			
			f->dist_pow = dist_pow;
			f->dv = dir;
		}		
	}
}

bool  CharactersArbiter::IsActiveCharacter(MissionObject* mo)
{
	for(long i = 0; i < characters; i++)
	{
		if (characters[i] == mo) return true;
	}

	return false;
}

bool CharactersArbiter::IfAnyCharacer(Character* chr,const Vector& pos,float radius,Vector &char_pos)
{
	for(long i = 0; i < characters; i++)
	{
		Character * c = characters[i];

		if (c == chr) continue;
		
		if (c->logic->IsActor()) continue;
		if (c == GetPlayer()) continue;
		//if (c->logic->GetHP()<0.1f) continue;
	
		if (((pos - c->physics->GetPos() - Vector(0,c->physics->GetHeight(),0)).GetLength() - c->physics->GetRadius())<radius)
		{
			char_pos = c->physics->GetPos(true);

			return true;
		}
	}

	return false;		
}

bool CharactersArbiter::IfEnemyCharacer(Character* chr,float radius, bool ignoreNpc)
{
	radius *= radius;

	for(long i = 0; i < characters; i++)
	{
		Character * c = characters[i];

		if (c == chr) continue;

		if (c->logic->IsActor()) continue;
		if (c->logic->GetHP()<0.1f) continue;

		if (GetPowDistance(chr, c)<radius && !c->IsPlayer())
		{
			if (ignoreNpc && c->logic->GetSide() == CharacterLogic::s_npc)
				continue;
			if (c->logic->IsEnemy(chr))	return true;
		}
	}

	return false;	
}

bool CharactersArbiter::IsEnemyStatistAllive(Character* chr)
{
	for(long i = 0; i < characters; i++)
	{
		Character * c = characters[i];

		if (c == chr) continue;
		if (c->logic->IsActor()) continue;
		if (!c->logic->IsEnemy(chr)) continue;
		if (!c->controller->IsStatist()) continue;
		if (c->logic->GetHP()<0.1f) continue;	
			
		return true;
	}

	return false;
}

void CharactersArbiter::DestroyAllWeapon()
{
	for(long i = 0; i < TOTAL_DROPPED_ITEMS; i++)
	{
		TakeItem(i);
	}
}

//Нарисовать сектор поиска
void CharactersArbiter::FindDraw(Character & chr, float rmin, float rmax, float amin, float amax, float backMove)
{
	Vector pos = chr.physics->GetPos();
	pos.y += 1.0f;
	if(backMove > 0.0f)
	{
		pos -= Vector().MakeXZ(chr.physics->GetAy())*backMove;
		rmin += backMove;
		rmax += backMove;
	}
	struct
	{
		Vector pos;
		dword color;
	} poly[32];
	
	for(long i = 0; i < 16; i++)
	{
		float ang = amin + (amax - amin)*(i/15.0f);		
		Vector dir = Vector().MakeXZ(chr.physics->GetAy()).Rotate(ang);
		poly[i*2].pos = pos + dir*rmin;
		poly[i*2].color = 0x8000ff00;
		poly[i*2 + 1].pos = pos + dir*rmax;
		poly[i*2 + 1].color = 0x8000ff00;
	}
	GetRender().SetWorld(Matrix());


	ShaderId id;
	GetRender().GetShaderId("stdPolygon", id);
	GetRender().DrawPrimitiveUP(id, PT_TRIANGLESTRIP, 30, poly, sizeof(poly[0]));
}

//Получить список активных персонажей
const array<Character *> & CharactersArbiter::GetActiveCharacters()
{
	return characters;
}

void CharactersArbiter::AttackPhyObj(MissionObject* from, const Vector& pos, float radius,float damage, MissionObject* filter)
{
	dword count = QTFindObjects(MG_DAMAGEACCEPTOR,
								pos - Vector(radius, 0.0f, radius),
								pos + Vector(radius, 0.0f, radius));

	dmg_reciver* dmg = &d_recivers[d_recivers.Add()];
	

	dmg->recivers.DelAll();

	for(dword i = 0; i < count; i++)
	{
		dmg->recivers.Add((DamageReceiver*)&QTGetObject(i)->GetMissionObject());
	}
		
	for(dword i = 0; i < count; i++)
	{
		DamageReceiver* mo = dmg->recivers[i];
		
		if (from == mo) continue;			
		if (IsActiveCharacter(mo)) continue;		
		if (!mo->IsActive()) continue;

		DamageReceiver::DamageSource dsrc = DamageReceiver::ds_bomb;
		
		if (filter == mo)
		{
			dsrc = DamageReceiver::ds_bullet;
		}

		mo->Attack(from, dsrc, damage, pos, radius);
	}

	d_recivers.DelIndex(d_recivers.Size()-1);
}

bool CharactersArbiter::AttackPhyObj(MissionObject* attacker, dword source, float damage,Vector from, Vector to)
{
	Vector v_min = from;
	v_min.Min(to);
	Vector v_max = from;
	v_max.Max(to);

	dword count = QTFindObjects(MG_DAMAGEACCEPTOR, v_min, v_max);

	dmg_reciver* dmg = &d_recivers[d_recivers.Add()];
	

	dmg->recivers.DelAll();

	for(dword i = 0; i < count; i++)
	{
		dmg->recivers.Add((DamageReceiver*)&QTGetObject(i)->GetMissionObject());
	}
		
	bool was_attacked = false;

	for(dword i = 0; i < count; i++)
	{
		DamageReceiver* mo = dmg->recivers[i];
		
		if (attacker == mo) continue;	
		if (IsActiveCharacter(mo)) continue;
		if (!mo->IsActive()) continue;
				
		was_attacked = was_attacked & mo->Attack(attacker, source, damage, from, to);				
	}

	d_recivers.DelIndex(d_recivers.Size()-1);

	return was_attacked;
}

//void CharactersArbiter::StatsMoneySet(float gold)		{ if (statMoney) statMoney->Set(gold); }
void CharactersArbiter::StatsMoneyAdd(float add)		{ StatAdd(*statMoney, add); }
void CharactersArbiter::StatsDismemberAdd()				{ StatAdd(*statKilledDismember, 1.0f); }
void CharactersArbiter::StatsFatalitiesAdd()			{ StatAdd(*statKilledFatalities, 1.0f); }
void CharactersArbiter::StatsTotalChestAdd()			{ StatAdd(*statChestsTotal, 1.0f); }
void CharactersArbiter::StatsOpenedChestAdd(float add)	{ StatAdd(*statChestsOpened, add); }

void CharactersArbiter::Death(dword source, Character & victim, Character * killer)
{
	if (!killer)
		return;

	StatAdd(*statKilledTotal, 1.0f);

	if (killer->IsPlayer())
	{
		StatAdd(*statKilledPlayer, 1.0f);

		switch (source)
		{
			case DamageReceiver::ds_bullet :
				StatAdd(*statKilledShot, 1.0f);
			break;
			case DamageReceiver::ds_bomb :
				StatAdd(*statKilledBombed, 1.0f);
			break;
			case htds_item :
				StatAdd(*statKilledByItems, 1.0f);
			break;
			case DamageReceiver::ds_shooter :
				StatAdd(*statKilledShootered, 1.0f);
			break;
			default:
				//if (htype != ht_force_dword)
				{
					//StatAdd(*statKilledByItems, 1.0f);
				}
			break;
		};
	}
}

bool CharactersArbiter::IsSphereInFrustrum(Vector pos, float radius)
{		
	const Plane * frustum = GetRender().GetFrustum();	
	int numPlanes = GetRender().GetNumFrustumPlanes();

	for (int p=0; p<numPlanes; p++)
		if (frustum[p].Dist(pos) < -radius) return false;

	return true;
}

bool CharactersArbiter::IsPointInFrustum(const Vector & pnt)
{
	const Plane * frustum = GetRender().GetFrustum();	
	int numPlanes = GetRender().GetNumFrustumPlanes();

	for (int p=0; p<numPlanes; p++)
		if (frustum[p].Dist(pnt) < 0.0f) return false;

	return true;
}


// Взорвать патч
void CharactersArbiter::ExplosionPatch(const Vector & pos, const ConstString & type)
{
	if( explosionPatch )
		explosionPatch->MakeExplosion( pos, type);
}

void CharactersArbiter::Boom(MissionObject * offender, dword source, const Vector & pos, float radius, float damage, float boom_power, MissionObject * boomCreator)
{
	if(radius <= 0.0f)
	{
		return;
	}

	//damage *= 0.1f;

	int numRagdolls = 0;
	int numKills = 0;

	Character * from = null;

	if (offender)
	{
		MO_IS_IF(is_Character, "Character", offender)
		{
			from = (Character*)offender;

			if (source == DamageReceiver::ds_bomb)
			{
				BombExplosionPattern * bombPattern = (BombExplosionPattern *)from->GetBombExplosionPattern();
				if (bombPattern)
				{
					bombPattern->SetMatrix(Matrix(true).BuildPosition(pos));
					bombPattern->Command("Boom", 0, null);
				}
			}
		}
	}	

	if (source == DamageReceiver::ds_bomb || source == DamageReceiver::ds_cannon)
	{
		// Nikita: надо переделать, что бы не вызывался поиск патча каждый раз (только на create object)
		// и можно передавать не имя группы, а ее индекс, предварительно получив его там же на create object
		static const ConstString gExplosionPatchBomb("bomb");
		ExplosionPatch(pos, gExplosionPatchBomb);
	}

	ACHIEVEMENT_SETZERO(from, DEMOLITION);

	for(long i = 0; i < characters; i++)
	{		
		Character * chr = characters[i];

		if (!chr) continue;
		if (from == chr) continue;
		if (chr->logic->IsActor()) continue;
		if (chr->logic->IsRageMode()) continue;
		if (chr->logic->IsAttackImune()) continue;
		if (chr->logic->master) continue;
		if (!chr->IsShow()) continue;

		bool isAlreadyDead = (chr->logic->GetHP() < 0.1f);

		if (!chr->ragdoll)
		{
			if(chr->logic->GetHP()<0.1f) continue;

			if(chr->logic->GetState() == CharacterLogic::state_dead) continue;
			if(chr->logic->GetState() == CharacterLogic::state_die) continue;
		}		

		Vector chr_pos = characters[i]->physics->GetPos(false);
		
		bool add_torque = true;

		if(characters[i]->ragdoll)
		{			
			Matrix mat;
			characters[i]->physics->GetMatrixWithBoneOffset(mat);

			chr_pos = mat.pos;

			add_torque = false;
		}

		Vector dv = chr_pos - pos;		

		float dist2 = ~dv;
		if(dist2 > radius*radius)
		{
			continue;
		}			

		dv = dv + Vector(0,1.0f,0.0f);		

		if(characters[i]->logic->IsPairMode()) continue;

		if (characters[i]->logic->GetState() == CharacterLogic::state_kick) continue;		
		if (characters[i]->logic->GetState() == CharacterLogic::state_imbo) continue;

		dv.Normalize();

		float power = 1.0f;// - dist2/(radius*radius) * 0.7f;
		
		chr->logic->SetHP(chr->logic->GetHP() - chr->items->CalcDamage(damage) * chr->fBombDmgMultiplayer);

		if (from && from->controller)
		{
			from->controller->Attack(chr);
			
			chr->logic->SetOffender(from);
		}		

		if (!isAlreadyDead && (chr != (Character*)boomCreator) && 
			chr->logic->IsDead() && chr->logic->IsRealEnemy(from, false))
				ACHIEVEMENT_REACHED(from, DEMOLITION);

		if (chr->controller->AllowToBeBlowed())
		{
			chr->BloodStain(damage);

			if (chr->logic->IsDead())
			{
				chr->DropItems();

				chr->logic->Death(DamageReceiver::ds_bomb, "blow");
			}

			/*if (dist2<0.5f && Rnd()>0.5f && chr->animation->TestActivateLink("blow"))
			{
				Character* charFrom = NULL;
				if (from->Is("Character")) charFrom = (Character*)from;
				chr->logic->SetHP(0);
				chr->logic->Hit(damage ,"blow",0.0f,"blow", NULL, charFrom);	
			}
			else*/
			{
				chr->ActivateRagdoll(true,0.1f, 1.0f);
			
				float side_rand = RRnd(1.5f, 2.5f);
				dv.x *= side_rand;
				dv.y =  power * RRnd(7.5f, 10.0f);
				dv.z *= side_rand;

				if (!isAlreadyDead)
					numRagdolls++;

				if(chr->ragdoll)
				{
					dv *= power * sqrtf(Max(0.0f, Min(1.6f, boom_power)));
					dv.Clamp(-15.0f, 15.0f);
					chr->ragdoll->ApplyImpulse( dv );

					if (add_torque) chr->ragdoll->ApplyLocalTorque(Vector(0.0f, 1.5f * RRnd(-1.0f, 1.0f), 0.0f));
				}			
			}
		}
		else
		{
			if (chr->logic->IsDead() && !isAlreadyDead)
				numKills++;

			if (dist2>1.0f && chr->logic->AllowOrient()) chr->physics->Orient(pos,true,true);
			//chr->logic->ActivateLink("kick");
			chr->logic->Hit(DamageReceiver::ds_bomb, damage ,"kicked", 0.0f, damage , "kicked", NULL, from, false);	
		}

		if (from && from->IsPlayer())		
			from->controller->Attack(chr);
	}

	/*
	//Приложимся к персонажам
	for(long i = 0; i < characters; i++)
	{
		Character * chr = characters[i];
		Vector dv = chr->physics->GetPos() - pos;
		float dist2 = ~dv;
		if(dist2 > radius*radius)
		{
			continue;
		}
		if(dv.Normalize2D() < 1e-5f)
		{
			dv = 0.0f;
		}
		if(damage >= 0.0f)
		{
			chr->logic->Kick(dv);
		}
		chr->logic->SetHP(chr->logic->GetHP() - damage);
	}*/

	//Пройдёмся по демедж-объектам
	AttackPhyObj(from, pos, radius, damage);

	////////////////////////////////////////

	if( stainManager && source != DamageReceiver::ds_shooter)
	{
		Vector p(pos); //p.y -= 1.42f;

		stainManager->AddStain(p,Vector(0.0f,-1.0f,0.0f),2.5f,/*2.0f*/1.5f,IStainManager::Soot);

	//	api->Trace("\n    BOOM stain [%f,%f,%f]\n",p.x,p.y,p.z);
	}

	// Проверяем, может стоит запустить slowmo
	if (from && from->IsPlayer() && IsSlowmoReady())
	{
		float probability = float(numRagdolls + numKills * 3) * 0.05f;
		if (Rnd() < probability)
		{
			if (IsSphereInFrustrum(pos + Vector(0.0f, 1.0f, 0.0f), 0.5f))
				((PlayerController*)from->controller)->StartSloMo(1.1f, "0.3");
		}
	}
}

bool CharactersArbiter::IsSlowmoReady()
{
	Character * player = GetPlayer();

	if (!player || (player->logic && player->logic->IsActor()))
		return false;

	if (!((PlayerController*)player->controller)->IsSlowmoReady())
		return false;

	return true;
}

//Взорвать бомбу
void CharactersArbiter::BlowSelf(MissionObject * from, const Vector & pos, float radius, float damage, float boom_power)
{
	if(radius <= 0.0f)
	{
		return;
	}
	//Рождаем эффекты
	IParticleSystem * p = null;
	if(damage >= 0.0f)
	{
		p = Particles().CreateParticleSystem("BombardirExplosion.xps");
		Sound().Create3D("bmb_blast", pos, _FL_);
	}
	/*else
	{
	p = Particles().CreateParticleSystem("healing");
	}*/	

	if(p)
	{
		p->Teleport(Matrix(Vector(0.0f), pos));
		p->AutoDelete(true);
	}

	for(long i = 0; i < characters; i++)
	{		
		Character * chr = characters[i];

		if(chr->logic->IsActor()) continue;
		if(chr->logic->IsRageMode()) continue;
		if (chr->logic->IsAttackImune()) continue;

		if(!chr) continue;		
		//if (from == chr) continue;

		if(characters[i]->ragdoll) continue;

		if(characters[i]->logic->IsPairMode() && from != chr) continue;

		if(characters[i]->logic->GetState() == CharacterLogic::state_kick) continue;		

		Vector dv = chr->physics->GetPos(true) - pos;
		float dist2 = ~dv;
		if(dist2 > radius*radius)
		{
			continue;
		}

		dv.Normalize();

		float power = 1.0f - dist2/(radius*radius) * 0.7f;		

		if (chr->controller->AllowToBeBlowed() || from == chr)
		{	
			if (chr==from)
			{				
				chr->logic->SetHP(0.0f);
			}
			else
			{
				chr->logic->SetHP(chr->logic->GetHP() - chr->items->CalcDamage(damage));
			}

			if (chr->logic->IsDead() && (chr!=from))
			{
				chr->DropItems();
				chr->logic->Death(DamageReceiver::ds_bomb);
			}
			else
			{			
				chr->ActivateRagdoll(true,0.1f, 10.0f);
	
				dv.x *=5.0f;
				dv.y  =power * 9.0f;
				dv.z *=5.0f;
			}

            if(chr->ragdoll && from != chr)
			{
				//chr->ragdoll->ApplyImpulse( dv * power * boom_power );
			}			
		}
		else
		{
			chr->physics->Orient(pos,true,true);			
			//chr->logic->ActivateLink("kick");
			Character* charFrom = NULL;

			if (from)
			{
				MO_IS_IF(is_Character, "Character", from)
				{
					charFrom = (Character*)from;
				}
			}

			chr->logic->Hit(DamageReceiver::ds_bomb, damage, "kicked", 0.0f, damage, "kicked", NULL, charFrom, false);
		}	
	}

	//Пройдёмся по демедж-объектам
	AttackPhyObj(from, pos, radius, damage);
}
void CharactersArbiter::SplashDamage(dword source, MissionObject* offender,
									const Vector & pos, float radius,
									float MAXDamage, bool orient_to_attacker, const char* particlename,
									bool DamageAlly,float pow,bool allowkick,
									const char* reaction,const char* deathLink, MissionObject* filter)
{
	Vector orient_pos = pos;
	
	if (orient_to_attacker && offender)
	{
		Matrix mat;

		offender->GetMatrix(mat);

		orient_pos = mat.pos;
	}

	if(radius <= 0.0f)
	{
		return;
	}

	if (particlename)
	{
		//Рождаем эффекты
		IParticleSystem * p = null;
	
		if(MAXDamage >= 0.0f)
		{
			p = Particles().CreateParticleSystem(particlename);

		}	

		if(p)
		{
			p->Teleport(Matrix(Vector(0.0f), pos));
			p->AutoDelete(true);
		}
	}

	Character * from = null;

	if (offender)
	{
		MO_IS_IF(is_Character, "Character", offender)
		{
			from = (Character*)offender;
		}
	}	

	//Приложимся к персонажам
	for (long i = 0; i < characters; i++) if (characters[i] != from) // && characters[i]->logic->IsEnemy(*((Character*)from)))
	{
		Character* chr = characters[i];

		if (chr->logic->IsActor()) continue;
		if (chr->logic->IsRageMode()) continue;
		if (chr->logic->IsAttackImune()) continue;
		if (!chr->IsShow()) continue;

		Vector dv = chr->physics->GetPos() - pos;
		
		float fDist = dv.GetLength2();
		
		if(fDist > Sqr(radius))
		{
			continue;
		}

		if (characters[i]->ragdoll) continue;

		if (characters[i]->logic->IsPairMode()) continue;

		if (characters[i]->logic->GetState() == CharacterLogic::state_kick) continue;
		if (characters[i]->logic->GetState() == CharacterLogic::state_imbo) continue;

		float fDamageKoef = 1.0f;
			
		if (!DamageAlly)
		{
			Character* charFrom = NULL;
			MO_IS_IF(is_Character, "Character", from)
			{
				charFrom = (Character*)from;

				if (!charFrom->logic->IsRealEnemy(chr))
				{
					fDamageKoef = 0.0f;
				}
			}
		}

		float damage = MAXDamage * fDamageKoef * (1.0f - fDist / Sqr(radius));

		if (reaction && reaction[0])
		{
			if (orient_to_attacker)
			{
				chr->physics->Orient(orient_pos,true,true);
			}

			chr->logic->Hit(source, damage, reaction, 0.0f, damage, reaction, deathLink, from, false);
		}
		else
		{
			if (allowkick)
			{		
				if (chr->controller->AllowToBeBlowed())
				{	
					if (chr==from)
					{				
						chr->logic->SetHP(0.0f);
					}
					else
					{
						chr->logic->SetHP(chr->logic->GetHP() - chr->items->CalcDamage(damage));
					}

					if (chr->logic->IsDead())
					{
						chr->DropItems();
						chr->logic->Death(source);
					}
					else
					{			
						chr->ActivateRagdoll(true,0.1f, 1.0f);

						dv.x *=5.0f;
						dv.y  = 0;
						dv.z *=5.0f;
					}
	
					if(chr->ragdoll)
					{
						dv *= 2.0f;
						dv.Clamp(-20,20);

						chr->ragdoll->ApplyImpulse( dv );
					}			
				}
				else
				{
					chr->physics->Orient(orient_pos,true,true);
					//chr->logic->ActivateLink("kick");

					chr->logic->Hit(source, damage, "kicked", 0.0f, damage, "kicked", deathLink, from, false);				
				}
			}
			else
			{
				chr->physics->Orient(orient_pos,true,true);
				chr->logic->Hit(source, damage, "hit", 0.0f, damage, "hit", NULL, from, false);
			}
		}

		if (from && from->IsPlayer())
			from->controller->Attack(chr);
	}
		
	//Пройдёмся по демедж-объектам
	AttackPhyObj(from, pos, radius, MAXDamage, filter);
}


//============================================================================================
#ifndef NO_CONSOLE

void CharactersArbiter::Action_ShowDebugInfo(int type)
{
	// FIX ME!!!
	{
		if (type<0)
		{
			debugInfoType = -1;		
		}
		else
		{
			if (debugInfoType<0) debugInfoType = 0;

			int pow = (int)powf(2.0f,(float)type);			

			if (debugInfoType & pow)
			{
				debugInfoType = debugInfoType ^ pow;
			}
			else
			{
				debugInfoType = debugInfoType | pow;
			}

			if (debugInfoType == 0) debugInfoType=-1;
		}
	}

	Action_SetDebugInfo(debugInfoType);
	if (debugInfoType>0)
	{		
		Action_ShowControllerInfo(((debugInfoType & 16)>0));	
	}	
	else
	{
		Action_ShowControllerInfo(false);
	}
}

void CharactersArbiter::Action_SetDebugInfo(int type)
{
	MGIterator& iter = Mission().GroupIterator(MG_CHARACTER, _FL_);

	string id;
	string filter = charFilter;
	filter.Lower();

	for (;!iter.IsDone();iter.Next())
	{
		Character* pObject = (Character*)iter.Get();

		id = pObject->GetObjectID().c_str();
		id.Lower();

		if (id.IsFileMask(filter))
		{
			pObject->SetDebugInfo(type);
		}
		else
		{
			pObject->SetDebugInfo(-1);
		}
	}

	iter.Release();	
}

void CharactersArbiter::Action_ShowControllerInfo(bool show)
{
	MGIterator& iter = Mission().GroupIterator(MG_CHARACTER, _FL_);

	for (;!iter.IsDone();iter.Next())
	{
		Character* pObject = (Character*)iter.Get();

		pObject->ShowControllerDebugInfo(show);
	}

	iter.Release();
}

void CharactersArbiter::Action_ShowMap(bool show)
{
	if (show)
	{
		SetUpdate((MOF_UPDATE)&CharactersArbiter::DrawMap, ML_DEBUG);
	}
	else
	{
		DelUpdate((MOF_UPDATE)&CharactersArbiter::DrawMap);
	}	
}

bool CharactersArbiter::Action_SetCharFilter(const ConsoleStack & params)
{
	char charFilterTmp[128]; 
	crt_strcpy(charFilterTmp,sizeof(charFilter),charFilter);

	if (params.GetSize()>0)
	{
		const char * name = params.GetParam(0);
		if(!name) name = "*";

		crt_strcpy(charFilter,sizeof(charFilter),name);		
	}
	else
	{
		crt_strcpy(charFilter,sizeof(charFilter),"*");
	}

	if (string::IsEqual(charFilterTmp,charFilter))
	{
		return false;
	}

	return true;
}

void CharactersArbiter::Action_DebugInfoChanged(const ConsoleStack & params,int type)
{
	if (!Action_SetCharFilter(params))
	{
		Action_ShowDebugInfo(type);
	}
	else
	{
		int pow = (int)powf(2.0f,(float)type);

		if (debugInfoType==-1 || !(debugInfoType & pow))
		{
			Action_ShowDebugInfo(type);
		}
		else
		{
			Action_SetDebugInfo(debugInfoType);
		}
	}
}

void _cdecl CharactersArbiter::Console_CharStats(const ConsoleStack & params)
{
	charStatsShow ^= 1;
}

void _cdecl CharactersArbiter::Console_CharID(const ConsoleStack & params)
{
	Action_DebugInfoChanged(params,0);
}

void _cdecl CharactersArbiter::Console_CharAnim(const ConsoleStack & params)
{
	Action_DebugInfoChanged(params,1);
}

void _cdecl CharactersArbiter::Console_CharRenderParams(const ConsoleStack & params)
{
	Action_DebugInfoChanged(params,2);	
}

void _cdecl CharactersArbiter::Console_CharHP(const ConsoleStack & params)
{
	Action_DebugInfoChanged(params,3);
}

void _cdecl CharactersArbiter::Console_CharAI(const ConsoleStack & params)
{
	Action_DebugInfoChanged(params,4);	
}

void _cdecl CharactersArbiter::Console_CharItems(const ConsoleStack & params)
{
	Action_DebugInfoChanged(params,5);
}

void _cdecl CharactersArbiter::Console_CharActor(const ConsoleStack & params)
{
	Action_DebugInfoChanged(params,6);
}

void _cdecl CharactersArbiter::Console_CharHideInfo(const ConsoleStack & params)
{
	Action_ShowDebugInfo(-1);
	Action_SetCharFilter(params);
}

void _cdecl CharactersArbiter::Console_CharSetFilter(const ConsoleStack & params)
{
	Action_SetCharFilter(params);		
}

void _cdecl CharactersArbiter::Console_SwordDebug(const ConsoleStack & params)
{
	CharacterItems::isCharacterSwordDebug ^= 1;
}

void _cdecl CharactersArbiter::Console_AnimDebug(const ConsoleStack & params)
{
	AnimDebugOn = !AnimDebugOn;

	if (AnimDebugOn)
	{
		SetUpdate((MOF_UPDATE)&CharactersArbiter::AnimDebug, ML_DEBUG1);
		Console().Trace(COL_CMD_OUTPUT, "AnimDebug Enabled");
	}
	else
	{
		DelUpdate((MOF_UPDATE)&CharactersArbiter::AnimDebug);
		Console().Trace(COL_CMD_OUTPUT, "AnimDebug Disabled");
	}	
}

void _cdecl CharactersArbiter::Console_ShowMap(const ConsoleStack & params)
{
	showMap = !showMap;
	Action_ShowMap(showMap);
}

void _cdecl CharactersArbiter::Console_ShowWaysOnMap(const ConsoleStack & params)
{
	showWayPointsOnMap = !showWayPointsOnMap;
}

void _cdecl CharactersArbiter::Console_ShowCPtOnMap(const ConsoleStack & params)
{
	showCPtOnMap = !showCPtOnMap;
}

void _cdecl CharactersArbiter::Console_ShowAIZnOnMap(const ConsoleStack & params)
{
	showAIZnOnMap = !showAIZnOnMap;
}

void _cdecl CharactersArbiter::Console_ShowWays(const ConsoleStack & params)
{
	showWayPoints = !showWayPoints;

	if (showWayPoints)
	{
		Console().Trace(COL_CMD_OUTPUT, "WayPoints was showed");
	}
	else
	{
		Console().Trace(COL_CMD_OUTPUT, "WayPoints was hided");
	}	
	
	{
		MGIterator& iter = Mission().GroupIterator(WAY_POINT_GROUP, _FL_);

		for (;!iter.IsDone();iter.Next())
		{
			PathFinder* path = (PathFinder*)iter.Get();

			path->ShowDebug(showWayPoints);			
		}

		iter.Release();
	}
}

void _cdecl CharactersArbiter::Console_ShowCPt(const ConsoleStack & params)
{
	showCPt = !showCPt;

	if (showCPt)
	{
		Console().Trace(COL_CMD_OUTPUT, "CombatPoints was showed");
	}
	else
	{
		Console().Trace(COL_CMD_OUTPUT, "CombatPoints was hided");
	}	

	{
		MGIterator& iter = Mission().GroupIterator(COMBAT_POINTS_GROUP, _FL_);

		for (;!iter.IsDone();iter.Next())
		{
			CombatPoints* cp = (CombatPoints*)iter.Get();

			cp->ShowDebug(showCPt);			
		}

		iter.Release();
	}
}

void _cdecl CharactersArbiter::Console_ShowAIZn(const ConsoleStack & params)
{
	showAIZn = !showAIZn;

	if (showAIZn)
	{
		Console().Trace(COL_CMD_OUTPUT, "AIZones was showed");
	}
	else
	{
		Console().Trace(COL_CMD_OUTPUT, "AIZones was hided");
	}	

	MGIterator& iter = Mission().GroupIterator(AIZones_GROUP, _FL_);

	for (;!iter.IsDone();iter.Next())
	{
		AIZones* aiZones = (AIZones*)iter.Get();

		aiZones->ShowDebug(showAIZn);
	}

	iter.Release();	
}

void _cdecl CharactersArbiter::Console_GodMode(const ConsoleStack & params)
{
	SwitchGodMode();	
}

void _cdecl CharactersArbiter::Console_GiveMoney(const ConsoleStack & params)
{
	Character* player = GetPlayer();

	if(player)
	{
		float money = 1000000.0f;
		
		if (params.GetSize()>=1)
		{
			char * pos = null;
			money = (float)strtod(params.GetParam(0), &pos);			
		}

		((PlayerController*)player->controller)->AddMoney(money);

		Console().Trace(COL_CMD_OUTPUT, "%4.0f gold was added", money);
	}
	else
	{
		Console().Trace(COL_CMD_OUTPUT, "Cant find player!");
	}
}

void _cdecl CharactersArbiter::Console_GiveAmmo(const ConsoleStack & params)
{
	Character* player = GetPlayer();

	if(player)
	{
		((PlayerController*)player->controller)->AddBomb(4);
		Console().Trace(COL_CMD_OUTPUT, "ammo was added");
	}
	else
	{
		Console().Trace(COL_CMD_OUTPUT, "Cant find player!");
	}
}

void _cdecl CharactersArbiter::Console_GiveAll(const ConsoleStack & params)
{
	Character* player = GetPlayer();

	if(player)
	{
		((PlayerController*)player->controller)->GiveMeAllINeed();
		Console().Trace(COL_CMD_OUTPUT, "All shop goods was given to player!");
	}
	else	
	{
		Console().Trace(COL_CMD_OUTPUT, "Cant find player!");
	}

	MOSafePointer mo;
	static const ConstString chDelayId("Checker_Delay");
	FindObject(chDelayId,mo);
	
	if (mo.Ptr())
	{
		mo.Ptr()->Activate(true);
	}
}

void _cdecl CharactersArbiter::Console_ShowBeacon(const ConsoleStack & params)
{
	show_beacon = !show_beacon;

	if (show_beacon)		
	{
		SetUpdate((MOF_UPDATE)&CharactersArbiter::DrawBeacon, ML_DEBUG);
		Console().Trace(COL_CMD_OUTPUT, "CharBeacon Draw was enabled");
	}
	else
	{
		DelUpdate((MOF_UPDATE)&CharactersArbiter::DrawBeacon);
		Console().Trace(COL_CMD_OUTPUT, "CharBeacon Draw was disabled");
	}
}

void _cdecl CharactersArbiter::Console_ShowPairMiniGame(const ConsoleStack & params)
{
	Character* player = GetPlayer();

	if(player)
	{
		Console().Trace(COL_CMD_OUTPUT, "Palyer has follow pairs:");

		for (int i=0;i<20;i++)
		{
			if (player->pair_minigame[i])
			{
				Console().Trace(COL_CMD_OUTPUT, "    %s", player->pair_minigame[i]->GetObjectID().c_str());
			}
		}		
	}
}

void _cdecl CharactersArbiter::Console_CharDebug(const ConsoleStack & params)
{
	if (params.GetSize()<=0) return;

	
	const char * name = params.GetParam(0);

	MGIterator& iter = Mission().GroupIterator(MG_CHARACTER, _FL_);

	for (;!iter.IsDone();iter.Next())
	{
		Character* pObject = (Character*)iter.Get();

		if (string::IsEqual(pObject->GetObjectID().c_str(),name))
		{
			pObject->controller->SetStopWhenDebug(true);
		}
		else
		{
			pObject->controller->SetStopWhenDebug(false);
		}
	}

	iter.Release();
}

void _cdecl CharactersArbiter::Console_CharDraw(const ConsoleStack & params)
{
	MGIterator& iter = Mission().GroupIterator(MG_CHARACTER, _FL_);

	for (;!iter.IsDone();iter.Next())
	{
		Character* pObject = (Character*)iter.Get();

		pObject->bDraw = !pObject->bDraw;		
	}

	iter.Release();	
}

void _cdecl CharactersArbiter::Console_CharHairDebug(const ConsoleStack & params)
{
	bHairDebug = !bHairDebug;
	
	if (bHairDebug)
	{
		Console().Trace(COL_CMD_OUTPUT, "Hair Debug Draw was Enabled");		
	}
	else
	{
		Console().Trace(COL_CMD_OUTPUT, "Hair Debug Draw was Disabled");		
	}

}

void _cdecl CharactersArbiter::Console_ShowAchievements(const ConsoleStack & params)
{
	Character* player = null;

	if (params.GetSize()<=0)
	{
		player = GetPlayer();
	}
	else
	{
		MOSafePointerTypeEx<Character> trgt;
		static const ConstString strTypeId("Character");
		trgt.FindObject(&Mission(),ConstString(params.GetParam(0)),strTypeId);

		player = trgt.Ptr();		
	}
	
	if (player)
	{
		if (player->chrAchievements)
		{			
			Console().Trace(COL_CMD_OUTPUT, "Achievements state of %s are follow:",player->GetObjectID().c_str());

			//player->chrAchievements->DebugDraw(&Console());
		}
		else
		{
			Console().Trace(COL_CMD_OUTPUT, "Error! Character hasn't achievements.");
		}
	}
	else
	{
		Console().Trace(COL_CMD_OUTPUT, "Error! Can't find character.");
	}
}

void _cdecl CharactersArbiter::Console_FreezeRagdolls(const ConsoleStack & params)
{
	Character::g_bFreezeRagdolls ^= 1;

	if (Character::g_bFreezeRagdolls)
		Console().Trace(COL_CMD_OUTPUT, "Ragdolls freezing");
	else
		Console().Trace(COL_CMD_OUTPUT, "Ragdolls not freezing now");
}

void _cdecl CharactersArbiter::Console_WatchAchieve(const ConsoleStack & params)
{
	if (params.GetSize()<=0) return;
	
	Character* player = GetPlayer();

	if (player)
	{
		if (player->chrAchievements)
		{			
			Console().Trace(COL_CMD_OUTPUT, "Watch state of %s was changed",player->GetObjectID().c_str());

			//player->chrAchievements->WatchAchieve((int)strtod(params.GetParam(0),null)-1);
		}
		else
		{
			Console().Trace(COL_CMD_OUTPUT, "Error! Character hasn't achievements.");
		}
	}
	else
	{
		Console().Trace(COL_CMD_OUTPUT, "Error! Invalide parameters.");
	}
}

#endif

bool CharactersArbiter::CheckNumAttckers(Character* target, int MaxAlowed, char* Type)
{
	int NumAttackers = 0;

	for(long i = 0; i < characters; i++)
	{
		if (characters[i]->controller->GetChrTarget() == target &&
			string::IsEqual(characters[i]->controller->Name(),Type))
		{
			NumAttackers++;
		}

		if (NumAttackers>=MaxAlowed)
		{
			return false;
		}
	}

	return true;
}

void CharactersArbiter::DropeItem(ItemData* item,Matrix& world_mat,bool isBroken)
{
	if (!AllowDropItem(false)) return;

	if (!isBroken)
	{
		DropeItemImpl(item,world_mat,0);
	}
	else
	{
		DropeItemImpl(item,world_mat,1);

		if (AllowDropItem(false))
		{
			DropeItemImpl(item,world_mat,2);
		}
	}
}



bool PhysRigidBodyActorCheckMtx(const Matrix & mtx)
{
	bool isScaled = false;

#ifndef _XBOX
	const float eps = 1e-4f;
	const float ortoEps = 0.9999f;
#else
	const float eps = 1e-2f;
	const float ortoEps = 0.999f;
#endif


	if(fabsf(~mtx.vx - 1.0f) > eps) isScaled = true;
	if(fabsf(~mtx.vy - 1.0f) > eps) isScaled = true;
	if(fabsf(~mtx.vz - 1.0f) > eps) isScaled = true;

	float orhto = (!(mtx.vx ^ mtx.vz) | !mtx.vy);
	bool isNonOrtho = (orhto >= -ortoEps);

	//if (api->DebugKeyState('8') && (rand()%10) == 5) isNonOrtho = true;

	if (isScaled || isNonOrtho)
	{
		return false;
	}

	return true;
}


bool CharactersArbiter::IsItemDroppableWeapon(TWeaponType weaponType)
{
	switch (weaponType)
	{
		case wp_sword1:
		case wp_sword2:
		case wp_sword3:
		case wp_axe1:
		case wp_axe2:
		case wp_axe3:
		case wp_twohanded1:
		case wp_twohanded2:
		case wp_twohanded3:
		case wp_rifle:
		case wp_rapire:
		case wp_gun:
			return true;
	}
	return false;
}

float CharactersArbiter::AdjustTimeLie(ItemData * item, int index, int max_weapons)
{
	if (!IsItemDroppableWeapon(item->weapon_type))
		return item->time_lie;

	droppedWeapons.Empty();

	for (int i=0; i<TOTAL_DROPPED_ITEMS; i++)
		if (i != index && droped_items[i].type == item->weapon_type && droped_items[i].fTimeLife > item->time_lie)
		{
			DroppedWeapon dwp;
			dwp.idx = i;
			dwp.timeLife = droped_items[i].fTimeLife;
			droppedWeapons.Add(dwp);
		}

	if (droppedWeapons.Len() > max_weapons)
	{
		droppedWeapons.QSort(DroppedWeaponsSortTimeLife);
		for (int i=max_weapons; i<droppedWeapons.Len(); i++)
		{
			int idx = droppedWeapons[i].idx;
			droped_items[idx].fTimeLife = item->time_lie;
		}
	}

	return 3.1536e+8f;	// возвращаем примерно 1 год лежания
}


void CharactersArbiter::DropeItemImpl(ItemData* item,Matrix& world_mat,int item_part)
{
	IGMXScene* model = item->model;

	if (item_part==1)
	{
		model = item->model_part1;
	}
	else
	if (item_part==2)
	{
		model = item->model_part2;		
	}

	if(!model && item->weapon_type != wp_armor)
	{		
		return;
	}
		

	Vector size,center;

	if (item->weapon_type != wp_armor)
	{
		//AABB
		const GMXBoundBox & volume = model->GetLocalBound();
		size = volume.vMax - volume.vMin;		
		center = volume.vMin + (size * 0.5f);
	}
	else
	{		
		size = item->drp_data.bMax - item->drp_data.bMin;
		center = item->drp_data.bMin + (size * 0.5f);
	}

	if(size.x <= 0.0001f || size.y <= 0.0001f || size.z <= 0.0001f)
	{		
		return;
	}

	if(size.x > 100.0f || size.y > 100.0f || size.z > 100.0f)
	{		
		return;
	}	

	size.Max(Vector(0.15f));	

	Matrix mt;					
	mt.pos=center;

	/*if (!PhysRigidBodyActorCheckMtx( mt * world_mat))
	{
		Assert(false);
		return;
	}*/
	
	// Vano: убрал mt *, потому что иначе были смещения у некоторых итемов, типа шляпы
	//IPhysBox* pPhysicBox = Physics().CreateBox(_FL_, size, (mt * world_mat), true);
	IPhysBox* pPhysicBox = Physics().CreateBox(_FL_, size, world_mat, true);
	
	item_index++;

	int count = 0;

	while (droped_items[item_index].fTimeLife>0.0f)
	{
		count++;
		if (count>=TOTAL_DROPPED_ITEMS) return;

		item_index++;		
		if (item_index>=TOTAL_DROPPED_ITEMS) item_index = 0;
	}
	

	TDropedItem &dr_item = droped_items[item_index];

	float time2Lie = (item_part == 0) ? AdjustTimeLie(item, item_index, 3) : item->time_lie;

	dr_item.fTimeLife = Max(8.0f, time2Lie);
	
	// Fix ME !!!!
	// необходима фича - физ клонировнаие данных у IGMXModel
	// dr_item.model = model->Clone();	
	
	if (item_part==0)
	{
		dr_item.model = Geometry().CreateScene(item->model_name, &Animation(), &Particles(), &Sound(), _FL_);
	}
	else
	if (item_part==1)
	{
		dr_item.model = Geometry().CreateScene(item->model_part1_name, &Animation(), &Particles(), &Sound(), _FL_);
	}
	else
	if (item_part==2)
	{
		dr_item.model = Geometry().CreateScene(item->model_part2_name, &Animation(), &Particles(), &Sound(), _FL_);
	}

	if (dr_item.model) dr_item.model->SetDynamicLightState(true);

	
	dr_item.init_data.model_name = item->model_name;
	dr_item.init_data.broken_part1 = item->model_part1_name;
	dr_item.init_data.broken_part2 = item->model_part2_name;	

	dr_item.init_data.tip_id = item->tip_id;
	dr_item.init_data.item_life = item->itemLife;

	// Vano: Закоментил тут Matrix(Vector(0.0f), -center);
	dr_item.physicPivot = Matrix();//Matrix(Vector(0.0f), -center);
	dr_item.pPhysicBox = pPhysicBox;

	dr_item.type = item->weapon_type;

	if (item->weapon_type == wp_armor)
	{
		dr_item.drp_data = &item->drp_data;
		
		dr_item.drp_data->alpha = 1.0f;
		dr_item.drp_data->transform = mt * world_mat;
	}
	else
	{
		dr_item.drp_data = null;
	}

	if (item_part==0)
	{
		dr_item.type = item->weapon_type;
	}	
	else
	{
		dr_item.type = wp_undropable;
	}
	
	dr_item.flash = Rnd();
	dr_item.is_bomb = (item->weapon_type == wp_bomb);

	
	if (dr_item.type != wp_armor &&
		dr_item.type != wp_bomb &&
		dr_item.init_data.tip_id.NotEmpty() && item_part==0)
	{
		if (tip_manager)
		{
			dr_item.tip = tip_manager->CreateTip(item->tip_id,0.0f,this);			

			if (dr_item.tip)
			{
				dr_item.tip->Activate(true);
			}
		}			
	}

	pPhysicBox->SetMass(10.0f);

	pPhysicBox->SetGroup(phys_charitems);	

	Vector impulse = world_mat.vy * RRnd(3.5f, 5.0f);

	pPhysicBox->ApplyImpulse( impulse, Vector(0, 0.0f, 0.0f));	

	impulse.Rand(Vector(0.0f), 1.0f);
	impulse.Normalize();

	pPhysicBox->ApplyLocalTorque(impulse * 2.0f);	

	item->visible = false;	
}

void CharactersArbiter::TakeItem(int item_index)
{
	RELEASE(droped_items[item_index].pPhysicBox);
	RELEASE(droped_items[item_index].tip);
	RELEASE(droped_items[item_index].model);

	droped_items[item_index].fTimeLife = 0.0f;
	droped_items[item_index].drp_data = null;
}

int CharactersArbiter::FindClosesetItem(Character* from, float dist)
{	
	if (!from) return -1;

	Vector pos = from->physics->GetPos();

	Matrix world;

	for (int i=0;i<TOTAL_DROPPED_ITEMS;i++)
	{
		TDropedItem& item = droped_items[i];

		if (item.fTimeLife < 1.0f) continue;
		if (!item.model) continue;
		if (!item.pPhysicBox) continue;

		if (item.type == wp_armor || item.type == wp_bomb || item.type ==wp_undropable) continue; 
		if (!from->controller->CanTakeWeapon() && item.type != wp_box) continue;
		
		item.pPhysicBox->GetTransform(world);
		world = item.physicPivot * world;

		if ((pos - world.pos).GetLength2() < Sqr(dist))
		{
			return i;
		}	
	}

	return -1;
}

bool CharactersArbiter::AllowDropItem(bool is_item_armor)
{
	int count = 0;

	for (int i=0;i<TOTAL_DROPPED_ITEMS;i++)
	{
		if (droped_items[i].fTimeLife > 0.0f)
		{
			if (is_item_armor)
			{
				if (droped_items[i].type == wp_armor)
				{
					count++;

					if (count==TOTAL_DROPPED_ITEMS - NUM_DROPPED_ITEMS) return false;
				}
			}
			else
			{
				if (droped_items[i].type != wp_armor)
				{
					count++;

					if (count==NUM_DROPPED_ITEMS) return false;
				}
			}
		}
	}	

	for (int i=0;i<TOTAL_DROPPED_ITEMS;i++)
	{
		if (droped_items[i].fTimeLife <= 0.0f) return true;
	}

	return false;
}

void _cdecl CharactersArbiter::DrawDropedItems(float dltTime, long level)
{
	Character * player = GetPlayer();
	if (player && (player->logic->IsActor() || !player->IsShow()))
	{
		if (tip_manager)
			tip_manager->Enable(false);
		return;
	}

	if (tip_manager)
		tip_manager->Enable(true);

	int picked_item = FindClosesetItem(GetPlayer());

	for (int i=0;i<TOTAL_DROPPED_ITEMS;i++)
	{
		TDropedItem& item = droped_items[i];

		if (item.fTimeLife<=0.0f)			
		{
			if (item.drp_data)
			{
				item.drp_data->alpha = 0.0f;
				item.drp_data = null;
			}

			continue;
		}

		item.flash += dltTime;

		float i_part;
		item.flash  = modff(item.flash,&i_part);
		

		item.fTimeLife-=dltTime;
		
		if (item.fTimeLife<0.0f)
		{
			if (item.is_bomb && item.pPhysicBox)
			{
				IParticleSystem * p = null;
			
				p = Particles().CreateParticleSystem("ExplosionBomb");
			
				Matrix world(true);
				item.pPhysicBox->GetTransform(world);

				if(p)
				{
					p->Teleport(Matrix(Vector(0.0f), world.pos));
					p->AutoDelete(true);
				}
				
				MOSafePointer mo;
				static const ConstString playerId("Player");
				FindObject(playerId,mo);

				Boom(mo.Ptr(),(item.pPhysicBox) ? DamageReceiver::ds_bomb : htds_item, world.pos, 2.5, 20, 1.0f);
			}

			TakeItem(i);

			continue;
		}			

		//if (!item.model) continue;
		if (!item.pPhysicBox) continue;		
		
		Matrix world(true);
		Matrix world2(true);

		//item.pPhysicBox->GetTransform(world2);
		item.pPhysicBox->GetTransform(world);
		/*world.m[0][3] = 0.0f;
		world.m[1][3] = 0.0f;
		world.m[2][3] = 0.0f;
		world.m[3][3] = 1.0f;		

		world = item.physicPivot * world;*/
		
		//GetRender().DrawMatrix(world2);

		float a = 1.0f;
		
		if (item.fTimeLife<2.0f && !item.is_bomb)
		{
			a = Clampf(item.fTimeLife/2.0f);
		}

		float k = 0.0f;
		
		if (item.type != wp_armor && item.type != wp_bomb && item.type != wp_undropable)
		{
			k = item.flash;
			
			if (k>0.5f)
			{
				k = 1 - k;
			}
		}

		Color userColor(k * 0.8f, k * 0.8f, k* 0.5f, a);		

		if (item.drp_data)
		{
			item.drp_data->alpha = a;
			item.drp_data->transform = world;
		}
		else
		{
			item.model->SetUserColor(userColor);
			item.model->SetTransform(world);
			item.model->Draw();
		}

		if (item.tip)
		{
			if (picked_item == i)
			{
				item.tip->SetState(ITip::active);				
			}

			item.tip->SetPos(world.pos);
			item.tip->SetAlpha(a);
		}
	}
}

TWeaponType CharactersArbiter::GetDropedWpType(int index)
{
	if (index<0 || index>=TOTAL_DROPPED_ITEMS) return wp_undropable;

	return droped_items[index].type;
}

CharactersArbiter::TDropedItemInitData* CharactersArbiter::GetDropedWpItemData(int index)
{
	if (index<0 || index>=TOTAL_DROPPED_ITEMS) return null;

	return &droped_items[index].init_data;
}

void CharactersArbiter::DrawFlare(int tex_index,Matrix& mat, float power)
{
	gmRData.vPower->SetFloat( power );

	if (tex_index==0)
	{
		gmRData.vTex->SetTexture(gmRData.flareTex1);
	}
	else
	{
		gmRData.vTex->SetTexture(gmRData.flareTex2);
	}

	GetRender().SetStreamSource(0, gmRData.flareBuffer, sizeof(Vertex));
	GetRender().SetWorld( mat );

	ShaderId id;
	GetRender().GetShaderId("Bullet_Trace", id);
	GetRender().DrawPrimitive(id, PT_TRIANGLESTRIP, 0, 2);
}

void CharactersArbiter::DrawBulletTrace(Matrix& mat, float power)
{
	gmRData.vPower->SetFloat( power );

	gmRData.vTex->SetTexture(gmRData.bulletTex);

	GetRender().SetStreamSource(0, gmRData.bulletBuffer, sizeof(Vertex));
	GetRender().SetWorld( mat );
	
	ShaderId id;
	GetRender().GetShaderId("Bullet_Trace", id);
	GetRender().DrawPrimitive(id, PT_TRIANGLELIST, 0, 4);
}

void CharactersArbiter::DrawCoin(int index, Matrix& mat, float alpha)
{
	if (index == -1) return;
	if (!coin_model) return;

	Color color( 0.0f, 0.0f, 0.0f, alpha);

	coin_model->SetUserColor(color);
	coin_model->SetTransform(mat);
	coin_model->Draw();
}

void CharactersArbiter::DrawBomb(Matrix& mat)
{	
	if (!bomb_model) return;

	bomb_model->SetTransform(mat);
	bomb_model->Draw();
}

void _cdecl CharactersArbiter::DrawBeacon(float dltTime, long level)
{
	int num_active = 0;
	int num_actor = 0;
	int num_dead = 0;
	int num_statist = 0;

	for(long i = 0; i < characters; i++)
	{
		Character* chr = characters[i];
		
		dword color = 0xff00ff00;		

		if (chr->IsDead() || chr->IsDie())
		{
			color = 0xffff0000;
			num_dead++;
		}
		else
		if (chr->logic->IsActor())
		{
			color = 0xffffff00;
			num_actor++;
		}
		else
		if (chr->controller->IsStatist())
		{
			color = 0xff0000ff;
			num_statist++;
		}
		else
		{
			num_active++;
		}

		GetRender().DrawLine(chr->physics->GetPos(),color,
						  chr->physics->GetPos()+Vector(0,25.0f,0),color);
	}

	GetRender().Print(10,25,0xff00ff00,"Active  - %i",num_active);
	GetRender().Print(10,40,0xffffff00,"Actor   - %i",num_actor);
	GetRender().Print(10,55,0xffff0000,"Dead    - %i",num_dead);
	GetRender().Print(10,70,0xff0000ff,"Statist - %i",num_statist);
	GetRender().Print(10,85,0xffffffff,"Total   - %i",(int)characters.Size());
}

void CharactersArbiter::PrintTextShadowed(const Vector& pos, int line, DWORD color, const char * pFormatString, ...)
{
	if (!pFormatString) return;

	Matrix mVP(GetRender().GetView(), GetRender().GetProjection());

	Vector4 v = mVP.Projection(pos, GetRender().GetViewport().Width * 0.5f, GetRender().GetViewport().Height * 0.5f);
		
	Vector ps = pos * GetRender().GetView();

	if (v.w > 0.0f && ps.z<15.0f)
	{
		va_list args;
		va_start(args, pFormatString);
		crt_vsnprintf(cTmpBuffer, sizeof(cTmpBuffer), pFormatString, args);
		va_end(args);

		float fXViewportScale = (float)GetRender().GetScreenInfo3D().dwWidth / (float)GetRender().GetViewport().Width;
		float fYViewportScale = (float)GetRender().GetScreenInfo3D().dwHeight / (float)GetRender().GetViewport().Height;

		float fWidth = fXViewportScale * GetRender().GetSystemFont()->GetLength(cTmpBuffer);
							
		GetRender().Print((v.x - fWidth * 0.5f)+1, (v.y - fYViewportScale * 16.0f * (0.5f - (-1.0f)))+1 + line * 16, 0xFF000000, cTmpBuffer);
		GetRender().Print(v.x - fWidth * 0.5f, v.y - fYViewportScale * 16.0f * (0.5f - (-1.0f)) + line * 16, color, cTmpBuffer);
	}
}

void CharactersArbiter::PrintTextShadowed(DWORD color, const char * pFormatString, ...)
{
	va_list args;
	va_start(args, pFormatString);
	crt_vsnprintf(cTmpBuffer, sizeof(cTmpBuffer), pFormatString, args);
	va_end(args);
	
	GetRender().Print( 6.0f, (float)(index2d*16+1), 0xFF000000, cTmpBuffer);
	GetRender().Print( 5.0f, (float)( index2d*16 ), color, cTmpBuffer);
	
	index2d++;
}

bool CharactersArbiter::IsFatalityAllowed(int num)
{
	switch (num)
	{
	case 0:
		{
			return (ReadGameState(gsFatalityA,0.0f) > 0.5f);
		}
		break;
	case 1:
		{
			return (ReadGameState(gsFatalityB,0.0f) > 0.5f);
		}
		break;
	case 2:
		{
			return (ReadGameState(gsFatalityC,0.0f) > 0.5f);
		}
		break;
	case 3:
		{
			return (ReadGameState(gsFatalityD,0.0f) > 0.5f);
		}
		break;
	}	

	return false;
}

void CharactersArbiter::SetFatalityAllowe(int num,float allow)
{
	switch (num)
	{
	case 0:
		{
			WriteGameState(gsFatalityA,allow);
		}
		break;
	case 1:
		{
			WriteGameState(gsFatalityB,allow);
		}
		break;
	case 2:
		{
			WriteGameState(gsFatalityC,allow);
		}
		break;
	case 3:
		{
			WriteGameState(gsFatalityD,allow);
		}
		break;
	}	
}

char* CharactersArbiter::CorrectStateId(const char* id)
{
	static char corrected_id[128];

	crt_strcpy(corrected_id,sizeof(corrected_id),"Profile.");
	crt_strcat(corrected_id,sizeof(corrected_id),level_difficulty_name.c_str());
	crt_strcat(corrected_id,sizeof(corrected_id),".");
	crt_strcat(corrected_id,sizeof(corrected_id),id);

	return corrected_id;
}

Character * CharactersArbiter::GetPlayer()
{
//	Assert(m_player);
	return m_player;
	/*MissionObject * mo = Mission().Player();

	if (!mo) return null;

	MO_IS_IF(is_Character, "Character", mo)
	{

		return (Character*)mo;
	}

	return null;*/
}

const ConstString & CharactersArbiter::GetDifficulty()
{
	return level_difficulty_name;
}

void CharactersArbiter::SwitchGodMode()
{
	bool bGODMode = false;

	if (ReadGameState(gsGodMode, 0)>0.5f)
	{
		bGODMode = true;
	}

	bGODMode = !bGODMode;

	if (bGODMode)
	{
		Console().Trace(COL_CMD_OUTPUT, "GOD mode was activated");
	}
	else
	{
		Console().Trace(COL_CMD_OUTPUT, "GOD mode was deactivated");
	}	

	Character* player = GetPlayer();

	if(player)
	{
		if (bGODMode)
		{
			player->logic->SetAttackImune(true);
			WriteGameState(gsGodMode, 1);
		}
		else
		{
			player->logic->SetAttackImune(false);
			WriteGameState(gsGodMode, 0);
		}
	}
}

void CharactersArbiter::AddDeadBody(Character & chr)
{
	if (deadBodies.Find(&chr) != INVALID_ARRAY_INDEX)
		return;

	deadBodies.Add(&chr);
}

void CharactersArbiter::DelDeadBody(Character & chr)
{
	dword idx = deadBodies.Find(&chr);
	if (idx != INVALID_ARRAY_INDEX)
		deadBodies.ExtractNoShift(idx);
}

void _cdecl CharactersArbiter::DeadBodiesWork(float dltTime, long level)
{
	// убираем деактивированных персонажей
	/*for (int i=0; i<deadBodies.Len(); i++)
		if (!deadBodies[i]->IsActive())
		{
			deadBodies.ExtractNoShift(i);
			i--;
		}*/

	// если кол-во тел небольшое - то оставляем их лежать
	if (deadBodies.Len() <= MAX_DEAD_BODIES)
		return;

	const Vector camPos = GetRender().GetView().GetCamPos();
	for (int i=0; i<deadBodies.Len(); i++)
		deadBodies[i]->last_dist2camera = (camPos - deadBodies[i]->last_frame_pos).GetLength2();

	// если тел много, то часть тел убираем
	// сортируем тела по дальности от камеры - дальние убираем первыми
	deadBodies.QSort(CharactersArbiter::SortDeadBodies);
	
	int num = deadBodies.Len() - (MAX_DEAD_BODIES + 1);
	for (int i=num; i>=0; i--)
	{
		if (deadBodies[i]->isDeadBodyInFrustum)
			continue;

		deadBodies[i]->VanishBody(dltTime);
		//stainManager->ClearRad(deadBodies[i]->last_frame_pos, 6.0f, 1.0f);
	}
}

bool CharactersArbiter::SortDeadBodies(Character * const & c1, Character * const & c2)
{
	float k1 = c1->last_dist2camera - c2->last_dist2camera;
	float k2 = c1->deadBodyTime - c2->deadBodyTime;

	return (k1 + k2 > 0.0f) && !c1->isDeadBodyInFrustum;
}

MOP_BEGINLIST(CharactersArbiter, "", '1.00', 0)
MOP_ENDLIST(CharactersArbiter)

