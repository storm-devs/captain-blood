#ifndef AI_LevasserAlly_BRAIN
#define AI_LevasserAlly_BRAIN

#include "aiBrain.h"
#include "../Thoughts/LevasserAlly/aiLevasserAllyKick.h"
#include "../Thoughts/LevasserAlly/aiLevasserAllyRoll.h"


class aiLevasserAlly : public aiBrain
{	
	aiLevasserAllyKick*    pLevasserAllyKick;
	aiLevasserAllyRoll*	   pLevasserAllyRoll;	

	enum EnemyBrainState
	{
		EBS_IDLE = 0,
		EBS_PURSUIT,
		EBS_KICK,		
		EBS_STRAFING,
		EBS_BLOCKING,
		EBS_FOLLOWHERO,
		EBS_FATALITY
	};

	EnemyBrainState BrainState;
	
	float fDistToKick;
	float fDistofView;	

	float fMaxDistToPlayer;	

public:

	aiLevasserAlly(Character & ch, const char* szName);
	virtual ~aiLevasserAlly();

	virtual void SetParams(CharacterControllerParams * params);	

	//Перезапустить контроллер при перерождении персонажа
	virtual void Reset();
	
	//Обновить состояние контролера 10 раз в секунду (распределено между персонажами)
	virtual void Tick();
	
	//Персонаж получил люлей
	virtual void Hit(float dmg,Character* _offender, const char* reaction, DamageReceiver::DamageSource source);	

	virtual void Death();	

	virtual bool AttackCharacter();

	virtual void FollowPlayer();	

	virtual const ConstString & GetAIParamsName(){ RETURN_CONSTSTRING("LevasserAllyParams"); }

	virtual bool AllowToBeBlowed() { return false; };
};


#endif


