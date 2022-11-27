//============================================================================================
// Spirenkov Maxim, 2006
//============================================================================================
// ActivateTrigger
//============================================================================================

#ifndef _ActivateTrigger_h_
#define _ActivateTrigger_h_


#include "..\MissionsManager.h"


class ActivateTrigger : public MissionObject
{
public:
	ActivateTrigger();
	virtual ~ActivateTrigger();

	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Активировать/деактивировать объект
	virtual void Activate(bool isActive);
	//Исполнить событие
	void _cdecl Action(float dltTime, long level);	
	//Выполнить событие активации
	void _cdecl ActivateEvent(const char * group, MissionObject * sender);
	//Выполнить событие деактивации
	void _cdecl DeactivateEvent(const char * group, MissionObject * sender);
	//Пересоздать объект
	virtual void Restart();
	//Инициализировать объект
	virtual bool EditMode_Create(MOPReader & reader);

private:
	MissionTrigger activateTrigger;
	MissionTrigger deactivateTrigger;
};


#endif

