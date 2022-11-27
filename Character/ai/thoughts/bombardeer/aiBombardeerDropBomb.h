#ifndef AI_BombardeerDropBomb
#define AI_BombardeerDropBomb


#include "../aiThought.h"

class Character;

class aiBombardeerDropBomb : public aiThought
{

	float fTimeToNextAttack;
	float fIdleTime;
	Character* pTarget;
	
	float fNearDistance;
	float fFarDistance;

	float fMinTimeToShoot;
	float fMaxTimeToShoot;

	Vector vLastTrgPos;

	bool  one_drop;
	bool  bomb_was_droped;

public:

	aiBombardeerDropBomb(aiBrain* Brain);

	virtual ~aiBombardeerDropBomb();

	virtual void Activate(Character* pChr,
						  float fShootNearDistance, float fShootFarDistance,
						  float fMinIdle, float fMaxIdle, bool only_once);

	virtual bool Process (float fDeltaTime);	

	virtual void Stop ();

	virtual void Hit ();
};

#endif