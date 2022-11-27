
#ifndef _RageBonuses_h_
#define _RageBonuses_h_

#include "BaseBonus.h"

class RageBonus : public BaseCharBonus
{	
	int min_value;
	int max_value;

	virtual void BonusAction(MissionObject* player);
	virtual bool Create(MOPReader & reader);
};

#endif


