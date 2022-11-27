
#include "RageBonus.h"

#include "..\..\Character\Character.h"
#include "..\..\player\PlayerController.h"


void RageBonus::BonusAction(MissionObject* player)
{
	if (((PlayerController*)((Character*)player)->controller)->IsFullRage())
	{
		((PlayerController*)((Character*)player)->controller)->AddMoney((float)money_add);

		return;
	}

	int rage_add = min_value + (int) (Rnd(0.99f) * (float)(max_value - min_value));
		
	((PlayerController*)((Character*)player)->controller)->AddRage(rage_add);
}

bool RageBonus::Create(MOPReader & reader)
{
	ReadStdParams(reader);

	min_value = reader.Long();
	max_value = reader.Long();	

	money_add = reader.Long();

	return true;
}




MOP_BEGINLISTCG(RageBonus, "Rage Bonus", '1.00', 5, "Rage Bonus", "Bonuses")

	BONUSBASE_PARAMS

	MOP_LONGEXC("Min rage points", 50, 0, 100000, "Minimum rage points to add")
	MOP_LONGEXC("Max rage points", 50, 0, 100000, "Maximum rage points to add")

	MOP_LONGEXC("Money Value", 50, 0, 100000, "Money to add when rage full")

MOP_ENDLIST(RageBonus);