//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// Mission objects
//===========================================================================================================================
// ObjectsGroup
//============================================================================================

#ifndef _RectDetector_h_
#define _RectDetector_h_

#include "..\..\..\Common_h\Mission\Mission.h"

class ObjectsGroup : public MissionObject
{
	struct ObjectInfo
	{
		MOSafePointer object;
		ConstString id;
		bool changeShow;
		bool changeActive;
		bool recivingCommand;
	};

//--------------------------------------------------------------------------------------------
public:
	ObjectsGroup();
	virtual ~ObjectsGroup();

//--------------------------------------------------------------------------------------------
public:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Показать/скрыть объект
	virtual void Show(bool isShow);
	//Активировать/деактивировать объект
	virtual void Activate(bool isActive);
	//Распространить команду
	virtual void Command(const char * id, dword numParams, const char ** params);
	//Пересоздать объект
	virtual void Restart();	

//--------------------------------------------------------------------
public:
	//Инициализировать объект
	virtual bool EditMode_Create(MOPReader & reader);
	//Обновить параметры
	virtual bool EditMode_Update(MOPReader & reader);

private:
	//Обновить список объектов
	void UpdateObjectsList();

//--------------------------------------------------------------------------------------------
protected:
	array<ObjectInfo> objects;
	bool processActivate;
	bool processShow;
	bool processCommand;
	bool processRestart;
};

#endif

