#ifndef WHRFOG_HPP
#define WHRFOG_HPP

#include "..\..\..\Common_h\Mission.h"

class Fog : public MissionObject
{
public:
	Fog();
	~Fog();

	bool Create(MOPReader & reader);
	bool EditMode_Create(MOPReader & reader);
	bool EditMode_Update(MOPReader & reader);

	MO_IS_FUNCTION(Fog, MissionObject);

	//Активировать/деактивировать объект
	virtual void Activate(bool isActive);

	//Установка тумана в основном цикле
	void _cdecl SetFog(float dltTime, long level);
	//Убирание тумана в основном цикле
	void _cdecl ResetFog(float dltTime, long level);

private:

	float fFogDensity;

	float fFogDistanceMin;
	float fFogDistanceMax;
	
	float fFogHeightDensity;
	float fFogHeightMax;
	float fFogHeightMin;

	


	Color cFogColor;
};

#endif