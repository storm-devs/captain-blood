#ifndef AI_EasterlingRage
#define AI_EasterlingRage


#include "..//aiThought.h"

class Character;

class aiEasterlingRage : public aiThought
{	
	Character* pTarget;

	bool  bHasKick;
	float bMaxRageTime;

	float fRageTime;
	float fRageRotTime;

	bool  bStopped;

public:

	aiEasterlingRage(aiBrain* Brain);
	virtual ~aiEasterlingRage();

	virtual void Activate(Character* pChr,float _fRageTime, float rotTime);

	virtual bool Process (float fDeltaTime);

	virtual void DebugDraw (const Vector& BodyPos, IRender* pRS);

	virtual void Stop ();

	virtual bool AllowBlocking () { return false; };
};

#endif