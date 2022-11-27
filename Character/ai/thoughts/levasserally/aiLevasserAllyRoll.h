#ifndef AI_LevasserAlly_ROLL
#define AI_LevasserAlly_ROLL


#include "../aiThought.h"

class Character;

class aiLevasserAllyRoll : public aiThought
{

	float fTime;
	float fMaxWaitTime;
	

	Character* pTarget;	

	Vector vTaraget;
	int    iNumBackSteps;

	bool   bAllowStepBack;

public:

	aiLevasserAllyRoll(aiBrain* Brain);		

	virtual void Activate(Character* pChr, float fWaitTime);

	virtual bool Process (float fDeltaTime);

	virtual void Stop ();
};

#endif