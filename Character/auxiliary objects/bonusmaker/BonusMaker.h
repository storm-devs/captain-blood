#ifndef BonusMaker_H
#define BonusMaker_H

#include "..\..\..\Common_h\Mission.h"

class BonusMaker : public MissionObject
{
	float startedBonusTime;
public:	

	array<ConstString> tables;
	array<Vector> spawn_points;

	BonusMaker();
	~BonusMaker();

	//Создание объекта
	virtual bool Create(MOPReader & reader);
	//Обновление параметров
	virtual bool EditMode_Update(MOPReader & reader);

	virtual void Activate(bool isActive);	

	static const char * comment;
};

#endif