//============================================================================================
// Spirenkov Maxim, 2005
//============================================================================================
// Mission objects
//============================================================================================
// Balancer
//============================================================================================

#ifndef _Balancer_h_
#define _Balancer_h_

#include "..\..\..\Common_h\mission\mission.h"

class Balancer : public MissionObject
{
	struct LevelTrigger
	{
		long level;
		MissionTrigger incCounter;
		MissionTrigger decCounter;
	};


	//--------------------------------------------------------------------------------------------
public:
	Balancer();
	virtual ~Balancer();


	//--------------------------------------------------------------------------------------------
public:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Обработчик команд для объекта
	virtual void Command(const char * id, dword numParams, const char ** params);

	//--------------------------------------------------------------------
public:
	//Инициализировать объект
	virtual bool EditMode_Create(MOPReader & reader);
	//Обновить параметры
	virtual bool EditMode_Update(MOPReader & reader);
	//Получить размеры описывающего ящика
	virtual void EditMode_GetSelectBox(Vector & min, Vector & max);

	//Пересоздать объект
	virtual void Restart()
	{
		ReCreate();
	};

	//--------------------------------------------------------------------------------------------
private:
	void CheckEvent(bool isUp);


	//--------------------------------------------------------------------------------------------
private:
	//Уровень
	long level;
	//Ограничение
	long minLevel;
	long maxLevel;
	//Уровни
	array<LevelTrigger> levels;

public:
	static const char * comment;
};

#endif

