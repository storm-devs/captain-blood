#include "blendShapeMixer.h"
#include "..\..\common_h\gmxservice\MeshStructure.h"

#define BS_EPSILON 0.0000001f

GMXBlendShapeMixer::GMXBlendShapeMixer(dword _bsMorphTargetsCount, float * _pMorphTargetsWeights, blendShapeTarget * _pMorphTargets,
							   dword _bsBonesTransformCount, blendShapeBoneTransform * _pBoneTransforms,
							   long * _pAnimToGeomBlendShapes_TranslateTable)
{
	bsMorphTargetsCount = _bsMorphTargetsCount;
	pMorphTargetsWeights = _pMorphTargetsWeights;
	pMorphTargets = _pMorphTargets;
	bsBonesTransformCount = _bsBonesTransformCount;
	pBoneTransforms = _pBoneTransforms;
	pAnimToGeomBlendShapes_TranslateTable = _pAnimToGeomBlendShapes_TranslateTable;
}

GMXBlendShapeMixer::~GMXBlendShapeMixer()
{
	UnregisterAnimation();
}


//Получить коэфициент блендинга для интересующей кости (многопоточная функция)
float GMXBlendShapeMixer::GetBoneBlend(long boneIndex)
{
	long indexInBlendShape = pAnimToGeomBlendShapes_TranslateTable[boneIndex];
	if (indexInBlendShape < 0)
	{
		return 0.0f;
	}

	float fTotalBoneWeight = 0.0f;
	for (dword i = 0; i < bsMorphTargetsCount; i++)
	{
		blendShapeBoneTransform * morphBonesArray = pMorphTargets[i].frameBones.ptr;
		fTotalBoneWeight += pMorphTargetsWeights[i] * morphBonesArray[indexInBlendShape].weightScale;
	}

	//Joker: странный код, можно без условий тут сделать мне кажется
	if (fTotalBoneWeight > BS_EPSILON || fTotalBoneWeight < -BS_EPSILON)
	{
		return Clampf(fTotalBoneWeight, 0.0f, 1.0f);
	}               

	return 0.0f;
}

//Получить трансформацию кости (многопоточная функция)
void GMXBlendShapeMixer::GetBoneTransform(long boneIndex, Quaternion & rotation, Vector & position, Vector & scale, const Quaternion & prevRotation, const Vector & prevPosition, const Vector & prevScale)
{
	long indexInBlendShape = pAnimToGeomBlendShapes_TranslateTable[boneIndex];
	if (indexInBlendShape < 0)
	{
		return;
	}

	Vector gPos;
	gPos.x = 0.0f; gPos.y = 0.0f; gPos.z = 0.0f;
	
	Quaternion gRot;
	gRot.x = 0; gRot.y = 0; gRot.z = 0; gRot.w = 0;


	float fDivider = 0.0f;
	for (dword i = 0; i < bsMorphTargetsCount; i++)
	{
		float fWeight = pMorphTargetsWeights[i];
		if (fWeight <= 0.0f) continue;

		blendShapeBoneTransform * morphBonesArray = pMorphTargets[i].frameBones.ptr;

		blendShapeBoneTransform & bone = morphBonesArray[indexInBlendShape];

		fWeight = fWeight * bone.weightScale;
		
		gPos += (bone.p * fWeight);
		gRot += (bone.q * fWeight);

		fDivider += fWeight;
	}

	if (fDivider > BS_EPSILON || fDivider < -BS_EPSILON)
	{
		position = gPos / fDivider;
	}

	rotation = gRot;
	rotation.Normalize();
	scale = 1.0f;
}

//Обновить (только основной поток, второй спит вне цикла исполнения)
void GMXBlendShapeMixer::Update(float dltTime)
{
	

}

void GMXBlendShapeMixer::RegisterAnimation (IAnimationTransform * _ani, long level)
{
	UnregisterAnimation();

	if (!_ani->Is(anitype_animation))
	{
		return;
	}

	SetAnimation((IAnimation *)_ani);
	RegistryBlendStage(level);
}

void GMXBlendShapeMixer::UnregisterAnimation()
{
	UnregistryBlendStage();
}


