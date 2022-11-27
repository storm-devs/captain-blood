#ifndef AI_ACTION_STACK
#define AI_ACTION_STACK

#include "..\..\auxiliary objects\arbiter\CharactersTime.h"

struct AttackParam
{
	float min_hp;
	float max_hp;
	const char* link;

	float minDistance, maxDistance;
	float minCooldown, maxCooldown;
};

class aiActionStack
{	
private:
	const CharactersTime & time;
	int cur_action;

	struct TAction
	{
		char link[24];
		float minDistance, maxDistance;
		float minCooldown, maxCooldown;
		dword cooldown;
		int	randIndex;
	};

	array<TAction> actions;
	//array<int> action_ref;

public:	

	aiActionStack(const CharactersTime & _time);
		
	void AddAction(const char * action, float max_dist, float min_dist, float minCooldown = 0.0f, float maxCooldown = 0.0f);
	void AddAction(const AttackParam * action);
	void Prepare();

	const char* GetCurAction();
	void MoveToNextAction(bool need_shufle = true);

	float GetCurActionDistance();

	void SecActionAsCurrent(const char* action_link);

	void CheckDist(float dst);

	void Reset();

private:
	TAction & GetRandActionIndexed(int index);
};

#endif