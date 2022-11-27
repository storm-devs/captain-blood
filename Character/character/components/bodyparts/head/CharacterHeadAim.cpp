#include "CharacterHeadAim.h"
#include "..\..\..\Components\CharacterLogic.h"
#include "..\..\..\Character.h"

CharacterHeadAim:: CharacterHeadAim()
{	
	head_bone = -1;

		rot = 0.0f;
	cur_rot = 0.0f;

	fLimit = 0.65f;

	offset = 0.0f;

	chr = null;
}

CharacterHeadAim::~CharacterHeadAim()
{	
}

void CharacterHeadAim::Init(const InitData &data)
{
	chr = data.chr;

	if( data.animation )
	{
		head_bone = data.animation->FindBone("head_joint",true);

		if( head_bone >= 0 )
		{
			RegBone(head_bone);
		}
	}
}

void CharacterHeadAim::ResetAnimation(IAnimation *animation)
{
//	ReleaseBones();

	head_bone = animation->FindBone("head_joint",true);

	if( head_bone >= 0 )
	{
		RegBone(head_bone);
	}
}

void CharacterHeadAim::Update(float dltTime)
{
	if( rot > cur_rot )
	{
		cur_rot += dltTime*4.25f;

		if( rot < cur_rot )
		{
			cur_rot = rot;
		}
	}
	else
	{
		cur_rot -= dltTime*4.25f;

		if( rot > cur_rot )
		{
			cur_rot = rot;
		}
	}	
}

void CharacterHeadAim::GetBoneTransform(int i, BlendData &data)
{
	Matrix m(true);

	data.prevRotation.GetMatrix(m);

	m.RotateY(cur_rot);

	data.rotation = Quaternion(m);

	data.position = data.prevPosition;
	data.scale	  = data.prevScale;
}

void CharacterHeadAim::SetLimit(float limit)
{
	fLimit = limit;
}

void CharacterHeadAim::SetOffset(float angle)
{
	offset = angle;
}

void CharacterHeadAim::SetRotation(float rotation)
{	
	if( chr )
	{
		if( !chr->logic->AllowOrientHead()) 
		{
		//	rot = 0;
			rot = offset;

			return;
		}		
	}
	
	rot = offset + Clampf(rotation,-fLimit, fLimit);
	rot *= 0.5f;
}
