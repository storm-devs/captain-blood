//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// MissionStaticCamera
//============================================================================================

#include "StaticCamera.h"

//============================================================================================
//MissionStaticCamera
//============================================================================================

//Прочитать параметры идущие после стандартных
bool MissionStaticCamera::CameraPreCreate(MOPReader & reader)
{
	pos = reader.Position();
	trg = pos + Matrix(reader.Angles()).vz;
	return true;
}

//Прочитать параметры идущие после стандартных
bool MissionStaticCamera::CameraPostCreate(MOPReader & reader)
{
	connect = reader.Bool();
	return true;
}

//Получить позицию камеры
void MissionStaticCamera::GetCameraPosition(Vector & position)
{
	if(!connect)
	{
		position = pos;
	}else{
		if(target.Validate())
		{
			position = target.Ptr()->GetMatrix(Matrix())*pos;
		}else{
			position = pos;
		}
	}
}

//Получить точку наблюдения камеры
bool MissionStaticCamera::GetCameraTarget(const Vector & position, Vector & target_pos, Vector & up)
{
	if(BaseCamera::GetCameraTarget(position, target_pos, up)) return true;
	up = Vector(0.0f, 1.0f, 0.0f);
	target_pos = trg;
	return true;
}



MOP_BEGINLISTCG(MissionStaticCamera, "Static camera", '1.00', 0, CAMERA_COMMENT("Static camera\n\n"), "Cameras")
	MOP_POSITIONC("Position", Vector(0.0f, 0.0f, 0.0f), "Camera position")
	//MOP_ANGLESEXC("Angles", Vector(0.6f, 0.0f, 0.0f), Vector(-PI*0.49f, -1000.0f, 0.0f), Vector(PI*0.49f, 1000.0f, 0.0f), "Camera orientation angles")
	MOP_ANGLESC("Angles", Vector(0.6f, 0.0f, 0.0f), "Camera orientation angles")
	MOP_CAM_STD
	MOP_BOOL("Connect to target", false)
MOP_ENDLIST(MissionStaticCamera)

