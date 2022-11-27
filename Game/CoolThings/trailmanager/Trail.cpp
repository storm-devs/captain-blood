#include "Trail.h"

void Trail::reset()
{
	show = true;
	staticLive = false;

	rBeg = 0.2f;
	rEnd = 2.5f;

	lr = RRnd(0.5f,1.5f)*rBeg*0.5f;
	li = 0;

	color = 0xffffffff;

	busy = false;

	beg = 0.0f;
	end = 0.0f;

	cnt = 0;

	rad = 0.0f;

	started = true;

	m.SetIdentity();
}

void Trail::SetParams(
	float  rBeg, float rEnd,
	float  minDelay,
	float  maxDelay,
	dword  color,
	float  offsetStrength,
	float  windVel,
	Vector windDir,
	float  forceValue)
{
	if( rBeg < 0.1f )
		rBeg = 0.1f;

	if( rEnd < 0.1f )
		rEnd = 0.1f;

	this->rBeg = rBeg;
	this->rEnd = rEnd;

	lr = RRnd(0.5f,1.5f)*rBeg*0.5f;

	this->color = color;

	liveMin = minDelay;
	liveMax = maxDelay;

	this->windDir = windDir*windVel;

	forceVal = forceValue;

	offStr = offsetStrength;

	beg = 0.0f;
	end = 0.0f;

	cnt = 0;

	rad = rBeg*0.5f;

	started = true;
}

void Trail::EnableStaticLive(bool enable)
{
	staticLive = enable;
}

void Trail::Reset()
{
	lr = RRnd(0.5f,1.5f)*rBeg*0.5f;

	started = true;
}

void Trail::Show(bool isShow)
{
	this->show = isShow;
}

void Trail::Update(const Matrix &m)
{
	this->m = m;
}

void Trail::Release()
{
	reset();
}
