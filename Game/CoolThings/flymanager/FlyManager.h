#pragma once

#include "..\TextureMapper\TextureMapper.h"

#include "Fly.h"

class FlyManager : public IFlyManager
{
	struct Vertex
	{
	//	Vector p;
		float px;
		float py;

		float tu;
		float tv;

	//	Color cl;
		float al;

		Vector q;
	};


	ShaderId Fly_id;

public:

	 FlyManager();
	~FlyManager();

public:

	bool Create(MOPReader &reader);

	static IFlysCloud *CreateFlys(IMission &mission, float radius, dword count, bool playSound = true);

	IFlysCloud *CreateFlys(float radius, dword count, bool playSound = true);

private:

	Swarm *GetFreeSwarm();
	Cloud *GetFreeCloud();

private:

	void _cdecl Draw(float dltTime, long level);

	void CreateBuffers();

	void _cdecl UpdateMapping(const char *group, MissionObject *sender);

private:

	array<Cloud> clouds;
	array<Swarm> swarms;

	IVBuffer *pVBuffer;
	IIBuffer *pIBuffer;

	IVariable	 *FlyTexture;
	IBaseTexture *pFace;

	TextureMapper::Flys place;
	bool loaded;

	bool debug;
	bool debugDown;

};
