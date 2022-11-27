#pragma once

#include "..\..\..\common_h\ICameraShocker.h"

class CameraShocker : public ICameraShocker
{
public:
	CameraShocker();
	virtual ~CameraShocker();

	MO_IS_FUNCTION(CameraShocker, MissionObject);

	virtual void Activate(bool isActive);

	virtual bool Create(MOPReader & reader);
	virtual bool EditMode_Update(MOPReader & reader);
	virtual void Command(const char * id, dword numParams, const char ** params);
	virtual void PostCreate();

	virtual void SetMatrix(const Matrix & mtx);

private:
	enum ShockType
	{
		shockRND = 0,
		shockRND1,
		shockRND2,
		shockRND3,
		shockRND4,
		shockRND5,
		shockRND_H,
		shockRND_V,
		shockPolar1,
		shockPolar2,
		shockPolar3,
		shockPolar4,
		shockPolar5,
		shockPolar,
		shockPolar_V,
		shockPolar_H,
		shockReset,
		shockLast,
		ShockType_ForceDword = 0x7FFFFFFF
	};

	enum FadingFrom
	{
		fromCamera = 0,
		fromMissionObject,
		from_ForceDword = 0x7FFFFFFF
	};

	ShockType shockType;

	bool useFading;
	ConstString moName;
	MOSafePointer moObject;

	float min_distance;
	float min_amplitude;
	float min_time;
	float min_intensity;

	float max_distance;
	float max_amplitude;
	float max_time;
	float max_intensity;

	char strAmplitude[12];
	char strTime[12];
	char strIntensity[12];

	char * shockParams[3];

	Vector shockPosition;
	float distanceDelta;

	void PrepareParams();

	FadingFrom fadingFrom;

public:
	struct ShockTypes
	{
		ConstString name;
		ShockType type;
	};

	static ShockTypes shockTypes[];
};