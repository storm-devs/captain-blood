#pragma once

#include "..\..\..\common_h\Mission.h"
#include "..\..\..\common_h\ICameraShocker.h"

class BombExplosionPattern : public MissionObject
{
public:
	BombExplosionPattern();
	virtual ~BombExplosionPattern();

	virtual bool Create(MOPReader & reader);
	virtual bool EditMode_Update(MOPReader & reader);
	virtual void PostCreate();

	virtual void Command(const char * id, dword numParams, const char ** params);

	void SetMatrix(Matrix & mtx);

	MO_IS_FUNCTION(BombExplosionPattern, MissionObject);

private:
	Vector position;

	ConstString particleName;
	ConstString soundName;
	ConstString cameraShockerName;
	ConstString connectTo;

	MOSafePointerType<ICameraShocker> cameraShockerMO;

	bool noSwing;

	float distanceBoom;

	MissionTrigger eventDistanceBoom;
	MissionTrigger eventCommonBoom;

public:
	static const char * BombExplosionPattern::comment;
};