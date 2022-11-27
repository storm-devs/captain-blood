
#include "aiPittWarrior.h"
#include "aiBrain.h"
#include "../CharacterAI.h"
#include "../../character/components/CharacterLogic.h"
#include "../../character/components/CharacterPhysics.h"


aiPittWarrior::aiPittWarrior(Character & ch, const char* szName) : aiBrain (ch, szName)
{
	fDistofView = 30.0f;
	fDistofViewPlayer = 30.0f;

	iNumPlayerAttckers = 5;
	iNumNPCAttckers = 1;

	BrainState=EBS_IDLE;

	pChrTarget=NULL;

	static const ConstString defWayPoints("WayPoints"); 
	WayPointsName=defWayPoints;

	fTime_to_return_to_spawn = 5.0f;
			
	pPittRoll = (aiPittRoll*)AddThought(NEW aiPittRoll(this));		
		
	pThoughtKick->AddAction("Attack1");
	pThoughtKick->AddAction("Attack2");
	pThoughtKick->AddAction("Attack3");	
	pThoughtKick->AddAction("Attack4");
	pThoughtKick->AddAction("Combo1");
	pThoughtKick->AddAction("Combo2");
	pThoughtKick->AddAction("Combo3");
	pThoughtKick->PrepareActions();
}

aiPittWarrior::~aiPittWarrior()
{
}

void aiPittWarrior::ActivateTargetPairStateThought()
{
	pThoughtStrafing->Activate( (Character*)pChrTarget, 15.0f);
	BrainState=EBS_WAIT_FATALITY;
}

bool aiPittWarrior::AttackCharacter()
{
	float fDist=(GetBody()->physics->GetPos()-pChrTarget->physics->GetPos()).GetLengthXZ();

	if (fDist<=GetAttackDistance())
	{			
		Vector vPos = GetBody()->physics->GetPos();

		float dy=fabs(vPos.y - pChrTarget->physics->GetPos().y);

		if (dy>0.5f)
		{
			pChrTarget=NULL;

			ExecuteThoughts(0);

			BrainState=EBS_IDLE;

			Reset();
		}
		else
		{						
			float k = Rnd();

			if (k<0.25f)
			{
				pPittRoll->Activate(pChrTarget, 2.0f + Rnd(1.0f));
				BrainState=EBS_STRAFING;
			}
			else
			if (k<0.5f)
			{
				/*char pairLink[128];
				FatalityParams::TFatalityType type;

				if (GetBody()->logic->CheckPairLinks(pChrTarget,"pair",pairLink,type))
				{
					pPittFatality->Activate(pChrTarget);
					BrainState=EBS_FATALITY;
				}
				else*/
				{
					pThoughtKick->Activate(pChrTarget);
					BrainState=EBS_KICK;
				}
			}
			else
			{			
				pThoughtKick->Activate(pChrTarget);
				BrainState=EBS_KICK;
			}
		}
	}
	else			
	{
		pThoughtPursuit->Activate(pChrTarget->physics->GetPos(), WayPointsName, GetAttackDistance() * 0.85f, true);
		BrainState=EBS_PURSUIT;	
	}

	return true;
}

DeclareCharacterController(aiPittWarrior, "PittWarrior", 11)