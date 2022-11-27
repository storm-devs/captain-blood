
#ifndef _NoFatality_h_
#define _NoFatality_h_


#include "..\..\..\Common_h\Mission.h"

#define NoFatality_GROUP GroupId('N','o','F','t')

class NoFatality : public MissionObject
{
	//--------------------------------------------------------------------------------------------

	struct Zone
	{
		Vector min;
		Vector max;
		Matrix mtx;
	};

public:

	NoFatality();
	~NoFatality();

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

public:
	//Проверить точку на попадание в зону
	bool IsInside(const Vector & pos);


private:
	void InitParams(MOPReader & reader);

private:
	array<Zone> zones;
};

#endif











