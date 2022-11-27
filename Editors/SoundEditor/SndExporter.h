

#pragma once



#include "SndBase.h"
struct ExtNameStr;

class SndExporter
{
	typedef void (SndExporter::* StageFunctionInit)();
	typedef bool (SndExporter::* StageFunctionStep)();	

	struct Step
	{		
		StageFunctionInit init;
		StageFunctionStep step;
	};

	struct SoundInfo
	{
		UniqId id;
		long name;
		long queueSelect;
		long queueLoop;
	};

	struct WaveInfo
	{
		WaveInfo();

		UniqId id;
		bool needPhonemes;
		bool isStartExport;
		array<byte> exportWaves;
		array<byte> exportPhonemes;
		long waveOffsetXbox;
		long formatOffsetXbox;
		long waveOffsetPC;
		long formatOffsetPC;
		long phonemesOffset;
	};

public:


	enum ErrorCode
	{
		ec_nowaves,
		ec_nobaseparams,
		ec_noattenuation,
		ec_errorwaveexport,
		ec_wavenotfound,
		ec_reimportwave,
		ec_filenotopen,
		ec_cantwrite,
		ec_nomemory,
		ec_forcedword = 0x7fffffff
	};


	struct Error
	{
		ErrorCode code;
		UniqId id;
	};

public:
	SndExporter();
	~SndExporter();


	//Подготовиться к выгрузке проекта
	void ExportPrepareBank(const UniqId & sndBankId);
	//Выполнить шаг выгрузки, возвращает true когда закончена выгрузка
	bool ExportStep();

	//Размер банка для xbox
	dword GetXboxBankSize();
	//Размер банка для pc
	dword GetPCBankSize();

private:
	//Написать о текущей стадии экспорта
	void PrintCurrentStage();
	//Подготовка таблицы звуков, которые будет экспортить
	bool ProcessFillSoundsTable();
	//Инициализация процесса подготовки звуков
	void InitProcessPrepareSounds();
	//Подготовка звуков, waveinfo, и таблицы имён
	bool ProcessPrepareSounds();
	//Инициализация процесса подготовки волн
	void InitProcessPrepareWaves();
	//Подготовка волн
	bool ProcessPrepareWaves();
	//Разметка и выделение памяти
	bool ProcessMemoryMaping();
	//Инициализация подготовки звуков, waveinfo, и таблицы имён
	void InitProcessFillTableSounds();
	//Заполнение таблиц звуков
	bool ProcessFillTableSounds();
	//Инициализация заполнения таблицы описания волн банка
	void InitProcessFillTableWaves();
	//Таблицы описания волн банка
	bool ProcessFillTableWaves();
	//Таблица поиска звуков по имени
	bool ProcessFillTableEntry();
	//Заполнение заголовков банков
	bool ProcessFillHeaders();
	//Бинарная подготовка файла
	bool ProcessPrepare();
	//Сохранение в файл
	bool ProcessSaveToFile();
	//Инициализация процесса бездействия
	void InitProcessIdle();
	//Процесс бездействия
	bool ProcessIdle();

	//Добавить индекс волны
	void AddUniqueWave(const UniqId & id, bool isNeedPhoneme);
	//Добавить запись в отладочную таблицу
	void AddDebugRecord(const UniqId & id, void * ptr);
	//Сохранить блоу данных в файл банка
	bool SaveData(dword offset, void * ptr, dword size);
	//Преобразовать банк только для РС (GAME_RUSSIAN)
	static void ConvertBankToPConly(IFile * file);

public:
	array<Error> errors;
	ExtNameStr * progressOutput;

private:
	long exportStageCount;				//Текущие состояние выгрузки
	array<SoundInfo> exportSounds;		//Таблица звуковов которые экспортим
	array<WaveInfo> exportWaves;		//Таблица волн которые экспортим
	array<char> names;					//Таблица имён
	//array<byte> waveFmtsXbox;			//Используемые форматы для xbox
	//array<byte> waveFmtsPC;				//Используемые форматы для PC
	array<byte> data;					//Данные банка без волн
	dword dataSize;						//Размер экспортных данных
	dword waveInfoCount;				//Количество описаний волн	
	dword entryTableSize;				//Размер входной таблицы поиска
	dword currentQueuePos;				//Текущая относительная позиция в буфере очередей
	dword currentIndexCounter;			//Счётчик индексов, используемый при экспорте разбросанном по кадрам
	dword currentCounter1;				//Дополнительный счётчик
	dword currentCounter2;				//Дополнительный счётчик
	UniqId soundBankId;					//Идентификатор звукового банка
	UniqId soundBankFolderId;			//Идентификатор папки звукового банка
	bool toNextStage;					//Переходим на следующую стадию
	bool isErrors;						//Были ошибки, сохранять файл нельзя
	static Step exportSteps[];			//Массив шагов выгрузки

private:
	//Файл куда сохраняем банк
	string filePath;
	IFile * file;
	//Счётчик относительного смещения расширений банка
	dword bankExtrasCount;
	dword bankExtrasOffset;
	//Размеры части для xbox
	dword xboxPartSize;
	dword xboxWavesDataSize;
	//Указатели на различные части банка
	SoundBankFileId * bankId;
	//Xbox часть
	SoundBankFileHeader * bankXboxHeader;
	SoundBankFileSound * bankXboxSounds;
	SoundBankFileWaveInfo * bankXboxInfos;
	SoundBankFileWave * bankXboxWaves;
	SoundBankFileSound ** bankXboxEntry;
	SoundBankFileExtra * bankXboxExtras;
	byte * bankXboxQueues;
	byte * bankXboxNames;
	byte * bankXboxExtrasData;
	byte * bankXboxWavesDataFormat;
	byte * bankXboxWavesData;
	//PC часть
	dword pcPartSize;
	dword pcWavesDataSize;
	SoundBankFileHeader * bankPCHeader;
	SoundBankFileSound * bankPCSounds;
	SoundBankFileWaveInfo * bankPCInfos;
	SoundBankFileWave * bankPCWaves;
	SoundBankFileSound ** bankPCEntry;
	SoundBankFileExtra * bankPCExtras;
	byte * bankPCQueues;
	byte * bankPCNames;
	byte * bankPCExtrasData;
	byte *	bankPCWavesDataFormat;
	byte * bankPCWavesData;
	dword bankPCDebugTableOffset;
	SoundBankFileObjectId * bankPCDebugTable;
	dword bankPCDebugTableCounter;
	dword bankPCDebugTableSize;
};














