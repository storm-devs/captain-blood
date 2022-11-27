//===========================================================================================================================
// Spirenkov Maxim, 2004
//===========================================================================================================================
// Character
//===========================================================================================================================
// CharacterLogic
//===========================================================================================================================

#include "CharacterLogic.h"
#include "CharacterPhysics.h"
#include "..\CharacterController.h"
#include "..\..\player\PlayerController.h"
#include "CharacterItems.h"
#include "CharacterAchievements.h"

#include "CharacterAnimation.h"
#include "CharacterAnimationEvents.h"
#include "..\..\auxiliary objects\bombsafe\bombsafe.h"
#include "..\Components\ButtonMG.h"

#include "..\..\..\Common_h\ITips.h"

#define SLOWDOWN_IN_HITS

bool CharacterLogic::allow_animslowdown = true;

//============================================================================================
//Инициализация
//============================================================================================

/*
float arrayElement[10];

float GetPoint(float time)
{
	float res = 0.0f;
	long index = long(time);
	time -= index;
	long maxIndex = ARRSIZE(arrayElement) - 1;
	for(long i = -1; i <= 1; i++)
	{
		long idx = index + i;
		if(idx < 0) idx = 0;
		if(idx > maxIndex) idx = maxIndex;
		float t = i - time;
		float k = 1.0f/pow(2.7182818284590452353602874713527f, t*t);
		res += arrayElement[idx]*k;
	}
	return res/sqrtf(PI);
}
*/


CharacterLogic::CharacterLogic(Character * character) : chr(*character), attacks(_FL_), shootTargets(_FL_)
{
	offender = NULL;
	offenderEffectTable = NULL;


	InitMaxHP(100.0f);
	InitHP(100.0f);
	side = s_npc;
	isDead = false;
	isActor = false;
	isAttack = false;
	isPairMode = false;
	turnSpd = 0.0f;
	attackDamage = 0.0f;
	stopPairCounter = 0;
	stopShootCounter = 0;
	aniSpeed = 1.0f;	

	pair = NULL;
	pair_mg = NULL;

	//animStateWasOrIsDieOrDead = false;

	pNoFatality = NULL;

	deathPass = false;

	fMinHPLimiter = -1.0f;

	time_to_return = -1.0f;

	is_attached = 0;
	master = null;
	slave = null;

	bonusWasDroped = false;	
	
	needUpdateState = true;

	allow_orient = true;

	time_scale = 1.0f;

	splahdamage_enabled = false;

	bRageMode = false;

	fRageAttackPow = 1.0f;
	fRageHitPow = 1.0f;

	hp_mult = 1.0f;
	attack_mult = 1.0f;	

	attack_react = null;
	attack_damage = -1.0f;

	attack_imune = false;

	pair_count = 0;
	dir_pair_v = 0.0f;

	check_fatality_state = true;

	auto_aim = false;
	targeted_char = NULL;	
	
	next_splahdamage = -1.0f;

	find_pair_failed = false;

#ifdef CHARACTER_STARFORCE
	time2StartDamageSpikes = NEW float[4];
	time2StartDamageSpikes[0] = RRnd(120.0f, 240.0f);
	time2StartDamageSpikes[1] = RRnd(140.0f, 340.0f);
	time2StartDamageSpikes[2] = RRnd(180.0f, 440.0f);
	time2StartDamageSpikes[3] = RRnd(120.0f, 240.0f);
#endif
}

CharacterLogic::~CharacterLogic()
{	
#ifdef CHARACTER_STARFORCE
	DELETE(time2StartDamageSpikes);
#endif
}

//Дополнительная инициализация, когда есть моделька и анимация
void CharacterLogic::Init()
{		
	if (chr.EditMode_IsOn()) return;	
	static const ConstString fcId("Fatality camera");
	chr.Mission().CreateObject(autoZoom.GetSPObject(),"AutoZoom", fcId);
}

void CharacterLogic::EnableAutoZoom(bool enable)
{
	if (!chr.arbiter->IsMultiplayer() && autoZoom.Ptr())
	{
		autoZoom.Ptr()->Activate(enable);
	}

}

void CharacterLogic::SetHPMultipler(float mult)
{
	hp_mult = mult;
	CorrectHP();
}

void CharacterLogic::Reset()
{
	//if(chr.animation)
	//{
		//chr.animation->Start();
	//}

	//animStateWasOrIsDieOrDead = false;

	chr.animation->SetAnimation("base",0.0f);
	chr.animation->Goto("idle",0.0f);

	if (chr.controller)
	{
		chr.controller->Init();
		chr.controller->Reset();
	}

	HP = GetMaxHP();
	isDead = false;
	isActor = false;
	isPairMode = false;
	chr.isMoneyFatality = false;
	deathPass = false;	
	fMinHPLimiter = -1;

	time_to_return = -1.0f;

	bonusWasDroped = false;

	allow_animslowdown = true;
	aniSpeed = 1.0f;
	
	turnSpd = 0.0f;
	attackDamage = 0.0f;
	stopPairCounter = 0;
	stopShootCounter = 0;
	aniSpeed = 1.0f;	

	pair = NULL;
		
	is_attached = 0;	
	master = null;
	slave = null;
	
	allow_orient = true;

	time_scale = 1.0f;

	splahdamage_enabled = false;

	bRageMode = false;	
}

//Перейти в режим парной анимации
void CharacterLogic::SetPairMode(bool isPair)
{	
	isPairMode = isPair;	
}

bool CharacterLogic::PairModeAllowed(bool ignoreFatalityTime, bool ignore_ai, bool* is_instant)
{	
	if (pNoFatality)
	{
		if (pNoFatality->IsInside(chr.physics->GetPos()))
		{
			return false;
		}
	}

	//return true;	

	if (FParams.Ptr())
	{
		for (int i=0;i<(int)(FParams.Ptr()->Links.Size());i++)
		{
			if (FParams.Ptr()->CheckHPBorder(GetHP()/GetMaxHP(),i))
			{
				FatalityParams::TFatalityType tp = FParams.Ptr()->Links[i].fatal_type;

				if (!ignoreFatalityTime && tp != FatalityParams::ingame_instante)
				{
					if (!ignoreFatalityTime)
					{
						if (chr.GetTimeInFatality() > FATALITY_TIME) continue;
					}

					if (!ignore_ai)
					{
						if (tp == FatalityParams::ai_initiate) continue;
					}
				}
				else
				if (ignoreFatalityTime && tp == FatalityParams::ingame_instante)
				{
					continue;
				}

				if (is_instant)
				{
					if (tp == FatalityParams::ingame_instante)
					{
						*is_instant = true;
					}
				}

				return true;
			}
		}

		return false;
	}
	
	return true;	
}




//Получить имя линка на парную анимацию
const char* CharacterLogic::GetToPairLink(FatalityParams::TFatalityType &type)
{	
	type = FatalityParams::ingame;

	if (FParams.Ptr())
	{
		for (int i=0;i<(int)FParams.Ptr()->Links.Size();i++)
		{
			if (FParams.Ptr()->CheckHPBorder(GetHP()/GetMaxHP(),i))			
			{
				type = FParams.Ptr()->Links[i].fatal_type;

				return FParams.Ptr()->Links[i].Name.c_str();
			}
		}
	}

	return null;
}

void CharacterLogic::PrepareForPairMode (Character* pair_chr,const char* pairLink,FatalityParams::TFatalityType pair_type)
{	
	if (pair_chr->IsPlayer())
	{
		if (pair_chr->controller)
		{
			PlayerController * pc = (PlayerController*)pair_chr->controller;
			//Vano: если Блада пытается кто-то схватить, то выбрасываем оружия, взрываем физику и гренадиров
			pc->DropWeaponBoxSlave();
		}
	}

	if (!pair_chr->animation->TestActivateLink(pairLink)||
		!chr.animation->TestActivateLink(pairLink))
	{
		return;
	}

	crt_strncpy(last_pair_link, sizeof(last_pair_link), pairLink, _TRUNCATE);

	chr.DisableTimeEvent();

	pair = pair_chr;
	pair->SetCharacterCoin(Character::chrcoin_none);
	
	if (pair->IsPlayer())
	{
		pair->logic->pair_mg = &chr;
		pair->DisableTimeEvent();
	}

	//Активируем линк старта парной анимации у врага
	pair->events->SetTimeEvent(false);
	pair->animation->ActivateLink(pairLink,true);
	
	//Активируем линк старта парной анимации у игрока
	chr.events->SetTimeEvent(false);
	chr.animation->ActivateLink(pairLink,true);


	//pair->animation->SetPlaySpeed(1.0f);

	//Ставим парный режим
	chr.logic->SetPairMode(true);
	pair->logic->SetPairMode(true);

	//Дистанция между врагами
	float Distance = Vector(chr.physics->GetPos() - pair->physics->GetPos()).GetLength();

	float pairDistance = 1.5f;

	//Разница в дистанции
	float DistanceDelta = (pairDistance - Distance);

	//2D Направление между персами...
	Vector Direction = Vector(chr.physics->GetPos() - pair->physics->GetPos());
	Direction.y = 0.0f;
	Direction.Normalize();

	//Смещение в плоскости XZ
	Vector vOffset = Direction * DistanceDelta;
	//Смещение по Y
	vOffset.y = chr.physics->GetPos().y - pair->physics->GetPos().y;


	//pair->physics->SetPairDelta(-vOffset);				
	//pair->physics->SetMoveDirection(-vOffset);	
	pair_count = 0;
	dir_pair_v = -Direction;

	last_pair_v = -Direction;
	last_pair_v.y = 0.0f;
	last_pair_v.Normalize();
	last_pair_v *= pairDistance;

	Matrix mat;

	pair->physics->GetModelMatrix(mat);
	chr.physics->Orient(mat.pos, false, true);

	chr.physics->GetModelMatrix(mat);
	pair->physics->Orient(mat.pos, false, true);				

	//Ставим парный режим
	chr.logic->SetPairMode(true);
	pair->logic->SetPairMode(true);
	
	//pair->physics->Activate(false);	
	pair->physics->SetUseFakeModelPosition(true);
	//pair->physics->SetEnableGravity(false);	

	find_pair_failed = false;
		
	if (chr.IsPlayer())
	{
		((PlayerController*)chr.controller)->NotifyCharAboutAttack();

		ITipsManager* tip_manager = ITipsManager::GetManager(&chr.Mission());

		if (tip_manager)
		{
			tip_manager->Enable(false);					
		}

		/*IParticleSystem* pParticale = chr.Particles().CreateParticleSystem("coindust.xps");

		if (pParticale)
		{
			Matrix mtx1(true);
			pair->physics->GetMatrixWithBoneOffset(mtx1);

			Vector pos = mtx1.pos + Vector(0.0f, pair->fCointAltetude - 1.0f + cos (pair->fCoindDltHgt) *0.105f,0.0f);

			pParticale->Teleport(Matrix(0.0f,pos));
			pParticale->AutoDelete(true);										
		}*/
	}
}

bool CharacterLogic::CheckPairLinks(Character* target, const char* action, char* pairLink, FatalityParams::TFatalityType& type, bool isTracePair)
{	
	if(isTracePair)
	{
		api->Trace("Pair: find target: \"%s\"", target->GetObjectID().c_str());
	}

	const char * charName = GetCharName();

	if (!charName || !charName[0])
	{
		if(isTracePair)
		{
			api->Trace("Pair: character don't have a name");
		}
	}

	const char * victimName = target->logic->GetCharName();

	if (!victimName || !victimName[0])
	{
		if(isTracePair)
		{
			api->Trace("Pair: victim don't have a name");
		}
	}

	const char * topairLink = target->logic->GetToPairLink(type);

	if (!topairLink || !topairLink[0])
	{				
		crt_snprintf(pairLink, 127, "%s %s %s",charName,victimName,action);
	}
	else
	{
		if (string::IsEqual(topairLink,""))
		{
			crt_snprintf(pairLink, 127, "%s %s %s",charName,victimName,action);
		}
		else
		{					
			crt_snprintf(pairLink, 127,"%s %s %s %s",charName,victimName,action,topairLink);
		}
	}		

	//Если персонаж не может начать выполнять парную анимацию пропускаем попытку
	if(!target->animation->TestActivateLink(pairLink))
	{
		if (target->IsPlayer() && !target->logic->IsActor() && target->IsActive())
		{
			if (((PlayerController*)target->controller)->IsWeaponOrPhysicsBoxEquiped())
			{
				//if (!target->animation->TestActivateLink(pairLink, true))
				{
					//api->Trace("Pair: PLAYER(LAST_ANIM) can't activate link \"%s\". Current node \"%s\"", pairLink, target->animation->CurrentNode());
					//return false;
				}
			}
		}
		else
		{
			api->Trace("Pair: target can't activate link \"%s\". Current node \"%s\"", pairLink, target->animation->CurrentNode());
			return false;
		}
	}
	
	//Если главный герой не может по какой либо причине начать парную анимацию, ничего не делаем
	if(!chr.animation->TestActivateLink(pairLink))
	{
		api->Trace("Pair: player can't activate link \"%s\". Current node \"%s\"", pairLink, chr.animation->CurrentNode());
		return false;
	}

	return true;
}

void CharacterLogic::PairProcess(float dltTime, int filter)
{	
	CharacterLogic::State state = GetState();

	if (pair && IsPairMode())
	{		
		if (pair->logic->is_attached==0)
		{
			Matrix mat(true);
			chr.physics->GetModelMatrix(mat);
		
			mat.pos = 0.0f;
			Vector dlt = mat * chr.physics->dltPos;		

			last_pair_v -= dlt;

			pair->physics->GetModelMatrix(mat);		
			dlt = mat.MulNormal(pair->physics->dltPos);

			if (!pair->ragdoll)
			{
				Vector det;
				pair->physics->GetFakeModelPosition(det);
	
				//chr.Render().Print(10,10,0xff00ffff,"%4.3f %4.3f %4.3f",det.x,det.y,det.z);

				Vector dt = pair->physics->GetPos();

				//chr.Render().Print(10,30,0xff00ffff,"%4.3f %4.3f %4.3f",dt.x,dt.y,dt.z);

				det = det - dt;
				det.y = 0.0f;
				//det.y = 0.0f;
				//chr.Render().Print(10,50,0xff00ffff,"%4.3f %4.3f %4.3f",det.x,det.y,det.z);
			
				//if (fabs(det.y)<0.0035f)
				{
					chr.physics->Move(-det * 0.15f,true);
				}
			}

			last_pair_v += dlt;

			chr.physics->GetModelMatrix(mat);
			chr.physics->Orient(mat.pos + dir_pair_v, false, true);

			if (!pair->ragdoll)
			{
				//Vector pl_pos  = mat.pos + last_pair_v;
				Vector diff  = mat.pos + last_pair_v - pair->physics->GetPos();
				diff.y = 0.0f;

				pair->physics->Move(diff,false);	
			
				pair->physics->SetFakeModelPosition(mat.pos + last_pair_v);
						
				//chr.Render().DrawSphere(mat.pos + last_pair_v,0.5f);

				//pair->physics->SetPos(mat.pos + last_pair_v);				
				
				pair->physics->GetModelMatrix(mat);
				pair->physics->Orient(mat.pos - dir_pair_v, false, true);			
			}
		}	
	}
	
	if(state != CharacterLogic::state_findpair)
	{		
		find_pair_failed = false;

		if(pair)
		{			
			if (state == CharacterLogic::state_topair || state == CharacterLogic::state_dmg_pair ||
				(state == CharacterLogic::state_pair && (chr.logic->IsPairMode() && pair->logic->IsPairMode())))
			{		
				pair_count = 0;
				return;
			}

			pair_count++;

			if (pair_count>2)
			{
				EndPairProcess();
			}

			return;
		}
	}
	else
	{
		if(!pair)
		{			
			char pairLink[128];
			FatalityParams::TFatalityType type;

			Character * chrForPair = FindCharacterForPair(chr, false, pairLink, type, filter);

			if (chrForPair)
				PrepareForPairMode(chrForPair, pairLink, type);
		}
	}
}

Character * CharacterLogic::FindCharacterForPair(Character & forWho, bool skip_ally, char * pairLink, FatalityParams::TFatalityType & type, int filter, bool * charExist)
{
	//bool skip_ally = false;

	if (charExist)
		*charExist = false;

	if (chr.IsPlayer())
	{
		if (GetFightAct()>1)
		{
			skip_ally = true;
		}
	}

	chr.arbiter->FindCircle(chr, 2.7f);

	if(chr.arbiter->find > 0)
	{				
		//Ищем ближайшую по углу цель
		Vector dir; dir.MakeXZ(chr.physics->GetAy());
		array<CharacterFind> & find = chr.arbiter->find;
		float cs;				

		const char* action = "pair";
		for(long i = 0, j = -1; i < find; i++)
		{					
			Character* finded_chr = find[i].chr;

			if(fabsf(chr.physics->GetPos().y - finded_chr->physics->GetPos().y) > 0.3f) continue;

			if (skip_ally)
			{
				if(!finded_chr->logic->IsEnemy(&chr)) continue;
			}

			if (filter == 1)
			{
				if (finded_chr->logic->GetSide() != s_enemy &&
					finded_chr->logic->GetSide() != s_boss)
				{
					continue;
				}
			}
			else
			if (filter == 2)
			{
				if (finded_chr->logic->GetSide() != s_ally &&
					finded_chr->logic->GetSide() != s_ally_frfire &&
					finded_chr->logic->GetSide() != s_npc)
				{
					continue;
				}
			}


			if(finded_chr->ragdoll) continue;

			float curCos = (dir | find[i].dv);

			if(!finded_chr->logic->PairModeAllowed(!chr.IsPlayer(),true))
			{
				if (curCos>0.8f)
				{												
					find_pair_failed = true;
				}

				continue;
				//action = "kick";
			}

			find[i].dv.y = 0.0f;
			find[i].dist_pow = find[i].dv.NormalizeXZ();
								

			if (curCos<-0.5) continue;

			if(j < 0 || curCos > cs)
			{
				j = i;
				cs = curCos;
			}
		}

		if(j < 0)
		{
			return null;
		}
		
		if (charExist)
			*charExist = true;
		if (!CheckPairLinks(find[j].chr,action,pairLink,type,api->DebugKeyState(VK_NUMPAD0))) return null;
		return find[j].chr;
	}

	return null;
}

void CharacterLogic::EndPairProcess()
{
	//if (chr.logic->IsEvairomentPairMode())
	//{			
	//pair->physics->SetPos(pair->physics->GetPos());
	//chr.physics->SetPos(chr.physics->GetPos());			
	//}

	chr.physics->Activate(true);

	if (pair)
	{
		if (!pair->logic->IsDead() && !pair->ragdoll && pair->GetHP()>0)
		{		
			if (!pair->physics->IsActive())
			{
				if (pair->logic->is_attached==0)
				{
					pair->physics->Activate(true);
				}
			}
			//pair->physics->SetEnableGravity(false);
		}

		/*Character * enemy = (pair->IsPlayer()) ? &chr : pair;
		if (enemy->logic->IsDead() && !enemy->ragdoll && !enemy->logic->IsActor() && enemy->GetHP() <= 0.0f)
		{
			if (!enemy->logic->animStateWasOrIsDieOrDead)
				if (enemy->animation->ActivateLink("death", true) || enemy->animation->Goto("Die", 0.1f))
				{
					animStateWasOrIsDieOrDead = true;
				}
		}*/

		if (!pair->ragdoll) pair->physics->SetUseFakeModelPosition(false);

		pair->logic->SetPairMode(false);
		pair->logic->pair_mg = null;
		pair = null;
		
	}

	/*if (chr.logic->IsEvairomentPairMode())
	{			
	chr.logic->SetEvairomentPairMode(false);
	pair->logic->SetEvairomentPairMode(false);

	Matrix mtx;
	Vector pos;

	mtx = chr.GetBoneMatrix(chr.hips_boneIdx);
	pos = chr.physics->GetPos();
	pos.x = mtx.pos.x;
	pos.y += 0.02f;
	pos.z = mtx.pos.z;				
	chr.physics->SetPos(pos);

	//chr.animation->ActivateLink("idle");

	mtx = pair->GetBoneMatrix(pair->hips_boneIdx);				
	pos = pair->physics->GetPos();				
	pos.x = mtx.pos.x;
	pos.y += 0.02f;
	pos.z = mtx.pos.z;
	pair->physics->SetPos(pos);

	pair->animation->ActivateLink("idle");
	}*/

	//pair->physics->moveOffset = 0.0f;
	//chr.physics->moveOffset = 0.0f;	

	chr.logic->pair=null;	

	if (chr.IsPlayer())
	{
		EnableAutoZoom(false);

	}			

	if (chr.IsPlayer())
	{
		ITipsManager* tip_manager = ITipsManager::GetManager(&chr.Mission());

		if (tip_manager)
		{
			tip_manager->Enable(true);					
		}	
	}

	chr.isMoneyFatality = false;
}

Character* CharacterLogic::FindTarget(float distance,float secktor)
{	
	chr.arbiter->FindCircle(chr, distance);

	float sctr_cos = cosf(secktor);
	Matrix mtx = chr.physics->GetMatrixWithBoneOffset(mtx);		
	Vector pos = mtx.pos;

	static Character* target;
	target = NULL;


	float pl_chr_dst = 1000.0f;
	Vector dir = Vector (0,0,1);
	dir.Rotate(chr.physics->GetAy());

	if(chr.arbiter->find > 0)
	{			
		array<CharacterFind> & find = chr.arbiter->find;

		for(long i = 0; i < find; i++)
		{			
			if (slave == find[i].chr) continue;
			if (find[i].chr->logic->IsDead()) continue;
			if (find[i].chr->logic->GetHP()<0.1f) continue;
			if (find[i].chr->logic->IsPairMode()) continue;
			if (!find[i].chr->logic->IsEnemy(&chr)) continue;

			if (fabs(find[i].chr->physics->GetPos().y-chr.physics->GetPos().y)>0.55f) continue;

			Vector chr_dir;
			chr_dir = find[i].chr->physics->GetPos()-chr.physics->GetPos();			
			float dst = chr_dir.Normalize();
						
			float dt_angle = chr_dir | dir;				
			if (dt_angle<sctr_cos && dst > 1.05f) continue;

			if (!target || (dst < pl_chr_dst))
			{
				target = find[i].chr;
				pl_chr_dst = dst;
			}		
		}
	}

	return target;
}

void CharacterLogic::AimToTarget(float distance,float secktor)
{
	Character* target = FindTarget( distance, secktor);

	if (target)
	{
		chr.physics->Orient(target->physics->GetPos());
	}

	/*if (chr.IsPlayer() && target)
	{
		chr.Render().Print(10,10,0xffffffff,"%s %4.3f",target->GetObjectID().c_str(),pl_chr_angle);
		chr.Render().DrawSphere(target->physics->GetPos(),0.25f);
		chr.Render().DrawLine(target->physics->GetPos(),0xff00ff00,chr.physics->GetPos(),0xff00ff00);
		chr.Render().DrawLine(chr.physics->GetPos()+dir,0xff00ff00,chr.physics->GetPos(),0xff00ff00);		
	}*/
}

//Установить количество жизней
void CharacterLogic::InitHP(float hp)
{
	if(hp < 1.0f) hp = 1.0f;
	HP = hp;
	deathPass = false;	
}

//Установить максимальное количество жизней
void CharacterLogic::InitMaxHP(float hp)
{
	if(hp < 1.0f) hp = 1.0f;
	maxHP = hp;
	deathPass = false;	
}

//Установить новое значение HP
void CharacterLogic::SetHP(float hp,bool drop_armor)
{
	float dmg = HP - hp;

	HP = Clampf(hp, 0.0f, GetMaxHP());
	CorrectHP();

	if (IsPairMode()) return;

	if (chr.EditMode_IsOn()) return;

	if (!drop_armor) return;

	chr.items->DropArmor(dmg,dmg);	
}

//Сделать персонажа актёкром
void CharacterLogic::SetActor(bool isEnable)
{
	if(isActor == isEnable) return;
	isActor = isEnable;
	chr.Activate(chr.IsActive());
}

//Умереть
void CharacterLogic::Death(dword source, const char* DeathLink)
{
	if (chr.logic->slave && chr.IsPlayer() && chr.controller)
	{
		// Vano: выбрасываем бомбардира на плече
		PlayerController * pc = (PlayerController*)chr.controller;
		pc->DropWeaponBoxSlave();
	}

	if(!isDead)
	{		 
		if (IsPairMode() && pair)
		{
			EndPairProcess();
		}

		chr.controller->Reset();
		chr.controller->Death();

		if (GetOffender())
		{
			chr.killerId = GetOffender()->GetObjectID();
			
			chr.arbiter->Death(source, chr, GetOffender());

			if (side != s_npc)
			{
				if (GetOffender()->chrAchievements)
					GetOffender()->chrAchievements->AnalyseDeath(&chr);

				//if (IsPairMode() && GetOffender()->isRealFatalityStarted)
				if (IsPairMode() && GetOffender()->isRealFatalityStarted)
				{
					GetOffender()->isRealFatalityStarted = false;
					chr.arbiter->StatsFatalitiesAdd();
				}
			}
		}
		else
		{
			if (IsPairMode())
				chr.arbiter->Death(source, chr, chr.arbiter->GetPlayer());
		}

		HP = 0.0f;

		if (IsPairMode())
		{
			isDead = true;

			if(DeathLink && chr.animation->ActivateLink(DeathLink,true))
			{
				isDead = true;
			}
			else
			if(chr.animation->ActivateLink("death",true))
			{
				isDead = true;
			}
		}
		else
		{
			if (DeathLink)
			{
				if(chr.animation->ActivateLink(DeathLink,true))
				{
					isDead = true;
				}
				else
				if(chr.animation->ActivateLink("death",true))
				{
					isDead = true;
				}
				else
				{
					if (GetState() != state_die && GetState() != state_dead)
					{
						chr.animation->Goto("Die",0.1f);
					}

					isDead = true;
				}
			}
			else
			if(chr.animation->ActivateLink("death",true))
			{
				isDead = true;
			}
			else
			{
				if (GetState() != state_die && GetState() != state_dead)
				{
					chr.animation->Goto("Die",0.1f);
				}

				isDead = true;
			}
		}

		//chr.Unregistry(MG_SHADOWCAST);
		//chr.Unregistry(MG_SHADOWRECEIVE);		


        if(chr.controller && !chr.IsPlayer())// && !bonusWasDroped)
		{
			Matrix mtx(true);
			chr.physics->GetModelMatrix(mtx);

			Matrix joint_mat = chr.GetBoneMatrix(chr.spine_boneIdx);		
			mtx = joint_mat * mtx;

			BonusesManager::DropParams params;

			params.minVy = 5.0f;
			params.maxVy = 5.0f;
			params.minVxz = 5.0f;
			params.maxVxz = 5.0f;
			params.friction = 5.0f;
			params.minAy = 0; 
			params.maxAy = 2 * PI;		
			
			params.skipLifeTime = 0.0f;

			BonusesManager::CreateBonus(chr.Mission(),mtx.pos,chr.bonusTable,&params);
			bonusWasDroped = true;
		}	
	}
}

//Получить состояние персонажа из анимации
float CharacterLogic::GetBlockState()
{
	const char * st = chr.animation->GetConstString("block_chance");
	if (!st) return -1.0f;
	return float(atof(st));
}

CharacterLogic::State CharacterLogic::GetCurrentState()
{
	const char * st = chr.animation->GetConstString("state");
	if(!st) return state_unknown;
	switch(st[0])
	{
	case 'i':
		if(st[1] == 'm') return state_imbo;
		return state_idle;
	case 'w':
		return state_walk;
	case 'r':
		if(st[1] == 'u') return state_run;
		if(st[1] == 'o') return state_roll;
		return state_run;
	case 's':
		if(st[1] == 't') return state_strafe;
		return state_shoot;
	case 'a':
		if(st[1] == 't') return state_attack;
		return state_aimbomb;
	case 'b':
		return state_block;
	case 'g':
		return state_guard;
	case 'f':
		return state_findpair;
	case 't':
		return state_topair;
	case 'p':
		return state_pair;
	case 'k':
		if(st[1] == 'i') return state_kick;
		return state_knockdown;
	case 'h':
		return state_hit;
	case 'd':
		if(st[1] == 'r') return state_dropbomb;
		if(st[1] == 'i') return state_die;
		if(st[1] == 'm') return state_dmg_pair;
		
		return state_dead;
	default:
		return state_unknown;
	}
}

void CharacterLogic::NeedStateUpdate()
{
	needUpdateState = true;
}

//Получить состояние персонажа из анимации
CharacterLogic::State CharacterLogic::GetState()
{
	//return GetCurrentState();

	if (needUpdateState)
	{
		cachedState = GetCurrentState();
		needUpdateState = false;
	}

	return cachedState;
}

//Получить состояние персонажа из анимации
const char* CharacterLogic::GetStateString()
{
	return chr.animation->GetConstString("state");	
}

//Получить текстовое описание стейта
const char * CharacterLogic::GetStateText(State state)
{
	switch(state)
	{
	case state_unknown:
		return "unknown";
	case state_idle:
		return "idle";
	case state_walk:
		return "walk";
	case state_run:
		return "run";
	case state_strafe:
		return "strafe";
	case state_attack:
		return "attack";
	case state_block:
		return "block";
	case state_shoot:
		return "shoot";
	case state_findpair:
		return "find pair";
	case state_topair:
		return "to pair";
	case state_pair:
		return "pair";
	case state_knockdown:
		return "knockdown";
	case state_kick:
		return "kick";
	case state_hit:
		return "hit reaction";
	case state_die:
		return "die";
	case state_dead:
		return "dead";
	case state_aimbomb:
		return "aim a bomb";
	case state_dropbomb:
		return "drop bomb";
	case state_guard:
		return "guard";
	case state_imbo:
		return "imbo";
	}
	return "unknow state";
}

//Враг персонаж или нет
//Реальная проверка
bool CharacterLogic::IsRealEnemy(Character* opponent, bool includeNPC)
{
	if (!opponent) return false;
	if (opponent == &chr) return false;

	if (side == s_npc && !includeNPC) return false;
	if (side == s_boss || side == s_npc) return true;

	if (opponent)
	{
		if(opponent->logic->side == s_boss && side == s_enemy) return false;

		if(opponent->logic->side == s_ally_frfire && side == s_ally_frfire) return true;

		if(opponent->logic->side == side) return false;
	}
	else
	{
		if(s_ally == side) return false;
	}

	return true;
}

bool CharacterLogic::IsSlave()
{
	return (master != null);
}

//Враг персонаж или нет
//Для плеера, даже друг возвращается как враг если рядом нет других врагов
bool CharacterLogic::IsEnemy(Character* opponent, bool includeNPC)
{
	if (!opponent) return false;

	if (opponent == &chr) return false;

	if (chr.IsPlayer())
	{
		if (chr.arbiter->GetPowDistance(&chr,opponent) < 25.0f)
			if (!chr.arbiter->IfEnemyCharacer(&chr, 5.0f, true))
				return true;
	}

	return IsRealEnemy(opponent, includeNPC);
}

//Пнуть персонажа в заданном направление
void CharacterLogic::Kick(const Vector & dir)
{
	if(isActor) return;
	if(chr.animation->ActivateLink("kick"))
	{
		chr.physics->Kick(dir);
	}
}

//Бросить бомбу
void CharacterLogic::DropBomb(const Vector & boomPosition)
{
	if(chr.animation->ActivateLink("drop bomb"))
	{
		chr.SetBombTarget(boomPosition);
	}
}

//Переход в режим атаки и обратно
void CharacterLogic::SetAttackActive(bool isAttackNow)
{
	if(isAttack == isAttackNow) return;
	attacks.Empty();
	turnSpd = Max(turnSpd, chr.physics->GetTurnSpeed());
	isAttack = isAttackNow;


	const float angle = 45.0f;
	chr.arbiter->Find(chr, 0.02f, 2.5f, false, -angle*(PI/180.0f), angle*(PI/180.0f), 0);
	array<CharacterFind> & fnd = chr.arbiter->find;	
	if(fnd)
	{
		float fMindist = fnd[0].dist_pow;
		long MinIndex = 0;

		for (int n = 1; n < (int)fnd.Size(); n++)
		{
			if (fnd[n].dist_pow < fMindist)
			{
				MinIndex = n;
				fMindist = fnd[n].dist_pow;
			}
		}

		chr.physics->Orient(fnd[MinIndex].chr->GetMatrix(Matrix()).pos, true);
	}

	if(isAttack)
	{
		chr.physics->SetTurnSpeed(0.0f);
	}else{
		chr.physics->SetTurnSpeed(turnSpd);
	}

}

//Атаковать демедж детектор
void CharacterLogic::Attack(DamageObject * obj, const Vector quadrangle[4])
{
	if(!obj) return;
	//Проверяем на возможность атаки
	for(long i = 0; i < attacks; i++)
	{
		if(attacks[i] == obj)
		{
			return;
		}
	}
	float dmg = 0.0f;
	if(chr.animation)
	{
		dmg = chr.animation->GetConstFloat("damage");
	}
	if(obj->Attack(&chr, DamageReceiver::ds_sword, dmg, quadrangle))
	{
		attacks.Add(obj);
	}
}

//Атаковать заданного персонажа
bool CharacterLogic::Attack(Character * ch, dword source)
{
	if(!ch) return false;
	if(!IsEnemy(ch)) return false;
	if(ch->logic->IsPairMode()) return false;
	if (ch->logic->IsActor()) return false;

	//Проверяем на возможность атаки
	for(long i = 0; i < attacks; i++)
	{
		if(attacks[i] == ch)
		{
			return false;
		}
	}
	//Получим параметры попадания
	float dmg = 0.0f;
	const char * reaction = "hit";
	const char * blockReaction = "blockhit";	
	const char * deathLink = "death";
	float hpLimit = -1.0f;
	float block_dmg = 0.0f;

	dmg = chr.animation->GetConstFloat("damage");
		
	const char * react = chr.animation->GetConstString("hit");
	if(react && react[0]) reaction = react;
		
	react = chr.animation->GetConstString("hitblk");
	if(react && react[0]) blockReaction = react;

	react = chr.animation->GetConstString("deathlnk");
	if(react && react[0]) deathLink = react;

	hpLimit = chr.animation->GetConstFloat("HPLimit");
	block_dmg = chr.animation->GetConstFloat("block_damage");

	if (attack_damage>0)
	{
		dmg = attack_damage;
	}

	if (!ch->IsPlayer())
	{
		if (attack_react)
		{
			reaction = attack_react;
		}
	}
	else
	{
// Срабатывание надоедливой защиты
#ifdef CHARACTER_STARFORCE
		float ds = UpdateDamageSpikes<float>();
		float kdmg = 1.0f;
		if (ds > 1e-2f)
		{
			float kkf = Min(1.0f, float(int(chr.arbiter->gameTicks->Get(0.0f) / 1972.0f)));
			kdmg = 1.0f + kkf * ((73.0f - chr.arbiter->GuardingValueDamage<int>(0)) * ds);
		}

		AssertStarForce(fabsf(kdmg - 1.0f) < 1e-5f);
		dmg *= kdmg;
#endif
	}

	if (bRageMode)
	{
		float rageMultiply = ch->fRageDmgMultiplayer * (fRageAttackPow - 1.0f);

		blockReaction = "blockBreak";
		dmg = dmg * (1.0f + rageMultiply);
		block_dmg = block_dmg * (1.0f + rageMultiply);
	}

	dmg *= attack_mult;	

	//reaction = "HeadOff";

	//Отметим попадание
	attacks.Add(ch);

	float block_spike = 0.0f;
#ifdef CHARACTER_STARFORCE
		if (chr.arbiter->timeInSecFromStart->Get(0.0f) > 2001.0f)
		{
			int ds = UpdateDamageSpikes<int>();
			block_spike = 15.0f * (1.0f + ((73.0f - chr.arbiter->GuardingValueDamage<float>(2)) * float(ds)));
			if (block_spike < 100.0f)
				block_spike = 0.0f;

			AssertStarForce(fabsf(block_spike) < 1e-5f);
		}
#endif

	ch->logic->Hit(source, dmg, reaction, hpLimit, block_dmg + block_spike, blockReaction, deathLink, &chr);

	if (chr.controller)
	{
		chr.controller->Attack(ch);
	}
	

	Vector vDir = chr.physics->GetPos() - ch->physics->GetPos();

	vDir.Normalize();

	ch->physics->vAttackerDir = vDir;


#ifdef SLOWDOWN_IN_HITS
	//if(chr.isPlayer/* && aniSpeed > 0.99f*/ && allow_animslowdown)
	//{
	//	aniSpeed = 0.0f;
	//}
#endif

	return true;
}

bool CharacterLogic::WasAttacked(Character* ch)
{
	for (int i=0;i<attacks;i++)
	{
		if (attacks[i] == ch) return true;		
	}

	return false;
}

//Попадание холодным оружием
void CharacterLogic::Hit(dword source, float dmg, const char * reaction, float hpLimit, float block_dmg, const char * blockReaction, const char * deathLink, Character* _offender, bool checkImmune, bool play_hit)
{
	if (master) return;

	if (GetState() == state_guard)
	{
		if (_offender)
		{			
			if (_offender->animation->TestActivateLink(chr.animation->GetConstString("hit")))
			{
				_offender->logic->Hit(source, chr.animation->GetConstFloat("damage"),		
								  chr.animation->GetConstString("hit"),0.0f,0,NULL,NULL,&chr,true,false);
				return;
			}			
		}		
	}

	if (chr.ragdoll) return;
	if (IsPairMode()) return;

	//FIX ME !!!
	if (dmg <= 0) return;
	//Assert (dmg > 0)
	if(IsHitImmune() && checkImmune) return;
	State st = GetState();
	
	float offenderDmgMultiplier = (_offender) ? _offender->fTotalDmgMultiplier : 1.0f;

	attackDamage += chr.items->CalcDamage(dmg) * chr.fHitDmgMultiplayer * offenderDmgMultiplier;
	
	offender = _offender;

	if (offender)
	{
		offenderEffectTable = offender->items->lastactive_effectTable;
	}
	else
	{
		offenderEffectTable = null;
	}
	

	if(st == state_block || st == state_guard)
	{	
		attackDamage = block_dmg;		

		if (chr.animation->TestActivateLink(blockReaction))
		{
			//if (!string::IsEqual(blockReaction,"BlockHit"))
			//{
				//reaction = blockReaction;
			//}

			reaction = blockReaction;
		}
		else
		{
			reaction = "BlockHit";
		}	
	}	

	if (bRageMode)
	{
		attackDamage *= fRageHitPow;		
	}

	if (offender)
	{
		if (offender->IsPlayer())
		{
			chr.physics->Orient(offender->physics->GetPos());

			if (!IsEnemy(offender))
			{
				reaction = "BloodHit";	
				attackDamage = 0;
			}
		}
	}

	if (IsAttackImune())
	{
		attackDamage = 0.0f;
	}

	// Fucking Hack!!!
	float atck = attackDamage;	

	HitMoment(reaction);

	if (IsDead())
	{
		if (offender)
		{
			if (offender->logic->IsRageMode()) deathLink = "RageDeath";

			if (offender->IsPlayer() && chr.arbiter->IsSlowmoReady())
			{
				float probability = 0.1f + atck * (1.0f / 2048.0f) + offender->killsPerSecond * 0.05f;
				if (Rnd() < probability)
					((PlayerController*)(offender->controller))->StartSloMo(1.05f, "0.3");
			}

			offender->killsPerSecond += 1.0f;
		}
		
		// FIX-ME возможно надо достать какое оружие било
		Death(source, deathLink);
	}
	else
	{		
		if (!string::IsEqual(reaction,"NoReaction") && !bRageMode)
		{	
			if (GetHP()<=hpLimit || hpLimit<=0.0f)
			{
				if (!chr.animation->ActivateLink(reaction,true))
				{
					api->Trace("Hits: char \"%s\" in node \"%s\" can't activate link - %s", chr.GetObjectID().c_str(), chr.animation->CurrentNode(),reaction);
					if (play_hit) chr.animation->ActivateLink("hit",true);
				}
			}
			else				
			{
				chr.animation->ActivateLink("hit",true);
			}
		}
		else
		{
			chr.BornParticle("bfbodyf","bloodsplash.xps");
		}

		bool cntr_hit = true;
		
		if (offender)
		{
			if (!IsEnemy(offender)) cntr_hit = false;
		}		
		
		if (cntr_hit)
		{
			chr.controller->Hit(atck, offender, reaction, DamageReceiver::DamageSource(source));
		}	
	}
		

#ifdef SLOWDOWN_IN_HITS
	//if(!chr.isPlayer && aniSpeed > 0.99f && allow_animslowdown)
	//{
		////aniSpeed = 0.0f;
	//}
#endif

	//if (attackDamage<=0) return;	
}

//Применить попадание
void CharacterLogic::HitMoment(const char * reaction)
{		
	if (attackDamage<=0) return;

	// FIX-ME: не рожать кровь если чар очень далеко и не в камере
	Character * player = chr.arbiter->GetPlayer();
	
	bool addStain = true;
	if (player && (chr.arbiter->GetPowDistance(&chr, player) >= Sqr(15.0f) && offender != player))
		addStain = false;

	if (addStain)
	{
		chr.BloodStain(attackDamage, offender);
	}

	time_to_return = GetCrossLinkTime();	

	SetHP(HP-attackDamage);	
	
	if (PairModeAllowed(true,false) && offender)
	{
		if (offender->isPlayer && side != s_npc)
		{
			if(chr.arbiter->IsFatalityAllowed(0) || chr.arbiter->IsFatalityAllowed(1) ||
			   chr.arbiter->IsFatalityAllowed(2) || chr.arbiter->IsFatalityAllowed(3) || !check_fatality_state)				
			{
				chr.SetCharacterCoin(Character::chrcoin_fatality);
			}
		}
	}


	attackDamage = 0.0f;
	chr.isHitLight = true;
}

void CharacterLogic::SetShootTarget(Character* target,const char* hit_react,float fDamageKoef, bool bFlyAway,bool bAppend)
{
	if (!target)
	{
		shootTargets.DelAll();

		return;
	}

	TShootTarget shootTarget;

	shootTarget.target = target;
	shootTarget.bFlyAway = bFlyAway;
	shootTarget.fDamageKoef = fDamageKoef * attack_mult;
	crt_strcpy(shootTarget.react,sizeof(shootTarget.react),hit_react);

	if (!bAppend)
	{
		shootTargets.DelAll();
	}

    shootTargets.Add(shootTarget);
	
	Vector vDir = target->physics->GetPos() - chr.physics->GetPos();
	vDir.Normalize();
	vDir.y = 0.5f;
	target->physics->vFlyDir = vDir * 4.0f;
}

bool CharacterLogic::IsMultipleShootTargets()
{
	if (shootTargets.Size()>1)
	{
		return true;
	}

	return false;
}

bool CharacterLogic::IsShotImmune()
{
	if (chr.IsShotImmuneWhenRoll() && GetState() == state_roll)
		return true;

	return false;
}

//Попадание выстрела
void CharacterLogic::ShootHit(float dmg)
{
	//Vano: сбрасываем ачивку одновременного убийства из пистолета нескольких челов
	ACHIEVEMENT_SETZERO(&chr, SNIPER);

	int numRagdolls = 0;
	int numKills = 0;
	int numRealEnemies = 0;

	for (int i=0;i<(int)shootTargets.Size();i++)
	{	
		Character * target = shootTargets[i].target;
		CharacterLogic * target_logic = target->logic;
		
		bool & shootHit = target->isHitLight;
		
		if (target_logic->IsHitImmune() || target_logic->IsShotImmune())
			continue;

		float damage = dmg * shootTargets[i].fDamageKoef * target->fShootDmgMultiplayer;
		
		if (chr.controller)
		{
			chr.controller->Attack(target);
		}

		if (shootTargets[i].bFlyAway && chr.IsPlayer())
		{		
			target_logic->Hit(DamageReceiver::ds_bullet, damage, "ShootHit", 0.0f, 1.0f, "ShootHit", "ShootDeath", &chr);
			shootHit = true;
			numRagdolls++;
		}
		else
		{
			const char * blockReaction = (chr.IsPlayer()) ? shootTargets[i].react : "ShotHit";
			target_logic->Hit(DamageReceiver::ds_bullet, damage, shootTargets[i].react, 0.0f, 0.0f, blockReaction, NULL, &chr);
		}

		bool isRealEnemy = target_logic->IsRealEnemy(&chr, false);
		
		if (isRealEnemy)
			numRealEnemies++;

		if (chr.IsPlayer() && target_logic->IsDead() && isRealEnemy)
			ACHIEVEMENT_REACHED(&chr, SNIPER);

		chr.arbiter->AttackPhyObj(&chr,DamageReceiver::ds_bullet, dmg,chr.physics->GetPos(), shootTargets[i].target->physics->GetPos());
		
		if (target_logic->GetHP() <= 0.0f)
			numKills++;
	}	

	if (chr.shoot_effectTable && chr.IsPlayer())
	{
		//Vector shoot_v_s[5];
		//Vector shoot_v_e[5];

		IPhysicsScene::RaycastResult res;

		int nums = (int)(Rnd(6)) + 1;

		Matrix mat;
		chr.physics->GetModelMatrix(mat);

		Vector pos = chr.physics->GetPos() + Vector(0.0,1,0.0);

		for (int i = 0;i < nums; i++)
		{
			Vector dir = Vector(RRnd(-0.25f,0.25f),RRnd(-0.25f,0.25f),1.0f);
			dir = mat.MulNormal(dir);
			dir *= 10.0f; 			

			if (!chr.arbiter->AttackPhyObj(&chr,DamageReceiver::ds_bullet,dmg,pos, pos + dir))
			{			
				if (chr.Physics().Raycast( pos, pos + dir, phys_mask(phys_character),&res))
				{
					chr.shoot_effectTable->MakeEffect((PhysTriangleMaterialID)res.mtl,&chr,res.position,res.normal);				
				}
			}			
		}
	}

	// Проверяем, может стоит запустить slowmo
	if (chr.IsPlayer() && chr.arbiter->IsSlowmoReady() && numRealEnemies)
	{
		float probability = float(numRagdolls + numKills * 5) * 0.02f;
		if (Rnd() < probability)
			((PlayerController*)chr.controller)->StartSloMo(1.05f, "0.3");
	}
}

//Невозможность нанесения повреждения
bool CharacterLogic::IsHitImmune()
{
	State st = GetState();
	switch(st)
	{
	//case state_findpair:
	case state_topair:
	case state_pair:
	//case state_knockdown:
	case state_kick:
	case state_die:
	case state_dead:
	case state_imbo:
		return true;
	}
	if(isActor) return true;
	if(!chr.IsActive()) return true;
	return false;
}

void CharacterLogic::UpdateFatalityParams ()
{
	if (chr.EditMode_IsOn()) return;

	if (!FParams.Ptr() && chr.FatalityParamsId.NotEmpty())
	{		
		static const ConstString strTypeId("FatalityParams");
		FParams.FindObject(&chr.Mission(),chr.FatalityParamsId,strTypeId);
		
		if (FParams.Ptr())
		{			
			FParams.Ptr()->FindCurHPBorder(chr.arbiter->GetDifficulty());			
		}			

		MGIterator* iter;

		iter = &chr.Mission().GroupIterator(NoFatality_GROUP, _FL_);

		if (!iter->IsDone())
		{
			pNoFatality = (NoFatality*)iter->Get();
		}

		iter->Release();

		/*
		MGIterator* iter;

		iter = &chr.Mission().GroupIterator(FATALITY_PARAMS_GROUP, _FL_);

		if (strcmp(chr.FatalityParams,"")==0)
		{		
			if (!iter->IsDone())
			{
				FParams = (FatalityParams*)iter->Get();
			}
		}
		else
		{
			for (;!iter->IsDone();iter->Next())
			{
				if (strcmp(chr.FatalityParams,iter->Get()->GetObjectID().c_str())==0)
				{
					FParams = (FatalityParams*)iter->Get();
				}
			}
		}

		iter->Release();		
		*/
	}

	if (!attackParams.Ptr() && chr.AttackParams.NotEmpty())
	{
		static const ConstString strTypeId("AttackParams");
		attackParams.FindObject(&chr.Mission(),chr.AttackParams,strTypeId);
		
		if (attackParams.Ptr())
		{
			attackParams.Ptr()->Init(attacks_params_st);
		}		
	}
	
}

void  CharacterLogic::UpdateAttacks(aiActionStack* actionStack)
{
	if (attackParams.Ptr())
	{
		attackParams.Ptr()->Update(GetHP()/GetMaxHP(), attacks_params_st, actionStack);
	}
}

//Обновить логическое состояние персонажа
void CharacterLogic::Update(float dltTime)
{
	/*const char * curNode = chr.animation->GetCurAnimation()->CurrentNode();
	if (string::IsEqual(chr.GetObjectID().c_str(), "part1_enemy_4_1"))
		api->Trace("cn: %s", curNode);

	animStateWasOrIsDieOrDead |= (GetState() == state_die || GetState() == state_dead);*/

	if (chr.items->IfAttackWeapon())
	{
		if (targeted_char)
		{
			if (targeted_char->logic->IsActor() || targeted_char->logic->IsPairMode() ||
				targeted_char->logic->GetHP() < 0.1f ||
				!targeted_char->IsShow() || !targeted_char->IsActive())
			{
				targeted_char = null;
			}
		}

		if (!targeted_char)
		{
			targeted_char = FindTarget(2.5f,0.5f * PI);
		}

		if (targeted_char)
		{
			chr.physics->Orient(targeted_char->physics->GetPos());
		}
	}
	else
	{
		EnableAutoAim(false);
	}

	if (chr.logic->GetState() != CharacterLogic::state_attack)
	{
		allow_orient = true;
	}
	
	if (splahdamage_enabled)
	{
		if (chr.logic->GetState() != CharacterLogic::state_attack)
		{
			splahdamage_enabled = false;
		}
		else
		{
			next_splahdamage -= dltTime;

			if (next_splahdamage<0.0f)
			{
				chr.arbiter->SplashDamage(DamageReceiver::ds_sword, &chr,chr.physics->GetPos(),2.0f,20,false,null,true,0.5f);
				next_splahdamage = 0.33f;
			}
		}
	}

	if (time_to_return>0)
	{
		time_to_return -= dltTime;

		if (time_to_return<0.0f)
		{			
			chr.animation->ActivateLink("crosslink",true);
			time_to_return = GetCrossLinkTime();
		}
	}



	/*for (int i=0; i<5; i++)
	{
		chr.Render().DrawLine(shoot_v_s[i],0xff00ff00,shoot_v_e[i],0xff0000ff);
	}*/

#ifdef SLOWDOWN_IN_HITS
	//if(chr.animation && chr.isPlayer)
	{
		/*aniSpeed += dltTime*2.0f;
		if(aniSpeed > 1.0f) aniSpeed = 1.0f;
		float k = 1.0f;
		float time = 0.27f;
		float t1 = time*0.00001f;
		float t2 = time*0.9f;
		if(aniSpeed < time)
		{
			if(aniSpeed < t1)
			{
				k = 1.0f - aniSpeed/t1;
			}else
			if(aniSpeed > t2)
			{
				k = (aniSpeed - t2)/(time - t2);
			}else{
				k = 0.0f;
			}
		}

		//if (allow_animslowdown && !chr.isPlayer)	chr.animation->SetPlaySpeed((k*0.97f) + 0.03f);
		if (allow_animslowdown) chr.animation->SetPlaySpeed((k*0.97f) + 0.03f);*/

		/*static bool ququ = false;
		if (api->DebugKeyState('M'))
			ququ = true;

		if (ququ)
		{
			chr.animation->SetPlaySpeed(3.0f);
			//chr.logic->SetTimeScale(3.0);
		}*/
	}	
#endif

	//Если перестаём стрелять - теряем цель
	/*if(shootTargets.Size()>0)
	{
		if(GetState() != state_shoot)
		{
			stopShootCounter++;
			if(stopShootCounter > 5)
			{
				shootTargets.DelAll();
			}
		}
		else
		{
			stopShootCounter = 0;
		}
	}
	else
	{
		stopShootCounter = 0;
	}*/

	if(chr.physics->IsKickState())
	{
		if(GetState() != state_kick && GetState() != state_knockdown)
		{
			stopKickCounter++;
			if(stopKickCounter > 5)
			{
				chr.physics->StopKick();
			}
		}else{
			stopKickCounter = 0;
		}
	}else{
		stopKickCounter = 0;
	}
	//Контролируем выход из прного режима
	if(isPairMode)
	{
		switch(GetState())
		{
			case state_findpair:
			case state_topair:
			case state_pair:
			case state_dmg_pair:
			{
				stopPairCounter = 0;
			}			
		}
		stopPairCounter++;
		if(stopPairCounter > 5)
		{
			SetPairMode(false);
		}
		//return;
	}
	else
	{
		stopPairCounter = 0;
	}
	//Если мертвы - умараем
	if(!isDead && !deathPass)
	{
		if(IsDead())
		{
			// FIX-ME - от чего это умирание?
			if (isPairMode)
				SetOffender(chr.arbiter->GetPlayer());
			Death((isPairMode) ? DamageReceiver::ds_sword : htds_force_dword);
			deathPass = true;

			return;
		}
	}
}

//Получить имя линка из текущего графа в графе главного персонажа для парной анимации
const char * CharacterLogic::GetPairLink(char* NodeName)
{
	if(chr.animation)
	{
		if (NodeName==NULL)
		{		
			return chr.animation->GetConstString("pairlink", "pair");
		}
		else
		{		
			return chr.animation->GetConstString("pairlink", NodeName);
		}
	}

	return null;
}

bool CharacterLogic::AllowOrient()
{
	if (!allow_orient || !chr.animation) return false;

	const char * orient = chr.animation->GetConstString("orient");	

	return (!orient || (orient[0] == 'y'));
}

bool CharacterLogic::AllowOrientHead()
{
	if (!chr.animation) return true;

	const char* orient = chr.animation->GetConstString("headorient", chr.animation->CurrentNode());	


	if (chr.IsPlayer())
	{
		if (orient)
		{
			if (orient[0] == 'y' || orient[0] == 'Y')
			{
				return true;
			}

			return false;
		}

		return false;
	}


	if (orient)
	{
		if (orient[0] == 'n' || orient[0] == 'N')
		{
			return false;
		}

		return true;
	}

	return true;
}



float CharacterLogic::GetCrossLinkTime()
{
	if (!chr.animation) return -1.0f;

	const char* crossLink = chr.animation->GetConstString("crossLink", chr.animation->CurrentNode());	

	if (crossLink)
	{
		char* time = null;
		return (float)strtod(crossLink, &time);		
	}

	return -1.0;
}

//Получить имя линка из текущего графа в графе главного персонажа для парной анимации
const char * CharacterLogic::GetCharName()
{
	if(chr.animation)
	{
		return chr.animation->GetConstString("pairlink", "idle");
	}
	return null;
}

bool CharacterLogic::InBombSafe(Vector pos)
{
	MGIterator* iter;

	iter = &chr.Mission().GroupIterator(BombSafe_GROUP, _FL_);

	while(!iter->IsDone())
	{
		BombSafe* pBombSafe = (BombSafe*)iter->Get();

		if (pBombSafe->IsActive())
		{		
			if (-pBombSafe->size.x*0.5f+pBombSafe->transform.pos.x<pos.x&&pos.x<pBombSafe->size.x*0.5f+pBombSafe->transform.pos.x&&
				-pBombSafe->size.y*0.5f+pBombSafe->transform.pos.y<pos.y&&pos.y<pBombSafe->size.y*0.5f+pBombSafe->transform.pos.y&&
				-pBombSafe->size.z*0.5f+pBombSafe->transform.pos.z<pos.z&&pos.z<pBombSafe->size.z*0.5f+pBombSafe->transform.pos.z)
			{
				return true;
			}				
		}

		iter->Next();
	}

	iter->Release();

	return false;
}

void CharacterLogic::OrientBodyPart(bool isHead)
{
	if (chr.logic->IsPairMode())
	{
		chr.bodyparts.SetRotation(0.0f,isHead);
		return;
	}

	if( !AllowOrientHead())
	{
		chr.bodyparts.SetRotation(0.0f,isHead);
		return;
	}

	chr.arbiter->Find(chr, 0.5f, 4.0f, false,-PI*0.45f,PI*0.45f);

	Matrix mtx = chr.physics->GetMatrixWithBoneOffset(mtx);		
	Vector pos = mtx.pos;

	Character* target = null;

	float pl_chr_angle = 1000.0f;
	Vector dir = Vector (0,0,1);
	dir.Rotate(chr.physics->GetAy());

	if(chr.arbiter->find > 0)
	{			
		array<CharacterFind> & find = chr.arbiter->find;

		for(long i = 0; i < find; i++)
		{						
			if (find[i].chr->logic->IsDead()) continue;
			if (find[i].chr->logic->GetHP()<0.1f) continue;
			if (find[i].chr->logic->IsPairMode()) continue;
			if (!find[i].chr->logic->IsEnemy(&chr)) continue;

			Vector chr_dir;
			chr_dir.Rotate(find[i].chr->physics->GetAy());

			float dt_angle = fabs(dir.GetAngle(chr_dir));

			if (!target || (dt_angle<pl_chr_angle))
			{
				target = find[i].chr;
				pl_chr_angle = dt_angle;
			}			
		}
	}

	if (target)
	{	
		Vector dir = target->physics->GetPos() - chr.physics->GetPos();
		dir.Normalize();

		Vector chr_dir = Vector(1,0,0);
		chr_dir.Rotate(chr.physics->GetAy());

		float angle = chr_dir.GetAngle(dir);

		angle = Clampf (angle,-PI,PI);
		angle = - angle + PI * 0.5f;

		chr.bodyparts.SetRotation(angle,isHead);

		//chr.Render().Print(10,10,0xff00ffff,"%f",angle);
	}
	else
	{
		chr.bodyparts.SetRotation(0.0f,isHead);
	}
}
void CharacterLogic::SetSpawnPoint(Vector pt)
{
	if (!chr.auto_spawnpoint)
	{
		spawn_point = chr.init_spawnpoint;
	}
	else
	{
		spawn_point = pt;
	}
}

Vector CharacterLogic::GetSpawnPoint()
{
	return spawn_point;
}

void CharacterLogic::SplashDamage(const char* locator, const char* particale, float radius,float damage, bool allowkick,const char* reaction,const char* deathLink)
{	
	Vector pos;

	if (!chr.FindLocator(locator,pos))
	{
		Matrix mtx;

		if (!chr.items->FindLocatorinItems(locator,mtx))
		{
			pos = Vector (0,0,0.75f);			
			pos.Rotate(chr.physics->GetAy());

			pos += chr.physics->GetPos();
		}
		else
		{
			pos = mtx.pos;
		}
	}			

	chr.arbiter->SplashDamage(DamageReceiver::ds_sword, &chr, pos, radius, damage, false, particale, false, 0.5f,allowkick,reaction,deathLink);	
}

void CharacterLogic::SetAnimSlowDown(bool animslowdown)
{
	allow_animslowdown = animslowdown;

	if (!allow_animslowdown)
	{
		chr.animation->SetPlaySpeed(1.0f);
	}
}

void CharacterLogic::EnableAutoAim(bool enable)
{
	auto_aim = enable;
	targeted_char = NULL;
}

bool CharacterLogic::IsAutoAimEnabled()
{
	return auto_aim;
}

int  CharacterLogic::GetFightAct()
{
	int act = 0;
	chr.arbiter->FindCircle(chr,5.0f);	
	
	for (int i = 0; i < chr.arbiter->find; i++)
	{
		Character* c = chr.arbiter->find[i].chr;

		if (c->logic->GetSide() != CharacterLogic::s_npc &&
			c->logic->IsEnemy(&chr) && c->GetHP()>0 &&
			!c->logic->IsActor() && !c->logic->IsPairMode())
		{
			bool is_instant;

			if(chr.arbiter->find[i].chr->logic->PairModeAllowed(false,true,&is_instant))
			{
				if (is_instant)
				{
					act = 3;
				}
				else
				{
					act = 2;
				}
				
				break;
			}

			act = 1;
		}					
	}

	return act;
}

//Работает ли минигейм в данный момент
bool CharacterLogic::IsMinigameStarted()
{
	return chr.buttonMG && (chr.buttonMG->mode != mg_disabled);
}
