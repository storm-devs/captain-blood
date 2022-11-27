
#include "Tips.h"
#include "TipsManager.h"


Tip::Tip(TipsManager* manager, int tip_ref,void* _ptr)
{
	this->manager = manager;
	this->tip_ref = tip_ref;
	bActive = true;	
	alpha = 1.0f;
	state = ITip::inactive;
	ptr = _ptr;

	sec_alpha = 0.0f;
}

void Tip::Activate(bool bActive)
{
	this->bActive = bActive;
}

bool Tip::IsActive()
{
	return bActive;
}

void Tip::SetPos(Vector pos)
{
	this->pos = pos;
}

Vector Tip::GetPos()
{
	return pos;
}

void Tip::SetAlpha(float _alpha)
{
	alpha = _alpha;	
}

float Tip::GetAlpha()
{
	return alpha * (0.35f + sec_alpha * 0.65f);
}

void Tip::SetState(ITip::TState _state)
{
	state = _state;
}

ITip::TState Tip::GetState()
{
	return state;
}

void Tip::Release()
{
	manager->DeleteTip(this);

	delete this;
}

void Tip::Update(float dltTime)
{
	float fade_speed = 2.5;

	if (state == inactive)
	{
		sec_alpha -= dltTime * fade_speed;

		if (sec_alpha<0.0f)
		{
			sec_alpha = 0.0f;
		}
	}
	else
	if (state == active)
	{
		sec_alpha += dltTime * fade_speed;

		if (sec_alpha>1.0f)
		{
			sec_alpha = 1.0f;
		}
	}
}

