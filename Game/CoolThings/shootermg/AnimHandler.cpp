#include "animhandler.h"
#include "shooter.h"

BulletAnimEventsHandler::BulletAnimEventsHandler(Shooter* obj, IMission * mis)
{
	pMiniGame = obj;
	Init(&mis->Sound(), &mis->Particles(), mis);
}

BulletAnimEventsHandler::~BulletAnimEventsHandler()
{
}

void BulletAnimEventsHandler::SetAnimation(IAnimation * anx)
{
	if(!anx) return;
	AnimationStdEvents::AddAnimation(anx);
}



void AniMinigunEvents::AddHandlers(IAnimation *anx)
{
	if( !anx )
		return;

	AnimationStdEvents::AddHandlers(anx);

	anx->SetEventHandler(this,(AniEvent)&AniMinigunEvents::MOCommand,"mocmd");
	anx->SetEventHandler(this, (AniEvent)&AniMinigunEvents::Shoot, "ShootDmg");
}

void _cdecl AniMinigunEvents::MOCommand(IAnimation *ani, const char *name, const char **params, dword numParams)
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

void _cdecl AniMinigunEvents::Shoot(IAnimation *ani, const char *name, const char **params, dword numParams)
{
	m_pMinigun->SetShootMoment();
}



