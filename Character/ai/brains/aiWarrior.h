#ifndef AI_WARRIOR_BRAIN
#define AI_WARRIOR_BRAIN

#include "aiBrain.h"
#include "../Thoughts/aiBlocking.h"

class aiWarrior : public aiBrain
{
public:
	aiWarrior(Character & ch, const char* szName);

	//Обновить состояние контролера 10 раз в секунду (распределено между персонажами)
	virtual void Tick();
	virtual void StatistTick();		
	
	//virtual bool CheckCharState(Character* target);

	//Обновить состояние контролера на каждом кадре
	virtual void Update(float dltTime);	

	virtual void ActivateTargetPairStateThought();

	//Персонаж получил люлей
	virtual void Hit(float dmg,Character* _offender, const char* reaction, DamageReceiver::DamageSource source);

	virtual bool IsStatist() { return bIsStatist; };
};


#endif


