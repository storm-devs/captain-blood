

#include "SoundBanks.h"
#include "..\..\common_h\FileService.h"


SoundBanks::SoundBank::SoundBank()
{
	header = null;
	data = null;
	dataSize = 0;
	refCount = 0;
	nameHash = 0;
}

SoundBanks::SoundBank::~SoundBank()
{
#ifdef _XBOX
	XPhysicalFree(data);
#else
	delete data;
#endif
	data = null;
}

SoundBanks::SoundBanks() : soundBank(_FL_)
{
	fs = (IFileService *)api->GetService("FileService");
	Assert(fs);
	SoundBankFileId::CheckMachine();
}

SoundBanks::~SoundBanks()
{
	soundBank.DelAllWithPointers();
}

//Загрузить звуковой банк
bool SoundBanks::LoadSoundBank(const char * path)
{
	if(string::IsEmpty(path))
	{
		return false;
	}
	//Вычленяем имя файла
	strBufferPath = path;
	strBufferTitle.Empty();
	strBufferTitle.GetFileTitle(strBufferPath);
	strBufferTitle.Lower();
	dword hash = string::HashNoCase(strBufferTitle.c_str());
	//Ищем среди загруженных
	for(long i = 0; i < soundBank; i++)
	{
		SoundBank * sb = soundBank[i];
		if(sb->nameHash == hash)
		{
			if(sb->name == strBufferTitle)
			{
				sb->refCount++;
				return true;
			}
		}
	}
	//Нужно загружать с диска
#ifndef _XBOX
	strBufferPath = "Resource\\sounds\\";
#else
	strBufferPath = "";
#endif
	strBufferPath += path;
	strBufferPath.AddExtention(".ssb");
	IDataFile * file = fs->OpenDataFile(strBufferPath.c_str(), file_open_any, _FL_);
	if(!file)
	{
		api->Trace("SoundService: Can't open sound bank file: %s", strBufferPath.c_str());
		return false;
	}
	dword fileSize = file->Size();
	if(fileSize < sizeof(SoundBankFileId) + sizeof(SoundBankFileHeader) + sizeof(SoundBankFileSound))
	{
		api->Trace("SoundService: Can't load sound bank file: %s,\n   incorrect file size...", strBufferPath.c_str());
		file->Release();		
		return false;
	}
#ifndef GAME_RUSSIAN
	//Загружаем идентификационный заголовок
	SoundBankFileId sbfId;
	if(file->Read(&sbfId, sizeof(sbfId)) != sizeof(sbfId))
	{		
		api->Trace("SoundService: Can't load sound bank file: %s,\n   io error...", strBufferPath.c_str());
		file->Release();
		return false;
	}
	//Проверяем идентификатор
	if(!sbfId.CheckId())
	{		
		api->Trace("SoundService: Invalidate sound bank file id: %s", strBufferPath.c_str());
		file->Release();
		return false;		
	}
	if(!sbfId.CheckVer())
	{		
		api->Trace("SoundService: Invalidate sound bank file version: %s", strBufferPath.c_str());
		file->Release();
		return false;
	}
	if(sbfId.GetPCChankOffset() < sizeof(SoundBankFileId) + sizeof(SoundBankFileHeader) + sizeof(SoundBankFileSound))
	{		
		api->Trace("SoundService: Can't load sound bank file: %s,\n   incorrect file size...", strBufferPath.c_str());
		file->Release();
		return false;
	}
	//В зависимости от платформы определяем что сколько и как загружать
#ifdef _XBOX
	//Загружаем файл куском в память
	fileSize = sbfId.GetXboxChankSize();
	byte * data = (byte *)XPhysicalAlloc(fileSize, MAXULONG_PTR, 2048, PAGE_READWRITE);
	Assert(data != null);
#else
	//Определяем сколько нужно прочитать инфы
	dword skipBytes = sbfId.GetXboxChankSize();
	if(file->Read(null, skipBytes) != skipBytes)
	{		
		api->Trace("SoundService: Can't load sound bank file: %s,\n   io error...", strBufferPath.c_str());
		file->Release();
		return false;		
	}
	fileSize = sbfId.GetPCChankSize(fileSize);
	byte * data = NEW byte[fileSize];
#endif
#else
	byte * data = NEW byte[fileSize];
#endif
	if(file->Read(data, fileSize) != fileSize)
	{
#ifdef _XBOX
		XPhysicalFree(data);
#else
		delete data;
#endif
		api->Trace("SoundService: Can't load sound bank file: %s,\n   io error...", strBufferPath.c_str());
		file->Release();
		return false;
	}
	//Пытаемся подготовить данные к использованию
	SoundBankFileHeader * header = (SoundBankFileHeader *)data;
	if(!header->Restore(fileSize))
	{
#ifdef _XBOX
		XPhysicalFree(data);
#else
		delete data;
#endif
		api->Trace("SoundService: Sound bank file is damaged: %s", strBufferPath.c_str());
		file->Release();
		return false;
	}
	file->Release();
	file = null;
	//Декодировать волны
#ifdef GAME_RUSSIAN
	for(dword i = 0; i < header->wavesCount; i++)
	{
		header->waves[i].DecodeWaveData();
	}
#endif
	//Добавляем новую учётную запись
	SoundBank * sb = NEW SoundBank();
	sb->header = header;
	sb->data = data;
	sb->dataSize = fileSize;
	sb->refCount = 1;
	sb->nameHash = hash;
	sb->name = strBufferTitle;
	soundBank.Add(sb);
	return true;
}

//Удалить звуковой банк
void SoundBanks::ReleaseSoundBank(const char * path)
{
	path = GetSoundBankNativeName(path);
	for(dword i = 0; i < soundBank.Size(); i++)
	{
		SoundBank * sb = soundBank[i];
		if(sb->name.c_str() == path)
		{
			sb->refCount--;
			if(sb->refCount <= 0)
			{
				//Удаляем описание
				delete sb;
				//Удаляем запись
				soundBank.DelIndex(i);
				return;
			}
		}
	}
}

//Получить внутренний указатель на имя звукового банка, который сохранён в звуках
const char * SoundBanks::GetSoundBankNativeName(const char * path)
{
	//Вычленяем имя файла
	strBufferPath = path;
	strBufferTitle.Empty();
	strBufferTitle.GetFileTitle(strBufferPath);
	strBufferTitle.Lower();
	dword hash = string::HashNoCase(strBufferTitle.c_str());
	//Ищем среди загруженных
	for(dword i = 0; i < soundBank.Size(); i++)
	{
		SoundBank * sb = soundBank[i];
		if(sb->nameHash == hash)
		{
			if(sb->name == strBufferTitle)
			{
				return sb->name.c_str();
			}
		}
	}
	return null;
}

//Найти звук по имени
SoundBankFileSound * SoundBanks::FindSound(const ConstString & name, const char ** sbankName)
{
	if(name.IsEmpty())
	{
		return null;
	}
	return FindSound(name.c_str(), name.Hash(), name.Len(), null, sbankName);
}

//Найти звук по имени с расчитаными параметрами имени
SoundBankFileSound * SoundBanks::FindSound(const char * name, dword nameHash, dword nameLen, SoundBankFileHeader ** header, const char ** sbankName)
{
	for(long i = 0; i < soundBank; i++)
	{
		SoundBank * sb = soundBank[i];
		dword entryIndex = nameHash & sb->header->mask;
		SoundBankFileSound * snd = sb->header->entry[entryIndex];
		for(; snd; snd = snd->next)
		{
			if(snd->nameHash == nameHash && snd->nameLen == nameLen)
			{
				if(string::IsEqual(snd->name, name))
				{
					if(header)
					{
						*header = sb->header;
					}
					if(sbankName)
					{
						*sbankName = sb->name.c_str();
					}
					return snd;
				}
			}
		}
	}
	return null;
}











