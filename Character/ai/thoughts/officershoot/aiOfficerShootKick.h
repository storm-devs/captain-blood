#ifndef AI_OfficerShoot_KICK
#define AI_OfficerShoot_KICK


#include "../aiThought.h"

class Character;

class aiOfficerShootKick : public aiThought
{

	float fTimeToNextAttack;
	float fIdleTime;
	Character* pTarget;
	float fDistance;

	bool  bHasKick;

public:

	aiOfficerShootKick(aiBrain* Brain);
	virtual ~aiOfficerShootKick();

	virtual void Activate(Character* pChr, float fKickDistance);

	virtual bool Process (float fDeltaTime);

	virtual void DebugDraw (const Vector& BodyPos, IRender* pRS);

	virtual void Stop ();

	virtual void Hit ();
};

#endif