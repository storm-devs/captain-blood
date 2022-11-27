
#include "CharacterArmor.h"
#include "CharacterItems.h"
#include "CharacterPhysics.h"

		
CharacterArmor::CharacterArmor(Character * character): chr(*character),data(_FL_)
{
	armor_model = null;
	armor_anim = null;
}

CharacterArmor::~CharacterArmor()
{	
	RELEASE(armor_anim);
	RELEASE(armor_model);
}

void CharacterArmor::Init(IGMXScene* model, IAnimationScene* animation)
{	
	if (!model) return;
	
	armor_model = model;

	armor_model->SetFloatAlphaReference(0.0f);
	
	bone_count = 0;
	
	static IAnimationScene::Bone creationSkeleton[32];

	boneDataReadOnly * boneData;
	boneMtxInputReadOnly * boneTransformations;

	dword count = model->GetBonesArray(&boneData, &boneTransformations);
	for (dword i=0; i<count; i++)
	{			
		const boneDataReadOnly & bone = boneData[i];

		if (i < 32)
		{
			bboxes[bone_count].bMin = bone.bbMin;
			bboxes[bone_count].bMax = bone.bbMax;

			creationSkeleton[bone_count].name = bone.name.c_str();

			bone_count++;

			item_data item;
			item.used = false;
			item.visible = false;

			data.Add(item);
		}
	}
	
	armor_anim = animation->CreateProcedural(creationSkeleton, bone_count, _FL_);
	armor_model->SetAnimation(armor_anim);	
}

bool CharacterArmor::RegisterArmor(const char* bone, int index, Vector &bMin, Vector &bMax)
{
	if (armor_anim)
	{
		int bone_index = armor_anim->FindBone(bone,true);

		if (bone_index >= 0)
		{
			bMin = bboxes[bone_index].bMin;
			bMax = bboxes[bone_index].bMax;

			data[bone_index].item_index = index;
			data[bone_index].used = true;
			data[bone_index].visible = true;

			return true;
		}
	}

	return false;	
}

void CharacterArmor::Draw(Matrix mtx)
{
	float alpha;	

#ifndef _XBOX
	/*bool spiked = fabsf(curScaleSpike - 1.0f) > 1e-5f;
	if (spiked)
		mtx = Matrix().BuildScale(curScaleSpike) * mtx;*/
#endif

	for(int i = 0; i < (int)data.Size(); i++)
	{
		if (data[i].used)
		{
			ItemData & item = chr.items->items[data[i].item_index];

			if (item.visible) data[i].visible = true;			

			if (data[i].visible)
			{
				Matrix mat(false);			

				if (item.visible)
				{								
					if(!chr.items->GetLocatorMatrix(item.locator, mat)) continue;
					mat = mat * mtx;
					alpha = chr.GetAlpha();
				}
				else
				{								
					mat = item.drp_data.transform;
					alpha = item.drp_data.alpha;
				}
				
				if (alpha>0.05f)
				{
					armor_anim->SetBoneMatrix(i,mat);
					armor_model->SetBoneAlpha(i,alpha);
				}
				else
				{
					armor_anim->CollapseBone(i);
				}
			}
			else
			{
				armor_anim->CollapseBone(i);
			}			
		}
		else
		{
			armor_anim->CollapseBone(i);
		}		
	}

	armor_model->Draw();
}