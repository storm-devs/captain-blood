
#ifndef _TrackCameraED_h_
#define _TrackCameraED_h_

#include "TrackCamera.h"


class TrackCameraED : public TrackCamera
{
public:

	//Прочитать параметры идущие перед стандартных
	virtual void ReadTrack(MOPReader & reader);
};

#endif

