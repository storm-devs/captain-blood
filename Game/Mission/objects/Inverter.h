//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// Mission objects
//===========================================================================================================================
// Inverter
//============================================================================================

#ifndef _Inverter_h_
#define _Inverter_h_

#include "RectDetector.h"

class Inverter : public RectDetector  
{
	struct LittleTrigger
	{
		bool show;
		bool changeShow;
		bool active;
		bool changeActive;
	};

	struct Command
	{
		const char * command;
		long paramsCount;
		const char * params[8];
	};

	struct Element
	{
		MOSafePointer pointer;
		ConstString objectId;
		LittleTrigger forward;
		LittleTrigger back;
		long commandsIndex;
		long commandsCount;
	};
//--------------------------------------------------------------------------------------------
public:
	Inverter();
	virtual ~Inverter();

//--------------------------------------------------------------------------------------------
public:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);

//--------------------------------------------------------------------
public:
	//Активация детектора
	virtual void ActivateDetector(const char * initiatorID);

//--------------------------------------------------------------------------------------------
private:
	//Обновить цель
	bool UpdateObject(Element & elm);

	//Пересоздать объект
	virtual void Restart()
	{
		ReCreate();
	};

//--------------------------------------------------------------------------------------------
protected:
	array<Element> objects;
	array<Command> commands;
};

#endif

