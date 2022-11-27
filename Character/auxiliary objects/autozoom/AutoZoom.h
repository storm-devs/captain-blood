
#ifndef _AutoZoom_h_
#define _AutoZoom_h_


#include "..\..\..\Common_h\Mission.h"
#include "..\..\..\Common_h\ICameraController.h"
#include "..\..\utils\SafePointerEx.h"

class AutoZoom : public MissionObject
{	
public:

	AutoZoom();	

	float fLastFov;
	float lst_fov, lst_aspect;

	float lerp_speed;

	float ang_speed;

	Vector last_pl_pos;

	ICameraController* camcontroller;

	//Инициализировать объект
	virtual bool Create(MOPReader & reader);	
	
	//Активировать/деактивировать объект
	virtual void Activate(bool isActive);
	//Работа
	void _cdecl Work(float dltTime, long level);	

	void SetLerpSpeed(float speed) { lerp_speed = speed; };
	void SetAngSpeed(float speed) { ang_speed = speed; };
};

#endif











