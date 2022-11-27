//============================================================================================
// Spirenkov Maxim, 2006
//============================================================================================
// ControlTrigger
//============================================================================================

#ifndef _ControlTrigger_h_
#define _ControlTrigger_h_

#include "..\MissionsManager.h"

class ControlTrigger : public DetectorObject
{
public:
	ControlTrigger();
	virtual ~ControlTrigger();

	//--------------------------------------------------------------------------------------------
public:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Активировать
	virtual void Activate(bool isActive);
	//Инициализировать объект
	virtual bool EditMode_Create(MOPReader & reader);

	//--------------------------------------------------------------------------------------------
private:
	//Работа детектора
	void _cdecl Work(float dltTime, long level);


	//Пересоздать объект
	virtual void Restart()
	{
		ReCreate();
	};

private:
	long contrrolCode;
	ControlStateType type;
	MissionTrigger trigger;
	bool isLookRepeat;
	float repeatWaitTime;
	float currentTime;
};

#endif
