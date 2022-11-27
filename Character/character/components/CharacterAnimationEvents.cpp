//============================================================================================
// Spirenkov Maxim, 2004
//============================================================================================
// Character
//============================================================================================
// CharacterAnimationEvents
//============================================================================================


#include "..\Character.h"
#include "CharacterAnimationEvents.h"
#include "CharacterItems.h"
#include "CharacterLogic.h"
#include "CharacterPhysics.h"
#include "CharacterAnimation.h"
#include "CharacterAchievements.h"
#include "ButtonMG.h"
#include "..\..\player\PlayerController.h"
#include "..\..\Auxiliary objects\ButtonMiniGame\ButtonMiniGame.h"
#include "..\..\..\common_h\bonuses.h"

CharacterAnimationEvents::CharacterAnimationEvents(Character * c) : chr(*c), updateEvents(_FL_)
{
	Assert(c);
	isEnableTimeEvent = 0;	
}

CharacterAnimationEvents::~CharacterAnimationEvents()
{
}

//Установить обработчики анимации на заданную анимацию
void CharacterAnimationEvents::SetAnimation(IAnimation * anx)
{
	AddAnimation(anx);
}

void CharacterAnimationEvents::AddHandlers(IAnimation * anx)
{
	if(!anx) return;
	
	AnimationStdEvents::AddHandlers(anx);

	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::ShootDamage, "ShootDmg");
	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::ShootTargets, "ShootMultipleDmg");
	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::MoveLogicLocator, "MoveLocator");	
	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::PlayItemParticles, "ChrItemPrt");
	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::ShowTrail, "ShowTrail");
	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::HideTrail, "HideTrail");
	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::PrepareForAttack, "PrepareAttack");
	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::BeginAttack, "BegAttack");
	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::EndAttack, "EndAttack");
	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::CharacterCommand, "ChrCommand");
	
	anx->DelEventHandler(this, (AniEvent)&AnimationStdEvents::ChangeActiveObject, "ActivateObject");
	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::ActivateObject, "ActivateObject");

	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::DropBomb, "DropBomb");
	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::AniToRagdoll, "AniToRgd");
	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::RagdollToAni, "RgdToAni");
	
	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::ArmRagdoll, "ArmRagdoll");
	
	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::FlyBodyPart, "FlyBodyPart");
	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::DropItems, "DropItems");
	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::DropItem, "DropItem");

	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::PlayItemLinkedParticles, "ChrItemLnkPrt");	
	
	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::BeginMiniGameParams, "BeginMiniGameParams");
	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::EndMiniGame, "ChooseAnimLink");


	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::OrientOnAttacker, "OrientOnAttacker");
	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::AutoAim, "AutoAim");	
	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::Kick, "Kick");
	
	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::BloodPuff, "BloodPuff");	

	//Выполнить команду миссионого объекта
	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::MissionCommand, "MOCmd");
		
	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::DeattachChar, "DeattachСhar");
	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::AttachChar, "AttachСhar");

	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::AnimSlowDown, "AnimSlowDown");

	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::StartVibration, "StartVibration");
	
	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::DropBonus, "DropBonus");

	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::BeginSloMo, "BeginSloMo");
	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::SloMoMarker, "SloMoMarker");

	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::Slowmo, "Slowmo");

	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::FootStep, "FootStep");	

	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::PlayEffect, "PlayEff");	
	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::PlayLocEffect, "PlayLocEff");		

	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::ShowWeapon, "ShowWeapon");	
	
	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::ProhibitionRotation, "ProhibitionRotation");
	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::AllowRotation, "AllowRotation");

	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::BloodStain, "BloodStain");
	
	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::BeginSplashdamage, "BeginSplashdamage");
	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::EndSplashdamage, "EndSplashdamage");

	//anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::PatchExplosion, "PatchExplosion");

	anx->SetEventHandler(this, (AniEvent)&CharacterAnimationEvents::ZoomCamera, "ZoomCamera");
	
	anx->SetEventHandler(this, (AniEvent)(&CharacterAnimationEvents::SplashDamage), "SpDamage");	
	anx->SetEventHandler(this, (AniEvent)(&CharacterAnimationEvents::SwordEffect), "SwordEffect");

	anx->SetEventHandler(this, (AniEvent)(&CharacterAnimationEvents::SwordStopEffect), "SwordStopEffect");	

	anx->SetEventHandler(this, (AniEvent)(&CharacterAnimationEvents::AddRage), "AddRage");	
	
	anx->SetEventHandler(this, (AniEvent)(&CharacterAnimationEvents::PairFailed), "PairFailed");	
	
/*	UpdateEvents & ue = updateEvents[updateEvents.Add()];
	ue.a = anx;
	ue.begAttack = anx->GetEventId("BegAttack");
	ue.endAttack = anx->GetEventId("EndAttack");
	ue.showTrail = anx->GetEventId("ShowTrail");
	ue.hideTrail = anx->GetEventId("HideTrail");
*/
}

void CharacterAnimationEvents::_DelHandlers(IAnimation * anx)
{
	if(!anx) return;
	
	AnimationStdEvents::DelHandlers(anx);

	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::ShootDamage, "ShootDmg");
	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::ShootTargets, "ShootMultipleDmg");
	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::MoveLogicLocator, "MoveLocator");	
	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::PlayItemParticles, "ChrItemPrt");
	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::ShowTrail, "ShowTrail");
	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::HideTrail, "HideTrail");
	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::PrepareForAttack, "PrepareAttack");
	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::BeginAttack, "BegAttack");
	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::EndAttack, "EndAttack");
	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::CharacterCommand, "ChrCommand");
	
	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::ActivateObject, "ActivateObject");

	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::DropBomb, "DropBomb");
	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::AniToRagdoll, "AniToRgd");
	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::RagdollToAni, "RgdToAni");
	
	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::ArmRagdoll, "ArmRagdoll");
	
	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::FlyBodyPart, "FlyBodyPart");
	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::DropItems, "DropItems");
	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::DropItem, "DropItem");

	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::PlayItemLinkedParticles, "ChrItemLnkPrt");	
	
	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::BeginMiniGameParams, "BeginMiniGameParams");
	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::EndMiniGame, "ChooseAnimLink");


	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::OrientOnAttacker, "OrientOnAttacker");
	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::AutoAim, "AutoAim");	
	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::Kick, "Kick");
	
	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::BloodPuff, "BloodPuff");	

	//Выполнить команду миссионого объекта
	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::MissionCommand, "MOCmd");
		
	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::DeattachChar, "DeattachСhar");
	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::AttachChar, "AttachСhar");

	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::AnimSlowDown, "AnimSlowDown");

	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::StartVibration, "StartVibration");
	
	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::DropBonus, "DropBonus");

	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::BeginSloMo, "BeginSloMo");
	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::SloMoMarker, "SloMoMarker");

	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::Slowmo, "Slowmo");

	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::FootStep, "FootStep");	

	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::PlayEffect, "PlayEff");	
	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::PlayLocEffect, "PlayLocEff");		

	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::ShowWeapon, "ShowWeapon");	
	
	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::ProhibitionRotation, "ProhibitionRotation");
	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::AllowRotation, "AllowRotation");

	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::BloodStain, "BloodStain");
	
	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::BeginSplashdamage, "BeginSplashdamage");
	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::EndSplashdamage, "EndSplashdamage");
	
	//anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::PatchExplosion, "PatchExplosion");

	anx->DelEventHandler(this, (AniEvent)&CharacterAnimationEvents::ZoomCamera, "ZoomCamera");
	
	anx->DelEventHandler(this, (AniEvent)(&CharacterAnimationEvents::SplashDamage), "SpDamage");	
	anx->DelEventHandler(this, (AniEvent)(&CharacterAnimationEvents::SwordEffect), "SwordEffect");

	anx->DelEventHandler(this, (AniEvent)(&CharacterAnimationEvents::SwordStopEffect), "SwordStopEffect");	

	anx->DelEventHandler(this, (AniEvent)(&CharacterAnimationEvents::AddRage), "AddRage");	
	
	anx->DelEventHandler(this, (AniEvent)(&CharacterAnimationEvents::PairFailed), "PairFailed");	
}

//Произошла смена кадра в анимации
void CharacterAnimationEvents::TimeEvent(IAnimation * a)
{
	/*
	for(long i = 0; i < ani; i++)
	{		
		UpdateEvents & ue = updateEvents[i];
		ue.a->ExecuteEvent(ue.showTrail);
		ue.a->ExecuteEvent(ue.begAttack);
		ue.a->ExecuteEvent(ue.endAttack);
		ue.a->ExecuteEvent(ue.hideTrail);
	}  */
	chr.items->AnimationFrameUpdate();
}

//Выполнить команду миссионого объекта
void _cdecl CharacterAnimationEvents::MissionCommand(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	if (numParams>=2)
	{	
		if (string::IsEqual(params[1],"AutoFocusOn") && !chr.isPlayer)
		{
			chr.Mission().LogicDebugError("Command from animation AutoFocusOn can't be execute for non player, node: %s, animation: %s", ani->CurrentNode(), ani->GetName());
			return;
		}

		if (string::IsEqual(params[1],"AutoFocusOff") && !chr.isPlayer)
		{
			chr.Mission().LogicDebugError("Command from animation AutoFocusOff can't be execute for non player, node: %s, animation: %s", ani->CurrentNode(), ani->GetName());
			return;
		}
	}

	if(numParams < 2 || !params[0])
	{
		api->Trace("Animation: can't be execute command for mission object because invalidate event params");
		chr.Mission().LogicDebugError("Command from animation can't be execute for mission object because invalidate event params, node: %s, animation: %s", ani->CurrentNode(), ani->GetName());
		return;
	}
	
	MOSafePointer mo;
	chr.FindObject(ConstString(params[0]),mo);
	
	if(!mo.Ptr())
	{
		api->Trace("Animation: can't be execute command for mission object %s because it not found", params[0]);
		chr.Mission().LogicDebugError("Command from animation can't be execute for mission object %s because it not found, node: %s, animation: %s", params[0], ani->CurrentNode(), ani->GetName());
		return;
	}
	chr.Mission().LogicDebug("Command from animation, node: %s, animation: %s", ani->CurrentNode(), ani->GetName());
	chr.Mission().LogicDebug("Execute command \"%s\" for object \"%s\"", params[1], params[0]);
	for(dword n = 0; n < numParams - 2; n++)
	{
		chr.Mission().LogicDebug("|-param %i: \"%s\"", n, params[n + 2]);
	}
	mo.Ptr()->Command(params[1], numParams - 2, params + 2);
}

//Отметить попадание
void _cdecl CharacterAnimationEvents::ShootDamage(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	if(numParams >= 1)
	{
		char * pos = null;
		float dmg = (float)strtod(params[0], &pos);
		//chr.logic->ShootHit(dmg);
		chr.Shoot(dmg);
	}
}

//Отметить попадание
void _cdecl CharacterAnimationEvents::ShootTargets(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	if(numParams >= 1)
	{
		char * pos = null;
		float dmg = (float)strtod(params[0], &pos);
		//chr.logic->ShootHit(dmg);


		bool kick = true;

		if(numParams >= 2)
		{
			if (params[1][0]=='N'||params[1][0]=='n')
			{
				kick = false;
			}
		}

		float fly_dist = 2.5f;
		if(numParams >= 3)
		{			
			fly_dist = (float)strtod(params[2], &pos);
		}

		float damage_radius = 10.0f;
		if(numParams >= 4)
		{			
			damage_radius = (float)strtod(params[3], &pos);
		}

		const char* react = "Hit";
		
		if(numParams >= 5)
		{	
			react = params[4];
		}

		bool only_once = false;

		if(numParams >= 6)
		{	
			if (params[5][0] == 'y' || params[5][0] == 'Y')
			{
				only_once = true;
			}
		}

		chr.ShootTargets(dmg, react, kick,fly_dist,damage_radius, only_once);
	}
}

//Отметить попадание
void _cdecl CharacterAnimationEvents::AttachChar(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	if(numParams < 3)
	{
		return;
	}

	MOSafePointerTypeEx<Character> mo;
	static const ConstString strTypeId("Character");
	mo.FindObject(&chr.Mission(),ConstString(params[0]),strTypeId);

	if (mo.Ptr())
	{		
		if (mo.Ptr()->IsPlayer())
		{
			chr.logic->is_attached = 2;
			chr.logic->master = mo.Ptr();

			chr.events->SetTimeEvent(false);
			mo.Ptr()->events->SetTimeEvent(false);
				
			chr.physics->Activate(false);
			chr.physics->SetUseFakeModelPosition(false);
				
			chr.animation->Goto(params[1],0.0f);
				
			mo.Ptr()->logic->slave = &chr;
			
			if (chr.controller)
				chr.controller->Reset();
			
			((PlayerController*)(mo.Ptr()->controller))->TakeWeapon(GetWeaponType(params[2]), "", null, null, ConstString::EmptyObject(),0.0f);
			
			// сбрасываем кол-во атакующих, иначе иногда при взятии бомбардира он оставался в атакующих
			mo.Ptr()->controller->SetNumAtackers(0);
		}
	}	
}

//Отметить попадание
void _cdecl CharacterAnimationEvents::DeattachChar(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	chr.DeattachChar();
}

void _cdecl CharacterAnimationEvents::AnimSlowDown(IAnimation * ani, const char * name, const char ** params, dword numParams)
{		
	if (chr.logic->GetOffender())
	{
		if (chr.logic->GetOffender() == chr.Mission().Player())
		{
			chr.logic->GetOffender()->logic->aniSpeed = 0.0f;
			chr.logic->aniSpeed = 0.0f;	
		}
	}
}

//Модифицировать фантомный локатор
void _cdecl CharacterAnimationEvents::MoveLogicLocator(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	if(numParams >= 2)
	{
		chr.items->SetLogicLocator(ConstString(params[0]), ConstString(params[1]));
	}
}

//Завести партикловую систему в локаторе эквипленного итема
void _cdecl CharacterAnimationEvents::PlayItemParticles(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	if(numParams < 3) return;
	Matrix mtx(true);
	float scale = 1.0f;
	if(numParams >= 4 && params[3][0] != 0)
	{
		char * pos = null;
		scale = (float)strtod(params[3], &pos);
	}
	chr.items->StartParticlesOnItems(ConstString(params[0]), params[1], params[2], false, chr.GetMatrix(mtx), scale, false);
}

//Завести партикловую систему в локаторе эквипленного итема
void _cdecl CharacterAnimationEvents::PlayItemLinkedParticles(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	if(numParams < 3) return;
	Matrix mtx(true);
	float scale = 1.0f;
	if(numParams >= 4 && params[3][0] != 0)
	{
		char * pos = null;
		scale = (float)strtod(params[3], &pos);
	}
	chr.items->StartParticlesOnItems(ConstString(params[0]), params[1], params[2], true, chr.GetMatrix(mtx), scale, false);
}

//Показать след от оружия
void _cdecl CharacterAnimationEvents::ShowTrail(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	if(numParams < 1) return;
	chr.items->SetTrailVisible(ConstString(params[0]), true);
	chr.items->AnimationFrameUpdate();
	isEnableTimeEvent |= teu_trail;
	UpdateTimeEventState(ani);	
}

//Скрыть след от оружия
void _cdecl CharacterAnimationEvents::HideTrail(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	if(numParams < 1) return;
	chr.items->AnimationFrameUpdate();
	chr.items->SetTrailVisible(ConstString(params[0]), false);
	isEnableTimeEvent &= ~teu_trail;
	UpdateTimeEventState(ani);
}

//Подготовить анимацию к аттаке, подняв внутреннии фпс
void _cdecl CharacterAnimationEvents::PrepareForAttack(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	isEnableTimeEvent |= teu_attack;
	UpdateTimeEventState(ani);	
}

//Начать атаку
void _cdecl CharacterAnimationEvents::BeginAttack(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	if(numParams < 1) return;

	if (chr.chrAchievements)
	{
		chr.chrAchievements->StartCountHead();
	}

	// FIX ME!!!!!
	chr.logic->SetAttackActive(false);

	chr.items->ActivateItem(ConstString(params[0]), true);
	chr.items->AnimationFrameUpdate();
	isEnableTimeEvent |= teu_attack;
	UpdateTimeEventState(ani);	
}

//Закончить атаку
void _cdecl CharacterAnimationEvents::EndAttack(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	if(numParams < 1) return;
	chr.items->AnimationFrameUpdate();

	chr.arbiter->AttackChrItems(&chr);	
	
	chr.items->ActivateItem(ConstString(params[0]), false);
	isEnableTimeEvent &= ~teu_attack;
	UpdateTimeEventState(ani);
}

//Отдать персонажу команду
void _cdecl CharacterAnimationEvents::CharacterCommand(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	if(numParams >= 1 && params[0])
	{
		chr.LogicDebug("Character command from animation");
		chr.LogicDebugLevel(true);
		chr.Command(params[0], numParams - 1, params + 1);
		chr.LogicDebugLevel(false);
	}else{
		chr.LogicDebugError("Can't execute character command from animation - invalidate params");
	}
}


//Активировать миссионный объект
void _cdecl CharacterAnimationEvents::ActivateObject(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	if(numParams >= 2 && params[0] && params[1])
	{
		MOSafePointer obj;
		chr.FindObject(ConstString(params[0]),obj);
		
		if(obj.Ptr())
		{
			bool act = true;

			if(params[1][0] == 'a' || params[1][0] == 'A')
			{
				chr.LogicDebug("Activate mission object \"%s\" from animation", params[0]);
			}
			else
			{
				chr.LogicDebug("Deactivate mission object \"%s\" from animation", params[0]);
				act = false;
			}

			obj.Ptr()->Activate(act);
		}
		else
		{
			chr.LogicDebugError("Can't change active of mission object - object \"%s\" not found", params[0]);
		}
	}
	else
	{
		chr.LogicDebugError("Can't change active of mission object - invalidate params");
	}
}

//Момент броска бомбы
void _cdecl CharacterAnimationEvents::DropBomb(IAnimation * ani, const char * name, const char ** params, dword numParams)
{	
	if(numParams >= 2)
	{
		char * pos = null;
		float timeLieBomb = (float)strtod(params[1], &pos);		

		chr.DropBomb(timeLieBomb);
	}
	else
	{		
		chr.DropBomb();
	}
}
//Перейти в рэгдол
void _cdecl CharacterAnimationEvents::AniToRagdoll(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	char * pos = null;
	float time = (float)strtod(params[0], &pos);
	chr.ActivateRagdoll(true, time);
}

//Перейти в анимацию
void _cdecl CharacterAnimationEvents::RagdollToAni(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	char * pos = null;
	float time = (float)strtod(params[0], &pos);
	chr.ActivateRagdoll(false, time);
}

void _cdecl CharacterAnimationEvents::ArmRagdoll(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	chr.ArmRagdoll();	
}
//Отлетание головы
void _cdecl CharacterAnimationEvents::FlyBodyPart(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	if (numParams<2) return;

	char ID[128];
	crt_strcpy(ID, 127, params[0]);

	char ParticaleName[128];
	crt_strcpy(ParticaleName,127, params[1]);

	if (numParams<3)
	{	
		chr.FlyBodyPart(ConstString(ID),ConstString(ParticaleName),1.0f,1.0f,20.0f,null);
	}
	else
	{
		char * pos = null;
		float pow = (float)strtod(params[2], &pos);

		if (numParams>4)
		{			
			float pow_h = (float)strtod(params[3], &pos);
			float torque = (float)strtod(params[4], &pos);

			chr.FlyBodyPart(ConstString(ID),ConstString(ParticaleName),pow,pow_h,torque,params[5]);
		}
		else
		{
			chr.FlyBodyPart(ConstString(ID),ConstString(ParticaleName),pow,1.0f,20.0f,null);
		}
	}
}

//Бросание предметов
void _cdecl CharacterAnimationEvents::DropItems(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	chr.DropItems();
}

//Бросание предмета
void _cdecl CharacterAnimationEvents::DropItem(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	if (numParams<1) return;	
	
	chr.DropItem(ConstString(params[0]));	
}

//начинаем мини игру
void _cdecl CharacterAnimationEvents::BeginMiniGameParams(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	if (numParams<1) return;

	if (chr.common_mg_params)
	{
		if (chr.common_mg_params->StartGame(&chr,params[0]))
		{
			return;			
		}
	}

	if (chr.mg_params)
	{
		chr.mg_params->StartGame(&chr,params[0]);
	}
}

void _cdecl CharacterAnimationEvents::EndMiniGame(IAnimation * ani, const char * name, const char ** params, dword numParams)
{	
	chr.EndButtonMG();	
}

//Повернуть в сторону нападающего
void _cdecl CharacterAnimationEvents::OrientOnAttacker(IAnimation * ani, const char * name, const char ** params, dword numParams)
{	
	chr.physics->Turn(chr.physics->vAttackerDir.GetAY(chr.physics->GetAy()), true, true);
}

//Довестись до ближайшего противника
void _cdecl CharacterAnimationEvents::AutoAim(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	if (numParams<1)
	{
		chr.logic->AimToTarget(1.5f,0.5f * PI);

		return;

	}

	if (numParams<2) return;

	char * pos = null;
	
	float dist = (float)strtod(params[0], &pos);	
	float sektor = (float)strtod(params[1], &pos);

	chr.logic->AimToTarget(dist, Deg2Rad(sektor));
}

//Толкнуть перса
void _cdecl CharacterAnimationEvents::Kick(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	if (numParams<1) return;

	char * pos = null;
	float pow = (float)strtod(params[0], &pos);

	if (pow == 0) pow = 1.0f;

	if (chr.logic->GetOffender())
	{
		Matrix mat1;
		chr.physics->GetMatrixWithBoneOffset(mat1);

		Matrix mat2;
		chr.logic->GetOffender()->physics->GetMatrixWithBoneOffset(mat2);

		
		chr.physics->vFlyDir = mat1.pos - mat2.pos;
		chr.physics->vFlyDir.y = 0.0f;
		chr.physics->vFlyDir.Normalize();

		chr.physics->vFlyDir.y = 0.2f;
		chr.physics->vFlyDir.Normalize();

		chr.ActivateRagdoll(true,0.01f,pow);
	}
	else
	{
		chr.physics->Turn(chr.physics->vAttackerDir.GetAY(chr.physics->GetAy()), true, true);
		chr.ActivateRagdoll(true,0.01f,pow);
	}
}

void _cdecl CharacterAnimationEvents::BloodPuff(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	if (chr.logic->GetOffender())
	{
		if (!chr.logic->GetOffender()->IsPlayer()) return;
		if (!chr.logic->IsEnemy(chr.logic->GetOffender())) return;
	}

	Matrix cam_mat = chr.Render().GetView();
	
	float dist = (cam_mat.GetCamPos() - chr.physics->GetPos()).GetLengthXZ2();

	Matrix mProj = chr.Render().GetProjection();
	float k = 1 / (mProj.m[0][0] * tanf(71.619f*PI/180.0f*0.5f));	

	if (dist*k*k>10.0f * 10.0f) return;

	MOSafePointer mo;
	static const ConstString ccId("Camera Controller");
	chr.FindObject(ccId,mo);
		
	if(!mo.Ptr())
	{
		api->Trace("Animation: can't be execute command for mission object \"Camera Controller\" because it not found");
		return;
	}
	
	if (numParams<1)
	{
		mo.Ptr()->Command("BloodPuff", 0, NULL);
	}
	else
	{
		char * pos = null;
		int num = (int)strtod(params[0], &pos);

		num = (int)(( num + Rnd((float)num) )* 0.35f);

		for (int i=0;i<num;i++)
		{
			mo.Ptr()->Command("BloodPuff", 0, NULL);
		}
	}
}

/*
//Голос персонажа
void _cdecl CharacterAnimationEvents::SndVoice(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	if(!sound || numParams < 2) return;
	IGMXEntity * loc = FindLocator(params[0]);
	if(loc)
	{
		ISound3D * snd = sound->Create3D(params[1], Vector(0.0f), _FL_, false, false);
		if(snd)
		{
			chr.bodyparts.SetVoice(snd, loc->Clone());
		}
	}else{
		api->Trace("Voice : Locator \"%s\" not found, animation %s, node %s", params[0], ani->GetName(), ani->CurrentNode());
	}
}
*/

//Начать вибрировыание :)
void _cdecl CharacterAnimationEvents::StartVibration(IAnimation * ani, const char * name, const char ** params, dword numParams)
{		
	if (numParams<1) return;

	IForce* force = chr.Controls().CreateForce(params[0]);

	if (force)
	{
		force->Play();
	}
}

//Бросить бонус
void _cdecl CharacterAnimationEvents::DropBonus(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	if (chr.IsPlayer() || numParams<1) return;
	
	Matrix mtx(true);
	chr.physics->GetModelMatrix(mtx);

	Matrix joint_mat = chr.GetBoneMatrix(chr.spine_boneIdx);		
	mtx = joint_mat * mtx;

	const char * k = strstr(params[0], "Fatality_bonus");

	if (!k)
	{
		k = strstr(params[0], "big coin");
	}

	if (k)
	{
		Character* player = chr.arbiter->GetPlayer();

		if (player)
		{
			//player->AchievementReached( <17 Racketeer> );
		}
	}
	

	BonusesManager::CreateBonus(chr.Mission(),mtx.pos,ConstString(params[0]));
	chr.logic->SetBonusWasDroped (true);	
}

inline void CharacterAnimationEvents::UpdateTimeEventState(IAnimation * ani)
{
	if (!ani) return;

	if(isEnableTimeEvent)
	{
		ani->SetTimeEventHandler(this, 0.02f);
	}else{
		ani->SetTimeEventHandler(null, 0.0f);
	}
}

void _cdecl CharacterAnimationEvents::BeginSloMo(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	//return;

	if(numParams < 1) return;

	if (chr.IsPlayer())
	{
		char * pos = null;
		int id = (int)strtod(params[0], &pos);
		
		float time = 1.0f;
		
		if(numParams > 1)
		{
			char * pos = null;
			time = (float)strtod(params[1], &pos);

			if (time<0.01f) time = 1.0f;
		}
		

		((PlayerController*)chr.controller)->BeginSloMo(id,time);
	}
}

// попытка включить slowmo или сделать отложенное включение, входные параметры:
// 0. тип (circle, sector)
// 1. название локатора
// 2. направление сектора удара
// 3. ширина сектора удара
// 4. радиус (для сектора длина удара)
// 5. дамаг
// 6. время работы slowmo
// 7. скалирование времени slowmo
// 8. вероятность с каждым врагом
// 9. рандомная задержка перед стартом slowmo
void _cdecl CharacterAnimationEvents::Slowmo(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	if (!chr.IsPlayer())
		return;

	if (numParams < 10)
	{
		api->Trace("Animation <Slowmo property> event have wrong number of parameters");
		return;
	}

	CharacterController::SlowmoParams smp;

	if (params[0][0] == 'c')
		smp.type = CharacterController::slowmo_circle;
	else if (params[0][0] == 's')
		smp.type = CharacterController::slowmo_sector;

	crt_strcpy(smp.locator, sizeof(smp.locator), params[1]);

	smp.sectorDirection = float(atof(params[2]));
	smp.sectorWidth = float(atof(params[3]));
	smp.radius = float(atof(params[4]));
	smp.damage = float(atof(params[5]));
	smp.duration = float(atof(params[6]));
	crt_strcpy(smp.timeScale, sizeof(smp.timeScale), params[7]);
	smp.probability = 0.01f * float(atof(params[8]));
	smp.delay = float(atof(params[9]));

	((PlayerController*)chr.controller)->CheckSlowmo(smp);
}

//Отметить маркер SloMO
void _cdecl CharacterAnimationEvents::SloMoMarker(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	if (!chr.IsPlayer() || numParams < 3) return;

	char * pos = null;
	
	int id = (int)strtod(params[0], &pos);
	int sub_id = (int)strtod(params[1], &pos);	

	((PlayerController*)chr.controller)->AddSloMoMarker(id,sub_id,ConstString(params[2]));
}

void _cdecl CharacterAnimationEvents::FootStep(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	if (chr.EditMode_IsOn())
		return;

	Vector pos = chr.physics->GetPos();

	if(numParams >= 1)
	{
		chr.FindLocator(params[0],pos);
	}
	
	IPhysicsScene::RaycastResult res;		

	if (chr.Physics().Raycast(pos + Vector(0.0,0.2f,0.0f), pos + Vector(0.0,-0.35f,0.0f), phys_mask(phys_character),&res))
	{
		if (chr.step_effectTable) chr.step_effectTable->MakeEffect((PhysTriangleMaterialID)res.mtl, &chr, res.position,res.normal);			
	}	
}

//Завести партикловую систему привязанную к локатору
void _cdecl CharacterAnimationEvents::PlayEffect(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	if(!particle || numParams < 1) return;
	GMXHANDLE loc = FindLocator(params[0]);

	if(loc.isValid() && chr.logic->offenderEffectTable)
	{
		Matrix mt;
		chr.physics->GetModelMatrix(mt);
		mt = geometry->GetNodeLocalTransform(loc) * mt;

		PhysTriangleMaterialID mtl = pmtlid_other3;

		if (chr.items->IfArmorMtrlPresent())
		{
			mtl = pmtlid_other2;
		}

		chr.logic->offenderEffectTable->MakeEffect(mtl, &chr, mt.pos,mt.vy);		
	}
}

//Завести партикловую систему привязанную к локатору
void _cdecl CharacterAnimationEvents::PlayLocEffect(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	if(!particle || numParams < 1) return;
	GMXHANDLE loc = FindLocator(params[0]);

	if (loc.isValid() && chr.logic->offenderEffectTable)
	{
		Matrix mt;
		chr.physics->GetModelMatrix(mt);
		mt = geometry->GetNodeLocalTransform(loc) * mt;

		PhysTriangleMaterialID mtl = pmtlid_other3;

		if (chr.items->IfArmorMtrlPresent())
		{
			mtl = pmtlid_other2;
		}		

		chr.logic->offenderEffectTable->MakeEffect(mtl, &chr, mt.pos, mt.vy, geometry, &loc, true);
	}
}

void _cdecl CharacterAnimationEvents::ShowWeapon(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	if(numParams < 2) return;

	bool show = false;
	if (params[0][0]=='Y'||params[0][0]=='y')
	{
		show = true;
	}

	chr.items->ShowWeapon(show,ConstString(params[1]));
}

void _cdecl CharacterAnimationEvents::ProhibitionRotation(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	chr.logic->SetAllowOrient(false);
}

void _cdecl CharacterAnimationEvents::AllowRotation(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	chr.logic->SetAllowOrient(true);
}

void _cdecl CharacterAnimationEvents::BloodStain(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	chr.BloodStain(-1.0f);

}

void CharacterAnimationEvents::CreateParticles(const char * name, IGMXScene * scene, GMXHANDLE loc, bool isAttach, const Matrix& mat, float scale)
{
	IParticleSystem * prt = chr.Particles().CreateParticleSystem(name);
	
	if(prt)
	{
		//Настраиваем партикловую систему		
		if (isAttach) prt->AttachTo(scene, loc, false);
		prt->AutoDelete(false);
		prt->Restart(rand());		
		prt->Teleport(mat);
		prt->SetScale(scale);

		LocalParticles & lp = particles[particles.Add()];	
		
		/*if (isAttach) 
		{
			lp.locator = loc->Clone();
		}
		else
		{*/
			lp.locator.reset();
		//}
		
		lp.prt = prt;	
	}
	else
	{
		api->Trace("Character, PlayItemParticles Particels \"%s\" not created", name);
	}
}

void _cdecl CharacterAnimationEvents::BeginSplashdamage(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	chr.logic->EnableSplahdamage(true);
}

/*void _cdecl CharacterAnimationEvents::PatchExplosion(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	if (numParams < 1) 
	{
		chr.LogicDebugError("PatchExplosion with zero parameters: ani = %s, name = %s", ani->GetName(), name);
		return;
	}

	Matrix mtx(true);
	Vector pos = chr.physics->GetModelMatrix(mtx).pos;

	// тип взрыва патча
	if (params[0][0] == 0)
	{
		chr.LogicDebugError("PatchExplosion with zero explosion type");
		return;
	}
	ConstString type; 
	type.Set(params[0]);

	// имя локатора если есть, иначе возьмем позицию персонажа
	if (numParams >= 2 && params[1][0] != 0)
		chr.FindLocator(params[1], pos);

	chr.arbiter->ExplosionPatch(pos, type);
}*/

void _cdecl CharacterAnimationEvents::EndSplashdamage(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	chr.logic->EnableSplahdamage(false);
}

void _cdecl CharacterAnimationEvents::ZoomCamera(IAnimation * ani, const char * name, const char ** params, dword numParams)
{	
	if (!chr.IsPlayer()) return;
	if(numParams < 1) return;

	bool zoom = true;
	if (params[0][0] == 'n') zoom = false;

	chr.logic->EnableAutoZoom(zoom);
}


void CharacterAnimationEvents::SetParticlesPlaySpeed(float speed)
{
	for (int i=0;i<particles;i++)
	{
		particles[i].prt->SetTimeScale(speed);

	}
}

void _cdecl CharacterAnimationEvents::SplashDamage(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	if (numParams<5)
	{
		return;
	}

	char* str = null;
	float radius = 2.25f;
	if (!string::IsEmpty(params[2]))
	{
		radius = (float)strtod(params[2], &str);
	}

	float damage = 20;
	if (!string::IsEmpty(params[3]))
	{
		damage = (float)strtod(params[3], &str);
	}


	bool allowkick = false;
	if (string::IsEqual(params[4],"yes")) allowkick = true;

	const char* reaction = "hit";

	if (numParams>=6)
	{
		reaction = params[5];
	}

	chr.logic->SplashDamage(params[0],params[1],radius,damage,allowkick,reaction);
}

void _cdecl CharacterAnimationEvents::SwordEffect(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	if(numParams < 1) return;	

	if (chr.chrEffect) chr.chrEffect->MakeEffect(&chr,ConstString(params[0]));
}

void _cdecl CharacterAnimationEvents::SwordStopEffect(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	if(numParams < 1) return;	

	if (chr.chrEffect) chr.chrEffect->StopEffect(ConstString(params[0]));
}

void _cdecl CharacterAnimationEvents::AddRage(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	if(numParams < 1) return;

	if (!chr.IsPlayer()) return;

	char* str = null;
	int count = 10;
	if (!string::IsEmpty(params[0]))
	{
		count = (int)strtod(params[0], &str);
	}

	((PlayerController*)chr.controller)->AddRage(count);
}

void _cdecl CharacterAnimationEvents::PairFailed(IAnimation * ani, const char * name, const char ** params, dword numParams)
{
	if (chr.logic->find_pair_failed)
	{
		chr.animation->ActivateLink("pair fail");
	}
}

void CharacterAnimationEvents::SetTimeEvent(bool enable)
{
	if (enable)
	{
		isEnableTimeEvent = 1;
	}
	else
	{
		isEnableTimeEvent = 0;
	}

	UpdateTimeEventState(chr.animation->GetCurAnimation());
}