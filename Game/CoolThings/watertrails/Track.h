#pragma once

#include "..\TextureMapper\TextureMapper.h"

#include "..\..\..\Common_h\Mission.h"
#include "RingBuffer.h"

#define BORT_FOAM_ARRAY_SIZE 5
#define TRAIL_SIZE 256

class WaterTrack : public MissionObject
{
	struct Bake
	{
		Vector p;
		Vector n; float len;

		Vector q;
		Vector m;

		float time;
	};

	struct Node
	{
		Vector p;
		Vector t; float len;
	};

	struct Vert
	{
		Vector p; dword c;

		float u;
		float v;
	};

	struct Drop
	{
		float time; bool busy;

		Vector p,q;
		Vector v,w;

		Vector a;
	};

/*	struct Vertex
	{
		Vector p; dword cl;

		float tu;
		float tv;

		Vector q;
	};*/

private:

	ShaderId SeaTrack_id;
	ShaderId Fly_id;

	bool bDrawDebug;

	float fSpeedFactor;
	float fAlphaScale;

	bool bPrevPosInited;
	Vector vPrevPos;

	Matrix objectMatrix;

	MOSafePointer pMaster;

	IParticleSystem* BurunParticle;
	IParticleSystem* BurunBackParticle;

	Vector offset;
	Vector back_offset;
	float fTrackHeight;

	float ExpansionSpeed;
	float BornExpansion;

	float maxDistToBorn;

//	Vector leftBort[BORT_FOAM_ARRAY_SIZE];
//	Vector rightBort[BORT_FOAM_ARRAY_SIZE];

	Vector curPos;
	Vector backPos;
	Vector leftBort_transformed[BORT_FOAM_ARRAY_SIZE];
	Vector rightBort_transformed[BORT_FOAM_ARRAY_SIZE];

	struct PathPoint
	{
		Vector p;
		Vector nrm;
		float time;
	//	float glob;
		float speed;
		float fDelta;
		float v;
		int i;
		float a;
		float b;
		float k1;
		float k2;
	};

	float m_delta;

	ringBuffer<TRAIL_SIZE, PathPoint> ObjectPath;

	float fTime;

	int m_index;	

	IVariable * trackTex1;
	IVariable * trackTex2;
	IVariable * trackTexBlend;
	IBaseTexture * pTrackTexture[4];

	IBaseTexture * pTrailTexture;


	float fTeleportDistance;

	float frame;

	float slide;


	float fGlobalScale;


	float fGlobalTime;


	bool bSmoothSpeedInited;
	float fSmoothSpeed;

	float fFoamAnimWidth;

	float FoamLifeTime;

	float currentV;

	bool bWorkOnlyWithActive;
	bool bWorkOnlyWithVisible;

public:

	 WaterTrack();
	~WaterTrack();

	//Создание объекта
	virtual bool Create			(MOPReader &reader);

	//Инициализировать объект
	virtual bool EditMode_Create(MOPReader &reader);
	//Обновление параметров
	virtual bool EditMode_Update(MOPReader &reader);

	//Получить матрицу объекта
	virtual Matrix &GetMatrix(Matrix &mtx);

	void _cdecl Work(float fDeltaTime, long level);

	void Activate(bool bIsActive);

	virtual void Restart();

	void _cdecl UpdateMapping(const char *group, MissionObject *sender);
	
private:

	void BuildPath(long count);

	Drop *GetFreeDrop();

//	void CreateBuffers();

private:

//	array<Vector> ObjectPath;

	float waterLevel;

	Node beg;
	Node end;

	array<Bake> bakes;
	array<Vert> verts;

	array<Vert> burun;

	float m_speedK;
	float m_speed;

//	IVBuffer *pVBuffer;
//	IIBuffer *pIBuffer;

	class TrackManager *manager;

	array<Drop> drops;

	IVariable	 *Texture;
	IBaseTexture *pFace;

	Matrix bakesMatrix;

	float m_k1;
	float m_k2;

	ConstString MissionObjectName;

	bool noSwing;

	TextureMapper::Water place;
	bool loaded;

};
