#pragma once

#include "..\TextureMapper\TextureMapper.h"

#include "..\..\..\Common_h\IStainManager.h"

class StainManager : public IStainManager
{
	struct Stain
	{
		array<Vector> list;
		array<Vector> vert;
		array<Matrix> locs;

		dword type;

		Vector pos;
		Vector dir;

		Vector center;

		float time;
		float a;

		Vector4 t;

		float del;

		float i,j;

		bool busy;

	//	Matrix m;

	//	enum {max_tri =  8};
	//	enum {max_tri = 16};
		enum {max_tri = 24};

		Stain() :
			list(_FL_,max_tri*3),
			vert(_FL_,max_tri*3),
			locs(_FL_,max_tri)
		{
			busy = false;
		}
	};

	struct Vertex
	{
		Vector p; 
		dword c;

		float u,v;

		Vector x;
		Vector y;
		Vector z;

		Vector4 t;

		float del;

		float i,j;

		float power;
		float value;
	};

	ShaderId BloodStain_id;

private:

	struct Point
	{
		Vector p; int a,b; bool r;

		Point()
		{
		}

		Point(const Vector &p, int a, int b)
			: p(p),a(a),b(b),r(false)
		{
		}
	};

	void Clip(
		const Vector &a,
		const Vector &b,
		const Vector &c, array<Point> &res, float scale);

public:

	 StainManager();
	~StainManager();

public:

	bool Create(MOPReader &reader);

	void Show	 (bool isShow);
	void Activate(bool isActive);
	
public:

	void AddStain(
		const Vector &pos, const Vector &dir, float rad = 1.5f, float scale = 1.0f, StainType type = Blood);

	void ClearRad(
		const Vector &pos, float rad, float time = 0.0f);

private:

	void _cdecl Draw(float dltTime, long level);

	void CreateBuffers();
	void UpdateBuffers();

	Stain *GetFreeStain();

	void _cdecl UpdateMapping(const char *group, MissionObject *sender);

private:
	struct StainQueued
	{
		Vector pos;
		Vector dir;
		float rad;
		float scale;
		StainType type;
	};

	array<StainQueued> stainsQueued;
	array<Stain>				  stains;
	array<PhysTriangleMaterialID> materials;

	int stainsCount;
	int    triCount;

	array<Point> list;

	IVBuffer *pVBuffer;
	int bufCount;

	bool updateBuffers;

	float stainResp;

	IVariable *Time;
	IVariable *K;

	IVariable *DiffMap;
	IVariable *NormMap;

	IVariable *ShadMap;

	IBaseTexture *pDiff;
	IBaseTexture *pNorm;

	IBaseTexture *pShad;

	float curTime;

	TextureMapper::Stains place;
	bool loaded;

	bool deb;
	bool debDown;

	void AddStain(const StainQueued & stQueue);
};
