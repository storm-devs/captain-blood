
#include "AnimBlender.h"

#include "..\..\..\Common_h\AnimationBlendLevels.h"


AnimBlender::AnimBlender()
{	
	cur_rot = 0.0f;
	angular_speed = 0.0f;
	angular_force = 0.0f;
	stoped = true;
	gun_bone = -1;
	barrel_bone = -1;
	barrel_rot = 0.f;
}

AnimBlender::~AnimBlender()
{
	//UnregistryBlendStage();	
}

void AnimBlender::Init(IAnimation* anim)
{
	UnregistryBlendStage();

	if( !anim )	return;

	SetAnimation(anim);

	gun_bone = anim->FindBone("guns",true);
	barrel_bone = anim->FindBone("cannon",true);

	RegistryBlendStage(aminationBlendLevel_autoEyes);
}

//Получить необходимость в данных предыдущих стадий
bool AnimBlender::NeedPrevStageData(long boneIndex)
{
	return true;
}

//Получить коэфициент блендинга для интересующей кости
float AnimBlender::GetBoneBlend(long boneIndex)
{
	if( boneIndex == gun_bone ||
		boneIndex == barrel_bone )
		return 1.f;
	return 0.0f;
}

//Получить трансформацию кости
void AnimBlender::GetBoneTransform(long boneIndex, Quaternion & rotation, Vector & position, Vector & scale, const Quaternion & prevRotation, const Vector & prevPosition, const Vector & prevScale)
{
	if( boneIndex == gun_bone )
	{
		Matrix m;
		m.RotateZ(-cur_rot);
		rotation = Quaternion(m);

		position = prevPosition;
		scale	 = prevScale;
	}
	else if( boneIndex == barrel_bone )
	{
		Matrix m;
		m.RotateX(barrel_rot);
		rotation = Quaternion(m);

		position = prevPosition;
		scale	 = prevScale;
	}
	else
	{
		rotation = prevRotation;
		position = prevPosition;
		scale	 = prevScale;
	}
}

void AnimBlender::AddForce(float force)
{
	angular_force += force;

	if (angular_force>8.0f) angular_force = 8.0f;

	stoped = false;
}

float AnimBlender::GetRelativeAngularSpeed()
{
	float fRelSpeed = angular_speed * (1.f / 5.f);
	return Clampf(fRelSpeed);
}

//Обновить состояние
void AnimBlender::Update(float dltTime)
{	
	float force = dltTime * 17.0f;

	if (force>angular_force)
	{
		force=angular_force;
		angular_force = 0.0f;
	}
	else
	{
		angular_force -= force;
	}

	if (force>0.0f)
	{
		angular_speed += force;

		if (angular_speed>5.0f) angular_speed = 5.0f;
	}
	else
	{
		if (!stoped)
		{
			angular_speed -= dltTime * 3.0f;

			if (angular_speed<1.45f) angular_speed = 1.45f;
		}
	}

	if (!stoped && force<=0.0f)
	{
		force = angular_speed;
	
		if (force>7.0f)
		{
			force = 7.0f;
		}
			
		if (angular_speed<2.75f)		
		{			
			int k = (int)(cur_rot / (PI));
			cur_rot += force * dltTime;	

			int k2 = (int)(cur_rot / (PI));

			if (k!=k2)
			{
				cur_rot = 0.0f;
				angular_speed = 0.0f;
				stoped = true;
			}
		}
		else
		{
			cur_rot += force * dltTime;	
		}
	}
	else
	{
		force = angular_speed;
	
		if (force>7.0f)
		{
			force = 7.0f;
		}
	
		cur_rot += force * dltTime;	
	}

	
}



