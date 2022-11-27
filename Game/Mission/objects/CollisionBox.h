//============================================================================================
// Spirenkov Maxim, 2005
//============================================================================================
// Mission objects
//============================================================================================
// CollisionBox
//============================================================================================

#ifndef _MissionCollisionBox_h_
#define _MissionCollisionBox_h_


#include "..\Mission.h"


class MissionCollisionBox : public MissionObject
{
	//--------------------------------------------------------------------------------------------
public:
	MissionCollisionBox();
	~MissionCollisionBox();

	//--------------------------------------------------------------------------------------------
public:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Пересоздать объект
	virtual void Restart();
	//Обработчик команд для объекта
	virtual void Command(const char * id, dword numParams, const char ** params);
	//Инициализировать объект
	virtual bool EditMode_Create(MOPReader & reader);
	//Обновить параметры
	virtual bool EditMode_Update(MOPReader & reader);
	//Получить размеры описывающего ящика
	virtual void EditMode_GetSelectBox(Vector & min, Vector & max);
	//Получить бокс, описывающий объект в локальных координатах
	virtual void GetBox(Vector & min, Vector & max);
	//Получить матрицу объекта
	virtual Matrix & GetMatrix(Matrix & mtx);
	//Нарисовать модельку
	void _cdecl Draw(float dltTime, long level);
	//Активировать/деактивировать объект
	virtual void Activate(bool isActive);

private:
	void InitParams(MOPReader & reader);
	void AddCollider(PhysicsCollisionGroup group);
	void DelAllColliders();

private:

	array<IPhysBox*> colliders;

	IMissionQTObject * aiColider;			//Препятствие для AI
	bool needAiRegistry;
};

#endif











