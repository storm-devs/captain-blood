#ifndef AI_Servant_BRAIN
#define AI_Servant_BRAIN

#include "aiBombardeer.h"


class aiServant : public aiBombardeer
{	
public:
	
	float fWaitBombTime;	

	aiServant(Character & ch, const char* szName);
	virtual ~aiServant();	

	//Обновить состояние контролера на каждом кадре
	virtual void Update(float dltTime);

	virtual void TickWhenEmpty();

	virtual bool AttackCharacter();

	//Персонаж получил люлей
	virtual void Hit(float dmg,Character* _offender, const char* reaction, DamageReceiver::DamageSource source);	

	virtual bool AllowToBeBlowed() { return false; };

	void Suicide() {};	

	void ResetWaitBomb();
};


#endif


