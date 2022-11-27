#include "CharacterTorsoAim.h"

void CharacterTorsoAim::Init(const InitData &data)
{
	chr = data.chr;

	if( data.animation )
	{
		head_bone = data.animation->FindBone("spine1_joint",true);

		if( head_bone >= 0 )
		{
			RegBone(head_bone);
		}
	}
}

void CharacterTorsoAim::ResetAnimation(IAnimation *animation)
{
	head_bone = animation->FindBone("spine1_joint",true);

	if( head_bone >= 0 )
	{
		RegBone(head_bone);
	}
}

void CharacterTorsoAim::SetRotation(float rotation)
{	
	rot = offset + Clampf(rotation,-fLimit, fLimit);
	rot *= 0.5f;
}
