//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// Mission objects
//===========================================================================================================================
// LogicSwitcher	
//============================================================================================

#ifndef _LogicSwitcher_h_
#define _LogicSwitcher_h_

#include "..\..\..\Common_h\MISSION\Mission.h"

class LogicSwitcher : public MissionObject
{
	struct Input
	{
		ConstString name;
		bool state;
	};

//--------------------------------------------------------------------------------------------
public:
	LogicSwitcher();
	virtual ~LogicSwitcher();


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
	//Входные тригерры
	array<Input> input;
	bool andLogic;
	//Выходной триггер
	MissionTrigger event;

public:
	static const char * comment;
	static const char * triggerDesc;
};

#endif

