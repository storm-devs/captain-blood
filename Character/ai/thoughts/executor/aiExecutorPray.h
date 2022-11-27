#ifndef AI_ExecutorPray
#define AI_ExecutorPray


#include "../aiThought.h"

class Character;

class aiExecutorPray : public aiThought
{
	float fIdleTime;
	bool  bAllowFatality;	
	bool  bHelpersAlive;

	float fHealingRate;

	MissionObject* pHelpers[5];		

	float fHPLimit;
	bool  bRegeneration;

public:

	aiExecutorPray(aiBrain* Brain);	

	virtual void Activate(Character* pChr,bool allowFatality,float HealingRate,float HPLimit, int iNumHelpers,const ConstString * first_helper);

	virtual bool Process (float fDeltaTime);

	virtual void Stop ();

	virtual bool Is(const char * thoughtName)
	{
		if (string::IsEqual(thoughtName, "Pray")) return true;
		return false;
	}
};

#endif