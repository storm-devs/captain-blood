//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// Mission objects
//===========================================================================================================================
// Delay	
//============================================================================================

#ifndef _Delay_h_
#define _Delay_h_

#include "..\..\..\Common_h\MISSION\Mission.h"

class Delay : public MissionObject
{
//--------------------------------------------------------------------------------------------
public:
	Delay();
	virtual ~Delay();


//--------------------------------------------------------------------------------------------
public:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Активировать
	virtual void Activate(bool isActive);
//--------------------------------------------------------------------
public:
	//Инициализировать объект
	virtual bool EditMode_Create(MOPReader & reader);
	//Обновить параметры
	virtual bool EditMode_Update(MOPReader & reader);
	//Получить размеры описывающего ящика
	virtual void EditMode_GetSelectBox(Vector & min, Vector & max);
	//Работа
	void _cdecl Work(float dltTime, long level);

	//Пересоздать объект
	virtual void Restart()
	{
		ReCreate();
	};


//--------------------------------------------------------------------------------------------
private:
	//Время ожидания
	float waitTime;
	float deltaTime;
	float initTime;
	bool isInstant;
	bool isProcess;
	//Триггер
	MissionTrigger event;

};

#endif

