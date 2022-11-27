#include "aiStanding.h"

#include "../Brains/aiBrain.h"
#include "../../character/components/CharacterPhysics.h"
#include "../../character/components/CharacterLogic.h"

aiThoughtStanding::aiThoughtStanding(aiBrain* Brain): aiThought (Brain), actionStack(Brain->GetBody()->arbiter->GetTime())
{
	target = NULL;
}

void aiThoughtStanding::Activate( float fWaitTime, Character* _target)
{
	fTime = 0.0f;
	fMaxWaitTime = fWaitTime;	
	target = _target;

	aiThought::Activate();
}

bool aiThoughtStanding::Process (float fDeltaTime)
{
	Character & chr = *GetBrain()->GetBody();

	if (target)
	{		
		chr.physics->Orient(target->physics->GetPos());

		if (chr.logic->GetState()!=CharacterLogic::state_strafe && !GetBrain()->IsStatist())
		{		
			if ((target->physics->GetPos() - chr.physics->GetPos()).GetLength2() < Sqr(2.75f))
			{
				chr.animation->ActivateLink("Step Backward", true);			
			}
		}
	} 

	if (chr.logic->GetState() == CharacterLogic::state_strafe) return true;

	if (chr.animation->ActivateLink(actionStack.GetCurAction(), true))
	{		
		actionStack.MoveToNextAction();
	}	
	
	fTime += fDeltaTime;

	if ( fTime >= fMaxWaitTime )
	{		 
		return false;
	}
	
	return true;
}

void aiThoughtStanding::Stop ()
{
	bool bRes = GetBrain()->GetBody()->animation->ActivateLink("idle");
}

void aiThoughtStanding::AddAction( const char* action)
{
	actionStack.AddAction(action, 2.5f, 0.0f);
}

void aiThoughtStanding::PrepareActions()
{
	actionStack.Prepare();
}