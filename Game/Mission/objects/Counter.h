//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// Mission objects
//===========================================================================================================================
// Counter	
//============================================================================================

#ifndef _Counter_h_
#define _Counter_h_

#include "..\..\..\Common_h\MISSION\Mission.h"

class Counter : public MissionObject
{
//--------------------------------------------------------------------------------------------
public:
	Counter();
	virtual ~Counter();


//--------------------------------------------------------------------------------------------
public:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Обработчик команд для объекта
	virtual void Command(const char * id, dword numParams, const char ** params);
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

	//Пересоздать объект
	virtual void Restart()
	{
		ReCreate();
	};

//--------------------------------------------------------------------------------------------
private:
	long counter;
	long maxCount;	
	bool lastTick;
	MissionTrigger eventTick;
	MissionTrigger eventCount;

public:
	static const char * comment;
};

#endif

