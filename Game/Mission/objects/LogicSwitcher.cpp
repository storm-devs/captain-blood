//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// Mission objects
//===========================================================================================================================
// LogicSwitcher	
//============================================================================================
			

#include "LogicSwitcher.h"
#include "MissionReloader.h"

//============================================================================================

LogicSwitcher::LogicSwitcher() : input(_FL_)
{
	andLogic = true;
}

LogicSwitcher::~LogicSwitcher()
{
}

//============================================================================================

//Инициализировать объект
bool LogicSwitcher::Create(MOPReader & reader)
{
	input.DelAll();
	andLogic = false;
	const char * mode = reader.Enum().c_str();
	if(mode)
	{
		if(mode[0] == 'A' || mode[0] == 'a')
		{
			if(mode[1] == 'L' || mode[1] == 'l')
			{
				andLogic = true;
			}
		}
	}
	long icount = reader.Array();
	for(long i = 0; i < icount; i++)
	{
		Input & inp = input[input.Add()];
		inp.name = reader.String();
		inp.state = false;
	}
	Activate(reader.Bool());
	event.Init(reader);
	return true;
}

//Обработчик команд для объекта
void LogicSwitcher::Command(const char * id, dword numParams, const char ** params)
{
	if(string::IsEmpty(id)) return;
	ConstString cid(id);
	static const ConstString creset("reset");
	if(cid == creset)
	{
		for(long i = 0; i < input; i++) input[i].state = false;
		event.Reset();
		LogicDebug("Reseting...");
		return;
	}
	if(!IsActive())
	{
		LogicDebug("Skip command \"%s\", switch now is't active...", id);
		return;
	}
	//Если ошибочны параметры - скипаем
	if(!numParams || !params[0])
	{
		LogicDebug("Logic switcher -> invalidate input trigger name: \"\"");
		return;
	}
	long index = -1;
	ConstString param(params[0]);
	for(long i = 0; i < input; i++)
	{
		if(input[i].name == param)
		{
			index = i;
			break;
		}
	}
	if(index < 0)
	{
		LogicDebug("Logic switcher -> unknown input trigger name: \"%s\"", params[0]);
		return;
	}
	//Активация
	static const ConstString cevent("event");
	if(cid == cevent)
	{
		LogicDebug("Receive event \"%s\"", params[0]);
		input[index].state = true;
		bool isStart = true;
		if(andLogic)
		{	
			for(long i = 0; i < input; i++)
			{
				if(!input[i].state)
				{
					isStart = false;
					break;
				}
			}
		}
		if(isStart)
		{
			LogicDebug("Triggering...");
			event.Activate(Mission());
		}
		return;
	}
	//Деактивация
	static const ConstString cunevent("unevent");
	if(cid == cunevent)
	{
		LogicDebug("Unevent \"%s\"", params[0]);
		input[index].state = false;
		return;
	}
	LogicDebugError("Unknown command: %s", id);
}

//Активировать
void LogicSwitcher::Activate(bool isActive)
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
bool LogicSwitcher::EditMode_Create(MOPReader & reader)
{
	return true;
}

//Обновить параметры
bool LogicSwitcher::EditMode_Update(MOPReader & reader)
{
	return true;
}

//Получить размеры описывающего ящика
void LogicSwitcher::EditMode_GetSelectBox(Vector & min, Vector & max)
{
	min = max = 0.0f;
}


//============================================================================================
//Описание
//============================================================================================


const char * LogicSwitcher::triggerDesc =
"Trigger type:\n"
"    #b\"All active\"@b when all input triggers active, activated output trigger\n"
"    #b\"Any active\"@b when any input trigger active, activated output trigger\n"
" ";

const char * LogicSwitcher::comment = 
"Logic element use for analyse of many event\n"
"\n"
"Commands list:\n"
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

MOP_BEGINLISTCG(LogicSwitcher, "Logic switcher", '1.00', 0x0fffffff, LogicSwitcher::comment, "Logic")
	MOP_ENUMBEG("LogicFunction")
		MOP_ENUMELEMENT("All active")
		MOP_ENUMELEMENT("Any active")
	MOP_ENUMEND
	MOP_ENUMC("LogicFunction", "Type", LogicSwitcher::triggerDesc)
	MOP_ARRAYBEGC("Input", 1, 1000, "Input triggers")
		MOP_STRINGC("Name", "", "Input trigger name")
	MOP_ARRAYEND
	MOP_BOOLC("Active", true, "Active trigger in start mission time")
	MOP_MISSIONTRIGGER("")
MOP_ENDLIST(LogicSwitcher)



