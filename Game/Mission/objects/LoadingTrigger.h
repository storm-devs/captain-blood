//============================================================================================
// Spirenkov Maxim, 2006
//============================================================================================
// LoadingTrigger
//============================================================================================

#ifndef _LoadingTrigger_h_
#define _LoadingTrigger_h_


#include "..\MissionsManager.h"


class LoadingTrigger : public MissionObject
{
public:
	LoadingTrigger();
	virtual ~LoadingTrigger();

	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Активировать/деактивировать объект
	virtual void Activate(bool isActive);
	//Исполнить событие
	void _cdecl Work(float dltTime, long level);	
	//Выполнить событие активации
	void _cdecl Event(const char * group, MissionObject * sender);
	//Пересоздать объект
	virtual void Restart();
	//Инициализировать объект
	virtual bool EditMode_Create(MOPReader & reader);

private:
	const char * mission;
	MissionTrigger trigger;
};


#endif

