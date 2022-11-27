//============================================================================================
// Spirenkov Maxim, 2006
//============================================================================================
// MissionTime
//============================================================================================

#include "MissionTime.h"


MissionTime::MissionTime()
{
	timeScale = 1.0f;
	oldTimeScale = newTimeScale = 1.0f;
	speed = blendPosition = 1.0f;
}

MissionTime::~MissionTime()
{
	::Mission & mis = (::Mission &)Mission();
	mis.RemoveTimeModifier(&timeScale);
}

//Инициализировать объект
bool MissionTime::Create(MOPReader & reader)
{
	speed = blendPosition = 1.0f;
	timeScale = oldTimeScale = newTimeScale = reader.Float();		
	Activate(reader.Bool());
	trigger.Init(reader);
	return true;
}

//Активировать/деактивировать объект
void MissionTime::Activate(bool isActive)
{
	MissionObject::Activate(isActive);
	::Mission & mis = (::Mission &)Mission();
	if(isActive)
	{
		mis.AddTimeModifier(&timeScale);
	}else{
		mis.RemoveTimeModifier(&timeScale);
		DelUpdate(&MissionTime::Update);
	}
}

//Обработчик команд для объекта
void MissionTime::Command(const char * id, dword numParams, const char ** params)
{
	Assert(id);
	if(string::IsEqual(id, "set"))
	{
		if(numParams)
		{
			char * ptr = (char *)params[0];
			newTimeScale = (float)strtod(params[0], &ptr);
			newTimeScale = Clampf(newTimeScale, 0.0f, 10000.0f);
			oldTimeScale = timeScale;
			if(numParams > 1)
			{
				char * ptr = (char *)params[1];
				speed = (float)strtod(params[1], &ptr);
				speed = Clampf(speed, 0.0f, 1000.0f);				
				LogicDebug("Command \"set\" blend current value %f to %f by %f seconds", oldTimeScale, newTimeScale, speed);
				if(speed > 1e-10f)
				{
					speed = 1.0f/speed;
					blendPosition = 0.0f;					
					SetUpdate(&MissionTime::Update);
				}else{					
					timeScale = newTimeScale;
				}				
			}else{
				timeScale = newTimeScale;
				LogicDebug("Command \"set\" current value is %f", timeScale);
			}			
		}else{
			LogicDebugError("Skipp command \"%s\" becouse no parameters", id);
		}
	}else{
		LogicDebugError("Undefined command: \"%s\"", id);
	}
}

//Исполниться
void _cdecl MissionTime::Update(float dltTime, long level)
{
	dltTime = api->GetDeltaTime();
	blendPosition += speed*dltTime;
	if(blendPosition < 1.0f)
	{
		timeScale = oldTimeScale + (newTimeScale - oldTimeScale)*blendPosition;		
	}else{
		timeScale = newTimeScale;
		DelUpdate(&MissionTime::Update);
		trigger.Activate(Mission(), false);
	}
}

//Пересоздать объект
void MissionTime::Restart()
{
	Unregistry();
	DelUpdate();
	ReCreate();
};

//Инициализировать объект
bool MissionTime::EditMode_Create(MOPReader & reader)
{
	return true;
}

const char * MissionTime::comment =
"Mission time apply time scale for current mission...\n"
"\n"
"Commands list:\n"
"----------------------------------------\n"
"  Command set - set new current value\n"
"----------------------------------------\n"
"    command: set\n"
"    parm: time\n"
"    [parm: blend time from current to default value]\n"
"    if blend time not defined, value set instantly\n"
" \n"
" ";

MOP_BEGINLISTCG(MissionTime, "Mission time", '1.00', 0, MissionTime::comment, "Managment")
	MOP_FLOATEXC("Time scale", 1.0f, 0.000001f, 1000.0f, "Default time scale")
	MOP_BOOL("Active", true)
	MOP_MISSIONTRIGGERG("Event", "End command event")
MOP_ENDLIST(MissionTime)
