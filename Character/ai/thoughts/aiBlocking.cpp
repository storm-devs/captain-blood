#include "aiBlocking.h"

#include "../Brains/aiBrain.h"
#include "../../character/components/CharacterPhysics.h"
#include "../../character/components/CharacterLogic.h"

aiThoughtBlocking::aiThoughtBlocking(aiBrain* Brain) : aiThought (Brain)
{
	iNumHits = 0;
	runaway_link[0] = 0;
}

void aiThoughtBlocking::Activate(Character* pChr, float fWaitTime)
{
	fTime = 0.0f;
	fMaxWaitTime = fWaitTime;

	pTarget=pChr;	

	bInBlock = false;

	iNumHits = 0;

	stoped = false;

	aiThought::Activate();
}

void aiThoughtBlocking::SetRunawayLink(const char* link)
{
	crt_strcpy(runaway_link, sizeof(runaway_link), link);
}

bool aiThoughtBlocking::Process (float fDeltaTime)
{
	if (stoped) return false;

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

void aiThoughtBlocking::Stop ()
{
	GetBrain()->GetBody()->animation->ActivateLink("End Block",true);
}

void aiThoughtBlocking::Hit()
{
	iNumHits++;

	if (GetBrain()->GetBody()->animation->TestActivateLink(runaway_link))
	{
		if (Rnd()>0.5f)
		{
			GetBrain()->GetBody()->animation->ActivateLink(runaway_link);
			stoped = true;

			return;
		}
	}

	fTime = 0.0f;	
}