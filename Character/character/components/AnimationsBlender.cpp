


#include "AnimationsBlender.h"

#include "..\..\..\Common_h\AnimationBlendLevels.h"


AnimationsBlender::AnimationsBlender(IAnimation * from, IAnimation * to, float time, bool autoBlend) : pose(_FL_)
{
	Assert(from);
	Assert(to);
	this->from = (IAnimation*)from->Clone();	
	k = 1.0f;		
	if(time < 1e-10f) time = 1e-10f;
	dlt = 1.0f/time;
	isAutoBlend = autoBlend;
	SetAnimation(to);
	Assign(from, to);
}

AnimationsBlender::~AnimationsBlender()
{
	UnregistryBlendStage();
	from->Release(); from = null;
}

//Прочитать данные
void AnimationsBlender::ReadData()
{
	for(long i = 0; i < pose; i++)
	{
		PoseElement & pe = pose[i];
		if(pe.sourseIndex >= 0)
		{		
			from->GetBoneMatrix(pe.sourseIndex);
			pe.q = from->GetBoneRotate(pe.sourseIndex);
			pe.p = from->GetBonePosition(pe.sourseIndex);
		}else{
			pe.q.SetIdentity();
			pe.p = 0.0f;
		}
	}	
}

//Перезапустить блендер
AnimationsBlender * AnimationsBlender::Restart()
{
	ReadData();
	if (isAutoBlend)
	{
		k = 1.0f;
	}
	else
	{
		k = 0.0f;
	}	

	RegistryBlendStage(aminationBlendLevel_betweenAnimations);
	return this;
}

//Получить необходимость в данных предыдущих стадий
bool AnimationsBlender::NeedPrevStageData(long boneIndex)
{
	return false;
}

//Получить коэфициент блендинга для интересующей кости
float AnimationsBlender::GetBoneBlend(long boneIndex)
{
	if(pose[boneIndex].sourseIndex >= 0)
	{
		return k;
	}
	return 0.0f;
}

//Получить трансформацию кости
void AnimationsBlender::GetBoneTransform(long boneIndex, Quaternion & rotation, Vector & position, Vector & scale, const Quaternion & prevRotation, const Vector & prevPosition, const Vector & prevScale)
{
	PoseElement & pe = pose[boneIndex];
	if(pe.sourseIndex >= 0)
	{		
		rotation = pe.q;
		position = pe.p;
		scale = prevScale;
	}else{
		rotation = prevRotation;
		position = prevPosition;
		scale = prevScale;
	}
}

//Обновить состояние
void AnimationsBlender::Update(float dltTime)
{
	ReadData();

	if (isAutoBlend)
	{
		//if(k > 0.0f)
		{
			k -= dlt*dltTime;
			
			if(k <= 0.0f)
			{
				UnregistryBlendStage();
			
				//delete this;
				//return;
			}
		}

		return;
	}	

	
}

//Составить таблицу соостветствия по скилетам
void AnimationsBlender::Assign(IAnimation * from, IAnimation * to)
{
	pose.Empty();
	long boneCount = to->GetNumBones();
	pose.AddElements(boneCount);
	for(long i = 0; i < boneCount; i++)
	{
		pose[i].sourseIndex = from->FindBone(to->GetBoneName(i));
	}
}

void AnimationsBlender::SetBlend(float blend)
{
	if (!isAutoBlend) k = Clampf(blend);
}




