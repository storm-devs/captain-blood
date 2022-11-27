
#ifndef _MissionFixedCamera_h_
#define _MissionFixedCamera_h_

#include "..\..\..\Common_h\Mission\Mission.h"
#include "..\..\..\Common_h\ICharacter.h"
#include "..\camera_inc.h"

class CharacterCamera : public BaseCamera
{
private:
	//Прочитать параметры идущие после стандартных
	virtual bool CameraPreCreate(MOPReader & reader);
	//Получить позицию камеры
	virtual void GetCameraPosition(Vector & position);
	//Получить точку наблюдения камеры
	virtual bool GetCameraTarget(const Vector & position, Vector & target_pos, Vector & up);	

	virtual void UpdateCharacterData();

	virtual float GetCameraFoV();

	//--------------------------------------------------------------------------------------------
private:
	ConstString    characterName;
	MOSafePointer  charObject;

	const char*    boneName;
	long           boneIndex;
	long           fovBoneIndex;
	float          fovFromBone;

	IAnimation*    characterAnim;
	Matrix         bone_matrix;
};

#endif