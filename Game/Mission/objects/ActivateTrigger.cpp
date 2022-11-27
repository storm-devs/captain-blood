//============================================================================================
// Spirenkov Maxim, 2006
//============================================================================================
// ActivateTrigger
//============================================================================================

#include "ActivateTrigger.h"

ActivateTrigger::ActivateTrigger()
{
}

ActivateTrigger::~ActivateTrigger()
{
}

//Инициализировать объект
bool ActivateTrigger::Create(MOPReader & reader)
{
	Activate(reader.Bool());
	activateTrigger.Init(reader);
	deactivateTrigger.Init(reader);
	return true;
}

//Активировать/деактивировать объект
void ActivateTrigger::Activate(bool isActive)
{
	MissionObject::Activate(isActive);
	if(isActive)
	{
		Registry(ACTIVATE_EVENT_GROUP, &ActivateTrigger::ActivateEvent, 0);
		Registry(DEACTIVATE_EVENT_GROUP, &ActivateTrigger::DeactivateEvent, 0);
	}else{
		Unregistry(ACTIVATE_EVENT_GROUP);
		Unregistry(DEACTIVATE_EVENT_GROUP);
	}
}

//Исполнить событие
void _cdecl ActivateTrigger::Action(float dltTime, long level)
{
	DelUpdate(&ActivateTrigger::Action);
	if(!IsActive()) return;
	activateTrigger.Activate(Mission(), false);
}

//Выполнить событие активации
void _cdecl ActivateTrigger::ActivateEvent(const char * group, MissionObject * sender)
{
	SetUpdate(&ActivateTrigger::Action, ML_ACTIVATE_TRIGGER);
}

//Выполнить событие деактивации
void _cdecl ActivateTrigger::DeactivateEvent(const char * group, MissionObject * sender)
{
	deactivateTrigger.Activate(Mission(), false);
}

//Пересоздать объект
void ActivateTrigger::Restart()
{
	DelUpdate();
	Unregistry();
	ReCreate();
};

//Инициализировать объект
bool ActivateTrigger::EditMode_Create(MOPReader & reader)
{
	return true;
}


MOP_BEGINLISTCG(ActivateTrigger, "Activate trigger", '1.00', 0, "Mission activate/deactivate trigger", "Managment")
	MOP_BOOL("Active", true)
	MOP_MISSIONTRIGGERC("Activate", "Activate mission")
	MOP_MISSIONTRIGGERC("Deactivate", "Deactivate mission")
MOP_ENDLIST(ActivateTrigger)

