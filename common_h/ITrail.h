#pragma once

#include "Mission.h"

struct ITrail
{
	virtual void SetParams(float rBeg, float rEnd, float minDelay, float maxDelay,
		dword  color = 0xffffffff,
		float  offsetStrength = 1.0f,
		float  windVel = 0.0f,
		Vector windDir = Vector(0.0f,1.0f,0.0f),
		float  forceValue = 0.0f) = 0;

	virtual void EnableStaticLive(bool enable) = 0;

	virtual void Reset() = 0;

	virtual void Show(bool isShow) = 0;
	virtual void Update(const Matrix &m) = 0;

	virtual void Release() = 0;
};

struct ITrailManager : public MissionObject
{
	virtual ITrail *Add() = 0;
};
