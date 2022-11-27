//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// Mission objects
//===========================================================================================================================
// SoundsManagment
//============================================================================================
			

#include "SoundsManagment.h"

#define BootupSoundBank	"mini"

//============================================================================================


class SoundBanksKipper : public Service
{
public:

	SoundBanksKipper()
	{
		s = null;
		isLoad = false;
	}

	//Инициализация
	virtual bool Init()
	{ 
		isLoad = false;
		s = (ISoundService *)api->GetService("SoundService");
		if(!s)
		{
			api->Trace("GlobalSoundBankKipper error: SoundService not found! Not load global sound bank.");
			return false;
		}		
		s->LoadSoundBank(BootupSoundBank);
		IFileService * fs = (IFileService *)api->GetService("FileService");
		if(fs)
		{
			bool isDelayedLoading = (fs->SystemIni()->GetLong("sound", "delayedloading", 0) != 0);
			if(isDelayedLoading)
			{
				return true;
			}
		}
		LoadSoundBanks();
		return true; 
	}

	virtual ~SoundBanksKipper()
	{
		ISoundService * s = (ISoundService *)api->GetService("SoundService");
		Assert(s);
#ifndef GAME_RUSSIAN
		s->ReleaseSoundBank(localBankPath.c_str());
#endif
		s->ReleaseSoundBank(SoundService_GlobalSoundBank);
		s->ReleaseSoundBank(BootupSoundBank);
	}


	void LoadSoundBanks()
	{
		if(isLoad)
		{
			return;
		}
		isLoad = true;
		s->LoadSoundBank(SoundService_GlobalSoundBank);
		ILocStrings * ls = (ILocStrings *)api->GetService("LocStrings");
		if(!ls)
		{
			api->Trace("GlobalSoundBankKipper error: LocStrings not found! Not load local sound bank.");
			return;
		}
#ifndef GAME_RUSSIAN
		localBankPath = SoundService_GlobalSoundBank;
		localBankPath += "_";
		localBankPath += ls->GetLocId();
		s->LoadSoundBank(localBankPath.c_str());
#endif
	}



private:
	ISoundService * s;
	bool isLoad;
	string localBankPath;
};

CREATE_SERVICE(SoundBanksKipper, 0x7fffffff - 10)

//============================================================================================

//Инициализировать объект
bool LoadSoundBanks::Create(MOPReader & reader)
{
	SoundBanksKipper * sbk = (SoundBanksKipper *)api->GetService("SoundBanksKipper");
	if(!sbk)
	{
		LogicDebugError("Service SoundBanksKipper not found");
		return false;
	}
	sbk->LoadSoundBanks();
	LogicDebug("Load sound banks...");
	return true;
}

MOP_BEGINLISTCG(LoadSoundBanks, "Load sound banks", '1.00', 0, "Load global and local sound banks when load mission", "Effects")
MOP_ENDLIST(LoadSoundBanks)



