#pragma once

#include "Mission.h"

class IFlysCloud
{
public:

	virtual void SetPosition(const Vector &pos) = 0;
	virtual void SetAlpha(float alpha) = 0;

	virtual void Release(bool isFade) = 0;

};

class IFlyManager : public MissionObject
{
public:

	virtual IFlysCloud *CreateFlys(float radius, dword count, bool playSound = true) = 0;

};
