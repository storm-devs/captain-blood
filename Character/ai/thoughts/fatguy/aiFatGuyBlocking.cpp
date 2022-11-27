#include "aiFatGuyBlocking.h"

#include "../../Brains/aiBrain.h"
#include "../../../character/components/CharacterPhysics.h"
#include "../../../character/components/CharacterLogic.h"

aiFatGuyBlocking::aiFatGuyBlocking(aiBrain* Brain) : aiThoughtBlocking (Brain)
{
	iNumHits = 0;
}

void aiFatGuyBlocking::Activate(Character* pChr, float fWaitTime)
{
	fTime = 0.0f;
	fMaxWaitTime = fWaitTime;

	pTarget=pChr;	

	bInBlock = false;

	iNumHits = 0;

	aiThought::Activate();
}

bool aiFatGuyBlocking::Process (float fDeltaTime)
{
	if (bInBlock)
	{
		if (GetBrain()->GetBody()->logic->GetState() != CharacterLogic::state_block &&
			GetBrain()->GetBody()->logic->GetState() != CharacterLogic::state_attack)
		{
			return false;
		}
	}

	if (!bInBlock)
	{	
		bInBlock  = GetBrain()->GetBody()->animation->ActivateLink("Block",true);
		
		if (GetBrain()->GetBody()->logic->GetState() == CharacterLogic::state_block) bInBlock = true;
	}

	if (bInBlock && pTarget)
	{	
		Vector vTrgtPos = pTarget->physics->GetPos();
		Vector vMyPos = GetBrain()->GetBody()->physics->GetPos();	

		GetBrain()->GetBody()->physics->Orient(vTrgtPos);			
	}

	if (bInBlock)
	{		
		fTime += fDeltaTime;

		if ( fTime >= fMaxWaitTime )
		{		 
			return false;
		}
	}
	
	return true;
}

void aiFatGuyBlocking::Stop ()
{
	GetBrain()->GetBody()->animation->ActivateLink("End Block",true);
}

void aiFatGuyBlocking::Hit()
{
	iNumHits++;

	if (Rnd()<(float)iNumHits * 0.125f)
	{
		if (Rnd()>0.2f)
		{
			GetBrain()->GetBody()->animation->ActivateLink("Attack",true);
			iNumHits = 0;
		}
		else
		{
			GetBrain()->GetBody()->animation->ActivateLink("Rage",true);
		}
	}

	fTime = 0.0f;	
}