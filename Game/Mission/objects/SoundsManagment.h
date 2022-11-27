//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// Mission objects
//============================================================================================
// SoundsManagment
//============================================================================================

#ifndef _SoundsManagment_h_
#define _SoundsManagment_h_

#include "..\..\..\Common_h\Mission\Mission.h"



class LoadSoundBanks : public MissionObject
{
public:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
};




#endif

