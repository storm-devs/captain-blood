#include "BombPattern.h"

BombExplosionPattern::BombExplosionPattern()
{
	position = 0.0f;
}

BombExplosionPattern::~BombExplosionPattern()
{
}

bool BombExplosionPattern::Create(MOPReader & reader)
{
	return EditMode_Update(reader);
}

bool BombExplosionPattern::EditMode_Update(MOPReader & reader)
{
	position = reader.Position();

	particleName = reader.String();
	soundName = reader.String();
	cameraShockerName = reader.String();
	//connectTo = reader.String();

	distanceBoom = reader.Float();
	eventDistanceBoom.Init(reader);
	eventCommonBoom.Init(reader);

	//noSwing = reader.Bool();
	bool activate = reader.Bool();

	Activate(activate);

	return true;
}

void BombExplosionPattern::PostCreate()
{
	if (FindObject(cameraShockerName, cameraShockerMO.GetSPObject()))
	{
		static const ConstString CameraShockerId = ConstString("CameraShocker");
		if (!cameraShockerMO.Ptr()->Is(CameraShockerId))
			cameraShockerMO.Reset();
	}
}

void BombExplosionPattern::SetMatrix(Matrix & mtx)
{
	position = mtx.pos;
}

void BombExplosionPattern::Command(const char * id, dword numParams, const char ** params)
{
	if (!IsActive())
	{
		LogicDebug("Trying to boom, but bomb pattern is not active");
		return;
	}

	if (numParams >= 3)
	{
		position.x = float(atof(params[0]));
		position.y = float(atof(params[1]));
		position.z = float(atof(params[2]));
	}

	// создаем партикл
	IParticleSystem * particle = Particles().CreateParticleSystem(particleName.c_str());
	if (particle)
	{
		particle->Teleport(Matrix(true).BuildPosition(position));
		particle->AutoDelete(true);
	}
	
	// создаем звук
	Sound().Create3D(soundName.c_str(), position, _FL_);

	// Активируем шокер камеры
	if (cameraShockerMO.Validate())
	{
		((ICameraShocker*)cameraShockerMO.Ptr())->SetMatrix(Matrix(true).BuildPosition(position));
		cameraShockerMO.Ptr()->Activate(true);
	}

	// Активируем тригеры
	if (~(Render().GetView().GetCamPos() - position) <= Sqr(distanceBoom))
		eventDistanceBoom.Activate(Mission(), true, this);

	eventCommonBoom.Activate(Mission(), true, this);
}

const char * BombExplosionPattern::comment =
"Bomb pattern for mission\n"
" \n"
"Commands list:\n"
"----------------------------------------\n"
"  Making Bomb boom in position			 \n"
"----------------------------------------\n"
"    command: Boom\n"
"    param1: x\n"
"    param2: y\n"
"    param3: z\n"
" \n"
" ";

MOP_BEGINLISTCG(BombExplosionPattern, "Bomb explosion pattern", '1.00', 0x0fffffff, BombExplosionPattern::comment, "Character")
	MOP_POSITIONC("Position", 0.0f, "Default position")
	MOP_STRING("Particle name", "ExplosionBomb")
	MOP_STRING("Sound name", "bmb_blast")
	MOP_STRING("Camera shocker object", "StdBombExplosionCameraShocker")
	//MOP_STRING("Connect to object", "")
	MOP_GROUPBEG("Distance trigger")
		MOP_FLOAT("Distance", 100.0f)
		MOP_MISSIONTRIGGERC("Distance.", "Trigger will be activated if distance to camera less or equal \"Distance\"")
	MOP_GROUPEND()
	MOP_MISSIONTRIGGERG("Common trigger", "Common.")
	//MOP_BOOLC("No swing", false, "No swing particles in swing machine")
	MOP_BOOL("Active", true)
MOP_ENDLIST(BombExplosionPattern)
