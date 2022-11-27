#ifndef AI_Easterling
#define AI_Easterling

#include "aiBrain.h"
#include "../Thoughts/Easterling/aiEasterlingRage.h"
#include "../Thoughts/Easterling/aiEasterlingDrink.h"

class aiEasterling : public aiBrain
{		
	aiEasterlingRage*   pEasterlingRage;	
	aiEasterlingDrink*  pEasterlingDrink;

	float fDamageCharger;

	float fDrinkHealth;
	float fRageTime;
	float fRageRotTime;	

public:

	struct Attack
	{
		float hp;
		ConstString link;
		bool  used;

		Attack()
		{
			hp = 0.0f;
			used = false;
		}
	};

	array<Attack> attacks;

	aiEasterling(Character & ch, const char* szName);
	virtual ~aiEasterling();

	virtual void SetParams(CharacterControllerParams * params);	
	
	//Обновить состояние контролера 10 раз в секунду (распределено между персонажами)
	virtual void Tick();
	
	//Персонаж получил люлей
	virtual void Hit(float dmg,Character* _offender, const char* reaction, DamageReceiver::DamageSource source);

	virtual bool AttackCharacter();	

	virtual const ConstString & GetAIParamsName(){ RETURN_CONSTSTRING("aiEasterlingParams"); }

	virtual bool AllowToBeBlowed() { return false; };	

	void UpdateAttacks();
};


#endif


