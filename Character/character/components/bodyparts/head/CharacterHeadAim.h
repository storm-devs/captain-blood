#pragma once

#include "..\..\..\..\..\Common_h\Mission.h"
#include "..\CharacterBlender.h"

class CharacterHeadAim : public CharacterBlender
{
public:

	 CharacterHeadAim();
	~CharacterHeadAim();

	virtual void Init(const InitData &data);

	virtual void Update(float dltTime);
	virtual void GetBoneTransform(int i, BlendData &data);

	virtual void ResetAnimation(IAnimation *animation);

public:

	virtual void SetRotation(float rotation);
	void SetLimit(float limit);
	void SetOffset(float angle);
	
//private:
	long head_bone;	
	float cur_rot;
	float rot;
	float fLimit;
	float offset; // базовый угол, от которого считается отклонение

};
