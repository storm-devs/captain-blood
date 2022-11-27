#include "aiKomendantShoot.h"
#include "../../Brains/aiBrain.h"
#include "../../../character/components/CharacterPhysics.h"
#include "../../../character/components/CharacterLogic.h"


aiKomendantShoot::aiKomendantShoot(aiBrain* Brain) : aiThought (Brain)
{	
}

void aiKomendantShoot::Activate(Character* pChr)
{
	pTarget = pChr;

	if (Rnd()>0.5f)
	{
		GetBrain()->GetBody()->logic->SetShootTarget(pTarget,"Hit");
	}
	else
	{
		GetBrain()->GetBody()->logic->SetShootTarget(NULL,"Hit");
	}

	aiThought::Activate();
}

bool aiKomendantShoot::Process (float fDeltaTime)
{
	CharacterLogic::State st = GetBrain()->GetBody()->logic->GetState();	

	if (st == CharacterLogic::state_shoot)
	{		
		{
			Vector vTrgtPos = pTarget->physics->GetPos();
			Vector vMyPos = GetBrain()->GetBody()->physics->GetPos();		

			GetBrain()->GetBody()->physics->Orient(vTrgtPos);
		}		
	}	

	if (st != CharacterLogic::state_shoot)
	{		
		return false;
	}	

	return true;
}

void aiKomendantShoot::Stop ()
{
	//bool bRes = GetBrain()->GetBody()->logic->ActivateLink("idle");
}