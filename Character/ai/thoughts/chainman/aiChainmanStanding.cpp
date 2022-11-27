#include "aiChainmanStanding.h"

#include "../../Brains/aiBrain.h"
#include "../../../character/components/CharacterPhysics.h"
#include "../../../character/components/CharacterLogic.h"

aiChainmanStanding::aiChainmanStanding(aiBrain* Brain) : aiThoughtStanding (Brain)
{	
}

aiChainmanStanding::~aiChainmanStanding()
{
	
}

void aiChainmanStanding::Activate( float fWaitTime,Character* _target)
{
	fTime = 0.0f;
	fMaxWaitTime = fWaitTime;

	pTarget=_target;

	aiThought::Activate();
}

bool aiChainmanStanding::Process (float fDeltaTime)
{
	if (pTarget && GetBrain()->GetBody()->logic->AllowOrient())
	{	
		Vector vTrgtPos = pTarget->physics->GetPos();
		Vector vMyPos = GetBrain()->GetBody()->physics->GetPos();	

		GetBrain()->GetBody()->physics->Orient(vTrgtPos);
	}
			
	fTime += fDeltaTime;

	if (fTime >= fMaxWaitTime)
	{	
		return false;
	}

	return true;
}

void aiChainmanStanding::DebugDraw (const Vector& BodyPos, IRender* pRS)
{
	//pRS->Print(BodyPos + Vector (0.0f, 1.9f, 0.0f), 10000.0f, 0.0f, 0xFFFFFFFF, "wait");
}
