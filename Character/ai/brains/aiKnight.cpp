#include "aiKnight.h"
#include "aiBrain.h"
#include "../CharacterAI.h"
#include "../../character/components/CharacterLogic.h"
#include "../../character/components/CharacterPhysics.h"

#include "aiBossParams.h"


aiKnight::aiKnight(Character & ch, const char* szName) : aiBrain (ch, szName)
{
	fDistofView = 30.0f;
	fDistofViewPlayer = 30.0f;

	iNumPlayerAttckers = 5;
	iNumNPCAttckers = 1;

	BrainState=EBS_IDLE;

	pChrTarget=NULL;

	fTime_to_return_to_spawn = 0.0f;

	static const ConstString defWayPoints("WayPoints"); 
	WayPointsName=defWayPoints;
	
	pKnightRage =       (aiKnightRage*)AddThought(NEW aiKnightRage(this));	

	pThoughtKick->AddAction("Attack1");
	pThoughtKick->AddAction("Attack2");
	pThoughtKick->AddAction("Attack3");	

	pThoughtKick->PrepareActions();
}

aiKnight::~aiKnight()
{
}

void aiKnight::ChangeStage(int stage, const char* param)
{
	if (stage==1)
	{
		chr.animation->Goto(param,0.2f);
	}
}

void aiKnight::ActivateTargetPairStateThought()
{
	pThoughtStrafing->Activate( (Character*)pChrTarget, 15.0f);
	BrainState=EBS_WAIT_FATALITY;
}

bool aiKnight::AttackCharacter()
{
	float fDist=(GetBody()->physics->GetPos()-pChrTarget->physics->GetPos()).GetLengthXZ();

	pThoughtKick->CheckDist(fDist);
	if (fDist<=GetAttackDistance())
	{			
		pThoughtKick->Activate(pChrTarget);
		BrainState=EBS_KICK;		
	}
	else
	{
		pThoughtPursuit->Activate(pChrTarget->physics->GetPos(), WayPointsName, PursuitDist(), true);
		BrainState=EBS_PURSUIT;
	}

	return true;
}

void aiKnight::Hit(float dmg,Character* _offender, const char* reaction, DamageReceiver::DamageSource source)
{	
	if (BrainState!=EBS_RAGE &&	string::IsEqual(reaction,"kicked") && GetCurStage() == 1)
	{		
		Reset();
			
		ExecuteThoughts(0);

		BrainState=EBS_IDLE;				

		if (_offender != NULL)
		{
			pChrTarget = _offender;

			pKnightRage->Activate(pChrTarget);
			BrainState=EBS_RAGE;	
		}
	}

	if (chr.logic->GetAbsoluteHP()<0.1f)
	{
		Reset();
		
		BrainState=EBS_IDLE;

		GetBody()->animation->ActivateLink("to stun",true);
		return;
	}
}

DeclareCharacterController(aiKnight, "Knight", 11)