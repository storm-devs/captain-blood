
#include "MissionSoundBase.h"


MissionSoundBase::MissionSoundBase() : sounds(_FL_)
{
	fadeInTime = 0.0f;
	fadeOutTime = 0.0f;
}

MissionSoundBase::~MissionSoundBase()
{
	Release();
}

//Активировать
void MissionSoundBase::Activate(bool isActive)
{
//	bool old = IsActive();
//	if(isActive == old) return;
	MissionObject::Activate(isActive);
	if(isActive)
	{
		for(long i = 0; i < sounds; i++)
		{
			SoundElement & se = sounds[i];
			if(se.sound)
			{
				se.sound->Stop();
			}
			se.waitTime = se.minTime + Rnd(se.maxTime);
			se.states &= ~s_stoped;
		}
		SetUpdate(&MissionSoundBase::Work, ML_POSTEFFECTS - 10);
		LogicDebug("Activate");
	}else{
		DelUpdate(&MissionSoundBase::Work);
		for(long i = 0; i < sounds; i++)
		{
			SoundElement & se = sounds[i];
			if(!se.sound) continue;
			se.sound->FadeOut(fadeOutTime);
		}
		LogicDebug("Deactivate");
	}
}

//Пересоздать объект
void MissionSoundBase::Restart()
{	
	ReCreate();	
}

#ifndef MIS_STOP_EDIT_FUNCS

//Перевести объект в спящий режим
void MissionSoundBase::EditMode_Sleep(bool isSleep)
{
	if(isSleep)
	{
		bool curState = IsActive();
		Activate(false);
		MissionObject::Activate(curState);
	}else{
		Activate(IsActive());
	}
}

//Получить размеры описывающего ящика
void MissionSoundBase::EditMode_Select(bool isSelect)
{
	bool current = EditMode_IsSelect();
	MissionObject::EditMode_Select(isSelect);
	if(isSelect && !current)
	{
		PreviewStart();
	}else{
		Stop();
	}
}

#endif

//Работа
void _cdecl MissionSoundBase::Work(float dltTime, long level)
{
	Update(dltTime);
}

//Инициализировать объект
bool MissionSoundBase::CreateSounds(MOPReader & reader, const Vector * pos)
{
	Release();
	long counter = reader.Array();
	for(long i = 0; i < counter; i++)
	{
		long n = i;
		if(n >= sounds)
		{
			n = sounds.Add();
			sounds[n].sound = null;
		}
		SoundElement & se = sounds[n];
		ConstString name = reader.String();
		se.volume = reader.Float();
		se.minTime = reader.Float();
		se.maxTime = reader.Float();
		se.minTime = coremax(se.minTime, 0);
		se.maxTime = coremax(se.maxTime, se.minTime);
		se.waitTime = Rnd(se.maxTime);
		se.states = reader.Bool() ? s_periodical : 0;
		if(reader.Bool())
		{
			//Если звук включён, то создаём его
			if(se.sound)
			{
				if(!string::IsEqual(name.c_str(), se.sound->GetName()))
				{
					se.sound->Release();
					se.sound = null;
				}
			}
			if(!se.sound)
			{
				if(pos)
				{
					se.sound = Sound().Create3D(name, *pos, _FL_, false, false);
				}else{
					se.sound = Sound().Create(name, _FL_, false, false);
				}
			}
		}else{
			if(se.sound)
			{
				se.sound->Release();
				se.sound = null;
			}
		}
	}
	float vol = reader.Float();
	for(long i = 0; i < sounds; i++)
	{
		SoundElement & se = sounds[i];
		se.volume *= vol;
		if(se.sound)
		{
			se.sound->SetVolume(se.volume);
		}
	}
	fadeInTime = reader.Float();
	fadeOutTime = reader.Float();
	Activate(reader.Bool());
	return sounds > 0;
}

//Обновить состояние счётчиков и звуков
void MissionSoundBase::Update(float dltTime)
{
	if(EditMode_IsOn() && !EditMode_IsSelect())
	{
		return;
	}
	for(long i = 0; i < sounds; i++)
	{
		SoundElement & se = sounds[i];
		if(!se.sound) continue;
		if(se.states & s_stoped) continue;
		if(se.sound->IsPlay()) continue;
		se.waitTime -= dltTime;
		if(se.waitTime > 0.0f) continue;
		if((se.states & s_periodical) == 0 && !se.sound->IsLoop())
		{
			se.states |= s_stoped;
		}
		se.waitTime = se.minTime + Rnd(se.maxTime - se.minTime);
		se.sound->FadeIn(fadeInTime);
	}
}

//Обновить позицию
void MissionSoundBase::UpdatePosition(const Vector & pos)
{
	for(long i = 0; i < sounds; i++)
	{
		SoundElement & se = sounds[i];
		if(!se.sound) continue;
		((ISound3D *)se.sound)->SetPosition(pos);
	}	
}

//Обновить громкость
void MissionSoundBase::UpdateVolumes(float vol)
{
	for(long i = 0; i < sounds; i++)
	{
		SoundElement & se = sounds[i];
		if(!se.sound) continue;
		((ISound3D *)se.sound)->SetVolume(se.volume*vol);
	}	
}

//Запустить последовательное проигрывание звуков
void MissionSoundBase::PreviewStart()
{
	for(long i = 0; i < sounds; i++)
	{
		sounds[i].waitTime = i*1.0f;
	}
}

//Остановить проигрывание звуков
void MissionSoundBase::Stop()
{
	for(long i = 0; i < sounds; i++)
	{
		SoundElement & se = sounds[i];
		if(!se.sound) continue;
		se.sound->Stop();
	}
}

//Приостановить проигрывание звуков с фэйдом
void MissionSoundBase::Fadeout()
{
	for(long i = 0; i < sounds; i++)
	{
		SoundElement & se = sounds[i];
		if(!se.sound) continue;
		se.sound->FadeOut(fadeOutTime);
	}
}

//Удалить все звуки
void MissionSoundBase::Release()
{
	for(long i = 0; i < sounds; i++)
	{
		SoundElement & se = sounds[i];
		if(!se.sound) continue;
		se.sound->Release();
		se.sound = null;
	}
	sounds.DelAll();
}



