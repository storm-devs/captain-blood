




#include "Balancer.h"


Balancer::Balancer() : levels(_FL_)
{
}

Balancer::~Balancer()
{
}

//Инициализировать объект
bool Balancer::Create(MOPReader & reader)
{
	levels.DelAll();
	level = reader.Long();
	maxLevel = reader.Long();
	minLevel = reader.Long();	
	if(maxLevel <= minLevel)
	{
		maxLevel = minLevel + 1;
		LogicDebug("Invalidate min or max level, now is min level = %i, max level = %i", minLevel, maxLevel);
	}
	if(level < minLevel || level > maxLevel)
	{
		if(level < minLevel) level = minLevel;
		if(level > maxLevel) level = maxLevel;
		LogicDebug("Initial level out of range and corrected: %i", level);
	}
	long count = reader.Array();
	for(long i = 0; i < count; i++)
	{
		LevelTrigger & lt = levels[levels.Add()];
		lt.level = reader.Long();
		lt.incCounter.Init(reader);
		lt.decCounter.Init(reader);
	}
	Activate(reader.Bool());
	if(levels == 0)
	{
		LogicDebug("No thresholds...");
	}
	return true;
}

//Обработчик команд для объекта
void Balancer::Command(const char * id, dword numParams, const char ** params)
{
	if(!IsActive())
	{
		LogicDebug("Skip command %s, because object not actvie", id);
		return;
	}
	if(string::IsEqual(id, "inc"))
	{
		if(level < maxLevel)
		{
			level++;
			LogicDebug("Command <inc>, level = %i", level);
			CheckEvent(true);
		}else{
			LogicDebug("Command <inc>, level already is max (%i), skip events", level);
		}
	}else
	if(string::IsEqual(id, "dec"))
	{
		if(level > minLevel)
		{
			level--;
			LogicDebug("Command <dec>, level = %i", level);
			CheckEvent(false);
		}else{
			LogicDebug("Command <dec>, level already is min (%i), skip events", level);
		}
	}
	if(!numParams)
	{
		LogicDebug("Command <%s> - no param", id);
		return;
	}
	//
	char * stop;
	long lvl = strtol(params[0], &stop, 10);
	if(lvl < minLevel) lvl = minLevel;
	if(lvl > maxLevel) lvl = maxLevel;
	if(string::IsEqual(id, "set"))
	{		
		level = lvl;
		LogicDebug("Command <set>, new level is %i", level);		
	}else
	if(string::IsEqual(id, "change"))
	{
		bool isUp = (level < lvl);
		level = lvl	;
		LogicDebug("Command <change>, new level is %i", level);
		CheckEvent(isUp);
	}
}

//Инициализировать объект
bool Balancer::EditMode_Create(MOPReader & reader)
{
	return Create(reader);
}

//Обновить параметры
bool Balancer::EditMode_Update(MOPReader & reader)
{
	levels.DelAll();
	return Create(reader);
}

//Получить размеры описывающего ящика
void Balancer::EditMode_GetSelectBox(Vector & min, Vector & max)
{
	min = max = 0.0f;
}


void Balancer::CheckEvent(bool isUp)
{
	for(long i = 0; i < levels; i++)
	{
		if(levels[i].level == level)
		{
			if(isUp)
			{
				LogicDebug("Activate increase event for level %i", level);
				levels[i].incCounter.Activate(Mission(), false);
			}else{
				LogicDebug("Activate decrease event for level %i", level);
				levels[i].decCounter.Activate(Mission(), false);
			}
		}
	}
}

const char * Balancer::comment = 
"Object for some events at some count\n"
" \n"
"Commands list:\n"
"----------------------------------------\n"
"  Increase counter value\n"
"----------------------------------------\n"
"    command: inc\n"
" \n"
"----------------------------------------\n"
"  Decrease counter value\n"
"----------------------------------------\n"
"    command: dec\n"
" \n"
"----------------------------------------\n"
"  Set counter value without events\n"
"----------------------------------------\n"
"    command: set\n"
"    parm: number value\n"
" \n"
"----------------------------------------\n"
"  Change counter value with events\n"
"----------------------------------------\n"
"    command: change\n"
"    parm: number value\n"
" \n"
" ";

MOP_BEGINLISTCG(Balancer, "Balancer", '1.00', 0x0fffffff, Balancer::comment, "Logic")
	MOP_LONGEX("Initial level", 0, 0, 1000000)
	MOP_LONGEX("Max level", 10, 0, 1000000)
	MOP_LONGEX("Min level", 0, 0, 1000000)
	MOP_ARRAYBEG("Level thresholds", 1, 1000)
		MOP_LONGC("Threshold", 5, "Level threshold")
		MOP_MISSIONTRIGGER("inc ")
		MOP_MISSIONTRIGGER("dec ")
	MOP_ARRAYEND
	MOP_BOOLC("Active", true, "Active trigger in start mission time")
MOP_ENDLIST(Balancer)





