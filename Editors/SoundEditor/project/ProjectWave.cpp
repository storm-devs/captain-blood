/*
	DataChunk
	RAW wave data (16 bit, mono or stereo)
	Exported wave data block
	Exported phonemes data block
*/




#include "ProjectWave.h"
#include "..\SndOptions.h"
#include "ProjectWaveExporter.h"

#pragma comment(lib, "Winmm")

//Сколько волн прогружено всего
long ProjectWave::totalWaveDataLoad = 0;

ProjectWave::ProjectWave(const UniqId & folderId) : ProjectObject(folderId, c_namelen, ".pwv", true)
#ifdef ProjectWaveDebugEnable
, flDebug(_FL_)
#endif
{
	memset(&data, 0, sizeof(data));
	memset(playTimeData, 0, sizeof(playTimeData));
	waveDataRefCounter = 0;
	lastUnloadTime = -1.0f;
	isWaveLoadForPlay = false;
	inExportQueue = 0;
	memset(&data.exportOptionsPC, 0, sizeof(data.exportOptionsPC));
	memset(&data.exportOptionsXBOX, 0, sizeof(data.exportOptionsXBOX));	
	rawWaveDataPtr = null;
	waveData = null;
	pcPreviewData = null;
	xboxPreviewData = null;
	memset(textId, 0, sizeof(textId));
}

ProjectWave::~ProjectWave()
{
	Stop();
	ProjectWaveExporter::DelWaveFromQueue(this);
	if(rawWaveDataPtr)
	{
		delete rawWaveDataPtr;
		rawWaveDataPtr = null;
		waveData = null;
	}
	if(pcPreviewData)
	{
		delete pcPreviewData;
		pcPreviewData = null;
	}
	if(xboxPreviewData)
	{
		delete xboxPreviewData;
		xboxPreviewData = null;
	}
}


//----------------------------------------------------------------------------------------
//Доступ к данным волны
//----------------------------------------------------------------------------------------

//Загрузим данные волны
void ProjectWave::LoadWaveData(long id, const char * cppFile)
{
	//Путь до файла
	string workPath;
	BuildFilePath(workPath);
	//Позволим в этой функции присутствовать только одному потоку	
	SyncroCode sc(waveDataAccessor);
#ifdef ProjectWaveDebugEnable
	flDebug.Add(FL_Debug(cppFile, id));
#endif
	if(waveDataRefCounter > 0)
	{
		waveDataRefCounter++;
		return;
	}
	Assert(data.dataSize > 0);
	if(rawWaveDataPtr)
	{
		delete rawWaveDataPtr;
		rawWaveDataPtr = null;
		waveData = null;
	}
	rawWaveDataPtr = NEW byte[data.dataSize + 32];
	waveData = AlignPtr(rawWaveDataPtr);
	IFile * file = project->FileOpenBin(workPath.c_str(), SndProject::poff_isWaitWhenOpen | SndProject::poff_isOpenForRead, null, _FL_, 104);
	if(!file)
	{
		api->Trace("Can't open wava data file: %s", workPath.c_str());
		Verify(false);
		//project->FileOpenBin(workPath.c_str(), SndProject::poff_isWaitWhenOpen | SndProject::poff_isOpenForRead, null, _FL_, 104);
	}
	Verify(file->SetPos(sizeof(data)) == sizeof(data));
	Verify(file->Read(waveData, data.dataSize) == data.dataSize);
	project->FileCloseBin(file, 104);
	waveDataRefCounter = 1;	
	lastUnloadTime = -1.0f;	
	totalWaveDataLoad++;
}

//выгрузить данные волны
void ProjectWave::UnloadWaveData(long id, const char * cppFile)
{
	SyncroCode sc(waveDataAccessor);
	waveDataRefCounter--;
	Assert(waveDataRefCounter >= 0);
#ifdef ProjectWaveDebugEnable
	for(dword i = 0; i < flDebug.Size(); i++)
	{
		if(flDebug[i].cppId == id && flDebug[i].cppFile == cppFile)
		{
			flDebug.DelIndex(i);
			break;
		}			
	}	
#endif
}

//Получить описание дефектов
void ProjectWave::GetDescription(string & text, string & hint)
{
	bool isError = false;
	text = GetName().str;
	hint = "Wave name: ";
	hint += GetName().str;
	if(data.waveFormat.nChannels == 1)
	{
		hint += "\nMono, time is ";
		text += ", mono, time is ";
	}else{
		hint += "\nStereo, time is ";
		text += ", stereo, time is ";
	}
	char tmpBuf[20];
	crt_snprintf(tmpBuf, sizeof(tmpBuf), "%.3fs", data.samplesCount*(1.0/44100.0));
	hint += tmpBuf;
	text += tmpBuf;	
	text += ", UT: ";
	hint += "\nUnimportant time of tail is ";
	crt_snprintf(tmpBuf, sizeof(tmpBuf), "%.3fs", data.unimportantTime);	
	text += tmpBuf;	
	hint += tmpBuf;
	hint += ".";
	dword amp = (data.defects & df_amplitude_mask) >> df_amplitude_shift;	
	text += ", PA: ";
	hint += "\nWave pick amplitude is ";
	text += (long)amp;
	hint += (long)amp;
	text += "%%";
	hint += "%%.";
	if(!(data.defects & df_ignore_amp) && amp < df_min_amplitude)
	{
		hint += " Error! Minimum pick value is ";
		text += " (min: ";
		text += (long)df_min_amplitude;
		hint += (long)df_min_amplitude;
		text += "%%)";
		hint += "%%.";
		isError = true;
	}else{
		if(amp < df_min_amplitude)
		{
			hint += " Ignored.";
		}else{
			hint += " It's ok.";			
		}
	}
	dword middle = (data.defects & df_middle_mask) >> df_middle_shift;
	crt_snprintf(tmpBuf, sizeof(tmpBuf), middle <= 100 ? "%.1f" : "10+", middle*0.1f);
	text += ", MA: ";
	hint += "\nMiddle amplitude (energy) is ";
	text += tmpBuf;
	hint += tmpBuf;
	text += "%%";
	hint += "%%.";
	if(!(data.defects & df_ignore_mid) && middle < df_min_middle)
	{
		hint += " Error! Minimum middle value is ";
		text += " (min: ";
		crt_snprintf(tmpBuf, sizeof(tmpBuf), "%.1f", df_min_middle*0.1f);
		text += tmpBuf;
		hint += tmpBuf;
		text += "%%)";
		hint += "%%.";
		isError = true;
	}else{
		if(middle < df_min_middle)
		{
			hint += " Ignored.";
		}else{
			hint += " It's ok.";
		}
	}
	dword sln = (data.defects & df_silence_mask) >> df_silence_shift;
	if(sln > 0)
	{
		hint += "\nSilence filling is ";
		text += ", SF: ";
		text += (long)sln;
		hint += (long)sln;
		text += "%%";
		hint += "%%.";
		if(!(data.defects & df_ignore_sln) && sln > df_max_silence)
		{
			hint += " Error! Maximum silence filling can be is ";
			text += " (max: ";
			text += (long)df_max_silence;
			hint += (long)df_max_silence;
			text += "%%)";
			hint += "%%.";
			isError = true;
		}else{
			if(sln > df_max_silence)
			{
				hint += " Ignored.";
			}else{
				hint += " It's ok.";
			}
		}
	}
	if(!(data.defects & df_ignore_clk) && data.defects & df_clicks)
	{
		hint += "\nWave content clicks! Error!";
		text += ", detect clicks";
		isError = true;
	}else{
		if(data.defects & df_clicks)
		{
			hint += "\nWave content clicks. Ignored.";
		}
	}
	if(data.defects & df_size_align)
	{
		hint += "Warning! Wave is old export time and have got not granular size.\nFor fix size problem, need reimport wave to progect";
		text += ", size non granular.";
		isError = true;
	}
	if(isError)
	{
		hint += "\n\nPlease fix problems and reimport wave.";
	}
}

//Установить заметки по волне
void ProjectWave::SetWaveNotes(const char * newNotes)
{
	if(string::IsEqual(newNotes, data.note)) return;
	memset(data.note, 0, sizeof(data.note));
	crt_strncpy(data.note, sizeof(data.note) - 1, newNotes, sizeof(data.note) - 1);
	data.note[sizeof(data.note) - 1] = 0;
	SetToSave();
}

//Сохранить данные волны на диске
void ProjectWave::SaveWaveToWAV(const char * path)
{
	LoadWaveData(ProjectWaveDebugId_SaveToWav);
	ProjectWaveExporter::SaveWave(path, waveData, data.samplesCount, data.waveFormat.nChannels, data.waveFormat.nSamplesPerSec);
	UnloadWaveData(ProjectWaveDebugId_SaveToWav);
}


//----------------------------------------------------------------------------------------
//Доступ к данным экспорта и параметрам экспорта
//----------------------------------------------------------------------------------------

//Получить подготовленные данные волны
bool ProjectWave::GetExportData(array<byte> * buffer, bool dontStartExport, bool isPhonemes)
{
	AssertCoreThread
	if(buffer) buffer->Empty();
	//Если в очереди, то ничего не пытаемся делать
	if(inExportQueue)
	{
		Sleep(0);
		return false;
	}
	//Открываем файл
	BuildExportPath(options->getExportDataTmp, isPhonemes);
	IFile * file = project->FileOpenBin(options->getExportDataTmp.c_str(), SndProject::poff_noflags, null, _FL_, 105);
	if(!file)
	{
		//Если нет файла то по необходимости запускаем экспорт
		if(!dontStartExport)
		{
			//Запускаем экспорт
			ProjectWaveExporter::AddWaveToQueue(this, isPhonemes, false);
		}
		Sleep(0);
		return false;
	}
	//Зачитываем общий заголовок	
	ExportWavesPart exportWaveHeader;
	ExportPhonemesPart exportPhonemesHeader;
	ExportTemporaryFile * fileHeaderPtr = null;// = isPhonemes ? &exportPhonemesHeader : &exportWaveHeader;
	if(!isPhonemes)
	{
		fileHeaderPtr = &exportWaveHeader;		
	}else{
		fileHeaderPtr = &exportPhonemesHeader;
	}
	ExportTemporaryFile & fileHeader = *fileHeaderPtr;
	dword headerSize = isPhonemes ? sizeof(ExportPhonemesPart) : sizeof(ExportWavesPart);
	fileHeader.size = 0;
	bool res = (file->Read(&fileHeader, headerSize) == headerSize);
	//Проверяем на корректность содержимое
	if(res)
	{
		res = false;
		dword fileSize = file->Size();
		if(fileHeader.size == fileSize && fileHeader.waveCheckSum == data.waveCheckSum && fileHeader.waveDataSize == data.dataSize)
		{
			if(!isPhonemes)
			{
				//Дополнительная проверка на формат экспорта
				if(data.exportOptionsPC.IsEqual(exportWaveHeader.exportOptsPC) && data.exportOptionsXBOX.IsEqual(exportWaveHeader.exportOptsXbox))
				{
					res = true;
				}
			}else{
				res = true;
			}
		}
	}	
	Assert(file);
	if(res)
	{
		//Если читать в буфер данные не надо, то возвращаем результат сразу
		if(!buffer)
		{
			project->FileCloseBin(file, 105);
			return true;
		}
		//Резервируем буфер для файла
		buffer->AddElements(fileHeader.size);
		//Копируем прочитанный заголовок
		byte * buf = buffer->GetBuffer();
		memcpy(buf, &fileHeader, headerSize);
		buf += headerSize;
		//Дочитываем данные
		long needSize = fileHeader.size - headerSize;
		if(needSize > 0)
		{		
			if(file->Read(buf, needSize) == needSize)
			{
				project->FileCloseBin(file, 105);
				return true;
			}
		}else{
			project->FileCloseBin(file, 105);
			return true;
		}
	}
	//Файл устарел или недоступен, надо выгружать заново
	project->FileCloseBin(file, 105);
	file = null;
	::DeleteFile(options->getExportDataTmp.c_str());
	if(!dontStartExport)
	{
		//Запускаем экспорт
		ProjectWaveExporter::AddWaveToQueue(this, isPhonemes, false);
	}
	Sleep(0);
	return false;
}

//Установить формат экспорта
void ProjectWave::SetExportOptions(dword platform, const WaveOptions & opts)
{
	ProjectWaveExporter::DelWaveFromQueue(this);
	Stop();
	optsAccessor.Enter();
	switch(platform)
	{
	case wce_platform_pc:
		data.exportOptionsPC = opts;
		data.exportOptionsPC.platform = wce_platform_pc;
		if(pcPreviewData)
		{
			delete pcPreviewData;
			pcPreviewData = null;
		}
		break;
	case wce_platform_xbox:
		data.exportOptionsXBOX = opts;
		data.exportOptionsPC.platform = wce_platform_xbox;
		if(xboxPreviewData)
		{
			delete xboxPreviewData;
			xboxPreviewData = null;
		}
		break;
	default:
		Assert(false);
	}
	optsAccessor.Leave();
}

//Получить формат экспорта
const ProjectWave::WaveOptions & ProjectWave::GetExportOptions(dword platform)
{
	SyncroCode sc(optsAccessor);
	switch(platform)
	{
	case wce_platform_pc:
		return data.exportOptionsPC;
	case wce_platform_xbox:
		return data.exportOptionsXBOX;
	}
	Assert(false);
	return *(ProjectWave::WaveOptions *)null;
}


//Остановить экспорт, удалить выгруженные данные
void ProjectWave::DeleteExportData()
{
	//Останавливаем экспорт
	AssertCoreThread
	ProjectWaveExporter::DelWaveFromQueue(this);
	//Удаляем экспорчёные данные
	string filePath, buffer;
	BuildExportPath(filePath, false);
	::DeleteFile(filePath.c_str());
	BuildExportPath(filePath, true);
	::DeleteFile(filePath.c_str());
}


//----------------------------------------------------------------------------------------
//Прослушивание волны
//----------------------------------------------------------------------------------------

//Проиграть волну, чтобы послушать
bool ProjectWave::Play()
{
#ifndef NO_TOOLS
	options->WavePreviewStop();
	Assert(GetId().IsValidate());
	if(!isWaveLoadForPlay)
	{
		LoadWaveData(ProjectWaveDebugId_play);
		isWaveLoadForPlay = true;
	}
	EditPrewiewWaveParams params;
	params.data = waveData;
	params.dataSize = data.dataSize;
	params.sampleRate = data.waveFormat.nSamplesPerSec;
	params.isStereo = data.waveFormat.nChannels > 1;
	bool res = options->sa->EditPrewiewPlay(GetId().data, &params, null, false);
	return res;
#else
	return false;
#endif
}

//Проиграть волну, чтобы послушать разницу
bool ProjectWave::PlayExported(dword wce_platform, bool isOriginal, ErrorId & errorCode)
{	
#ifndef NO_TOOLS
	options->WavePreviewStop();
	errorCode = options->ErrorOut(null, false, "Preview export error: ");	
	Assert(GetId().IsValidate());
	//Получаем экспортные данные волны в формате PCM
	EditPrewiewWaveParams mirror;
	memset(&mirror, 0, sizeof(mirror));
	if(wce_platform == wce_platform_pc)
	{
		if(!pcPreviewData)
		{
			pcPreviewData = ProjectWaveExporter::WavePreviewMake(this, true, errorCode);
			if(!pcPreviewData)
			{
				errorCode = options->ErrorOut(null, true, "");
				return false;
			}
		}
		mirror.data = pcPreviewData;
	}else
		if(wce_platform == wce_platform_xbox)
		{
			if(!xboxPreviewData)
			{
				xboxPreviewData = ProjectWaveExporter::WavePreviewMake(this, false, errorCode);
				if(!xboxPreviewData)
				{
					errorCode = options->ErrorOut(null, true, "");
					return false;
				}
			}
			mirror.data = xboxPreviewData;
		}else{
			Assert(false);
			options->ErrorCancel(errorCode);
			errorCode.SetOk();
			return false;
		}
		Assert(mirror.data);
		//Получаем параметры экспортной волны
		WAVEFORMATEX * format = ProjectWaveExporter::WavePreviewGetFormat(mirror.data);
		Assert(format);
		Assert(format->wBitsPerSample == 16);
		mirror.sampleRate = format->nSamplesPerSec;
		mirror.isStereo = format->nChannels > 1;
		mirror.dataSize = 0;
		mirror.data = ProjectWaveExporter::WavePreviewGetData(mirror.data, mirror.dataSize);
		Assert(mirror.data);
		//Запускаем на проигрывание волну
		if(!isWaveLoadForPlay)
		{
			LoadWaveData(ProjectWaveDebugId_play);
			isWaveLoadForPlay = true;
		}
		EditPrewiewWaveParams params;
		params.data = waveData;
		params.dataSize = data.dataSize;
		params.sampleRate = data.waveFormat.nSamplesPerSec;
		params.isStereo = data.waveFormat.nChannels > 1;
		bool res = options->sa->EditPrewiewPlay(GetId().data, &params, &mirror, !isOriginal);	
		if(!res)
		{
			options->ErrorOut(&errorCode, true, "Sound engine can't start wave play");
		}else{
			options->ErrorCancel(errorCode);
			errorCode.SetOk();
		}
		return res;
#else
	return false;
#endif
}

//После запуска предпросмотра можно получить размер данных
dword ProjectWave::GetExportedWaveSize(dword platform)
{
	byte * data = null;
	if(platform == wce_platform_pc)
	{
		data = pcPreviewData;
	}else
		if(platform == wce_platform_xbox)
		{
			data = xboxPreviewData;
		}else{
			Assert(false);
		}
		return ProjectWaveExporter::WavePreviewGetDataSize(data);
}


//Переключить прослушивание на оригинальную волну или экспорчёную
void ProjectWave::PlaySwitch(bool isOriginal)
{
#ifndef NO_TOOLS
	Assert(GetId().IsValidate());
	options->sa->EditPrewiewSwitch(GetId().data, !isOriginal);
#endif
}

//Остановить прослушивание волны
void ProjectWave::Stop()
{
#ifndef NO_TOOLS
	Assert(GetId().IsValidate());
	options->sa->EditPrewiewStop(GetId().data);
	if(isWaveLoadForPlay)
	{
		isWaveLoadForPlay = false;
		UnloadWaveData(ProjectWaveDebugId_play);
	}
#endif
}

//Прослушиваеться ли волна
bool ProjectWave::IsPlay()
{
#ifndef NO_TOOLS
	Assert(GetId().IsValidate());
	return options->sa->EditPrewiewIsPlay(GetId().data);
#else
	return false;
#endif
}

//Прослушиваеться ли волна
bool ProjectWave::IsPlay(float & curPosition)
{
#ifndef NO_TOOLS
	Assert(GetId().IsValidate());
	dword curPos = 0;
	if(options->sa->EditPrewiewIsPlay(GetId().data, &curPos))
	{
		curPosition = curPos/float(data.samplesCount);
		curPosition = Clampf(curPosition, 0.0f, 1.0f);
		return true;
	}
#endif
	curPosition = 0.0f;
	return false;
}

//Прослушиваеться ли волна вместе с экспорчёной
bool ProjectWave::IsPlayMirror()
{
#ifndef NO_TOOLS
	Assert(GetId().IsValidate());
	bool isMirror = false;
	if(options->sa->EditPrewiewIsPlay(GetId().data, null, &isMirror))
	{
		return isMirror;
	}
	return false;
#else
	return false;
#endif
}

//Установить громкость прослушивания
void ProjectWave::PlayVolume(float vol)
{
#ifndef NO_TOOLS
	Assert(GetId().IsValidate());
	options->sa->EditPrewiewSetVolume(vol, GetId().data);
#endif
}


//----------------------------------------------------------------------------------------
//Методы для проекта
//----------------------------------------------------------------------------------------

//Отложеная работа
void ProjectWave::WorkUpdate(bool deleteNow)
{
	AssertCoreThread
	//Смотрим необходимость выгрузки данных
	SyncroCode sc(waveDataAccessor);	
	Assert(waveDataRefCounter >= 0);
	if(isWaveLoadForPlay)
	{
		if(!IsPlay())
		{
			waveDataRefCounter--;
			Assert(waveDataRefCounter >= 0);
			isWaveLoadForPlay = false;
		}
	}
	if(waveData != null)
	{
		if(waveDataRefCounter == 0)
		{
			if(lastUnloadTime >= 0.0f || deleteNow)
			{
				//Определим фактор времени удержания волны в памети в зависимости от количества
				Assert(totalWaveDataLoad > 0);
				double x = double(totalWaveDataLoad)*1.0/5.0;	//До 5 штук влияние на время выгрузки минимально
				double k = 1.0 - pow(x, 3.5)*0.01;
				if(k < 0.05) k = 0.05;							//Всегда оставляем 5% от общего времени			
				double waitTime = 10.0f*60.0f*k;				//Большее время удержание неиспользуемой волны 10 минут
				double delta = project->GetProjectTime() - lastUnloadTime;			
				if(delta < 0.0f || delta > waitTime || deleteNow)
				{				
					delete rawWaveDataPtr;
					rawWaveDataPtr = null;
					waveData = null;
					totalWaveDataLoad--;
				}
			}else{
				//Начнём отсчитывать время ожидания отгрузки
				lastUnloadTime = project->GetProjectTime();
			}
		}else{
			if(deleteNow)
			{
#ifdef ProjectWaveDebugEnable
				for(dword i = 0; i < flDebug.Size(); i++)
				{
					api->Trace("Unrelease wave data (lock id: %i): %s from %s", flDebug[i].cppId, GetName().str, flDebug[i].cppFile);
				}
#endif
			}
		}
	}else{
		Assert(waveDataRefCounter == 0);
	}
}

//Фоновый экспорт
bool ProjectWave::IdleExport(bool isPhonemes)
{
	if(options->policy == policy_sounder)
	{
		return false;
	}
	AssertCoreThread
	//При необходимости экспортим в фоне данные
	if(GetExportData(null, true, false))
	{
		if(isPhonemes)
		{
			if(GetExportData(null, true, true))
			{
				return true;
			}
		}		
	}
	//Поставим на экспорт
	if(!inExportQueue)
	{
		ProjectWaveExporter::AddWaveToQueue(this, isPhonemes, true);
		return true;
	}
	return false;
}

//Импортировать волну
ErrorId ProjectWave::Import(const char * path, ProjectWave * replaceWave)
{
	SyncroCode sc(waveDataAccessor);
	AssertCoreThread
	ProjectWaveExporter::DelWaveFromQueue(this);
	//Очищаем буфера
	if(rawWaveDataPtr)
	{
		delete rawWaveDataPtr;
		rawWaveDataPtr = null;
		waveData = null;
	}	
	array<byte> pcmWFBuffer(_FL_);
	memset(&data, 0, sizeof(data));
	//Сохраняем путь, откуда брали
	crt_strncpy(data.sourcePath, sizeof(data.sourcePath) - 1, path, sizeof(data.sourcePath) - 2);
	//Пытаемся открыть файл
	HMMIO hmmio = mmioOpen((LPSTR)path, NULL, MMIO_ALLOCBUF | MMIO_READ);
	if(hmmio == null)
	{
		return options->ErrorOut(null, true, "Error wave import: Can't open file \"%s\"", path);
	}
	//Читаем содержимое
	MMCKINFO ckRiff;
	if(mmioDescend(hmmio, &ckRiff, NULL, 0) != MMSYSERR_NOERROR)
	{
		mmioClose(hmmio, 0);
		return options->ErrorOut(null, true, "Error wave import: File \"%s\" is no RIFF format", path);		
	}
	if(ckRiff.ckid != FOURCC_RIFF || ckRiff.fccType != mmioFOURCC('W', 'A', 'V', 'E'))
	{
		mmioClose(hmmio, 0);
		return options->ErrorOut(null, true, "Error wave import: File \"%s\" is no WAVE file", path);		
	}
	//Формат файла
	if(!ReadMMIOFormat(hmmio, ckRiff, pcmWFBuffer))
	{
		mmioClose(hmmio, 0);
		return options->ErrorOut(null, true, "Error wave import: File \"%s\" is corrupt", path);
	}
	//Исходники загружаем только в достаточно избыточных форматах (16bit, mono-stereo, 44100-48000)
	WAVEFORMATEX * curWFormat = (WAVEFORMATEX *)pcmWFBuffer.GetBuffer();
	if(curWFormat->wFormatTag != WAVE_FORMAT_PCM)
	{
		mmioClose(hmmio, 0);
		return options->ErrorOut(null, true, "Error wave import: editor support only PCM files. File \"%s\"", path);
	}
	if(curWFormat->wBitsPerSample != 16)
	{
		mmioClose(hmmio, 0);
		pcmWFBuffer.Empty();
		return options->ErrorOut(null, true, "Error wave import: editor support only 16 bit per sample. File \"%s\"", path);
	}
	if(curWFormat->nSamplesPerSec != 44100)
	{
		mmioClose(hmmio, 0);
		pcmWFBuffer.Empty();
		return options->ErrorOut(null, true, "Error wave import: editor support only 44100 Hz sample rate. File \"%s\"", path);
	}
	if(curWFormat->nChannels != 1 && curWFormat->nChannels != 2)
	{
		mmioClose(hmmio, 0);
		return options->ErrorOut(null, true, "Error wave import: editor support only mono or stereo wave file. File \"%s\"", path);
	}
	//Читаем данные файла
	ErrorId readError = ReadMMIOData(hmmio, ckRiff, curWFormat->nChannels != 1, path);
	if(readError.IsError())
	{
		mmioClose(hmmio, 0);
		if(rawWaveDataPtr)
		{
			delete rawWaveDataPtr;
			rawWaveDataPtr = null;
			waveData = null;
		}
		return readError;
	}
	//Ищим метки цикла
	//ReadLoopLabels(hmmio, ckRiff);
	//Закрываем файл
	mmioClose(hmmio, 0);
	//Структура формата файла без лишней информации
	data.waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	data.waveFormat.nChannels = curWFormat->nChannels;
	data.waveFormat.nSamplesPerSec = curWFormat->nSamplesPerSec;
	data.waveFormat.wBitsPerSample = 16;
	data.waveFormat.nBlockAlign = (data.waveFormat.nChannels*data.waveFormat.wBitsPerSample)/8;
	data.waveFormat.nAvgBytesPerSec = data.waveFormat.nBlockAlign*data.waveFormat.nSamplesPerSec;	
	data.waveFormat.cbSize = 0;
	//Общая информация по файлу
	data.bytesPerSample = (curWFormat->wBitsPerSample*curWFormat->nChannels)/8;
	Assert(data.bytesPerSample > 0);
	data.samplesCount = data.dataSize/data.bytesPerSample;
	Assert(data.samplesCount >= wave_minsize);				//Допустимый минимальный размер
	Assert(data.samplesCount % wave_granular_align == 0);	//Количество сэмплов гранулированно
	data.playTime = float(double(data.samplesCount)/double(data.waveFormat.nSamplesPerSec));
	//Контрольная сумма
	data.waveCheckSum = 0;
	for(dword i = 0; i < data.dataSize; i++)
	{
		data.waveCheckSum += (dword)waveData[i];
	}
	//Данные быстрого предпросмотра
	if(data.playTime > 0.0f)
	{
		data.hotPreviewStep = (data.samplesCount + 63)/64;
		/*
		if(data.playTime < 1.0f)
		{
			double modifySamplesPerUnit = double(data.hotPreviewStep)/data.playTime;
			if(modifySamplesPerUnit > data.samplesCount)
			{
				modifySamplesPerUnit = (double)data.samplesCount;
			}
			data.hotPreviewStep = dword(modifySamplesPerUnit);
		}*/
		//Собираем амплитуды участков
		long minValue = 0, maxValue = 0;
		dword c = 0, i = 0;
		data.hotPreview[0].x = 0;
		data.hotPreview[0].y = 8;
		data.hotPreviewCount = 1;
		dword channels = data.waveFormat.nChannels;
		while(true)
		{
			if(i < data.hotPreviewStep)
			{
				//Смотрим размах амплитуд
				if(c < data.samplesCount)
				{
					short * offset = (short *)&waveData[c*data.bytesPerSample];					
					for(dword j = 0; j < channels; j++)
					{
						if(minValue > offset[j]) minValue = offset[j];
						if(maxValue < offset[j]) maxValue = offset[j];
					}
				}
				c++;
				i++;
			}else{
				//Добавляем точку
				if(maxValue < -minValue) maxValue = -minValue;
				long y = maxValue/(32768/8);
				if(y > 7) y = 7;
				data.hotPreview[data.hotPreviewCount].x = data.hotPreviewCount;
				data.hotPreview[data.hotPreviewCount].y = (data.hotPreviewCount & 1) ? 8 - y : 8 + y;
				data.hotPreviewCount++;
				if(data.hotPreviewCount >= 63 || c >= data.samplesCount)
				{
					break;
				}
				i = 0;
				minValue = 0, maxValue = 0;
			}
		}
		if(data.hotPreviewCount < 63)
		{
			data.hotPreview[data.hotPreviewCount].x = data.hotPreviewCount;
			data.hotPreview[data.hotPreviewCount].y = 8;
			data.hotPreviewCount++;
		}
		data.hotPreview[data.hotPreviewCount].x = 63;
		data.hotPreview[data.hotPreviewCount].y = 8;
		data.hotPreviewCount++;
	}
	UpdatePlayTime();
	//Формируем платформонезависимые опции
	Assert(data.waveFormat.nSamplesPerSec >= wce_samplerate_min);
	Assert(data.waveFormat.nSamplesPerSec <= wce_samplerate_max);
	Assert(data.waveFormat.nChannels == 1 || data.waveFormat.nChannels == 2);
	//Формируем опции для PC волны
	optsAccessor.Enter();
	data.exportOptionsPC.samplerate = 44100;
	data.exportOptionsPC.channels = byte(data.waveFormat.nChannels);
	data.exportOptionsPC.format = wce_format_16bit;
	data.exportOptionsPC.compression = 0;
	data.exportOptionsPC.platform = wce_platform_pc;
	//Формируем опции для XBOX волны
	data.exportOptionsXBOX.samplerate = 44100;
	data.exportOptionsXBOX.channels = byte(data.waveFormat.nChannels);
	data.exportOptionsXBOX.format = wce_format_xma;
	data.exportOptionsXBOX.compression = wce_compression_def;
	data.exportOptionsXBOX.platform = wce_platform_xbox;
	data.version = fileVersion;
	optsAccessor.Leave();
	totalWaveDataLoad++;
	waveDataRefCounter = 1;
	//Перезаписываем параметры, если надо
	if(replaceWave)
	{
		data.exportOptionsPC = replaceWave->data.exportOptionsPC;
		data.exportOptionsXBOX = replaceWave->data.exportOptionsXBOX;
		data.exportOptionsPC.channels = byte(data.waveFormat.nChannels);
		data.exportOptionsXBOX.channels = byte(data.waveFormat.nChannels);
		memcpy(data.note, replaceWave->data.note, sizeof(data.note));
		if(data.exportOptionsPC.format == wce_format_xwma)
		{
			if(!IsCanXWmaEncode(data.exportOptionsPC.samplerate))
			{
				data.exportOptionsPC.format = wce_format_16bit;
			}
		}
		data.exportOptionsPC.channels = (byte)data.waveFormat.nChannels;
		if(data.exportOptionsXBOX.format == wce_format_xwma)
		{
			if(!IsCanXWmaEncode(data.exportOptionsXBOX.samplerate))
			{
				data.exportOptionsXBOX.format = wce_format_16bit;
			}
		}
		data.exportOptionsXBOX.channels = (byte)data.waveFormat.nChannels;
	}
	//Сохраняем в описании текстовое описание идентификатора
	memset(data.idString, sizeof(data.idString), ' ');
	const char * sid = GetId().ToString();
	long sidlen = string::Len(sid);
	Assert(sidlen < sizeof(data.idString));
	memcpy(data.idString, sid, sidlen + 1);
	//Проверяем волну
	WaveAnalize();
	//Находим длинну тихого хвоста
	FindUnimportantRemainder();
	//Сохраняем файл
	string defaultFilePath;
	BuildFilePath(defaultFilePath);
	Assert(defaultFilePath.Len() > 0);
	IFile * file = project->FileOpenBin(defaultFilePath.c_str(), SndProject::poff_isCreateNewFile, null, _FL_, 106);
	if(!file)
	{
		return options->ErrorOut(null, true, "Error wave import: Can't open file \"%s\"", defaultFilePath.c_str());
	}
	//Сохраняем блок с информацией
	if(file->Write(&data, sizeof(data)) == sizeof(data))
	{
		//Волновые данные волны
		if(file->Write(waveData, data.dataSize) == data.dataSize)
		{
			project->FileCloseBin(file, 106);
			UnloadWaveData(ProjectWaveDebugId_import);
			return ErrorId::ok;
		}
	}
	project->FileCloseBin(file, 106);
	return options->ErrorOut(null, true, "Error wave import: Can't save data to file \"%s\"", defaultFilePath.c_str());
}

//Обрезать хвосты и нормализовать длинну по размеру кратному 128 сэмплов
void ProjectWave::CropData(bool isStereo)
{
	//Ищем началольный сэмпл со значением выше порогового
	long samplesCount = data.dataSize/sizeof(short);
	dword step = 1;
	if(isStereo)
	{
		samplesCount /= 2;
		step = 2;
	}
	Assert(samplesCount >= wave_minsize);
	short * s = (short *)waveData;
	for(long i = 0; i < samplesCount; i++, s += step)
	{
		short v1 = abs(s[0]);
		short v2 = abs(s[step - 1]);
		short v = coremax(v1, v2);
		if(v >= wave_crop_in_threshold)
		{
			break;
		}
	}
	//Обрезаем начало и фэйдим, если надо
	if(i > wave_crop_in_samples_fade)
	{
		//Удаляем начало
		dword deleteSize = (i - wave_crop_in_samples_fade)*sizeof(short)*step;
		dword newDataSize = data.dataSize - deleteSize;
		samplesCount = data.dataSize/(sizeof(short)*step);
		memcpy(waveData, waveData + deleteSize, newDataSize);
		data.dataSize = newDataSize;
		//Фейдим начало
		s = (short *)waveData;
		for(long i = 0; i < wave_crop_in_samples_fade && i < samplesCount; i++, s += step)
		{
			float k = i*(1.0f/float(wave_crop_in_samples_fade));
			s[0] = short(long(s[0]*k));
			if(isStereo)
			{
				s[1] = short(long(s[1]*k));
			}
		}
	}
	//Ищем оконечный сэмпл по порогу
	s = ((short *)waveData) + (samplesCount - 1)*step;
	for(long i = samplesCount - 1; i > 0; i--, s -= step)
	{
		short v1 = abs(s[0]);
		short v2 = abs(s[step - 1]);
		short v = coremax(v1, v2);
		if(v >= wave_crop_out_threshold)
		{
			break;
		}
	}
	//Считаем сколько удалять
	long tailSamples = (samplesCount - 1) - i;							//Столько сэмплов в хвосте после порогового
	long deleteSamples = tailSamples - wave_crop_out_samples_fade;		//Столько сэмплов надо оставить
	if(deleteSamples < 0) deleteSamples = 0;
	samplesCount -= deleteSamples;
	//Выравниваем новый размер по границе грануляции в большую сторону
	samplesCount = ((samplesCount + wave_granular_align - 1)/wave_granular_align)*wave_granular_align;
	dword newDataSize = samplesCount*sizeof(short)*step;
	if(newDataSize < 0) newDataSize = 0;
	Assert((newDataSize % (sizeof(short)*step)) == 0);
	//Работаем с хвостом
	if(data.dataSize > (dword)newDataSize)
	{
		//Фэйдим окончание
		short * s = ((short *)waveData) + (samplesCount - 1)*step;
		for(dword i = 0; i < wave_crop_out_samples_fade; i++)
		{
			float k = 1.0f - i*(1.0f/float(wave_crop_out_samples_fade - 1));			
			s[0] = short(long(s[0]*k));
			if(isStereo)
			{
				s[1] = short(long(s[1]*k));
			}
			s -= step;
			if(s < (short *)waveData)
			{
				break;
			}
		}
	}else{
		//Добиваем конец нулями		
		memset(waveData + data.dataSize, 0, newDataSize - data.dataSize);
	}
	data.dataSize = newDataSize;
}

//Анализировать данные и сохранить результат в data.defects
void ProjectWave::WaveAnalize()
{
	Assert(waveData);
	Assert(data.samplesCount > 0);
	float ampl = 0.0f;
	float middle = 0.0f;	
	dword silenceCount = 0;
	dword clicksCount = 0;
	if(data.waveFormat.nChannels == 1)
	{
		ChannelAnalize((short *)waveData, data.samplesCount, 1, ampl, middle, silenceCount, clicksCount);
	}else{
		float amplR = 0.0f;
		float middleR = 0.0f;
		dword silenceCountR = 0;
		dword clicksCountR = 0;
		ChannelAnalize((short *)waveData, data.samplesCount, 2, amplR, middleR, silenceCountR, clicksCountR);
		float amplL = 0.0f;
		float middleL = 0.0f;
		dword silenceCountL = 0;
		dword clicksCountL = 0;
		ChannelAnalize(((short *)waveData) + 1, data.samplesCount, 2, amplL, middleL, silenceCountL, clicksCountL);
		ampl = coremax(amplR, amplL);
		middle = coremax(middleR, middleL);
		silenceCount = coremax(silenceCountR, silenceCountL);
		clicksCount = coremax(clicksCountR, clicksCountL);
	}
	float silencePrc = float(double(silenceCount)/double(data.samplesCount));
	data.defects = 0;
	data.defects |= ((long)Clampf(ampl*100.0f, 0.0f, 100.0f)) << df_amplitude_shift;	
	data.defects |= ((long)Clampf(middle*1000.0f, 0.0f, 110.0f)) << df_middle_shift;
	data.defects |= ((long)Clampf(silencePrc*100.0f, 0.0f, 100.0f)) << df_silence_shift;
	data.maxAmplitude = ampl;
	if(!CheckGranularSize())
	{
		data.defects |= df_size_align;
	}
	if(clicksCount > 0)
	{
		data.defects |= df_clicks;
	}
}

//Собрать статистику по каналу
void ProjectWave::ChannelAnalize(short * buffer, dword samples, dword step, float & ampl, float & middle, dword & silenceCount, dword & clicksCount)
{
	Assert(samples > 0);
	clicksCount = 0;
	long lastSilenceIndex = -100000;	
	double sum = 0.0f;
	silenceCount = 0;
	dword middleNorm = 0;
	long maxVal = 0;
	long history[4];
	for(long j = 0; j < ARRSIZE(history); j++) history[j] = lastSilenceIndex;
	for(long i = 0; i < (long)samples; i++)
	{
		long v = abs(buffer[i*step]);
		maxVal = coremax(maxVal, v);		
		if(v > 0)
		{
			sum += (double)v;
			middleNorm++;
			long silenceRange = i - lastSilenceIndex;
			if(silenceRange > 16)
			{
				//Добавляем в историю новый фрагмент
				for(long j = ARRSIZE(history) - 1; j >= 2; j--) history[j] = history[j - 2];
				history[0] = i;
				history[1] = lastSilenceIndex;
				//Анализируем текущий буффер
				if(history[1] - history[2] < 10)
				{
					if(history[0] - history[1] > 30 && history[2] - history[3] > 30)
					{
						//Проверяем пик в заданном диапазоне
						long maxVal = 0;
						Assert(history[1] >= 0);
						Assert(history[2] >= 0);
						for(long j = history[2]; j < history[1]; j++)
						{
							long vClick = abs(buffer[j*step]);
							maxVal = coremax(vClick, maxVal);
						}
						if(maxVal > (df_min_click_level*32767)/100)
						{
							clicksCount++;
						}
					}
				}
				silenceCount += i - coremax(lastSilenceIndex, 0);
			}
			lastSilenceIndex = i + 1;
		}
	}
	ampl = maxVal/32767.0f;
	if(ampl > 1.0f) ampl = 1.0f;	
	if(middleNorm)
	{
		middle = float(sum/(middleNorm*32767.0));
	}else{
		middle = 0.0f;
	}
	if(middle > 1.0f) middle = 1.0f;
}

//Найти время остатка, который можно пропустить
void ProjectWave::FindUnimportantRemainder()
{
	Assert(waveData);
	Assert(data.samplesCount > 0);
	Assert(data.waveFormat.wBitsPerSample == 16);
	Assert(data.waveFormat.nSamplesPerSec == 44100);	
	Assert(data.waveFormat.nChannels == 1 ||  data.waveFormat.nChannels == 2);
	short * wave = (short *)waveData;
	static const short threshold = coremin((long)(32767.0f*0.01f*df_unimportant_amp), 32767);
	long i = data.samplesCount;
	if(data.waveFormat.nChannels == 2) i *= 2;
	for(i = i - 1; i > 0; i--)
	{
		if(abs(wave[i]) > threshold)
		{
			break;
		}
	}
	if(data.waveFormat.nChannels == 2) i = i >> 1;	
	long saples = data.samplesCount - i;
	Assert(saples <= (long)data.samplesCount);
	Assert(saples >= 0);
	data.unimportantTime = saples*(1.0f/44100.0f);
}


//Прочитать формат файла
bool ProjectWave::ReadMMIOFormat(HMMIO & hmmio, MMCKINFO & ckRiff, array<byte> & pcmWFBuffer)
{
	MMCKINFO ckIn;
	ckIn.ckid = mmioFOURCC('f', 'm', 't', ' ');
	if(mmioDescend(hmmio, &ckIn, &ckRiff, MMIO_FINDCHUNK) != MMSYSERR_NOERROR 
		|| ckIn.cksize < (long)sizeof(PCMWAVEFORMAT))
	{
		return false;
	}
	PCMWAVEFORMAT pcmWaveFormat;
	if(mmioRead(hmmio, (HPSTR)&pcmWaveFormat, sizeof(pcmWaveFormat) ) != sizeof(pcmWaveFormat))
	{
		return false;
	}
	if(pcmWaveFormat.wf.wFormatTag == WAVE_FORMAT_PCM)
	{
		pcmWFBuffer.AddElements(sizeof(WAVEFORMATEX));
		memcpy(pcmWFBuffer.GetBuffer(), &pcmWaveFormat, sizeof(pcmWaveFormat));
		WAVEFORMATEX * pcmWaveFormatEx = (WAVEFORMATEX *)pcmWFBuffer.GetBuffer();
		pcmWaveFormatEx->cbSize = 0;
	}else{
		WORD extraBytes = 0;
		if(mmioRead(hmmio, (CHAR *)&extraBytes, sizeof(WORD)) != sizeof(WORD))
		{
			return false;
		}
		pcmWFBuffer.AddElements(sizeof(WAVEFORMATEX) + extraBytes);
		memcpy(pcmWFBuffer.GetBuffer(), &pcmWaveFormat, sizeof(pcmWaveFormat));
		WAVEFORMATEX * pcmWaveFormatEx = (WAVEFORMATEX *)pcmWFBuffer.GetBuffer();
		pcmWaveFormatEx->cbSize = extraBytes;
		if(mmioRead(hmmio, (CHAR *)&pcmWaveFormatEx->cbSize + sizeof(WORD), extraBytes) != extraBytes)
		{
			return false;
		}
	}
	if(mmioAscend(hmmio, &ckIn, 0) != 0)
	{
		return false;
	}
	return true;
}

//Прочитать данные файла
ErrorId ProjectWave::ReadMMIOData(HMMIO & hmmio, MMCKINFO & ckRiff, bool isStereo, const char * importFileName)
{
	if(rawWaveDataPtr)
	{
		delete rawWaveDataPtr;
		rawWaveDataPtr = null;
		waveData = null;
		data.dataSize = 0;
	}
	MMCKINFO dataInfo;
	dataInfo.ckid = mmioFOURCC('d', 'a', 't', 'a');
	if(mmioDescend(hmmio, &dataInfo, &ckRiff, MMIO_FINDCHUNK) != MMSYSERR_NOERROR)
	{
		return options->ErrorOut(null, true, "Error wave import: no data chunk in file \"%s\"", importFileName);
	}
	data.dataSize = dataInfo.cksize;
	if(data.dataSize < wave_minsize*sizeof(short)*2)
	{
		return options->ErrorOut(null, true, "Error wave import: file \"%s\" have got to small size (need more then %u samples)", importFileName, (long)wave_minsize);
	}
	rawWaveDataPtr = NEW byte[data.dataSize + wave_minsize*sizeof(short)*2 + 32];
	waveData = AlignPtr(rawWaveDataPtr);
	memset(waveData, 0, data.dataSize);
	if(mmioRead(hmmio, (CHAR *)waveData, dataInfo.cksize) != dataInfo.cksize)
	{
		return options->ErrorOut(null, true, "Error wave import: MMIO error, file \"%s\"", importFileName);
	}
	if(mmioAscend(hmmio, &dataInfo, 0) != MMSYSERR_NOERROR)
	{
		return options->ErrorOut(null, true, "Error wave import: MMIO error, file \"%s\"", importFileName);
	}	
	//Модифицируем данные под необходимые требования
	CropData(isStereo);
	if(data.dataSize < wave_minsize*sizeof(short)*2)
	{
		return options->ErrorOut(null, true, "Error wave import: file \"%s\" have got to many silence data (need more then %u samples with greater amplitude)", importFileName, (long)wave_minsize);
	}
	return ErrorId::ok;
}

//Попытаться прочитать метки цикла
void ProjectWave::ReadLoopLabels(HMMIO & hmmio, MMCKINFO & ckRiff)
{
	/*
	MMCKINFO sampleInfo;
	sampleInfo.ckid = mmioFOURCC('s', 'm', 'p', 'l');
	if(mmioDescend(hmmio, &sampleInfo, &ckRiff, MMIO_FINDCHUNK) != MMSYSERR_NOERROR)
	{
		return;
	}
	if(sampleInfo.cksize < sizeof(SapmlerData))
	{
		mmioAscend(hmmio, &sampleInfo, 0);
		return;
	}
	SapmlerData samplerData;
	if(mmioRead(hmmio, (CHAR *)&samplerData, sizeof(SapmlerData)) != sizeof(SapmlerData))
	{
		if(samplerData.header.numSampleLoops >= 1)
		{
			if(samplerData.loops[0].playCount < 0x80000000)
			{
				data.loopCount = samplerData.loops[0].playCount;
				data.loopStartOffset = samplerData.loops[0].start;
				data.loopEndOffset = samplerData.loops[0].end;
			}
		}
	}
	mmioAscend(hmmio, &sampleInfo, 0);
	*/
}

//Обновить время проигрывания
void ProjectWave::UpdatePlayTime()
{
	crt_snprintf(playTimeData, sizeof(playTimeData), "%.3f%s", data.playTime, options->GetString(SndOptions::s_units_time_sec));
	playTimeData[sizeof(playTimeData) - 1] = 0;
	playTimeName.Init(playTimeData);
}


//----------------------------------------------------------------------------------------
//Методы для экспортёра
//----------------------------------------------------------------------------------------

//Сгенерировать путь до экспорчёного файла
void ProjectWave::BuildExportPath(string & fileName, bool isPhonemes)
{
	//Формируем общий путь
	fileName = options->pathTempSaveFolder;
	fileName += data.idString;
	fileName += isPhonemes ? ".ph" : ".we";
}


//----------------------------------------------------------------------------------------
//ProjectObject
//----------------------------------------------------------------------------------------

//Перезаписать идентификатор объекта
void ProjectWave::ReplaceId(const UniqId & newId)
{
	ProjectObject::ReplaceId(newId);
	//Сохраняем в описании текстовое описание идентификатора
	memset(data.idString, sizeof(data.idString), ' ');
	const char * sid = GetId().ToString();
	long sidlen = string::Len(sid);
	Assert(sidlen < sizeof(data.idString));
	memcpy(data.idString, sid, sidlen + 1);
}

//Сохранить объект
ErrorId ProjectWave::OnSaveObject(const char * defaultFilePath)
{
	AssertCoreThread
	IFile * file = project->FileOpenBin(defaultFilePath, SndProject::poff_isWaitWhenOpen, null, _FL_, 107);
	Assert(file);
	//Сохраняем блок с информацией
	if(file->Write(&data, sizeof(data)) != sizeof(data))
	{
		return options->ErrorOut(null, true, "Error wave save: Can't save file \"%s\"", defaultFilePath);
	}
	project->FileCloseBin(file, 107);
	return ErrorId::ok;	
}

//Загрузить описание волны
ErrorId ProjectWave::OnLoadObject(const char * defaultFilePath)
{	
	AssertCoreThread
	DataChunk d;
	IFile * file = project->FileOpenBin(defaultFilePath, SndProject::poff_isWaitWhenOpen | SndProject::poff_isOpenForRead, null, _FL_, 108);
	if(!file)
	{
		return options->ErrorOut(null, true, "Error wave load: Can't open file \"%s\"", defaultFilePath);
	}
	if(file->Read(&d, sizeof(d)) != sizeof(d))
	{
		project->FileCloseBin(file, 108);
		return options->ErrorOut(null, true, "Error wave load: Can't read data from file \"%s\"", defaultFilePath);
	}
	project->FileCloseBin(file, 108);
	UniqId id;
	if(!id.FromString(d.idString))
	{
		return options->ErrorOut(null, true, "Error wave load: Can't restore id (%s). File: \"%s\"", d.idString, defaultFilePath);
	}
	ReplaceId(id);
	const char * sidz = id.ToString();
	crt_strcpy(textId, sizeof(textId), sidz);
	data = d;
	UpdatePlayTime();
	Assert(data.samplesCount >= 256);
	Assert(data.waveFormat.nSamplesPerSec == 44100);
	Assert(data.waveFormat.nChannels == 1 || data.waveFormat.nChannels == 2);
	//Если версия расходиться, то фиксим необходимое
	if(data.version <= fileVersion02)
	{
		data.version = fileVersion03;
		LoadWaveData(ProjectWaveDebugId_loadconv);
		WaveAnalize();
		FindUnimportantRemainder();
		UnloadWaveData(ProjectWaveDebugId_loadconv);
		if(data.exportOptionsPC.format == wce_format_xwma)
		{
			if(!IsCanXWmaEncode(data.exportOptionsPC.samplerate))
			{
				data.exportOptionsPC.format = wce_format_16bit;
				options->ErrorOut(null, true, "Warning wave load: Fix PC format from wXMA to PCM. File: \"%s\"", defaultFilePath);
			}
		}
		if(data.exportOptionsPC.channels != data.waveFormat.nChannels)
		{
			data.exportOptionsPC.channels = (byte)data.waveFormat.nChannels;
			options->ErrorOut(null, true, "Warning wave load: Fix PC channels. File: \"%s\"", defaultFilePath);
		}
		if(data.exportOptionsXBOX.format == wce_format_xwma)
		{
			if(!IsCanXWmaEncode(data.exportOptionsXBOX.samplerate))
			{
				data.exportOptionsXBOX.format = wce_format_16bit;
				options->ErrorOut(null, true, "Warning wave load: Fix XBOX format from wXMA to PCM. File: \"%s\"", d.idString, defaultFilePath);
			}
		}
		if(data.exportOptionsXBOX.channels != data.waveFormat.nChannels)
		{
			data.exportOptionsXBOX.channels = (byte)data.waveFormat.nChannels;
			options->ErrorOut(null, true, "Warning wave load: Fix XBOX channels. File: \"%s\"", defaultFilePath);
		}
		SetToSave();
		DeleteExportData();
	}
	if(data.exportOptionsPC.format == wce_format_xwma) Assert(IsCanXWmaEncode(data.exportOptionsPC.samplerate));
	if(data.exportOptionsXBOX.format == wce_format_xwma) Assert(IsCanXWmaEncode(data.exportOptionsXBOX.samplerate));
	Assert(data.exportOptionsPC.channels == data.waveFormat.nChannels);
	Assert(data.exportOptionsXBOX.channels == data.waveFormat.nChannels);
	return ErrorId::ok;
}

//Событие удаления объекта из проекта
void ProjectWave::OnDeleteObject()
{
	//Остановить экспорт, удалить выгруженные данные
	DeleteExportData();
}
















