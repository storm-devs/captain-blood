#ifndef AI_KnightRage
#define AI_KnightRage


#include "../aiThought.h"

class Character;

class aiKomendantShoot : public aiThought
{	
	Character* pTarget;		

public:

	aiKomendantShoot(aiBrain* Brain);

	virtual void Activate(Character* pChr);	

	virtual bool Process (float fDeltaTime);	

	virtual void Stop ();	
};

#endif