/*


//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// AniCamera	
//============================================================================================

#ifndef _AniCamera_h_
#define _AniCamera_h_

#include "..\..\..\common_h\mission\Mission.h"
#include "..\TrackCamera\CameraTracksFile.h"
#include "..\camera_inc.h"


class AniCamera : public BaseCamera
{
	struct Point
	{
		Quaternion q;
		Vector p;
	};

	struct FrameEvent
	{
		MissionTrigger trigger;
		long frame;
	};

//--------------------------------------------------------------------------------------------
public:
	AniCamera();
	virtual ~AniCamera();

	virtual void InitParams();

	//--------------------------------------------------------------------------------------------
	//Параметры камеры
	//--------------------------------------------------------------------------------------------
	//Получить позицию камеры
	virtual void GetCameraPosition(Vector & position);
	//Получить точку наблюдения камеры
	virtual bool GetCameraTarget(const Vector & position, Vector & target_pos, Vector & up);

	//--------------------------------------------------------------------------------------------
	//Работа с камерой
	//--------------------------------------------------------------------------------------------
	//Прочитать параметры идущие перед стандартным
	virtual bool CameraPreCreate(MOPReader & reader);
	//Прочитать параметры идущие после стандартных
	virtual bool CameraPostCreate(MOPReader & reader);
	//Сбросить состояние камеры в исходное
	virtual void Reset();
	//Обновить состояние камеры
	virtual void WorkUpdate(float dltTime);
	//Обновить состояние для предпросмотра
	virtual void SelectedUpdate(float dltTime);
	//Нарисовать дополнительную информацию нри селекте
	virtual void SelectedDraw(float dltTime);

	//Нарисовать дополнительную информацию нри селекте
	void DrawFrame();

	//Получить текущую позицию камеру
	float GetCurrentPosition(long & i1, long & i2);
	//Загрузить трек
	//bool LoadTrackCTF(const char * name);
	bool LoadTrackANT(const char * name);


//--------------------------------------------------------------------------------------------
private:
	Point * track;						//Трек позиций камеры
	long numPoints;						//Количество точек в треке
	float curPosition;					//Текущая позиция на треке
	float speed;						//Скорость движения по треку
	float teleDist;						//Пороговая дистанция телепорта
	float teleCosAng;					//Пороговый косинус угла для телепорта
	MissionTrigger event;				//Событие
	array<FrameEvent> events;			//Покадровые события
	Matrix world;						//Матрица преобразования в мировую систему
};

#endif

*/