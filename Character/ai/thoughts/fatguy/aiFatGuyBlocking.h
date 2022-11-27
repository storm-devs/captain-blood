#ifndef AI_FATGUY_BLOCKING
#define AI_FATGUY_BLOCKING


#include "../aiBlocking.h"

class Character;

class aiFatGuyBlocking : public aiThoughtBlocking
{
	float fTime;
	float fMaxWaitTime;
	
	int   iNumHits;

	Character* pTarget;	

	bool bInBlock;

public:

	aiFatGuyBlocking(aiBrain* Brain);

	virtual void Activate(Character* pChr, float fWaitTime);

	virtual bool Process (float fDeltaTime);	

	virtual void Stop ();

	virtual void Hit ();

	virtual bool Is(const char * thoughtName)
	{
		if (string::IsEqual(thoughtName, "Blocking")) return true;
		return false;
	}
};

#endif