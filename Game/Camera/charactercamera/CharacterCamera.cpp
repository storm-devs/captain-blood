
#include "CharacterCamera.h"


//Прочитать параметры идущие после стандартных
bool CharacterCamera::CameraPreCreate(MOPReader & reader)
{
	characterName = reader.String();
	charObject.Reset();

	boneName = reader.String().c_str();
	boneIndex = -1;
	fovBoneIndex = -1;
	fovFromBone = 0.0f;
	
	characterAnim = NULL;

	return true;
}

//Получить позицию камеры
void CharacterCamera::GetCameraPosition(Vector & position)
{
	UpdateCharacterData();

	position = bone_matrix.pos;	
}

//Получить точку наблюдения камеры
bool CharacterCamera::GetCameraTarget(const Vector & position, Vector & target_pos, Vector & up)
{
	UpdateCharacterData();

	if(BaseCamera::GetCameraTarget(position, target_pos, up)) return true;
	up = bone_matrix.vy;
	target_pos = bone_matrix.pos - bone_matrix.vz;

	return true;
}


void CharacterCamera::UpdateCharacterData()
{
	ICharacter * character = null;
	if(charObject.Validate())
	{
		character = (ICharacter*)charObject.Ptr();
	}else{
		if(FindObject(characterName, charObject))
		{
			static const ConstString charId("Character");
			if (charObject.Ptr()->Is(charId))
			{
				character = (ICharacter*)charObject.Ptr();
			}else{
				charObject.Reset();
			}
		}
	}
	
	if (character)
	{
		
		if (character->GetAnimation() != characterAnim)
		{
			characterAnim = character->GetAnimation();
			if (characterAnim)
			{
				boneIndex = characterAnim->FindBone(boneName, true);
				fovBoneIndex = characterAnim->FindBone("camera_fov", true);
			}
			else
			{
				boneIndex = -1;
				fovBoneIndex = -1;
			}
		}
		
		character->GetColiderMatrix(bone_matrix);

		if (characterAnim && boneIndex >= 0)
		{
			bone_matrix = characterAnim->GetBoneMatrix(boneIndex) * bone_matrix;


			if (fovBoneIndex >= 0)
			{
				fovFromBone = characterAnim->GetBoneMatrix(fovBoneIndex).pos.x;
			}
		}		
	}
	else
	{
		bone_matrix.SetIdentity();
	}
}


float CharacterCamera::GetCameraFoV()
{
	if (fovFromBone <= 0.001f)
	{
		return BaseCamera::GetCameraFoV();
	}

	//fov'ы для 4:3 у нас в движке, а из Maya 16:9 переводим...
	return (fovFromBone / 1.225f);
}

MOP_BEGINLISTCG(CharacterCamera, "Character camera", '1.00', 0, CAMERA_COMMENT("character camera\n\n"), "Cameras")
	MOP_STRING("Character","")
	MOP_STRING("Bone Name","")	
	MOP_CAM_STD	
MOP_ENDLIST(CharacterCamera)

