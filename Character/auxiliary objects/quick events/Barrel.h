#ifndef Barrel_H
#define Barrel_H

#include "QuickEvent.h"

class Barrel : public QuickEvent
{
public:

	Vector cur_pl_offset;

	struct TBarrelWay
	{
		int num_points;
		Vector points[5];
	};
	
	int num_ways;
	TBarrelWay ways[5];

	int iSelWay;

	bool   dir_changed;
	Vector init_pos;
	int    barrel_stage;
	int    cur_way_pt;
	float  fSpeed;
	float  distance;
	float  segment_lenght;
	Vector orient;
	Vector new_orient;
	float  angle;
	Matrix center_matrix;
	float  fRadius;	
	float  fAcceleration;

	virtual void InitData();
	virtual bool EditMode_Update(MOPReader & reader);	

	virtual void _cdecl EditMode_Draw(float dltTime, long level);

	virtual void BeginQuickEvent();	
	virtual void QuickEventUpdate(float dltTime);
	virtual void Interupt(bool win_game);			
	virtual void AnimEvent(const char * param);	

	virtual const char* GetName() { return "Barrel";};

	static const char * comment;
};

#endif