
#include "SoundsEngine.h"
#include "..\..\common_h\corecmds.h"

#ifdef _XBOX
#include <xmp.h>
#endif


SoundsEngine * SoundsEngine::engine = null;

//-----------------------------------------------------------------------------------------------
//Мастер сцены, применяющий к её звукам эффекты
//-----------------------------------------------------------------------------------------------

SoundsEngine::SceneMaster::SceneMaster()
{
	Assert(SoundsEngine::engine != null);
	voice = null;
	silence = null;
	flags = 0;
}

SoundsEngine::SceneMaster::~SceneMaster()
{
	//Удаляем объекты
	if(silence)
	{
		silence->DestroyVoice();
		silence = null;
	}
	if(voice)
	{
		voice->DestroyVoice();
		voice = null;
	}
}

HRESULT SoundsEngine::SceneMaster::Init()
{
	//Создать войс для применения эффектов сцены
	Assert(!voice);
	Assert(!silence);
	//Войс
	HRESULT hr;
	XAUDIO2_VOICE_SENDS sendList;
	sendList.SendCount = 0;
	sendList.pSends = null;
	hr = SoundsEngine::engine->pXAudio2->CreateSubmixVoice(&voice, 2, GetFxMixFreq(), 0, 15, &sendList, null);
	if(hr != S_OK) return hr;
	//Создаём источник тишины, чтобы эффект работал постоянно	
	XAUDIO2_SEND_DESCRIPTOR sendDesc;
	sendDesc.Flags = 0;
	sendDesc.pOutputVoice = voice;
	sendList.SendCount = 1;
	sendList.pSends = &sendDesc;
	WAVEFORMATEX format;
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = 2;
	format.nSamplesPerSec = SoundsEngine::GetFxMixFreq();
	format.wBitsPerSample = 16;	
	format.nBlockAlign = (format.nChannels*format.wBitsPerSample)/8;
	format.nAvgBytesPerSec = (format.nChannels*format.wBitsPerSample*format.nSamplesPerSec)/8;
	format.cbSize = 0;
	Assert(SoundsEngine::engine->pXAudio2);
	hr = SoundsEngine::engine->pXAudio2->CreateSourceVoice(&silence, &format, 0, XAUDIO2_DEFAULT_FREQ_RATIO, null, &sendList, null);
	if(hr != S_OK) return hr;
	Assert(silence);
	hr = silence->SubmitSourceBuffer(&SoundsEngine::engine->silenceBuffer, null);
	if(hr != S_OK) return hr;
	return hr;
}

//Использовать этот мастер сцены, если возможно
bool SoundsEngine::SceneMaster::Alloc()
{
	if(flags & flags_isActive)
	{
		return false;
	}
	flags |= flags_isActive;
	Assert(voice);
	XAUDIO2_SEND_DESCRIPTOR sendDesc;
	sendDesc.Flags = 0;
	sendDesc.pOutputVoice = SoundsEngine::engine->fxPremasteringVoice;
	XAUDIO2_VOICE_SENDS sendList;
	sendList.SendCount = 1;
	sendList.pSends = &sendDesc;
	voice->SetOutputVoices(&sendList);
	return true;
}

//Удалить
void SoundsEngine::SceneMaster::Release()
{
	if((flags & flags_isActive) == 0)
	{
		return;
	}
	//Сбрасываем параметры эффект-процессора сцены
	fxScene.Clear();
	//Остановим звук тишины
	if(silence)
	{
		silence->Stop();
	}
	//Остановим отсоединим мастер сцены
	if(voice)
	{	
		XAUDIO2_VOICE_SENDS sends;
		sends.SendCount = 0;
		sends.pSends = null;
		voice->SetEffectChain(null);
		voice->SetOutputVoices(&sends);
	}
	//Сбрасываем флажки
	flags = 0;
}

//Запустить работу мастера
void SoundsEngine::SceneMaster::Resume()
{
	if((flags & flags_isPause) == 0)
	{
		return;
	}	
	if(flags & flags_isInitFx)
	{		
		Assert(silence);
		silence->Start();
		fxScene.Reset();
	}
	flags &= ~flags_isPause;
}

//Остановить работу мастера
void SoundsEngine::SceneMaster::Stop()
{
	if(flags & flags_isPause)
	{
		return;
	}
	if(flags & flags_isInitFx)
	{
		Assert(silence);
		silence->Stop();
		fxScene.Reset();
	}
	
	flags |= flags_isPause;
}

//Установить параметры серды окружения
void SoundsEngine::SceneMaster::SetEnvironment(const FxScene::EnvParams & params)
{
	if((flags & flags_isInitFx) == 0)
	{
		//Описание эффекта
		XAUDIO2_EFFECT_DESCRIPTOR fxDesc;
		fxDesc.pEffect = (IXAPO *)&fxScene;
		fxDesc.InitialState = true;
		fxDesc.OutputChannels = 2;
		XAUDIO2_EFFECT_CHAIN fxChain;
		fxChain.EffectCount = 1;
		fxChain.pEffectDescriptors = &fxDesc;
		voice->SetEffectChain(&fxChain);
		silence->Start();
		flags |= flags_isInitFx;
	}
	fxScene.SetParams(params);
}

//-----------------------------------------------------------------------------------------------
//Источник звука
//-----------------------------------------------------------------------------------------------

SoundsEngine::SoundChannel::SoundChannel() : xWmaBuffer(_FL_, 1)
{
	voice = null;
	fxVoice = null;
	states = 0;
	lowPrtCounter = 0;
	mode = 0;
	format = 0;
	waveVolume = 0.0f;
	time = 0;
	fadeOutTime = 0.0f;
	samplesCount = 0;
	startPlayPosition = 0;
	continuePosition = 0;
	currentUniqPtr = null;	
}

SoundsEngine::SoundChannel::~SoundChannel()
{
	Assert(!voice);
	Assert(!fxVoice);
	Assert(!currentUniqPtr);
}

void SoundsEngine::SoundChannel::Release()
{
	if(voice)
	{
		voice->DestroyVoice();
		voice = null;
	}
	if(fxVoice)
	{
		fxVoice->DestroyVoice();
		fxVoice = null;
	}
	currentUniqPtr = null;
}

//Запустить источник звука на проигрывание
void SoundsEngine::SoundChannel::Play()
{
	if(!(states & state_isPlay))
	{
		Assert(SoundsEngine::engine);
		bool isNeedPause = (SoundsEngine::engine->isPause != 0);
		Assert(voice);
		XAUDIO2_VOICE_SENDS sendList;
		sendList.SendCount = 1;
		XAUDIO2_SEND_DESCRIPTOR sendDesc;
		sendDesc.Flags = 0;
		sendDesc.pOutputVoice = fxVoice;
		sendList.pSends = &sendDesc;
		voice->SetOutputVoices(&sendList);
		if((states & state_isPause) == 0 && !isNeedPause)
		{
			voice->Start(0);
		}
		lowPrtCounter = 0;
		states &= ~state_fadeoutProcess;
		states |= state_isPlay;		
		if(isNeedPause)
		{
			Pause(true);
		}
	}
}

//Приостоновить буффер не освобождая канала
void SoundsEngine::SoundChannel::Pause(bool isPause)
{
	if(((states & state_isPause) != 0) == isPause)
	{
		return;
	}
	if(isPause)
	{
		states |= state_isPause;
		if((states & state_isPlay) != 0 && voice)
		{
			voice->Stop();
		}
	}else{
		states &= ~state_isPause;
		if((states & state_isPlay) != 0 && voice)
		{
			voice->Start(0);
		}
	}
}


//Плавно увеличить громкость с текущего значения до максимума
void SoundsEngine::SoundChannel::FadeIn(float time)
{
	if(!(states & state_isPlay))
	{
		Assert(voice);
		//Сколько надо шагов
		float steps = time*SoundsEngine::GetFxMixFreq();
		if(steps < 16)
		{
			//Очень мало шагов, сразу проигрываем
			Play();
			return;
		}
		//Шаг с которым надо увеличивать громкость с 0 до 1
		float fadeDelta = 1.0f/steps;
		fxProcessor.FadeIn(fadeDelta);
		Play();
	}
}

//Плавно уменьшить громкость с текущего значения до нуля
void SoundsEngine::SoundChannel::FadeOut(float time)
{
	if((states & state_isPlay) && voice)
	{
		states |= state_fadeoutProcess;		
		//Сколько надо шагов
		float steps = time*SoundsEngine::GetFxMixFreq();
		if(steps < 16)
		{
			//Очень мало шагов, просто останавливаем
			Stop();
			return;
		}
		//Шаг с которым надо уменьшать громкость с 1 до 0
		float fadeDelta = -1.0f/steps;
		fxProcessor.FadeOut(fadeDelta);
		fadeOutTime = time;
	}
}

//Установить позицию в 3D (включает позиционирование звука)
void SoundsEngine::SoundChannel::SetLocators(const FxVoice::Locator * locs, dword count)
{
	fxProcessor.SetLocators(locs, count);
}

//Установить громкость канала
void SoundsEngine::SoundChannel::SetVolume(float volume)
{
	fxProcessor.SetVolume(waveVolume*volume);
}

//Установить режим низчайшего приоритета, при котором можно сразу вытеснять канал
void SoundsEngine::SoundChannel::SetLowPriority(bool isLowPriority)
{
	if(!isLowPriority)
	{
		lowPrtCounter = 0;
	}else{
		lowPrtCounter++;
		if(lowPrtCounter > 0x0fffffff)
		{
			lowPrtCounter = 0x0fffffff;
		}
	}
}

//Установить громкость волны
void SoundsEngine::SoundChannel::FixWaveVolume(float volume)
{
	waveVolume = volume;
}

//Получить текущую громкость канала
float SoundsEngine::SoundChannel::DebugGetCurrentVolume()
{
	return fxProcessor.GetCurrentVolume();
}

//Активен ли канал для данного владельца
bool SoundsEngine::SoundChannel::IsLost(void * uniqPtr)
{
	return (currentUniqPtr != uniqPtr);
}

//На паузе ли буфер
bool SoundsEngine::SoundChannel::IsPause()
{
	return (states & state_isPause) != 0;
}

//Играет ли звук
bool SoundsEngine::SoundChannel::IsPlay()
{
	if(states & state_isPlay)
	{
		if(voice)
		{
			XAUDIO2_VOICE_STATE vstate;
			voice->GetState(&vstate);
			if(vstate.BuffersQueued > (states & state_buffersForStopMask))
			{
				if(!(states & state_fadeoutProcess))
				{
					return true;
				}else{
					float vol = fxProcessor.GetFadeVolume();
					if(vol > (0.1f/65536.0f))
					{
						return true;
					}					
				}				
			}
		}
		Stop();
	}
	return false;
}


//Получить позицию проигрывания в сэмплах
bool SoundsEngine::SoundChannel::GetPlayPosition(dword & playPosition)
{
	if(states & state_isPlay)
	{
		if(voice)
		{
			XAUDIO2_VOICE_STATE vstate;
			voice->GetState(&vstate);
			if(vstate.BuffersQueued)
			{
				//Позиция проигрывания в текущем буфере
				UINT64 currentPlayPos = vstate.SamplesPlayed + continuePosition - startPlayPosition;
				if((mode & SoundBankFileSetup::mode_loop_mask) != SoundBankFileSetup::mode_loop_diasble)
				{
					Assert(samplesCount > 0);
					currentPlayPos %= (UINT64)samplesCount;
				}
				if(currentPlayPos > 0xffffffff)
				{
					currentPlayPos = 0xffffffff;
				}
				playPosition = (dword)currentPlayPos;
				return true;
			}
		}
		Stop();
	}
	return false;
}

//Остановить канал
void SoundsEngine::SoundChannel::Stop()
{
	if(states & state_isPlay)
	{
		if(voice)
		{
			XAUDIO2_VOICE_SENDS sends;
			sends.SendCount = 0;
			sends.pSends = null;
			voice->SetOutputVoices(&sends);
			fxVoice->SetOutputVoices(&sends);
			fxProcessor.Reset();
			samplesCount = 0;
			startPlayPosition = 0;
			voice->DestroyVoice();
			voice = null;
		}
		states &= ~(state_isPlay | state_fadeoutProcess | state_isPause);
		lowPrtCounter = 0;
	}
}

//Получить логическое время
long SoundsEngine::SoundChannel::GetTime()
{
	if(states & state_fadeoutProcess)
	{
		//Если идёт процесс фейда, то возвращаем эмпирическое значение, зависящее от текущей громкости
		float vol = fxProcessor.GetCurrentVolume();
		vol = 1.0f - Clampf(vol);
		return long(vol*-1000000.0f);
	}
	return time;
}


//-----------------------------------------------------------------------------------------------
//SoundsEngine
//-----------------------------------------------------------------------------------------------

SoundsEngine::SoundsEngine() : sceneMasters(_FL_, 256)
{
	Assert(!engine);
	engine = this;
	//Системное
	pXAudio2 = null;
	masteringVoice = null;
	fxPremasteringVoice = null;
	musicPremasteringVoice = null;
	FxVoice::StaticInit(GetFxMixFreq());
	isPause = 0;
	notificationListenerForXbox = null;
	//Буфер тишины
	memset(silenceLoop, 0, sizeof(silenceLoop));
	memset(&silenceBuffer, 0, sizeof(silenceBuffer));
	silenceBuffer.Flags = XAUDIO2_END_OF_STREAM;
	silenceBuffer.AudioBytes = sizeof(silenceBuffer);
	silenceBuffer.pAudioData = (BYTE *)silenceLoop;
	silenceBuffer.LoopCount = XAUDIO2_LOOP_INFINITE;
	//Звуки	
	dword c = 0;
	for(dword i = 0; i < c_soundChannels; i++)
	{
		sound[i] = &soundChannels[c++];
	}
	soundCount = 0;
	for(dword i = 0; i < c_reserveChannels; i++)
	{
		reserved[i] = &soundChannels[c++];
	}
	reservedCount = 0;
	Assert(c == c_totalSoundChannels);
	Assert(ARRSIZE(soundChannels) == c_totalSoundChannels);
	creationCounter = 0;
#ifndef _XBOX
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
#endif
#ifndef NO_TOOLS
	previewMaster = null;
	previewMasterVolume = 1.0f;
	previewVoice = null;
	previewVoiceMirror = null;
	previewUserId[0] = previewUserId[1] = previewUserId[2] = previewUserId[3] = 0;
#endif
}

SoundsEngine::~SoundsEngine()
{
	Release();
#ifndef _XBOX
	CoUninitialize();
#endif
	Assert(engine == this);
	engine = null;
}

//Инициализировать
bool SoundsEngine::Init()
{
	HRESULT hr;
	dword flags = 0;
	XAUDIO2_PROCESSOR processor = XAUDIO2_DEFAULT_PROCESSOR;
#ifdef _DEBUG
	//flags = XAUDIO2_DEBUG_ENGINE;
#endif
#ifdef _XBOX
	processor = XboxThread4;
	notificationListenerForXbox = XNotifyCreateListener(XNOTIFY_XMP);
	if(notificationListenerForXbox == null)
	{
		api->Trace("SoundService error: Can't create XNotifyCreateListener object!");
	}
#endif
	if(FAILED(hr = XAudio2Create(&pXAudio2, flags, processor)))
	{
		api->Trace("SoundService error: Can't create XAudio2 object, error code: 0x%x", hr);
		return false;
	}
	//Мастер
	if(FAILED(hr = pXAudio2->CreateMasteringVoice(&masteringVoice)))
	{
		api->Trace("SoundService error: Can't create mastering voice object, error code: 0x%x", hr);
		return false;
	}
	Assert(masteringVoice);
	XAUDIO2_VOICE_SENDS sendToMaster;
	sendToMaster.SendCount = 1;
	XAUDIO2_SEND_DESCRIPTOR sendDesc;
	sendDesc.Flags = 0;
	sendDesc.pOutputVoice = masteringVoice;
	sendToMaster.pSends = &sendDesc;	
	XAUDIO2_EFFECT_DESCRIPTOR fxDesc;
	fxDesc.pEffect = (IXAPO *)&fxPremaster;
	fxDesc.InitialState = true;
	fxDesc.OutputChannels = 2;
	XAUDIO2_EFFECT_CHAIN fxChain;
	fxChain.EffectCount = 1;
	fxChain.pEffectDescriptors = &fxDesc;
	//Премастер канала эффектов
	if(FAILED(hr = pXAudio2->CreateSubmixVoice(&fxPremasteringVoice, 2, GetFxMixFreq(), 0, 20, &sendToMaster, &fxChain)))
	{
		api->Trace("SoundService error: Can't create fx premastering voice object, error code: 0x%x", hr);
		return false;
	}
	Assert(fxPremasteringVoice);

	api->ExecuteCoreCommand(CoreCommand_CheckMemory());




//	float output4to2[2][4] = {{1.0f, 1.0f, 1.0f, 1.0f},{1.0f, 1.0f, 1.0f, 1.0f}};
//	hr = fxPremasteringVoice->SetOutputMatrix(masteringVoice, 4, 2, (float *)output4to2);


	//Премастер музыкального канала
	if(FAILED(hr = pXAudio2->CreateSubmixVoice(&musicPremasteringVoice, 2, GetMusMixFreq(), 0, 20, &sendToMaster)))
	{
		api->Trace("SoundService error: Can't create music premastering voice object, error code: 0x%x", hr);
		return false;
	}
	Assert(musicPremasteringVoice);
	//Инициализация каналов
	for(dword i = 0; i < ARRSIZE(soundChannels); i++)
	{		
		SoundChannel & channel = soundChannels[i];
		XAUDIO2_EFFECT_DESCRIPTOR fxDesc;
		fxDesc.pEffect = (IXAPO *)&channel.fxProcessor;
		fxDesc.InitialState = true;
		fxDesc.OutputChannels = 2;
		XAUDIO2_EFFECT_CHAIN fxChain;
		fxChain.EffectCount = 1;
		fxChain.pEffectDescriptors = &fxDesc;
		HRESULT hr = pXAudio2->CreateSubmixVoice(&channel.fxVoice, 2, GetFxMixFreq(), 0, 10, null, &fxChain);
		if(hr != S_OK)
		{
			api->Trace("SoundsEngine::Init() -> CreateSubmixVoice: result failed: hr = 0x%x", hr);
			return false;
		}
		Assert(channel.fxVoice);		
	}
	//Запускаем звуковую библиотеку
	pXAudio2->StartEngine();
	UpdateMusicState();
	return true;
}


//Освободить ресурсы и перейди в неинициализированное состояние
void SoundsEngine::Release()
{
	for(dword i = 0; i < ARRSIZE(soundChannels); i++)
	{
		soundChannels[i].Release();
	}
	for(dword i = 0; i < sceneMasters.Size(); i++)
	{
		sceneMasters[i]->Release();
	}
	Sleep(100);
#ifndef NO_TOOLS	
	if(previewVoice)
	{
		previewVoice->DestroyVoice();
		previewVoice = null;
	}
	if(previewVoiceMirror)
	{
		previewVoiceMirror->DestroyVoice();
		previewVoiceMirror = null;
	}
	if(previewMaster)
	{
		previewMaster->DestroyVoice();
		previewMaster = null;
	}
#endif
	for(dword i = 0; i < sceneMasters.Size(); i++)
	{
		delete sceneMasters[i];
		sceneMasters[i] = null;
	}
	sceneMasters.Empty();	
	if(musicPremasteringVoice)
	{
		musicPremasteringVoice->DestroyVoice();
		musicPremasteringVoice = null;
	}
	if(fxPremasteringVoice)
	{
		fxPremasteringVoice->DestroyVoice();
		fxPremasteringVoice = null;
	}
	if(masteringVoice)
	{
		masteringVoice->DestroyVoice();
		masteringVoice = null;
	}
	if(pXAudio2)
	{
		pXAudio2->StopEngine();
		Sleep(100);
		pXAudio2->Release();		
		pXAudio2 = null;
	}
	dword c = 0;
	for(dword i = 0; i < c_soundChannels; i++)
	{
		sound[i] = &soundChannels[c++];
	}
	soundCount = 0;
	for(dword i = 0; i < c_reserveChannels; i++)
	{
		reserved[i] = &soundChannels[c++];
	}
	reservedCount = 0;
	creationCounter = 0;
#ifdef _XBOX
	if(notificationListenerForXbox != null)
	{
		XCloseHandle(notificationListenerForXbox);
	}
#endif
}

//Обновить состояние
void SoundsEngine::Update()
{
	//Глушим доигрываемые каналы
	for(dword i = 0; i < reservedCount; i++)
	{
		if(!reserved[i]->IsPlay())
		{
			reserved[i]->Stop();
			ExchangeChannels(reserved[i], reserved[reservedCount - 1]);
			reservedCount--;
		}
	}
	//Глушим зависшие каналы (PCM fix)
	for(dword i = 0; i < soundCount; i++)
	{
		dword pos = 0;
		if(sound[i]->GetPlayPosition(pos))
		{
			if(pos > sound[i]->samplesCount*2)
			{
				//Принудительно останавливаем (1 канал за кадр)
				ReleaseSoundChannel(sound[i], sound[i]->currentUniqPtr);
				break;
			}
		}
	}
#ifdef _XBOX
	if(notificationListenerForXbox)
	{
		//Смотрим состояние проигрывателя
		DWORD msgFilter;
		ULONG_PTR param;		
		if(XNotifyGetNext(notificationListenerForXbox, 0, &msgFilter, &param))
		{			
			if(msgFilter == XN_XMP_STATECHANGED)
			{
				UpdateMusicState();
			}
		}
	}
#endif
}

//Приостановить проигрывание звуков
void SoundsEngine::SetPause(bool isSetPause)
{
//	api->Trace("SoundsEngine> Set pause: %s, current flag is %i", isSetPause ? "true" : "false", isSetPause);

	if((isPause != 0) == isSetPause)
	{
		return;
	}
	isSetPause ? 1 : 0;
	//Глушим доигрываемые каналы
	if(isSetPause)
	{
//		api->Trace("SoundsEngine> Set to pause");
		//Отмечаем, что на паузе
		isPause = 1;
		//Останавливаем хвосты
		for(dword i = 0; i < reservedCount; i++)
		{
			reserved[i]->Stop();
		}
		reservedCount = 0;
		//Останавливаем звуки
		for(dword i = 0; i < soundCount; i++)
		{
			sound[i]->Pause(true);
		}
		//Останавливаем мастера сцен
		for(dword i = 0; i < sceneMasters.Size(); i++)
		{
			sceneMasters[i]->Stop();
		}		
	}else{
//		api->Trace("SoundsEngine> Resume from pause");
		//Отмечаем, что работаем
		isPause = 0;
		//Запускаем мастера сцен
		for(dword i = 0; i < sceneMasters.Size(); i++)
		{
			sceneMasters[i]->Resume();
		}
		//Востанавливаем звуки
		for(dword i = 0; i < soundCount; i++)
		{
			sound[i]->Pause(false);
		}
	}
}

//Создать мастер сцены
SoundsEngine::SceneMaster * SoundsEngine::CreateSceneMaster()
{
	//Пробуем выделить среди созданных
	for(dword i = 0; i < sceneMasters.Size(); i++)
	{
		if(sceneMasters[i]->Alloc())
		{
			return sceneMasters[i];
		}
	}
	//Добавляем объект эффекта окружения
	SceneMaster * sm = NEW SceneMaster();
	HRESULT hr = sm->Init();
	if(hr == S_OK)
	{		
		Verify(sm->Alloc());
		sceneMasters.Add(sm);
		return sm;
	}
	delete sm;
	return null;
}


//Попытаться получить доступный звуковой канал
SoundsEngine::SoundChannel * SoundsEngine::GetSoundChannel(SoundBankFileSound & sbfs, SoundBankFileWaveInfo * selWave, dword startPosition, SceneMaster & scene, void * uniqPtr)
{
	//Получаем волну, которую будем проигрывать
	Assert(selWave);
	SoundBankFileWave * wave = selWave->wave;
	if(!wave)
	{
		//Проигрывание тишины
		return null;
	}
	//Контроль логического времени создания
	if(creationCounter > 0xfffffff)
	{
		for(dword i = 0; i < c_totalSoundChannels; i++)
		{
			soundChannels[i].time -= 0xff00000;
			if(soundChannels[i].time < 0)
			{
				soundChannels[i].time = 0;
			}
		}
	}
	//Если всё занято, пробуем освободить канал
	if(soundCount >= c_soundChannels)
	{
		//Надо освободить какой нибудь, поскольку свободных уже нет
		dword soundPriority = sbfs.setup.mode & SoundBankFileSetup::mode_priority_mask;
		long bestIndex = ChannelSelector(sound, soundCount, soundPriority, creationCounter);
		if(bestIndex < 0)
		{			
			return null;	//С текущим или более низким приоритетом каналов нет, поэтому ничего не можем сделать
		}
		//Освобождаем 1 канал в конце массива
		sound[bestIndex]->currentUniqPtr = null;
		soundCount = c_soundChannels - 1;
		ExchangeChannels(sound[bestIndex], sound[c_soundChannels - 1]);
		//Теперь решаем, отправить выбраный на фейд или убить сразу
		if(reservedCount < c_reserveChannels)
		{
			bestIndex = reservedCount++;
		}else{
			//Поступаем аналогично с доигрываемыми звуками
			bestIndex = ChannelSelector(reserved, reservedCount, soundPriority, sound[c_soundChannels - 1]->GetTime());
		}
		if(bestIndex >= 0)
		{						
			//Есть резервные каналы, оставляем звук фейдиться
			sound[c_soundChannels - 1]->FadeOut(c_replaceFadeTimeInMs*0.001f);
			ExchangeChannels(sound[c_soundChannels - 1], reserved[bestIndex]);
		}
	}
	//Формат волны
	dword waveFormat = wave->format;
	if((sbfs.setup.mode & SoundBankFileSetup::mode_fx_mask) == SoundBankFileSetup::mode_fx_music)
	{
		waveFormat |= SoundBankFileWave::f_tmp_music;
	}
	//Смотрим канал с подходящим форматом
	Assert(soundCount < c_soundChannels);


#ifndef STOP_DEBUG
	/*
	//!!! Проверка, что свободные каналы не играют
	for(dword i = soundCount; i < c_soundChannels; i++)
	{
		if(sound[i]->voice)
		{			
			XAUDIO2_VOICE_STATE vstate;
			sound[i]->voice->GetState(&vstate);
			Assert(vstate.BuffersQueued == 0);
		}
	}
	//*/
#endif
/*
	for(dword i = soundCount; i < c_soundChannels; i++)
	{
		if(sound[i]->format == waveFormat)
		{
			//Есть нужный формат, поместим его в конец активных
			ExchangeChannels(sound[soundCount], sound[i]);
			break;
		}
	}
*/
	SoundChannel * channel = sound[soundCount];
	//Гарантированно останавливаем канал
	channel->Stop();
	channel->states = 0;
	//Подготавливаем канал к использованию
	XAUDIO2_VOICE_SENDS sendList;
	sendList.SendCount = 1;
	XAUDIO2_SEND_DESCRIPTOR sendDesc;
	sendDesc.Flags = 0;
	sendDesc.pOutputVoice = null;
	sendList.pSends = &sendDesc;
	//if(channel->format != waveFormat/* || ((channel->mode & SoundBankFileSetup::mode_loop_mask) != SoundBankFileSetup::mode_loop_diasble)*/)
	{
		//Необходимо пересоздать голос под нужный формат
		HRESULT hr;
		IXAudio2SourceVoice * sourceVoice = null;
		sendDesc.pOutputVoice = channel->fxVoice;
		dword workFreq = GetFxMixFreq();
		dword flags = XAUDIO2_VOICE_NOPITCH;
		if(workFreq == (waveFormat & SoundBankFileWave::f_freq_mask))
		{
				flags |= XAUDIO2_VOICE_NOSRC;
		}
		/*if(sbfs.setup.GetFxMode() == SoundBankFileSetup::mode_fx_music)
		{
				flags |= XAUDIO2_VOICE_MUSIC;
		}*/
		hr = pXAudio2->CreateSourceVoice(&sourceVoice, (const WAVEFORMATEX *)wave->waveFormatInfo, 0, XAUDIO2_DEFAULT_FREQ_RATIO, null, &sendList, null);		
		if(hr != S_OK)
		{
			//Фига се, чегото не создалось... пока проскипаем
			api->Trace("SoundsEngine::GetSoundChannel -> CreateSourceVoice return error code 0x%x", hr);
			Assert(sourceVoice);
			return null;
		}
		Assert(sourceVoice);
		if(channel->voice)
		{
			channel->voice->DestroyVoice();
		}
		channel->voice = sourceVoice;
		channel->format = waveFormat;
	}/*else{
		channel->voice->Stop();
	}*/
	soundCount++;
	//Определяем куда подключать звук
	switch(sbfs.setup.GetFxMode())
	{
	case SoundBankFileSetup::mode_fx_full:
		Assert(scene.voice);
		sendDesc.pOutputVoice = scene.voice;
		channel->fxProcessor.EnableEnvironment(true);
		break;
	case SoundBankFileSetup::mode_fx_premaster:
		sendDesc.pOutputVoice = fxPremasteringVoice;
		channel->fxProcessor.EnableEnvironment(false);
		break;
	case SoundBankFileSetup::mode_fx_master:
		sendDesc.pOutputVoice = masteringVoice;
		channel->fxProcessor.EnableEnvironment(false);
		break;
	case SoundBankFileSetup::mode_fx_music:	
		sendDesc.pOutputVoice = musicPremasteringVoice;
		channel->fxProcessor.EnableEnvironment(false);
		break;
	default:
		Assert(false);
	}
	channel->fxVoice->SetOutputVoices(&sendList);
	//Сохраняем атрибуты текущего звука
	channel->mode = sbfs.setup.mode;
	channel->currentUniqPtr = uniqPtr;
	channel->waveVolume = selWave->volume;
	channel->pickAmp = wave->maxNormalizedAmp;
	channel->tailTime = wave->unimportantTime;
	channel->fadeOutTime = 0.0f;
	channel->time = creationCounter++;	
	channel->samplesCount = wave->samplesCount;
	//Фикс для незацикленных PCM, которые выходят за пределы буфера. Начало
	UINT32 flagsForMainBuffer = XAUDIO2_END_OF_STREAM;
	bool isPCM_fix_on = false;
	if((wave->format & SoundBankFileWave::f_format_mask) == SoundBankFileWave::f_format_pcm)
	{
		isPCM_fix_on = true;
		flagsForMainBuffer = 0;
	}
	//Заполняем буффер
	bool isLoop = sbfs.setup.GetModeLoop() != SoundBankFileSetup::mode_loop_diasble;
	XAUDIO2_BUFFER xbuffer;
	xbuffer.Flags = flagsForMainBuffer;
	xbuffer.AudioBytes = wave->dataSize;
	xbuffer.pAudioData = (BYTE *)wave->data;
	xbuffer.PlayBegin = startPosition & ~0x7f;	//Для XMA надо выравнивать первый сэмпл по 128
	xbuffer.PlayLength = 0;
	
	//*
	xbuffer.PlayLength = wave->samplesCount - xbuffer.PlayBegin;
	if((waveFormat & SoundBankFileWave::f_format_mask) == SoundBankFileWave::f_format_xma)
	{
		Assert((xbuffer.PlayLength & 0x7f) == 0);
	}
	//*/

	xbuffer.LoopBegin = isLoop ? 0 : XAUDIO2_NO_LOOP_REGION;
	xbuffer.LoopLength = 0;
	xbuffer.LoopCount = isLoop ? XAUDIO2_LOOP_INFINITE : 0;
	xbuffer.pContext = null;
	//Устанавливаем буфер
	HRESULT hr = XAUDIO2_E_INVALID_CALL;
	if((waveFormat & SoundBankFileWave::f_format_mask) != SoundBankFileWave::f_format_xvma)
	{
		hr = channel->voice->SubmitSourceBuffer(&xbuffer, null);
	}else{
		//Дополнительные плюшки для xWMA
		const SoundBankFileWave::XWMAWAVEFORMAT * xwmaFormat = (const SoundBankFileWave::XWMAWAVEFORMAT *)wave->waveFormatInfo;		
		if(channel->xWmaBuffer.Size() < xwmaFormat->tableCount)
		{
			channel->xWmaBuffer.AddElements(xwmaFormat->tableCount - channel->xWmaBuffer.Size());
		}
		memcpy(channel->xWmaBuffer.GetBuffer(), xwmaFormat->table, xwmaFormat->tableCount*sizeof(dword));
		XAUDIO2_BUFFER_WMA wma;
		wma.PacketCount = xwmaFormat->tableCount;
		wma.pDecodedPacketCumulativeBytes = channel->xWmaBuffer.GetBuffer();
		hr = channel->voice->SubmitSourceBuffer(&xbuffer, &wma);
	}	
	if(hr != S_OK)
	{
		//Не получилось подписать данный буфер на проигрывание, похоже баг в формате
		ReleaseSoundChannel(channel, uniqPtr);
		api->Trace("SoundsEngine::GetSoundChannel -> SubmitSourceBuffer return error code 0x%x (for sound %s, wave: %i)", hr, sbfs.name, sbfs.waves - selWave);
		return null;
	}
	//Сохраняем стартовую позицию
	Assert(channel->samplesCount > 0);
	Assert(channel->voice);
	XAUDIO2_VOICE_STATE vstate;
	channel->voice->GetState(&vstate);
	Assert(vstate.BuffersQueued > 0);
	Assert(vstate.SamplesPlayed == 0);
	channel->startPlayPosition = vstate.SamplesPlayed;
	channel->continuePosition = xbuffer.PlayBegin;
	//Фикс для незацикленных PCM, которые выходят за пределы буфера. Окончание
	if(isPCM_fix_on)
	{
		hr = channel->voice->SubmitSourceBuffer(&silenceBuffer, null);
		if(hr != S_OK)
		{
			//Не получилось подписать данный буфер на проигрывание, похоже баг в формате
			ReleaseSoundChannel(channel, uniqPtr);
			api->Trace("SoundsEngine::GetSoundChannel -> SubmitSourceBuffer (PCM fix loop buffer) return error code 0x%x (for sound %s, wave: %i)", hr, sbfs.name, sbfs.waves - selWave);
			return null;
		}
		channel->states |= 1;
	}
	return channel;
}

//Освободить звуковой канал
void SoundsEngine::ReleaseSoundChannel(SoundChannel * ch, void * uniqPtr)
{
	if(ch->currentUniqPtr != uniqPtr) return;
	//Переводим канал к свободным
	for(dword i = 0; i < soundCount; i++)
	{
		if(sound[i] == ch)
		{			
			ch->Stop();
			if(ch->voice)
			{
				ch->voice->DestroyVoice();
				ch->voice = null;
			}
			ch->currentUniqPtr = null;
			soundCount--;
			ExchangeChannels(sound[i], sound[soundCount]);
			return;
		}
	}
	//Сюда попадать не должно
	Assert(false);
}

//Получить количество активных каналов
dword SoundsEngine::GetPlayChannels()
{
	return soundCount;
}

//Получить максимальное количество каналов
dword SoundsEngine::GetMaxChannels()
{
	return c_soundChannels;
}

//Получить количество активных каналов для доигрывания
dword SoundsEngine::GetPlayReservedChannels()
{
	return reservedCount;
}

//Получить максимальное количество каналов для доигрывания
dword SoundsEngine::GetMaxReservedChannels()
{
	return c_reserveChannels;
}


#ifdef _XBOX
//Получить объект XAudio
IXAudio2 * SoundsEngine::GetXAudio()
{
	return pXAudio2;
}
#endif


#ifndef NO_TOOLS

//Запустить волну на проигрывание 16бит (для звукового редактора)
bool SoundsEngine::EditPrewiewPlay(const dword userId[4], EditPrewiewWaveParams * mainWave, EditPrewiewWaveParams * mirrorWave, bool isMirror)
{

	//-------------------------------------------------------------------
	static IXAudio2SubmixVoice * testFxVoice = null;
	//Отладка эффектов
#ifdef _DEBUG
	if(false)
	//if(!testFxVoice)
#else
	if(false)
#endif
	{		
		XAUDIO2_VOICE_SENDS sendList;
		sendList.SendCount = 1;
		XAUDIO2_SEND_DESCRIPTOR sendDesc;
		sendDesc.Flags = 0;
		sendDesc.pOutputVoice = masteringVoice;
		sendList.pSends = &sendDesc;
		static FxPremaster fxPremaster;
		static FxScene fxScene;
		XAUDIO2_EFFECT_DESCRIPTOR fxDesc[3];
		fxDesc[0].pEffect = (IXAPO *)&fxScene;
		fxDesc[0].InitialState = true;
		fxDesc[0].OutputChannels = 2;
		XAUDIO2_EFFECT_CHAIN fxChain;
		fxChain.EffectCount = 1;
		fxChain.pEffectDescriptors = &fxDesc[0];
		HRESULT hr = pXAudio2->CreateSubmixVoice(&testFxVoice, 2, 44100, 0, 15, &sendList, &fxChain);
		
		
		sendList.SendCount = 1;
		sendDesc.Flags = 0;
		sendDesc.pOutputVoice = testFxVoice;
		WAVEFORMATEX format;
		format.wFormatTag = WAVE_FORMAT_PCM;
		format.nChannels = 2;
		format.nSamplesPerSec = 44100/8;
		format.wBitsPerSample = 16;	
		format.nBlockAlign = (format.nChannels*format.wBitsPerSample)/8;
		format.nAvgBytesPerSec = (format.nChannels*format.wBitsPerSample*format.nSamplesPerSec)/8;
		format.cbSize = 0;
		Assert(pXAudio2);
		hr = pXAudio2->CreateSourceVoice(&previewVoice, &format, 0, XAUDIO2_DEFAULT_FREQ_RATIO, null, &sendList, null);
		Assert(hr == S_OK)
		XAUDIO2_BUFFER xbuffer;
		xbuffer.Flags = XAUDIO2_END_OF_STREAM;
		static short buffer[4096];
		memset(buffer, 0, ARRSIZE(buffer));
		xbuffer.AudioBytes = 4096;
		xbuffer.pAudioData = (BYTE *)buffer;
		xbuffer.PlayBegin = 0;
		xbuffer.PlayLength = 0;
		xbuffer.LoopBegin = 0;
		xbuffer.LoopLength = 0;
		xbuffer.LoopCount = XAUDIO2_LOOP_INFINITE;
		xbuffer.pContext = null;
		hr = previewVoice->SubmitSourceBuffer(&xbuffer, null);
		previewVoice->Start();
		previewVoice = null;


	}
	//-------------------------------------------------------------------

	if(!previewMaster)
	{
		XAUDIO2_VOICE_SENDS sendToMaster;
		sendToMaster.SendCount = 1;
		XAUDIO2_SEND_DESCRIPTOR sendDesc;
		sendDesc.Flags = 0;
		sendDesc.pOutputVoice = masteringVoice;
		sendToMaster.pSends = &sendDesc;
		if(pXAudio2->CreateSubmixVoice(&previewMaster, 2, GetMusMixFreq(), 0, 25, &sendToMaster) != S_OK)
		{
			previewMaster = null;
		}
		EditPrewiewSetMasterVolume(previewMasterVolume);
	}
	//Если играло, то надо остановить проигрывание
	static dword stop_userId[4] = {0, 0, 0, 0};
	EditPrewiewStop(stop_userId);
	Assert(!previewVoice);
	Assert(!previewVoiceMirror);
	Assert((previewUserId[0] | previewUserId[1] | previewUserId[2] | previewUserId[3]) == 0);
	//Идентификатор
	Assert((userId[0] | userId[1] | userId[2] | userId[3]) != 0);
	Assert(mainWave != null);
	previewUserId[0] = userId[0];
	previewUserId[1] = userId[1];
	previewUserId[2] = userId[2];
	previewUserId[3] = userId[3];
	//Куда выводить звук
	Assert(masteringVoice);
	XAUDIO2_VOICE_SENDS sendList;
	sendList.SendCount = 1;
	XAUDIO2_SEND_DESCRIPTOR sendDesc;
	sendDesc.Flags = 0;
	if(testFxVoice)
	{
		sendDesc.pOutputVoice = testFxVoice;
	}else{
		if(!previewMaster)
		{
			sendDesc.pOutputVoice = masteringVoice;
		}else{
			sendDesc.pOutputVoice = previewMaster;
		}
	}
	sendList.pSends = &sendDesc;
	//Формат волны
	WAVEFORMATEX format;
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = mainWave->isStereo ? 2 : 1;
	format.nSamplesPerSec = mainWave->sampleRate;	
	format.wBitsPerSample = 16;	
	format.nBlockAlign = (format.nChannels*format.wBitsPerSample)/8;
	format.nAvgBytesPerSec = (format.nChannels*format.wBitsPerSample*format.nSamplesPerSec)/8;
	format.cbSize = 0;
	//Создаём войс для воспроизведения
	Assert(pXAudio2);
	HRESULT hr = pXAudio2->CreateSourceVoice(&previewVoice, &format, 0, XAUDIO2_DEFAULT_FREQ_RATIO, null, &sendList, null);
	if(hr != S_OK)
	{
		Assert(!previewVoice);
		return false;
	}
	Assert(previewVoice);
	XAUDIO2_BUFFER xbuffer;
	xbuffer.Flags = XAUDIO2_END_OF_STREAM;
	xbuffer.AudioBytes = mainWave->dataSize;
	xbuffer.pAudioData = (BYTE *)mainWave->data;
	xbuffer.PlayBegin = 0;
	xbuffer.PlayLength = 0;
	xbuffer.LoopBegin = 0;
	xbuffer.LoopLength = 0;
	xbuffer.LoopCount = 0;
	xbuffer.pContext = null;
	hr = previewVoice->SubmitSourceBuffer(&xbuffer, null);
	if(hr != S_OK)
	{
		EditPrewiewStop(stop_userId);
		return false;
	}
	if(mirrorWave)
	{
		format.nChannels = mirrorWave->isStereo ? 2 : 1;
		format.nSamplesPerSec = mirrorWave->sampleRate;
		format.nBlockAlign = (format.nChannels*format.wBitsPerSample)/8;
		format.nAvgBytesPerSec = (format.nChannels*format.wBitsPerSample*format.nSamplesPerSec)/8;
		HRESULT hr = pXAudio2->CreateSourceVoice(&previewVoiceMirror, &format, 0, XAUDIO2_DEFAULT_FREQ_RATIO, null, &sendList, null);
		if(hr != S_OK)
		{
			Assert(!previewVoiceMirror);
			EditPrewiewStop(stop_userId);
			return false;
		}
		xbuffer.AudioBytes = mirrorWave->dataSize;
		xbuffer.pAudioData = (BYTE *)mirrorWave->data;
		hr = previewVoiceMirror->SubmitSourceBuffer(&xbuffer, null);
		if(hr != S_OK)
		{
			EditPrewiewStop(stop_userId);
			return false;
		}
		if(isMirror)
		{
			previewVoice->SetVolume(0.0f);
			previewVoiceMirror->SetVolume(1.0f);
		}else{
			previewVoice->SetVolume(1.0f);
			previewVoiceMirror->SetVolume(0.0f);
		}
		previewVoice->Start();
		previewVoiceMirror->Start();
	}else{
		previewVoice->SetVolume(1.0f);
		previewVoice->Start();
	}
	return true;
}

//Переключить волну на зеркальную или оригинальную
bool SoundsEngine::EditPrewiewSwitch(const dword userId[4], bool isMirror)
{
	if(previewUserId != userId)
	{
		return false;
	}	
	if(!EditPrewiewIsPlay(userId, null))
	{
		return false;
	}
	if(!previewVoice || !previewVoiceMirror)
	{
		return false;
	}
	if(!isMirror)
	{
		previewVoice->SetVolume(1.0f);
		previewVoiceMirror->SetVolume(0.0f);
	}else{
		previewVoice->SetVolume(0.0f);
		previewVoiceMirror->SetVolume(1.0f);
	}
	return true;
}

//Играет ли волна на прослушивании (для звукового редактора)
bool SoundsEngine::EditPrewiewIsPlay(const dword userId[4], dword * samplesCount, bool * playWithMirror)
{
	static dword stop_userId[4] = {0, 0, 0, 0};
	if(previewVoice)
	{
		if((userId[0] | userId[1] | userId[2] | userId[3]) != 0)
		{
			if(((previewUserId[0] ^ userId[0]) | (previewUserId[1] ^ userId[1]) | (previewUserId[2] ^ userId[2]) | (previewUserId[3] ^ userId[3])) != 0)
			{
				return false;
			}
		}
		XAUDIO2_VOICE_STATE state;
		previewVoice->GetState(&state);
		if(state.BuffersQueued > 0)
		{
			if(samplesCount)
			{
				*samplesCount = (dword)state.SamplesPlayed;
			}
			if(playWithMirror)
			{
				*playWithMirror = (previewVoiceMirror != null);
			}
			return true;
		}
		EditPrewiewStop(stop_userId);
	}else{
		if(previewVoiceMirror)
		{
			EditPrewiewStop(stop_userId);
		}
	}
	return false;
}

//Остановить прослушивание волны
void SoundsEngine::EditPrewiewStop(const dword userId[4])
{
	if(previewVoice || previewVoiceMirror)
	{
		if((userId[0] | userId[1] | userId[2] | userId[3]) != 0)
		{
			if(((previewUserId[0] ^ userId[0]) | (previewUserId[1] ^ userId[1]) | (previewUserId[2] ^ userId[2]) | (previewUserId[3] ^ userId[3])) != 0)
			{
				return;
			}
		}
		if(previewVoice)
		{			
			previewVoice->DestroyVoice();
			previewVoice = null;
		}
		if(previewVoiceMirror)
		{
			previewVoiceMirror->DestroyVoice();
			previewVoiceMirror = null;
		}
	}
	previewUserId[0] = previewUserId[1] = previewUserId[2] = previewUserId[3] = 0;
}

//Установить громкость прослушиваемой волны
void SoundsEngine::EditPrewiewSetVolume(float volume, const dword userId[4])
{
	if(previewVoice)
	{
		if((userId[0] | userId[1] | userId[2] | userId[3]) != 0)
		{
			if(((previewUserId[0] ^ userId[0]) | (previewUserId[1] ^ userId[1]) | (previewUserId[2] ^ userId[2]) | (previewUserId[3] ^ userId[3])) != 0)
			{
				return;
			}
		}
		float setVol = Clampf(volume, 0.0f, 1.0f);
		previewVoice->SetVolume(volume);
		if(previewVoiceMirror)
		{
			previewVoiceMirror->DestroyVoice();
			previewVoiceMirror = null;
		}
	}
}

//Установить громкость мастера предпрослушивания
void SoundsEngine::EditPrewiewSetMasterVolume(float volume)
{
	previewMasterVolume = volume;
	if(previewMaster)
	{
		previewMaster->SetVolume(volume);
	}
}

#endif


//Обновить состояние музыкального канала в зависимости от состояния проигрывателя
void SoundsEngine::UpdateMusicState()
{
#ifdef _XBOX
	XMP_STATE state;
	XMPGetStatus(&state);
	if(state == XMP_STATE_IDLE)
	{
		musicPremasteringVoice->SetVolume(1.0f);
	}else{
		musicPremasteringVoice->SetVolume(0.0f);
	}
#endif
}

//Выбрать канал для замещения
long SoundsEngine::ChannelSelector(SoundChannel ** ch, long count, long priority, long time)
{
	long bestIndex = -1;
	dword bestPriority = priority;
	long bestTime = time;
	dword bestLowPrt = 0;
	bool bestIsLoop = false;
	bool bestIsTail = false;
	float bestTailTime = 1000000.0f;
	float bestPickVolume = 1.0f;
	for(long i = 0; i < count; i++)
	{
		SoundChannel * channel = ch[i];
		bool isLoop = (channel->mode & SoundBankFileSetup::mode_loop_mask) != SoundBankFileSetup::mode_loop_diasble;
		//Проверяем режим низкоприоритетной работы (3D звук за пределами радиуса)
		if(channel->lowPrtCounter > 0)
		{
			//Если переход с зацикленного звука на незацикленный, то не смотрим остальное
			if(channel->lowPrtCounter > bestLowPrt || (bestIsLoop && !isLoop))
			{
				bestIndex = i;
				bestLowPrt = channel->lowPrtCounter;
				bestIsLoop = isLoop;
				continue;
			}
			//Возможно будут ещё условия
			continue;
		}else
		if(bestLowPrt > 0)
		{
			continue;
		}
		//Пропускаем волны с большим приоритетом
		dword channelPriority = channel->mode & SoundBankFileSetup::mode_priority_mask;
		if(channelPriority > bestPriority)
		{
			continue;
		}
		//Расширеные параметры канала
		float pickVolume = channel->waveVolume*channel->pickAmp;
		float tailTime = 1000000.0f;
		bool isTail = false;
		if((channel->states & SoundChannel::state_fadeoutProcess) == 0)
		{
			if(!isLoop)
			{
				dword pos = 0;
				if(!channel->GetPlayPosition(pos)) pos = 0;
				dword tailSamples = channel->samplesCount - pos;
				float freq = (float)(channel->format & SoundBankFileWave::f_freq_mask);
				if(freq < 1.0f) freq = 1.0f;
				tailTime = (float)tailSamples/freq;
				isTail = (tailTime < channel->tailTime);
			}
		}else{
			//Если идёт процесс фейда, то возвращаем эмпирическое значение, зависящее от текущей громкости
			isLoop = false;
			float vol = Clampf(channel->fxProcessor.GetCurrentVolume());
			pickVolume *= vol;
			tailTime = vol*channel->fadeOutTime;
			
		}
		if(pickVolume < 0.00001f) pickVolume = 0.00001f;
		//Сначала проверияем приоритет
		if(bestIndex >= 0 && channelPriority == priority)
		{
			//Среди звуков с одинаковым приоритетом просматриваем остальные критерии
			if(bestIsTail == isTail)
			{
				if(bestTailTime*bestPickVolume < tailTime*pickVolume)
				{
					continue;
				}
			}else
			if(!isTail)
			{
				continue;
			}
		}
		//Звук с меньшим преоритетом всегда вытесняеться нисмотря на другие критерии
		bestIndex = i;
		bestPriority = channelPriority;
		bestTime = channel->GetTime();
		bestIsLoop = isLoop;
		bestIsTail = isTail;
		bestTailTime = tailTime;
		bestPickVolume = pickVolume;
	}
	return bestIndex;
}

//Поменять местами каналы
__forceinline void SoundsEngine::ExchangeChannels(SoundChannel * & ch1, SoundChannel * & ch2)
{
	SoundChannel * tmp = ch1;
	ch1 = ch2;
	ch2 = tmp;
}

//Получить частоту для канала эфектов
dword SoundsEngine::GetFxMixFreq()
{	
#ifdef _XBOX
	//static const dword fxSubMixFreq = dword(48000/XAUDIO2_QUANTUM_DENOMINATOR)*XAUDIO2_QUANTUM_DENOMINATOR;
	static const dword fxSubMixFreq = dword(44100/XAUDIO2_QUANTUM_DENOMINATOR)*XAUDIO2_QUANTUM_DENOMINATOR;
#else
	static const dword fxSubMixFreq = dword(44100/XAUDIO2_QUANTUM_DENOMINATOR)*XAUDIO2_QUANTUM_DENOMINATOR;
#endif
	return fxSubMixFreq;
}

//Получить частоту для канала музыки
dword SoundsEngine::GetMusMixFreq()
{
#ifdef _XBOX
	//static const dword musSubMixFreq = dword(48000/XAUDIO2_QUANTUM_DENOMINATOR)*XAUDIO2_QUANTUM_DENOMINATOR;
	static const dword musSubMixFreq = dword(44100/XAUDIO2_QUANTUM_DENOMINATOR)*XAUDIO2_QUANTUM_DENOMINATOR;
#else
	static const dword musSubMixFreq = dword(44100/XAUDIO2_QUANTUM_DENOMINATOR)*XAUDIO2_QUANTUM_DENOMINATOR;
#endif
	return musSubMixFreq;
}

