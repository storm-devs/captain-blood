//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// MissionFixedCamera
//============================================================================================

#include "FixedCamera.h"

//============================================================================================
//MissionFixedCamera
//============================================================================================

//Прочитать параметры идущие после стандартных
bool MissionFixedCamera::CameraPreCreate(MOPReader & reader)
{
	dir = Matrix(reader.Angles()).vz;
	float dist = reader.Float();
	dir = dir*dist;
	return true;
}

//Получить позицию камеры
void MissionFixedCamera::GetCameraPosition(Vector & position)
{
	if(!BaseCamera::GetCameraTarget(Vector(0.0f), position, Vector(0.0f, 1.0f, 0.0f)))
	{
		position = 0.0f;
		return;
	}
	position -= dir;
}

MOP_BEGINLISTCG(MissionFixedCamera, "Fixed view camera", '1.00', 0, CAMERA_COMMENT("Fixed view camera\n\n"), "Cameras")
	MOP_ANGLESC("Angles", Vector(0.6f, 0.0f, 0.0f), "Fixed camera's orientation angles")
	MOP_FLOATEXC("Distance", 3.0f, 0.5f, 195.0f, "Distance to target point in meters")
	MOP_CAM_STD
MOP_ENDLIST(MissionFixedCamera)

