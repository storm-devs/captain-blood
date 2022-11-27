#pragma once

class IAchievement : public MissionObject
{
public:
	virtual void Increment(float count = 1.0f) = 0;
	virtual void Set(float count) = 0;
	virtual void SetZero() = 0;

	MO_IS_FUNCTION(IAchievement, MissionObject);

protected:
	IAchievement() {}
	virtual ~IAchievement() {}
};
