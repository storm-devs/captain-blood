#ifndef AI_FinalBoss
#define AI_FinalBoss

#include "aiBrain.h"

class aiFinalBoss : public aiBrain
{		
	aiThoughtKick*      pThoughtCounterKick;
	float damage_to_block;
	float dist_to_block;
	float strafe_cooldown;
	float strafeTime;

public:	

	aiFinalBoss(Character & ch, const char* szName);
	virtual ~aiFinalBoss();

	virtual void SetParams(CharacterControllerParams * params);	
	
	//Обновить состояние контролера 10 раз в секунду (распределено между персонажами)
	virtual void Tick();
	
	//Мыслить ;)
	virtual void ExecuteThoughts (float fDeltaTime);	

	//Персонаж получил люлей
	virtual void Hit(float dmg, Character* _offender, const char* reaction, DamageReceiver::DamageSource source);

	virtual void NotifyAboutAttack(Character* attacker,float damage);

	virtual bool AttackCharacter();	

	virtual const ConstString & GetAIParamsName(){ RETURN_CONSTSTRING("aiFinalBossParams"); }

	virtual bool AllowToBeBlowed() { return false; };	
};


#endif


