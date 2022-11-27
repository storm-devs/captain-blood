
#include "HPBonus.h"

#include "..\..\Character\Character.h"
#include "..\..\Character\components\CharacterLogic.h"
#include "..\..\player\PlayerController.h"

void HPBonus::BonusAction(MissionObject* player)
{
	if (player->GetHP()==player->GetMaxHP())
	{
		((PlayerController*)((Character*)player)->controller)->AddMoney((float)money_add);	

		return;
	}

	int hp_add = min_value + (int) (Rnd(0.99f) * (float)(max_value - min_value));

	if (hp_add<0)
	{
		hp_add = 0;
	}

	((Character*)player)->logic->SetHP(((Character*)player)->logic->GetHP() + hp_add);
}

bool HPBonus::Create(MOPReader & reader)
{
	ReadStdParams(reader);

	min_value = reader.Long();
	max_value = reader.Long();	

	money_add = reader.Long();

	return true;
}

MOP_BEGINLISTCG(HPBonus, "HP Bonus", '1.00', 5, "HP Bonus", "Bonuses")

BONUSBASE_PARAMS

MOP_LONGEXC("Min HP", 50, 0, 100000, "Minimum health points to add")
MOP_LONGEXC("Max HP", 50, 0, 100000, "Maximum health points to add")
MOP_LONGEXC("Money Value", 50, 0, 100000, "Money to add when full HP")

MOP_ENDLIST(HPBonus);