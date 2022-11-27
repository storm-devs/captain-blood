#pragma once

#include "..\TextureMapper\TextureMapper.h"

#include "Trail.h"
#include "Register\Register.h"

class TrailManager : public ITrailManager
{
/*	struct Vertex
	{
	//	Vector p;
		float r;

		float tu;
		float tv;

		dword cl;

		Vector q;

		float az;
	};*/

	struct Vertex
	{
		Vector q;

		dword cl;

		float tu;
		float tv;

		float az;
		float r;
	};

	ShaderId Trail_id;


public:

	 TrailManager();
	~TrailManager();

public:

	ITrail *Add();

public:

	bool Create(MOPReader &reader);

	void Activate(bool isActive);
	void Show	 (bool isShow);

	MO_IS_FUNCTION(TrailManager, MissionObject);

	void Command(const char *id, dword numParams, const char **params);

private:

	void _cdecl Update(float dltTime, long level);
	void _cdecl Draw  (float dltTime, long level);

	void CreateBuffers();

	void _cdecl UpdateMapping(const char *group, MissionObject *sender);

private:

	struct QVertex
	{
		Vector pos;
		dword col;
		float ang;
		float z;
	};

	Plane frustum[6];

	IVBuffer *pVBuffer;
	IIBuffer *pIBuffer;

	IVariable *TrailTexture;
	IVariable *TrailBumpMap;

	IBaseTexture *pFace;
	IBaseTexture *pBump;

	Register<Block,100> reg;

	array<QVertex>	qverts;
	array<Trail *> trails;

	bool m_isMultiThreading;
	HANDLE m_hThread, m_hStartEvent, m_hExitEvent, m_hDoneEvent;
	float deltaTime;

	static dword __stdcall WorkThread(LPVOID lpParameter);
	void Execute(float dltTime);

	TextureMapper::Trails place;
	bool loaded;
};
