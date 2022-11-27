

#include "aiParams.h"
#include "..\CharacterAI.h"


aiParams::aiParams():stages(_FL_)
{
}

aiParams::~aiParams()
{
}

bool aiParams::IsControllerSupport(const ConstString & controllerClassName)
{
	CHECK_CONTROLLERSUPPORT("aiParams")
}

bool aiParams::Create(MOPReader & reader)
{
	Init(reader);
	return true;
}

bool aiParams::EditMode_Create(MOPReader & reader)
{
	Init(reader);
	return true;
}

bool aiParams::EditMode_Update(MOPReader & reader)
{
	Init(reader);
	return true;
}

void aiParams::Init(MOPReader & reader)
{		
	fDistofView=reader.Float();
	fDistofViewPlayer=reader.Float();
	fDistToLoseTarget = reader.Float();

	iNumPlayerAttckers=reader.Long();
	iNumNPCAttckers=reader.Long();

	WayPointsName=reader.String();

	isStatist = reader.Bool();

	attack_react=reader.String().c_str();
	attack_damage=reader.Float();

	block_cooldown=reader.Float();

	Target=reader.String();

	AIZoneName=reader.String();

	int count = reader.Array();

	for (int i=0; i<count; i++)
	{
		aiBrain::Stage& stg = stages[stages.Add()];

		stg.hp = reader.Float() * 0.01f;
		stg.param = reader.String().c_str();		
	}

	fTimeToNextAttack = reader.Float();
	iMaxSimelouslyAttackers=reader.Long();	
}

MOP_BEGINLISTCG(aiParams, "AI params", '1.00', 90, "AI Params", "Character")
		
	MOP_FLOATEX("Dist of View", 12.0f, 3.0f, 1000.0f)
	MOP_FLOATEX("Dist of View Player", 12.0f, 3.0f, 1000.0f)
	MOP_FLOATEX("Dist to Lose Target", 15.0f, 3.0f, 1000.0f)

	MOP_LONGEX("MAX Player Attckers", 5, 1, 20)
	MOP_LONGEX("MAX NPC Attckers", 1, 1, 20)

	MOP_STRING("WayPoints Name", "WayPoints")


	MOP_BOOL("isStatist", false)

	MOP_STRING("Statist Attack Reaction", "Hit")
	MOP_FLOATEX("Statist Attack Damage", 1.0f, 1.0f, 1024.0f)

	MOP_FLOATEX("Block cooldown | v2 |", -1.0f, -1.0f, 1024.0f)

	MOP_STRING("Target", "")	

	MOP_STRING("AIZone", "")

	MOP_ARRAYBEG("Stages",0,5)
		MOP_FLOATEX("HP border", 90, 0, 100)
		MOP_STRING("Param", "")
	MOP_ARRAYEND

	MOP_FLOATEX("Time To Next Attack", 5.0f, 0.01f, 1000.0f)
	MOP_LONGEX("Max Simelously Attackers", 1, 1, 20)
	

MOP_ENDLIST(aiParams)