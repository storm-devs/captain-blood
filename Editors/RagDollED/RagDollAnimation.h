
#ifndef RAGDOLL_ANIMATION_H
#define RAGDOLL_ANIMATION_H

#include "..\..\common_h\Animation.h"


struct RDBone
{
	string Name;
	Matrix InitMat;

	int parent;
	
	Matrix LocalMat;
	Matrix FinalMat;	

	Matrix SkinMat;

	Matrix BlenderLocalMat;

	bool NeedCalclFinalMat;
};

struct TOptBonesName
{
	string Name;
};
	
class RagDollAnimation //: public IAnimation  
{
	
	//--------------------------------------------------------------------------------------------
public:

	array<RDBone> Bones;

	int NumOptBones;
	Matrix * matrices;
	TOptBonesName * OptBonesNames;

	RagDollAnimation();
	virtual ~RagDollAnimation();

public:
	//Получить количество костей в анимации
	virtual long GetNumBones();
	//Получить имя кости
	virtual const char * GetBoneName(long index);


	//Получить имя родительской кости
	virtual string GetParentName(const char* BoneName);
	virtual string GetParentName(const char* BoneName, int Depth);

	//Получить индекс родительской кости
	virtual long GetBoneParent(long index);
};

/*
class RagDollBlender : public IAniBlendStage
{
public:

	virtual ~RagDollBlender()
	{
		UnregistryBlendStage();
	}

	RagDollAnimation* animation;

	//Получить коэфициент блендинга для интересующей кости
	virtual float GetBoneBlend(long boneIndex)
	{		
		return 1.0f;
	}

	//Получить трансформацию кости
	virtual void GetBoneTransform(long boneIndex, Quaternion & rotation, Vector & position, Vector & scale, const Quaternion & prevRotation, const Vector & prevPosition, const Vector & prevScale)
	{		
		rotation.Set(animation->Bones[boneIndex].BlenderLocalMat);
		position = animation->Bones[boneIndex].BlenderLocalMat.pos;
		scale = 1.0f;
	};
};*/


#endif