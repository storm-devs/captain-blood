#pragma once

#include "Mission.h"

class ISlowmo : public MissionObject
{
public:
	enum SlowmoType
	{
		circle = 0,
		sector,
		slowmotype_forcedword = 0x7FFFFFFF
	};

	struct SlowmoParams
	{
		ConstString name;
		ConstString locator;
		SlowmoType type;
		//char timeScale[8];
		float timeScale;
		float sectorDirection;
		float sectorWidth;
		float radius;
		float damage;				// Damage 
		float duration;				// Slowmo duration
		float probability;			// Probability per unit
		float delay;				// Delay for slowmo starting
	};

	MO_IS_FUNCTION(Slowmo, MissionObject);

	virtual const SlowmoParams * GetSlowmo(const ConstString & name) const = null;

protected:
	ISlowmo() {}
};
