




#include "RandomSelector.h"



RandomSelector::RandomSelector() : events(_FL_)
{
}

RandomSelector::~RandomSelector()
{
}

//Инициализировать объект
bool RandomSelector::Create(MOPReader & reader)
{
	events.DelAll();
	long count = reader.Array();
	for(long i = 0; i < count; i++)
	{
		events[events.Add()].Init(reader);
	}
	return true;
}

//Активировать
void RandomSelector::Activate(bool isActive)
{
	MissionObject::Activate(isActive);
	if(isActive)
	{
		LogicDebug("Activate");
		LogicDebugLevel(true);
		if(events > 0)
		{
			long randCount = long(Rnd(events*1024.0f));
			long i = randCount % events;
			LogicDebug("Activate item %i", i);			
			events[i].Activate(Mission(), false);
		}
		LogicDebugLevel(false);
		MissionObject::Activate(false);
	}else{
		LogicDebug("Deactivate");
	}
}

//Инициализировать объект
bool RandomSelector::EditMode_Create(MOPReader & reader)
{
	return Create(reader);
}

//Обновить параметры
bool RandomSelector::EditMode_Update(MOPReader & reader)
{
	events.DelAll();
	return Create(reader);
}

//Получить размеры описывающего ящика
void RandomSelector::EditMode_GetSelectBox(Vector & min, Vector & max)
{
	min = max = 0.0f;
}


MOP_BEGINLISTCG(RandomSelector, "Random selector", '1.00', 0x0fffffff, "When activate, random select trigger from array", "Logic")
	MOP_ARRAYBEG("Events", 1, 1000)
		MOP_MISSIONTRIGGER("")
	MOP_ARRAYEND
MOP_ENDLIST(RandomSelector)





