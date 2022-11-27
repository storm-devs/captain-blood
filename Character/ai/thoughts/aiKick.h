#ifndef AI_THOUGHT_KICK
#define AI_THOUGHT_KICK


#include "aiThought.h"
#include "../Util/aiActionStack.h"

class Character;

class aiThoughtKick : public aiThought
{	
	float fTimeToNextAttack;
	float fIdleTime;
	Character* pTarget;	
	bool  bHasKick;	
	aiActionStack actionStack;

	char far_kick[32];

public:

	aiThoughtKick(aiBrain* Brain);

	virtual void Activate(Character* pChr);

	virtual bool Process (float fDeltaTime);

	virtual void Stop ();	

	void AddAction( const char* action, float max_dist = 2.5f, float min_dist = 0.0f);
	void PrepareActions();

	void ResetStack();

	void SetFarAttack(const char* attack);
	
	void SetCurAction(const char* act);

	float GetAttackDistance();

	void CheckDist(float dst);

	aiActionStack* GetActionStack() { return &actionStack; };
	
	virtual bool Is(const char * thoughtName)
	{
		if (string::IsEqual (thoughtName, "Kick")) return true;
		return false;
	}
};

#endif