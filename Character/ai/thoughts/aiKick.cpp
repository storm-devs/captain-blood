#include "aiKick.h"
#include "../Brains/aiBrain.h"
#include "../../character/components/CharacterPhysics.h"
#include "../../character/components/CharacterLogic.h"

aiThoughtKick::aiThoughtKick(aiBrain* Brain) : aiThought (Brain), actionStack(Brain->GetBody()->arbiter->GetTime())
{
	far_kick[0] = 0;
}

void aiThoughtKick::Activate(Character* pChr)
{
	pTarget = pChr;
	fIdleTime = 0.0f;
	fTimeToNextAttack = Max(0.25f, (Rnd(0.7f) + 0.5f) * GetBrain()->GetIdleAfterAttackModifier());

	bHasKick = false;	

	if (pTarget!=NULL)
	{
		pTarget->controller->SetNumAtackers(pTarget->controller->GetNumAtackers()+1);
		GetBrain()->GetBody()->physics->Orient(GetBrain()->GetBody()->physics->GetPos());
	}

	aiThought::Activate();
}

bool aiThoughtKick::Process (float fDeltaTime)
{
	GetBrain()->OrinentToTarget();

	CharacterLogic::State st = GetBrain()->GetBody()->logic->GetState();

	if (!bHasKick)
	{
		//VANO: убрал if (GetBrain()...), этот фикс убирает зависание солдата перед ратушью
		GetBrain()->GetBody()->animation->ActivateLink(actionStack.GetCurAction(), true);
		{
			bHasKick = true;
			actionStack.MoveToNextAction();
		}
	}
	else
	if (bHasKick && st == CharacterLogic::state_attack)
	{
		if (!string::IsEmpty(far_kick))
		{
			if (GetBrain()->GetPowDistToTarget()>9.0f)
			{
				GetBrain()->GetBody()->animation->ActivateLink(far_kick,true);
			}
		}		
	}	

	if (st != CharacterLogic::state_attack && bHasKick)
	{
		fIdleTime += fDeltaTime;

		if (fIdleTime > fTimeToNextAttack) return false;
	}

	if (pTarget && pTarget->logic->IsSlave())
		return false;

	return true;
}

void aiThoughtKick::Stop ()
{
	GetBrain()->GetBody()->animation->ActivateLink("idle");

	if (pTarget) pTarget->controller->SetNumAtackers(pTarget->controller->GetNumAtackers()-1);
}

void aiThoughtKick::AddAction(const char* action, float max_dist, float min_dist)
{
	actionStack.AddAction(action, max_dist, min_dist);
}

void aiThoughtKick::SetFarAttack(const char* attack)
{
	crt_strcpy(far_kick,sizeof(far_kick),attack);
}

void aiThoughtKick::SetCurAction(const char* act)
{
	actionStack.SecActionAsCurrent(act);

}

void aiThoughtKick::PrepareActions()
{
	actionStack.Prepare();
}

void aiThoughtKick::ResetStack()
{
	actionStack.Reset();
}

float aiThoughtKick::GetAttackDistance()
{
	return actionStack.GetCurActionDistance();
}

void aiThoughtKick::CheckDist(float dst)
{
	return actionStack.CheckDist(dst);
}