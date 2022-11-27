
#ifndef _QuadDamageBonuses_h_
#define _QuadDamageBonuses_h_

#include "BaseBonus.h"

class QuadDamageBonus : public BaseCharBonus
{
	float work_time;

	virtual void BonusAction(MissionObject* player);
	virtual bool Create(MOPReader & reader);	
};

#endif


