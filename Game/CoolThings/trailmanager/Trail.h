#pragma once

#include "..\..\..\Common_h\ITrail.h"

struct Block
{
	Vector pos;
	Vector off;

	Vector force;

	Vector windDir;

	bool show;

	float time;
	float live;

	float rk;

	float r;

	float cl;

	dword color;

	int i;

	Block()
	{
		show = false;
	}
};

class Trail : public ITrail
{
	void reset();

public:

	Trail()
	{
		reset();
	}

	void SetParams(
		float  rBeg, float rEnd,
		float  minDelay,
		float  maxDelay,
		dword  color = 0xffffffff,
		float  offsetStrength = 1.0f,
		float  windVel = 0.0f,
		Vector windDir = Vector(0.0f,1.0f,0.0f),
		float  forceValue = 0.0f);

	void EnableStaticLive(bool enable);

	void Reset();

	void Show(bool isShow);
	void Update(const Matrix &m);

	void Release();

public:

	bool show;
	bool staticLive;

	float rBeg;
	float rEnd;

	Vector windDir;

	float forceVal;

	float offStr;

	float  lr;
	Matrix lm;
	int	   li;

	bool started;

	dword color;

	float liveMin;
	float liveMax;

	Matrix m;

	Vector beg;
	Vector end;

	float time;
	
	int cnt;

	float rad;

	bool busy;

};
