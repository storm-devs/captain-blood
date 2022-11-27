/********************************************************************
created:	2010 jan
author:		Sergey Makeev
purpose:	bone blend shapes animation mixer
*********************************************************************/
#ifndef GMX_BLEND_SHAPES_MIXER
#define GMX_BLEND_SHAPES_MIXER

#include "..\..\common_h\animation.h"

#define DEFAULT_BLENDSHAPE_LEVEL (0)

struct blendShapeTarget;
struct blendShapeBoneTransform;

class GMXBlendShapeMixer : public IAniBlendStage
{

	dword bsMorphTargetsCount;
	float * pMorphTargetsWeights;
	blendShapeTarget * pMorphTargets;
	dword bsBonesTransformCount;
	blendShapeBoneTransform * pBoneTransforms;
	long * pAnimToGeomBlendShapes_TranslateTable;

public:



	GMXBlendShapeMixer(dword _bsMorphTargetsCount, float * _pMorphTargetsWeights, blendShapeTarget * _pMorphTargets,
                   dword _bsBonesTransformCount, blendShapeBoneTransform * _pBoneTransforms,
                   long * _pAnimToGeomBlendShapes_TranslateTable);

	virtual ~GMXBlendShapeMixer();

	//Получить коэфициент блендинга для интересующей кости (многопоточная функция)
	virtual float GetBoneBlend(long boneIndex);

	//Получить трансформацию кости (многопоточная функция)
	virtual void GetBoneTransform(long boneIndex, Quaternion & rotation, Vector & position, Vector & scale, const Quaternion & prevRotation, const Vector & prevPosition, const Vector & prevScale);

	//Обновить (только основной поток, второй спит вне цикла исполнения)
	virtual void Update(float dltTime);


	void RegisterAnimation (IAnimationTransform * _ani, long level);
	void UnregisterAnimation();


};



#endif