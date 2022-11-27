#include "aiPittBlocking.h"

#include "../../Brains/aiBrain.h"
#include "../../../character/components/CharacterPhysics.h"
#include "../../../character/components/CharacterLogic.h"

aiPittBlocking::aiPittBlocking(aiBrain* Brain) : aiThoughtBlocking(Brain)
{	
}

aiPittBlocking::~aiPittBlocking()
{
}

void aiPittBlocking::Activate(Character* pChr, float fWaitTime)
{
	fTime = 0.0f;
	fMaxWaitTime = fWaitTime;

	pTarget=pChr;	

	bInBlock = false;

	aiThought::Activate();
}

bool aiPittBlocking::Process (float fDeltaTime)
{
	if (!bInBlock)
	{	
		bInBlock  = GetBrain()->GetBody()->animation->ActivateLink("toBlock");
	}

	if (bInBlock)
	{	
		Vector vTrgtPos = pTarget->physics->GetPos();
		Vector vMyPos = GetBrain()->GetBody()->physics->GetPos();	

		GetBrain()->GetBody()->physics->Orient(vTrgtPos);			
	}

	if (bInBlock)
	{
		CharacterLogic::State st = GetBrain()->GetBody()->logic->GetState();

		if (st == CharacterLogic::state_attack)
		{
			return false;
		}
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

void aiPittBlocking::Stop ()
{
	bool bRes = GetBrain()->GetBody()->animation->ActivateLink("idle");
}