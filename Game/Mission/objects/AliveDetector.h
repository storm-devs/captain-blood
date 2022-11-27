//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// Mission objects
//===========================================================================================================================
// AliveDetector	
//============================================================================================

#ifndef _AliveDetector_h_
#define _AliveDetector_h_

#include "..\..\..\Common_h\Mission\Mission.h"

class AliveDetector : public DetectorObject
{
	struct AIElement
	{
		MOSafePointer object;	//Указатель на объект за которым следим
		bool isLive;			//Отменим что объект был жив и за ним можно начать наблюдение
		bool isDead;			//Было событие смерти
	};

//--------------------------------------------------------------------------------------------
public:
	AliveDetector();
	virtual ~AliveDetector();

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
	//Проверить жив или нет
	bool IsAlive(MissionObject * obj);


	//Пересоздать объект
	virtual void Restart()
	{
		ReCreate();
	};

//--------------------------------------------------------------------------------------------
protected:
	array<AIElement> aiObjects;
	MissionTrigger event;
	float time, delayTime;
	bool watchDieFact;
	bool deadState;
	bool inverseLogic;
};

#endif

