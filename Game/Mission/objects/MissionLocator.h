//============================================================================================
// Spirenkov Maxim, 2004
//============================================================================================
// Mission objects
//============================================================================================
// MissionLocator
//============================================================================================

#ifndef _MissionLocator_h_
#define _MissionLocator_h_

#include "..\Mission.h"
#include "..\..\..\Common_h\gmx.h"
#include "..\..\..\Common_h\AnimationStdEvents.h"

class MissionLocator : public MissionObject
{
//--------------------------------------------------------------------------------------------
public:
	MissionLocator();
	~MissionLocator();

//--------------------------------------------------------------------------------------------
public:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Инициализировать объект
	virtual bool EditMode_Create(MOPReader & reader);
	//Обновить параметры
	virtual bool EditMode_Update(MOPReader & reader);
	//Получить размеры описывающего ящика
	virtual void EditMode_GetSelectBox(Vector & min, Vector & max);
	//Получить матрицу объекта
	Matrix & GetMatrix(Matrix & mtx);
	//Нарисовать локатор
	void _cdecl Draw(float dltTime, long level);
	//Нарисовать локатор
	void _cdecl EditMode_Draw(float dltTime, long level);
	//Пересоздать объект
	virtual void Restart();	
	//Обработчик команд для объекта
	virtual void Command(const char * id, dword numParams, const char ** params);
	//Обновить позицию  и состояние звука
	void _cdecl UpdateSound(float dltTime, long level);
	//Удалить звук
	void ReleaseSound();

	MO_IS_FUNCTION(MissionLocator, MissionObject);

private:
	Matrix matrix;
	MOSafePointer objectPtr;
	ConstString connectToObject;
	ISound3D * sound;
};

#endif

