

#include "MissionMusic.h"


MissionMusic::Translate MissionMusic::actions[] =
{
	{act_play_mission, ConstString("Start mission music")},
	{act_play_movie_p, ConstString("Start movie music, pause mission")},
	{act_play_movie_s, ConstString("Start movie music, stop mission")},
	{act_pause_movie, ConstString("Pause movie music")},
	{act_continue_movie, ConstString("Continue movie music")},
	{act_play_global, ConstString("Start global music")},
	{act_stop_all, ConstString("Stop all channels")}
};


MissionMusic::MissionMusic()
{
	Assert(MusicService::ptr);
	musicName = null;
	action = act_error;
}

MissionMusic::~MissionMusic()
{
	MusicService::ptr->StopMusic(s_mission, 0.0f);
	MusicService::ptr->StopMusic(s_movie, 0.0f);
}

//Инициализировать объект
bool MissionMusic::Create(MOPReader & reader)
{
	action = act_error;
	ConstString actStr = reader.Enum();
	for(dword i = 0; i < ARRSIZE(actions); i++)
	{
		if(actStr == actions[i].str)
		{
			action = (Action)actions[i].id;
			break;
		}
	}
	Assert(action != act_error);
	musicName = reader.String().c_str();
	fadeIn = reader.Float();
	fadeOut = reader.Float();
	volume = reader.Float();
	return true;
}

//Активировать/деактивировать объект
void MissionMusic::Activate(bool isActive)
{
	if(EditMode_IsOn()) return;
	if(isActive)
	{		
		switch(action)
		{
		case act_play_mission:
			MusicService::ptr->StopMusic(s_movie, fadeOut);
			MusicService::ptr->StopMusic(s_global, fadeOut);			
			if(MusicService::ptr->PlayMusic(s_mission, musicName, volume, fadeIn, fadeOut, false))
			{
				LogicDebug("Start mission music \"%s\" with fadein time %f, fadeout time %f.", musicName, fadeIn, fadeOut);
			}else{
				LogicDebugError("Can't start play mission music: \"%s\". Sound not found!", musicName);
			}
			break;
		case act_play_movie_p:
			MusicService::ptr->PauseMusic(s_mission, true);
			MusicService::ptr->StopMusic(s_global, fadeOut);
			MusicService::ptr->PauseMusic(s_movie, false);
			if(MusicService::ptr->PlayMusic(s_movie, musicName, volume, fadeIn, fadeOut, false))
			{
				LogicDebug("Pause mission music. Start movie music \"%s\" with fadein time %f, fadeout time %f.", musicName, fadeIn, fadeOut);
			}else{
				LogicDebugError("Can't start play movie music: \"%s\". Sound not found!", musicName);
			}
			break;
		case act_play_movie_s:
			MusicService::ptr->StopMusic(s_mission, fadeOut);
			MusicService::ptr->StopMusic(s_global, fadeOut);
			MusicService::ptr->PauseMusic(s_movie, false);
			if(MusicService::ptr->PlayMusic(s_movie, musicName, volume, fadeIn, fadeOut, false))
			{
				LogicDebug("Stop mission music. Start movie music \"%s\" with fadein time %f, fadeout time %f.", musicName, fadeIn, fadeOut);
			}else{
				LogicDebugError("Can't start play movie music: \"%s\". Sound not found!", musicName);
			}
			break;
		case act_pause_movie:
			MusicService::ptr->PauseMusic(s_movie, true);
			break;
		case act_continue_movie:
			MusicService::ptr->PauseMusic(s_movie, false);
			break;
		case act_play_global:
			MusicService::ptr->StopMusic(s_mission, fadeOut);
			MusicService::ptr->StopMusic(s_movie, fadeOut);
			if(MusicService::ptr->PlayMusic(s_global, musicName, volume, fadeIn, fadeOut, true))
			{
				LogicDebug("Stop mission music. Start global music \"%s\" with fadein time %f, fadeout time %f.", musicName, fadeIn, fadeOut);
			}else{
				LogicDebugError("Can't start play global music: \"%s\". Sound not found is global sound bank!", musicName);
			}
		case act_stop_all:			
			MusicService::ptr->StopMusic(s_mission, fadeOut);
			MusicService::ptr->StopMusic(s_movie, fadeOut);
			MusicService::ptr->StopMusic(s_global, fadeOut);
			LogicDebug("Stop all music with fadeout time %f.", fadeOut);
			break;
		}
	}else{
		LogicDebug("Deactivate do nothing");
	}
}

#ifndef MIS_STOP_EDIT_FUNCS

//Инициализировать объект
bool MissionMusic::EditMode_Create(MOPReader & reader)
{
	Create(reader);
	EditMode_Select(EditMode_IsSelect());
	return true;
}

//Инициализировать объект
bool MissionMusic::EditMode_Update(MOPReader & reader)
{
	return EditMode_Create(reader);
}

//Выделить объект
void MissionMusic::EditMode_Select(bool isSelect)
{
	MissionObject::EditMode_Select(isSelect);
	if(EditMode_IsSelect())
	{		
		MusicService::ptr->PlayMusic(s_mission, musicName, volume, fadeIn, fadeOut, false);
	}else{
		MusicService::ptr->StopMusic(-1, 0.2f);
	}
}

//Перевести объект в спящий режим
void MissionMusic::EditMode_Sleep(bool isSleep)
{
	MissionObject::EditMode_Sleep(isSleep);
	MusicService::ptr->StopMusic(-1, 0.0f);
}

#endif

MOP_BEGINLISTCG(MissionMusic, "Mission music", '1.00', 0x0, "Object present multislot music system in mission", "Effects")

	MOP_ENUMBEG("Action")
		for(dword i = 0; i < ARRSIZE(MissionMusic::actions); i++)
		{
			MOP_ENUMELEMENT(MissionMusic::actions[i].str.c_str())
		}
	MOP_ENUMEND

	MOP_ENUM("Action", "Action")
	MOP_STRING("Music name", "")
	MOP_FLOATEXC("Fadein", 1.0f, 0.0f, 10.0f, "Fade time for starting music in seconds")
	MOP_FLOATEXC("Fadeout", 1.0f, 0.0f, 10.0f, "Fade time for current playing music in seconds")
	MOP_FLOATEXC("Volume", 1.0f, 0.0f, 1.0f, "Additinal volume control");
MOP_ENDLIST(MissionMusic)



CREATE_SERVICE(MusicService, 1000)

MusicService * MusicService::ptr = null;

MusicService::MusicService()
{
	Assert(!ptr);
	ptr = this;
	memset(slots, 0, sizeof(slots));
}

MusicService::~MusicService()
{
	for(dword i = 0; i < MissionMusic::max_slots_count; i++)
	{
		Slot & slot = slots[i];
		RELEASE(slot.music);
		RELEASE(slot.fadeoutPlay);
		RELEASE(slot.musicScene);
	}
	Assert(ptr == this);
	ptr = null;
}

bool MusicService::Init()
{
	ISoundService * ss = (ISoundService *)api->GetService("SoundService");
	if(!ss)
	{
		api->Storage().SetString("system.error", "MusicService not create, because SoundServece not created.");
		return false;
	}
	static const char * names[] =
	{
		"Mission music",	//0
		"Movie music",		//1
		"Global music"		//2
	};
	memset(slots, 0, sizeof(slots));
	slots[MissionMusic::s_mission].name = names[0];
	slots[MissionMusic::s_movie].name = names[1];
	slots[MissionMusic::s_global].name = names[2];
	for(dword i = 0; i < MissionMusic::max_slots_count; i++)
	{
		Assert(names[i]);
		Slot & slot = slots[i];
		slot.musicScene = ss->CreateScene(names[i], _FL_);
		Assert(slot.musicScene);
	}	
	api->SetEndFrameLevel(this, Core_DefaultExecuteLevel + 1);
	return true;
}

void MusicService::EndFrame(float dltTime)
{
	for(dword i = 0; i < MissionMusic::max_slots_count; i++)
	{
		Slot & slot = slots[i];
		if(slot.music)
		{
			if(!slot.music->IsPlay())
			{
				slot.music->Release();
				slot.music = null;
			}
		}
		if(slot.fadeoutPlay)
		{
			if(!slot.fadeoutPlay->IsPlay())
			{
				slot.fadeoutPlay->Release();
				slot.fadeoutPlay = null;
			}
		}
	}
}

//Начать играть музыку
bool MusicService::PlayMusic(long slotIndex, const char * name, float volume, float fadeInTime, float fadeOutTime, bool isCheckGlobal)
{
	//Текущий слот
	Slot & playSlot = slots[slotIndex];
	//Если текущее состояние слота в паузе, не доигрываем хвост
	if(playSlot.musicScene->IsPause())
	{
		if(playSlot.fadeoutPlay)
		{
			playSlot.fadeoutPlay->Release();
			playSlot.fadeoutPlay = null;
		}
		//Музыка не изменилась, продолжаем играть текущую
		playSlot.musicScene->Resume();
	}
	//Проверяем текущую музыку на совпадение
	if(playSlot.music)
	{
		if(string::IsEqual(name, playSlot.music->GetName()))
		{
			return true;
		}
	}	
	//Музыка меняеться, надо ставить текущую на фейд
	StopMusic(slotIndex, fadeOutTime);
	//Создаём новую музыку и начинаем проигрывать
	Assert(playSlot.musicScene);
	playSlot.music = playSlot.musicScene->Create(name, _FL_, false, false);
	if(!playSlot.music)
	{
		return false;
	}
	playSlot.music->SetVolume(volume);
	if(fadeInTime > 1e-3f)
	{
		playSlot.music->FadeIn(fadeInTime);
	}else{
		playSlot.music->Play();
	}
	return true;
}

//Пауза слота
void MusicService::PauseMusic(long slotIndex, bool isPause)
{
	//Текущий слот
	Slot & playSlot = slots[slotIndex];
	if(playSlot.musicScene->IsPause() == isPause)
	{
		//Ничего не меняеться
		return;
	}
	if(isPause)
	{
		playSlot.musicScene->Pause();
		RELEASE(playSlot.fadeoutPlay);
	}else{
		playSlot.musicScene->Resume();
	}
}

//Остановить музыку
void MusicService::StopMusic(long slotIndex, float fadeOutTime)
{
	if(slotIndex >= 0)
	{
		StopSlot(slotIndex, fadeOutTime);
	}else{
		for(dword i = 0; i < MissionMusic::max_slots_count; i++)
		{
			StopSlot(i, fadeOutTime);
		}
	}
}

void MusicService::StopSlot(long slotIndex, float fadeOutTime)
{
	//Текущий слот
	Slot & playSlot = slots[slotIndex];
	//Мгновенная остановка
	if(fadeOutTime < 1e-3f)
	{		
		if(playSlot.music)
		{
			playSlot.music->Release();
			playSlot.music = null;
		}
		if(playSlot.fadeoutPlay)
		{
			playSlot.fadeoutPlay->Release();
			playSlot.fadeoutPlay = null;
		}		
		return;
	}
	//Отправляем текущую музыку на доигрывание
	if(playSlot.music)
	{
		if(playSlot.fadeoutPlay)
		{
			playSlot.fadeoutPlay->Release();
			playSlot.fadeoutPlay = null;
		}
		playSlot.fadeoutPlay = playSlot.music;
		playSlot.fadeoutPlay->FadeOut(fadeOutTime);
		playSlot.music = null;
	}
	if(playSlot.fadeoutPlay)
	{
		playSlot.fadeoutPlay->FadeOut(fadeOutTime);
	}
}















