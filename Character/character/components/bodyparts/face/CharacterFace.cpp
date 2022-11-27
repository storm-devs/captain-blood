#include "CharacterFace.h"

CharacterFace:: CharacterFace()
{
	eyesPosition = 1.0f;
	timeForNextWink = Rnd(5.0f);
	flags = 0;
}

CharacterFace::~CharacterFace()
{
}

void CharacterFace::Init(const InitData &data)
{
	chr = data.chr;
	CharacterFace::ResetAnimation(data.animation);
}

void CharacterFace::ResetAnimation(IAnimation *animation)
{
//	ReleaseBones();
	flags &= ~flag_isProcess;
	if(animation)
	{
		long aniEyelidLeft = animation->FindBone("left_veko_up", true);
		long aniEyelidRight = animation->FindBone("right_veko_up", true);
		if(aniEyelidLeft >= 0)
		{
			RegBone(aniEyelidLeft);
			flags |= flag_isProcess;
		}
		if(aniEyelidRight >= 0)
		{
			RegBone(aniEyelidRight);
			flags |= flag_isProcess;
		}
	}
}

void CharacterFace::Activate(bool isActive, ISoundScene *scene)
{
}

void CharacterFace::Update(float dltTime)
{
	if(flags & flag_isProcess)
	{
		//Моргание глазами
		if(!(flags & flag_isDead))
		{
			if(timeForNextWink > 0.0f)
			{
				timeForNextWink -= dltTime;
				eyesPosition += dltTime*10.0f;
				if(eyesPosition > 1.0f) eyesPosition = 1.0f;
			}else{
				eyesPosition -= dltTime*10.0f;
				if(eyesPosition <= 0.0f)
				{
					timeForNextWink = 1.0f + Rnd(5.0f);
					eyesPosition = 0.0f;
				}
			}
		}else{
			eyesPosition -= dltTime*3.0f;
			if(eyesPosition < 0.0f) eyesPosition = 0.0f;
		}

		Matrix mtx(false);
		mtx.BuildRotateX((1.0f - eyesPosition)*30.0f*(PI/180.0f));
		eyelidsRot.Set(mtx);
	}
}

void CharacterFace::GetBoneTransform(int i, BlendData &data)
{
	data.rotation = eyelidsRot;
	data.position = data.prevPosition;
	data.scale = data.prevScale;
}

void CharacterFace::SetDead(bool is)
{
	if(is)
	{
		flags |= flag_isDead;
	}else{
		flags &= ~flag_isDead;
	}
}

