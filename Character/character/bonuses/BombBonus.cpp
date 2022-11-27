
#include "BombBonus.h"

#include "..\..\Character\Character.h"
#include "..\..\player\PlayerController.h"

void BombBonus::BonusAction(MissionObject* player)
{
	if (((PlayerController*)((Character*)player)->controller)->IsMaxBombs())
	{
		((PlayerController*)((Character*)player)->controller)->AddMoney((float)money_add);	

		return;
	}

	int bomb_add = min_value + (int) (Rnd(0.99f) * (float)(max_value - min_value));
		
	((PlayerController*)((Character*)player)->controller)->AddBomb(bomb_add);
}

bool BombBonus::Create(MOPReader & reader)
{	
	ReadStdParams(reader);

	min_value = reader.Long();
	max_value = reader.Long();	

	money_add = reader.Long();

	return true;
}




MOP_BEGINLISTCG(BombBonus, "Bomb Bonus", '1.00', 5, "Bomb Bonus", "Bonuses")

BONUSBASE_PARAMS

MOP_LONGEXC("Min Bomb", 50, 0, 100000, "Minimum bombs to add")
MOP_LONGEXC("Max Bomb", 50, 0, 100000, "Maximum bombs to add")
MOP_LONGEXC("Money Value", 50, 0, 100000, "Money to add when bombs full")

MOP_ENDLIST(BombBonus);