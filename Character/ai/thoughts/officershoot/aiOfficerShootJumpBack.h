#ifndef AI_OfficerShoot_JumpBack
#define AI_OfficerShoot_JumpBack


#include "..\aiThought.h"

class Character;

class aiOfficerShootJumpBack : public aiThought
{	
	bool bStartJumpBack;

	Character* pTarget;

	float fTimetoAim;

	bool  bNeedToStop;
	bool  bBarrage;

public:

	aiOfficerShootJumpBack(aiBrain* Brain);
	virtual ~aiOfficerShootJumpBack();

	virtual void Activate(Character* pChr);

	virtual bool Process (float fDeltaTime);	

	virtual void Stop();	

	virtual void Hit();	
};

#endif