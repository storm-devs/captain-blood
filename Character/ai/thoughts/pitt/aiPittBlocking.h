#ifndef AI_PITT_BLOCKING
#define AI_PITT_BLOCKING

#include "../aiBlocking.h"

class Character;

class aiPittBlocking : public aiThoughtBlocking
{
	float fTime;
	float fMaxWaitTime;
	

	Character* pTarget;	

	bool bInBlock;

public:

	aiPittBlocking(aiBrain* Brain);
	virtual ~aiPittBlocking();

	virtual void Activate(Character* pChr, float fWaitTime);

	virtual bool Process (float fDeltaTime);	

	virtual void Stop ();
};

#endif