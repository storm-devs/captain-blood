#ifndef AI_MUSHKETER_SHOOT
#define AI_MUSHKETER_SHOOT


#include "../aiThought.h"

class Character;

class aiMushketerShoot : public aiThought
{

	float fTimeToNextAttack;
	float fIdleTime;
	Character* pTarget;
	
	float fNearDistance;
	float fFarDistance;

	float fMinTimeToShoot;
	float fMaxTimeToShoot;

	Vector vLastTrgPos;

	float fWaitTime;

	int   numstrafes;
	bool  needStrafe;

public:

	aiMushketerShoot(aiBrain* Brain);	

	virtual void Activate(Character* pChr,
						  float fShootNearDistance, float fShootFarDistance,
						  float fMinIdle, float fMaxIdle, float waittime,
						  bool  needstrafe);

	virtual bool Process (float fDeltaTime);

	virtual void DebugDraw (const Vector& BodyPos, IRender* pRS);

	virtual void Stop ();

	virtual void Hit ();
};

#endif