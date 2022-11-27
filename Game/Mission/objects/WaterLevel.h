//============================================================================================
// Spirenkov Maxim, 2006
//============================================================================================
// Mission objects
//============================================================================================
// WaterLevel
//============================================================================================

#ifndef _WaterLevel_h_
#define _WaterLevel_h_

#include "..\..\..\Common_h\Mission.h"


class WaterLevelAccessor : public MissionObject
{
public:
	WaterLevelAccessor();
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Активировать
	virtual void Activate(bool isActive);
	//Привязка к объекту
	void _cdecl UpdateLevel(float dltTime, long level);

private:
	MOSafePointer ptr;
	float level;
	ConstString connectID;
	MOSafePointer connect;
};

class WaterLevel : public IWaterLevel
{
public:
	void SetLevel(float lvl);
};

#endif
