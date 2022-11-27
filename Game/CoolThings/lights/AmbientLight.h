//============================================================================================
// Spirenkov Maxim, 2004
//============================================================================================
// Mission objects
//============================================================================================
// MissionLocator
//============================================================================================

#ifndef _AmbientLight_h_
#define _AmbientLight_h_

#include "..\..\..\Common_h\Mission.h"
#include "..\..\..\Common_h\gmx.h"
#include "..\..\..\Common_h\AnimationStdEvents.h"

class AmbientLight : public MissionObject
{
//--------------------------------------------------------------------------------------------
public:
	AmbientLight();
	~AmbientLight();

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

	void Activate(bool isActive);

	//Установка источника в основном цикле
	void _cdecl SetLight(float dltTime, long level);

	//Выполнить событие активации
	void _cdecl ActivateEvent(const char * group, MissionObject * sender);

private:
	Matrix matrix;
	Color vAmbientColor;
	Color vAmbientColorSpecular;
	float fGamma;
};

#endif

