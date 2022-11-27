
#include "aiActionStack.h"

aiActionStack::aiActionStack(const CharactersTime & _time) : 
	actions(_FL_, 6),
	time(_time)
{	
	cur_action = -1;
}

void aiActionStack::AddAction(const char * link, float maxDistance, float minDistance, float minCooldown, float maxCooldown)
{
	TAction & attack = actions[actions.Add()];

	crt_strcpy(attack.link, sizeof(attack.link), link);
	attack.minDistance = minDistance;
	attack.maxDistance = maxDistance;
	attack.minCooldown = minCooldown;
	attack.maxCooldown = maxCooldown;

	attack.cooldown = time.GetTimeDword() + time.Convert2Dword(RRnd(minCooldown, maxCooldown));
	attack.randIndex = actions.Last();
}

void aiActionStack::AddAction(const AttackParam * action)
{
	Assert(action);

	AddAction(action->link, action->maxDistance, action->minDistance, action->minCooldown, action->maxCooldown);
}

void aiActionStack::Prepare()
{	
	if (actions.IsEmpty())
	{
		cur_action = -1;
		return;
	}

	int num = actions.Len();
	for (int i=0; i<num; i++)
	{
		int index1 = (int)(Rnd(0.999f) * num);
		int index2 = (int)(Rnd(0.999f) * num);

		Swap(actions[index1].randIndex, actions[index2].randIndex);
	}

	cur_action = 0;
}

const char* aiActionStack::GetCurAction()
{
	if (cur_action == -1) return "no_attacks_available";

	int nums = 0;
	while (nums < actions.Len())
	{
		TAction & action = GetRandActionIndexed(cur_action);
		if (action.cooldown <= time.GetTimeDword())
		{
			action.cooldown = time.GetTimeDword() + time.Convert2Dword(RRnd(action.minCooldown, action.maxCooldown));
			return action.link;
		}

		cur_action++;
		if (cur_action >= actions.Len())
			cur_action = 0;

		nums++;
	}

	return "all_attacks_oncooldown";
}

float aiActionStack::GetCurActionDistance()
{
	if (cur_action == -1) return -1.0f;

	return GetRandActionIndexed(cur_action).maxDistance;
}

void aiActionStack::MoveToNextAction(bool need_shufle)
{
	cur_action++;
	
	if (cur_action >= actions.Len())
	{
		if (need_shufle)
		{
			Prepare();
		}
		else
		{
			cur_action = 0;
		}
	}
}

void aiActionStack::SecActionAsCurrent(const char* attack_link)
{
	Prepare();

	for (int i=0; i<actions.Len(); i++)
		if (string::IsEqual(GetRandActionIndexed(i).link, attack_link))
		{
			cur_action = i;
			return;		
		}
}

void aiActionStack::Reset()
{
	cur_action = -1;
	actions.Empty();
}

void aiActionStack::CheckDist(float dist)
{
	int nums = 0;
	
	while (nums < actions.Len() &&
		(GetRandActionIndexed(cur_action).minDistance > dist))
	{
		MoveToNextAction(false);
		nums++;
	}
}

aiActionStack::TAction & aiActionStack::GetRandActionIndexed(int index)
{
	return actions[actions[index].randIndex];
}
