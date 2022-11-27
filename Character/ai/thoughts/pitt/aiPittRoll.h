#ifndef AI_THOUGHT_ROLL
#define AI_THOUGHT_ROLL


#include "../aiThought.h"

class Character;

class aiPittRoll : public aiThought
{

	float fTime;
	float fMaxWaitTime;
	

	Character* pTarget;	

	Vector vTaraget;
	int    iNumBackSteps;

	bool   bAllowStepBack;

public:

	aiPittRoll(aiBrain* Brain);	
	virtual ~aiPittRoll();

	virtual void Activate(Character* pChr, float fWaitTime);

	virtual bool Process (float fDeltaTime);

	virtual void Stop ();
};

#endif