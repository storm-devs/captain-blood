
#ifndef _BombBoom_h_
#define _BombBoom_h_


#include "..\..\..\Common_h\mission.h"

class BombBoom : public MissionObject
{
public:
	BombBoom();
	virtual ~BombBoom();

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

	virtual void Command(const char * id, dword numParams, const char ** params);

	//Обновить указатель на объект к которому прицепленны
	void UpdateConnectPointer();
	//Обновить позицию источника
	void _cdecl UpdateConnection(float dltTime, long level);

private:
	//Рисование точек
	void _cdecl Draw(float dltTime, long level);

	void Boom(const Vector & p);

private:
	Vector pos;
	float radius;
	float damage;
	float power;
	bool  bDamagePlayer;
	Matrix transform;

	//Объект к которому прицепляемся
	MOSafePointer connectObjectPtr;	
	ConstString connectToObject;
};

#endif