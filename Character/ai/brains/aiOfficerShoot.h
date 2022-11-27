#ifndef AI_MUSKETEER_BRAIN
#define AI_MUSKETEER_BRAIN

#include "aiBrain.h"

#include "../Thoughts/aiPursuit.h"
#include "../Thoughts/aiStanding.h"
#include "../Thoughts/OfficerShoot/aiOfficerShootShoot.h"
#include "../Thoughts/aiBlocking.h"

class aiOfficerShoot : public aiBrain
{			
	aiOfficerShootShoot*    pOfficerShootShoot;

	float fDistToShoot;	
	
	float fMinTimeToShoot;
	float fMaxTimeToShoot;	
	
public:

	aiOfficerShoot(Character & ch, const char* szName);
	virtual ~aiOfficerShoot();

	virtual void SetParams(CharacterControllerParams * params);		

	virtual void ActivateTargetPairStateThought();

	virtual void TickWhenEmpty();

	virtual bool AttackCharacter();	

	//Персонаж получил люлей
	virtual void Hit(float dmg,Character* _offender, const char* reaction, DamageReceiver::DamageSource source);
	
	virtual void Shoot(float dmg);
	
	virtual const ConstString & GetAIParamsName(){ RETURN_CONSTSTRING("OfficerShootParams"); }

	virtual float PursuitDist() { return fDistToShoot * 0.8f; };
};


#endif


