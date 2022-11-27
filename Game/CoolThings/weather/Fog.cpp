#include "Fog.h"

Fog::Fog()
{
}

Fog::~Fog()
{
}

bool Fog::Create(MOPReader & reader)
{
	return EditMode_Update(reader);
}

bool Fog::EditMode_Create(MOPReader & reader)
{
	return Create(reader);
}

void Fog::Activate(bool isActive)
{
	if(isActive)
	{
		LogicDebug("Activate");
		Console().Trace(COL_ALL, "Fog '%s' enabled", GetObjectID().c_str());
		SetUpdate(&Fog::SetFog, ML_LIGHTS_ON);
		SetUpdate(&Fog::ResetFog, ML_LIGHTS_OFF);		
		
	}else{
		LogicDebug("Deactivate");
		Console().Trace(COL_ALL, "Fog '%s' disabled", GetObjectID().c_str());
		DelUpdate(&Fog::SetFog);		
		DelUpdate(&Fog::ResetFog);			
		ResetFog(0, ML_LIGHTS_OFF);
	}


	MissionObject::Activate(isActive);
}

bool Fog::EditMode_Update(MOPReader & reader)
{
	fFogHeightDensity = reader.Float() * 0.01f;
	fFogHeightMin = reader.Float();
	fFogHeightMax = reader.Float();

	fFogDensity = reader.Float() * 0.01f;


	fFogDistanceMin = reader.Float();
	fFogDistanceMax = reader.Float();

	cFogColor = reader.Colors();

	Activate(reader.Bool());

	return true;
}

//Установка тумана в основном цикле
void _cdecl Fog::SetFog(float dltTime, long level)
{
	Render().setFogParams(fFogHeightDensity, fFogHeightMin, fFogHeightMax, fFogDensity, fFogDistanceMin, fFogDistanceMax, cFogColor);
}

//Убирание тумана в основном цикле
void _cdecl Fog::ResetFog(float dltTime, long level)
{
	Render().setFogParams(0.0f, fFogHeightMin, fFogHeightMax, 0.0f, fFogDistanceMin, fFogDistanceMax, cFogColor);
}



MOP_BEGINLISTCG(Fog, "Fog", '1.00', 100, "", "Weather")
	MOP_FLOATEX("Height Fog Density in percents", 100.0f, 0.0f, 100.0f)
	MOP_FLOAT("Height Fog Min Height", 0.0f)
	MOP_FLOAT("Height Fog Max Height", 1.0f)
	
	MOP_FLOATEX("Distance Fog Density in percents", 100.0f, 0.0f, 100.0f)
	MOP_FLOAT("Distance Fog Min Distance", 0.0f)
	MOP_FLOAT("Distance Fog Max Distance", 100.0f)

	MOP_COLOR("Color", Color(dword(0xBDCBDE)))

	MOP_BOOL("Active", true);
MOP_ENDLIST(Fog)
