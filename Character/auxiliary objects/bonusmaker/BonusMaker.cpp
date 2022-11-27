
#include "BonusMaker.h"
#include "..\..\..\common_h\bonuses.h"

BonusMaker::BonusMaker():tables(_FL_),spawn_points (_FL_)
{
	startedBonusTime = 0.f;
}

BonusMaker::~BonusMaker()
{	
}

bool BonusMaker::Create(MOPReader & reader)
{	
	tables.Empty();

	int count = reader.Array();

	for (int i=0;i<count;i++)
	{
		tables.Add(reader.String());
	}

	count = reader.Array();

	for (int i=0;i<count;i++)
	{
		spawn_points.Add(reader.Position());
	}

	startedBonusTime = reader.Float();

	return true;
}


bool BonusMaker::EditMode_Update(MOPReader & reader)
{	
	return true;
}

void BonusMaker::Activate(bool isActive)
{	
	if (!isActive) return;

	if (spawn_points.Size()==0 || tables.Size()==0) return;

	int pos = (int)(Rnd(0.999f) * spawn_points.Size());
	int bonus = (int)(Rnd(0.999f) * tables.Size());	

	BonusesManager::DropParams params;
	params.maxAy = 0.0f;
	params.minAy = 0.0f;

	params.maxVy = 0.0f;
	params.minVy = 0.0f;	

	params.maxVxz = 0.0f;
	params.minVxz = 0.0f;

	params.friction = 5.0f;

	params.skipLifeTime = startedBonusTime;

	BonusesManager::CreateBonus(Mission(),spawn_points[pos],tables[bonus],&params);
}

const char * BonusMaker::comment = "Bonus Maker - рождает бонус в одной из заданной позиции при активации";

MOP_BEGINLISTCG(BonusMaker, "BonusMaker", '1.00', 150, BonusMaker::comment,"Character objects")
			
	MOP_ARRAYBEGC("Bonus Tabels", 0, 1000, "List of Bonus Tabeles")
		MOP_STRING("Bonus Table", "")		
	MOP_ARRAYEND

	MOP_ARRAYBEGC("Spawn Points", 0, 1000, "List of bonus spawn points")
		MOP_POSITION("point", 0.0f)		
	MOP_ARRAYEND

	MOP_FLOATC("SkipTime", 0.f, "Started life time for new bonuses (2 sec - speed rotate, 5 sec - immune time)")

MOP_ENDLIST(BonusMaker)