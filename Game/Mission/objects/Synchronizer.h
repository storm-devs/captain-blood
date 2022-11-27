//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// Mission objects
//===========================================================================================================================
// Synchronizer	
//============================================================================================

#ifndef _Synchronizer_h_
#define _Synchronizer_h_

#include "..\..\..\Common_h\MISSION\Mission.h"

class Synchronizer : public MissionObject
{
	struct Event
	{
		ConstString name;			//Имя события
		MissionTrigger event;		//Событие
		bool state;					//Состояние
	};

public:

	typedef void (_cdecl Synchronizer::*LogicFunc)();

	struct Logic
	{
		ConstString name;
		LogicFunc logic;
	};

//--------------------------------------------------------------------------------------------
public:
	Synchronizer();
	virtual ~Synchronizer();


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

	//Логики работы
	void _cdecl LogicAll();
	void _cdecl LogicRand();
	void _cdecl LogicFirst();


	//Пересоздать объект
	virtual void Restart()
	{
		ReCreate();
	};
	

//--------------------------------------------------------------------------------------------
private:
	long logicIndex;		//Логика работы
	array<Event> events;	//События
	MissionTrigger tick;	//Событие тика

public:
	static const char * comment;
	static const char * desc;
	static const Logic logics[];
};

#endif

