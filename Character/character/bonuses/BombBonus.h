
#ifndef _BombBonuses_h_
#define _BombBonuses_h_

#include "BaseBonus.h"

class BombBonus : public BaseCharBonus
{	
	int min_value;
	int max_value;	

	virtual void BonusAction(MissionObject* player);
	virtual bool Create(MOPReader & reader);
};

#endif


