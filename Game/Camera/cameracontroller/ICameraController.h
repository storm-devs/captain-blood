#ifndef ICAMERA_CONTROLLER
#define ICAMERA_CONTROLLER

#include "..\..\..\Common_h\Mission.h"

#define CAMERA_CONTROLLER_GROUP GroupId('C','M','C','R')

class ICameraController : public MissionObject
{			
public:

	virtual void GetTargetPoint(Vector& pt) = 0;
	virtual void GetBlenderMatrix(Matrix& shock, float fov) = 0;
	virtual bool AllowZooming()  = 0;	

	MO_IS_FUNCTION(CameraController, MissionObject);
};

#endif