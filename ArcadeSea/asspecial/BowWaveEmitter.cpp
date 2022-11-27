#include "..\pch.h"
#include ".\bowwaveemitter.h"



BowWaveEmitter::BowWaveEmitter(void) :
	prevHostPos_(0.0f),
	prevHostDir_(0.0f, 0.0f, 1.0f),
	shipVelocity_(0.0f),
	shipAngularVel_(0.0f),
#pragma warning(disable : 4355)
	particles_(*this),
	geometry_(*this)
#pragma warning(default : 4355)
{
	params_.isEmissionStopped_ = true;
}


BowWaveEmitter::~BowWaveEmitter(void)
{

}


void BowWaveEmitter::DrawEditorFeatures()
{
	Matrix m;
	Vector pos = params_.pos;
	if (host_.Ptr())
		pos += host_.Ptr()->GetMatrix(m).pos;

	float A[2], C[2];
	C[0] = params_.curveHeight[0];
	C[1] = params_.curveHeight[1];
	A[0] = (-params_.endLine - C[0])/(params_.curveWidth[0]*params_.curveWidth[0]);
	A[1] = (-params_.endLine - C[1])/(params_.curveWidth[1]*params_.curveWidth[1]);

	unsigned int tessFactor = 20;
	float cx = 0.0f;
	Vector v1, v2;
	for (unsigned int i = 0; i < tessFactor; ++i)
	{
		cx = -params_.curveWidth[0] + 2*i*(params_.curveWidth[0])/tessFactor;
		v1.x = cx;
		v1.y = 0;
		v1.z = A[0]*v1.x*v1.x + C[0];

		cx = -params_.curveWidth[0] + 2*(i+1)*(params_.curveWidth[0])/tessFactor;
		v2.x = cx;
		v2.y = 0;
		v2.z = A[0]*v2.x*v2.x + C[0];

		Render().DrawLine(pos + v1, 0xFFFFFFFF, pos + v2, 0xFFFFFFFF);

		cx = -params_.curveWidth[1] + 2*i*(params_.curveWidth[1])/tessFactor;
		v1.x = cx;
		v1.y = 0;
		v1.z = A[1]*v1.x*v1.x + C[1];

		cx = -params_.curveWidth[1] + 2*(i+1)*(params_.curveWidth[1])/tessFactor;
		v2.x = cx;
		v2.y = 0;
		v2.z = A[1]*v2.x*v2.x + C[1];

		Render().DrawLine(pos + v1, 0xFFFFFFFF, pos + v2, 0xFFFFFFFF);

		ParticleController solver(	pos, v1 + pos, GetHostLinearSpeed(),
								params_.maxHeight, 0.0f, params_.parabolaTrackCoef	);
		for (unsigned int k = 0; k < tessFactor; ++k)
		{
			Render().DrawLine(	solver.GetPosition(params_.parabolaTrackCoef*k/(float)tessFactor), 0xFF0000FF,
								solver.GetPosition(params_.parabolaTrackCoef*(k+1)/(float)tessFactor), 0xFF0000FF);
		}
		Render().DrawLine(	solver.GetPosition(params_.parabolaTrackCoef), 0xFF0000FF,
							solver.GetPosition(1.0f), 0xFF0000FF);
	}
	ParticleController solver(	pos, v2 + pos, GetHostLinearSpeed(),
							params_.maxHeight, 0.0f, params_.parabolaTrackCoef );
	for (unsigned int k = 0; k < tessFactor; ++k)
	{
		Render().DrawLine(	solver.GetPosition(params_.parabolaTrackCoef*k/(float)tessFactor), 0xFF0000FF,
							solver.GetPosition(params_.parabolaTrackCoef*(k+1)/(float)tessFactor), 0xFF0000FF);
	}
	Render().DrawLine(	solver.GetPosition(params_.parabolaTrackCoef), 0xFF0000FF,
						solver.GetPosition(1.0f), 0xFF0000FF);

	Render().DrawSphere(pos, params_.curveHeight[0]/2, 0xFF00FF00);
}

void _cdecl BowWaveEmitter::PartWork(float deltaTime, long level)
{
	if (!host_.Ptr())
	{
		FindObject(params_.hostName, host_);
	}

	if (host_.Ptr())
	{
		Matrix m;

		shipVelocity_ = (host_.Ptr()->GetMatrix(m).pos - prevHostPos_)/deltaTime;
		prevHostPos_ = host_.Ptr()->GetMatrix(m).pos;

		shipAngularVel_ = host_.Ptr()->GetMatrix(m).MulNormal(Vector(0.0f, 0.0f, 1.0f)).GetAngleXZ(prevHostDir_)/deltaTime;
		prevHostDir_ = host_.Ptr()->GetMatrix(m).MulNormal(Vector(0.0f, 0.0f, 1.0f));
	}

	if (EditMode_IsOn())
		{ if (EditMode_IsSelect()) DrawEditorFeatures(); }
	else
	{
		particles_.Work(deltaTime);
	}
}

void _cdecl BowWaveEmitter::GeoWork(float deltaTime, long level)
{
//	if (!EditMode_IsOn() && host_)
//		geometry_.Work(deltaTime, shipVelocity_);
}



void BowWaveEmitter::ReadMOPs(MOPReader & reader)
{
	params_.hostName = reader.String();
	params_.texName = reader.String().c_str();
	params_.pos = reader.Position();
	params_.maxPartCount = reader.Long();
	params_.curveHeight[0] = reader.Float();
	params_.curveHeight[1] = reader.Float();
	params_.curveWidth[0] = reader.Float();
	params_.curveWidth[1] = reader.Float();
	params_.endLine = reader.Float();
	params_.parabolaTrackCoef = reader.Float();
	params_.maxHeight = reader.Float();
	params_.minHeight = reader.Float();
	params_.maxParticleSize = reader.Float();
	params_.minParticleSize = reader.Float();
	params_.maxParticleLifeTime = reader.Float();
	params_.minParticleLifeTime = reader.Float();
	params_.maxRotateSpeed = reader.Float();
	params_.minRotateSpeed = reader.Float();

	
}

// Создание объекта
bool BowWaveEmitter::Create(MOPReader & reader)
{
	ReadMOPs(reader);
	geometry_.Init();
	particles_.Init();
	SetUpdate(&BowWaveEmitter::PartWork, ML_PARTICLES5);
	SetUpdate(&BowWaveEmitter::GeoWork, ML_ALPHA4+10);

	return true;
}

// Обновление параметров
bool BowWaveEmitter::EditMode_Update(MOPReader & reader)
{
	ReadMOPs(reader);
	return true;
}

// Инициализация 
bool BowWaveEmitter::Init()
{
	return true;
}

//MOP_BEGINLISTG(BowWaveEmitter, "BowWaveEmitter", '1.00', 110, "Arcade Sea");
//	MOP_STRING("Host ship", "Ship")
//	MOP_STRING("Texture", "bowwave")
//	MOP_POSITION("Position", Vector(0.0f))
//	MOP_LONGEX("Max particles count", 500, 50, 5000)
//
//	MOP_FLOATEX("First curve height", 0.5f, 0.0f, 100.0f)
//	MOP_FLOATEX("Second curve height", 1.0f, 0.0f, 100.0f)
//	MOP_FLOATEX("First curve width", 1.0f, 0.1f, 100.0f)
//	MOP_FLOATEX("Second curve width", 1.0f, 0.1f, 100.0f)
//
//	MOP_FLOATEX("Parabola end line", 1.0f, 0.0f, 100.0f)
//	MOP_FLOATEX("Parabola-track relation", 0.1f, 0.0f, 1.0f)
//
//	MOP_FLOATEX("Max bow height", 0.5f, 0.1f, 10.0f)
//	MOP_FLOATEX("Min bow height", 0.3f, 0.1f, 10.0f)
//
//	MOP_FLOATEX("Max particle size", 0.5f, 0.1f, 10.0f)
//	MOP_FLOATEX("Min particle size", 0.3f, 0.1f, 10.0f)
//
//	MOP_FLOATEX("Max particle life time", 2.0f, 0.1f, 10.0f)
//	MOP_FLOATEX("Min particle life time", 1.3f, 0.1f, 10.0f)
//
//	MOP_FLOATEX("Max rotate speed (grad/sec)", 180.0f, -360.0f, 360.0f)
//	MOP_FLOATEX("Min rotate speed (grad/sec)", 90.0f, -360.0f, 360.0f)
//
//MOP_ENDLIST(BowWaveEmitter)
