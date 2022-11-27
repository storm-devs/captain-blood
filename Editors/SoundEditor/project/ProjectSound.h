
#pragma once

#include "..\SndBase.h"
#include "ProjectObject.h"
#include "SoundParams.h"

struct SoundPreviewExport;
struct EditPatchSoundData;

class ProjectSound : public ProjectObject
{
	friend struct SoundPreviewExport;
public:

	enum Consts
	{
		c_namelen = 16,
		c_maxwavescount = 256,
	};


	struct Wave
	{
		UniqId waveId;				//Индекс волны в проекте
		float weight;				//Веc выбора волны (ненормализованое значение)
		float time;					//Время дополнительной активности волны (занимает виртуально канал)
		float volume;				//Громкость 0..1
	};

public:
	ProjectSound(const UniqId & folder);
	~ProjectSound();


//----------------------------------------------------------------------------------------
//Параметры звука
//----------------------------------------------------------------------------------------
public:
	//Установить ссылку на глобальные базовые параметры (для невалидной ссылки используються локальные параметры)
	void SetGlobalBaseParams(const UniqId & id);
	//Получить ссылку на глобальные базовые параметры
	const UniqId & GetGlobalBaseParams();
	//Получить ссылку на локальные базовые параметры
	SoundBaseParams & GetLocalBaseParams();
	//Получить текущие параметры звука
	const SoundBaseParams * GetCurrentBaseParams();
	//Установить ссылку на глобальный график затухания (для невалидной ссылки используються локальный график)
	void SetGlobalAttenuation(const UniqId & id);
	//Получить ссылку на глобальный график затухания
	const UniqId & GetGlobalAttenuation();		
	//Получить локальный график затухания
	SoundAttGraph & GetLocalAttenuation();
	//Получить текущий график затухания
	const SoundAttGraph * GetCurrentAttenuation();


//----------------------------------------------------------------------------------------
//Волны
//----------------------------------------------------------------------------------------
public:
	//Получить количество волн в звуке
	long GetWavesCount();
	//Получить волну из массива
	Wave & GetWave(long index);
	//Добавить новые волны
	bool AddWaves(const UniqId * waves, long count);
	//Удалить волны
	void DeleteWaves(long * indeces, dword count);
	//Коэфициент нормализации волн
	float GetKSumWeights();
	//Получить минимальное и максимальные времена проигрывания
	void GetPlayTimes(float & minTime, float & maxTime);


//----------------------------------------------------------------------------------------
//Прослушивание
//----------------------------------------------------------------------------------------
public:
	//Подготовить звук для прослушивания
	bool MakePreview();
	//Закончить прослушивание звука и освободить память
	void StopPreview();
	//Получить данные для обновления звука в сервисе
	bool GetPatchSoundData(EditPatchSoundData & patchData);
	//Обновить параметры звука, которые возможно при проигрывании
	void UpdateExportParams();


//----------------------------------------------------------------------------------------
//Методы для проекта
//----------------------------------------------------------------------------------------
public:
	//Обновление параметров волны во времени
	bool WorkUpdate();
	//Установить на обноврение предпросмотр
	void ResetUpdateCounters();

//----------------------------------------------------------------------------------------
//События удаления глобальных данных
//----------------------------------------------------------------------------------------
public:
	//Событие удаление волны их проекта
	void OnDeleteWave(const UniqId & waveId);
	//Событие изменения данных волны
	void OnReplaceWave(const UniqId & waveId);
	//Событие удаление волны их проекта
	void OnDeleteSoundParams(const UniqId & waveId);
	//Событие удаление волны их проекта
	void OnDeleteAttenuation(const UniqId & waveId);

//----------------------------------------------------------------------------------------
//ProjectObject
//----------------------------------------------------------------------------------------
private:
	//Сохранить объект
	virtual ErrorId OnSaveObject(const char * defaultFilePath);
	//Загрузить из файла
	virtual ErrorId OnLoadObject(const char * defaultFilePath);


//----------------------------------------------------------------------------------------
private:
	UniqId baseParamsId;				//Внешнии базовые параметры звука
	SoundBaseParams baseParams;			//Собственные параметры звука
	UniqId attenuationId;				//Внешний график затухания
	SoundAttGraph attenuation;			//Собственный график затухания
	array<Wave> waves;					//Волны	

private:
	SoundPreviewExport * previewExport;	//Данные предпросмотра
	long previewRefCounter;				//Счётчик запросов на прослушивание
	double previewDeleteTime;			//Время последнего удаления
	bool isNeedRefreshPreview;			//Когда понадобиться следующие данные для прослушивания, их надо перегрузить
	dword updateCounterSBP;				//Счётчик для проверки обновления базовых параметров (для прослушивания звука)
	dword updateCounterAtt;				//Счётчик для проверки графика затухания (для прослушивания звука)	

private:	
	dword exportCounterWave;			//Текущий экспорт волны
};


