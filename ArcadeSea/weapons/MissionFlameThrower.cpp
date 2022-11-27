//////////////////////////////////////////////////////////////////////////
// Modenov Ivan 2006
//////////////////////////////////////////////////////////////////////////
#include "..\pch.h"
#include ".\MissionFlameThrower.h"
#include "..\ballistics.h"

MissionFlameThrower::MissionFlameThrower(void)
{
	ft_.SetOwner(this);
}

MissionFlameThrower::~MissionFlameThrower(void)
{
}

void MissionFlameThrower::Command(const char * id, dword paramCount, const char ** params)
{
	if ( string::IsEqual("Fire", id) )
	{
		ft_.Fire(Vector(0.f),1.f);
	}
}

void _cdecl MissionFlameThrower::DrawEditorFeatures(float, long)
{
	if (!EditMode_IsSelect())
		return;

	ft_.DrawEditorFeatures();
}

void _cdecl MissionFlameThrower::Draw(float fDeltaTime, long level)
{
	ft_.Simulate(fDeltaTime);

	if (ft_.IsActive())
	{
		ft_.Fire( WeaponTargetZone(Vector(0.0f)), 1.f );
	//	ft_.Activate(false);
	}

	Matrix mtx;
	mtx.SetIdentity();

	ft_.SetParentTransform(mtx);
	ft_.Draw(fDeltaTime);

}

void MissionFlameThrower::CommonUpdate(MOPReader & reader)
{
	IGMXScene * model = NULL;
	model = Geometry().CreateGMX(reader.String().c_str(), &Animation(), &Particles(), &Sound());
	Assert(model != NULL);
	ft_.SetModel(model);
	model->Release();

	ft_.SetSFX(reader.String().c_str());

	ft_.SetPosition(reader.Position());
	ft_.SetDirectionAngle(PI*reader.Float()/180.0f);
	ft_.SetShootAngle(PI*reader.Float()/180.0f);
	ft_.SetMaxDistance(reader.Float());
	ft_.SetDamage(reader.Float());
	ft_.SetDamageArea(reader.Float());
	ft_.SetReloadTime(reader.Float());
	ft_.SetCollisionRaysCount(reader.Long());
	ft_.Activate(reader.Bool());
	
	SetUpdate(&MissionFlameThrower::Draw, ML_GEOMETRY1);
}
// Создание объекта
bool MissionFlameThrower::Create(MOPReader & reader)
{
	CommonUpdate(reader);
	SetUpdate(&MissionFlameThrower::DrawEditorFeatures, ML_GEOMETRY1);
	return true;
}

// Обновление параметров
bool MissionFlameThrower::EditMode_Update(MOPReader & reader)
{
	CommonUpdate(reader);
	return true;
}

// Инициализация 
bool MissionFlameThrower::Init()
{
	return true;
}

void MissionFlameThrower::EditMode_GetSelectBox(Vector & min, Vector & max)
{
	if (ft_.GetModel())
	{
		min = ft_.GetModel()->GetLocalBound().vMin;
		max = ft_.GetModel()->GetLocalBound().vMax;
	}
	else
	{
		min = Vector(-0.3);
		max = Vector(0.3);
	}
}

Matrix & MissionFlameThrower::GetMatrix(Matrix & mtx)
{
	mtx = ft_.GetTransform();
	return mtx;
}

MOP_BEGINLISTG(MissionFlameThrower, "Fort flame thrower", '1.00', 100, "Arcade Sea");
	MOP_STRING("FlameThrower model", "cannon.gmx");
	MOP_STRING("Shoot SFX", "shotcannon.xps");
	MOP_POSITION("Position", Vector(0,0,0));
	MOP_FLOATEX("Direction angle", 360.0f, 0.0f, 360.0f);
	MOP_FLOATEX("Shoot angle", 360.0f, 0.0f, 360.0f);
	MOP_FLOAT("Max distance", 15);
	MOP_FLOAT("Damage", 0.5f);
	MOP_FLOAT("Damage area", 0.5f);
	MOP_FLOAT("Reload time", 1.0f);
	MOP_LONG("Collision rays count", 5);
	MOP_BOOL("Active", false);
MOP_ENDLIST(MissionFlameThrower)



