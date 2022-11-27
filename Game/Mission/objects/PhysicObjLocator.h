//============================================================================================
// Spirenkov Maxim, 2007
//============================================================================================
// Mission objects
//============================================================================================
// PhysicObjLocator
//============================================================================================

#ifndef _PhysicObjLocator_h_
#define _PhysicObjLocator_h_

#include "..\..\..\Common_h\IMissionPhysObject.h"

class PhysicObjLocator : public MissionObject
{
//--------------------------------------------------------------------------------------------
public:
	PhysicObjLocator();
	~PhysicObjLocator();

//--------------------------------------------------------------------------------------------
public:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Инициализировать объект
	virtual bool EditMode_Create(MOPReader & reader);
	//Обновить параметры
	virtual bool EditMode_Update(MOPReader & reader);
	//Получить размеры описывающего ящика
	virtual void EditMode_GetSelectBox(Vector & min, Vector & max);
	//Получить матрицу объекта
	Matrix & GetMatrix(Matrix & mtx);
	//Нарисовать локатор
	void _cdecl EditMode_Draw(float dltTime, long level);

private:
	MOSafePointer po;
	ConstString objectName;
	ConstString locatorName;
	long locatorIndex;
	bool error;
};

#endif

