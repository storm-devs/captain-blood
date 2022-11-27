//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// Mission objects
//===========================================================================================================================
// ConvexAreaDetector	
//============================================================================================

#ifndef _ConvexAreaDetector_h_
#define _ConvexAreaDetector_h_

#include "..\..\..\Common_h\Mission\Mission.h"

class ConvexAreaDetector : public MissionObject
{
	struct Target
	{
		MOSafePointer target;
		ConstString targetID;
		bool isInside;
		MissionTrigger eventEnter;
		MissionTrigger eventExit;
		bool actEnter;
		bool actExit;
	};

	struct Vertex
	{
		Vector pos;
		DColor color;
	};


//--------------------------------------------------------------------------------------------
public:
	ConvexAreaDetector();
	virtual ~ConvexAreaDetector();

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

//--------------------------------------------------------------------------------------------
private:
	//Работа детектора
	void _cdecl Work(float dltTime, long level);
	//Работа детектора в режиме редактирования
	void _cdecl EditModeWork(float dltTime, long level);

	//Пересоздать объект
	virtual void Restart()
	{
		ReCreate();
	};


//--------------------------------------------------------------------------------------------
private:
	array<Target> targets;
	Matrix matrix;
	ConstString connectID;	
	MOSafePointer connect;
	MissionTrigger eventEnter;
	MissionTrigger eventExit;
	bool isCheckVisible;
	bool isCheckActive;
	bool thisTriggers;
	bool isDebug;
	Plane plane[16];
	long planesCount;
	Vector center;
	float hmin, hmax;
	Vertex * previewPoints;
	long pCount;
};

#endif

