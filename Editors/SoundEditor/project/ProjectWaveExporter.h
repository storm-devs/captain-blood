

#pragma once

#include "..\SndBase.h"


class ProjectWave;

class ProjectWaveExporter
{
	//Структура предоставляющая тип данных для выгрузки
	struct ExportInfo
	{
		bool inProcess;		//Информация в текущий момент обрабатываеться
		bool isDone;		//Обработка информации завершена
		bool isError;		//Была ошибка при выгрузки
		byte * buffer;		//Выделенный буфер под подготовленные данные
		dword size;			//Размер буфера данных
	};

	//Индексы на каждый тип данных
	enum ExportsIndex
	{
		e_pcWave = 0,		//Информация выгрузки волны для формата PC
		e_xboxWave = 1,		//Информация выгрузки волны для формата Xbox
		e_phonemes = 2,		//Информация выгрузки фонем
		e_count = 3			//Общее количество элементов выгрузки
	};

	struct QuiueRecord
	{
		ProjectWave * wave;				//Волна, которую надо обработать
		ExportInfo exports[e_count];	//Различные типы данных, которые надо получить из волны
		ErrorId errorCode;				//Код ошибки
		bool markForDelete;				//Эта запись в очереди подлежит удалению, новые работы с ней не начинать
		bool waitToSave;				//Подготовленные данные необходимо сохранить на диске
		bool saveProcess;				//Процесс сохранения
	};

	//Передаёться в экспортёр волны
	struct WaveDataExportParams
	{
		bool isPC;			//Волна true для PC, false Xbox
		bool isMakePreview;	//Режим генерации данных для прослушивания
	};

	//Заголовок выгруженных данных волны, распологаеться в начале буфера с выгруженной волной
	struct WaveDataInfo
	{
		dword waveExSize;	//Размер структуры описывающей формат волны в байтах
		dword waveDataSize;	//Размер данных волны в байтах
		dword waveSplCount;	//Количество сэмплов в волне
		dword waveFormat;	//Описание формата волны для звукового банка
		dword sizeofAdds;	//Размер дополнительных данных
		byte adds[128-20];	//Дополнителоьные данные
	};

public:
	ProjectWaveExporter();
	~ProjectWaveExporter();

	//Запустить экспортёр
	static void StartExporter();
	//Запустить экспортёр
	static void StopExporter();
	//Установить нормальный режим работы
	static void SetActiveMode();
	//Установить фоновый режим работы
	static void ReleaseActiveMode();
	//Экспорт волны для предварительного прослушивания
	static byte * WavePreviewMake(ProjectWave * wave, bool isPC, ErrorId errorCode);
	//Получить указатель на структуру WAVEFORMATEX в буфере предпрослушивания
	static WAVEFORMATEX * WavePreviewGetFormat(byte * buffer);
	//Получить указатель на данные в буфере предпрослушивания
	static byte * WavePreviewGetData(byte * buffer, dword & dataSizeInBytes);
	//Получить экспортный размер посчитаный при генерации данных предпрослушивания
	static dword WavePreviewGetDataSize(byte * buffer);


//---------------------------------------------------------------------------------------------
//Работа с очередью
//---------------------------------------------------------------------------------------------
public:
	//Добавить волну в очередь экспорта
	static void AddWaveToQueue(ProjectWave * wave, bool isNeedPhonemes, bool isIdleExport);
	//Удалить волну из очереди экспорта
	static void DelWaveFromQueue(ProjectWave * wave);

//---------------------------------------------------------------------------------------------
//Потоковые функции	(потоки не объеденять, т.к. выгрузка могут запускать другие приложения)
//---------------------------------------------------------------------------------------------
private:
	//Выгрузка волны в PC формате
	static DWORD WINAPI ThreadPCWaveExport(LPVOID lpParameter);
	//Выгрузка волны в Xbox формате
	static DWORD WINAPI ThreadXboxWaveExport(LPVOID lpParameter);
	//Подготовка фонем для волны
	static DWORD WINAPI ThreadPhonemesExport(LPVOID lpParameter);
	//Шаг экспорта
	static void ProcessFunction(dword exportIndex, byte * (_cdecl * ExportFunction)(ProjectWave * wave, dword & size, void * params, ErrorId errorCode), void * params);
	//Ожидание работы
	static void IdleFunction();

//---------------------------------------------------------------------------------------------
//Функции экспорта
//---------------------------------------------------------------------------------------------
private:	
	//Удалить выделенный элемент из очереди. Необходимо находиться в секции queueAccessor
	static void DeleteQueueElement(long index);
	//Преобразование данных волны в нужный формат
	static byte * _cdecl ConvertWaveData(ProjectWave * wave, dword & size, void * params, ErrorId errorCode);
	//Подготовка волны пониженой частоты (16bit) возвращает выделеную по NEW память
	static void DownSample(const byte * sourceData, dword srcSamples, dword srcSampleRate, dword srcChannels, dword dstSampleRate, array<byte> & buffer, dword & dstSamples);
	//Волна 16 бит
	static byte * ConvertToPCM(bool isPC, const byte * sourceData, dword samplesCount, dword channels, dword sampleRate, bool isMakePreviewMode);
	//XMA (только на XBOX)
	static byte * ConvertToXMA(const byte * sourceData, dword samplesCount, dword channels, dword sampleRate, dword compressionRate, const char * waveName, bool isMakePreviewMode);
	//xWMA
	static byte * ConvertToXWMA(bool isPC, const byte * sourceData, dword samplesCount, dword channels, dword sampleRate, dword compressionRate, const char * waveName, bool isMakePreviewMode);
	//Сохранить волну во временный файл RIFF PCM
	static void ConvertSaveToTmpFile(char * name, dword nameMaxSize, const char * waveName, const byte * sourceData, dword samplesCount, dword channels, dword sampleRate, bool isSaveWithExtentione);
	//Загрузить из полученого RIFF файла необходимые чанки
	static void ConvertLoadTmpWavFile(const char * path, dword id, array<byte> & format, array<byte> & table, array<byte> & data);
	//Построение списка фонем по волне	
	static byte * _cdecl PreparePhonemes(ProjectWave * wave, dword & size, void * params, ErrorId errorCode);
	//Выполнение внешнего конвертора
	static bool DoExternProcess(const char * commandString);
public:
	//Сохранить волну во временный файл RIFF PCM
	static void SaveWave(const char * path, const byte * sourceData, dword samplesCount, dword channels, dword sampleRate, HANDLE handle = INVALID_HANDLE_VALUE);

//---------------------------------------------------------------------------------------------
private:
	HANDLE threadPCWaveExport;
	HANDLE threadXboxWaveExport;
	HANDLE threadPhonemesExport;
	array<QuiueRecord> exportQueue;
	CritSection queueAccessor;
	volatile bool isWork;
	volatile bool threadPCExit;
	volatile bool threadXboxExit;
	volatile bool threadPhonemesExit;
	volatile long activeWorkModeCounter;

//---------------------------------------------------------------------------------------------
private:
	static ProjectWaveExporter globalObject;
};