#ifndef AI_THOUGHT_INTERFACE_CLASS
#define AI_THOUGHT_INTERFACE_CLASS



#include "..\..\..\Common_h\Mission.h"
class aiBrain;

class aiThought
{	
public:	
	aiBrain* pBrain;	

public:

	aiThought(aiBrain* Brain)
	{
		pBrain = Brain;		
	}

	virtual ~aiThought() {};

	//Работа мысли
	//Если мысль больше не нужна надо вернуть false и она сдохнет...
	virtual bool Process (float fDeltaTime) = 0;

	//Получить мозг в котором мысль родилась
	aiBrain* GetBrain ()
	{
		return pBrain;
	}

	virtual void DebugDraw (const Vector& BodyPos, IRender* pRS) {};

	virtual void Stop () = 0;

	virtual void Activate();

	virtual void Hit() {};

	virtual bool AllowBlocking () { return true; };
	
	virtual bool Is(const char * thoughtName)
	{
		if (string::IsEqual(thoughtName, "Thought")) return true;
		return false;
	}
};

#endif