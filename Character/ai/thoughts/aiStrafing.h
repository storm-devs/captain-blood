#ifndef AI_THOUGHT_STRAFING
#define AI_THOUGHT_STRAFING


#include "aiThought.h"

class Character;

class aiThoughtStrafing : public aiThought
{

	float fTime;
	float fMaxWaitTime;
	

	Character* pTarget;	

	Vector vTaraget;
	int    iNumBackSteps;

	bool   bAllowStepBack;

	float fStrafeTime;

	Vector lvrt, rvrt, bvrt;

public:

	aiThoughtStrafing(aiBrain* Brain);	

	virtual void Activate(Character* pChr, float fWaitTime);
	virtual void Activate(Vector _vTaraget, float fWaitTime);

	virtual bool Process (float fDeltaTime);

	virtual void DebugDraw (const Vector& BodyPos, IRender* pRS);

	virtual void Stop ();

	virtual bool Is(const char * thoughtName)
	{
		if (string::IsEqual(thoughtName, "Strafing")) return true;
		return false;
	}
};

#endif