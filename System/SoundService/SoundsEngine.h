
#ifndef _SoundsEngine_h_
#define _SoundsEngine_h_

#include "ISoundEditorAccessor.h"
#include <Xaudio2.h>
#include "FxPremaster.h"
#include "FxScene.h"
#include "FxVoice.h"


class SoundsEngine
{
public:
	//Константы
	enum Consts
	{
		//Максимально допустимое количество:		
		c_soundChannels = 64,		//звуковых каналов
		c_reserveChannels = 32,		//каналов, доигрывающих звук с фейдом после замены
		c_totalSoundChannels = c_soundChannels + c_reserveChannels,
		//Время фейда в милисекундах при принудительной смене каналов
		c_replaceFadeTimeInMs = 800,
	};

	//Режимы вывода отладочной информации
	enum DebugStates
	{
		dbg_drawlevel_min = 0x1,
		dbg_drawlevel_max = 0x2,
		dbg_drawlevel_mask = 0xf,
		dbg_draw3d = 0x100,
		dbg_draw2d = 0x200,
		dbg_drawlistener = 0x400,
		dbg_logout = 0x10000,		
		dbg_snd_off = 0x20000,
		dbg_tracecreates = 0x40000,
		dbg_mask = 0xfffff,
	};


public:

	//Мастер сцены, применяющий к её звукам эффекты
	class SceneMaster
	{
		friend class SoundsEngine;

		enum Consts
		{
			flags_isActive = 1,
			flags_isInitFx = 2,
			flags_isPause = 4,
		};


	protected:
		SceneMaster();
		~SceneMaster();
		HRESULT Init();
		
	public:
		//Использовать этот мастер сцены, если возможно
		bool Alloc();
		//Удалить
		void Release();


		//Запустить работу мастера
		void Resume();
		//Остановить работу мастера
		void Stop();

		//Установить параметры серды окружения
		void SetEnvironment(const FxScene::EnvParams & params);

	private:
		IXAudio2SubmixVoice * voice;
		IXAudio2SourceVoice * silence;
		FxScene fxScene;
		dword flags;
	};

	friend class SceneMaster;
	class SoundChannel;

	//Звуковой канал
	class SoundChannel
	{
		friend class SoundsEngine;
	protected:
		enum Consts
		{
			state_isPlay = 0x100,				//Играеться канал
			state_fadeoutProcess = 0x200,		//Фэйд канала перед остановкой
			state_isPause = 0x400,				//Пауза канала
			state_buffersForStopMask = 0xff,	//Если в очереди осталось столько буферов, то считаем что канал не играет
		};

	protected:
		SoundChannel();
		~SoundChannel();
		void Release();

	//Управление каналом
	public:
		//Запустить источник звука на проигрывание
		void Play();
		//Приостоновить буффер не освобождая канала
		void Pause(bool isPause);
		//Плавно увеличить громкость с текущего значения до максимума
		void FadeIn(float time);
		//Плавно уменьшить громкость с текущего значения до нуля
		void FadeOut(float time);
		//Установить позицию в 3D (включает позиционирование звука)
		void SetLocators(const FxVoice::Locator * locs, dword count);
		//Установить громкость канала
		void SetVolume(float volume);
		//Установить режим низчайшего приоритета, при котором можно сразу вытеснять канал
		void SetLowPriority(bool isLowPriority);
		//Изменить текущую громкость волны (редактор)
		void FixWaveVolume(float volume);

	//Состояния канала
	public:
		//Активен ли канал для данного владельца
		bool IsLost(void * uniqPtr);
		//На паузе ли буфер
		bool IsPause();
		//Играет ли звук
		bool IsPlay();
		//Получить позицию проигрывания в сэмплах
		bool GetPlayPosition(dword & playPosition);
		//Получить текущую громкость канала
		float DebugGetCurrentVolume();


	private:
		//Остановить канал
		void Stop();
		//Получить логическое время
		long GetTime();

		float GetTailTime();

	private:
		//Интерфейсы, с которыми взаимодействуем
		IXAudio2SourceVoice * voice;
		IXAudio2SubmixVoice * fxVoice;
		FxVoice fxProcessor;
		//Системные поля
		dword	states;				//Состояние канала
		dword lowPrtCounter;		//Счётчик низкого приоритета
		dword mode;					//Режим в котором играет канал		
		dword format;				//Формат проигрываемого буфера
		float waveVolume;			//Громкость волны
		float pickAmp;				//Пиковая амплитуда в волне
		float tailTime;				//Время малозначительного хвоста волны
		float fadeOutTime;			//Время фейда в 0
		array<UINT32> xWmaBuffer;	//Буфер для xWma волны
		long time;					//Логическое время создания канала		
		dword samplesCount;			//Количество выборок в проигрываемом буфере
		UINT64 startPlayPosition;	//Позиция войса с которой было начато проигрывание
		UINT64 continuePosition;	//Позиция буфера с которой продолжели проигрывание
		void * currentUniqPtr;		//Привязка к хозяину
	};

public:
	SoundsEngine();
	virtual ~SoundsEngine();

	//Инициализировать
	bool Init();
	//Освободить ресурсы
	void Release();
	//Обновить состояние
	void Update();
	//Приостановить проигрывание звуков
	void SetPause(bool isSetPause);

	//Загрузить звуковой банк
	bool LoadSoundBank(const char * path);
	//Удалить звуковой банк
	void ReleaseSoundBank(const char * path);

	//Создать мастер сцены
	SceneMaster * CreateSceneMaster();
	//Попытаться получить доступный звуковой канал
	SoundChannel * GetSoundChannel(SoundBankFileSound & sbfs, SoundBankFileWaveInfo * selWave, dword startPosition, SceneMaster & scene, void * uniqPtr);
	//Освободить звуковой канал
	void ReleaseSoundChannel(SoundChannel * ch, void * uniqPtr);
	//Получить количество активных каналов
	dword GetPlayChannels();
	//Получить максимальное количество каналов
	dword GetMaxChannels();
	//Получить количество активных каналов для доигрывания
	dword GetPlayReservedChannels();
	//Получить максимальное количество каналов для доигрывания
	dword GetMaxReservedChannels();

#ifdef _XBOX
	//Получить объект XAudio
	IXAudio2 * GetXAudio();
#endif

public:
#ifndef NO_TOOLS
	//Запустить волну на проигрывание 16бит (для звукового редактора)
	bool EditPrewiewPlay(const dword userId[4], EditPrewiewWaveParams * mainWave, EditPrewiewWaveParams * mirrorWave, bool isMirror);
	//Переключить волну на зеркальную или оригинальную
	bool EditPrewiewSwitch(const dword userId[4], bool isMirror);
	//Играет ли волна на прослушивании (для звукового редактора), userId[4] == 0 игнорировать его
	bool EditPrewiewIsPlay(const dword userId[4], dword * samplesCount = null, bool * playWithMirror = null);
	//Остановить прослушивание волны, userId[4] == 0 игнорировать его
	void EditPrewiewStop(const dword userId[4]);
	//Установить громкость прослушиваемой волны, userId[4] == 0 игнорировать его
	void EditPrewiewSetVolume(float volume, const dword userId[4]);
	//Установить громкость мастера предпрослушивания
	void EditPrewiewSetMasterVolume(float volume);
#endif

private:
	//Обновить состояние музыкального канала в зависимости от состояния проигрывателя
	void UpdateMusicState();
	//Выбрать канал для замещения
	static long ChannelSelector(SoundChannel ** ch, long count, long priority, long time);
	//Поменять местами каналы
	static void ExchangeChannels(SoundChannel * & ch1, SoundChannel * & ch2);

public:
	//Получить частоту для канала эфектов
	static dword GetFxMixFreq();
	//Получить частоту для канала музыки
	static dword GetMusMixFreq();

private:
	//Звуковая библиотека
	IXAudio2 * pXAudio2;
	//Мастер задающий общую громкость
	IXAudio2MasteringVoice * masteringVoice;
	//Премастер канала эффектов
	IXAudio2SubmixVoice * fxPremasteringVoice;
	FxPremaster fxPremaster;
	//Премастер музыкального канала
	IXAudio2SubmixVoice * musicPremasteringVoice;
	//Премастера сцен
	array<SceneMaster *> sceneMasters;
	//Массив звуковых каналов для эффектов
	SoundChannel * sound[c_soundChannels];
	dword soundCount;
	//Массив каналов, находящихся на доигрывании
	SoundChannel * reserved[c_reserveChannels];
	dword reservedCount;
	//Общий массив каналов, используемых системой
	SoundChannel soundChannels[c_totalSoundChannels];
	//Счётчик создаваемых звуков
	long creationCounter;
	//Движёк на паузе
	long isPause;
	//Нотификация изменения состояния проигрывателя пользователя (xbox)
	HANDLE notificationListenerForXbox;
	//Буфер для проигрывания зацикленной тишины
	XAUDIO2_BUFFER silenceBuffer;
	byte silenceLoop[4096];	
private:
	static SoundsEngine * engine;

private:
#ifndef NO_TOOLS
	//Звук для прослушивания
	IXAudio2SubmixVoice * previewMaster;
	float previewMasterVolume;
	IXAudio2SourceVoice * previewVoice;
	IXAudio2SourceVoice * previewVoiceMirror;

	dword previewUserId[4];
#endif
};

#endif

