#ifndef _Achievments_h_
#define _Achievments_h_


#include "..\..\..\common_h\Mission.h"
#include "..\..\..\common_h\Achievements.h"
#include "..\Character.h"
#include "..\..\Utils\StorageWork.h"
#include "..\..\Utils\SafePointerEx.h"
#include "..\..\..\common_h\ILiveService.h"

#define ACHIEVEMENT_REACHED(chr, name) { if ((chr) && (chr)->chrAchievements) (chr)->chrAchievements->AchievementReached((chr)->chrAchievements->name); }
#define ACHIEVEMENT_SETZERO(chr, name) { if ((chr) && (chr)->chrAchievements) (chr)->chrAchievements->AchievementSetZero((chr)->chrAchievements->name); }

class CharacterAchievements
{	
public:
	MOSafePointerType<IAchievement> DEMOLITION;
	MOSafePointerType<IAchievement> BURGLAR;
	MOSafePointerType<IAchievement> UNSTOPPABLE;
	MOSafePointerType<IAchievement> FISH_FOOD;
	MOSafePointerType<IAchievement> RIPPER;
	MOSafePointerType<IAchievement> SNIPER;
	//MOSafePointerType<IAchievement> MANSLAUGHTER;
	//MOSafePointerType<IAchievement> MASSACRE;
	MOSafePointerType<IAchievement> FIRST_BLOOD;
	MOSafePointerType<IAchievement> MUSKETEER;
	MOSafePointerType<IAchievement> DUELIST;
	MOSafePointerType<IAchievement> BUTCHER;
	MOSafePointerType<IAchievement> SHARPSHOOTER;
	MOSafePointerType<IAchievement> KNIGHT;
	MOSafePointerType<IAchievement> BERSERKER;
	MOSafePointerType<IAchievement> HEADCUTTER;
	MOSafePointerType<IAchievement> CATAPULT;

	MOSafePointerType<IAchievement> RACKETEER;
	MOSafePointerType<IAchievement> WEAPON_MASTER;
	MOSafePointerType<IAchievement> RAGE;
	MOSafePointerType<IAchievement> MURDERER;

	// Bosses
	MOSafePointerType<IAchievement> SAILOR;
	MOSafePointerType<IAchievement> BOATSWAIN;
	MOSafePointerType<IAchievement> CAPTAIN;

	MOSafePointerType<IAchievement> CAPER;
	MOSafePointerType<IAchievement> JOLLY_ROGER;
	MOSafePointerType<IAchievement> PIRATE_KING;

	MOSafePointerType<IAchievement> AVENGER;
	MOSafePointerType<IAchievement> DEFENDER_OF_THE_WEAK;
	MOSafePointerType<IAchievement> MASTER_OF_REVENGE;

	MOSafePointerType<IAchievement> PETER_THE_CONQUEROR;
	MOSafePointerType<IAchievement> PETER_THE_VICTORIOUS;
	MOSafePointerType<IAchievement> CAPTAIN_BLOOD;

	MOSafePointerType<IAchievement> TRICKSTER;

public:
	CharacterAchievements(Character * character, dword playerNum, bool isMultiplayer);
	~CharacterAchievements();

	void PostCreate();

	void AchievementReached(MOSafePointerType<IAchievement> & var, int count = 1);
	void AchievementSetZero(MOSafePointerType<IAchievement> & var);

	//void AchievementReached(dword id, int count = 1);
	//void AchievementSetZero(dword id);
	void AchievementReachedByID(const char * id, int count = 1);
	//void AchievementReached2(const char* id, int count);

	void AchievementReachedMP(MOSafePointerType<IAchievement> & var, int count);
	void AchievementSetZeroMP(MOSafePointerType<IAchievement> & var);

	void AnalyseDeath(Character* victim);
	void StartCountDeathInRage(bool enable);
	
	void StartCountHead();
	void CountHead();

private:
	const char* node;

	bool count_deaths_in_rage;
	bool isMultiplayer;
	dword multiplayerIndex;
	Character * chr;	

	void InitVariable(MOSafePointerType<IAchievement> & var, const char * name);

	__forceinline bool IsMultiplayer() { return isMultiplayer; }
};

#endif

