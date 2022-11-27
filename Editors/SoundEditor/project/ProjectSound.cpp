

#include "ProjectSound.h"
#include "..\SndOptions.h"

//Структура данных для экспорта звука для нужд редактора
struct SoundPreviewExport
{
public:
	SoundPreviewExport();
	bool Init(ProjectSound * snd);
	void UpdateParams();
	bool Unload();
	

private:
	~SoundPreviewExport();

private:
	SoundBankFileSound sound;			//Описание звука
	char soundName[16];					//Имя звука
	SoundBankFileWaveInfo infos[256];	//Параметры волн, используемые в звуке
	SoundBankFileWave waves[256];		//Физическое описание волн
	UniqId previewWaves[256];			//Идентификаторы волн из проекта, задействованных в данной структуре
	SoundBankFileWaveQueue queue;		//Очередь для выбора звуков
	byte queueData[256*sizeof(SoundBankFileWaveQueue::Wave)];		
	ProjectSound * projectSound;
};

SoundPreviewExport::SoundPreviewExport()
{
	memset(&sound, 0, sizeof(sound));
	memset(soundName, 0, sizeof(soundName));
	memset(infos, 0, sizeof(infos));
	memset(waves, 0, sizeof(waves));
	memset(previewWaves, -1, sizeof(previewWaves));
	memset(&queue, 0, sizeof(queue));
	memset(queueData, 0, sizeof(queueData));
	projectSound = null;
}

bool SoundPreviewExport::Init(ProjectSound * snd)
{
#ifndef NO_TOOLS
	//Привязка к звуку
	projectSound = snd;
	Assert(projectSound->previewExport == this);
	//Получим параметры звука
	bool isWavesOk = false;
	for(dword i = 0; i < snd->waves.Size(); i++)
	{
		if(snd->waves[i].waveId.IsValidate())
		{
			if(project->ObjectFind(snd->waves[i].waveId, SndProject::pot_wave) != null)
			{
				isWavesOk = true;
			}else{
				isWavesOk = false;
				break;
			}
		}
	}
	const SoundBaseParams * sbp = snd->GetCurrentBaseParams();	
	const SoundAttGraph * atg = snd->GetCurrentAttenuation();
	if(!sbp || !atg || !isWavesOk)
	{
		delete this;
		return false;
	}
	float globalVolume = 1.0f;
	//SoundBankFileSound
	sound.waves = infos;
	sound.wavesCount = snd->waves.Size();
	sound.playSoundsCount = 0;
	sound.playSoundsList = null;
	sound.squeue = &queue;
	memcpy(soundName, snd->GetName().str, ARRSIZE(soundName));
	sound.name = soundName;
	sound.nameLen = 0;
	sound.nameHash = string::HashNoCase(sound.name, sound.nameLen);	
	sound.next = null;
	sound.extraData = null;
	sound.extraCount = 0;
	//SoundBankFileSetup
	long priority = sbp->priority + SoundBankFileSetup::mode_priority_base;
	Assert(priority < SoundBankFileSetup::mode_priority_base + SoundBankFileSetup::mode_priority_range);
	sound.setup.mode = priority;
	switch(sbp->selectMethod)
	{
	case sbpc_select_rnd:
		sound.setup.mode |= SoundBankFileSetup::mode_select_rnd;
		break;
	case sbpc_select_queue:
		sound.setup.mode |= SoundBankFileSetup::mode_select_queue;
		break;
	case sbpc_select_sequence:
		sound.setup.mode |= SoundBankFileSetup::mode_select_sequence;
		break;
	default:
		Assert(false);
	}
	switch(sbp->loopMode)
	{
	case sbpc_loop_disable:
		sound.setup.mode |= SoundBankFileSetup::mode_loop_diasble;
		break;
	case sbpc_loop_one_wave:
		sound.setup.mode |= SoundBankFileSetup::mode_loop_one_wave;
		break;
	default:
		Assert(false);
	}
	switch(sbp->fx)
	{
	case sbpc_fx_full:
		sound.setup.mode |= SoundBankFileSetup::mode_fx_full;
		break;
	case sbpc_fx_premaster:
		sound.setup.mode |= SoundBankFileSetup::mode_fx_premaster;
		break;
	case sbpc_fx_master:
		sound.setup.mode |= SoundBankFileSetup::mode_fx_master;
		break;
	case sbpc_fx_music:
		sound.setup.mode |= SoundBankFileSetup::mode_fx_music;
		break;
	}
	sound.setup.maxCount = sbp->maxCount;
	globalVolume = sbp->volume;
	//SoundBankFileAttenuation
	sound.att.c[0] = atg->c[0];
	sound.att.c[1] = atg->c[1];
	sound.att.c[2] = atg->c[2];
	sound.att.c[3] = atg->c[3];
	sound.att.minDist2 = atg->minDist*atg->minDist;
	sound.att.maxDist2 = atg->maxDist*atg->maxDist;
	sound.att.kNorm2 = 1.0f/(sound.att.maxDist2 - sound.att.minDist2);
	//SoundBankFileWaveInfo, SoundBankFileWave, previewWaves
	float sumWeight = 0.0f;
	for(dword i = 0; i < snd->waves.Size(); i++)
	{		
		ProjectSound::Wave & psw = snd->waves[i];
		previewWaves[i] = psw.waveId;
		//Вес для расчёта вероятностей
		sumWeight += psw.weight;
		//Волна
		ProjectWave * pw = (ProjectWave *)project->ObjectFind(snd->waves[i].waveId, SndProject::pot_wave);
		if(pw)
		{
			pw->LoadWaveData(ProjectWaveDebugId_soundpreview);
		}
		//Время проигрывания
		float playTime = psw.time;
		if(pw)
		{
			if(playTime < pw->GetPlayTime())
			{
				playTime = pw->GetPlayTime();			
			}
		}else{
			if(playTime < 0.0001f)
			{
				playTime = 0.0001f;
			}
		}
		Assert(playTime > 0.0f);
		//Заполняем информационную структуру
		infos[i].playTime = playTime;
		infos[i].volume = Clampf(globalVolume*psw.volume, SndExporter_MinExportVolume, SndExporter_MaxExportVolume);
		infos[i].probability = psw.weight;
		infos[i].wave = (pw != null) ? &waves[i] : null;
		//Заполняем описание волны
		if(pw)
		{
			waves[i].data = pw->GetWaveData(waves[i].dataSize);
			const WAVEFORMATEX & wfx = pw->GetFormat();
			waves[i].samplesCount = pw->GetSamplesCount();
			waves[i].format = SoundBankFileWave::f_format_pcm;
			Assert((wfx.nSamplesPerSec & SoundBankFileWave::f_freq_mask) == wfx.nSamplesPerSec);
			waves[i].format |= wfx.nSamplesPerSec;
			if(wfx.nChannels > 1) waves[i].format |= SoundBankFileWave::f_stereo;
			waves[i].waveFormatInfo = (const byte *)&pw->GetFormat();
			waves[i].extraData = null;
			waves[i].extraCount = 0;
		}
	}
	if(sumWeight > 0.00001f)
	{
		float kNorm = 1.0f/sumWeight;
		for(dword i = 0; i < snd->waves.Size(); i++)
		{
			infos[i].probability *= kNorm;
		}
	}else{
		float kNorm = 1.0f/float(snd->waves.Size());
		for(dword i = 0; i < snd->waves.Size(); i++)
		{
			infos[i].probability = kNorm;
		}
	}
	//SoundBankFileWaveQueue
	sound.InitSelectors();
	//Регестрируем звук в сервисе
	options->sa->EditRegistrySound(&sound);
	return true;
#else
	delete this;
	return false;
#endif
}

void SoundPreviewExport::UpdateParams()
{
#ifndef NO_TOOLS
	Assert(projectSound);
	float globalVolume = 1.0f;
	const SoundBaseParams * sbp = projectSound->GetCurrentBaseParams();
	if(sbp)
	{
		globalVolume = sbp->volume;
	}
	for(dword i = 0; i < sound.wavesCount; i++)
	{
		UniqId & wId = previewWaves[i];
		for(dword j = 0; j < projectSound->waves.Size(); j++)
		{
			ProjectSound::Wave & psw = projectSound->waves[i];
			if(psw.waveId.IsValidate() && psw.waveId == wId)
			{
				infos[i].volume = Clampf(psw.volume*globalVolume, SndExporter_MinExportVolume, SndExporter_MaxExportVolume);;
				//Время проигрывания
				ProjectWave * pw = (ProjectWave *)project->ObjectFind(wId, SndProject::pot_wave);
				float playTime = psw.time;
				if(pw)
				{
					if(playTime < pw->GetPlayTime())
					{
						playTime = pw->GetPlayTime();			
					}
				}else{
					if(playTime < 0.0001f)
					{
						playTime = 0.0001f;
					}
				}
				Assert(playTime > 0.0f);
				infos[i].playTime = playTime;
				break;
			}
		}
	}
	//Заставим сервис обновить громкости
	float curVolume = options->soundService->GetGlobalVolume();
	options->soundService->SetGlobalVolume(0.0f);
	options->soundService->SetGlobalVolume(curVolume);
#endif
}

bool SoundPreviewExport::Unload()
{
#ifndef NO_TOOLS
	if(options->sa->EditUnregistrySound(&sound))
	{
		delete this;
		return true;
	}
	return false;
#else
	return true;
#endif
}

SoundPreviewExport::~SoundPreviewExport()
{
	Assert(projectSound->previewExport == this);
	projectSound->previewExport = null;
	for(dword i = 0; i < sound.wavesCount; i++)
	{
		waves[i].data = null;
		waves[i].waveFormatInfo = null;
		ProjectWave * pw = project->WaveGet(previewWaves[i]);
		if(pw)
		{
			pw->UnloadWaveData(ProjectWaveDebugId_soundpreview);
		}
	}
}

ProjectSound::ProjectSound(const UniqId & folder) : ProjectObject(folder, c_namelen, ".txt"), waves(_FL_)
{
	previewExport = null;
	previewRefCounter = 0;
	previewDeleteTime = 0;
	isNeedRefreshPreview = false;
	ResetUpdateCounters();
	exportCounterWave = 0;
}

ProjectSound::~ProjectSound()
{
	if(previewExport)
	{
		previewExport->Unload();
		Assert(!previewExport);
		previewRefCounter = 0;
	}
}


//----------------------------------------------------------------------------------------
//Параметры звука
//----------------------------------------------------------------------------------------

//Установить ссылку на глобальные базовые параметры (для невалидной ссылки используються локальные параметры)
void ProjectSound::SetGlobalBaseParams(const UniqId & id)
{
	if(baseParamsId == id)
	{
		return;
	}
	baseParamsId = id;
	isNeedRefreshPreview = true;
	//------------------------------------------
	//А теперь ставим костыль, копируя установленный пресет в текущий, и сбрасывая его
	ProjectSoundBaseParams * sbpd = project->SndBaseParamsGet(baseParamsId);
	if(sbpd)
	{
		baseParams = sbpd->params;
	}
	baseParamsId.Reset();
	//------------------------------------------
	SetToSave();
}

//Получить ссылку на глобальные базовые параметры
const UniqId & ProjectSound::GetGlobalBaseParams()
{
	return baseParamsId;
}

//Получить ссылку на локальные базовые параметры
SoundBaseParams & ProjectSound::GetLocalBaseParams()
{
	return baseParams;
}

//Получить текущие параметры звука
const SoundBaseParams * ProjectSound::GetCurrentBaseParams()
{
	SoundBaseParams * sbp = null;	
	if(baseParamsId.IsValidate())
	{
		ProjectSoundBaseParams * sbpd = project->SndBaseParamsGet(baseParamsId);
		if(sbpd) sbp = &sbpd->params;
	}else{
		sbp = &baseParams;
	}
	return sbp;
}

//Установить ссылку на глобальный график затухания (для невалидной ссылки используються локальный график)
void ProjectSound::SetGlobalAttenuation(const UniqId & id)
{
	if(attenuationId == id)
	{
		return;
	}
	attenuationId = id;
	isNeedRefreshPreview = true;
	//------------------------------------------
	//А теперь ставим костыль, копируя установленный пресет в текущий, и сбрасывая его
	ProjectSoundAttGraph * atgd = project->AttGraphGet(attenuationId);
	if(atgd)
	{
		attenuation = atgd->attGraph;
	}
	attenuationId.Reset();
	//------------------------------------------
	SetToSave();
}

//Получить ссылку на глобальный график затухания
const UniqId & ProjectSound::GetGlobalAttenuation()
{
	return attenuationId;
}

//Получить локальный график затухания
SoundAttGraph & ProjectSound::GetLocalAttenuation()
{
	return attenuation;
}

//Получить текущий график затухания
const SoundAttGraph * ProjectSound::GetCurrentAttenuation()
{
	SoundAttGraph * atg = null;
	if(attenuationId.IsValidate())
	{
		ProjectSoundAttGraph * atgd = project->AttGraphGet(attenuationId);
		if(atgd) atg = &atgd->attGraph;
	}else{
		atg = &attenuation;
	}
	return atg;
}


//----------------------------------------------------------------------------------------
//Волны
//----------------------------------------------------------------------------------------

//Получить количество волн в звуке
long ProjectSound::GetWavesCount()
{
	return waves.Size();
}

//Получить волну
ProjectSound::Wave & ProjectSound::GetWave(long index)
{
	return waves[index];
}

//Добавить новые волны
bool ProjectSound::AddWaves(const UniqId * ptr, long count)
{
	for(long i = 0; i < count; i++)
	{
		if(waves.Size() >= c_maxwavescount)
		{
			return false;
		}
		const UniqId * waveId = &ptr[i];
		float initTime = 0.0f;
		if(waveId->IsValidate())
		{
			//Ищем попадания, если не тишина
			for(dword j = 0; j < waves.Size(); j++)
			{
				if(waves[j].waveId == *waveId)
				{
					break;
				}
			}
			if(j < waves.Size())
			{
				continue;
			}			
		}else{
			waveId = &UniqId::zeroId;
			initTime = 0.0f;
		}
		Wave & wave = waves[waves.Add()];
		wave.waveId = *waveId;
		wave.weight = 0.5f;
		wave.time = initTime;
		wave.volume = 1.0f;
	}
	return true;
}


//Удалить волны
void ProjectSound::DeleteWaves(long * indeces, dword count)
{
	Assert(indeces);
	Assert(count > 0);
	//Проверяем что значение которое используем как флажёк больше минимального
	for(dword i = 0; i < waves.Size(); i++)
	{
		Wave & w = waves[i];
		Assert(w.volume >= -10.0f);
	}
	//Маркируем удаляемых
	for(dword i = 0; i < count; i++)
	{
		waves[indeces[i]].volume = -110.0f;
	}
	//Удаляем маркированные
	for(dword i = 0; i < waves.Size(); )
	{
		if(waves[i].volume > -100.0f)
		{
			i++;
		}else{
			waves.DelIndex(i);
		}
	}	
}

//Коэфициент нормализации волн
float ProjectSound::GetKSumWeights()
{
	float sum = 0.0f;
	for(dword i = 0; i < waves.Size(); i++)
	{
		sum += waves[i].weight;
	}
	if(sum > 1e-8f)
	{
		sum = 1.0f/sum;
	}else{
		sum = 0.0f;
	}
	return sum;
}


//Получить минимальное и максимальные времена проигрывания
void ProjectSound::GetPlayTimes(float & minTime, float & maxTime)
{
	minTime = 0.0f;
	maxTime = 0.0f;
	bool isCompare = false;
	for(dword i = 0; i < waves.Size(); i++)
	{
		ProjectWave * wave = (ProjectWave *)project->ObjectFind(waves[i].waveId, SndProject::pot_wave);
		if(wave)
		{
			float playTime = wave->GetPlayTime();
			if(isCompare)
			{
				minTime = coremin(playTime, minTime);
				maxTime = coremax(playTime, maxTime);
			}else{
				minTime = playTime;
				maxTime = playTime;
			}
		}
	}
}


//----------------------------------------------------------------------------------------
//Прослушивание
//----------------------------------------------------------------------------------------

//Подготовить звук для прослушивания
bool ProjectSound::MakePreview()
{
	bool isForceRelease = isNeedRefreshPreview;
	//Обределяем, были ли изменены глобальные параметры
	ProjectSoundBaseParams * sbpd = project->SndBaseParamsGet(baseParamsId);	
	dword sbpRC = sbpd ? sbpd->GetUpdateCounter() : updateCounterSBP;
	ProjectSoundAttGraph * atgd = project->AttGraphGet(attenuationId);
	dword attRC = atgd ? atgd->GetUpdateCounter() : updateCounterAtt;
	if(sbpRC != updateCounterSBP || attRC != updateCounterAtt)
	{
		isForceRelease = true;
	}
	//Если нужно обновление, то отгружаем данные
	bool isForceUnload = false;
	if(isForceRelease && previewExport)
	{
		if(previewExport->Unload())
		{				
			Assert(!previewExport);
			isForceUnload = true;
		}
	}
	if(!previewExport)
	{
		Assert(isForceUnload || previewRefCounter == 0);
		previewDeleteTime = 0.0f;
		previewExport = NEW SoundPreviewExport();
		if(!previewExport->Init(this))
		{
			Assert(!previewExport);
			return false;
		}
		isNeedRefreshPreview = false;
		updateCounterSBP = sbpRC;
		updateCounterAtt = attRC;
	}
	previewRefCounter++;

	return true;
}

//Закончить прослушивание звука и освободить память
void ProjectSound::StopPreview()
{
	previewRefCounter--;
	Assert(previewRefCounter >= 0);
	if(previewRefCounter == 0)
	{
		previewDeleteTime = project->GetProjectTime();
	}
}

//Получить данные для обновления звука в сервисе
bool ProjectSound::GetPatchSoundData(EditPatchSoundData & patchData)
{
#ifndef NO_TOOLS
	memset(&patchData, 0, sizeof(EditPatchSoundData));
	const SoundBaseParams * sbp = GetCurrentBaseParams();
	const SoundAttGraph * atg = GetCurrentAttenuation();
	if(!sbp || !atg || waves.Size() == 0)
	{
		//Недостаточно параметров, не работает с этим звуком
		return false;
	}
	if(atg->maxDist <= atg->minDist)
	{
		return false;
	}
	if(waves.Size() > ARRSIZE(patchData.waves))
	{
		return false;
	}
	//Заполняем текущими данными звука
	Assert(sizeof(patchData.soundName) >= c_namelen);
	memcpy(patchData.soundName, GetName().str, c_namelen);
	patchData.nameHash = GetName().hash;
	patchData.nameLen = GetName().len;
	patchData.soundId[0] = GetId().data[0];
	patchData.soundId[1] = GetId().data[1];
	patchData.soundId[2] = GetId().data[2];
	patchData.soundId[3] = GetId().data[3];
	patchData.setupId[0] = baseParamsId.data[0];
	patchData.setupId[1] = baseParamsId.data[1];
	patchData.setupId[2] = baseParamsId.data[2];
	patchData.setupId[3] = baseParamsId.data[3];
	patchData.attId[0] = attenuationId.data[0];
	patchData.attId[1] = attenuationId.data[1];
	patchData.attId[2] = attenuationId.data[2];
	patchData.attId[3] = attenuationId.data[3];
	patchData.priority = sbp->priority;
	patchData.maxCount = sbp->maxCount;
	patchData.c[0] = atg->c[0];
	patchData.c[1] = atg->c[1];
	patchData.c[2] = atg->c[2];
	patchData.c[3] = atg->c[3];	
	patchData.minDist2 = atg->minDist*atg->minDist;
	patchData.maxDist2 = atg->maxDist*atg->maxDist;
	patchData.kNorm2 = 1.0f/(patchData.maxDist2 - patchData.minDist2);
	patchData.wavesCount = waves.Size();
	float kNorm = 0.0f;
	for(dword i = 0; i < waves.Size(); i++)
	{
		kNorm += waves[i].weight;
	}
	if(kNorm > 1e-5f)
	{
		kNorm = 1.0f/kNorm;
	}else{
		kNorm = 1.0f/waves.Size();
	}
	for(dword i = 0; i < patchData.wavesCount; i++)
	{
		EditPatchSoundData::WaveParams & waveParam = patchData.waves[i];
		Wave & wave = waves[i];
		waveParam.playTime = wave.time;
		float minTime = 0.0001f;
		ProjectWave * pwave = project->WaveGet(wave.waveId);
		if(pwave)
		{
			waveParam.waveId[0] = wave.waveId.data[0];
			waveParam.waveId[1] = wave.waveId.data[1];
			waveParam.waveId[2] = wave.waveId.data[2];
			waveParam.waveId[3] = wave.waveId.data[3];
			minTime = coremax(pwave->GetPlayTime(), minTime);
		}else{
			waveParam.waveId[0] = 0;
			waveParam.waveId[1] = 0;
			waveParam.waveId[2] = 0;
			waveParam.waveId[3] = 0;
		}
		waveParam.playTime = coremax(waveParam.playTime, minTime);
		waveParam.volume = Clampf(waves[i].volume*sbp->volume, 0.0f, 1.0f);
		if(kNorm > 0.0f)
		{
			waveParam.probability = wave.weight*kNorm;
		}else{
			waveParam.probability = kNorm;
		}
	}
	return true;
#else
	return false;
#endif
}

//Обновить параметры звука, которые возможно при проигрывании
void ProjectSound::UpdateExportParams()
{
	if(previewExport)
	{
		previewExport->UpdateParams();
	}
}


//----------------------------------------------------------------------------------------
//Методы для проекта
//----------------------------------------------------------------------------------------

//Обновление параметров волны во времени
bool ProjectSound::WorkUpdate()
{
	if(previewRefCounter == 0 && previewExport)
	{
		double delta = project->GetProjectTime() - previewDeleteTime;
		if(delta > 5.0*60.0)
		{
			//Начинаем пытаться отгружать звук, если больше не используеться то отгрузим
			if(previewExport->Unload())
			{				
				Assert(!previewExport);
				return true;
			}
		}
	}
	if(waves.Size() > 0)
	{
		dword loopCount = coremin(waves.Size(), 4);
		for(dword i = 0; i < loopCount; i++)
		{
			if(exportCounterWave >= waves.Size())
			{
				exportCounterWave = 0;
			}
			ProjectWave * wave = (ProjectWave *)project->ObjectFind(waves[exportCounterWave].waveId, SndProject::pot_wave);
			exportCounterWave++;
			if(wave)
			{
				const SoundBaseParams * sbp = GetCurrentBaseParams();
				if(sbp)
				{
					bool isAction = wave->IdleExport((sbp->modifiers & sbpc_mod_phoneme) != 0);
					if(isAction)
					{
						//Было ресурсоёмкое действие, прерываемся
						return true;
					}
				}
			}			
		}
	}	
	return false;
}

//Установить на обноврение предпросмотр
void ProjectSound::ResetUpdateCounters()
{
	updateCounterSBP = 1;
	updateCounterAtt = 1;
}

//----------------------------------------------------------------------------------------
//События удаления глобальных данных
//----------------------------------------------------------------------------------------


//Событие удаление волны их проекта
void ProjectSound::OnDeleteWave(const UniqId & waveId)
{
	//Удаляем из таблицы волн
	bool isDelete = false;
	for(dword i = 0; i < waves.Size(); )
	{
		if(waves[i].waveId != waveId)
		{
			i++;
		}else{
			isDelete = true;
			waves.DelIndex(i);
		}
	}
	if(isDelete)
	{
		//Удаляем из экспортной части
		if(previewExport)
		{
			//TODO: Будет зависать, надо будет копировать данные волны к себе
			while(!previewExport->Unload())
			{
				Sleep(10);
			}
			Assert(!previewExport);
		}
	}
	if(isDelete)
	{
		SetToSave();
	}
}

//Событие изменения данных волны
void ProjectSound::OnReplaceWave(const UniqId & waveId)
{
	bool isFind = false;
	for(dword i = 0; i < waves.Size(); i++)
	{
		if(waves[i].waveId == waveId)
		{
			//Удаляем из экспортной части
			if(previewExport)
			{
				//TODO: Будет зависать, надо будет копировать данные волны к себе
				while(!previewExport->Unload())
				{
					Sleep(10);
				}
				Assert(!previewExport);
			}
			return;
		}
	}
}



//Событие удаление волны их проекта
void ProjectSound::OnDeleteSoundParams(const UniqId & waveId)
{

}

//Событие удаление волны их проекта
void ProjectSound::OnDeleteAttenuation(const UniqId & waveId)
{

}

//----------------------------------------------------------------------------------------
//ProjectObject
//----------------------------------------------------------------------------------------

//Сохранить объект
ErrorId ProjectSound::OnSaveObject(const char * defaultFilePath)
{
	isNeedRefreshPreview = true;
	IEditableIniFile * ini = project->FileOpenIni(defaultFilePath,false, false, _FL_);
	if(!ini)
	{
		return options->ErrorOut(null, true, "ProjectSound error: can't open file \"%s\"", defaultFilePath);
	}
	ini->SetString("Sound", "id", GetId().ToString());
	ini->SetString("Sound", "setupid", baseParamsId.ToString());
	ini->SetString("Sound", "attenuationid", attenuationId.ToString());
	ini->SetLong("Sound", "wavescount", waves.Size());
	baseParams.SaveToIni(ini);
	attenuation.SaveToIni(ini);
	for(dword i = 0; i < waves.Size(); i++)
	{
		char section[32];
		crt_snprintf(section, sizeof(section), "wave_%i", i);
		Wave & wave = waves[i];
		ini->SetString(section, "id", wave.waveId.ToString());
		ini->SetFloat(section, "weight", wave.weight);
		ini->SetFloat(section, "time", wave.time);
		ini->SetFloat(section, "volume", wave.volume);
	}
	project->FileCloseIni(ini);
	return ErrorId::ok;
}

//Загрузить из файла
ErrorId ProjectSound::OnLoadObject(const char * defaultFilePath)
{
	IEditableIniFile * ini = project->FileOpenIni(defaultFilePath, false, true, _FL_);
	if(!ini)
	{
		return options->ErrorOut(null, true, "ProjectSound error: can't open file \"%s\"", defaultFilePath);
	}
	//С невалидным именем загрузка невозможна
	Assert(GetName().len < c_namelen);	
	//Параметры звука
	const char * s = ini->GetString("Sound", "id", null);
	UniqId soundId;	
	soundId.FromString(s);
	if(!soundId.IsValidate())
	{
		project->FileCloseIni(ini);
		return options->ErrorOut(null, true, "ProjectSound error: setup id is invalidate. File \"%s\"", defaultFilePath);
	}
	ReplaceId(soundId);
	if(project->ObjectFind(baseParamsId, SndProject::pot_sound) != null)
	{		
		project->FileCloseIni(ini);
		return options->ErrorOut(null, true, "ProjectSound error: sound id (%s) is repeat. File \"%s\"", s, defaultFilePath);
	}
	s = ini->GetString("Sound", "setupid", null);
	baseParamsId.FromString(s);
	s = ini->GetString("Sound", "attenuationid");
	attenuationId.FromString(s);
	baseParams.LoadFromIni(ini);
	attenuation.LoadFromIni(ini);
	//Волны
	long wavesCount = ini->GetLong("Sound", "wavescount", 0);
	wavesCount = MinMax<long>(0, c_maxwavescount, wavesCount);
	waves.DelAll();
	waves.AddElements(wavesCount);
	for(dword i = 0; i < waves.Size(); i++)
	{
		char section[32];
		crt_snprintf(section, sizeof(section), "wave_%i", i);
		Wave & wave = waves[i];
		s = ini->GetString(section, "id", null);
		wave.waveId.FromString(s);
		wave.weight = ini->GetFloat(section, "weight", 0.5f);
		wave.weight = MinMax(0.0f, 1.0f, wave.weight);
		wave.time = ini->GetFloat(section, "time", 0.0f);
		wave.time = MinMax(0.0f, 1000000.0f, wave.time);
		wave.volume = ini->GetFloat(section, "volume", 1.0f);
		wave.volume = MinMax(0.0f, 1.0f, wave.volume);
	}
	project->FileCloseIni(ini);
	if(baseParamsId.IsValidate())
	{
		UniqId setId = baseParamsId;
		baseParamsId.Reset();
		SetGlobalBaseParams(setId);
	}
	if(attenuationId.IsValidate())
	{
		UniqId setId = attenuationId;
		attenuationId.Reset();
		SetGlobalAttenuation(setId);
	}
	return ErrorId::ok;
}


