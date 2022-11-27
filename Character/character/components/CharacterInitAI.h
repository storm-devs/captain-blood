
#ifndef _CharacterInitAI_h_
#define _CharacterInitAI_h_

#include "..\Character.h"


class CharacterInitAI
{	
public:
	
	const char* init_node;
	float argo_dist;
	float allarm_dist;
	float time_to_alarm;
	
	float sect_angle;
	float sect_cos_angle;

	float hear_dist;

	enum init_action_type
	{
		act_init_none = 0,
		act_init_waiting = 1,
		act_init_wait_and_alarm = 2,
		act_init_patrolling = 3,
		act_init_patrolling_and_alarm = 4
	};

	init_action_type init_action;	

	struct patrol_point
	{
		Vector pos;
		
		float wait_time;
		Vector dir;
		const char* wait_node;
	};

	array<patrol_point> patrol_points;

	CharacterInitAI(Character * character);
	~CharacterInitAI();

	void Init(MOPReader& reader);

private:

	Character & chr;	
};

#endif

