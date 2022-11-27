
#ifndef _BombSafe_h_
#define _BombSafe_h_


#include "..\..\..\Common_h\Mission.h"

#define BombSafe_GROUP GroupId('B','S','f','e')

class BombSafe : public MissionObject
{
	//--------------------------------------------------------------------------------------------
public:
	BombSafe();
	~BombSafe();

	//--------------------------------------------------------------------------------------------
public:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);	
	//Обновить параметры
	virtual bool EditMode_Update(MOPReader & reader);
	//Получить размеры описывающего ящика
	virtual void EditMode_GetSelectBox(Vector & min, Vector & max);
	//Получить матрицу объекта
	Matrix & GetMatrix(Matrix & mtx);
	//Нарисовать модельку
	void _cdecl Draw(float dltTime, long level);
	//Активировать/деактивировать объект
	virtual void Activate(bool isActive);

private:
	void InitParams(MOPReader & reader);

public:
	Vector size;
	Matrix transform;
};

#endif











