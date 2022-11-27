//============================================================================================
// Spirenkov Maxim, 2006
//============================================================================================
// ControlTrigger
//============================================================================================


#include "ControlTrigger.h"



ControlTrigger::ControlTrigger()
{
	contrrolCode = CST_INACTIVE;
	type = CST_ACTIVATED;
	repeatWaitTime = 0.1f;
	currentTime = 0.0f;
	isLookRepeat = false;
}

ControlTrigger::~ControlTrigger()
{
}


//Инициализировать объект
bool ControlTrigger::Create(MOPReader & reader)
{
	contrrolCode = Controls().FindControlByName(reader.String().c_str());
	const char * mode = reader.Enum().c_str();
	if(mode[0] == 'P' || mode[0] == 'p')
	{
		type = CST_ACTIVATED;
		isLookRepeat = false;
		if(mode[5] != 0)
		{
			isLookRepeat = true;
			repeatWaitTime = api->Storage().GetFloat("Runtime.System.RepeatWaitTime", 0.1f);
			repeatWaitTime = Clampf(repeatWaitTime, 0.001f, 10.0f);
		}
	}else{
		type = CST_INACTIVATED;
	}
	trigger.Init(reader);
	Activate(reader.Bool());
	return true;
}

//Активировать
void ControlTrigger::Activate(bool isActive)
{
	DetectorObject::Activate(isActive);
	if(EditMode_IsOn()) return;
	if(isActive)
	{
		SetUpdate(&ControlTrigger::Work, ML_TRIGGERS);
	}else{
		DelUpdate(&ControlTrigger::Work);
	}
}

//Инициализировать объект
bool ControlTrigger::EditMode_Create(MOPReader & reader)
{
	return true;
}

//Работа детектора
void _cdecl ControlTrigger::Work(float dltTime, long level)
{
	if(MissionsManager::Ptr())
	{
		if(!MissionsManager::Ptr()->IsActive(&Mission()))
		{
			return;
		}
	}
	
	if(Controls().GetControlStateType(contrrolCode) == type)
	{
		LogicDebug("Triggering");
		trigger.Activate(Mission(), false);
		currentTime = 0.0f;
	}
	if(isLookRepeat && Controls().GetControlStateType(contrrolCode) == CST_ACTIVE)
	{
		currentTime += dltTime;
		if(currentTime >= repeatWaitTime)
		{
			LogicDebug("Triggering repeat");
			trigger.Activate(Mission(), false);
			currentTime = 0.0f;
		}
	}
}

MOP_BEGINLISTCG(ControlTrigger, "Control trigger", '1.00', 0x0fffffff, "Object trigger at default control (key)\nRepeat time get from \"float Runtime.System.RepeatWaitTime\"", "Logic")
	MOP_ENUMBEG("KeyMode")
		MOP_ENUMELEMENT("Press")
		MOP_ENUMELEMENT("Press and repeat")
		MOP_ENUMELEMENT("Release")
	MOP_ENUMEND
	MOP_STRINGC("Control", "", "Control, definition at some resource\\ini\\control\\*.ini")
	MOP_ENUM("KeyMode", "Control action")
	MOP_MISSIONTRIGGERG("Event", "")
	MOP_BOOL("Active", false)
MOP_ENDLIST(ControlTrigger)




