#ifndef _AnimBlender_h_
#define _AnimBlender_h_

#include "..\..\..\Common_h\mission.h"

class AnimBlender : public IAniBlendStage
{
	struct PoseElement
	{
		Quaternion q;
		Vector p;
		long sourseIndex;
	};

public:
	AnimBlender();
	virtual ~AnimBlender();

	void Init(IAnimation* anim);

	//Получить необходимость в данных предыдущих стадий
	virtual bool NeedPrevStageData(long boneIndex);
	//Получить коэфициент блендинга для интересующей кости
	virtual float GetBoneBlend(long boneIndex);
	//Получить трансформацию кости
	virtual void GetBoneTransform(long boneIndex, Quaternion & rotation, Vector & position, Vector & scale, const Quaternion & prevRotation, const Vector & prevPosition, const Vector & prevScale);
	//Обновить состояние
	virtual void Update(float dltTime);

	void AddForce(float force);

	// установить угол вращения ствола по вертикали
	void SetBarrelAngle(float fAng) {barrel_rot = fAng;}

	// получить относительную величину скорости вращения ствола
	float GetRelativeAngularSpeed();

	float angular_speed;
	float angular_force;
	float cur_rot;

	bool stoped;

protected:

	long gun_bone;
	long barrel_bone;

	float barrel_rot;
};

#endif
