#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "..\..\..\Common_h\Mission\Mission.h"
#include "..\..\..\Common_h\FreeCamera.h"

class FreeCameraModule
{
public:
	FreeCameraModule();
	virtual ~FreeCameraModule();
	bool Init();

public:
	//Перемещение камеры
	void Move(float dltTime);
	//Установить углы по точке
	bool SetTarget(float x, float y, float z);

public:
	IRender	* render;
	IControls * ctr;
	Vector pos, ang;
	Matrix swing;
	float fov;
	string pref;
	string ctrl;
	
};

class FreeCamera : public IFreeCamera
{
//--------------------------------------------------------------------------------------------
public:
	FreeCamera();
	virtual ~FreeCamera();

//--------------------------------------------------------------------------------------------
private:
	//Инициализация entity
	virtual bool Init();
	//Исполнение из слоя
	virtual void __fastcall Execute(float dltTime);

	//Установить новую позицию камеры
	virtual void SetPosition(const Vector & pos);
	//Получить позицию камеры
	virtual Vector GetPosition();
	//Установить углы
	virtual void SetAngles(const Vector & ang);
	//Получить углы
	virtual Vector GetAngles();
	//Соориентировать камеру на точку
	virtual void SetTarget(const Vector & pos);
	//Установить угол обзора
	virtual void SetFOV(float fov);
	//Получить угол обзора
	virtual float GetFOV();
	//Приостановить работу камеры
	virtual void Pause(bool isPause);

//--------------------------------------------------------------------------------------------
private:
	FreeCameraModule camera;
};

class MissionFreeCamera : public MissionObject
{
//--------------------------------------------------------------------------------------------
public:
	MissionFreeCamera();
	virtual ~MissionFreeCamera();
	
//--------------------------------------------------------------------------------------------
public:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Получить матрицу объекта
	virtual Matrix & GetMatrix(Matrix & mtx);

	//Инициализировать объект
	virtual bool EditMode_Create(MOPReader & reader);
	//Обновить параметры
	virtual bool EditMode_Update(MOPReader & reader);
	//Получить размеры описывающего ящика
	virtual void EditMode_GetSelectBox(Vector & min, Vector & max);

//--------------------------------------------------------------------------------------------
private:
	//Работа
	void _cdecl Work(float dltTime, long level);
	//Работа в режиме редактирования
	void _cdecl EditModeWork(float dltTime, long level);

//--------------------------------------------------------------------------------------------
private:
	FreeCameraModule camera;			//Камера
	IGMXScene * model;					//Моделька для редактирования
};

#endif