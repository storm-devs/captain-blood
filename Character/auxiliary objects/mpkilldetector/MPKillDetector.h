#pragma once

#include "..\..\..\common_h\Mission.h"

class MPKillDetector : public MissionObject
{
public:
	MPKillDetector();
	virtual ~MPKillDetector();

	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Активировать
	virtual void Activate(bool isActive);

	//Инициализировать объект
	virtual bool EditMode_Create(MOPReader & reader);
	//Обновить параметры
	virtual bool EditMode_Update(MOPReader & reader);

private:
	//Работа детектора
	void _cdecl Work(float dltTime, long level);
	//Проверить жив или нет
	bool IsAlive();

	//Пересоздать объект
	virtual void Restart();

private:
	struct Activator
	{
		ConstString		object;
		bool			isActivate;
	};

	struct Killer
	{
		ConstString offender;
		array<Activator> objects;

		Killer() : objects(_FL_, 2) {}
	};
	array<Killer>	killers;
	array<Activator> unknownKiller;
	MOSafePointer object;	//Указатель на объект за которым следим
	bool isLive;			//Отменим что объект был жив и за ним можно начать наблюдение
	bool isDead;			//Было событие смерти

	void CreateKiller(MOPReader & reader, Killer & killer);
	void ActivateObjects(const array<Activator> & activator);
};