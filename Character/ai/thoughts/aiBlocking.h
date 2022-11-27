#ifndef AI_BLOCKING
#define AI_BLOCKING


#include "aiThought.h"

class Character;

class aiThoughtBlocking : public aiThought
{
	float fTime;
	float fMaxWaitTime;

	int   iNumHits;

	Character* pTarget;	

	bool bInBlock;

	bool stoped;

	char runaway_link[32];

public:

	aiThoughtBlocking(aiBrain* Brain);

	virtual void Activate(Character* pChr, float fWaitTime);

	virtual bool Process (float fDeltaTime);		

	virtual void Stop ();

	virtual void Hit ();

	virtual bool Is(const char * thoughtName)
	{
		if (string::IsEqual(thoughtName, "Blocking")) return true;
		return false;
	}

	void SetRunawayLink(const char* link);
};

#endif