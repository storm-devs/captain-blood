#include "aiKomendantKick.h"
#include "../../Brains/aiBrain.h"
#include "../../../character/components/CharacterPhysics.h"
#include "../../../character/components/CharacterLogic.h"


aiKomendantKick::aiKomendantKick(aiBrain* Brain) : aiThought (Brain)
{
}

void aiKomendantKick::Activate(Character* pChr, float fKickDistance)
{
	pTarget = pChr;
	fDistance = fKickDistance;
	fIdleTime = 0.0f;
	fTimeToNextAttack =  Rnd(0.7f)+0.5f;// + Rnd(2.0f);;

	bHasKick = false;
	//GetBrain()->GetBody()->logic->Attack(pChr);	

	aiThought::Activate();
}

bool aiKomendantKick::Process (float fDeltaTime)
{
	float angle = GetBrain()->GetBody()->physics->GetAy();
	float r = RRnd(4.5f,6.0f);

	float dx = sin(angle) * r;
	float dz = cos(angle) * r;
	Matrix mtx;
	GetBrain()->GetBody()->SetBombTarget(GetBrain()->GetBody()->GetMatrix(mtx).pos+Vector(dx,0,dz));	

	GetBrain()->OrinentToTarget();

	CharacterLogic::State st = GetBrain()->GetBody()->logic->GetState();	

	if (st == CharacterLogic::state_attack)
	{
		bHasKick = true;
	}


	if (st != CharacterLogic::state_attack && !bHasKick)
	{
		int k = (int)(Rnd(0.999f)*4);

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

void aiKomendantKick::Stop ()
{
	bool bRes = GetBrain()->GetBody()->animation->ActivateLink("idle");
}

void aiKomendantKick::Hit ()
{
	//Отвечать, если бьют...
	fTimeToNextAttack = 0.05f;
}