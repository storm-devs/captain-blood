#include "..\..\..\Common_h\AnimationStdEvents.h"
#include "minigun.h"

#ifndef MINI_GAME_ANIM_HANDLER_FOR_BULLET
#define MINI_GAME_ANIM_HANDLER_FOR_BULLET

class Shooter;

class BulletAnimEventsHandler : public AnimationStdEvents
{
protected:

	Shooter* pMiniGame;

public:
	BulletAnimEventsHandler(Shooter* obj, IMission * mis);
	~BulletAnimEventsHandler();

	virtual void SetAnimation(IAnimation * anx);
};

class AniLoaderEvents : public AnimationStdEvents
{
public:

	virtual void AddHandlers(IAnimation *anx)
	{
		if( !anx )
			return;

		AnimationStdEvents::AddHandlers(anx);

		anx->SetEventHandler(this,(AniEvent)&AniLoaderEvents::MOCommand,"mocmd");
	}

public:

	void _cdecl MOCommand(IAnimation *ani, const char *name, const char **params, dword numParams)
	{
		if( numParams < 3 )
			return;

		if( string::IsEqual( params[0], "Camera Controller" ) )
		{
			MOSafePointer mo;
			if( mission->FindObject( ConstString(params[0]), mo ) )
			{
				mo.Ptr()->Command( params[1], numParams-2, &params[2] );
			}
		}
	}

};

class AniMinigunEvents : public AnimationStdEvents
{
	Minigun* m_pMinigun;
public:
	AniMinigunEvents(Minigun* pMinigun) : AnimationStdEvents() {m_pMinigun = pMinigun;}

	virtual void AddHandlers(IAnimation *anx);

public:
	void _cdecl MOCommand(IAnimation *ani, const char *name, const char **params, dword numParams);

	void _cdecl Shoot(IAnimation *ani, const char *name, const char **params, dword numParams);
};

#endif
