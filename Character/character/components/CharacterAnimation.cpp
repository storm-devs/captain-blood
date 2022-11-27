
#include "CharacterAnimation.h"
#include "CharacterAnimationEvents.h"

CharacterAnimation::CharacterAnimation(Character * character): chr(*character),animData(_FL_, 2)
{
	animation = null;
	last_animation = null;
	anim_blender = null;
	anim_blended_index = -1;

	if (chr.IsPlayer())
		animData.SetAddElements(18);
}

CharacterAnimation::~CharacterAnimation()
{
	for (int i=0;i<(int)animData.Size();i++)
	{
		if (animData[i].animation)
		{
			animData[i].animation->Release();
			animData[i].animation = null;
		}
	}

	animData.Empty();

	if (anim_blender) delete anim_blender;
}

IAnimation* CharacterAnimation::AddAnimation(const char* id, const char* anim_name)
{
	for (int i=0;i<(int)animData.Size();i++)
	{
		if (string::IsEqual(id,animData[i].id))
		{
			/*if (animData[i].animation)
			{
				animData[i].animation->Release();
				animData[i].animation = null;
			}

			animData[i].animation = chr.Animation().Create(anim_name, _FL_);
			if (animData[i].animation) animData[i].animation->Pause(true);*/

			return animData[i].animation;
		}
	}

	AnimData& data = animData[animData.Add()];

	crt_strcpy(data.id,sizeof(data.id),id);
	data.animation = chr.Animation().Create(anim_name, _FL_);
	if (data.animation) data.animation->Pause(true);

	return data.animation;
}

IAnimation* CharacterAnimation::GetAnimation(const char* id)
{
	for (int i=0;i<(int)animData.Size();i++)
	{
		if (string::IsEqual(id,animData[i].id))
		{
			return animData[i].animation;
		}
	}

	return null;
}

IAnimation* CharacterAnimation::GetCurAnimation()
{
	return animation;
}

IAnimation* CharacterAnimation::GetLastAnimation()
{
	return last_animation;
}

void CharacterAnimation::SetAnimation(IAnimation* anim,float blend_time)
{	
	DisableGraphBlend();

	if (anim_blender)
	{		
		delete anim_blender;
		anim_blender = null;
	}

	if (anim == animation)
	{
		if (animation)
			animation->Pause(false);

		chr.ApplyAnimation();
		return;
	}
	 
	last_animation = animation;	
	animation = anim;	

	if (last_animation)
	{
		last_animation->Pause(true);
	}

	if (animation)
	{
		animation->Pause(false);
		//animation->Start();		
	}

	if (animation && last_animation && blend_time>0.05f)
	{
		anim_blender = NEW AnimationsBlender(last_animation,animation,blend_time,true);
		anim_blender->Restart();
	}	

	chr.ApplyAnimation();
}

void CharacterAnimation::SetAnimation(const char* id, float blend_time)
{
	for (int i=0;i<(int)animData.Size();i++)
	{
		if (string::IsEqual(id,animData[i].id))
		{
			SetAnimation(animData[i].animation,blend_time);

			return;
		}
	}
}

void CharacterAnimation::SetLastAnimation(float blend_time)
{
	if (!last_animation) return;
	
	SetAnimation(last_animation,blend_time);
}

void CharacterAnimation::EnableGraphBlend(const char* id)
{
	DisableGraphBlend();

	for (int i=0;i<(int)animData.Size();i++)
	{
		if (string::IsEqual(id,animData[i].id) && animData[i].animation)
		{
			if (anim_blender)
			{		
				delete anim_blender;
				anim_blender = null;
			}

			anim_blended_index = i;
			anim_blender = NEW AnimationsBlender(animData[i].animation,animation, 0.2f,false);
			anim_blender->Restart();
			animData[i].animation->Pause(false);			

			return;
		}
	}
}

void CharacterAnimation::DisableGraphBlend()
{
	if (anim_blended_index != -1)
	{
		if (anim_blender)
		{		
			delete anim_blender;
			anim_blender = null;
		}

		animData[anim_blended_index].animation->Pause(true);
		anim_blended_index = -1;
	}	
}

void CharacterAnimation::SetGraphBlend(float blend)
{
	if (anim_blender) anim_blender->SetBlend(blend);
}

void CharacterAnimation::Reset()
{
	for (int i=0;i<(int)animData.Size();i++)
	{
		if (animData[i].animation) animData[i].animation->Start();
	}
}