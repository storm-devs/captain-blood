#include "aiOfficerShootJumpBack.h"
#include "../../Brains/aiBrain.h"
#include "../../../character/components/CharacterPhysics.h"
#include "../../../character/components/CharacterLogic.h"

aiOfficerShootJumpBack::aiOfficerShootJumpBack(aiBrain* Brain) : aiThought (Brain)
{	
}

aiOfficerShootJumpBack::~aiOfficerShootJumpBack()
{
}

void aiOfficerShootJumpBack::Activate(Character* pChr)
{
	pTarget = pChr;

	bStartJumpBack = false;

	fTimetoAim = Rnd(0.15f) + 0.15f;

	bNeedToStop = false;
	bBarrage = false;

	aiThought::Activate();
}

bool aiOfficerShootJumpBack::Process (float fDeltaTime)
{	
	if (bNeedToStop) return false;

	CharacterLogic::State st = GetBrain()->GetBody()->logic->GetState();

	if (bStartJumpBack && st == CharacterLogic::state_attack)
	{
		bBarrage = true;

		Vector vTrgtPos = pTarget->physics->GetPos();
		Vector vMyPos = GetBrain()->GetBody()->physics->GetPos();	

		GetBrain()->GetBody()->physics->Orient(vTrgtPos);

		fTimetoAim -= fDeltaTime;

		if (fTimetoAim<0.0f)
		{
			fTimetoAim = 0.0f;

			CharacterLogic::State trg_st = pTarget->logic->GetState();

			if ( trg_st == CharacterLogic::state_roll)
			{
				GetBrain()->GetBody()->logic->SetShootTarget(NULL,"Hit");
			}
			else
			{
				GetBrain()->GetBody()->logic->SetShootTarget(pTarget,"Hit");
			}

			if (pTarget->IsPlayer())
			{			
				if (!GetBrain()->GetBody()->logic->InFrustrum(GetBrain()->GetBody()->physics->GetPos(),0.1f))
				{
					GetBrain()->GetBody()->logic->SetShootTarget(NULL,"Hit");
				}
			}

			GetBrain()->GetBody()->animation->ActivateLink("Shoot");
		}
	}
	else
	if (bStartJumpBack && st != CharacterLogic::state_shoot)
	{
		return false;
	}

	if (fTimetoAim == 0.0f)
	{	
		Vector vTrgtPos = pTarget->physics->GetPos();
		Vector vMyPos = GetBrain()->GetBody()->physics->GetPos();	

		GetBrain()->GetBody()->physics->Orient(vTrgtPos);
	}
	
	if (!bStartJumpBack && st != CharacterLogic::state_shoot)
	{
		float k = Rnd();
		
		/*if (k<0.5f)
		{		
			bStartJumpBack = GetBrain()->GetBody()->animation->ActivateLink("Walk Back");
		}
		else*/
		{			
			bStartJumpBack = GetBrain()->GetBody()->animation->ActivateLink("Jump Back Shoot");
		}
	}




	return true;
}

void aiOfficerShootJumpBack::Stop ()
{
	bool bRes = GetBrain()->GetBody()->animation->ActivateLink("idle");
}

void aiOfficerShootJumpBack::Hit()
{
	if (bBarrage)
	{
		bNeedToStop = true;
	}
}