#ifndef ChestKeeper_H
#define ChestKeeper_H

#include "QuickEvent.h"

class ChestKeeper : public MissionObject
{
public:	
	
	MGIterator* iter;

	ChestKeeper();
	~ChestKeeper();

	//Создание объекта
	virtual bool Create(MOPReader & reader);
	//Обновление параметров
	virtual bool EditMode_Update(MOPReader & reader);

	virtual void Activate(bool isActive);
	virtual void Restart();	

	static const char * comment;
};

#endif