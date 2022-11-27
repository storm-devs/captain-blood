#ifndef AI_BOATSWAIN_BRAIN
#define AI_BOATSAWIN_BRAIN

#include "aiBrain.h"
#include "../Thoughts/Boatswain/aiBoatswainWirst.h"
#include "../Thoughts/aiBlocking.h"


class aiBoatswain : public aiBrain
{		
	//aiBoatswainWirst*	 pBoatswainWirst;	
	aiThoughtKick*	     pThoughtKickFar;
	aiThoughtKick*	     pThoughtKickMidle;
	aiThoughtKick*	     pThoughtKickNear;		
				
	float  fDamageCharger;

	float fDistToKick;
	float fDistToFarKick;
	float fDistToMidleKick;	
	
	bool  sec_stage;
	float hp_sec_stage;
	ConstString triger_sec_stage;

	bool  blocking;

public:

	aiBoatswain(Character & ch, const char* szName);
	virtual ~aiBoatswain();

	virtual void SetParams(CharacterControllerParams * params);	
		
	//Обновить состояние контролера 10 раз в секунду (распределено между персонажами)
	virtual void Tick();
	
	//Персонаж получил люлей
	virtual void Hit(float dmg,Character* _offender, const char* reaction, DamageReceiver::DamageSource source);

	virtual void NotifyAboutAttack(Character* attacker,float damage);	

	virtual bool AttackCharacter();	
	
	virtual const ConstString & GetAIParamsName(){ RETURN_CONSTSTRING("aiBoatswainParams"); }

	virtual bool AllowToBeBlowed() { return false; };

	virtual void Command(dword numParams, const char ** params);
};


#endif


