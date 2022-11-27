#ifndef AI_KnightRage
#define AI_KnightRage


#include "../aiThought.h"

class Character;

class aiKnightRage : public aiThought
{	
	Character* pTarget;	
	float  fRotateTime;

public:

	aiKnightRage(aiBrain* Brain);	

	virtual void Activate(Character* pChr);

	virtual bool Process (float fDeltaTime);	

	virtual void Stop ();	
};

#endif