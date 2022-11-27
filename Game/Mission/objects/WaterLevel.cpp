//============================================================================================
// Spirenkov Maxim, 2006
//============================================================================================
// Mission objects
//============================================================================================
// WaterLevel
//============================================================================================

#include "WaterLevel.h"


WaterLevelAccessor::WaterLevelAccessor()
{
};

//Инициализировать объект
bool WaterLevelAccessor::Create(MOPReader & reader)
{
	level = reader.Float();
	bool activateState = reader.Bool();
	connectID = reader.String();
	Activate(activateState);
	return true;
}


//Активировать
void WaterLevelAccessor::Activate(bool isActive)
{
	static const GroupId waterLevelGroup = MG_WATERLEVEL;
	MissionObject::Activate(isActive);
	IWaterLevel::GetWaterLevel(Mission(), ptr);
	if(isActive)
	{
		Registry(waterLevelGroup);
		Assert(ptr.Validate());
		((WaterLevel *)ptr.Ptr())->SetLevel(level);
		if(connectID.NotEmpty())
		{
			UpdateLevel(0.0f, 0);
			SetUpdate(&WaterLevelAccessor::UpdateLevel, ML_EXECUTE9);
		}
	}else{
		DelUpdate();
		Unregistry(waterLevelGroup);
		MGIObject it(GroupIterator(waterLevelGroup, _FL_));
		if(it.IsDone())
		{
			((WaterLevel *)ptr.Ptr())->SetLevel(0.0f);
		}else{
			((WaterLevel *)ptr.Ptr())->SetLevel(((WaterLevelAccessor *)it.Get())->level);
		}
	}
}

//Привязка к объекту
void _cdecl WaterLevelAccessor::UpdateLevel(float dltTime, long level)
{
	Assert(connectID.NotEmpty());
	if(connect.Validate())
	{
		float lvl = connect.SPtr()->GetMatrix(Matrix()).pos.y;
		((WaterLevel *)ptr.SPtr())->SetLevel(lvl);
	}else{
		FindObject(connectID, connect);
	}
}



void WaterLevel::SetLevel(float lvl)
{
	level = lvl;
}



MOP_BEGINLISTCG(WaterLevelAccessor, "Water level", '1.00', 0x0, "Set current water level", "Managment")
	MOP_FLOAT("Level", 0.0f)
	MOP_BOOLC("Active", false, "Active object in start mission time")
	MOP_STRING("Connect to object", "")
MOP_ENDLIST(WaterLevelAccessor)


MOP_BEGINLIST(WaterLevel, "", '1.00', 0)
MOP_ENDLIST(WaterLevel)
