#ifndef AI_ExecutorKick
#define AI_ExecutorKick


#include "../aiThought.h"

class Character;

class aiExecutorKick : public aiThought
{

	float fTimeToNextAttack;
	float fIdleTime;
	Character* pTarget;
	float fDistance;

	bool  bHasKick;	

public:

	aiExecutorKick(aiBrain* Brain);
	virtual ~aiExecutorKick();

	virtual void Activate(Character* pChr, float fKickDistance);

	virtual bool Process (float fDeltaTime);

	virtual void Stop ();

	virtual void Hit ();

	virtual bool Is(const char * thoughtName)
	{
		if (string::IsEqual(thoughtName, "Kick")) return true;
		return false;
	}
};

#endif