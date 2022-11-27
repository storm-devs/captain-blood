//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// Mission objects
//===========================================================================================================================
// RectDetector	
//============================================================================================

#ifndef _RectDetector_h_
#define _RectDetector_h_

#include "..\..\..\Common_h\Mission\Mission.h"

class RectDetector : public DetectorObject  
{
//--------------------------------------------------------------------------------------------
public:
	RectDetector();
	virtual ~RectDetector();

//--------------------------------------------------------------------------------------------
public:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Получить матрицу объекта
	virtual Matrix & GetMatrix(Matrix & mtx);
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
	//Активация детектора
	virtual void ActivateDetector(const char * initiatorID);

//--------------------------------------------------------------------------------------------
protected:
	//Работа детектора
	void _cdecl Work(float dltTime, long level);
	//Работа детектора в режиме редактирования
	void _cdecl EditModeWork(float dltTime, long level);
	//Обновить цель
	bool UpdateTarget();

	//Пересоздать объект
	virtual void Restart()
	{
		ReCreate();
	};

//--------------------------------------------------------------------------------------------
protected:
	Plane plane;
	Matrix mtx, imtx;
	float width05, height05;
	ConstString targetID;
	MOSafePointer target;
	Vector targetPos;
	bool isDoubleSide;
	MissionTrigger event;
	float highlight;
	float resetTime;
	float ignoreDistance;
};

#endif

