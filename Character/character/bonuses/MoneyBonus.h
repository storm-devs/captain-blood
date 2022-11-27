
#ifndef _MoneyBonuses_h_
#define _MoneyBonuses_h_

#include "BaseBonus.h"

class MoneyBonus : public BaseCharBonus
{
	int min_dropcount;
	int max_dropcount;

	int min_value;
	int max_value;

	virtual void BonusAction(MissionObject* player);
	
	virtual bool Create(MOPReader & reader);	
	
	//Указать количество создаваемых бонусов при дропе
	virtual long DropsCount();	
};

#endif


