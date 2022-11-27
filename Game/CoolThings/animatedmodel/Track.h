#pragma once

#include "..\..\..\Common_h\Mission.h"

enum MoveMode
{
	move_arrive,
	move_wait,
	move_linear,
	move_step
};

struct Bake
{
//	float t;

	Vector pos;
	Vector up;
	Vector look;

	float time;

	float za;
	float ya;
	float xa;

	float sc;

	Vector zp;
	Vector yp;
	Vector xp;

	Vector sp;
};

class Node
{
public:

//	Node() : bakes(_FL_,64)
	Node() : bakes(_FL_,16)
	{
	}

public:

	void Init(MOPReader &reader);

public:

	array<Bake> bakes;

	Vector pos;

	Vector tangent;
	Vector binormal;

	Vector normal;
	Vector vx;

	float za,zt,zl;
	float ya,yt,yl;
	float xa,xt,xl;

	float sc,st,sl;

	Vector zp;
	Vector yp;
	Vector xp;

	Vector sp;

	Vector ztg;
	Vector ytg;
	Vector xtg;

	Vector stg;

	float time;
	float len;

	float timeTotal;

	float smooth;

	const char *node;
//	string realNode;

	MoveMode moveMode;

	float blendTime;

	MissionTrigger event;
};

enum LookMode
{
	look_static,
	look_follow,
	look_full
};

class Track
{
public:

	Track() : nodes(_FL_)
	{
	}

public:

	void DrawPath(dword i1, dword i2, IRender &render, Matrix &matWorld, bool showRotation, bool showScale);
	void GetNodeMatrix(Matrix &m, Node &node, Matrix &matWorld, Matrix &matAngle, float modelScale);

	void CreateData();

public:

	array<Node> nodes; bool looped;

	LookMode lookMode;

	int focusedNode; float delay;

};
