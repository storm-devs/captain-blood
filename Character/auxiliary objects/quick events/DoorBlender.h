#ifndef DoorBlender_H
#define DoorBlender_H

#include "..\..\..\common_h\animation.h"
#include "..\..\..\common_h\gmx.h"
	
class DoorBlendStage : public IAniBlendStage
{	
	public:

	struct TBonePos
	{
		long boneIndex;		
		Vector pos;
		char name[64];
		Matrix inv_mat;
		Matrix mat;

		TBonePos()
		{
			boneIndex = -1;
			name[0]=0;
		};
	};

	TBonePos bones[4];

	virtual ~DoorBlendStage()
	{
		UnregistryBlendStage();
	}

	//Получить индекс родительской кости
	void GetBoneName(const char* name,string& BoneName);

	void Init(IGMXScene* door,IAnimation* anim);

	//Получить необходимость в данных предыдущих стадий
	virtual bool NeedPrevStageData(long boneIndex) { return true;};

	//Получить коэфициент блендинга для интересующей кости
	virtual float GetBoneBlend(long boneIndex) { return 1.0f;};
	
	//Получить трансформацию кости
	virtual void GetBoneTransform(long boneIndex,
								  Quaternion & rotation, Vector & position, Vector & scale,
								  const Quaternion & prevRotation, const Vector & prevPosition, const Vector & prevScale);
};

#endif