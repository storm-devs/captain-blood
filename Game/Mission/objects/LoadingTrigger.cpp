//============================================================================================
// Spirenkov Maxim, 2006
//============================================================================================
// LoadingTrigger
//============================================================================================

#include "LoadingTrigger.h"

LoadingTrigger::LoadingTrigger()
{
	mission = null;
}

LoadingTrigger::~LoadingTrigger()
{
}

//Инициализировать объект
bool LoadingTrigger::Create(MOPReader & reader)
{
	mission = reader.String().c_str();
	if(!mission || !mission[0])
	{
		if(!EditMode_IsOn())
		{
			return false;
		}		
	}
	Activate(reader.Bool());
	trigger.Init(reader);
	return true;
}

//Активировать/деактивировать объект
void LoadingTrigger::Activate(bool isActive)
{
	MissionObject::Activate(isActive);
	if(!EditMode_IsOn())
	{
		if(isActive)
		{
			SetUpdate(&LoadingTrigger::Work, ML_TRIGGERS);
		}else{
			DelUpdate(&LoadingTrigger::Work);
		}
	}
}

//Исполнить событие
void _cdecl LoadingTrigger::Work(float dltTime, long level)
{
	if(!IsActive())
	{
		Activate(IsActive());
		return;
	}
	MissionsManager * mng = MissionsManager::Ptr();
	if(mng)
	{
		if(mng->IsLoadMission(mission))
		{
			Activate(false);
			trigger.Activate(Mission(), false);
		}
	}else{
		LogicDebugError("Mission manager not found (start from editor?)");
		Activate(false);
	}
}

//Пересоздать объект
void LoadingTrigger::Restart()
{
	DelUpdate();
	Unregistry();
	ReCreate();
};

//Инициализировать объект
bool LoadingTrigger::EditMode_Create(MOPReader & reader)
{
	return true;
}


MOP_BEGINLISTCG(LoadingTrigger, "Loading trigger", '1.00', 0, "Activete when selected mission loading done", "Managment")
	MOP_STRING("Mission", "")	
	MOP_BOOL("Active", true)
	MOP_MISSIONTRIGGERC("Event", "End command up/down trigger")
MOP_ENDLIST(LoadingTrigger)

