#include "Wind.h"

Wind::Wind()
{
	vAngle = 0.0f;
}

Wind::~Wind()
{
}

bool Wind::Create(MOPReader & reader)
{
	return EditMode_Update(reader);
}

bool Wind::EditMode_Create(MOPReader & reader)
{
	return EditMode_Update(reader);
}

bool Wind::EditMode_Update(MOPReader & reader)
{
	vAngle = reader.Angles();
	fSpeed = reader.Float();
	fMaxSpeed = reader.Float();
	return true;
}

void _cdecl	Wind::Execute(float fDeltaTime, long level)
{
//	Interface().SetWindDirection(GetAngle().y);
}

Vector Wind::GetAngle() 
{ 
	return vAngle; 
}

float Wind::GetSpeed() 
{ 
	return fSpeed; 
}

float Wind::GetRelativeSpeed()
{
	return Clamp(GetSpeed() / fMaxSpeed);
}

MOP_BEGINLISTCG(Wind, "Wind", '1.00', 100, "", "Weather")
	MOP_ANGLES("Wind Angle", 0.0f);
	MOP_FLOATEX("Wind Speed", 2.0f, 0.0f, 1e12f);
	MOP_FLOATEX("Max Wind Speed", 18.0f, 1.0f, 1e12f);
MOP_ENDLIST(Wind)
