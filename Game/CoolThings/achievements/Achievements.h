#pragma once

#include "..\..\..\common_h\Mission.h"
#include "..\..\..\common_h\Achievements.h"
#include "..\..\..\XENGINE\UnderJollyRoger.spa.h"

#define ACHIEVEMENT(obsolete, name, value, multiplayer)			\
	class Achievements_##name : public Achievement								\
	{															\
	public:														\
		Achievements_##name() {}												\
		virtual ~Achievements_##name() {}										\
	};															\
	MOP_BEGINLISTCG(Achievements_##name, "ACHIEVEMENT_"#name, '1.00', 900, Achievement::comment, "Achievements")	\
		MOP_STRING("Name |" #name "|", #name)					\
		MOP_LONGEX("Index |" #name "|", ACHIEVEMENT_##name, ACHIEVEMENT_##name, ACHIEVEMENT_##name)		\
		MOP_LONG("Value |" #name "|", value)					\
		MOP_BOOL("Multiplayer |" #name "|", multiplayer)		\
		MOP_LONGC("Multiplayer index |" #name "|", 0, "Player index is multiplayer game")		\
		MOP_BOOL("Active |" #name "|", true)					\
	MOP_ENDLIST(Achievements_##name)

class Achievement : public IAchievement
{
public:
	Achievement();
	virtual ~Achievement();

	virtual bool Create(MOPReader & reader);
	virtual bool EditMode_Update(MOPReader & reader);

	virtual void Activate(bool active);

	void _cdecl Execute(float deltaTime, long level);

	void Command(const char * id, dword numParams, const char * * params);

	virtual void Increment(float count = 1);
	virtual void Set(float count);
	virtual void SetZero();

public:
	static const char * comment;
private:
	ConstString name;
	int liveID;
	int maxCount;
	int mpIndex;
	bool isMultiplayer;
	bool isEarned;
	ICoreStorageFloat * storageValue;
	ILiveService * liveService;

	void CheckAchievement();
	void EarnAchievement();
};
