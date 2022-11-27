#pragma once

#define IForce_force_path	"Resource\\Forces\\"

struct IForce
{
	virtual void Play() = 0;
	virtual void Stop() = 0;

	virtual float GetPosition() = 0;

	virtual void Release() = 0;
};

struct IForceService : public Service
{
	virtual IForce *Create(const char *profile) = 0;
	virtual IForce *Create(array<Vector> &ls, array<Vector> &rs) = 0;
};
