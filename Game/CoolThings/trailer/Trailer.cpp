#include "Trailer.h"

Trailer:: Trailer() : objects(_FL_)
{
}

Trailer::~Trailer()
{
}

bool Trailer::Create		  (MOPReader &reader)
{
	InitParams(reader);

	Show(true);

	return true;
}

bool Trailer::EditMode_Update(MOPReader &reader)
{
	InitParams(reader);

	return true;
}

void Trailer::Show(bool isShow)
{
	MissionObject::Show(isShow);

	if( isShow )
		SetUpdate(&Trailer::Draw,ML_ALPHA5);
	else
		DelUpdate(&Trailer::Draw);
}

void Trailer::Activate(bool isActive)
{
	MissionObject::Activate(isActive);

	//
}

void Trailer::Command(const char *id, dword numParams, const char **params)
{
	//
}

void _cdecl Trailer::Draw(float dltTime, long level)
{
	if( IsActive())
	{
		Matrix m;

		for( int i = 0 ; i < objects ; i++ )
		{
			Object &o = objects[i];

			o.t->Update(o.p->GetMatrix(m));
		}
	}
}

void Trailer::InitParams(MOPReader &reader)
{
	for( int i = 0 ; i < objects ; i++ )
		objects[i].t->Release();

	objects.DelAll();

	MOSafePointer sp;

	static const ConstString objectId("TrailManager");
	Mission().CreateObject(sp,"TrailManager",objectId);

	ITrailManager *tm = (ITrailManager *)sp.Ptr();
	Assert(tm)

	long n = reader.Array();

	for( int i = 0 ; i < n ; i++ )
	{
		MOSafePointer sp;

		FindObject(reader.String(),sp);

		MissionObject *p = (MissionObject *)sp.Ptr();

		float trailBegin = reader.Float();
		float trailEnd   = reader.Float();

		float trailDelayMin = reader.Float();
		float trailDelayMax = reader.Float();

		dword trailColor = reader.Colors();

		float trailOffStr = reader.Float();

		Vector trailDir = Matrix(reader.Angles()).vz;
		float  trailVel = reader.Float();

		float  forceValue = reader.Float();

		bool staticLive = reader.Bool();

		if( p )
		{
			Object &o = objects[objects.Add()];

			o.p = p;
			o.t = tm->Add();

			Assert(o.t)

			o.t->SetParams(trailBegin,trailEnd,trailDelayMin,trailDelayMax,
						   trailColor,trailOffStr,trailVel,trailDir,forceValue);

			o.t->EnableStaticLive(staticLive);
		}
	}

	Activate(reader.Bool());
}

MOP_BEGINLISTCG(Trailer, "Trailer", '1.00', 100, "Trailer\n\n    Use to add trails to mission objects", "Default")

	MOP_ARRAYBEG("Objects", 0, 100)

		MOP_STRING("Name", "")

		MOP_FLOAT("Beg radius", 0.55f);
		MOP_FLOAT("End radius", 2.50f);

		MOP_FLOAT("Min fade delay", 2.0f);
		MOP_FLOAT("Max fade delay", 3.0f);

		MOP_COLOR("Color", Color((dword)-1));

		MOP_FLOAT("Offset strength", 1.0f);

	//	MOP_GROUPBEG("Wind params")

			MOP_ANGLES("Wind Dir", Vector(-PI*0.5f,0.0f,0.0f))
			MOP_FLOAT ("Wind Vel", 0.0f)

	//	MOP_GROUPEND()

		MOP_FLOATC("Force value", 0.0f, "Скорость выбрасывания дыма");

		MOP_BOOLC("Enable static live", false, "Генерить дым при остановке");

	MOP_ARRAYEND

	MOP_BOOL("Active", false)

MOP_ENDLIST(Trailer)
