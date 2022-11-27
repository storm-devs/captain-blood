//////////////////////////////////////////////////////////////////////////
// Modenov Ivan 2006
//////////////////////////////////////////////////////////////////////////
#include "..\pch.h"
#include ".\MissionCannon.h"
#include "..\ballistics.h"

MissionCannon::MissionCannon(void)
{
	cannon_.SetOwner(this);
}

MissionCannon::~MissionCannon(void)
{
}


void _cdecl MissionCannon::DrawEditorFeatures(float, long)
{
	cannon_.DrawEditorFeatures();
}

void _cdecl MissionCannon::Draw(float fDeltaTime, long level)
{
	if (IsActive())
	{
		MOSafePointer target;
		static const ConstString playerId("Player");
		FindObject(playerId, target);
		bool fired = false;
		if (target.Ptr() && !target.Ptr()->IsDead() && target.Ptr()->IsActive() && target.Ptr()->IsShow())
		{
			Matrix mtx;
			target.Ptr()->GetMatrix(mtx);
			fired = cannon_.Fire(mtx.pos, 1.f);
		}
		else
			fired = cannon_.Fire( cannon_.GetPosition() + cannon_.GetDirection2D()*cannon_.GetMaxDistance(), 1.f );

		if ( !cannon_.GetAutoReload() && fired)
			Activate(false);
	}


	cannon_.Simulate(fDeltaTime);

	Matrix mtx;
	mtx.SetIdentity();

	cannon_.SetParentTransform(mtx);
	cannon_.Draw(fDeltaTime);
}

void MissionCannon::ReadMOPs(MOPReader & reader)
{
	IGMXScene * model = NULL;
	model = Geometry().CreateGMX(reader.String().c_str(), &Animation(), &Particles(), &Sound());
	if (model)
	{
		cannon_.SetModel(model);
		model->Release();
	}

	ConstString shootModelName = reader.String();
	model = Geometry().CreateGMX(shootModelName.c_str(), &Animation(), &Particles(), &Sound());
	if (model)
	{
		cannon_.SetShotModel(model);
		model->Release();
	}

	// регистрируем батчер моделки
	long nBatcher = -1;
	CannonBallContainer* pContainer = cannon_.GetBallContainer();
	if( pContainer )
		nBatcher = pContainer->RegistryCannonballBatcher( shootModelName.c_str(), model );
	cannon_.SetBallBatcher(nBatcher);

	WeaponPattern::ReadTrailParams( cannon_.GetTrailParameters(), reader );
	cannon_.GetTrailParametersPower() = cannon_.GetTrailParameters();

	cannon_.SetSFX(reader.String().c_str());
	cannon_.SetWaterHitSFX(reader.String().c_str());
	cannon_.SetShootSound(reader.String().c_str(), 0.f);

	cannon_.SetPosition(reader.Position());
	cannon_.SetDirectionAngle(PI*reader.Float()/180.0f);
	cannon_.SetShootSectorAngle(PI*reader.Float()/180.0f);
	cannon_.SetShotSpeed(reader.Float());
	cannon_.SetMinDistance(reader.Float());
	cannon_.SetMaxDistance(reader.Float());
	cannon_.SetMinAngle(PI*reader.Float()/180.0f);
	cannon_.SetMaxAngle(PI*reader.Float()/180.0f);
	cannon_.SetDamage(reader.Float());
	cannon_.SetRollbackDistance(reader.Float());
	cannon_.SetMaxReloadTime(reader.Float());
	Activate(reader.Bool());
	cannon_.SetAutoReload(reader.Bool());
	
}
// Создание объекта
bool MissionCannon::Create(MOPReader & reader)
{
	ReadMOPs(reader);

	SetUpdate(&MissionCannon::Draw, ML_GEOMETRY1);
	if (EditMode_IsOn())
		SetUpdate(&MissionCannon::DrawEditorFeatures, ML_GEOMETRY1);

	return true;
}

// Обновление параметров
bool MissionCannon::EditMode_Update(MOPReader & reader)
{
	ReadMOPs(reader);
	return true;
}

// Инициализация 
bool MissionCannon::Init()
{
	return true;
}

void MissionCannon::EditMode_GetSelectBox(Vector & min, Vector & max)
{
	if (cannon_.GetModel())
	{
		min = cannon_.GetModel()->GetLocalBound().vMin;
		max = cannon_.GetModel()->GetLocalBound().vMax;
	}
	else
	{
		min = Vector(-0.3);
		max = Vector(0.3);
	}
}

Matrix & MissionCannon::GetMatrix(Matrix & mtx)
{
	mtx = cannon_.GetTransform();
	return mtx;
}

static const char description[] = "Cannon object for fortresses and towers. Target is always Player object";
MOP_BEGINLISTCG(MissionCannon, "Fort cannon", '1.00', 100, description, "Arcade Sea");
	MOP_STRING("Cannon model", "");
	MOP_STRING("Shot model", "");
	MOP_TRAILPARAMS("Trail");
	MOP_STRING("Shoot SFX", "");
	MOP_STRING("Water hit SFX", "");
	MOP_STRING("Shoot sound", "");
	
	MOP_POSITION("Position", Vector(0,0,0));
	MOP_FLOATEX("Direction angle", 0.0f, 0.0f, 360.0f);
	MOP_FLOATEX("Shoot sector angle", 5.0f, 0.0f, 360.0f);
	MOP_FLOAT("Shot speed", 20);
	MOP_FLOAT("Min distance", 5);
	MOP_FLOAT("Max distance", 15);
	MOP_FLOATEX("Min shoot angle", 0.0f, 0.0f, 90.0f);
	MOP_FLOATEX("Max shoot angle", 60.0f, 0.0f, 90.0f);
	MOP_FLOAT("Damage", 0.5f);
	MOP_FLOAT("Rollback distance", 0.5f);
	MOP_FLOAT("Reload time", 0.5f);
	MOP_BOOL("Active", false);
	MOP_BOOL("Auto reload", false);
MOP_ENDLIST(MissionCannon)



