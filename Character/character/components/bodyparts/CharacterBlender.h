#pragma once

#include "..\..\..\..\Common_h\Mission.h"

class Character;
class CharacterBodyParts;

class CharacterBlender
{
public:

	struct InitData
	{
		IAnimation *animation;
		IGMXScene  *model;

		Character  *chr;
	};

	struct Link
	{
		CharacterBlender *p; int i;

		Link()
		{
			p = null;
		}

		operator bool()
		{
			return p != null;
		}
	};

	struct BlendData
	{
		long boneIndex;

		Quaternion &rotation;
		Vector	   &position;
		Vector	   &scale;

		const Quaternion &prevRotation;
		const Vector	 &prevPosition;
		const Vector	 &prevScale;
	};

public:

	CharacterBlender();

	virtual void Init(const InitData &data) = 0;

	virtual void Update(float dltTime) = 0;
	virtual void GetBoneTransform(int i, BlendData &data) = 0;

	virtual void ResetAnimation(IAnimation *animation) = 0;

	virtual void Draw(float dltTime){};

	virtual float GetBoneBlend(long boneIndex)
	{
		return 1.0f;
	}

	void SetParent(CharacterBodyParts *p)
	{
		parent = p;
	}

	void Activate(bool act = true) // внешнее запрещение обработки
	{
		active = act;
	}

	bool Enabled()
	{
		return active && enabled;
	}

protected:

	Character  *chr;

	void RegBone(int boneIndex, int i = 0);
	void ReleaseBones();

	bool enabled;

	void  Enable() { enabled =  true; }
	void Disable() { enabled = false; }

	bool active;

private:

	CharacterBodyParts *parent;

};
