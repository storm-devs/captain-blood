//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// TrackCamera
//============================================================================================

#ifndef _TrackCamera_h_
#define _TrackCamera_h_

#include "..\..\..\common_h\mission\Mission.h"
#include "PathTracks.h"
#include "..\camera_inc.h"

#define MOP_TRACK_CAM_FOVNEAR  MOP_FLOATEXC("Camera FOV near", 1.25f*180.0f/PI, 10.0f, 140.0f, "Field of veiw in degrees for near distance")
#define MOP_TRACK_CAM_FOVNDIST MOP_FLOATEXC("Camera FOV near distance", 10.0f, 0.0f, 1000000.0f, "Near distance from camera to target for FOV near")
#define MOP_TRACK_CAM_FOVFAR   MOP_FLOATEXC("Camera FOV far", 0.8f*180.0f/PI, 10.0f, 140.0f, "Field of veiw in degrees for far distance")
#define MOP_TRACK_CAM_FOVFDIST MOP_FLOATEXC("Camera FOV far distance", 50.0f, 0.0f, 1000000.0f, "Far distance from camera to target for FOV far")
#define MOP_TRACK_CAM_DISTFACT MOP_FLOATEXC("Distance factor", 1.0f, 0.01f, 100.0f, "FOV blend curve from near to far. 1 - linear, less 1 approach to far value, above 1 approach to near")
#define MOP_TRACK_CAM_TRACETRG MOP_BOOLC("Trace target", true, "If true then ignore camera angles and look by target")
#define MOP_TRACK_CAM_OFFSET   MOP_POSITIONC("Track offset", Vector(0.0f, 0.0f, 0.0f), "Offset vector of track points")
#define MOP_TRACK_CAM_NORM     MOP_BOOLC("Normalize", true, "Normalize UpVector")

#define MOP_TRACK_CAM	MOP_TRACK_CAM_FOVNEAR  \
						MOP_TRACK_CAM_FOVNDIST \
						MOP_TRACK_CAM_FOVFAR   \
						MOP_TRACK_CAM_FOVFDIST \
						MOP_TRACK_CAM_DISTFACT \
						MOP_TRACK_CAM_TRACETRG \
						MOP_TRACK_CAM_OFFSET \
						MOP_TRACK_CAM_NORM


class TrackCamera : public BaseCamera
{
//--------------------------------------------------------------------------------------------
public:	
	virtual ~TrackCamera();

//--------------------------------------------------------------------------------------------
public:

	void InitParams();

	//Прочитать параметры идущие перед стандартных
	virtual void ReadTrack(MOPReader & reader);

	//Прочитать параметры идущие перед стандартных
	virtual bool CameraPreCreate(MOPReader & reader);	

//--------------------------------------------------------------------------------------------
protected:

	//Работа
	void WorkUpdate(float dltTime);		

	//Получить позицию камеры
	virtual void Reset();
	//Получить позицию камеры
	virtual void GetCameraPosition(Vector & position);	

	//Нарисовать дополнительную информацию нри селекте
	virtual void SelectedDraw(float dltTime);

	bool GetCameraTarget(const Vector & position, Vector & target_pos, Vector & up);

	//Отрисовка в выделеном режиме
	virtual void _cdecl DebugDraw(float dltTime, long level);
//--------------------------------------------------------------------------------------------
public:
	
	PathTracks * track;	
	Vector pos;
	Quaternion ang;
	Vector chrPos;
	bool isMoveToPos;

private:

	long currentIndex;

	float fovNear;
	float fovNearDist;
	float fovFar;
	float fovFarDist;

	float distFactor;	
	float previewIndex;	
	bool traceTarget;	

	float CamBlendK;

	bool  bNormalize;
};

#endif

