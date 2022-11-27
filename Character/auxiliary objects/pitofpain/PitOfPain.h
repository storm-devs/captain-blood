
#ifndef _PitOfPain_h_
#define _PitOfPain_h_


#include "..\..\..\Common_h\Mission.h"
#include "..\arbiter\CharactersArbiter.h"
#include "..\..\Character\Character.h"

#define AIZones_GROUP GroupId('A','I','Z','n')

class PitOfPain : public MissionObject
{
	struct Zone
	{
		Vector min;
		Vector max;
		Matrix mtx;
	};

	Zone zone;

	float fDamage;
	const char* reaction;
	
	MOSafePointer      ptr_arbiter;
	CharactersArbiter* arbiter;
	
	

	bool bReactOnPlayer;
	bool bReactOnAlly;
	bool bReactOnEnemy;
	bool bReactOnStatist;

	array <Character*> prisioners;

public:

	PitOfPain();

	//Инициализировать объект
	virtual bool Create(MOPReader & reader);	
	//Обновить параметры
	virtual bool EditMode_Update(MOPReader & reader);
	// Чтение параметров
	void InitParams(MOPReader & reader);	
	//Получить размеры описывающего ящика
	virtual void EditMode_GetSelectBox(Vector & min, Vector & max);
	//Получить матрицу объекта
	Matrix & GetMatrix(Matrix & mtx);
	//Нарисовать модельку
	void _cdecl Draw(float dltTime, long level);
	//Работа
	void _cdecl Work(float dltTime, long level);
	//Активировать/деактивировать объект
	virtual void Activate(bool isActive);

	virtual void Restart();

	//Проверить точку на попадание в зону
	bool IsInside(const Vector & pos);	
};

#endif











