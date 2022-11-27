#include "..\pch.h"
#include ".\weaponpattern.h"

//////////////////////////////////////////////////////////////////////////
// реализация MinePattern
//////////////////////////////////////////////////////////////////////////

void MinePattern::ReadParams(MOPReader& reader)
{
	IGMXScene * model = NULL;

	mine_.SetPosition(reader.Position());

	mine_.SetMinesCount(reader.Long());
	mine_.SetDropInterval(reader.Float());
	
	mine_.SetKillRadius(reader.Float());
	mine_.SetTriggerRadius(reader.Float());
	mine_.SetDamage(reader.Float());
	mine_.SetAttractMinRadius(reader.Float());
	mine_.SetAttractMaxRadius(reader.Float());
	mine_.SetAttractSpeed(reader.Float());
	mine_.SetSFX(reader.String().c_str());
	mine_.SetShootSound(reader.String().c_str(),0.f);
	mine_.SetExplosionSound(reader.String().c_str());

	model = Geometry().CreateGMX(reader.String().c_str(), &Animation(), &Particles(), &Sound());
	if (model)
	{
		mine_.SetModel(model);
		model->Release();
	}

	mine_.SetTimer(reader.Float());
	mine_.EnableTimer(reader.Bool());
	mine_.EnableHostDamage(reader.Bool());
	
	mine_.SetOnboard(reader.Bool());
	mine_.Activate(true);
}

bool MinePattern::EditMode_Update(MOPReader & reader)
{
	ReadParams(reader);
	return true;
}

void MinePattern::EditMode_GetSelectBox(Vector & min, Vector & max)
{
	min = -0.3f;
	max = 0.3f;
}

Matrix & MinePattern::GetMatrix(Matrix & mtx)
{
	mtx.BuildPosition(mine_.GetPosition());
	return mtx;
}

void _cdecl MinePattern::Draw( float deltaTime, long level)
{
	if (!EditMode_IsSelect())
		return;

	mine_.DrawEditorFeatures();
}


//////////////////////////////////////////////////////////////////////////
// реализация CannonPattern
//////////////////////////////////////////////////////////////////////////
void CannonPattern::ReadParams(MOPReader& reader)
{
	IGMXScene * model = NULL;

	cannon_.SetPosition(reader.Position());

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

	//cannon_.SetTrailParticles(reader.String().c_str());
	ReadTrailParams( cannon_.GetTrailParameters(), reader );
	ReadTrailParams( cannon_.GetTrailParametersPower(), reader );
	/*float fTraceInitSize = reader.Float();
	float fTraceFinalSize = reader.Float();
	float fTraceMinFadeoutTime = reader.Float();
	float fTraceMaxFadeoutTime = reader.Float();
	float fTraceOffsetStrength = reader.Float();
	Color cTraceColor = reader.Colors();
	cannon_.SetTrailParameters(fTraceInitSize,fTraceFinalSize,fTraceMinFadeoutTime,fTraceMaxFadeoutTime,fTraceOffsetStrength,cTraceColor);*/

	cannon_.SetSFX(reader.String().c_str());
	cannon_.SetWaterHitSFX(reader.String().c_str());
	cannon_.SetWaterHitSound(reader.String().c_str());
	cannon_.SetColliderHitSFX(reader.String().c_str());
	cannon_.SetColliderHitSound(reader.String().c_str());
	const char* pcShootSound = reader.String().c_str();
	float fShootSoundDelay = reader.Float();
	cannon_.SetShootSound(pcShootSound,fShootSoundDelay);
	cannon_.SetExplosionSound(reader.String().c_str());
	cannon_.SetFlySound(reader.String().c_str());
	cannon_.SetFlySoundTime(reader.Float());

	cannon_.SetDirectionAngle(Deg2Rad(reader.Float()));
	cannon_.SetShootSectorAngle(Deg2Rad(reader.Float()));
	cannon_.SetShotSpeed(reader.Float());
	cannon_.SetMinDistance(reader.Float());
	cannon_.SetMaxDistance(reader.Float());
	cannon_.SetMinAngle(Deg2Rad(reader.Float()));
	cannon_.SetMaxAngle(Deg2Rad(reader.Float()));
	cannon_.SetMaxAngleKnippels(Deg2Rad(reader.Float()));
	cannon_.SetDamage(reader.Float());
	cannon_.SetPowerMultiplier(reader.Float());
	cannon_.SetRollbackDistance(reader.Float());
	cannon_.SetMaxReloadTime(reader.Float());
	cannon_.SetAutoReload(reader.Bool());
	cannon_.SetAnimBoneName(reader.String().c_str());

	cannon_.Activate(true);
}

bool CannonPattern::EditMode_Update(MOPReader & reader)
{
	ReadParams(reader);

	return true;
}

void CannonPattern::EditMode_GetSelectBox(Vector & min, Vector & max)
{
	min = -0.3f;
	max = 0.3f;
}

Matrix & CannonPattern::GetMatrix(Matrix & mtx)
{
	mtx.BuildPosition(cannon_.GetPosition());
	return mtx;
}

void _cdecl CannonPattern::Draw( float deltaTime, long level)
{
	if (!EditMode_IsSelect())
		return;

	cannon_.DrawEditorFeatures();
}


//////////////////////////////////////////////////////////////////////////
// реализация FlameThrowerPattern
//////////////////////////////////////////////////////////////////////////
void FlameThrowerPattern::ReadParams(MOPReader& reader)
{
	IGMXScene * model = NULL;
	IParticleSystem * sfx = NULL;

	ft_.SetPosition(reader.Position());

	model = Geometry().CreateGMX(reader.String().c_str(), &Animation(), &Particles(), &Sound());
	if (model)
	{
		ft_.SetModel(model);
		model->Release();
	}

	ft_.SetSFX(reader.String().c_str());
	ft_.SetShootSound( reader.String().c_str(), 0.f );
	ft_.SetExplosionSound( reader.String().c_str() );

	ft_.SetDirectionAngle(Deg2Rad(reader.Float()));
	ft_.SetShootAngle(Deg2Rad(reader.Float()));
	ft_.SetMaxDistance(reader.Float());
	ft_.SetDamage(reader.Float());
	ft_.SetDamageArea(reader.Float());
	ft_.SetFireDeltaTime(reader.Float());
	ft_.SetReloadTime(reader.Float());
	ft_.SetReloadSpeed(reader.Float());
	ft_.SetCollisionRaysCount(reader.Long());
	ft_.Activate(true);
}

bool FlameThrowerPattern::EditMode_Update(MOPReader & reader)
{
	ReadParams(reader);

	return true;
}

void FlameThrowerPattern::EditMode_GetSelectBox(Vector & min, Vector & max)
{
	min = -0.3f;
	max = 0.3f;
}

Matrix & FlameThrowerPattern::GetMatrix(Matrix & mtx)
{
	mtx.BuildPosition(ft_.GetPosition());
	return mtx;
}

void _cdecl FlameThrowerPattern::Draw(float, long)
{
	if (!EditMode_IsSelect())
		return;

	ft_.DrawEditorFeatures();
}


//////////////////////////////////////////////////////////////////////////
// Миссионные параметры для шаблона мины
//////////////////////////////////////////////////////////////////////////
MOP_BEGINLISTG(MinePattern, "Mine_Pattern", '1.00', 80, "Arcade Sea");
	MOP_POSITION("Pattern position", Vector(0,0,0))

	MOP_LONGEX("Mines count", 1, 1, 20)
	MOP_FLOAT("Drop interval (reload time)", 2.0f)
	MOP_FLOAT("Damage radius", 5.0f)
	MOP_FLOAT("Trigger radius", 1.0f)
	MOP_FLOAT("Damage", 1.0f)
	MOP_FLOAT("Attract min radius", 1.f);
	MOP_FLOAT("Attract max radius", 20.f);
	MOP_FLOATC("Attract speed", 0.7f, "relative of distance per second (example: 0.5=half distance per second");
	MOP_STRING("Explosion particles", "")
	MOP_STRING("Shoot sound", "")
	MOP_STRING("Explosion sound", "")
	MOP_STRING("Mine model", "")
	MOP_FLOAT("Timer", 5.0f)
	MOP_BOOL("Use timer", false)
	MOP_BOOL("Host damage", false)
	MOP_BOOL("Is Onboard", false)
MOP_ENDLIST(MinePattern)

//////////////////////////////////////////////////////////////////////////
// Миссионные параметры для шаблона пушки
//////////////////////////////////////////////////////////////////////////
MOP_BEGINLISTG(CannonPattern, "Cannon_Pattern", '1.00', 80, "Arcade Sea");
	MOP_POSITION("Pattern position", Vector(0,0,0))

	MOP_STRING("Cannon model", "cannon.gmx");
	MOP_STRING("Shot model", "bomb.gmx");
	MOP_TRAILPARAMS("Shoot trace");
	MOP_TRAILPARAMS("Shoot power trace");
	MOP_STRING("Shoot particles", "shotcannon.xps");
	MOP_STRING("Water hit particles", "");
	MOP_STRING("Water hit sound", "");
	MOP_STRINGC("Collider hit particles", "", "Particles name for hit effect to not ship object");
	MOP_STRINGC("Collider hit sound", "", "Sound name for hit effect to not ship object");
	MOP_STRING("Shoot sound", "");
	MOP_FLOAT("Shoot sound delay", 0.f);
	MOP_STRING("Explosion sound", "");
	MOP_STRING("Fly sound", "");
	MOP_FLOAT("Fly sound time", 2.0f);
	MOP_FLOATEX("Direction angle", 0.0f, 0.0f, 360.0f);
	MOP_FLOATEX("Shoot sector angle", 5.0f, 0.0f, 360.0f);
	MOP_FLOAT("Shot speed", 20);
	MOP_FLOAT("Min distance", 5);
	MOP_FLOAT("Max distance", 15);
	MOP_FLOATEX("Min shoot angle", 0.0f, 0.0f, 360.0f);
	MOP_FLOATEX("Max shoot angle", 60.0f, 0.0f, 360.0f);
	MOP_FLOATEX("Max shoot angle for knippels", 60.0f, 0.0f, 360.0f);
	MOP_FLOAT("Damage", 0.5f);
	MOP_FLOAT("Power damage multiplier", 1.f);
	MOP_FLOAT("Rollback distance", 0.5f);
	MOP_FLOAT("Reload time", 1.0f);
	MOP_BOOL("Auto reload", false);
	MOP_STRING("Animation bone name", "")
MOP_ENDLIST(CannonPattern)

//////////////////////////////////////////////////////////////////////////
// Миссионные параметры для шаблона огнемета
//////////////////////////////////////////////////////////////////////////
MOP_BEGINLISTG(FlameThrowerPattern, "Flamethrower_Pattern", '1.00', 80, "Arcade Sea");
	MOP_POSITION("Pattern position", Vector(0,0,0))

	MOP_STRING("FlameThrower model", "cannon.gmx");
	MOP_STRING("Shoot particles", "shotcannon.xps");
	MOP_STRING("Shoot sound", "");
	MOP_STRING("Explosion sound", "");
	MOP_FLOATEX("Direction angle", 0.0f, 0.0f, 360.0f);
	MOP_FLOATEX("Shoot angle", 0.0f, 0.0f, 360.0f);
	MOP_FLOAT("Max distance", 15);
	MOP_FLOAT("Damage", 0.5f);
	MOP_FLOAT("Damage area", 0.5f);
	MOP_FLOATC("Fire delta time", 2.0f, "");
	MOP_FLOAT("Charge time", 1.0f);
	MOP_FLOATC("Reload speed", 0.2f, "");
	MOP_LONG("Collision rays count", 5);
MOP_ENDLIST(FlameThrowerPattern)