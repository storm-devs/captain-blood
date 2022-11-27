//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// Mission objects
//===========================================================================================================================
// RandomSelector
//============================================================================================

#ifndef _RandomSelector_h_
#define _RandomSelector_h_

#include "..\..\..\Common_h\mission\mission.h"

class RandomSelector : public MissionObject
{
	//--------------------------------------------------------------------------------------------
public:
	RandomSelector();
	virtual ~RandomSelector();


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

	//Пересоздать объект
	virtual void Restart()
	{
		ReCreate();
	};

	//--------------------------------------------------------------------------------------------
private:
	//Триггера
	array<MissionTrigger> events;
};

#endif

