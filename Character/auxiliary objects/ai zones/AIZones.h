
#ifndef _AIZones_h_
#define _AIZones_h_


#include "..\..\..\Common_h\Mission.h"

#define AIZones_GROUP GroupId('A','I','Z','n')

class AIZones : public MissionObject
{	
public:

	struct Zone
	{
		Vector min;
		Vector max;
		Matrix mtx;

		bool  isSelected;
	};

	array<Zone> zones;

	AIZones();
	~AIZones();
	
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

	void ShowDebug(bool show);

public:
	//Проверить точку на попадание в зону
	bool IsInside(const Vector & pos,bool check_buffer = false);

private:
	void InitParams(MOPReader & reader);	
};

#endif











