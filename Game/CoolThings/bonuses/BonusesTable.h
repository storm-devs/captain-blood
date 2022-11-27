

#ifndef _BonusesTable_h_
#define _BonusesTable_h_


#include "..\..\..\Common_h\bonuses.h"


class BonusesTable : public MissionObject
{
	struct Desc
	{
		MOSafePointerType<BonusBase> bonus;		//Указатель на бонус
		//long hashId;			//Число для быстрой проверки
		float hiP;				//Ненормализованая вероятность для верхнего порога 0..1
		float hiLevel;			//Верхний порог
		float loP;				//Ненормализованая вероятность для нижнего порога 0..1
		float loLevel;			//Нижний порог
		float p;				//Текущая вероятность
		bool isUnique;			//Уникальный или нет бонус
		bool select;			//Выбранный бонус
	};


public:
	BonusesTable();
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Обработчик команд для объекта
	virtual void Command(const char * id, dword numParams, const char ** params);
	//Сбор остальных таблиц
	void _cdecl PostInit(float dltTime, long level);
	//Выбрать бонус из таблицы
	bool SelectBonus(array<BonusBase *> & bonuses);

	MO_IS_FUNCTION(BonusesTable, MissionObject);


private:
	array<Desc> descs;	//Описания бонусов
};


#endif
