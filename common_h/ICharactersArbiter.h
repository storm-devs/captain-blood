#ifndef _CharactersArbiter_h_interface_
#define _CharactersArbiter_h_interface_


#include "mission.h"


class ICharactersArbiter : public MissionObject
{

public:

	ICharactersArbiter() {};
	virtual ~ICharactersArbiter() {};

	//Взорвать бомбу
	virtual void Boom(MissionObject * from, dword source, const Vector & pos, float radius, float damage, float boom_power, MissionObject * boomCreator = null) = null;

	//Взорвать бомбу
	virtual void BlowSelf(MissionObject * from, const Vector & pos, float radius, float damage, float boom_power) = null;

	//Нанести SplashDamage
	virtual void SplashDamage(dword source, MissionObject* from, const Vector & pos, float radius,float MAXDamage, bool orient_to_attacker, const char* particlename = null,bool DamageAlly = true,float pow = 1.0f,bool allowkick = true,const char* reaction = null,const char* deathLink = null, MissionObject* filter = null) = null;

};


#endif
