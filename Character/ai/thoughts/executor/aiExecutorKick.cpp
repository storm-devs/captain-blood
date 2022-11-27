#include "aiExecutorKick.h"
#include "../../Brains/aiBrain.h"
#include "../../../character/components/CharacterPhysics.h"
#include "../../../character/components/CharacterLogic.h"


aiExecutorKick::aiExecutorKick(aiBrain* Brain) : aiThought (Brain)
{

}

aiExecutorKick::~aiExecutorKick()
{
}

void aiExecutorKick::Activate(Character* pChr, float fKickDistance)
{
	pTarget = pChr;
	fDistance = fKickDistance;
	fIdleTime = 0.0f;
	fTimeToNextAttack =  Rnd(0.7f)+0.5f;// + Rnd(2.0f);;

	bHasKick = false;

	aiThought::Activate();
}

bool aiExecutorKick::Process (float fDeltaTime)
{
	GetBrain()->OrinentToTarget();

	CharacterLogic::State st = GetBrain()->GetBody()->logic->GetState();	

	if (st == CharacterLogic::state_attack)
	{
		bHasKick = true;
	}


	if (st != CharacterLogic::state_attack && !bHasKick)
	{
		int k = (int)(Rnd(0.999f)*6);

		//k = 4;

		switch (k)
		{
			case 0:
			case 1:
			{
				GetBrain()->GetBody()->animation->ActivateLink("Attack1");				
			}
			break;
			case 2:
			case 3:
			{
				GetBrain()->GetBody()->animation->ActivateLink("Attack2");				
			}
			break;				
			case 4:
			case 5:
				{
					GetBrain()->GetBody()->animation->ActivateLink("Attack3");				
				}
				break;				
		}

		if (st == CharacterLogic::state_attack)
		{
			bHasKick = true;
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

	if (st != CharacterLogic::state_attack && bHasKick)
	{
		fIdleTime += fDeltaTime;

		if (fIdleTime > fTimeToNextAttack) return false;
	}

		

	return true;
}

void aiExecutorKick::Stop ()
{
	bool bRes = GetBrain()->GetBody()->animation->ActivateLink("idle");
}

void aiExecutorKick::Hit ()
{
	//Отвечать, если бьют...
	fTimeToNextAttack = 0.05f;
}