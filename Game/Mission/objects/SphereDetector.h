//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// Mission objects
//===========================================================================================================================
// SphereDetector	
//============================================================================================

#ifndef _SphereDetector_h_
#define _SphereDetector_h_

#include "..\..\..\Common_h\Mission\Mission.h"

class SphereDetector : public DetectorObject  
{
//--------------------------------------------------------------------------------------------
public:
	SphereDetector();
	virtual ~SphereDetector();

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
private:
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
private:
	Sphere sphere;
	ConstString targetID;
	MOSafePointer target;
	Vector targetPos;
	MissionTrigger event;
	float highlight;
	float resetTime;
	ConstString connectID;
	MOSafePointer connect;
	Vector iniPos;
};

#endif

