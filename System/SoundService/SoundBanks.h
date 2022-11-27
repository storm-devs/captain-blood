
#pragma once

#include "SoundBankFile.h"

class IFileService;

class SoundBanks
{
	struct SoundBank
	{
		SoundBank();
		~SoundBank();
		SoundBankFileHeader * header;
		byte * data;
		dword dataSize;
		long refCount;
		dword nameHash;
		string name;
	};

public:
	SoundBanks();
	virtual ~SoundBanks();
	
	//Загрузить звуковой банк
	bool LoadSoundBank(const char * path);
	//Удалить звуковой банк
	void ReleaseSoundBank(const char * path);
	//Получить внутренний указатель на имя звукового банка, который сохранён в звуках
	const char * GetSoundBankNativeName(const char * path);

	//Найти звук по имени
	SoundBankFileSound * FindSound(const ConstString & name, const char ** sbankName);
	//Найти звук по имени с расчитаными параметрами имени
	SoundBankFileSound * FindSound(const char * name, dword nameHash, dword nameLen, SoundBankFileHeader ** header = null, const char ** sbankName = null);


private:
	array<SoundBank *> soundBank;
	IFileService * fs;
	string strBufferTitle;
	string strBufferPath;
};




