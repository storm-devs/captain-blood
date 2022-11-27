#include "aiPittFatality.h"
#include "../../Brains/aiBrain.h"
#include "../../../character/components/CharacterPhysics.h"
#include "../../../character/components/CharacterLogic.h"


aiPittFatality::aiPittFatality(aiBrain* Brain) : aiThought (Brain)
{	
}

aiPittFatality::~aiPittFatality()
{
}

void aiPittFatality::Activate(Character* pChr)
{
	pTarget = pChr;	
	fIdleTime = 0.0f;
	fTimeToNextAttack =  Rnd(0.7f)+0.5f;// + Rnd(2.0f);;

	bHasKick = false;
	//GetBrain()->GetBody()->logic->Attack(pChr);

	bAllowRotate = false;

	bDoFatality = false;

	fTimeToFatality = 0.0f;

	GetBrain()->bAllowUpdateInPairModel=true;

	aiThought::Activate();
}

bool aiPittFatality::Process (float fDeltaTime)
{
	CharacterLogic::State st = GetBrain()->GetBody()->logic->GetState();

	//if (st==CharacterLogic::state_attack)
	{	
		//if (bAllowRotate)
		{
			Vector vTrgtPos = pTarget->physics->GetPos();
			Vector vMyPos = GetBrain()->GetBody()->physics->GetPos();	

			GetBrain()->GetBody()->physics->Orient(vTrgtPos);
		}
	}
	

	if (st == CharacterLogic::state_findpair && !bHasKick)
	{
		bHasKick = true;
	}
	
	if (st != CharacterLogic::state_findpair && st != CharacterLogic::state_topair && !bHasKick)
	{
		GetBrain()->GetBody()->animation->ActivateLink("C");

		if (st == CharacterLogic::state_findpair)
		{
			bHasKick = true;
		}
	}

	if (st!= CharacterLogic::state_findpair && bHasKick && !bDoFatality)
	{		
		fTimeToFatality +=fDeltaTime;

		if (fTimeToFatality>0.25f)
		{		
			float k = Rnd();

			if (k<0.5f)
			{
				GetBrain()->ActivatePairLink("pair A");
			}	
			else
			if (k<=1.0f)
			{
				GetBrain()->ActivatePairLink("pair B");
			}	
			/*else
			{
				//GetBrain()->ActivatePairLink("pair C");
				GetBrain()->ActivatePairLink("pair A");
			}*/

			bDoFatality = true;
		}
	}

	/*if (st != CharacterLogic::state_attack && fIdleTime > fTimeToNextAttack)
	{
		/*if ((vTrgtPos-vMyPos).GetLength() > fDistance)
		{
			return false;
		}*/

		/*if (Rnd(1)>0.5f) 
		{
			GetBrain()->GetBody()->logic->ActivateLink("Attack1");
		}		
		else
		{
			GetBrain()->GetBody()->logic->ActivateLink("Attack2");
		}

		fTimeToNextAttack = 0.5f + Rnd(2.0f);
		fIdleTime = 0.0f;

		return false;

	} else
	{
		if (st != CharacterLogic::state_attack) fIdleTime += fDeltaTime;
	}*/

	if (st != CharacterLogic::state_findpair &&
		st != CharacterLogic::state_topair &&
		st != CharacterLogic::state_pair && bHasKick)
	{
		bAllowRotate = true;

		fIdleTime += fDeltaTime;
		
		if (fIdleTime > fTimeToNextAttack)
		{
			GetBrain()->bAllowUpdateInPairModel=false;

			return false;
		}		
	}

		

	return true;
}

void aiPittFatality::DebugDraw (const Vector& BodyPos, IRender* pRS)
{
	//pRS->Print(BodyPos + Vector (0.0f, 1.9f, 0.0f), 10000.0f, 0.0f, 0xFFFFFFFF, "wait");
}

void aiPittFatality::Stop ()
{
	bool bRes = GetBrain()->GetBody()->animation->ActivateLink("idle");
}