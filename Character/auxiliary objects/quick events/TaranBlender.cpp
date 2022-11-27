#include "TaranBlender.h"
#include "..\..\..\Common_h\AnimationBlendLevels.h"

TaranBlender::TaranBlender(IAnimation * idle, IAnimation * light, IAnimation * strong) : pose(_FL_)
{
	Assert(idle);
	Assert(light);
	Assert(strong);

	this->idle = (IAnimation*)idle->Clone();
	this->light = (IAnimation*)light->Clone();
	this->strong = (IAnimation*)strong->Clone();

	SetAnimation(this->idle);

	kBlend = 0.0f;

	Assign();
}

TaranBlender::~TaranBlender()
{
	UnregistryBlendStage();

	RELEASE(idle);
	RELEASE(light);
	RELEASE(strong);
}

//Прочитать данные
void TaranBlender::ReadData()
{
	for(long i = 0; i < pose; i++)
	{
		PoseElement & pe = pose[i];
		if (pe.lightIndex >= 0 && pe.strongIndex >= 0)
		{		
			light->GetBoneMatrix(pe.lightIndex);
			pe.ql = light->GetBoneRotate(pe.lightIndex);
			pe.pl = light->GetBonePosition(pe.lightIndex);

			strong->GetBoneMatrix(pe.strongIndex);
			pe.qs = strong->GetBoneRotate(pe.strongIndex);
			pe.ps = strong->GetBonePosition(pe.strongIndex);
		}else{
			pe.ql.SetIdentity();
			pe.pl = 0.0f;

			pe.qs.SetIdentity();
			pe.ps = 0.0f;
		}
	}	
}

//Перезапустить блендер
void TaranBlender::Restart()
{
	RegistryBlendStage(aminationBlendLevel_betweenAnimations);
}

//Получить необходимость в данных предыдущих стадий
bool TaranBlender::NeedPrevStageData(long boneIndex)
{
	return false;
}

//Получить коэфициент блендинга для интересующей кости
float TaranBlender::GetBoneBlend(long boneIndex)
{
	if (pose[boneIndex].lightIndex >= 0 && pose[boneIndex].strongIndex >= 0)
		return 1.0f;

	return 0.0f;
}

//Получить трансформацию кости
void TaranBlender::GetBoneTransform(long boneIndex, Quaternion & rotation, Vector & position, Vector & scale, const Quaternion & prevRotation, const Vector & prevPosition, const Vector & prevScale)
{
	PoseElement & pe = pose[boneIndex];
	if (pe.lightIndex >= 0 && pe.strongIndex >= 0)
	{		
		rotation.SLerp(pe.ql, pe.qs, kBlend);
		position.Lerp(pe.pl, pe.ps, kBlend);
		scale = prevScale;
	}else{
		rotation = prevRotation;
		position = prevPosition;
		scale = prevScale;
	}
}

//Обновить состояние
void TaranBlender::Update(float dltTime)
{
}

//Составить таблицу соостветствия по скелетам
void TaranBlender::Assign()
{
	pose.Empty();
	long boneCount = idle->GetNumBones();
	pose.AddElements(boneCount);
	for(long i = 0; i < boneCount; i++)
	{
		const char * boneName = idle->GetBoneName(i);
		pose[i].lightIndex = light->FindBone(boneName);
		pose[i].strongIndex = strong->FindBone(boneName);
	}
}

void TaranBlender::SetBlend(float blend)
{
	//if (api->DebugKeyState('V'))
	kBlend = blend;
}




