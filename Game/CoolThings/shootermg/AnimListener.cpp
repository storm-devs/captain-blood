#include "animlistener.h"
#include "shooter.h"

ShooterAnimEventsHandler::ShooterAnimEventsHandler(Shooter* obj)
{
	pMiniGame = obj;
}

ShooterAnimEventsHandler::~ShooterAnimEventsHandler()
{
}

void ShooterAnimEventsHandler::SetAnimation(IAnimation * anx)
{
	if(!anx) return;	
	AnimationStdEvents::AddAnimation(anx);	
}

void ShooterAnimEventsHandler::AddHandlers(IAnimation *anx)
{
	if( !anx )
		return;

	AnimationStdEvents::AddHandlers(anx);

	anx->SetEventHandler(this,(AniEvent)&ShooterAnimEventsHandler::MOCommand,"mocmd");
}

void _cdecl ShooterAnimEventsHandler::MOCommand(IAnimation *ani, const char *name, const char **params, dword numParams)
{
	if( numParams < 3 )
		return;

	static const ConstString objectId("Camera Controller");
	if( string::IsEqual( params[0], objectId.c_str() ) )
	{
		MOSafePointer mo;
		if( mission->FindObject( objectId, mo ) )
		{
			mo.Ptr()->Command( params[1], numParams-2, &params[2] );
		}
	}
}
