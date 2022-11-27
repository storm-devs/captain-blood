
#include "AttackParams.h"

AttackParams::AttackParams(): attacks(_FL_)
{
	
}

AttackParams::~AttackParams()
{
}

bool AttackParams::Create(MOPReader & reader)
{
	Activate(true);	
	EditMode_Update (reader);	

	return true;
}

bool AttackParams::EditMode_Update(MOPReader & reader)
{	
	attacks.DelAll();

	int count = reader.Array();

	for (int i=0;i<count;i++)
	{
		AttackParam & attack = attacks[attacks.Add()];

		attack.min_hp = reader.Float()*0.01f;
		attack.max_hp = reader.Float()*0.01f;

		attack.link = reader.String().c_str();		

		attack.maxDistance = reader.Float();
		attack.minDistance = reader.Float();

		attack.minCooldown = reader.Float();
		attack.maxCooldown = reader.Float();
	}

	return true;
}

void AttackParams::Init(attacks_params_state& params_state)
{
	params_state.attack_used.DelAll();
	params_state.attack_used.AddElements(attacks.Size());

	for (int i=0;i<(int)attacks.Size();i++)
	{
		params_state.attack_used[i] = false;
	}
}

void AttackParams::Update(float hp, attacks_params_state& params_state, aiActionStack* actionStack)
{
	bool need_update = false;

	for (int i=0;i<(int)attacks.Size();i++)
	{
		if (attacks[i].min_hp <= hp && hp <= attacks[i].max_hp)
		{
			if (!params_state.attack_used[i])
			{
				need_update = true;
				break;
			}
		}
		else
		{
			if (params_state.attack_used[i])
			{
				need_update = true;
				break;
			}
		}
	}

	if (need_update)
	{
		actionStack->Reset();		

		for (int i=0;i<(int)attacks.Size();i++)
		{
			if (attacks[i].min_hp <= hp && hp <= attacks[i].max_hp)
			{
				actionStack->AddAction(&attacks[i]);
				params_state.attack_used[i] = true;
			}
			else
			{
				params_state.attack_used[i] = false;
			}
		}

		actionStack->Prepare();
	}
}

static const char * comment;
const char * AttackParams::comment = 
"Attack Params";

MOP_BEGINLISTG(AttackParams, "Attack Params", '1.00', 50, "Character")
	
MOP_ARRAYBEGC("Attacks", 0, 1000, "Attacks")
		MOP_FLOATEX("Min HP", 0.0f, 0.0f, 100.0f)
		MOP_FLOATEX("Max HP", 100.0f, 0.0f, 100.0f)
		MOP_STRINGC("Attack Link", "", "Attack Link")	
		MOP_FLOATEX("Max Distance", 2.5f, 0.0f, 100.0f)
		MOP_FLOATEX("Min Distance", 0.0f, 0.0f, 100.0f)
		MOP_FLOATEX("Min Cooldown", 0.0f, 0.0f, 1000.0f)
		MOP_FLOATEX("Max Cooldown", 0.0f, 0.0f, 1000.0f)
MOP_ARRAYEND

MOP_ENDLIST(AttackParams)