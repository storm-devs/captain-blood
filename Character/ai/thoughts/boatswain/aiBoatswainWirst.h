#ifndef AI_BOATSWAIN_WIRST
#define AI_BOATSWAIN_WIRST


#include "..//aiThought.h"

class Character;

class aiBoatswainWirst : public aiThought
{	
	Character* pTarget;

	bool  bHasKick;

	float fWirstTime;

	bool  bStopped;

public:

	aiBoatswainWirst(aiBrain* Brain);
	virtual ~aiBoatswainWirst();

	virtual void Activate(Character* pChr,float _fWirstTime);

	virtual bool Process (float fDeltaTime);

	virtual void DebugDraw (const Vector& BodyPos, IRender* pRS);

	virtual void Stop ();

	virtual bool AllowBlocking () { return false; };
};

#endif