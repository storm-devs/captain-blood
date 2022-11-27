


#ifndef _QTTest_h_
#define _QTTest_h_


#include "..\..\Mission.h"


class QTTest : public MissionObject
{
	struct Obj
	{		
		Vector pos;
		Vector vp;
		Vector ang;
		Vector va;
		dword color;
		IMissionQTObject * qt;		
	};
public:
	QTTest();
	virtual ~QTTest();
	//Инициализировать объект
	bool Create(MOPReader & reader);
	//Обработчик команд для объекта
	void Command(const char * id, dword numParams, const char ** params);
	//Инициализировать объект
	virtual bool EditMode_Create(MOPReader & reader);
	//Обновить параметры
	virtual bool EditMode_Update(MOPReader & reader);
	//Нарисовать модельку
	virtual void _cdecl Work(float dltTime, long level);

	//Объекты дерева, которые тестируем
	array<Obj> objects;

	Vector minAbb;
	Vector maxAbb;

	bool isSelfTest;
	bool isTreeDraw;
	GroupId qtGroupName;
	float levelScale;

	static const Vector minRange;
	static const Vector maxRange;
	static const GroupId group;
};



#endif

