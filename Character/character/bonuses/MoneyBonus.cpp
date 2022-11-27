
#include "MoneyBonus.h"
#include "..\..\Character\Character.h"
#include "..\..\player\PlayerController.h"

void MoneyBonus::BonusAction(MissionObject* player)
{
	int money_add = min_value + (int) (Rnd(0.99f) * (float)(max_value - min_value));
		
	if (money_add<0)
	{
		money_add = 0;
	}

	((PlayerController*)((Character*)player)->controller)->AddMoney((float)money_add);	
}

bool MoneyBonus::Create(MOPReader & reader)
{
	ReadStdParams(reader);

	min_dropcount = reader.Long();
	max_dropcount = reader.Long();

	min_value = reader.Long();
	max_value = reader.Long();

	return true;
}

//Указать количество создаваемых бонусов при дропе
long MoneyBonus::DropsCount()
{
	return (min_dropcount + (int) (Rnd(0.99f) * (float)(max_dropcount - min_dropcount)));
}


MOP_BEGINLISTCG(MoneyBonus, "Money Bonus", '1.00', 5, "Money Bonus", "Bonuses")

BONUSBASE_PARAMS

MOP_LONGEXC("Min drop count", 1, 0, 10000, "Minimum number of droped coneys")
MOP_LONGEXC("Max drop count", 3, 0, 10000, "Maximum number of droped coneys")

MOP_LONGEXC("Min money value", 50, 0, 100000, "Minimum number of droped coneys")
MOP_LONGEXC("Max money value", 50, 0, 100000, "Maximum number of droped coneys")

MOP_ENDLIST(MoneyBonus);