
#include "CharacterAchievements.h"
#include "CharacterAnimation.h"
#include "CharacterLogic.h"
#include "..\..\player\PlayerController.h"

#define ToSignRange(x) (x  - 0.5f) * 2.0f

CharacterAchievements::CharacterAchievements(Character * character, dword playerNum, bool isMultiplayer) //: achievements(_FL_)
{
	this->multiplayerIndex = playerNum - 1;
	this->isMultiplayer = isMultiplayer;
	this->chr = character;
	this->count_deaths_in_rage = false;
	this->node = "";
}

void CharacterAchievements::PostCreate()
{
#define INIT_VARIABLE(name) InitVariable(name, "ACHIEVEMENT_"#name)

	INIT_VARIABLE(DEMOLITION);
	INIT_VARIABLE(BURGLAR);
	INIT_VARIABLE(UNSTOPPABLE);
	INIT_VARIABLE(FISH_FOOD);
	INIT_VARIABLE(RIPPER);
	INIT_VARIABLE(SNIPER);
	//INIT_VARIABLE(MANSLAUGHTER);
	//INIT_VARIABLE(MASSACRE);
	INIT_VARIABLE(FIRST_BLOOD);
	INIT_VARIABLE(MUSKETEER);
	INIT_VARIABLE(DUELIST);
	INIT_VARIABLE(BUTCHER);
	INIT_VARIABLE(SHARPSHOOTER);
	INIT_VARIABLE(KNIGHT);
	INIT_VARIABLE(BERSERKER);
	INIT_VARIABLE(HEADCUTTER);
	INIT_VARIABLE(CATAPULT);

	INIT_VARIABLE(RACKETEER);
	INIT_VARIABLE(WEAPON_MASTER);
	INIT_VARIABLE(RAGE);
	INIT_VARIABLE(MURDERER);

	/*INIT_VARIABLE(SAILOR);
	INIT_VARIABLE(BOATSWAIN);
	INIT_VARIABLE(CAPTAIN);

	INIT_VARIABLE(CAPER);
	INIT_VARIABLE(JOLLY_ROGER);
	INIT_VARIABLE(PIRATE_KING);

	INIT_VARIABLE(AVENGER);
	INIT_VARIABLE(DEFENDER_OF_THE_WEAK);
	INIT_VARIABLE(MASTER_OF_REVENGE);

	INIT_VARIABLE(PETER_THE_CONQUEROR);
	INIT_VARIABLE(PETER_THE_VICTORIOUS);
	INIT_VARIABLE(CAPTAIN_BLOOD);*/

	INIT_VARIABLE(TRICKSTER);
}

void CharacterAchievements::InitVariable(MOSafePointerType<IAchievement> & var, const char * name)
{
	if (!chr->Mission().FindObject(ConstString(name), var.GetSPObject()))
	{
		api->Error("ERROR: Achievements: Can't find achievement object (character): %s", name);
	}
}

CharacterAchievements::~CharacterAchievements()
{
}

//void CharacterAchievements::AchievementReached(dword id, int count)
//{
	//if (!IsMultiplayer())
	//	pLiveService->Achievement_Increment(id, count);
//}

//void CharacterAchievements::AchievementSetZero(dword id)
//{
	//if (!IsMultiplayer())
	//	pLiveService->Achievement_SetZero(id);
//}

// 

void CharacterAchievements::AchievementReached(MOSafePointerType<IAchievement> & var, int count)
{
	if (var.Validate())
		var.Ptr()->Increment(float(count));
}

void CharacterAchievements::AchievementSetZero(MOSafePointerType<IAchievement> & var)
{
	if (var.Validate())
		var.Ptr()->SetZero();
}

void CharacterAchievements::AchievementReachedMP(MOSafePointerType<IAchievement> & var, int count)
{
	//if (IsMultiplayer())
	//	pLiveService->Achievement_IncrementMP(multiplayerIndex, id, count);
}

void CharacterAchievements::AchievementSetZeroMP(MOSafePointerType<IAchievement> & var)
{
	//if (IsMultiplayer())
	//	pLiveService->Achievement_SetZeroMP(multiplayerIndex, id);
}

void CharacterAchievements::AchievementReachedByID(const char * id, int count)
{
	/*static const ConstString cs_sailor = ConstString("20 Sailor");
	static const ConstString cs_boatswain = ConstString("21 Boatswain");
	static const ConstString cs_captain = ConstString("22 Captain");
		  
	static const ConstString cs_caper = ConstString("23 Caper");
	static const ConstString cs_jolly_roger = ConstString("24 Jolly Roger");
	static const ConstString cs_pirate_king = ConstString("25 Pirate King");
		  
	static const ConstString cs_avenger = ConstString("26 Avenger");
	static const ConstString cs_defender_of_the_weak = ConstString("27 Defender of the Weak");
	static const ConstString cs_master_of_revenge = ConstString("28 Master of Revenge");

	static const ConstString cs_peter_the_conqueror = ConstString("29 Peter the Conqueror");
	static const ConstString cs_peter_the_victorious = ConstString("30 Peter the Victorious");
	static const ConstString cs_captain_blood = ConstString("31 Captain Blood");*/

	static const ConstString cs_trickster = ConstString("33 Trickster");

	//static const ConstString cs_racketeer = ConstString("Racketeer");

	ConstString achiev(id);

	// выполняется только по разу после убийства боссов обычно, или выигрыша минигеймов, потому 
	// можно особо не оптимизить
#define ACH_REACH(x, y, z) if (achiev == x) { AchievementReached(y, z); return; }
	/*ACH_REACH(cs_sailor, SAILOR, 1);
	ACH_REACH(cs_boatswain, BOATSWAIN, 1);
	ACH_REACH(cs_captain, CAPTAIN, 1);
		  
	ACH_REACH(cs_caper, CAPER, 1);
	ACH_REACH(cs_jolly_roger, JOLLY_ROGER, 1);
	ACH_REACH(cs_pirate_king, PIRATE_KING, 1);
		  
	ACH_REACH(cs_avenger, AVENGER, 1);
	ACH_REACH(cs_defender_of_the_weak, DEFENDER_OF_THE_WEAK, 1);
	ACH_REACH(cs_master_of_revenge, MASTER_OF_REVENGE, 1);

	ACH_REACH(cs_peter_the_conqueror, PETER_THE_CONQUEROR, 1);
	ACH_REACH(cs_peter_the_victorious, PETER_THE_VICTORIOUS, 1);
	ACH_REACH(cs_captain_blood, CAPTAIN_BLOOD, 1);*/

	ACH_REACH(cs_trickster, TRICKSTER, 1);

	//ACH_REACH(cs_racketeer, RACKETEER, 1);
}

void CharacterAchievements::AnalyseDeath(Character * victim)
{
	if (IsMultiplayer())
	{
		//AchievementReachedMP(MANSLAUGHTER, 1);
		//AchievementReachedMP(MASSACRE, 1);
		return;
	}

	//это вроде не нужно, npc и так не приходят сюда
	//if (victim && victim->logic && !victim->logic->IsRealEnemy(chr, false))
	//	return;

	AchievementReached(FIRST_BLOOD, 1);
	
	if (chr->IsPlayer())
	{
		if (((PlayerController*)(chr->controller))->cur_weapon_style && !chr->logic->IsPairMode())
		{
			TWeaponType type = ((PlayerController*)(chr->controller))->cur_weapon_style->weapon_type;

			if (type == wp_rifle)
				AchievementReached(MUSKETEER, 1);

			if (type == wp_rapire)
				AchievementReached(DUELIST, 1);

			if (type == wp_sword1 || type == wp_sword2 || type == wp_sword3)
				AchievementReached(BUTCHER, 1);

			if (type == wp_gun)
				AchievementReached(SHARPSHOOTER, 1);

			if (type == wp_twohanded2 || type == wp_twohanded3)
				AchievementReached(KNIGHT, 1);
		}
	}

	if (count_deaths_in_rage)
		AchievementReached(BERSERKER, 1);
}

void CharacterAchievements::StartCountDeathInRage(bool enable)
{
	AchievementSetZero(BERSERKER);
	count_deaths_in_rage = enable;
}

void CharacterAchievements::StartCountHead()
{
	if (!string::IsEqual(chr->animation->CurrentNode(),node))
	{
		AchievementSetZero(HEADCUTTER);
		node = chr->animation->CurrentNode();
	}
}
	
void CharacterAchievements::CountHead()
{
	if (!string::IsEqual(chr->animation->CurrentNode(), node))
	{
		node = "";
		AchievementSetZero(HEADCUTTER);
	}
	else
	{
		AchievementReached(HEADCUTTER, 1);
	}
}