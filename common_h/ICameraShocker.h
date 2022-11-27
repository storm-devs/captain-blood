#pragma once

#include "Mission.h"

class ICameraShocker : public MissionObject
{
public:
	virtual void SetMatrix(const Matrix & mtx) = 0;

protected:
	ICameraShocker() {}
	virtual ~ICameraShocker() {}
};
