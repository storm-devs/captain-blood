
#include "BaseBonus.h"
#include "..\..\player\PlayerController.h"

bool BaseCharBonus::ReadyForPickup(IGreedy* greedy)
{	
	if (!greedy->OwnerMO() || greedy->OwnerMO()->GetHP()<=0.0f) return false;

	return true;
}

bool BaseCharBonus::Apply(IGreedy* greedy)
{
	if (!greedy->OwnerMO()) return true;

	MO_IS_IF(is_Character, "Character", greedy->OwnerMO())
	{
		if (greedy->OwnerMO()->GetHP()>0) BonusAction(greedy->OwnerMO());

		return true;
	}	

	return true;
}

void BaseCharBonus::BonusAction(MissionObject* player)
{

}

bool BaseCharBonus::Create(MOPReader & reader)
{
	ReadStdParams(reader);

	return true;
}

float BaseCharBonus::GetPoints()
{
	MissionObject* player = Mission().Player();

	if (!player) return 1.0f;

	return player->GetHP()/player->GetMaxHP();	
}

//Найти пересечение колижена и отрезка, если нет пересечения вернуть false
bool BaseCharBonus::CollisionLine(const Vector & start, const Vector & end, Vector & p, Vector & n)
{
	IPhysicsScene::RaycastResult res;

	if (Physics().Raycast(start, end, phys_mask(phys_character),&res))
	{
		p = res.position;
		n = res.normal;

		return true;
	}

	return false;		
}

//Указать количество создаваемых бонусов при дропе
long BaseCharBonus::DropsCount()
{
	return 1;
}

MOP_BEGINLISTCG(BaseCharBonus, "BaseBonus", '1.00', 5, "base fake bonus", "Bonuses")

BONUSBASE_PARAMS

MOP_ENDLIST(BaseCharBonus);