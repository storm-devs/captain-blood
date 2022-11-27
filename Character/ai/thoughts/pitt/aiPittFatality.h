#ifndef AI_PITTFATALITY_KICK
#define AI_PITTFATALITY_KICK


#include "../aiThought.h"

class Character;

class aiPittFatality : public aiThought
{

	float fTimeToNextAttack;
	float fIdleTime;
	Character* pTarget;	

	bool  bHasKick;
	bool  bDoFatality;
	bool  bAllowRotate;

	float fTimeToFatality;

public:

	aiPittFatality(aiBrain* Brain);
	virtual ~aiPittFatality();

	virtual void Activate(Character* pChr);

	virtual bool Process (float fDeltaTime);

	virtual void DebugDraw (const Vector& BodyPos, IRender* pRS);

	virtual void Stop ();
};

#endif