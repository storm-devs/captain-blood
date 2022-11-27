#include "..\..\..\Common_h\AnimationStdEvents.h"

#ifndef MINI_GAME_ANIM_LISTENER
#define MINI_GAME_ANIM_LISTENER

class Shooter;

class ShooterAnimEventsHandler : public AnimationStdEvents
{
protected:

	Shooter* pMiniGame;

public:
	ShooterAnimEventsHandler(Shooter* obj);
	~ShooterAnimEventsHandler();

	virtual void SetAnimation(IAnimation * anx);
	virtual void AddHandlers(IAnimation *anx);

	void _cdecl MOCommand(IAnimation *ani, const char *name, const char **params, dword numParams);
};


#endif
