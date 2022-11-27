//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// Mission objects
//===========================================================================================================================
// Synchronizer	
//============================================================================================
			

#include "Synchronizer.h"
#include "MissionReloader.h"

const Synchronizer::Logic Synchronizer::logics[] = 
{
	{ConstString("All at one time"), &Synchronizer::LogicAll},
	{ConstString("One at one time (random)"), &Synchronizer::LogicRand},
	{ConstString("One at one time (first)"), &Synchronizer::LogicFirst},
};


//============================================================================================

Synchronizer::Synchronizer() : events(_FL_)
{
	logicIndex = -1;
}

Synchronizer::~Synchronizer()
{
}

//============================================================================================

//Инициализировать объект
bool Synchronizer::Create(MOPReader & reader)
{
	events.DelAll();
	ConstString type = reader.Enum();
	logicIndex = -1;
	for(long i = 0; i < ARRSIZE(logics); i++)
	{
		if(type == logics[i].name)
		{
			logicIndex = i;
			break;
		}
	}
	Assert(logicIndex >= 0);
	long count = reader.Array();	
	for(long i = 0; i < count; i++)
	{
		Event & evt = events[events.Add()];
		evt.name = reader.String();
		evt.event.Init(reader);
		evt.state = false;
	}	
	tick.Init(reader);
	Activate(reader.Bool());	
	return true;
}

//Обработчик команд для объекта
void Synchronizer::Command(const char * id, dword numParams, const char ** params)
{
	if(!id) return;
	ConstString cid(id);
	static const ConstString reset("reset");
	if(cid == reset)
	{
		for(long i = 0; i < events; i++)
		{
			events[i].state = false;
			events[i].event.Reset();
		}
		LogicDebug("Reseting...");
		return;
	}
	if(!IsActive()) return;
	//Тик
	static const ConstString ctick("tick");
	if(cid == ctick)
	{
		LogicDebug("Tick event...");
		LogicDebugLevel(true);
		if(logicIndex >= 0)
		{
			LogicDebug("Logic: \"%s\"", logics[logicIndex].name.c_str());
			(this->*logics[logicIndex].logic)();
		}
		LogicDebugLevel(false);
		tick.Activate(Mission(), false);
		return;
	}
	//Если ошибочны параметры - скипаем
	if(!numParams || !params[0])
	{
		LogicDebug("Synchronizer -> invalidate event name: \"\"");
		return;
	}
	ConstString param(params[0]);
	long index = -1;
	for(long i = 0; i < events; i++)
	{
		if(events[i].name == param)
		{
			index = i;
			break;
		}
	}
	if(index < 0)
	{
		LogicDebug("Synchronizer -> invalidate input event name: \"%s\"", params[0]);
		return;
	}
	//Активация
	static const ConstString evt("event");
	if(cid == evt)
	{
		LogicDebug("Registry event \"%s\"", params[0]);
		events[index].state = true;
		return;
	}
	//Деактивация
	static const ConstString unevt("unevent");
	if(cid == unevt)
	{
		LogicDebug("Unevent \"%s\"", params[0]);
		events[index].state = false;
		return;
	}
	LogicDebugError("Unknown command: %s", id);
}

//Активировать
void Synchronizer::Activate(bool isActive)
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
bool Synchronizer::EditMode_Create(MOPReader & reader)
{
	return true;
}

//Обновить параметры
bool Synchronizer::EditMode_Update(MOPReader & reader)
{
	return true;
}

//Получить размеры описывающего ящика
void Synchronizer::EditMode_GetSelectBox(Vector & min, Vector & max)
{
	min = max = 0.0f;
}

void _cdecl Synchronizer::LogicAll()
{
	for(long i = 0; i < events; i++)
	{
		if(events[i].state)
		{
			LogicDebug("Activate event \"%s\"", events[i].name.c_str());
			events[i].event.Activate(Mission(), false);
			events[i].state = false;
		}
	}
}

void _cdecl Synchronizer::LogicRand()
{
	long count = 0;
	for(long i = 0; i < events; i++)
	{
		if(events[i].state)
		{
			count++;
		}
	}
	if(!count) return;
	long index = (long)Rnd(count + 0.999f);
	if(index >= count) index = count - 1;
	count = 0;
	for(long i = 0; i < events; i++)
	{
		if(events[i].state)
		{
			if(count++ == index)
			{
				LogicDebug("Activate event \"%s\"", events[i].name.c_str());
				events[i].event.Activate(Mission(), false);
				events[i].state = false;
				return;
			}
		}
	}
	Assert(false);
}

void _cdecl Synchronizer::LogicFirst()
{
	for(long i = 0; i < events; i++)
	{
		if(events[i].state)
		{
			LogicDebug("Activate event \"%s\"", events[i].name.c_str());
			events[i].event.Activate(Mission(), false);
			events[i].state = false;
			break;
		}
	}
}


//============================================================================================
//Описание
//============================================================================================


const char * Synchronizer::desc =
"Trigger type:\n"
"    #b\"All at one time\"@b          when get tick event, all active input triggers,\n"
"                                     sended events and reset it active state\n"
"    #b\"One at one time (random)\"@b when get tick event, one random active input trigger\n"
"                                     sended event and reset his active state\n"
"    #b\"One at one time (first)\"@b  when get tick event, one first active input trigger\n"
"                                     sended event and reset his active state\n"
" ";

const char * Synchronizer::comment = 
"Object for synchinize events in time\n"
" \n"
"Commands list:\n"
"----------------------------------------\n"
"  Tick event - time for action\n"
"----------------------------------------\n"
"    command: tick\n"
" \n"
"----------------------------------------\n"
"  Activate input trigger\n"
"----------------------------------------\n"
"    command: event\n"
"    parm: input trigger name\n"
" \n"
"----------------------------------------\n"
"  Deactivate input trigger\n"
"----------------------------------------\n"
"    command: unevent\n"
"    parm: input trigger name\n"
" \n"
"----------------------------------------\n"
"  Reset all input triggers and\n"
"  output trigger\n"
"----------------------------------------\n"
"    command: reset\n"
" \n"
" ";

//============================================================================================
//Параметры инициализации
//============================================================================================

MOP_BEGINLISTCG(Synchronizer, "Synchronizer", '1.00', 0x0fffffff, Synchronizer::comment, "Logic")
	MOP_ENUMBEG("SynchroFunction")
		for(dword i = 0; i < ARRSIZE(Synchronizer::logics); i++)
		{
			MOP_ENUMELEMENT(Synchronizer::logics[i].name.c_str())
		}
	MOP_ENUMEND
	MOP_ENUMC("SynchroFunction", "Type", Synchronizer::desc)
	MOP_ARRAYBEGC("Events", 1, 1000, "Synchronize/unsynchronize events")
		MOP_STRINGC("Name", "", "Input event name")
		MOP_MISSIONTRIGGER("")
	MOP_ARRAYEND
	MOP_MISSIONTRIGGERG("Tick event", "Tick event")
	MOP_BOOLC("Active", true, "Active trigger in start mission time")
MOP_ENDLIST(Synchronizer)



