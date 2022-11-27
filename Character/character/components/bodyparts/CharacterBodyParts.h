#pragma once

#include "..\..\..\..\Common_h\Mission.h"
#include "CharacterBlender.h"

#include "Face\CharacterFace.h"
#include "Hair\CharacterHair.h"
#include "Head\CharacterHeadAim.h"
#include "Legs\CharacterLegs.h"
#include "Torso\CharacterTorsoAim.h"

class CharacterBodyParts : public IAniBlendStage
{
public:

	 CharacterBodyParts();
	~CharacterBodyParts();

	void Init(const CharacterBlender::InitData &data);

public:

	virtual bool NeedPrevStageData(long boneIndex);

	virtual float GetBoneBlend	  (long boneIndex);
	virtual void  GetBoneTransform(long boneIndex, Quaternion &rotation, Vector &position, Vector &scale, const Quaternion &prevRotation, const Vector &prevPosition, const Vector &prevScale);

	virtual void Update(float dltTime);

	virtual void Draw(float dltTime);

public:

	void SetMatrix(const Matrix &m)  { hair.SetMatrix(m); }
	void SetDeltaPos(Vector pos)	 { hair.SetDeltaPos(pos); }

	void SetRotation(float rotation, bool isHead);
	void SetLimit	(float limit   , bool isHead);
	void SetOffset	(float angle   , bool isHead);
	
	void Activate(bool isActive, ISoundScene *scene) { face.Activate(isActive,scene); }
	void SetDead (bool isDead)						 { face.SetDead(isDead); }
	//void SetVoice(ISound3D *v, IGMXEntity *loc)		 {  }

	void ResetAnimation(IAnimation *animation);

	void ActivateHair(bool act = true)
	{
		hair.Activate(act);
	}

	void ActivateLegs(bool act = true)
	{
	//	legs.Activate(act);
		legs.setEnabled(act);
	}

private:

	array<CharacterBlender::Link> map;

	CharacterFace	  face;
	CharacterHair	  hair;
	CharacterHeadAim  head;
	CharacterLegs	  legs;
	CharacterTorsoAim tors;

	friend CharacterBlender;

};
