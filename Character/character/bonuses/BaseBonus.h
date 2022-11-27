
#ifndef _CharacterBonuses_h_
#define _CharacterBonuses_h_

#include "..\..\..\common_h\bonuses.h"

class BaseCharBonus : public BonusBase
{		
	virtual bool ReadyForPickup(IGreedy* greedy);

	virtual bool Apply(IGreedy* greedy);

	virtual void BonusAction(MissionObject* player);

	float GetPoints();

	virtual bool Create(MOPReader & reader);	

	//Найти пересечение колижена и отрезка, если нет пересечения вернуть false
	virtual bool CollisionLine(const Vector & start, const Vector & end, Vector & p, Vector & n);

	//Указать количество создаваемых бонусов при дропе
	virtual long DropsCount();	

public:

	int money_add;
};

#endif


