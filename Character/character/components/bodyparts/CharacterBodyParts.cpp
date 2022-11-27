#include "CharacterBodyParts.h"

#include "..\..\..\..\Common_h\AnimationBlendLevels.h"
#include "..\..\Character.h"

CharacterBodyParts:: CharacterBodyParts() : map(_FL_)
{
}

CharacterBodyParts::~CharacterBodyParts()
{
	UnregistryBlendStage();
}

void CharacterBodyParts::Init(const CharacterBlender::InitData &data)
{
	UnregistryBlendStage();

	if( !data.animation )
		return;

	SetAnimation(data.animation);

	map.DelAll();
	map.AddElements(data.animation->GetNumBones());

	face.SetParent(this);
	hair.SetParent(this);
	head.SetParent(this);
	legs.SetParent(this);
	tors.SetParent(this);

	face.Init(data);
	hair.Init(data);
	head.Init(data);
	legs.Init(data);
	tors.Init(data);

	RegistryBlendStage(aminationBlendLevel_autoEyes);
}

bool CharacterBodyParts::NeedPrevStageData(long boneIndex)
{
	return true;
}

float CharacterBodyParts::GetBoneBlend(long boneIndex)
{
/*	if( map[boneIndex] )
		return 1.0f;
	else
		return 0.0f;*/

	CharacterBlender *p = map[boneIndex].p;

	if( p )
	{
		return p->GetBoneBlend(boneIndex);
	}
	else
		return 0.0f;
}

void CharacterBodyParts::GetBoneTransform(long boneIndex, Quaternion &rotation, Vector &position, Vector &scale, const Quaternion &prevRotation, const Vector &prevPosition, const Vector &prevScale)
{
	CharacterBlender::Link &link = map[boneIndex];

	if( link && link.p->Enabled())
	{
		CharacterBlender::BlendData data = {boneIndex,rotation,position,scale,prevRotation,prevPosition,prevScale};

		link.p->GetBoneTransform(link.i,data);
	}		
	else
	{
		rotation = prevRotation;
		position = prevPosition;
		scale	 = prevScale;
	}
}

void CharacterBodyParts::Update(float dltTime)
{
	if( !Ani())
		return;

	face.Update(dltTime);
	head.Update(dltTime);
	hair.Update(dltTime);
	legs.Update(dltTime);
	tors.Update(dltTime);
}

void CharacterBodyParts::Draw(float dltTime)
{
	if( !Ani())
		return;

	face.Draw(dltTime);
	head.Draw(dltTime);
	hair.Draw(dltTime);
	legs.Draw(dltTime);
	tors.Draw(dltTime);
}

void CharacterBodyParts::ResetAnimation(IAnimation *animation)
{
	UnregistryBlendStage();

	if( !animation )
		return;

	SetAnimation(animation);

	map.DelAll();
	map.AddElements(animation->GetNumBones());

	face.ResetAnimation(animation);
	head.ResetAnimation(animation);
	hair.ResetAnimation(animation);
	legs.ResetAnimation(animation);
	tors.ResetAnimation(animation);

	RegistryBlendStage(aminationBlendLevel_autoEyes);
}

void CharacterBodyParts::SetRotation(float rotation, bool isHead)
{ 	
	if( isHead )
	{
		head.SetRotation(rotation);
	}
	else
	{
		tors.SetRotation(rotation);
	}
}

void CharacterBodyParts::SetLimit(float limit, bool isHead)
{	
	if( isHead )
	{
		head.SetLimit(limit);
	}
	else
	{
		tors.SetLimit(limit);
	}
}

void CharacterBodyParts::SetOffset(float angle, bool isHead)
{
	if( isHead )
	{
		head.SetOffset(angle);
	}
	else
	{
		tors.SetOffset(angle);
	}
}
