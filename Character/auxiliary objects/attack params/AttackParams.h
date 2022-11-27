#ifndef ATTACK_PARAMS_H
#define ATTACK_PARAMS_H

#include "..\..\..\Common_h\Mission.h"
#include "..\..\ai\Util\aiActionStack.h"

class AttackParams : public MissionObject
{	
public:

	array<AttackParam> attacks;

	struct attacks_params_state
	{
		array<bool> attack_used;		

		attacks_params_state():attack_used(_FL_)
		{
		};
	};	

	AttackParams();
	virtual ~AttackParams();
	
	virtual bool Create(MOPReader & reader);
	
	virtual bool EditMode_Update(MOPReader & reader);


	MO_IS_FUNCTION(AttackParams, MissionObject);

	void Init(attacks_params_state& params_state);
	void Update(float hp, attacks_params_state& params_state, aiActionStack* actionStack);

	static const char * comment;

};

#endif