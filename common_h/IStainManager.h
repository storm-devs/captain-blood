#pragma once

#include "Mission.h"

class IStainManager : public MissionObject
{
public:

	enum StainType {Blood, Soot};

public:
	
/*	static void AddStain(IMission &mis, const Vector &pos, const Vector &dir, float rad = 1.5f)
	{
		IStainManager *p = (IStainManager *)mis.CreateObject("StainManager", "StainManager");

		if( p )
			p->AddStain(pos,dir,rad);
	}*/

	virtual void AddStain(
		const Vector &pos, const Vector &dir, float rad = 1.5f, float scale = 1.0f, StainType type = Blood) = 0;

	virtual void ClearRad(
		const Vector &pos, float rad, float time = 0.0f) = 0;

};
