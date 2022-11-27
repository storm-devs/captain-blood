#include "aiKnightRage.h"
#include "../../Brains/aiBrain.h"
#include "../../../character/components/CharacterPhysics.h"
#include "../../../character/components/CharacterLogic.h"


aiKnightRage::aiKnightRage(aiBrain* Brain) : aiThought (Brain)
{	
}

void aiKnightRage::Activate(Character* pChr)
{
	pTarget = pChr;	
	fRotateTime = 0.0f;

	aiThought::Activate();
}

bool aiKnightRage::Process (float fDeltaTime)
{
	CharacterLogic::State st = GetBrain()->GetBody()->logic->GetState();	

	if (st == CharacterLogic::state_strafe)
	{
		fRotateTime += fDeltaTime;

		if (fRotateTime>1.22f)
		{
			fRotateTime = 1.22f;
		}
		else
		{
			Vector vTrgtPos = pTarget->physics->GetPos();
			Vector vMyPos = GetBrain()->GetBody()->physics->GetPos();		

			GetBrain()->GetBody()->physics->Orient(vTrgtPos);
		}

		GetBrain()->GetBody()->arbiter->SplashDamage(DamageReceiver::ds_bomb, GetBrain()->GetBody(),GetBrain()->GetBody()->physics->GetPos(),2,20,false,null,true,0.5f);
	}	

	if (st != CharacterLogic::state_kick && st != CharacterLogic::state_strafe)
	{		
		return false;
	}	

	return true;
}

void aiKnightRage::Stop ()
{
	//bool bRes = GetBrain()->GetBody()->logic->ActivateLink("idle");
}