
#ifndef _LiveWatcher_h_
#define _LiveWatcher_h_

#include "..\..\..\Common_h\Mission\Mission.h"

class LiveWatcher : public MissionObject
{
	//--------------------------------------------------------------------------------------------
public:
	LiveWatcher();
	virtual ~LiveWatcher();

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

	//--------------------------------------------------------------------------------------------
private:
	//Работа детектора
	void _cdecl Work(float dltTime, long level);

	//Пересоздать объект
	virtual void Restart()
	{
		ReCreate();
	};

	//--------------------------------------------------------------------------------------------
protected:
	MOSafePointer object;		//Указатель на объект за которым следим
	float triggerHP;			//Уровень когда срабатывает триггер
	MissionTrigger trigger;		//Тригер
};

#endif

