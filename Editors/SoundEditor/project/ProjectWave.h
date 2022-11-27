
#pragma once

#include "..\SndBase.h"
#include "ProjectObject.h"


//#define ProjectWaveDebugEnable
#define ProjectWaveDebugId_import 0, __FILE__
#define ProjectWaveDebugId_loadconv 1, __FILE__
#define ProjectWaveDebugId_play 2, __FILE__
#define ProjectWaveDebugId_soundpreview 3, __FILE__
#define ProjectWaveDebugId_cnvwavedata 4, __FILE__
#define ProjectWaveDebugId_genphonemes 5, __FILE__
#define ProjectWaveDebugId_SaveToWav 6, __FILE__
#define	ProjectWaveDebugId_DecomposeTest 7, __FILE__

class ProjectWave : public ProjectObject
{
public:
	enum WaveOptionConsts
	{
		//Параметры подготовки волны
		wave_crop_in_threshold = 20,		//До какого уровня отсекать пустоту в начале
		wave_crop_in_samples_fade = 256,	//Количество сэмплов на фэйд от начала данных до порогового сэмпла
		wave_crop_out_threshold = 50,		//С какого уровня в конце отсекать хвост волны
		wave_crop_out_samples_fade = 256,	//Количество сэмплов на фэйд от порогового сэмпла до конца
		wave_granular_align = 128,			//По какому количеству сэмплов надо выравнивать волну (степень 2)
		wave_minsize = wave_granular_align*4,//Минимально допустимое количество сэмплов
		//Параметры экспорта
		wce_samplerate_min = 8000,	//Нижняя допустимая частота выгрузки
		wce_samplerate_max = 48000,	//Верхняя предельная частота выгрузки
		wce_compression_min = 0,	//Самое низшее качество и сильное сжатие
		wce_compression_med = 40,	//Самое низшее качество и сильное сжатие
		wce_compression_max = 63,	//Самое лучшее качество и меньшее сжатие
		wce_compression_def = wce_compression_med,	//Коэфициент качества по умолчанию
		//Тип сжатия
		wce_format_16bit =   0,		//16 бит, некомпрессированый
		wce_format_xma =     1,		//ХМА
		wce_format_xwma =    2,		//xWMA
		wce_format_max_count = 7,	//Предельно допустимое количество форматов		
		//Платформа
		wce_platform_pc =    0,		//Волну готовить для PC
		wce_platform_xbox =  1,		//Волну готовить для XBOX
	};

	struct WaveOptions
	{
		dword samplerate;		//Частота сэмплирования
		byte channels;			//Количество каналов
		byte format;			//Формат волны
		byte compression;		//Степень сжатия
		byte platform;			//Платформа
		dword reserved[2];

		bool IsEqual(const WaveOptions & opts) const;
	};

	struct FL_Debug
	{
		FL_Debug(){};
		FL_Debug(const char * f, long id)
		{
			cppFile = f;
			cppId = id;
		};

		const char * cppFile;
		long cppId;
	};

public:
	enum GlobalConsts
	{
		c_namelen = 48,
	};

private:
	//Данные для семплера "smpl"
	struct SamplerHeader
	{
		dword manufacturer;			//0-0xffffffff (0)
		dword product;				//0-0xffffffff (0)
		dword samplePeriod;			//0-0xffffffff (1 / Samplers Per Second * 10^9)
		dword midiUnityNote;		//0-127 (28)
		dword midiPitchFraction;	//0-0xffffffff (0)
		dword smpteFormat;			//0, 24, 25, 29, 30 (0)
		dword smpteOffset;			//... (0)
		dword numSampleLoops;		//0-0xffffffff (0,1)
		dword samplerData;			//(0)
	};

	struct SampleLoop
	{
		dword cuePointID;			//unique ID (0)
		dword type;					//0 forward, 1 pingpong, 2 reverse, 3+ reserved (0)
		dword start;				//byte offset in sample data
		dword end;					//byte offset in sample data
		dword fraction;				//0 is no fraction (0)
		dword playCount;			//0 is infinite
	};

	struct SapmlerData
	{
		SamplerHeader header;
		SampleLoop loops[1];
	};

	enum FileVersion
	{
		fileVersionError = 0,
		fileVersion01 = 1,
		fileVersion02 = 2,
		fileVersion03 = 3,

		fileVersion = fileVersion03,
	};

	enum Defects
	{
		df_ok = 0,						//Нет вопросов по волне
		df_amplitude_mask = 0x7f,		//Пиковое использованность динамического диапазона в процентах
		df_amplitude_shift = 0,
		df_middle_mask = 0x7f << 7,		//Среднее использование динамического диапазона в процентах
		df_middle_shift = 7,
		df_silence_mask = 0x7f << 14,	//Общий объём тишины относительно всего файла в процентах
		df_silence_shift = 14,
		df_size_align =     0x400000,	//Размер волны имеет не кратный грануляции
		df_clicks =         0x800000,	//В волне есть щелчёки		
		df_ignore_amp = 0x1000000,		//Игнорировать пиковую амплитуду
		df_ignore_mid = 0x2000000,		//Игнорировать среднюю амплитуду
		df_ignore_sln = 0x4000000,		//Игнорировать тишину
		df_ignore_clk = 0x8000000,		//Игнорировать щелчки

		df_min_amplitude = 50,			//Минимально допустимый диапазон в волне
		df_min_middle = 20,				//Минимально допустимое среднее значение
		df_max_silence = 30,			//Предельно допустимое количество нулевых сэмплов в волне
		df_min_click_level = 20,		//Минимальный уровень (в процентах) при котором засчитываеться щелчёк
		df_unimportant_amp = 10,		//Амплитуда в процентах, ниже которой остаток волны считаеться незначительным
	};

	struct DataChunk
	{
		char idString[64];				//Идентификатор преобразованный в строку
		long version;					//Версия данных
		dword dataSize;					//Размер данных
		float playTime;					//Время проигрывания
		dword samplesCount;				//Количество отсчётов в файле
		dword bytesPerSample;			//Байт на выборку
		WAVEFORMATEX waveFormat;		//Формат волны		
		GUIPoint hotPreview[64];		//Предпросмотр (высота 0..16)
		dword hotPreviewCount;			//Количество точек в предпросмотре
		dword hotPreviewStep;			//Сколько отсчётов в 1ой точке предпросмотра		
		dword sourceLowDateTime;		//Время последней модификации файла (lo)
		dword sourceHighDateTime;		//Время последней модификации файла (hi)
		dword waveCheckSum;				//Контрольная сумма
		WaveOptions exportOptionsPC;	//Опции выгрузи для PC
		WaveOptions exportOptionsXBOX;	//Опции выгрузи для XBOX		
		char sourcePath[1024];			//Откуда взяли
		dword defects;					//Флаги дефектов волны
		float unimportantTime;			//Время незначительного остатка, когда звук можно проигнорировать
		float maxAmplitude;				//Максимально допустимая амплитуда
		char reserved[2036];			//Зарезервированно на будующие изменения
		char note[2048];				//Коментарии
	};

public:
#pragma pack(push, 1)

	struct ExportTemporaryFile
	{
		dword size;						//Общий размер чанка с заголовком
		dword waveCheckSum;				//Контрольная сумма волны
		dword waveDataSize;				//Размер волны в байтах
	};

	//Экспорчённые данные
	struct ExportWavesPart : public ExportTemporaryFile
	{
		dword waveformatSizeXbox;		//Описание структуры описывающей волну для xbox
		dword waveSizeForXbox;			//Размер данных для xbox
		dword waveSmplsForXbox;			//Количество сэмплов для xbox
		dword exportFormatXbox;			//Формат выгруженой волны для хвох (SoundBankFileWave)
		WaveOptions exportOptsXbox;		//С такими опциями выгружали волну для хвох
		dword waveformatSizePC;			//Описание структуры описывающей волну для pc
		dword waveSizeForPC;			//Размер данных для pc
		dword waveSmplsForPC;			//Количество сэмплов для pc
		dword exportFormatPC;			//Формат выгруженой волны для рс (SoundBankFileWave)
		WaveOptions exportOptsPC;		//С такими опциями выгружали волну для рс
		//далее идут:
		//структура описывающая волну для xbox
		//данные волны для xbox
		//структура описывающая волну для pc
		//данные волны для pc
	};
	
	//Фонемы
	struct ExportPhonemesPart : public ExportTemporaryFile
	{
		struct Phoneme
		{
			byte id;
			dword time;
			float itensity;
		};
		
		dword count;					//Количество фонем
		Phoneme phoneme[1];				//Список фонем с количеством count
	};
#pragma pack(pop)

public:
	ProjectWave(const UniqId & folderId);
	~ProjectWave();

//----------------------------------------------------------------------------------------
//Доступ к данным волны
//----------------------------------------------------------------------------------------
public:
	//Загрузим данные волны
	void LoadWaveData(long id, const char * cppFile);
	//выгрузить данные волны
	void UnloadWaveData(long id, const char * cppFile);
	//Проверить размер
	bool CheckGranularSize();
	//Получить сумму проверки данных
	dword GetWaveDataCheckSum() const;
	//Получить объём данных
	dword GetWaveDataSize() const;
	//Получить данные волны
	const byte * GetWaveData(dword & size) const;
	//Проверить волну на правильность
	bool IsNoWaveDefects() const;
	//Получить значение флага игнорирования дефекта амплитуды
	bool GetDefectIgnoreAmp() const;
	//Установить значение флага игнорирования дефекта амплитуды
	void SetDefectIgnoreAmp(bool isSet);
	//Получить значение флага игнорирования дефекта среднего
	bool GetDefectIgnoreMid() const;
	//Установить значение флага игнорирования дефекта среднего
	void SetDefectIgnoreMid(bool isSet);
	//Получить значение флага игнорирования дефекта тишины
	bool GetDefectIgnoreSln() const;
	//Установить значение флага игнорирования дефекта тишины
	void SetDefectIgnoreSln(bool isSet);
	//Получить значение флага игнорирования дефекта щелчков
	bool GetDefectIgnoreClk() const;
	//Установить значение флага игнорирования дефекта щелчков
	void SetDefectIgnoreClk(bool isSet);
private:
	void SetDefectFlag(bool isSet, dword mask);
public:
	//Получить описание волны
	void GetDescription(string & text, string & tips);	
	//Получить заметки по волне
	const char * GetWaveNotes();
	//Установить заметки по волне
	void SetWaveNotes(const char * newNotes);
	//Сохранить данные волны на диске
	void SaveWaveToWAV(const char * path);

//----------------------------------------------------------------------------------------
//Доступ к данным экспорта и параметрам экспорта
//----------------------------------------------------------------------------------------
public:
	//Получить подготовленные данные волны или фонемы
	bool GetExportData(array<byte> * buffer, bool dontStartExport, bool isPhonemes);
	//Установить формат экспорта
	void SetExportOptions(dword platform, const WaveOptions & opts);
	//Получить формат экспорта
	const WaveOptions & GetExportOptions(dword platform);
	//Остановить экспорт, удалить выгруженные данные
	void DeleteExportData();


//----------------------------------------------------------------------------------------
//Общии параметры волны
//----------------------------------------------------------------------------------------
public:
	//Получить массив быстрого предпросмотра
	GUIPoint * GetHotPreview();
	//Получить количество точек быстрого предпросмотра
	dword GetHotPreviewCount();
	//Получить время проигрывания
	float GetPlayTime();	
	//Получить время проигрывания
	const ExtName & GetPlayTimeName();
	//Получить описание файла
	const WAVEFORMATEX & GetFormat();
	//Можноли кодировать волну в xWma
	bool IsCanXWmaEncode(dword sampleRate) const;
	//Проверка заданного количества сэмплов на возможность кодирования в xWma
	static bool IsSamplesCanXWmaEncode(dword samples);
	//Получить количество сэмплов в волне
	dword GetSamplesCount();
	//Получить время остатка, который можно пропустить
	float GetUnimportantTime();
	//Получить пиковую нормализованную амплитуду в волне
	float GetPickAmplitude();

//----------------------------------------------------------------------------------------
//Прослушивание волны
//----------------------------------------------------------------------------------------
public:
	//Проиграть волну, чтобы послушать
	bool Play();
	//Проиграть волну, чтобы послушать разницу
	bool PlayExported(dword wce_platform, bool isOriginal, ErrorId & errorCode);
	//После запуска предпросмотра можно получить размер данных
	dword GetExportedWaveSize(dword platform);
	//Переключить прослушивание на оригинальную волну или экспорчёную
	void PlaySwitch(bool isOriginal);
	//Остановить прослушивание волны
	void Stop();
	//Прослушиваеться ли волна
	bool IsPlay();
	//Прослушиваеться ли волна
	bool IsPlay(float & curPosition);
	//Прослушиваеться ли волна вместе с экспорчёной
	bool IsPlayMirror();
	//Установить громкость прослушивания
	void PlayVolume(float vol);


//----------------------------------------------------------------------------------------
//Методы для проекта
//----------------------------------------------------------------------------------------
public:
	//Отложеная работа
	void WorkUpdate(bool deleteNow);
	//Фоновый экспорт
	bool IdleExport(bool isPhonemes);
	//Импортировать волну, возвращает errorId или -1 если всё впорядке
	ErrorId Import(const char * path, ProjectWave * replaceWave);
private:
	//Обрезать хвосты и нормализовать длинну по размеру кратному 128 сэмплов
	void CropData(bool isStereo);
	//Анализировать данные и сохранить результат в data.defects
	void WaveAnalize();
	//Найти время остатка, который можно пропустить
	void FindUnimportantRemainder();
	//Собрать статистику по каналу
	void ChannelAnalize(short * buffer, dword samples, dword step, float & ampl, float & middle, dword & silenceCount, dword & clicksCount);
	//Прочитать формат файла
	bool ReadMMIOFormat(HMMIO & hmmio, MMCKINFO & ckRiff, array<byte> & pcmWFBuffer);
	//Прочитать данные файла
	ErrorId ReadMMIOData(HMMIO & hmmio, MMCKINFO & ckRiff, bool isStereo, const char * importFileName);
	//Попытаться прочитать метки цикла
	void ReadLoopLabels(HMMIO & hmmio, MMCKINFO & ckRiff);
	//Обновить время проигрывания
	void UpdatePlayTime();

//----------------------------------------------------------------------------------------
//Методы для экспортёра
//----------------------------------------------------------------------------------------
public:
	//Получить имя волны для экспорта
	const char * ProcessExportGetName();
	//Установили волну в очередь
	void ProcessExportSetToQueue();	
	//Удалили волну из очереди
	void ProcessExportRemoveFromQueue(ErrorId errorCode);
	//Получить код ошибки экспорта. -1 ошибки нет
	ErrorId GetErrorCode();
	//Сгенерировать путь до экспорчёного файла с данными волны
	void BuildExportPath(string & fileName, bool isPhonemes);



//----------------------------------------------------------------------------------------
//ProjectObject
//----------------------------------------------------------------------------------------
public:	
	//Перезаписать идентификатор объекта
	virtual void ReplaceId(const UniqId & newId);
private:
	//Сохранить объект
	virtual ErrorId OnSaveObject(const char * defaultFilePath);
	//Загрузить описание волны
	virtual ErrorId OnLoadObject(const char * defaultFilePath);
	//Событие удаления объекта из проекта
	virtual void OnDeleteObject();

private:	
	DataChunk data;					//Данные волны в чанке для удобства сохранения
	byte * rawWaveDataPtr;			//Невыровненный поинтер на буфер для волны
	byte * waveData;				//Данные файла

private:
	ExtName playTimeName;			//Описание строки времени проигрывания	
	char playTimeData[16];			//Строковой вариант времени проигрывания
	long waveDataRefCounter;		//Количество ссылок на загруженные данные
	double lastUnloadTime;			//Время когда было освобождена волны
	CritSection waveDataAccessor;	//Контроль за загрузкой-выгрузкой данных волны
	CritSection optsAccessor;		//Контроль за доступом к опциям экспорта
	byte isWaveLoadForPlay;			//Волна была загружена для проигрывания
	byte inExportQueue;				//Находиться в очереди выгрузки
	byte * pcPreviewData;			//Данные предпрослушивания экспорчёной волны для PC (преобразованна обратно в PCM)
	byte * xboxPreviewData;			//Данные предпрослушивания экспорчёной волны для Xbox (преобразованна обратно в PCM)
	ErrorId exportErrorCode;		//Код ошибки экспорта волны
	char textId[64];				//Идентификатор в виде текста
#ifdef ProjectWaveDebugEnable
	array<FL_Debug> flDebug;		//Массив для отладки алокаций
#endif
	static long totalWaveDataLoad;	//Сколько волн прогружено всего
};


//----------------------------------------------------------------------------------------
//Доступ к данным волны
//----------------------------------------------------------------------------------------

//Проверить размер
inline bool ProjectWave::CheckGranularSize()
{
	return (data.samplesCount % wave_granular_align == 0);
}

//Получить сумму проверки данных
inline dword ProjectWave::GetWaveDataCheckSum() const
{
	return data.waveCheckSum;
}

//Получить объём данных
inline dword ProjectWave::GetWaveDataSize() const
{
	return data.dataSize;
}

//Получить данные волны
inline const byte * ProjectWave::GetWaveData(dword & size) const
{
	size = data.dataSize;
	return waveData;
}

//Проверить волну на правильность
inline bool ProjectWave::IsNoWaveDefects() const
{
	dword amp = (data.defects & df_amplitude_mask) >> df_amplitude_shift;
	if(!(data.defects & df_ignore_amp) && amp < df_min_amplitude) return false;
	dword middle = (data.defects & df_middle_mask) >> df_middle_shift;
	if(!(data.defects & df_ignore_mid) && middle < df_min_middle) return false;
	dword sln = (data.defects & df_silence_mask) >> df_silence_shift;
	if(!(data.defects & df_ignore_sln) && sln > df_max_silence) return false;
	if(!(data.defects & df_ignore_clk) && data.defects & df_clicks) return false;
	if(data.defects & df_size_align) return false;
	return true;
}

//Получить значение флага игнорирования дефекта амплитуды
inline bool ProjectWave::GetDefectIgnoreAmp() const
{
	return (data.defects & df_ignore_amp) != 0;
}

//Установить значение флага игнорирования дефекта амплитуды
inline void ProjectWave::SetDefectIgnoreAmp(bool isSet)
{
	SetDefectFlag(isSet, df_ignore_amp);
}

//Получить значение флага игнорирования дефекта среднего
inline bool ProjectWave::GetDefectIgnoreMid() const
{
	return (data.defects & df_ignore_mid) != 0;
}

//Установить значение флага игнорирования дефекта среднего
inline void ProjectWave::SetDefectIgnoreMid(bool isSet)
{
	SetDefectFlag(isSet, df_ignore_mid);
}

//Получить значение флага игнорирования дефекта тишины
inline bool ProjectWave::GetDefectIgnoreSln() const
{
	return (data.defects & df_ignore_sln) != 0;
}

//Установить значение флага игнорирования дефекта тишины
inline void ProjectWave::SetDefectIgnoreSln(bool isSet)
{
	SetDefectFlag(isSet, df_ignore_sln);
}

//Получить значение флага игнорирования дефекта щелчков
inline bool ProjectWave::GetDefectIgnoreClk() const
{
	return (data.defects & df_ignore_clk) != 0;
}

//Установить значение флага игнорирования дефекта щелчков
inline void ProjectWave::SetDefectIgnoreClk(bool isSet)
{
	SetDefectFlag(isSet, df_ignore_clk);
}

__forceinline void ProjectWave::SetDefectFlag(bool isSet, dword mask)
{
	dword flag = isSet ? mask : 0;
	dword needSave = ((flag ^ data.defects) & mask);
	data.defects &= ~mask;
	data.defects |= flag;
	if(needSave != 0) SetToSave();
}

//Получить заметки по волне
inline const char * ProjectWave::GetWaveNotes()
{
	return data.note;
}

//----------------------------------------------------------------------------------------
//Общии параметры волны
//----------------------------------------------------------------------------------------

//Получить массив быстрого предпросмотра
inline GUIPoint * ProjectWave::GetHotPreview()
{
	return data.hotPreview;
}

//Получить количество точек быстрого предпросмотра
inline dword ProjectWave::GetHotPreviewCount()
{
	return data.hotPreviewCount;
}

//Получить время проигрывания
inline float ProjectWave::GetPlayTime()
{
	return data.playTime;
}

//Получить время проигрывания
inline const ExtName & ProjectWave::GetPlayTimeName()
{
	return playTimeName;
}

//Получить описание файла
inline const WAVEFORMATEX & ProjectWave::GetFormat()
{
	return data.waveFormat;
}

//Можноли кодировать волну в xWma
inline bool ProjectWave::IsCanXWmaEncode(dword sampleRate) const
{
	float kScale = sampleRate*(1.0f/44100.0f);
	return IsSamplesCanXWmaEncode(long(data.samplesCount*kScale));
}

//Проверка заданного количества сэмплов на возможность кодирования в xWma
inline bool ProjectWave::IsSamplesCanXWmaEncode(dword samples)
{
	return samples > 100000;
}

//Получить количество сэмплов в волне
inline dword ProjectWave::GetSamplesCount()
{
	return data.samplesCount;
}

//Получить время остатка, который можно пропустить
inline float ProjectWave::GetUnimportantTime()
{
	return data.unimportantTime;
}

//Получить пиковую нормализованную амплитуду в волне
inline float ProjectWave::GetPickAmplitude()
{
	return data.maxAmplitude;
}

//----------------------------------------------------------------------------------------
//Методы для экспортёра
//----------------------------------------------------------------------------------------

//Получить имя волны для экспорта
__forceinline const char * ProjectWave::ProcessExportGetName()
{
	return textId;
}

//Установили волну в очередь
__forceinline void ProjectWave::ProcessExportSetToQueue()
{
	inExportQueue = 1;
}

//Удалили волну из очереди
__forceinline void ProjectWave::ProcessExportRemoveFromQueue(ErrorId errorCode)
{
	exportErrorCode = errorCode;
	inExportQueue = 0;	
}

//Получить код ошибки экспорта. ErrorId::ok ошибки нет
__forceinline ErrorId ProjectWave::GetErrorCode()
{
	return exportErrorCode;
}

//----------------------------------------------------------------------------------------
//ProjectWave::WaveOptions
//----------------------------------------------------------------------------------------

__forceinline bool ProjectWave::WaveOptions::IsEqual(const WaveOptions & opts) const
{
	if(samplerate != opts.samplerate) return false;
	if(channels != opts.channels) return false;
	if(format != opts.format) return false;
	if(compression != opts.compression) return false;
	if(platform != opts.platform) return false;
	return true;
}
