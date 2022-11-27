#ifndef ChestPattern_H
#define ChestPattern_H

#include "..\BasePattern\BasePattern.h"
#include "QuickEvent.h"

class ChestPattern : public BasePattern
{
public:	
	
	const char* model_name;
	const char* anim_name;
	
	Vector loc_pos;
	Vector loc_angles;
	Vector loc_dropPosition;
	float dropMinAngle;
	float dropMaxAngle;
	float dropMinVy;
	float dropMaxVy;
	float dropMinVxz;
	float dropMaxVxz;
	float friction;

	float radius_activation;
	ConstString player_node;
	const char* button_name;

	float player_distance;
	
	QuickEvent::TColider colider;

	ConstString particle_name;	
	Vector particle_loc_pos;
	Vector particle_loc_angels;	

	ConstString button_widget_name;

	bool dyn_light;
	bool shadow_cast;
	bool shadow_recive;
	
	virtual bool EditMode_Update(MOPReader & reader);

	MO_IS_FUNCTION(ChestPattern, MissionObject);

	static const char * comment;
};

#endif