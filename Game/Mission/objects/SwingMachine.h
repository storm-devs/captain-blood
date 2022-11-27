//============================================================================================
// Spirenkov Maxim, 2003
//============================================================================================
// Mission objects
//============================================================================================
// SwingMachine	
//============================================================================================

#ifndef _SwingMachine_h_
#define _SwingMachine_h_

#include "..\..\..\Common_h\Mission.h"

class SwingMachine : public MissionObject
{
//--------------------------------------------------------------------------------------------
public:
	SwingMachine();
	virtual ~SwingMachine();

//--------------------------------------------------------------------------------------------
protected:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Активировать/деактивировать объект
	virtual void Activate(bool isActive);

//--------------------------------------------------------------------
protected:
	//Инициализировать объект
	virtual bool EditMode_Create(MOPReader & reader);
	//Инициализировать объект
	virtual bool EditMode_Update(MOPReader & reader);
	//Получить матрицу объекта
	virtual Matrix & GetMatrix(Matrix & mtx);

//--------------------------------------------------------------------------------------------
protected:
	void _cdecl Work(float dltTime, long level);
	void _cdecl EditMode_Work(float dltTime, long level);

protected:
	float angX, angZ;
	float spdX, spdZ;
	float ampX, ampZ;
	Matrix matrix;
	bool isGlobalSet;	
};

#endif

