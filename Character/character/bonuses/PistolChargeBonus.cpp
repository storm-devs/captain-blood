
#include "PistolChargeBonus.h"

#include "..\..\Character\Character.h"
#include "..\..\player\PlayerController.h"


void PistolChargeBonus::BonusAction(MissionObject* player)
{
	((PlayerController*)((Character*)player)->controller)->ChargePistol();	
}


MOP_BEGINLISTCG(PistolChargeBonus, "Pistol Charge Bonus", '1.00', 5, "Pistol Charge Bonus", "Bonuses")

BONUSBASE_PARAMS

MOP_ENDLIST(PistolChargeBonus);