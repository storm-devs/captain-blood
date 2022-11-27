#ifndef AI_ChainmanStanding
#define AI_ChainmanStanding


#include "../aiStanding.h"

class Character;

class aiChainmanStanding : public aiThoughtStanding
{

	float fTime;
	float fMaxWaitTime;

	Character* pTarget;

public:

	aiChainmanStanding(aiBrain* Brain);
	virtual ~aiChainmanStanding();

	virtual void Activate( float fWaitTime,Character* _target);

	virtual bool Process (float fDeltaTime);

	virtual void DebugDraw (const Vector& BodyPos, IRender* pRS);

	virtual void Stop () {};
};

#endif