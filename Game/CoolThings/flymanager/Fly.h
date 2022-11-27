#pragma once

#include "..\..\..\Common_h\Mission.h"
#include "..\..\..\Common_h\IFly.h"

struct Fly
{
	Vector pos;

	Vector beg;
	Vector end; float time;

	bool in;
};

struct Cloud;

struct Swarm
{
//	static int total;

	ISound3D *sound;

	array<Fly> flys;

	Vector pos; float r;

	float a;
	float delay;

	float start;

	float k;

	bool busy;

	bool play;

	 Swarm() : flys(_FL_,1)
	{
		sound = null;
	//	cloud = null;

		busy = false;

		r = 0.0f;
	}
	~Swarm()
	{
		RELEASE(sound)
	}

	void Init(float radius, dword count, bool playSound);

	void Update(float dltTime);
	void Move  (float dltTime);

	void SetPosition(const Vector &pos);
	void SetAlpha(float alpha);

	void Release(bool isFade);

//	Cloud *cloud;
};

struct Cloud : public IFlysCloud
{
//	static int total;

	Swarm *swarm;

	Vector pos; float r;

	float a;
	float delay;

	dword cnt;

	bool busy;

	bool play;

	Cloud()
	{
		busy = false;
	}

	void Init(float radius, dword count, bool playSound)
	{
		delay = 0.0f;

		pos = 0.0f;

		r = radius;
		a = 1.0f;

		cnt = count;

		play = playSound;

		swarm = null;
	}

public:

	void SetPosition(const Vector &pos);
	void SetAlpha(float alpha);

	void Release(bool isFade);

};
