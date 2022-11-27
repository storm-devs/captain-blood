#ifndef WIND_HPP
#define WIND_HPP

#include "..\..\..\Common_h\Mission.h"
#include "..\..\..\Common_h\Weather.h"

class Wind : public IWind
{
public:
	Wind();
	~Wind();

	MO_IS_FUNCTION(Wind, IWind);

	bool Create(MOPReader & reader);
	bool EditMode_Create(MOPReader & reader);
	bool EditMode_Update(MOPReader & reader);

	void _cdecl	Execute(float fDeltaTime, long level);

	virtual Vector GetAngle();
	virtual float GetSpeed();
	virtual float GetRelativeSpeed();

private:
	Vector vAngle;
	float fSpeed, fMaxSpeed;
};

#endif