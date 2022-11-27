//============================================================================================
// Spirenkov Maxim, 2007
//============================================================================================
// Mission objects
//============================================================================================
// MultiDelay
//============================================================================================

#ifndef _MultiDelay_h_
#define _MultiDelay_h_

#include "..\..\..\Common_h\MISSION\Mission.h"

class MultiDelay : public MissionObject
{
	struct Element
	{
		//Время ожидания
		float waitTime;
		float deltaTime;
		float initTime;
		//Триггер
		MissionTrigger event;
	};


	//--------------------------------------------------------------------------------------------
public:
	MultiDelay();
	virtual ~MultiDelay();


	//--------------------------------------------------------------------------------------------
public:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Активировать
	virtual void Activate(bool isActive);
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
	//Работа
	void _cdecl ParalelWork(float dltTime, long level);
	void _cdecl ChainWork(float dltTime, long level);

	//Пересоздать объект
	virtual void Restart();


	//--------------------------------------------------------------------------------------------
private:
	array<Element> elements;
	long currentIndex;
	bool isChain;
	bool isPause;
};

#endif

