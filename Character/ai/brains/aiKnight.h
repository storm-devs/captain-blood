#ifndef AI_Knight
#define AI_Knight

#include "aiBrain.h"
#include "../Thoughts/Knight/aiKnightRage.h"

class aiKnight : public aiBrain
{	
	aiKnightRage*	   pKnightRage;
			
public:

	aiKnight(Character & ch, const char* szName);
	virtual ~aiKnight();	
	
	virtual void  ChangeStage(int stage, const char* param);

	virtual void ActivateTargetPairStateThought();

	virtual bool AttackCharacter();	
	
	virtual void Hit(float dmg,Character* _offender, const char* reaction, DamageReceiver::DamageSource source);	

	virtual bool AllowToBeBlowed() { return false; };
};


#endif


