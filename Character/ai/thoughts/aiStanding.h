#ifndef AI_STANDING
#define AI_STANDING


#include "aiThought.h"
#include "../Util/aiActionStack.h"

class Character;

class aiThoughtStanding : public aiThought
{
	float fTime;
	float fMaxWaitTime;
	
	bool bInBlock;

	Character* target;

	aiActionStack actionStack;

public:

	aiThoughtStanding(aiBrain* Brain);

	virtual void Activate( float fWaitTime,Character* _target);

	virtual bool Process (float fDeltaTime);	

	virtual void Stop();	

	void AddAction( const char* action);
	void PrepareActions();

	virtual bool Is(const char * thoughtName)
	{
		if (string::IsEqual(thoughtName, "Standing")) return true;
		return false;
	}
};

#endif