#pragma once

#include "..\..\..\..\..\Common_h\Mission.h"
#include "..\CharacterBlender.h"

class CharacterFace : public CharacterBlender
{
	enum Flags
	{		
		flag_isProcess = 1,
		flag_isDead = 2,
	};

public:
	 CharacterFace();
	~CharacterFace();

	virtual void Init(const InitData &data);

	virtual void Update(float dltTime);
	virtual void GetBoneTransform(int i, BlendData &data);

	virtual void ResetAnimation(IAnimation *animation);

public:

	void Activate(bool isActive, ISoundScene *scene);
	void SetDead(bool isDead);

private:
	
	float eyesPosition;					//0 - закрытые глаза, 1 - открытые глаза
	float timeForNextWink;				//Время до следующего моргания	
	dword flags;						//Мёртв ли персонаж
	Quaternion eyelidsRot;				//Поворот век
};
