#ifndef AI_LevasserAlly_KICK
#define AI_LevasserAlly_KICK


#include "../aiThought.h"

class Character;

class aiLevasserAllyKick : public aiThought
{

	float fTimeToNextAttack;
	float fIdleTime;
	Character* pTarget;
	float fDistance;

	bool  bHasKick;
	bool  bAllowRotate;

public:

	aiLevasserAllyKick(aiBrain* Brain);	

	virtual void Activate(Character* pChr, float fKickDistance);

	virtual bool Process (float fDeltaTime);

	virtual void DebugDraw (const Vector& BodyPos, IRender* pRS);

	virtual void Stop ();

	virtual void Hit ();
};

#endif