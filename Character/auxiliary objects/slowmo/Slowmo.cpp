#include "Slowmo.h"

Slowmo::Slowmo() : 
	slowmos(_FL_, 32)
{
}

Slowmo::~Slowmo()
{
}

bool Slowmo::Create(MOPReader & reader)
{
	return EditMode_Update(reader);
}

bool Slowmo::EditMode_Update(MOPReader & reader)
{
	int numSlowmos = reader.Array();
	for (int i=0; i<numSlowmos; i++)
	{
		SlowmoParams & sp = slowmos[slowmos.Add()];
		sp.name = reader.String();
		sp.locator = reader.String();
		ConstString type = reader.Enum();
		sp.type = ISlowmo::circle;
		sp.sectorDirection = reader.Angles().y;
		sp.sectorWidth = reader.Float();
		sp.radius = reader.Float();
		sp.damage = reader.Float();
		sp.probability = reader.Float();
		sp.timeScale = reader.Float();
		sp.duration = reader.Float();
		sp.delay = reader.Float();
	}
	return true;
}

const Slowmo::SlowmoParams * Slowmo::GetSlowmo(const ConstString & name) const
{
	return null;
}

MOP_BEGINLISTCG(Slowmo, "Slowmo", '1.00', 50, "Slowmo parameters", "Character")
	MOP_ENUMBEG("DamageAreaType")
		MOP_ENUMELEMENT("circle")
		MOP_ENUMELEMENT("sector")
	MOP_ENUMEND

	MOP_ARRAYBEG("Slowmos", 0, 1000)
		MOP_STRING("Name", "")
		MOP_STRING("Locator", "")
		MOP_ENUM("DamageAreaType", "Damage area")
		MOP_ANGLES("Sector direction", 0.0f)
		MOP_FLOAT("Sector width", 0.0f)
		MOP_FLOAT("Radius", 2.0f)
		MOP_FLOAT("Damage", 20.0f)
		MOP_FLOATEX("Probability", 5, 0, 100)
		MOP_FLOAT("Time scale", 0.25f)
		MOP_FLOAT("Duration", 1.0f)
		MOP_FLOATEX("Delay", 0.0f, 0.0f, 10000.0f)
	MOP_ARRAYEND
MOP_ENDLIST(Slowmo)
