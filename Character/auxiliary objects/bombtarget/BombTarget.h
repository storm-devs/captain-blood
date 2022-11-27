

#include "..\..\..\Common_h\mission.h"

class BombTarget : public MissionObject
{
public:
	BombTarget();
	virtual ~BombTarget();

	float fAimDistance;

public:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Инициализировать объект в режиме редактирования
	virtual bool EditMode_Create(MOPReader & reader);
	//Обновить параметры в режиме редактирования
	virtual bool EditMode_Update(MOPReader & reader);
	//Получить размеры описывающего ящика
	virtual void EditMode_GetSelectBox(Vector & min, Vector & max);

	//Активировать/деактивировать объект
	virtual void Activate(bool isActive);
	
	//Получить матрицу объекта
	virtual Matrix & GetMatrix(Matrix & mtx);


private:
	//Рисование точек
	void _cdecl Draw(float dltTime, long level);

private:
	Vector pos;
};

