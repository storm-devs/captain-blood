#ifndef AI_KomendantKick
#define AI_KomendantKick


#include "../aiThought.h"

class Character;

class aiKomendantKick : public aiThought
{

	float fTimeToNextAttack;
	float fIdleTime;
	Character* pTarget;
	float fDistance;

	bool  bHasKick;	

public:

	aiKomendantKick(aiBrain* Brain);	

	virtual void Activate(Character* pChr, float fKickDistance);

	virtual bool Process (float fDeltaTime);

	virtual void Stop ();

	virtual void Hit ();
};

#endif