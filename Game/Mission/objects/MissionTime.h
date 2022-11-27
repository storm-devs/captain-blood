//============================================================================================
// Spirenkov Maxim, 2006
//============================================================================================
// MissionTime
//============================================================================================

#ifndef _MissionTime_h_
#define _MissionTime_h_


#include "..\Mission.h"


class MissionTime : public MissionObject
{
public:
	MissionTime();
	virtual ~MissionTime();

	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Активировать/деактивировать объект
	virtual void Activate(bool isActive);
	//Обработчик команд для объекта
	virtual void Command(const char * id, dword numParams, const char ** params);
	//Исполниться
	void _cdecl Update(float dltTime, long level);
	//Пересоздать объект
	virtual void Restart();
	//Инициализировать объект
	virtual bool EditMode_Create(MOPReader & reader);

private:
	float timeScale;
	float oldTimeScale, newTimeScale;
	float speed, blendPosition;
	MissionTrigger trigger;

public:
	static const char * comment;
};

#endif

