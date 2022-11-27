//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// MissionStarter
//============================================================================================

#ifndef _MissionStarter_h_
#define _MissionStarter_h_


#include "..\Mission.h"


class MissionStarter : public MissionObject
{
public:
	MissionStarter();
	virtual ~MissionStarter();

	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Вызываеться, когда все объекты созданны но ещё не началось исполнение миссии
	virtual void PostCreate();
	//Пересоздать объект
	virtual void Restart();
	//Отложенное срабатывание
	void _cdecl PostRestart(float dltTime, long level);

private:
	MissionTrigger trigger;
};

#endif