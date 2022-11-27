#ifndef IWEATHER_HPP
#define IWEATHER_HPP

class IWind : public MissionObject
{
public:
	IWind() {};
	~IWind() {};

	MO_IS_FUNCTION(IWind, MissionObject);

	virtual Vector GetAngle() = 0;
	virtual float GetSpeed() = 0;
	virtual float GetRelativeSpeed() = 0;
};

#define WIND_MODELING_LOCAL true


#endif