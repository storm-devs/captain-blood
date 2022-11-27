#pragma once

#include "..\TextureMapper\TextureMapper.h"

#include "..\..\..\Common_h\Mission.h"

class LocalEffect : public MissionObject, public IRenderFilter
{
	struct Vertex
	{
		Vector p;

		float u1,v1;
		float u2,v2;

		float a;
		float b;

//		Color c;
	};

	struct Drop
	{
		float time;
		float x;
		float y;
		float angCos;
		float angSin;
		union
		{
			float spd;
			dword isBusy;
		};
	};

	struct Verte_
	{
		Vector p;

		float tu;
		float tv;
	};

	struct Smoke
	{
		struct Vertex
		{
			Vector p;

			float u1,v1;
			float u2,v2;

			float bl;

			Color cl;

			float az;

			Vector q;
		};

		IVBuffer *pVB;
		IIBuffer *pIB;

		IVariable *Texture; IBaseTexture *pFace;

		bool use;

		int clouds; Color color;
	};

	struct Cloud
	{
		Vector p;

		float time;
		float live;

		int i;

		float rs;
	};

	struct Line
	{
		Vector p; bool busy;
	};

	ShaderId SimpleQuad_id;
	ShaderId LocalEffect_id;
	ShaderId Cloud_id;
	ShaderId LocalLines_id;

public:

	 LocalEffect();
	~LocalEffect();

public:

	bool Create			(MOPReader &reader);
	bool EditMode_Update(MOPReader &reader);

	void Restart();

	void Activate(bool isActive);
	void Show	 (bool isShow);

	void Command(const char *id, dword numParams, const char **params);

public:

	void FilterImage(IBaseTexture *source, IRenderTarget *destination);

private:

	void _cdecl Draw(float dltTime, long level);

	void CreateBuffers();
	void InitParams		(MOPReader &reader);

	void Generate();

	Drop *GetDrop();

	void _cdecl UpdateMapping(GroupId group, MissionObject *sender);

private:

	IVBuffer *pVBuffer;
	IIBuffer *pIBuffer;

	IRender *pRS;

	IVariable *NatMap;

	IVariable *NorMap; IBaseTexture *pNor;
	IVariable *DifMap; IBaseTexture *pDif;

	float time;
	float time_remain;
	float kAspect;

	float deltaTime;

	array<Drop> drops;
	array<Line> lines;

	IVBuffer *pVB;
	IIBuffer *pIB;

	Matrix dir;
	float  vel;

	long count;

	bool rain;

	Smoke smoke;

	array<Cloud> clouds;

	bool m_restart;

	TextureMapper::Drops  dr_place;
	TextureMapper::Clouds cl_place;

	bool loaded;

};
