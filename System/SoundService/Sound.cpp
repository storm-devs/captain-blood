
#include "SoundService.h"
#include "SoundsEngine.h"
#include "SoundScene.h"
#include "Sound.h"


Sound::Sound(SoundScene & scene, bool is3D, bool isAutodelete, SoundBankFileSound & snd, const char * bankName, const char * _cppFile, long _cppLine) : soundScene(scene), sbfSound(snd)
{
	cppFile = _cppFile;
	cppLine = _cppLine;
	channel = null;
	soundBankName = bankName;
	states = 0;
	if(is3D) states |= att_is3D;
	if(isAutodelete) states |= att_isAutoDelete;
	if(sbfSound.setup.GetModeLoop() != SoundBankFileSetup::mode_loop_diasble)
	{
		Assert(!isAutodelete);	//Не должно быть флага автоудаления
		states |= att_isLoop;		
	}	
	currentTime = 0.0f;
	wave = null;
	phonemes = null;
	phonemesFrame = 0;
	currentVolume = 1.0f;
	globalPos = 0.0f;
	fadeTime = 0.0f;
	playPrev = null;
	playNext = null;
	pullCode = -1;
}

Sound::~Sound()
{	
	//В списках уже не значимся, осталось только вычистить себя
	StopNow(true);
}

//Удалить звук
void Sound::Release()
{
	//Сцена удалит из своих списков и вызовет деструктор
	soundScene.ReleaseSound(this);
}

//Проиграть
void Sound::Play()
{
	//Запустим проигрывание в конце кадра
	if(!(states & s_play))
	{
		SetCommand(cmd_play);
	}	
}

//Остановить
void Sound::Stop()
{
	//Остановим в конце кадра
	if(states & s_play)
	{
		SetCommand(cmd_stop);
	}	
}

//Узнать текущее состояние
bool Sound::IsPlay()
{
	//Если играем или ждём запуска, считаем что играем
	if(states & s_play) return true;
	if((states & cmd_mask) == cmd_play) return true;
	if((states & cmd_mask) == cmd_fadein) return true;
	return false;
}

//Установить громкость звука
void Sound::SetVolume(float volume)
{
	currentVolume = volume;
	UpdateVolume();
}

//Получить громкость звука
float Sound::GetVolume()
{
	return currentVolume;
}

//Получить имя звука
const char * Sound::GetName()
{
	return sbfSound.name;
}

//Получить имя звукового банка
const char * Sound::GetSoundBankName()
{
	return soundBankName;
}

//Запустить звук проигрываться и плавно увеличить громкость за time секунд
void Sound::FadeIn(float time)
{
	//Фейдиться начнём в конце кадра
	if(!(states & s_play))
	{
		if(time > 0.001f)
		{
			fadeTime = time;
			SetCommand(cmd_fadein);
		}else{
			SetCommand(cmd_play);
		}
	}
}

//Плавно уменьшить громкость до 0 за time секунд и остановить проигрывание звука
void Sound::FadeOut(float time)
{
	//Фейдиться начнём в конце кадра
	if(states & s_play)
	{
		if(time > 0.001f)
		{
			fadeTime = time;
			SetCommand(cmd_fadeout);
		}else{
			SetCommand(cmd_stop);
		}
	}
}

//Узнать, зациклен звук или нет
bool Sound::IsLoop()
{
	return (states & att_isLoop) != 0;
}

//Вернуть идентификатор фонемы. Если отрицательный результат - фонемы нет, 0 - тишина
long Sound::GetPhonemeId(ErrorCode * ecode)
{
	//Если нет указателя на фонемы, попробуем его получить
	if(!phonemes)
	{
		phonemesFrame = 0;
		if(!wave)
		{
			if(ecode)
			{
				*ecode = ec_phonemes_sound_not_play;
			}
			return -1;
		}
		SoundBankFileWave * w = wave->wave;
		if(!w)
		{
			if(ecode)
			{
				*ecode = ec_phonemes_sound_not_play;
			}
			return -1;
		}
		for(dword i = 0; i < w->extraCount; i++)
		{
			if(w->extraData[i].id == sbf_extra_phonemes)
			{
				phonemes = (SoundBankFilePhonemes *)w->extraData[i].data;
				break;
			}
		}
		if(!phonemes)
		{
			if(ecode)
			{
				*ecode = ec_phonemes_no_data;
			}
			return -1;
		}
		if(phonemes->framesCount == 0)
		{
			phonemes = null;
			if(ecode)
			{
				*ecode = ec_phonemes_data_is_empty;
			}
			return -1;
		}
	}
	//Получаем текущее время волны
	dword timeInMs;
	/*
	if(channel && !channel->IsLost(this) && channel->GetPlayPosition(timeInMs))
	{
		dword sample = timeInMs;
		timeInMs = long(1000.0f*timeInMs/float(wave->wave->format & SoundBankFileWave::f_freq_mask));
	}else{
		timeInMs = long(currentTime*1000.0f);
	}

	
	Assert(fabsf(timeInMs*0.001f - currentTime) < 2.0f);
	*/

	timeInMs = long(currentTime*1000.0f);



	phonemesFrame = phonemes->FindFrame(timeInMs, phonemesFrame);
	dword phoneme = (long)phonemes->GetPhonemeId(phonemesFrame);
	if(ecode)
	{
		*ecode = ec_ok;
	}
	//api->Trace("^^^  GetPhonemeId() = %i", phoneme);
	return phoneme;
}


//Установить позицию источника звука
void Sound::SetPosition(const Vector & position)
{
	globalPos = position;
}

//Получить позицию источника звука
Vector Sound::GetPosition()
{
	return globalPos;
}

//Приостоновить звук
void Sound::ScenePause()
{
	//Если уже на паузе, ничего не делаем
	if(states & s_pause)
	{
		return;
	}
	//Молчащий звук тоже не играем
	if((states & s_play) == 0 || !channel || channel->IsLost(this))
	{
		return;
	}
	//Запоминаем позицию проигрывания
	if(channel->GetPlayPosition(pausePosition))
	{
		states |= s_pause;
	}
	//Освобождаем канал
	soundScene.GetSoundsEngine().ReleaseSoundChannel(channel, this);
	channel = null;
}

//Продолжить проигрывать звук
void Sound::SceneResume()
{
	if((states & s_pause) == 0)
	{
		return;
	}
	if(states & s_play)
	{
		states &= ~s_play;
		//Востанавливаем канал
		PlayNow(false);
		states &= ~s_pause;
	}
}

//Продолжить проигрывание звука с потерянным каналом
bool Sound::ContinueLost()
{	
	//Проверяем что условия запуска не изменились	
	if(!(states & s_play) || !wave || wave->playTime <= 0.0f || wave->playTime - currentTime <= c_time_to_skip_lost_ms*0.001f)
	{
		return true;
	}
	if(states & s_pause)
	{
		return false;
	}
	states |= s_pause;
	Assert(wave->wave);
	Assert(wave->wave->samplesCount > 0);
	float kPos = Clampf(currentTime/wave->playTime);
	long pos = long(kPos*wave->wave->samplesCount);
	if(pos < 0) pos = 0;
	if((dword)pos >= wave->wave->samplesCount)
	{
		return true;
	}
	pausePosition = (dword)pos;
	if(states & att_is3D)
	{
		float att = Update3DPosition(false);
		fadeTime = Clampf(att, 0.0f, c_fade_time_to_restore*0.001f);
	}else{
		fadeTime = c_fade_time_to_restore*0.001f;
	}
	states &= ~s_play;
	PlayNow(true);
	pausePosition = 0;
	states &= ~s_pause;
	return true;
}

//Исполнить команду
void Sound::ExecuteCommand()
{
	//Отложенная до конца кадра команда
	dword cmd = states & cmd_mask;
	SetCommand(cmd_none);
	switch(cmd)
	{
	case cmd_play:
		if(states & s_play)
		{
			StopNow();
		}
		PlayNow(false);
		break;
	case cmd_stop:
		StopNow();
		break;
	case cmd_fadein:
		if(states & s_play)
		{
			StopNow();
		}
		PlayNow(true);
		break;
	case cmd_fadeout:
		if(channel && !channel->IsLost(this))
		{
			channel->FadeOut(fadeTime);
		}else{
			StopNow();
		}
		break;
	}
}

//Обновить звук
void Sound::Update(float dltTime)
{
	if(!wave)
	{
		StopNow();
		return;
	}
	//Счётчик времени
	bool isLoop = (sbfSound.setup.GetModeLoop() != SoundBankFileSetup::mode_loop_diasble);
	currentTime += dltTime;
	if(!wave->wave)
	{
		//Если играем тишину, то только проверяем счётчик и занимаем очередь
		if(currentTime >= wave->playTime)
		{
			StopNow();
		}
		return;
	}
	//Зацикливаем счётчик времени
	if(isLoop)
	{
		while(currentTime >= wave->playTime)
		{
			currentTime -= wave->playTime;
		}
		if(currentTime < 0.0f) currentTime = 0.0f;
	}
	//Следим за состоянием канала
	if(channel)
	{
		if(!channel->IsLost(this))
		{
			if(channel->IsPlay())
			{				
				//Обновим позицию 3D звука
				Update3DPosition(true);
				channel->SetLowPriority((states & s_outofrange) != 0);
				//Хот фикс зависшего канала
				if(currentTime > wave->playTime + 1.0f)
				{
					if(!channel->IsPause())
					{
						channel->FadeOut(1.0f);	
					}else{
						StopNow();
					}					
				}
				return;
			}else{
				StopNow();
				return;
			}
		}else{
			channel = null;
		}
	}
	//Время на доигрывание звука
	float tailTime = wave->playTime - currentTime;
	if(tailTime <= 0.0f)
	{
		StopNow();
		return;
	}
	//Решаем что делать с отсутствующим каналом
	Update3DPosition(false);
	if((states & s_outofrange) != 0)
	{
		//3D звук за пределами радиуса, ничего не делаем
		return;
	}	
	//Смотрим сколько осталось доигрывать звуку
	
	Assert(wave->wave);
	if(!isLoop)
	{		
		if(tailTime <= c_time_to_skip_lost_ms*0.001f || tailTime <= wave->wave->unimportantTime)
		{
			//Фэйковое доигрывание без попытки востановить канал
			return;
		}
	}
	//Встаём/обновляем очередь на востановление канала
	dword priority = sbfSound.setup.GetPriority();
	Assert(!channel);
	soundScene.SetToContinueQueue(this, tailTime, priority, isLoop);	
}
//Написать в лог сообщение об неудалённом звуке
void Sound::UnReleaseMessage(const char * sceneOvner)
{
	if(!(states & att_isAutoDelete))
	{
		api->Trace("Unrelease sound: %s, (scene: \"%s\", cpp: %s, %i, sbank: %s)", sbfSound.name, sceneOvner, cppFile, cppLine, soundBankName);
	}
}

//Установить команду на исполнение
__forceinline void Sound::SetCommand(States cmd)
{
	if((states & cmd_mask) == cmd_none)
	{
		if(cmd != cmd_none)
		{
			//Если не пустая команда, то подпишемся на разовое исполнение
			soundScene.ExecuteSoundCommand(this);
		}		
	}
	//Замещаем текущую команду новой
	states = (states & ~cmd_mask) | cmd;	
}

//Запустить звук на проигрывание немедленно
void Sound::PlayNow(bool isFadeIn)
{
	Assert(!(states & s_play));
	//Канала активного быть не должно сейчас и играть не должны
	Assert(!channel);
	//Волна для проигрывания
	if((states & s_pause) == 0)
	{
		//Новая волна
		dword waveIndex = sbfSound.SelectWaveIndex();
		wave = &sbfSound.waves[waveIndex];
		phonemes = null;
		currentTime = 0.0f;
		pausePosition = 0;		
		if(!wave->wave && wave->playTime < 1e-3f)
		{
			//Просто пропускаем проигрывание
			return;
		}
		//Проверяем очередь
		if(sbfSound.setup.maxCount > 0)
		{
			if(sbfSound.playSoundsCount >= sbfSound.setup.maxCount)
			{
				//Надо завершить самый давний звук в пользу текущего
				Sound * s = (Sound *)sbfSound.playSoundsList;
				Assert(s);
				for(; s->playNext; s = s->playNext);
				s->Replase();
			}
		}
		//Добавляемся в список проигрываемых
		playPrev = null;
		playNext = (Sound *)sbfSound.playSoundsList;
		sbfSound.playSoundsList = this;
		if(playNext)
		{
			playNext->playPrev = this;
		}
	}
	Assert(wave);
	Assert(wave->playTime > 0.0f);
	//Настраиваем флажки
	states |= s_play;
	states &= ~s_outofrange;
	//Скажем сцене обновлять звук
	soundScene.StartUpdate(this);
	//Проверяем дистанцию, если звук 3D
	Update3DPosition(false);
	if(states & s_outofrange)
	{
		//Пока нет необходимости заводить канал
		return;
	}
	//Создаём канал для волны
	channel = soundScene.GetSoundsEngine().GetSoundChannel(sbfSound, wave, pausePosition, soundScene.GetSceneMaster(), this);
	if(channel)
	{
		//Канал удалось создать, работаем в нормальном режиме		
		UpdateVolume();
		Update3DPosition(true);
		if(isFadeIn)
		{
			channel->FadeIn(fadeTime);
		}else{		
			channel->Play();
		}
	}
}

//Остановить звук немедленно
void Sound::StopNow(bool noRelease)
{	
	wave = null;
	phonemes = null;
	pausePosition = 0;
	if(states & s_play)
	{
		//Исключаем из списка проигрываемых	
		if(playPrev)
		{
			playPrev->playNext = playNext;
		}else{
			sbfSound.playSoundsList = playNext;
		}
		if(playNext)
		{
			playNext->playPrev = playPrev;
		}
		//Останавливаем обновление
		soundScene.StopUpdate(this);
	}
	//Устанавливаем текущее состояние
	states &= ~s_mask;
	//Освобождаем канал, если использовали
	if(channel)
	{
		soundScene.GetSoundsEngine().ReleaseSoundChannel(channel, this);
		channel = null;
	}
	soundScene.RemoveFromContinueQueue(this);
	//Смотрим атрибут самоудаления
	if(states & att_isAutoDelete)
	{
		if(!noRelease)
		{
			Release();
		}
	}
}

//Заместить этот звук новым
void Sound::Replase()
{
	//Удалим канал с быстрым фэйдом
	if(channel)
	{
		channel->FadeOut(SoundsEngine::c_replaceFadeTimeInMs*0.001f);
		channel = null;
	}
	StopNow();
}

//Обновить громкость звука
__forceinline void Sound::UpdateVolume()
{
	if(channel)
	{
#ifndef NO_TOOLS
		if(wave)
		{
			channel->FixWaveVolume(wave->volume);
		}
#endif
		if(sbfSound.setup.GetFxMode() != SoundBankFileSetup::mode_fx_music)
		{
			channel->SetVolume(currentVolume*soundScene.GetVolumeSounds());
		}else{
			channel->SetVolume(currentVolume*soundScene.GetVolumeMusic());
		}
	}
}

//Обновить позицию звука
__forceinline float Sound::Update3DPosition(bool isUpdateChannel)
{	
	if(states & att_is3D)
	{
		//Квадрат дистанции до слушателя
		FxVoice::Locator loc;
		Vector localPos(0.0f);
		float dist2 = 0.0f;
		bool inRange = soundScene.ListenerTransfromPosition(globalPos, sbfSound.att.maxDist2, localPos, dist2);
		if(!inRange)
		{
			//Ничего не слышно, обрубаем звук
			if(isUpdateChannel)
			{
				Assert(channel);
				Assert(!channel->IsLost(this));
				channel->SetLocators(&loc, 1);
			}
			states |= s_outofrange;
			return 0.0f;
		}
		//Дистанция от слушателя до звука
		float distXZ = localPos.GetLengthXZ();
		//Прямой звук. Определяем затухание
		loc.attenuation = sbfSound.att.Attenuation(dist2);
		if(loc.attenuation < 1e-10f)
		{
			//Слишком тихо, отражения даже и не пытаемся считать
			loc.attenuation = 0.0f;
			if(isUpdateChannel)
			{
				Assert(channel);
				Assert(!channel->IsLost(this));
				channel->SetLocators(&loc, 1);
			}
			states |= s_outofrange;
			return 0.0f;
		}
		states &= ~s_outofrange;
		if(!isUpdateChannel) return loc.attenuation;
		Assert(channel);
		Assert(!channel->IsLost(this));
		//Определяем направление от слушателя до звука
		const float headRad = 1.0f;
		const float headDirThreshold = 0.35f;
		if(distXZ > headRad)
		{
			//За пределами головы направление указываем обычно
			float kNorm = 1.0f/distXZ;
			loc.dirFront = localPos.z*kNorm;
			loc.dirRight = localPos.x*kNorm;
		}else
		if(distXZ > headRad*headDirThreshold)
		{
			//Внутри головы начинаем терять направление
			float kLen = (distXZ - headDirThreshold)/(1.0f - headDirThreshold);   //1 на внешнем радиусе и 0 на внутренем
			float kNorm = kLen*1.0f/distXZ;
			loc.dirFront = localPos.z*kNorm;
			loc.dirRight = localPos.x*kNorm;
		}else{
			//С какого то порога направления нет
			loc.dirFront = 0.0f;
			loc.dirRight = 0.0f;
		}
		channel->SetLocators(&loc, 1);
		return loc.attenuation;
	}
	return 0.0f;
}

#ifndef STOP_DEBUG
#include "..\..\common_h\render.h"
#endif


//Отобразить отладочную информацию
void Sound::DebugDraw(dword debugStates, long & line, const char * sceneOvner)
{
#ifndef STOP_DEBUG
		if(states & s_play)
		{
			if(states & att_is3D)
			{
				if(debugStates & SoundsEngine::dbg_draw3d)
				{
					DebugDraw3D(debugStates & SoundsEngine::dbg_drawlevel_mask, sceneOvner);
				}
			}else{
				if(debugStates & SoundsEngine::dbg_draw2d)
				{
					DebugDraw2D(debugStates & SoundsEngine::dbg_drawlevel_mask, line, sceneOvner);
				}
			}
		}
#endif
}

//Нарисовать слушателя
void Sound::DebugDrawListener(dword debugStates, const Matrix & listener, const char * sceneOvner)
{
#ifndef STOP_DEBUG
	if(debugStates & SoundsEngine::dbg_drawlistener)
	{
		dword drawLevel = debugStates & SoundsEngine::dbg_drawlevel_mask;
		//Рендер для вывода информации
		static IRender * render = null;
		if(!render)
		{
			render = (IRender *)api->GetService("DX9Render");
			if(!render) return;
		}
		//Рисуем слушателя
		if(drawLevel == SoundsEngine::dbg_drawlevel_max)
		{
			render->DrawCapsule(0.3f, 0.8f, 0x80808080, Matrix().RotateZ(PI*0.5f)*listener);
			render->DrawMatrix(listener);
			render->Print(listener.pos, 40.0f, 0.0f, 0xffffffff, "Sound listener");
			render->Print(listener.pos, 40.0f, 1.1f, 0xffffffff, "\"%s\"", sceneOvner);
		}else{
			render->DrawMatrix(listener);
		}
	}
#endif
}

//Написать о параметрах эффекта среды окружения
void Sound::DebugDrawEnvironment(dword debugStates, long & line, const char * sceneOvner, const FxScene::EnvParams & env)
{
#ifndef STOP_DEBUG
	if(debugStates & (SoundsEngine::dbg_drawlistener | SoundsEngine::dbg_draw2d | SoundsEngine::dbg_draw3d))
	{		
		//Рендер для вывода информации
		static IRender * render = null;
		if(!render)
		{
			render = (IRender *)api->GetService("DX9Render");
			if(!render) return;
		}
		//Рисуем слушателя
		float eps = 0.001f;
		if(!((fabs(env.dry - 1.0f) < eps) && (fabs(env.wet) < eps)))
		{
			render->Print(10.0f, 30.0f + line*20.0f, 0xcfffffff, "* Env. Scene: \"%s\", params: dry=%f, wet=%f, predelay=%f, early=%f, damping=%f, dispersion=%f", 
				sceneOvner, 
				env.dry, env.wet, env.predelayTime, env.earlyTime, env.earlyAttenuation, env.damping, env.dispersion);
			line++;
		}
	}
#endif
}

//Вывести глобальные параметры
void Sound::DebugDrawGlobals(dword debugStates, SoundService & service)
{
#ifndef STOP_DEBUG
	SoundsEngine & engine = service.Engine();
	dword drawLevel = debugStates & SoundsEngine::dbg_drawlevel_mask;
	//Рендер для вывода информации
	static IRender * render = null;
	if(!render)
	{
		render = (IRender *)api->GetService("DX9Render");
		if(!render) return;
	}
	//Пишем общую статистику
	render->Print(10.0f, 10.0f, 0xcfffffff, "Play channels %3u of %3u, reserved play: %3u of %3u, continue waiting: %3u", 
					engine.GetPlayChannels(), engine.GetMaxChannels(), 
					engine.GetPlayReservedChannels(), engine.GetMaxReservedChannels(), service.GetContinueWaiting());
#endif
}


#ifndef STOP_DEBUG

//Вывести отладочную информацию для 3D звука
void Sound::DebugDraw3D(dword drawLevel, const char * sceneOvner)
{
	//Рендер для вывода информации
	static IRender * render = null;
	if(!render)
	{
		render = (IRender *)api->GetService("DX9Render");
		if(!render) return;
	}
	//Текущая реальная громкость
	char buf[64];
	if(channel)
	{
		if(!channel->IsPause())
		{
			crt_snprintf(buf, sizeof(buf) - 1, "vol = %2.3f", channel->DebugGetCurrentVolume());
		}else{
			crt_snprintf(buf, sizeof(buf) - 1, "channel on pause", channel->DebugGetCurrentVolume());
		}
	}else{
		crt_strcpy(buf, sizeof(buf) - 1, "silence mode");
	}
	//Общии параметры
	const float minDist = sqrtf(sbfSound.att.minDist2);
	const float maxDist = sqrtf(sbfSound.att.maxDist2);
	const float textViewDist = coremax(50.0f, maxDist + 10.0f);
	//Крестовина
	render->DrawLine(globalPos - Vector(minDist, 0.0f, 0.0f), 0xffff0000, globalPos + Vector(minDist, 0.0f, 0.0f), 0xffff0000);
	render->DrawLine(globalPos - Vector(minDist, 0.0f, 0.0f), 0xffff0000, globalPos - Vector(maxDist, 0.0f, 0.0f), 0xff0000ff);
	render->DrawLine(globalPos + Vector(minDist, 0.0f, 0.0f), 0xffff0000, globalPos + Vector(maxDist, 0.0f, 0.0f), 0xff0000ff);
	render->DrawLine(globalPos - Vector(0.0f, 0.0f, minDist), 0xffff0000, globalPos + Vector(0.0f, 0.0f, minDist), 0xffff0000);
	render->DrawLine(globalPos - Vector(0.0f, 0.0f, minDist), 0xffff0000, globalPos - Vector(0.0f, 0.0f, maxDist), 0xff0000ff);
	render->DrawLine(globalPos + Vector(0.0f, 0.0f, minDist), 0xffff0000, globalPos + Vector(0.0f, 0.0f, maxDist), 0xff0000ff);
	//Круги обозначающие границу в горизонтальной плоскости проходящей через позицию звука
	render->DrawXZCircle(globalPos, minDist, 0xffff0000);
	render->DrawXZCircle(globalPos, maxDist, 0xff0000ff);
	dword textColor = 0xffffffff;
	if(wave && currentTime > wave->playTime)
	{
		textColor = 0xffff2020;
	}
	if(drawLevel == SoundsEngine::dbg_drawlevel_max)
	{
		//Рисуем график затухания
		const float height = 4.0f;
		const dword color = 0xffc0ffc0;
		//Графики затухания
		for(long i = 0; i < 50; i++)
		{
			float k1 = i/50.0f;
			float k2 = (i + 1.0f)/50.0f;
			float d1 = minDist + (maxDist - minDist)*k1;
			float d2 = minDist + (maxDist - minDist)*k2;
			float y1 = sbfSound.att.Attenuation(d1*d1)*height;
			float y2 = sbfSound.att.Attenuation(d2*d2)*height;
			render->DrawLine(globalPos + Vector(0.0f, y1, d1), color, globalPos + Vector(0.0f, y2, d2), color);			
			render->DrawLine(globalPos + Vector(d1, y1, 0.0f), color, globalPos + Vector(d2, y2, 0.0f), color);
			render->DrawLine(globalPos + Vector(0.0f, y1, -d1), color, globalPos + Vector(0.0f, y2, -d2), color);
			render->DrawLine(globalPos + Vector(-d1, y1, 0.0f), color, globalPos + Vector(-d2, y2, 0.0f), color);
		}
		//Кольца показывающие гравик в пространстве
		for(long i = 0; i < 10; i++)
		{
			float k = i/10.0f;
			float d = minDist + (maxDist - minDist)*k;
			float y = sbfSound.att.Attenuation(d*d)*height;
			render->DrawXZCircle(globalPos + Vector(0.0f, y, 0.0f), d, i ? 0x1fc0ffc0 : color);
			render->DrawXZCircle(globalPos, d, 0x1f0000ff);
		}
		//Текущая проекция позиции камеры на графике затухания
		Vector camPos = render->GetView().GetCamPos();
		Vector delta = globalPos - camPos;
		float d2 = delta.GetLength2();
		if(d2 <= sbfSound.att.maxDist2)
		{
			float y = sbfSound.att.Attenuation(d2)*height;
			float d = sqrtf(d2);
			render->DrawXZCircle(globalPos + Vector(0.0f, y, 0.0f), d, 0xff20ff20);
			render->DrawXZCircle(globalPos + Vector(0.0f, 0.0f, 0.0f), d, 0xff00cf00);
		}
		//Пишем расширенную информацию
		render->Print(globalPos, textViewDist, -2.0f, textColor, "%s", GetName());
		render->Print(globalPos, textViewDist, -1.0f, textColor, "scene: \"%s\"", sceneOvner);
		render->Print(globalPos, textViewDist, 0.0f, textColor, "%s", buf);
		render->Print(globalPos, textViewDist, 1.0f, textColor, "[%.1f of %.1f sec]", currentTime, wave ? wave->playTime : 0.0f);
		render->Print(globalPos, textViewDist, 2.0f, textColor, "prty = %i", sbfSound.setup.mode & SoundBankFileSetup::mode_priority_mask);
		render->Print(globalPos, textViewDist, 3.0f, textColor, "loop = %s", (states & att_isLoop) ? "true" : "false");		
	}else{
		//Только подписываем некоторую текстовую информацию
		render->Print(globalPos, textViewDist, -1.0f, textColor, "%s", GetName());
		render->Print(globalPos, textViewDist, 0.0f, textColor, "%s", buf);
		render->Print(globalPos, textViewDist, 1.0f, textColor, "scene: \"%s\"", sceneOvner);
	}
}

//Вывести отладочную информацию для 2D звука
void Sound::DebugDraw2D(dword drawLevel, long & line, const char * sceneOvner)
{
	//Рендер для вывода информации
	static IRender * render = null;
	if(!render)
	{
		render = (IRender *)api->GetService("DX9Render");
		if(!render) return;
	}
	//Текущая реальная громкость
	char buf[64];
	if(channel)
	{
		if(!channel->IsPause())
		{
			crt_snprintf(buf, sizeof(buf) - 1, "vol = %2.3f", channel->DebugGetCurrentVolume());
		}else{
			crt_snprintf(buf, sizeof(buf) - 1, "channel on pause", channel->DebugGetCurrentVolume());
		}
	}else{
		crt_strcpy(buf, sizeof(buf) - 1, "silence mode");
	}
	//Вывод информации в зависимости от уровня
	dword textColor = 0xcfffffff;
	if(wave && currentTime > wave->playTime)
	{
		textColor = 0xcfff2020;
	}
	if(drawLevel == SoundsEngine::dbg_drawlevel_max)
	{
		render->Print(10.0f, 30.0f + line*20.0f, textColor, "S2d: %s, scene: \"%s\", [%.1f of %.1f sec], %s, prty = %i, loop = %s", GetName(), sceneOvner, currentTime, wave ? wave->playTime : 0.0f, buf, sbfSound.setup.mode & SoundBankFileSetup::mode_priority_mask, (states & att_isLoop) ? "true" : "false");
	}else{
		render->Print(10.0f, 30.0f + line*20.0f, textColor, "S2d: %s, scene: \"%s\", %s", GetName(), sceneOvner, buf);
	}
	line++;	
}


#endif