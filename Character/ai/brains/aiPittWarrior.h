#ifndef AI_PITTWARRIOR_BRAIN
#define AI_PITTWARRIOR_BRAIN

#include "aiBrain.h"
#include "../Thoughts/Pitt/aiPittRoll.h"

class aiPittWarrior : public aiBrain
{		
	aiPittRoll*       pPittRoll;
			
public:

	aiPittWarrior(Character & ch, const char* szName);
	virtual ~aiPittWarrior();	
	
	virtual void ActivateTargetPairStateThought();

	virtual bool AttackCharacter();		

	virtual bool AllowToBeBlowed() { return false; };
};


#endif


