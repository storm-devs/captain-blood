#include "aiOfficerShootKick.h"
#include "../../Brains/aiBrain.h"
#include "../../../character/components/CharacterPhysics.h"
#include "../../../character/components/CharacterLogic.h"


aiOfficerShootKick::aiOfficerShootKick(aiBrain* Brain) : aiThought (Brain)
{	
}

aiOfficerShootKick::~aiOfficerShootKick()
{
}

void aiOfficerShootKick::Activate(Character* pChr, float fKickDistance)
{
	if (!pChr) return;

	pTarget = pChr;
	fDistance = fKickDistance;
	fIdleTime = 0.0f;
	fTimeToNextAttack =  Rnd(0.7f)+0.5f;// + Rnd(2.0f);;

	bHasKick = false;
	//GetBrain()->GetBody()->logic->Attack(pChr);

	CharacterLogic::State trg_st = pTarget->logic->GetState();

	if ( trg_st == CharacterLogic::state_strafe || trg_st == CharacterLogic::state_run)
	{		
		GetBrain()->GetBody()->logic->SetShootTarget(NULL,"Hit");
	}
	else
	{		
		GetBrain()->GetBody()->logic->SetShootTarget(pTarget,"Hit");
	}	

	aiThought::Activate();
}

bool aiOfficerShootKick::Process (float fDeltaTime)
{
	GetBrain()->OrinentToTarget();

	CharacterLogic::State st = GetBrain()->GetBody()->logic->GetState();	

	if (st == CharacterLogic::state_attack)
	{
		bHasKick = true;
	}


	if (st != CharacterLogic::state_attack && !bHasKick)
	{
		float k = Rnd();		
		
		if (k<0.25f)
		{
			GetBrain()->GetBody()->animation->ActivateLink("Attack1");
		}
		else
		if (k<0.5f)
		{
			GetBrain()->GetBody()->animation->ActivateLink("Attack2");
		}
		else
		if (k<0.75f)
		{		
			GetBrain()->GetBody()->animation->ActivateLink("Attack3");
		}
		else		
		{		
			GetBrain()->GetBody()->animation->ActivateLink("Attack4");
		}

		if (st == CharacterLogic::state_attack)
		{
			bHasKick = true;
		}
	}

	if (st != CharacterLogic::state_attack && bHasKick)
	{
		fIdleTime += fDeltaTime;

		if (fIdleTime > fTimeToNextAttack) return false;
	}

		

	return true;
}

void aiOfficerShootKick::DebugDraw (const Vector& BodyPos, IRender* pRS)
{
	//pRS->Print(BodyPos + Vector (0.0f, 1.9f, 0.0f), 10000.0f, 0.0f, 0xFFFFFFFF, "wait");
}

void aiOfficerShootKick::Stop ()
{
	bool bRes = GetBrain()->GetBody()->animation->ActivateLink("idle");
}

void aiOfficerShootKick::Hit ()
{
	//Отвечать, если бьют...
	fTimeToNextAttack = 0.05f;
}