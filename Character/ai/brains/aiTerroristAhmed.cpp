#include "aiTerroristAhmed.h"
#include "aiBrain.h"
#include "..\CharacterAI.h"
#include "../../character/components/CharacterLogic.h"
#include "../../character/components/CharacterPhysics.h"
/*
aiTerroristAhmed::aiTerroristAhmed(Character & ch, const char* szName) : aiBombardeer (ch, szName)
{		
	fDistofView = 10;
	BrainState=EBS_IDLE;
	static const ConstString defWayPoints("WayPoints"); 
	WayPointsName=defWayPoints;	
			
	pThoughtKick->AddAction("suicide rush");
	pThoughtKick->AddAction("suicide catch");	
	pThoughtKick->PrepareActions();	

	pThoughtStanding->AddAction("Angre");
	pThoughtStanding->AddAction("Klich");
	pThoughtStanding->AddAction("Pot");	
	pThoughtStanding->PrepareActions();	
}

aiTerroristAhmed::~aiTerroristAhmed()
{
}

void aiTerroristAhmed::ActivateTargetPairStateThought()
{
	pThoughtStrafing->Activate( (Character*)pChrTarget, 15.0f);
	BrainState=EBS_WAIT_FATALITY;
}

void aiTerroristAhmed::TickWhenEmpty()
{	
	BrainState=EBS_IDLE;
}

bool aiTerroristAhmed::AttackCharacter()
{	
	SuicideThought();

	return true;
}

void aiTerroristAhmed::Hit(float dmg,Character* _offender, const char* reaction)
{
	if (BrainState!=EBS_KICK && BrainState!=EBS_PLACEBOMB && BrainState!=EBS_SUICIDE)
	{
		Reset();
			
		ExecuteThoughts(0);

		BrainState=EBS_IDLE;		

		pChrTarget=SearchTarget(false);

		if (pChrTarget!=NULL)
		{
			float fDist=(GetBody()->physics->GetPos()-pChrTarget->physics->GetPos()).GetLengthXZ();

			if ((GetBody()->physics->GetPos()-pChrTarget->physics->GetPos()).GetLengthXZ()<=GetAttackDistance())
			{
				pThoughtKick->Activate(pChrTarget);
				BrainState=EBS_KICK;
			}
		}
	}
}

DeclareCharacterController(aiTerroristAhmed, "TerroristAhmed", 11)*/