#ifndef AI_Params
#define AI_Params

#include "aiBrain.h"

class aiParams : public CharacterControllerParams
{

public:
	
	float fDistofView;	
	float fDistofViewPlayer;

	float fDistToLoseTarget;

	int   iNumPlayerAttckers;
	int   iNumNPCAttckers;

	bool  isStatist;

	ConstString WayPointsName;
	ConstString AIZoneName;
	ConstString Target;

	float fTimeToNextAttack;
	int   iMaxSimelouslyAttackers;

	const char*  attack_react;
	float attack_damage;

	float block_cooldown;

	array<aiBrain::Stage> stages;

	aiParams();
	~aiParams();
	
	virtual bool IsControllerSupport(const ConstString & controllerClassName);
	
	bool Create(MOPReader & reader);
	
	bool EditMode_Create(MOPReader & reader);
	
	bool EditMode_Update(MOPReader & reader);
	
	void Init (MOPReader & reader);	
};

#endif


