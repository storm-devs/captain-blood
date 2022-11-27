
#include "Sound.h"
#include "SoundScene.h"
#include "SoundService.h"
#include "SoundsEngine.h"
#include "..\..\common_h\templates\pulls.h"


SoundScene::SoundScene(SoundService & service, const char * ovnerName, const char * cppFile, long cppLine) : 
													soundService(service),
													sounds(_FL_, 256),
													soundCommands(_FL_, 256),
													soundUpdates(_FL_, 256),
													continueQueue(_FL_, 256)
{
	Assert(sizeof(FxScene::EnvParams) == sizeof(Enveronment));
	ovnerNameSaved = ovnerName;
	cppFileSaved = cppFile;
	cppLineSaved = cppLine;
	volume = 1.0f;
	volumeSounds = 1.0f;
	volumeMusic = 1.0f;
	isPause = false;
	isQueueSorted = false;
	isEvtBlend = false;
	sm = soundService.Engine().CreateSceneMaster();
	Assert(sm);
	memset(&envCurrent, 0, sizeof(envCurrent));
	envCurrent.dry = 1.0f;
	envSet = envCurrent;
	envBlendWeight = 0.0f;
	envBlendSpeed = 0.0f;
	UpdateVolumes();
	if(soundService.IsTraceCreates())
	{
		api->Trace("SoundsService debug message: SoundScene::SoundScene(%s, %s, %i)", ovnerName, cppFile, cppLine);
	}
}

SoundScene::~SoundScene()
{
	//Удаляем все звуки
	while(sounds)
	{
		sounds[sounds - 1]->UnReleaseMessage(ovnerNameSaved.c_str());
		sounds[sounds - 1]->Release();
	}
	//Удаляемся из списка сцен
	soundService.RemoveScene(this);
	//Удалить мастера сцены
	sm->Release();
	if(soundService.IsTraceCreates())
	{
		api->Trace("SoundsService debug message: SoundScene::~SoundScene(%s)", ovnerNameSaved.c_str());
	}
}


//Удалить сцену
void SoundScene::Release()
{
	delete this;
}

//Создать непозиционированный звук, если звук не создался - вернётся null
ISound * SoundScene::Create(const ConstString & soundName, const char * _cppFile, long _cppLine, bool isStartPlay, bool autoDelete, float vol)
{
	if(soundService.IsTraceCreates())
	{
		api->Trace("SoundsService debug message: SoundScene::Create(\"%s\", %s, %i, isStartPlay=%s, autoDelete=%s, %f)", soundName.c_str(), _cppFile, _cppLine, isStartPlay ? "true" : "false", autoDelete ? "true" : "false", vol);
	}
	return (ISound *)CreateSound(soundName, _cppFile, _cppLine, isStartPlay, autoDelete, null, vol);
}

//Создать позиционированный в 3D звук, если звук не создался - вернётся null
ISound3D * SoundScene::Create3D(const ConstString & soundName, const Vector & worldPos, const char * _cppFile, long _cppLine, bool isStartPlay, bool autoDelete, float vol)
{
	if(soundService.IsTraceCreates())
	{
		api->Trace("SoundsService debug message: SoundScene::Create(\"%s\", (%f, %f, %f), %s, %i, isStartPlay=%s, autoDelete=%s, %f)", soundName.c_str(), worldPos.x, worldPos.y, worldPos.z, _cppFile, _cppLine, isStartPlay ? "true" : "false", autoDelete ? "true" : "false", vol);
	}
	return (ISound3D *)CreateSound(soundName, _cppFile, _cppLine, isStartPlay, autoDelete, &worldPos, vol);
}

//Создать звук
Sound * SoundScene::CreateSound(const ConstString & soundName, const char * _cppFile, long _cppLine, bool isStartPlay, bool autoDelete, const Vector * worldPos, float vol)
{
	//Ищем звук в звуковом банке
	const char * bankName = null;
	SoundBankFileSound * sbfs = soundService.FindSound(soundName, &bankName);
	if(!sbfs)
	{
		if(soundService.IsLogOut())
		{
			api->Error("SoundService: sound \"%s\" not found!", soundName.c_str());
		}
		return null;
	}
	Assert(bankName);
	//Проверяем допустимую комбинацию флагов и режимов
	if(autoDelete)
	{
		if(!isStartPlay)
		{
			if(soundService.IsLogOut())
			{
				api->Error("SoundService error: Can't create paused sound with autoDelete flag, [%s, line %i], sound name = \"%s\" error in function ISoundScene::Create", _cppFile, _cppLine, soundName.c_str());
			}		
			return null;
		}
		if(sbfs->setup.GetModeLoop() != SoundBankFileSetup::mode_loop_diasble)
		{
			if(soundService.IsLogOut())
			{
				api->Error("SoundService error: Can't create loop sound with autoDelete flag, [%s, line %i], sound name = \"%s\" error in function ISoundScene::Create", _cppFile, _cppLine, soundName.c_str());
			}
			return null;
		}
	}
	//Создаём своё представление звука
	Sound * sound = SoundNew(*this, worldPos != null, autoDelete, *sbfs, bankName, _cppFile, _cppLine);
	sound->SetVolume(vol);
	sounds.Add(sound);
	if(worldPos)
	{
		sound->SetPosition(*worldPos);
	}	
	if(isStartPlay)
	{
		sound->Play();
	}
	return autoDelete ? null : sound;
}

//Продолжить звучание сцены
void SoundScene::Resume()
{
	if(!isPause) return;
	isPause = false;
	for(long i = 0; i < sounds; i++)
	{
		sounds[i]->SceneResume();
	}
}

//Приостановить звучание сцены
void SoundScene::Pause()
{
	if(isPause) return;
	for(long i = 0; i < sounds; i++)
	{
		sounds[i]->ScenePause();
	}
	isPause = true;
}

//Узнать текущее состояние сцены
bool SoundScene::IsPause()
{
	return isPause;
}

//Установить общую громкость звучание всей сцены
void SoundScene::SetVolume(float volume)
{
	this->volume = volume;
	UpdateVolumes();
}

//Получить общую громкость звучание всей сцены
float SoundScene::GetVolume()
{
	return volume;
}

//Установить позицию слушателя
void SoundScene::SetListenerMatrix(const Matrix & mtx)
{
	listener = mtx;
}

//Получить позицию слушателя
void SoundScene::GetListenerMatrix(Matrix & mtx)
{
	mtx = listener;
}

//Установить текущую окружающую сцену с временем перехода в секундах
void SoundScene::SetSoundEnvironmentScene(const Enveronment * envPeset, float blendTime)
{
	if(isEvtBlend)
	{
		float kSet = Clampf(envBlendWeight);
		float kCur = 1.0f - kSet;
		for(dword i = 0; i < ARRSIZE(envCurrent.val); i++)
		{
			envCurrent.val[i] = envCurrent.val[i]*kCur + envSet.val[i]*kSet;
		}
	}
	isEvtBlend = true;
	if(envPeset)
	{
		for(dword i = 0; i < ARRSIZE(envSet.val); i++)
		{
			envSet.val[i] = envPeset->val[i];
		}
	}else{
		envSet = envCurrent;
		envSet.wet = 0.0f;
		envSet.dry = 1.0f;
	}
	if(blendTime > 0.005f)
	{		
		envBlendWeight = 0.0f;
		envBlendSpeed = 1.0f/blendTime;
	}else{
		envBlendWeight = 1.0f;
		envBlendSpeed = 1000.0f;
	}
}

//Получить текущую окружающую сцену
void SoundScene::GetSoundEnvironmentScene(Enveronment & envPeset)
{
	for(dword i = 0; i < ARRSIZE(envPeset.val); i++)
	{
		envPeset.val[i] = envSet.val[i];
	}
}

//Переименовать владельца сцены
void SoundScene::ModifyOvnerName(const char * ovnerName)
{
	ovnerNameSaved = ovnerName;
}

//Нарисовать отладочную информацию
void SoundScene::DebugDraw(dword debugStates, long & line)
{	
	//Рисуем слушателя
	Sound::DebugDrawEnvironment(debugStates, line, ovnerNameSaved.c_str(), envCurrent);
	//Рисуем звуки
	for(dword i = 0; i < sounds.Size(); i++)
	{
		sounds[i]->DebugDraw(debugStates, line, ovnerNameSaved.c_str());
	}
	//Рисуем слушателя
	Sound::DebugDrawListener(debugStates, listener, ovnerNameSaved.c_str());
}

//Написать в лог сообщение об неудалённой сцене
void SoundScene::UnReleaseMessage()
{
	api->Trace("Unrelease sound scene: cpp file: %s,%i; ovner tag: %s", cppFileSaved.c_str(), cppLineSaved, ovnerNameSaved.c_str());
}

//Обновить сцену
void SoundScene::Update(float dltTime)
{	
	if(isPause) return;
	//Параметры среды окружения
	if(isEvtBlend)
	{
		FxScene::EnvParams params;
		envBlendWeight += dltTime*envBlendSpeed;		
		if(envBlendWeight < 1.0f)
		{
			float kSet = envBlendWeight;
			float kCur = 1.0f - kSet;
			for(dword i = 0; i < ARRSIZE(params.val); i++)
			{
				params.val[i] = envCurrent.val[i]*kCur + envSet.val[i]*kSet;
			}
		}else{
			isEvtBlend = false;
			envBlendWeight = 0.0f;
			envBlendSpeed = 0.0f;
			for(dword i = 0; i < ARRSIZE(params.val); i++)
			{
				params.val[i] = envCurrent.val[i] = envSet.val[i];
			}
		}
		Assert(sm);
		sm->SetEnvironment(params);
	}
	//Инверсная матрица слушателя
	inverseListener.Inverse(listener);
	//Исполняем команды
	for(dword i = 0; i < soundCommands.Size(); i++)
	{
		Sound * s = soundCommands[i];
		if(s)
		{
			s->ExecuteCommand();
		}		
	}
	soundCommands.Empty();
	//Обновляем звуки
	for(dword i = 0; i < soundUpdates.Size();)
	{
		Sound * s = soundUpdates[i];
		if(s)
		{
			//Исполняем звук
			s->Update(dltTime);
			i++;
		}else{
			//Замещаем пустой указатель последним и ужимаем массив
			if(i < soundUpdates.Size() - 1)
			{
				soundUpdates[i] = soundUpdates[soundUpdates.Size() - 1];				
			}
			soundUpdates.DelIndex(soundUpdates.Size() - 1);
		}
	}
	//Запускаем звуки на доигрывание
	if(continueQueue.Size() > 0)
	{
		//Проверяем доступное количество свободных каналов у движка
		dword count = soundService.Engine().GetMaxChannels() - soundService.Engine().GetPlayChannels();
		if(count > 0)
		{
			if(count > continueQueue.Size())
			{
				isQueueSorted = true;
				count = continueQueue.Size();
			}
			//Если на всех нет места, сортируем по приоритетам
			if(!isQueueSorted)
			{
				continueQueue.QSort(&SoundScene::CompareCntQueue);
				isQueueSorted = true;
			}
			//Пытаемся продолжить звуки
			for(dword i = 0; i < count && i < continueQueue.Size(); i++)
			{
				CntQueue & cq = continueQueue[i];
				if(cq.snd->ContinueLost())
				{
					cq.snd = null;
				}
			}
			//Удаляем отработавшие записи
			for(dword i = 0; i < continueQueue.Size(); )
			{
				if(!continueQueue[i].snd)
				{
					continueQueue.DelIndex(i);
				}else{
					i++;
				}
			}
		}
	}
}

bool SoundScene::CompareCntQueue(const CntQueue & q1, const CntQueue & q2)
{
	//Зацикленные звуки имеют больший приоритет к востановлению (бесконечное время проигрывания)
	if(q1.isLoop != q2.isLoop)
	{
		return q1.isLoop == 0;
	}
	//Приоритетные звуки востанавливаються первыми
	if(q1.priority != q2.priority)
	{
		return q2.priority > q1.priority;
	}
	//Первыми востанавливаються те, кому больше играть
	return (q1.tailTime < q2.tailTime);
}

//Обновить громкости
void SoundScene::UpdateVolumes()
{
	//Громкости групп
	volumeSounds = volume*soundService.GetCurrentSoundsVolume();
	volumeMusic = volume*soundService.GetCurrentMusicVolume();
	//Громкости активных звуков
	Sound ** s = soundUpdates.GetBuffer();
	dword count = soundUpdates.Size();
	for(dword i = 0; i < count; i++)
	{
		if(s[i])
		{
			s[i]->SetVolume(s[i]->GetVolume());
		}
	}
}

//Остановить проигрывание всех звуков
void SoundScene::StopAllSounds(const char * sceneOvner)
{
	if(sceneOvner && string::NotEqual(ovnerNameSaved.c_str(), sceneOvner))
	{
		return;
	}
	for(dword i = 0; i < sounds.Size(); i++)
	{
		sounds[i]->Stop();
		sounds[i]->ExecuteCommand();
	}
}

//Проверить на используемость
bool SoundScene::IsUse(SoundBankFileSound & sbfs)
{
	for(dword i = 0; i < sounds.Size(); i++)
	{
		if(sounds[i]->IsThis(sbfs)) return true;
	}
	return false;
}

//Путь удаляемого звукового банка
void SoundScene::CheckForDelete(const char * soundBankName)
{
#ifndef STOP_DEBUG
	bool TryDeleteSoundBankWithUsingSounds = true;
	for(dword i = 0; i < sounds.Size(); i++)
	{
		if(sounds[i]->GetSoundBankName() == soundBankName)
		{
			api->Trace("SoundService error! Sound \"%s\" not release from removed sound bank \"%s\"", sounds[i]->GetName(), soundBankName);
			TryDeleteSoundBankWithUsingSounds = false;			
		}
	}
	Assert(TryDeleteSoundBankWithUsingSounds);
#endif
}

//Поставить звук на обновление
void SoundScene::StartUpdate(Sound * snd)
{
	Sound ** s = soundUpdates.GetBuffer();
	dword count = soundUpdates.Size();
	for(dword i = 0; i < count; i++)
	{
		if(s[i] == snd) return;
	}
	soundUpdates.Add(snd);
}

//Прекратить обновлять звук
void SoundScene::StopUpdate(Sound * snd)
{
	Sound ** s = soundUpdates.GetBuffer();
	dword count = soundUpdates.Size();
	for(dword i = 0; i < count; i++)
	{
		if(s[i] == snd)
		{
			s[i] = null;
			return;
		}
	}
}

//Поставить звук в очередь на доигрывание
void SoundScene::SetToContinueQueue(Sound * snd, float tailTime, dword priority, bool isLoop)
{
	isQueueSorted = false;
	for(dword i = 0; i < continueQueue.Size(); i++)
	{
		CntQueue & cq = continueQueue[i];
		if(cq.snd == snd)
		{			
			//Изменились параметры
			cq.tailTime = tailTime;
			cq.priority = priority;
			cq.isLoop = isLoop ? 1 : 0;
			return;
		}
	}	
	//Надо добавить новую запись
	CntQueue & cnt = continueQueue[continueQueue.Add()];
	cnt.snd = snd;
	cnt.tailTime = tailTime;
	cnt.priority = priority;
	cnt.isLoop = isLoop ? 1 : 0;
}

//Удалить звук из очереди на доигрывание
void SoundScene::RemoveFromContinueQueue(Sound * snd)
{
	for(dword i = 0; i < continueQueue.Size(); i++)
	{
		if(continueQueue[i].snd == snd)
		{
			continueQueue.DelIndex(i);
			return;
		}
	}
}

//Удалить звук
void SoundScene::ReleaseSound(Sound * snd)
{
	sounds.Del(snd);
	StopUpdate(snd);
	RemoveFromContinueQueue(snd);
	for(long i = 0; i < soundCommands; i++)
	{
		if(soundCommands[i] == snd)
		{
			soundCommands[i] = null;
		}
	}
	SoundDelete(snd);
}

//Получить звуковой движёк	
SoundsEngine & SoundScene::GetSoundsEngine()
{
	return soundService.Engine();
}


namespace SoundScenePulls
{
	Pulls<sizeof(Sound)> soundPulls;
};


//Выделить из пула звук
Sound * SoundScene::SoundNew(SoundScene & scene, bool is3D, bool isAutodelete, SoundBankFileSound & sbfs, const char * bankName, const char * cppFile, long cppLine)
{
	dword code;
	void * ptr = SoundScenePulls::soundPulls.Alloc(code);
	Sound * snd = new('a', ptr) Sound(scene, is3D, isAutodelete, sbfs, bankName, cppFile, cppLine);
	snd->SetPullCode(code);
	return snd;
}

//Освободить в пуле звук
void SoundScene::SoundDelete(Sound * ptr)
{
	dword code = ptr->GetPullCode();
	ptr->~Sound();
	SoundScenePulls::soundPulls.Delete(code);
}

//Очистить все пулы
void SoundScene::ClearPulls()
{
	SoundScenePulls::soundPulls.Clear();
}

