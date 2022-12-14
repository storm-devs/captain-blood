//===========================================================================================================================
// Spirenkov Maxim, 2004
//===========================================================================================================================//	
// 
//===========================================================================================================================
// MissionFixedCamera
//============================================================================================

#ifndef _MissionFixedCamera_h_
#define _MissionFixedCamera_h_

#include "..\..\..\Common_h\Mission\Mission.h"
#include "..\camera_inc.h"

class MissionFixedCamera : public BaseCamera
{
//--------------------------------------------------------------------------------------------
private:
	//Прочитать параметры идущие после стандартных
	virtual bool CameraPreCreate(MOPReader & reader);
	//Получить позицию камеры
	virtual void GetCameraPosition(Vector & position);


//--------------------------------------------------------------------------------------------
private:
	Vector dir;							//Направление смещения камеры
};

#endif