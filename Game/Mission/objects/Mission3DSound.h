//===========================================================================================================================
// Spirenkov Maxim, 2004
//===========================================================================================================================
// Mission objects
//===========================================================================================================================
// Mission3DSound
//===========================================================================================================================

#ifndef _Mission3DSound_h_
#define _Mission3DSound_h_

#include "MissionSoundBase.h"


class Mission3DSound : public MissionSoundBase
{


	//--------------------------------------------------------------------------------------------
public:
	Mission3DSound();
	virtual ~Mission3DSound();


	//--------------------------------------------------------------------------------------------
public:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Получить матрицу объекта
	virtual Matrix & GetMatrix(Matrix & mtx);
	//Обработчик команд для объекта
	virtual void Command(const char * id, dword numParams, const char ** params);

	//--------------------------------------------------------------------
public:
	//Инициализировать объект
	virtual bool EditMode_Create(MOPReader & reader);
	//Обновить параметры
	virtual bool EditMode_Update(MOPReader & reader);
	//Получить размеры описывающего ящика
	virtual void EditMode_GetSelectBox(Vector & min, Vector & max);
	//Рисование модельки в режиме редактирования
	void _cdecl EditModeDraw(float dltTime, long level);

	//--------------------------------------------------------------------------------------------
private:
	Vector position;			//Позиция звука
	IGMXScene * model;			//Моделька отображающая звук
	float angle;				//Угол поворота модельки
};

#endif

