
#include "DoorBlender.h"

void DoorBlendStage::GetBoneName(const char* name,string& BoneName)
{	
	BoneName = name;
	dword len = BoneName.Size();
	if (len == 0) return;

	len--;

	for (dword n = len; n > 0; n--)
	{
		if (BoneName[n] == '|')
		{
			BoneName.Delete(0, n);

			return;
		}
	}

	return;
}

void DoorBlendStage::Init(IGMXScene* door,IAnimation* anim)
{			
	SetAnimation(anim);

	boneDataReadOnly * boneData;
	boneMtxInputReadOnly * boneTransformations;

	dword count = door->GetBonesArray(&boneData, &boneTransformations);
	for (dword i=0; i<count; i++)
	{				
		const boneDataReadOnly & bone = boneData[i];

		string boneName;
		GetBoneName(bone.name.c_str(), boneName);

		Matrix mat2 = boneTransformations[i].mtxBindPose;//pBone->mInitial;
		Matrix mat = boneTransformations[i].mtxBindPose;//pBone->mInitial;
		mat.Inverse();				
		//mat.pos.x=-mat.pos.x;
		//mat.pos= 0.0f;

		bool skip = false;
		for (int i=0;i<4;i++)
		{
			if (string::IsEqual(bones[i].name,boneName.c_str()))
			{
				skip = true;
				break;
			}
		}				

		if (skip) continue;

		if (string::IsEqual(boneName.c_str(),"door_left"))
		{														
			bones[0].pos = mat.pos;
			bones[0].boneIndex = anim->FindBone("|door|door_left");
			bones[0].mat = mat;
			bones[0].inv_mat = mat2;//pBone->mInitial;
			crt_strcpy(bones[0].name,63,"|door_left");								
		}
		else
			if (string::IsEqual(boneName.c_str(),"door_right"))
			{							
				bones[1].pos = mat.pos;
				bones[1].boneIndex = anim->FindBone("|door|door_right");
				bones[1].mat = mat;
				bones[1].inv_mat = mat2;//pBone->mInitial;
				crt_strcpy(bones[1].name,63,"|door_right");								
			}
			else
				if (string::IsEqual(boneName.c_str(),"door_ring_left"))
				{							
					bones[2].pos = mat.pos;
					bones[2].boneIndex = anim->FindBone("|door|door_left|door_ring_left");
					bones[2].mat = mat;
					bones[2].inv_mat = mat2;//pBone->mInitial;
					crt_strcpy(bones[2].name,63,"|door_ring_left");								
				}
				else
					if (string::IsEqual(boneName.c_str(),"door_ring_right"))
					{							
						bones[3].pos = mat.pos;
						bones[3].boneIndex = anim->FindBone("|door|door_right|door_ring_right");
						bones[3].mat = mat;
						bones[3].inv_mat = mat2;//pBone->mInitial;
						crt_strcpy(bones[3].name,63,"|door_ring_right");								
					}						
	}






	long index = anim->FindBone("|door");

	Matrix  mat;

	if (index!=-1)
	{
		Matrix  mat = anim->GetBoneMatrix(index);
		mat.Inverse();
	}


	bones[0].pos = (bones[0].mat * mat).pos;
	bones[1].pos = (bones[1].mat * mat).pos;

	bones[2].pos = (bones[2].mat * bones[0].inv_mat).pos;
	bones[3].pos = (bones[3].mat * bones[1].inv_mat).pos;


	RegistryBlendStage(5);
}

void DoorBlendStage::GetBoneTransform(long boneIndex,
											Quaternion & rotation, Vector & position, Vector & scale,
											const Quaternion & prevRotation, const Vector & prevPosition, const Vector & prevScale)
{
	rotation = prevRotation;
	position = prevPosition;
	scale = prevScale;

	for (int i=0;i<4;i++)
		if (bones[i].boneIndex == boneIndex)
		{
			position = bones[i].pos;
			break;
		}		 
}