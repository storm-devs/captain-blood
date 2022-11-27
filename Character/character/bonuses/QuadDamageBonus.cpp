
#include "QuadDamageBonus.h"

#include "..\..\Character\Character.h"
#include "..\..\player\PlayerController.h"


void QuadDamageBonus::BonusAction(MissionObject* player)
{
	((PlayerController*)((Character*)player)->controller)->QuadDamage(work_time);	
}

bool QuadDamageBonus::Create(MOPReader & reader)
{
	ReadStdParams(reader);

	work_time = reader.Float();	

	return true;
}


MOP_BEGINLISTCG(QuadDamageBonus, "QuadDamage Bonus", '1.00', 5, "QuadDamage Bonus", "Bonuses")

BONUSBASE_PARAMS
MOP_FLOATEXC("Work time", 7.0f, 1.0f, 1000.0f, "Work time of QuadDamage") \


MOP_ENDLIST(QuadDamageBonus);