//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// MissionStaticCamera
//============================================================================================

#ifndef _MissionStaticCamera_h_
#define _MissionStaticCamera_h_

#include "..\..\..\Common_h\Mission\Mission.h"
#include "..\camera_inc.h"

class MissionStaticCamera : public BaseCamera
{
//--------------------------------------------------------------------------------------------
private:
	//Прочитать параметры идущие после стандартных
	virtual bool CameraPreCreate(MOPReader & reader);
	//Получить позицию камеры
	virtual void GetCameraPosition(Vector & position);
	//Получить точку наблюдения камеры
	virtual bool GetCameraTarget(const Vector & position, Vector & target_pos, Vector & up);
	//Прочитать параметры идущие после стандартных
	virtual bool CameraPostCreate(MOPReader & reader);


//--------------------------------------------------------------------------------------------
private:
	Vector pos;		//Позиция камеры
	Vector trg;		//На кого смотрит камера
	bool connect;	//Прицепить камеру к цели
};

#endif