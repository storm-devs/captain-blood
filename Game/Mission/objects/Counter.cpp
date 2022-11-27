//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// Mission objects
//===========================================================================================================================
// Counter	
//============================================================================================
			

#include "Counter.h"
#include "MissionReloader.h"

//============================================================================================

Counter::Counter()
{
	counter = 0;
	maxCount = 0;
	lastTick = false;
}

Counter::~Counter()
{
}

//============================================================================================

//Инициализировать объект
bool Counter::Create(MOPReader & reader)
{
	counter = 0;
	maxCount = reader.Long();
	lastTick = reader.Bool();
	Activate(reader.Bool());
	eventTick.Init(reader);
	eventCount.Init(reader);
	return true;
}

//Обработчик команд для объекта
void Counter::Command(const char * id, dword numParams, const char ** params)
{
	if(!id) id = "";
	if(string::IsEqual(id, "reset"))
	{
		counter = 0;
		eventTick.Reset();
		eventCount.Reset();
		LogicDebug("Reseting...");
		return;
	}
	if(!IsActive())
	{
		LogicDebug("Skip command \"%s\", counter now is't active...", id);
		return;
	}
	if(counter >= maxCount)
	{
		LogicDebug("Skip command \"%s\", counter already done (%i of %i)...", id, counter, maxCount);
		return;
	}
	if(string::IsEqual(id, "tick"))
	{
		long delta = 1;
		if(numParams > 0)
		{
			char * chr = null;
			long delta = abs(strtol(params[0], &chr, 10));			
		}
		counter += delta;
		bool isEnableTick = true;
		if(counter >= maxCount)
		{
			isEnableTick = lastTick;
		}
		LogicDebug("Tick %i of %i (step = %i)", counter, maxCount, delta);
		if(isEnableTick)
		{
			LogicDebug("Tick event");
			eventTick.Activate(Mission(), false);
		}
		if(counter >= maxCount)
		{
			LogicDebug("End count event");
			eventCount.Activate(Mission(), false);
		}
		return;
	}
	if(string::IsEqual(id, "tickdown"))
	{
		long delta = 1;
		if(numParams > 0)
		{
			char * chr = null;
			long delta = abs(strtol(params[0], &chr, 10));			
		}
		counter -= delta;
		if(counter < 0) counter = 0;
		LogicDebug("Tickdown %i of %i (step = %i)", counter, maxCount, delta);
	}
	LogicDebug("Unknow command \"%s\"", id);
}

//Активировать
void Counter::Activate(bool isActive)
{
	MissionObject::Activate(isActive);
	if(IsActive())
	{
		LogicDebug("Activate");
	}else{
		LogicDebug("Deactivate");
	}
}

//Инициализировать объект
bool Counter::EditMode_Create(MOPReader & reader)
{
	return true;
}

//Обновить параметры
bool Counter::EditMode_Update(MOPReader & reader)
{
	return true;
}

//Получить размеры описывающего ящика
void Counter::EditMode_GetSelectBox(Vector & min, Vector & max)
{
	min = max = 0.0f;
}


//============================================================================================
//Описание
//============================================================================================

const char * Counter::comment = 
"Counter use for count some events...\n"
"\n"
"Commands list:\n"
"----------------------------------------\n"
"  Command tick - increase counter\n"
"----------------------------------------\n"
"    command: tick\n"
" \n"
"----------------------------------------\n"
"  Command tick count - increase counter\n"
"----------------------------------------\n"
"    command: tick\n"
"      param: count\n"
" \n"
"----------------------------------------\n"
"  Command tickdown - decrease counter\n"
"----------------------------------------\n"
"    command: tickdown\n"
" \n"
"----------------------------------------\n"
"  Command tickdown count - decrease counter\n"
"----------------------------------------\n"
"    command: tickdown\n"
"      param: count\n"
" \n"
"----------------------------------------\n"
"  Reset counter\n"
"----------------------------------------\n"
"    command: reset\n"
" ";

//============================================================================================
//Параметры инициализации
//============================================================================================

MOP_BEGINLISTCG(Counter, "Counter", '1.00', 0x0fffffff, Counter::comment, "Logic")
	MOP_LONGEXC("Repeat count", 5, 1, 1000000000, "Count\nFor repeat activate need command \"reset\".");
	MOP_BOOLC("Last tick", true, "Enable last \"Tick event\" before \"End count event\"")
	MOP_BOOLC("Active", true, "Active trigger in start mission time")
	MOP_MISSIONTRIGGER("Tick event")
	MOP_MISSIONTRIGGER("End count event")
MOP_ENDLIST(Counter)

