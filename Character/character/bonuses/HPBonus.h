
#ifndef _HPBonuses_h_
#define _HPBonuses_h_

#include "BaseBonus.h"

class HPBonus : public BaseCharBonus
{
	int min_value;
	int max_value;
	
	virtual void BonusAction(MissionObject* player);
	virtual bool Create(MOPReader & reader);
};

#endif


