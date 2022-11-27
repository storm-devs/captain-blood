//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// Mission objects
//===========================================================================================================================
// AcceptDetector	
//============================================================================================

#ifndef _AcceptDetector_h_
#define _AcceptDetector_h_

#include "..\..\..\Common_h\Mission\Mission.h"

class AcceptDetector : public AcceptorObject
{
//--------------------------------------------------------------------------------------------
public:
	AcceptDetector();
	virtual ~AcceptDetector();

//--------------------------------------------------------------------------------------------
public:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Получить матрицу объекта
	virtual Matrix & GetMatrix(Matrix & mtx);
	//Активировать
	virtual void Activate(bool isActive);
	//Воздействовать на объект
	virtual bool Accept(MissionObject * obj);

//--------------------------------------------------------------------
public:
	//Инициализировать объект
	virtual bool EditMode_Create(MOPReader & reader);
	//Обновить параметры
	virtual bool EditMode_Update(MOPReader & reader);
	//Получить размеры описывающего ящика
	virtual void EditMode_GetSelectBox(Vector & min, Vector & max);

//--------------------------------------------------------------------------------------------
private:
	//Работа детектора
	void _cdecl Work(float dltTime, long level);
	//Работа детектора в режиме редактирования
	void _cdecl EditModeWork(float dltTime, long level);
	//Обновить цель
	bool UpdateTarget();
	//Обновить позицию
	void UpdatePosition();


	//Пересоздать объект
	virtual void Restart()
	{
		ReCreate();
	};


//--------------------------------------------------------------------------------------------
private:
	Sphere sphere;
	ConstString targetID;
	MOSafePointer target;
	Vector targetPos;
	bool isTrgInside;
	bool isSelected;
	bool checkDirection;
	bool acceptEnable;
	ConstString connectID;
	MOSafePointer connect;	
	Vector iniPos;
	MissionTrigger eventAccept;
	MissionTrigger eventEnter;
	MissionTrigger eventExit;
};

#endif

