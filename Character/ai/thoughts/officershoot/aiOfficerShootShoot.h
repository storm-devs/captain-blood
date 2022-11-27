#ifndef AI_OfficerShoot_SHOOT
#define AI_OfficerShoot_SHOOT


#include "../aiThought.h"

class Character;

class aiOfficerShootShoot : public aiThought
{	
	float fIdleTime;
	Character* pTarget;
	
	float fNearDistance;
	float fFarDistance;

	float fMinTimeToShoot;
	float fMaxTimeToShoot;	

	bool   stoped;
	bool   started;

public:

	aiOfficerShootShoot(aiBrain* Brain);
	virtual ~aiOfficerShootShoot();

	virtual void Activate(Character* pChr,
						  float fShootNearDistance, float fShootFarDistance,
				  		  float fMinIdle, float fMaxIdle);

	virtual bool Process (float fDeltaTime);

	virtual void Stop ();

	virtual void Hit();	
};

#endif