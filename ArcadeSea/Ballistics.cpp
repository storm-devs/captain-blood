//////////////////////////////////////////////////////////////////////////
// Modenov Ivan 2006
//////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include ".\ballistics.h"
#include "..\common_h\defines.h"

const float Ballistics::gravity_ = -9.8f;

// конструктор
Ballistics::Ballistics() :
curTime_(0.0f),
v0_(0.0f),
v1_(0.0f),
startSpeed_(0.0f),
curPos_(0.0f),
bValid_(false)
{

}

// деструктор
Ballistics::~Ballistics()
{

}

// устанавливает новую траекторию
bool Ballistics::SetTrajectoryParams(const Vector& v0, const Vector& v1, float startSpeed, float minAngle, float maxAngle)
{
	SetCommonTrajectoryParams(v0,v1);
	startSpeed_ = startSpeed;
	isPhysicsFly_ = true;
	bValid_ = ComputeShootAngle( v0_, v1_, startSpeed_, angle_, minAngle, maxAngle );
	g_ = gravity_;
	fTotalTime_ = GetTotalFlightTime();
	return bValid_;
}

bool Ballistics::SetTrajectoryParamsByTime(const Vector& v0, const Vector& v1, float fMinHeight, float fTime)
{
	SetCommonTrajectoryParams(v0,v1);

	isPhysicsFly_ = false;
	bValid_ = true;
	fTotalTime_ = fTime;
	height_ = fMinHeight;

	//bValid_ = ComputeShootParams(v0, v1, fMinHeight, fTime, startSpeed_, angle_, fTotalTime_, g_);
	//return bValid_;
	return true;
}

// возвращает полное время полета
float Ballistics::GetTotalFlightTime() const
{
	return ComputeFlightTime((v1_-v0_).GetLengthXZ(), angle_, startSpeed_);
}

// расчитывает положение снаряда в произвольный момент времени
Vector Ballistics::GetPosition(float time) const
{
	if (!bValid_)
		return v0_;

	if( isPhysicsFly_ )
	{
		Vector pos;
		ComputePosition(v0_, v1_, g_, startSpeed_, angle_, time, pos);
		return pos;
	}
	return v0_;
}

// расчитывает новое положение снаряда
void Ballistics::Step(float deltaTime)
{
	curTime_ += deltaTime;
	if( isPhysicsFly_ )
	{
		ComputePosition(v0_, v1_, g_, startSpeed_, angle_, curTime_, curPos_);
	}
	else
	{
		if( curTime_ < fTotalTime_ )
		{
			float k = curTime_ / fTotalTime_;
			curPos_.Lerp( v0_, v1_, k );
			curPos_.y += k*(1.f-k) * height_ * 4.f;
		}
		else
		{
			curPos_ = v1_;
			curTime_ = fTotalTime_;
		}
	}
}

// расчитывает угол, под которым надо
// запустить снаряд, чтобы он попал в заданную точку
bool Ballistics::ComputeShootAngle(const Vector& v0, const Vector& v1, float startSpeed, float & angle, float minAngle, float maxAngle)
{
	float B = (v1-v0).GetLengthXZ();
	float A = gravity_*B*B/(2*startSpeed*startSpeed);
	float C = A - (v1-v0).y;

	float D = B*B-4*A*C;

	if (D < 0.0f)
	{
		return false;
	}

	float angle0 = atan((-B + sqrt(D))/(2*A));
	float angle1 = atan((-B - sqrt(D))/(2*A));

/*	Vector dir = v1 - v0;
	float fdist = dir.Normalize();
	float ftime = fdist / startSpeed;
	float fVSpeed = -gravity_ * ftime * .5f;
	float fRealVSpeed = dir.y * startSpeed + fVSpeed;
	float fRealHSpeed = dir.GetLengthXZ() * startSpeed;

	angle = atan2(fRealVSpeed,fRealHSpeed);
	if( angle < minAngle || angle > maxAngle )
	{
		api->Trace("ComputeShootAngle angle limit: speed=%.2f, dist=%.2f, angle=%.3f, limit=%.3f - %.3f",
			startSpeed, fdist, angle, minAngle, maxAngle );
		return false;
	}
	return true;*/

	if (	angle0 < minAngle && angle1 < minAngle ||
			angle0 > maxAngle && angle1 > maxAngle ||
			angle0 < minAngle && angle1 > maxAngle ||
			angle0 > maxAngle && angle1 < minAngle)
	{
		return false;
	}

	if( angle0 < minAngle || angle0 > maxAngle )
	{
		angle = angle1;
		return true;
	}

	if( angle1 < minAngle || angle1 > maxAngle )
	{
		angle = angle0;
		return true;
	}

	// выбор угла ближайшего к 45 градусам
	angle = fabs(angle0 - PI/4) < fabs(angle1 - PI/4) ? angle0 : angle1;
	return true;
}

// расчитывает время полета снаряда между точками
bool Ballistics::ComputeFlightTime(const Vector& v0, const Vector& v1, float startSpeed, float & time)
{
	float angle;
	if ( !ComputeShootAngle(v0, v1, startSpeed, angle) )
		return false;

	float distance = (v1-v0).GetLengthXZ();
	time = distance/(startSpeed*cos(angle));
	return true;
}

// расчитывает время полета снаряда между точками
float Ballistics::ComputeFlightTime(float distanceXZ, float angle, float startSpeed)
{
	return distanceXZ/(startSpeed*cos(angle));
}

// расчитывает положение снаряда в любой точке траектории
bool Ballistics::ComputePosition(const Vector& start, const Vector& end, float g, float startSpeed, float angle, float time, Vector & pos)
{
	Vector dir = end - start;
	dir.y = 0;
	dir.Normalize();

	pos.x = start.x + dir.x*time*startSpeed*cos(angle);
	pos.y = start.y + startSpeed*sin(angle)*time + g*time*time*0.5f;
	pos.z = start.z + dir.z*time*startSpeed*cos(angle);

	return true;
}

void Ballistics::DebugDraw(IRender & render, dword color)
{
	float fMaxTime = GetTotalFlightTime();
	float fTimeStep = fMaxTime * 0.05f;
	if( fTimeStep < 0.1f ) fTimeStep = 0.1f;

	RS_LINE lines[100];
	lines[0].dwColor = color;
	lines[0].vPos = v0_;
	long n=1;
	for( float fTime=fTimeStep; n<50; fTime += fTimeStep, n++ )
	{
		if( fTime > fMaxTime ) fTime = fMaxTime;

		lines[n*2].dwColor = color;
		ComputePosition(v0_, v1_, g_, startSpeed_, angle_, fTime, lines[n*2].vPos);

		lines[n*2-1].dwColor = color;
		lines[n*2-1].vPos = lines[n*2].vPos;

		if( fTime >= fMaxTime )
			break;
	}
	Matrix mWorldSave = render.GetWorld();
	render.SetWorld( Matrix() );
	render.DrawLines( lines, n, "dbgLine" );
	render.SetWorld(mWorldSave);
}
