#pragma once

#include "..\..\common_h\core.h"
#include "..\..\common_h\math3d\vector.h"

#include "..\..\common_h\IForce.h"

#define FORCE_PLAY  0.0f
#define FORCE_STOP -1.0f

struct Head
{
	long ln;
	long rn;

	Head(long l = 0, long r = 0) : ln(l),rn(r)
	{
	}

	operator void *()
	{
		return &ln;
	}

	int Size()
	{
		return 2*sizeof(long);
	}
};
/*
struct Node
{
	float t;
	float f;
};
*/
typedef Vector Node;

struct Data
{
	Node *lp;
	Node *rp; int size;

	Data(Head &h)
	{
		lp = NEW Node[size = h.ln + h.rn];
		rp = lp + h.ln;
	}

	operator void *()
	{
		return lp;
	}

	int Size()
	{
		return size*sizeof(Node);
	}

	void Release()
	{
		DELETE_ARRAY(lp)
	}
};

struct ForceData
{
	ForceData()
	{
		ls = null;
		rs = null;
	}

//	string name;

	Vector *ls; long ln;
	Vector *rs; long rn;

	void Release()
	{
		DELETE_ARRAY(ls)
	}
};

struct Force : public IForce
{
	long deviceIndex; bool autoRelease;

/*	const Vector *ls; long ln;
	const Vector *rs; long rn;*/

	const ForceData *data;

	float time;

	bool busy; bool stopped;

	const class IControls *instance;

/*	Force() :
		ls(_FL_),
		rs(_FL_)
	{
	}*/

/*	void Play();
	void Stop();

	float GetPosition();

	void Release();*/

	void Play() { time = FORCE_PLAY; stopped = false; }
	void Stop()	{ time = FORCE_STOP; stopped = true;  }

	float GetPosition()
	{
		return time;
	}

	void Release()
	{
		busy = false; stopped = true;
	}
};
