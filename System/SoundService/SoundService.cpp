


#include "SoundService.h"
#include "SoundsEngine.h"
#include "SoundBanks.h"
#include "Sound.h"
#include "..\..\common_h\FileService.h"
#include "..\..\common_h\LocStrings.h"
#include "UnitTests\SoundServiceUnitTests.h"


//============================================================================================

CREATE_SERVICE_NAMED("SoundService", SoundService, 2)

//============================================================================================


SoundService::SoundService() : scenes(_FL_)
							#ifndef NO_TOOLS
								, previewSounds(_FL_)
							#endif
{
	states = 0;
	engine = NEW SoundsEngine();
	soundBanks = NEW SoundBanks();
	//Громкости
	globalVolume = 1.0f;
	soundsVolume = 1.0f;
	musicVolume = 0.4f;
	//Текущие громкости
	curSoundsVolume = globalVolume*soundsVolume;
	curMusicVolume = globalVolume*musicVolume;
	//Доступ к громкостям в базе
	stGlobalVolume = api->Storage().GetItemFloat("Options.GlobalVolume", _FL_);
	Assert(stGlobalVolume);
	stSoundsVolume = api->Storage().GetItemFloat("Options.FxVolume", _FL_);
	Assert(stSoundsVolume);
	stMusicVolume = api->Storage().GetItemFloat("Options.MusicVolume", _FL_);
	Assert(stMusicVolume);
#ifndef NO_TOOLS
	isEnablePreview = false;
	
	
	//********************
//	UnitTestWaveSelectorsProcess(false);
	//********************
#endif	
}

SoundService::~SoundService()
{
	while(scenes)
	{
		scenes[scenes - 1]->UnReleaseMessage();
		scenes[scenes - 1]->Release();
	}	
	SoundScene::ClearPulls();
	delete engine;
	delete soundBanks;	
	stGlobalVolume->Release();
	stSoundsVolume->Release();
	stMusicVolume->Release();
}

//Инициализвация
bool SoundService::Init()
{
	//Параметры сервиса
	IFileService * fs = (IFileService *)api->GetService("FileService");
	Assert(fs);
	IIniFile * ini = fs->SystemIni();
	if(ini)
	{
		if(ini->GetLong("Sound", "off", 0) != 0)
		{
			states |= SoundsEngine::dbg_snd_off;
		}
		long traceMode = ini->GetLong("Sound", "trace", 0);
		if(traceMode != 0)
		{
			states |= SoundsEngine::dbg_logout;
		}
		if(traceMode > 1)
		{
			states |= SoundsEngine::dbg_tracecreates;
		}
		long dl = ini->GetLong("Sound", "debug", 0);
		switch(dl)
		{
		case 0:
			SetDebugView(dv_none);
			break;
		case 1:
			SetDebugLevel(dl_minimal);
			SetDebugView(dv_all);
			break;
		default:
			SetDebugLevel(dl_maximum);
			SetDebugView(dv_all);
		}
		globalVolume = (float)ini->GetDouble("Sound", "global volume", 1.0f);
		soundsVolume = (float)ini->GetDouble("Sound", "sounds volume", 1.0f);
		musicVolume = (float)ini->GetDouble("Sound", "music volume", 0.4f);
		ini = null;
	}
	//Если выключен то прекращаем инициализацию
	if(states & SoundsEngine::dbg_snd_off)
	{
		api->Trace("SoundService started in silence mode (off)...");
		return true;
	}
	//Инициализируем звук
	if(!engine->Init())
	{
		api->Trace("SoundService error: Sound engine init error. Switch to silence mode (off)...");
		engine->Release();
		states |= SoundsEngine::dbg_snd_off;
		return true;
	}
	//Подписываем сервис
	api->SetEndFrameLevel(this, Core_DefaultExecuteLevel);
	api->Trace("SoundService started is successful");
	states |= s_needUpdateVolumes;
	EndFrame(0.0f);
	return true;
}

//Исполнение
void SoundService::EndFrame(float dltTime)
{
	//Проверяем изменение громкостей
	float vol = Clampf(stGlobalVolume->Get(globalVolume), 0.0f, 1.0f);
	if(fabsf(vol - globalVolume) > 1e-8f)
	{
		globalVolume = vol;
		states |= s_needUpdateVolumes;
	}
	vol = Clampf(stSoundsVolume->Get(soundsVolume), 0.0f, 1.0f);
	if(fabsf(vol - soundsVolume) > 1e-8f)
	{
		soundsVolume = vol;
		states |= s_needUpdateVolumes;
	}
	vol = Clampf(stMusicVolume->Get(musicVolume), 0.0f, 1.0f)*0.6f;
	if(fabsf(vol - musicVolume) > 1e-8f)
	{
		musicVolume = vol;
		states |= s_needUpdateVolumes;
	}
	//Звук не умеет скалироваться
	dltTime = api->GetNoScaleDeltaTime();
	if(!(states & s_needUpdateVolumes))
	{
		//Обновляем сцены
		for(long i = 0; i < scenes; i++)
		{
			scenes[i]->Update(dltTime);
		}
	}else{
		curSoundsVolume = soundsVolume*globalVolume;
		curMusicVolume = musicVolume*globalVolume;
#ifndef NO_TOOLS
		engine->EditPrewiewSetMasterVolume(curSoundsVolume);
#endif
		//Обновляем сцены
		for(long i = 0; i < scenes; i++)
		{
			scenes[i]->UpdateVolumes();
			scenes[i]->Update(dltTime);
		}
		states &= ~s_needUpdateVolumes;
	}
	if(engine)
	{
		engine->Update();
		bool isPause = !api->IsActive();
		engine->SetPause(isPause);
	}
	
	
}

//Создать звуковую сцену
ISoundScene * SoundService::CreateScene(const char * ovnerName, const char * cppFile, long cppLine)
{
	if(states & SoundsEngine::dbg_snd_off)
	{
		return &sceneOff;
	}
	SoundScene * scene = NEW SoundScene(*this, ovnerName, cppFile, cppLine);
	scenes.Add(scene);
	return scene;
}

//Загрузить звуковой банк
bool SoundService::LoadSoundBank(const char * path)
{
	if(states & SoundsEngine::dbg_snd_off)
	{
		return true;
	}
	return soundBanks->LoadSoundBank(path);
}

//Удалить звуковой банк
void SoundService::ReleaseSoundBank(const char * path)
{
	//Поток XAudio после удаления продолжал работать, поэтому банки не удаляю
	return;

	if(states & SoundsEngine::dbg_snd_off)
	{
		return;
	}
	path = soundBanks->GetSoundBankNativeName(path);
	if(!path)
	{
		return;
	}
	for(dword i = 0; i < scenes.Size(); i++)
	{
		scenes[i]->CheckForDelete(path);
	}
	return soundBanks->ReleaseSoundBank(path);
}

//Установить общую громкость
void SoundService::SetGlobalVolume(float volume)
{
	globalVolume = volume;
	states |= s_needUpdateVolumes;
}

//Получить общую громкость
float SoundService::GetGlobalVolume()
{
	return globalVolume;
}

//Установить громкость звуков
void SoundService::SetSoundsVolume(float volume)
{
	soundsVolume = volume;
	states |= s_needUpdateVolumes;
}

//Получить громкость звуков
float SoundService::GetSoundsVolume()
{
	return soundsVolume;
}

//Установить громкость музыки
void SoundService::SetMusicVolume(float volume)
{
	musicVolume = volume;
}

//Получить громкость музыки
float SoundService::GetMusicVolume()
{
	return musicVolume;
}

//Установка отладочных параметров
void SoundService::SetDebugLevel(DebugLevel dl)
{
	switch(dl)
	{
	case dl_minimal:
		states = (states & ~SoundsEngine::dbg_drawlevel_mask) | SoundsEngine::dbg_drawlevel_min;
		break;
	case dl_maximum:
		states = (states & ~SoundsEngine::dbg_drawlevel_mask) | SoundsEngine::dbg_drawlevel_max;
		break;
	}
}

SoundService::DebugLevel SoundService::GetDebugLevel()
{
	if((states & SoundsEngine::dbg_drawlevel_mask) == SoundsEngine::dbg_drawlevel_max)
	{
		return dl_maximum;
	}
	return dl_minimal;
}

void SoundService::SetDebugView(dword dv)
{
	states &= ~(SoundsEngine::dbg_draw2d | SoundsEngine::dbg_draw3d | SoundsEngine::dbg_drawlistener);
	if(dv & dv_2d)
	{
		states |= SoundsEngine::dbg_draw2d;
	}
	if(dv & dv_3d)
	{
		states |= SoundsEngine::dbg_draw3d;
	}
	if(dv & dv_listener)
	{
		states |= SoundsEngine::dbg_drawlistener;
	}
}

dword SoundService::GetDebugView()
{
	dword debugView = 0;
	if(states & SoundsEngine::dbg_draw2d)
	{
		debugView |= dv_2d;
	}
	if(states & SoundsEngine::dbg_draw3d)
	{
		debugView |= dv_3d;
	}
	if(states & SoundsEngine::dbg_drawlistener)
	{
		debugView |= dv_listener;
	}
	return debugView;
}

//Нарисовать отладочную информацию
void SoundService::DebugDraw()
{
	if(!IsDebugDraw() || !engine) return;
	long line = 0;
	//Рисуем сцены
	for(dword i = 0; i < scenes.Size(); i++)
	{
		scenes[i]->DebugDraw(states, line);
	}
	//Вывод глобальной информации
	Sound::DebugDrawGlobals(states, *this);
}

#ifdef _XBOX
void * SoundService::GetXAudio2()
{
	if(engine)
	{
		return engine->GetXAudio();
	}
	return null;
}
#endif

//Найти звук по имени
SoundBankFileSound * SoundService::FindSound(const ConstString & name, const char ** bankName)
{
#ifndef NO_TOOLS
	if(isEnablePreview)
	{
		static const char * previewBankName = "Fake preview bank name";
		if(bankName)
		{
			*bankName = previewBankName;
		}
		if(previewSounds.Size() > 0)
		{			
			for(dword i = 0; i < previewSounds.Size(); i++)
			{
				if(string::IsEqual(previewSounds[i]->name, name.c_str()))
				{
					return previewSounds[i];
				}
			}
		}
		return null;
	}
#endif
	return soundBanks->FindSound(name, bankName);
}

//Получить количество звуков ожидающих продолжение
dword SoundService::GetContinueWaiting()
{
#ifndef STOP_DEBUG
	dword count = 0;
	for(dword i = 0; i < scenes.Size(); i++)
	{
		SoundScene * scene = scenes[i];
		if(scene)
		{
			count += scene->GetCouninueQueueSize();
		}
	}
	return count;
#else
	return 0;
#endif
}

#ifndef NO_TOOLS

//Разрешить-запретить создавать звуки с временных данных
void SoundService::EditEnablePreview(bool isEnable)
{
	isEnablePreview = isEnable;
}

//Зарегистрировать звук для предпрослушивания
void SoundService::EditRegistrySound(SoundBankFileSound * sbfs)
{
	for(dword i = 0; i < previewSounds.Size(); i++)
	{
		if(previewSounds[i] == sbfs)
		{
			return;
		}
	}
	previewSounds.Add(sbfs);
}

//Отменить регистрацию звука для предпрослушивания
bool SoundService::EditUnregistrySound(SoundBankFileSound * sbfs)
{
	Assert(sbfs);
	for(dword i = 0; i < scenes.Size(); i++)
	{
		if(scenes[i]->IsUse(*sbfs))
		{
			return false;
		}
	}	
	for(dword i = 0; i < previewSounds.Size(); i++)
	{
		if(previewSounds[i] == sbfs)
		{
			previewSounds.DelIndex(i);
		}
	}
	return true;
}

//Обновить параметры звука
bool SoundService::EditPatchSound(EditPatchSoundData & data, bool isCheckIds)
{
	if(string::IsEmpty(data.soundName))
	{
		return false;
	}		
	//Получаем по имени звук
	Assert(data.nameLen < sizeof(data.soundName));
	SoundBankFileHeader * header = null;
	SoundBankFileSound * sbfSound = soundBanks->FindSound(data.soundName, data.nameHash, data.nameLen, &header);
	if(!sbfSound || sbfSound->wavesCount != data.wavesCount)
	{
		return false;
	}	
	//Таблица отладки
	Assert(header);
	if(!header->ids)
	{
		return false;
	}
	SoundBankFileObjectId * ids = header->ids;
	dword idsCount = header->idsCount;	
	if(isCheckIds)
	{
		//Сверяем идентификаторы звука
		for(dword i = 0; i < idsCount; i++)
		{
			if(ids[i].objectPtr == (byte *)sbfSound)
			{
				if(!ids[i].IsEqual(data.soundId))
				{
					return false;
				}
			}
		}
		//Сверяем волны
		for(dword i = 0; i < sbfSound->wavesCount; i++)
		{			
			if(sbfSound->waves[i].wave)
			{
				byte * wavePtr = (byte *)sbfSound->waves[i].wave;
				for(dword j = 0; j < idsCount; j++)
				{
					if(ids[i].objectPtr == wavePtr)
					{
						if(!ids[i].IsEqual(data.waves[i].waveId))
						{
							return false;
						}
					}
				}
			}else{				
				dword * wid = data.waves[i].waveId;
				if(wid[0] | wid[1] | wid[2] | wid[3])
				{
					//Идентификатор не пустой
					return false;
				}
			}
		}
	}
	//Обнавляем параметры
	dword priority = data.priority & SoundBankFileSetup::mode_priority_mask;
	Assert(priority == data.priority);
	sbfSound->setup.mode &= ~SoundBankFileSetup::mode_priority_mask;
	sbfSound->setup.mode |= priority;
	sbfSound->setup.maxCount = data.maxCount;
	sbfSound->att.c[0] = data.c[0];
	sbfSound->att.c[1] = data.c[1];
	sbfSound->att.c[2] = data.c[2];
	sbfSound->att.c[3] = data.c[3];
	Assert(data.minDist2 < data.maxDist2);
	sbfSound->att.minDist2 = data.minDist2;
	sbfSound->att.maxDist2 = data.maxDist2;
	sbfSound->att.kNorm2 = data.kNorm2;
	Assert(data.wavesCount == sbfSound->wavesCount);
	for(dword i = 0; i < data.wavesCount; i++)
	{
		sbfSound->waves[i].volume = data.waves[i].volume;
		sbfSound->waves[i].probability = data.waves[i].probability;
		sbfSound->waves[i].playTime = data.waves[i].playTime;
	}
	states |= s_needUpdateVolumes;
	return true;
}

//Остановить проигрывание всех звуков
void SoundService::EditStopAllSounds(const char * sceneOvner)
{
	for(dword i = 0; i < scenes.Size(); i++)
	{
		scenes[i]->StopAllSounds(sceneOvner);
	}
}


#endif

