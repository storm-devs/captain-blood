//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// Mission objects
//===========================================================================================================================
// MissionEnvSound	
//============================================================================================
			

#include "MissionEnvSound.h"

//============================================================================================

MissionEnvSound::MissionEnvSound()
{
}

MissionEnvSound::~MissionEnvSound()
{
}


//============================================================================================

//Инициализировать объект
bool MissionEnvSound::Create(MOPReader & reader)
{
	return CreateSounds(reader, null);
}

//Инициализировать объект
bool MissionEnvSound::EditMode_Create(MOPReader & reader)
{
	Create(reader);
	return true;
}

//Обновить параметры
bool MissionEnvSound::EditMode_Update(MOPReader & reader)
{
	Create(reader);
	return true;
}

//============================================================================================
//Параметры инициализации
//============================================================================================

MOP_BEGINLISTCG(MissionEnvSound, "Environment sounds", '1.00', 100, "Not posiotion sound, playing as stereo", "Effects")
	MISSION_SOUND_PARAMS
MOP_ENDLIST(MissionEnvSound)


