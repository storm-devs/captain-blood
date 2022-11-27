#ifndef AI_MUSKETEER_BRAIN
#define AI_MUSKETEER_BRAIN

#include "aiBrain.h"
#include "../Thoughts/Mushketer/aiMushketerShoot.h"

class aiMusketeer : public aiBrain
{		
public:
		
	aiMushketerShoot*      pMushketerShoot;	

	bool  isGuard;
	float fDistToShoot;
	float fDistofView;
	
	float fMinTimeToShoot;
	float fMaxTimeToShoot;

	float fWaitTime;	

	bool  bAutoSwitchFromSentinel;
	float fRaduisWhenSwitchFromSentinel;

	bool  bAngleHasBeenInit;
	float fInitialAngle;
	float fShootSektor; 

	bool  allow_kick;

public:

	aiMusketeer(Character & ch, const char* szName);
	virtual ~aiMusketeer();

	virtual void SetParams(CharacterControllerParams * params);	

	//Перезапустить контроллер при перерождении персонажа
	virtual void Reset();
	
	virtual void GuardTick();	
	virtual void ActivateTargetPairStateThought();

	virtual void Tick();

	virtual void TickWhenEmpty();

			
	virtual bool AttackCharacter();	

	//Персонаж получил люлей
	virtual void Hit(float dmg,Character* _offender, const char* reaction, DamageReceiver::DamageSource source);	

	virtual const ConstString & GetAIParamsName(){ RETURN_CONSTSTRING("MusketeerParams");   };	

	virtual bool AllowToBeMoved() { return !isGuard; };

	virtual float PursuitDist() { return fDistToShoot * 0.8f; };
};


#endif


