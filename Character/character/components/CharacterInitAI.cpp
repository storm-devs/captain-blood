
#include "CharacterInitAI.h"
	
CharacterInitAI::CharacterInitAI(Character * character): chr(*character),patrol_points(_FL_)
{
	init_action = act_init_none;
}

CharacterInitAI::~CharacterInitAI()
{
	patrol_points.Empty();
}

void CharacterInitAI::Init(MOPReader& reader)
{	
	ConstString state = reader.Enum();

	init_action = act_init_none;

	struct StateEnum
	{
		ConstString id;
		init_action_type type;
	};

	static const StateEnum states[] =
	{
		{ConstString("waiting"), act_init_waiting},
		{ConstString("wait and alarm"), act_init_wait_and_alarm},
		{ConstString("patrolling"), act_init_patrolling},
		{ConstString("patrolling and alarm"), act_init_patrolling_and_alarm}
	};
	
	for(dword i = 0; i < ARRSIZE(states); i++)
	{
		if(states[i].id == state)
		{
			init_action = states[i].type;
			break;
		}
	}

	init_node = reader.String().c_str();
	argo_dist = reader.Float();
	allarm_dist = reader.Float();
	time_to_alarm = reader.Float();
	
	hear_dist = reader.Float();
	sect_angle = reader.Float();
	sect_angle *= PI/180.0f;

	sect_cos_angle = cosf(sect_angle);


	int count = reader.Array();

	patrol_points.Empty();

	for (int i=0;i<count;i++)
	{
		patrol_point* pt = &patrol_points[patrol_points.Add(patrol_point())];

		Vector pos = reader.Position();
		pt->pos = pos;
		pt->wait_time = reader.Float();
		pt->wait_node = reader.String().c_str();

		Matrix mat(reader.Angles(),0.0f);

		pt->dir = mat.vz;
	}
}