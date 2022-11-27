#ifndef AI_TerroristAhmed_BRAIN
#define AI_TerroristAhmed_BRAIN

#include "aiBombardeer.h"

class aiTerroristAhmed : public aiBombardeer
{		
public:

	aiTerroristAhmed(Character & ch, const char* szName);
	virtual ~aiTerroristAhmed();
	
	virtual void ActivateTargetPairStateThought();
	
	virtual void TickWhenEmpty();

	//Персонаж получил люлей
	virtual void Hit(float dmg,Character* _offender, const char* reaction);			

	virtual bool AttackCharacter();	
};


#endif


