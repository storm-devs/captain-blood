#pragma once

#include "..\..\..\Common_h\Mission.h"

class TrailParams : public MissionObject
{
public:

	bool Create			(MOPReader &reader);
	bool EditMode_Update(MOPReader &reader);

private:

	void InitParams		(MOPReader &reader);

public:

	float updated;

	////////////////////////////

	bool use;

	float radBegin;
	float radEnd;

	float fadeMin;
	float fadeMax;

	dword color;

	float offStrength;

	Vector windDir;
	float  windVel;

	float forceValue;

	bool staticLive;

	////////////////////////////

	bool  ballRot;
	float ballRotSpeed;

};
