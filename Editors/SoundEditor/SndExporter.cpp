

#include "SndExporter.h"

#include "SndOptions.h"
#include "project\SndProject.h"

#include "project\ProjectWaveExporter.h"


//Массив шагов выгрузки
SndExporter::Step SndExporter::exportSteps[] = 
{
	{null, &SndExporter::ProcessFillSoundsTable},
	{&SndExporter::InitProcessPrepareSounds, &SndExporter::ProcessPrepareSounds},
	{&SndExporter::InitProcessPrepareWaves, &SndExporter::ProcessPrepareWaves},
	{null, &SndExporter::ProcessMemoryMaping},
	{&SndExporter::InitProcessFillTableSounds, &SndExporter::ProcessFillTableSounds},
	{&SndExporter::InitProcessFillTableWaves, &SndExporter::ProcessFillTableWaves},
	{null, &SndExporter::ProcessFillTableEntry},
	{null, &SndExporter::ProcessFillHeaders},
	{null, &SndExporter::ProcessPrepare},
	{null, &SndExporter::ProcessSaveToFile},
};



SndExporter::WaveInfo::WaveInfo() : exportWaves(_FL_),
									exportPhonemes(_FL_)
{

}

SndExporter::SndExporter() : exportSounds(_FL_),
								exportWaves(_FL_, 1024),
								names(_FL_, 1024),
								data(_FL_),
								errors(_FL_)
{
	ProjectWaveExporter::SetActiveMode();
	progressOutput = null;
	toNextStage = false;
	dataSize = 0;
	isErrors = false;
	file = null;
	exportStageCount = 0;
	waveInfoCount = 0;
	xboxPartSize = 0;
	xboxWavesDataSize = 0;
	pcPartSize = 0;
	pcWavesDataSize = 0;
	bankId = null;
	bankXboxHeader = null;
	bankPCHeader = null;
}

SndExporter::~SndExporter()
{
	exportStageCount = 0;
	waveInfoCount = 0;
	xboxPartSize = 0;
	xboxWavesDataSize = 0;
	pcPartSize = 0;
	pcWavesDataSize = 0;
	bankId = null;
	bankXboxHeader = null;
	bankPCHeader = null;
	ProjectWaveExporter::ReleaseActiveMode();
	if(file)
	{
		file->Release();
		options->fileService->Delete(filePath.c_str());		
	}
}

//Подготовиться к выгрузке проекта
void SndExporter::ExportPrepareBank(const UniqId & sndBankId)
{
	toNextStage = false;
	isErrors = false;
	exportStageCount = 0;
	currentIndexCounter = 0;
	exportSounds.Empty();
	soundBankId = sndBankId;
	ProjectSoundBank * sbk = project->SoundBankGet(sndBankId);
	Assert(sbk);
	soundBankFolderId = sbk->GetSBFolderId();
}

//Выполнить шаг выгрузки
bool SndExporter::ExportStep()
{
	if(exportStageCount >= (long)ARRSIZE(exportSteps))
	{
		if(progressOutput)
		{
			if(!progressOutput->data.IsEmpty())
			{
				progressOutput->data.Empty();
				progressOutput->Init(null);
			}
			ProcessIdle();
		}
		return true;
	}
	dword oldTime = GetTickCount();
	dword dltTime = 0;
	while(true)
	{
		if(!toNextStage)
		{
			PrintCurrentStage();
			toNextStage = (this->*exportSteps[exportStageCount].step)();
			if(toNextStage)
			{				
				//Стадия меняеться надо написать об этом
				PrintCurrentStage();
				break;
			}
		}else{
			if(!isErrors)
			{
				exportStageCount++;
			}else{
				exportStageCount = ARRSIZE(exportSteps);
			}
			PrintCurrentStage();
			if(exportStageCount < ARRSIZE(exportSteps))
			{
				toNextStage = false;
				if(exportSteps[exportStageCount].init)
				{
					(this->*exportSteps[exportStageCount].init)();
					break;
				}				
			}else{
				InitProcessIdle();
				break;
			}			
		}
		dword currentTime = GetTickCount();
		dltTime += currentTime - oldTime;
		oldTime = currentTime;
		//Обновляем экран не слишком часто
		if(dltTime > 80) break;
	}
	progressOutput->Init(progressOutput->data.c_str());
	return false;
}

//Размер банка для xbox
dword SndExporter::GetXboxBankSize()
{
	return xboxPartSize;
}

//Размер банка для pc
dword SndExporter::GetPCBankSize()
{
	return pcPartSize;
}


//Написать о текущей стадии экспорта
void SndExporter::PrintCurrentStage()
{
	if(progressOutput)
	{
		if(exportStageCount < ARRSIZE(exportSteps))
		{
			progressOutput->data = options->GetString(SndOptions::s_export_msg_stage_1);
			progressOutput->data += (exportStageCount + 1);
			progressOutput->data += options->GetString(SndOptions::s_export_msg_of);
			progressOutput->data += ARRSIZE(exportSteps);
			progressOutput->data += options->GetString(SndOptions::s_export_msg_stage_2);
		}else{
			progressOutput->data.Empty();
		}
	}
}

//Подготовка процесса построения таблицы звуков, которые будет экспортить
bool SndExporter::ProcessFillSoundsTable()
{	
	const array<ProjectSound *> & sounds = project->SoundArray();
	if(currentIndexCounter <= 0)
	{
		Assert(currentIndexCounter == 0);
		exportSounds.Reserve(sounds.Size());
	}
	if((dword)currentIndexCounter >= sounds.Size())
	{
		if(progressOutput)
		{
			progressOutput->data += options->GetString(SndOptions::s_export_msg_1_1);
		}
		return true;
	}
	if(progressOutput)
	{		
		progressOutput->data += options->GetString(SndOptions::s_export_msg_1_2);
		progressOutput->data += (currentIndexCounter + 1);
		progressOutput->data += options->GetString(SndOptions::s_export_msg_of);
		progressOutput->data += sounds.Size();
	}
	long soundIndex = currentIndexCounter;
	currentIndexCounter++;
	ProjectSound * sound = sounds[soundIndex];	
	if(!sound) return false;
	const UniqId & soundFolder = sound->GetFolder();
	if(project->FolderIsChild(soundBankFolderId, soundFolder))
	{
		SoundInfo si;
		si.id = sound->GetId();
		si.queueSelect = -1;
		si.queueLoop = -1;
		exportSounds.Add(si);
	}
	return false;
}

//Инициализация процесса подготовки звуков
void SndExporter::InitProcessPrepareSounds()
{
	if(progressOutput)
	{
		progressOutput->data += options->GetString(SndOptions::s_export_msg_2_1);
	}
	exportWaves.Empty();
	names.Empty();
	waveInfoCount = 0;
	entryTableSize = 0;
	currentQueuePos = 0;
	exportWaves.Reserve(exportSounds.Size()*4);
	currentIndexCounter = 0;
	bankId = null;
	bankXboxHeader = null;
	bankPCHeader = null;
	bankPCDebugTableSize = 0;
	bankPCDebugTableCounter = 0;
	Assert(soundBankFolderId.IsValidate());
}

//Подготовка звуков
bool SndExporter::ProcessPrepareSounds()
{	
	if(currentIndexCounter == exportSounds.Size())
	{
		if(progressOutput)
		{
			progressOutput->data += options->GetString(SndOptions::s_export_msg_2_2);
		}
	}
	if(currentIndexCounter >= exportSounds.Size())
	{
		return true;
	}
	if(progressOutput)
	{
		progressOutput->data += options->GetString(SndOptions::s_export_msg_2_3);
		progressOutput->data += (currentIndexCounter + 1);
		progressOutput->data += options->GetString(SndOptions::s_export_msg_of);
		progressOutput->data += exportSounds.Size();
	}
	//Обрабатываем текущий звук
	SoundInfo & sinfo = exportSounds[currentIndexCounter];
	ProjectSound * sound = project->SoundGet(sinfo.id);
	long wavesCount = sound->GetWavesCount();
	if(wavesCount <= 0)
	{
		isErrors = true;
		Error & error = errors[errors.Add()];
		error.code = ec_nowaves;
		error.id = sinfo.id;
		exportSounds.DelIndex(currentIndexCounter);
		return false;
	}	
	//Сохраняем имя
	const ExtName & sname = sound->GetName();
	sinfo.name = names.Size();
	names.AddElements(sname.len + 1);
	for(dword i = 0; i < sname.len + 1; i++)
	{
		char c = sname.str[i];
		if(c >= 'A' && c <= 'Z') c += 'a' - 'A';
		names[sinfo.name + i] = c;
	}	
	//Добавляем базовые параметры
	const SoundBaseParams * sbp = sound->GetCurrentBaseParams();
	if(!sbp)
	{
		isErrors = true;
		Error & error = errors[errors.Add()];
		error.code = ec_nobaseparams;
		error.id = sinfo.id;
		exportSounds.DelIndex(currentIndexCounter);
		return false;
	}
	//Добавляем график затухания
	const SoundAttGraph * att = sound->GetCurrentAttenuation();
	if(!att)
	{
		isErrors = true;
		Error & error = errors[errors.Add()];
		error.code = ec_noattenuation;
		error.id = sinfo.id;
		exportSounds.DelIndex(currentIndexCounter);
		return false;
	}
	//Параметры выбора волн на запуске и цикле
	if(sbp->selectMethod == sbpc_select_queue)
	{
		sinfo.queueSelect = currentQueuePos;
		currentQueuePos += SoundBankFileWaveQueue::GetSize(wavesCount);
	}
/*	if(sbp->loopMode == sbpc_loop_sel_wave_que)
	{
		sinfo.queueLoop = currentQueuePos;
		currentQueuePos += SoundBankFileWaveQueue::GetSize(wavesCount);
	}
*/
	//Добавляем волны в таблицу текущего банка
	waveInfoCount += wavesCount;
	bool isNeedPhonemes = (sbp->modifiers & sbpc_mod_phoneme) != 0;
	for(long i = 0; i < wavesCount; i++)
	{
		const UniqId & wid = sound->GetWave(i).waveId;
		if(wid.IsValidate())
		{
			ProjectWave * wave = project->WaveGet(wid);
			if(!wave)
			{
				isErrors = true;
				Error & error = errors[errors.Add()];
				error.code = ec_wavenotfound;
				error.id = sinfo.id;
				exportSounds.DelIndex(currentIndexCounter);
				return false;
			}
		}
		AddUniqueWave(wid, isNeedPhonemes);
	}
	currentIndexCounter++;
	return false;
}

//Инициализация процесса подготовки волн
void SndExporter::InitProcessPrepareWaves()
{
	if(progressOutput)
	{
		progressOutput->data += options->GetString(SndOptions::s_export_msg_3_1);
	}
	currentIndexCounter = 0;
}

//Подготовка волн
bool SndExporter::ProcessPrepareWaves()
{
	if(currentIndexCounter == exportWaves.Size())
	{
		if(progressOutput)
		{
			progressOutput->data += options->GetString(SndOptions::s_export_msg_3_2);
		}
		currentIndexCounter++;
	}
	if(currentIndexCounter >= exportWaves.Size())
	{		
		return true;
	}
	if(progressOutput)
	{
		progressOutput->data += options->GetString(SndOptions::s_export_msg_3_3);
		progressOutput->data += (currentIndexCounter + 1);
		progressOutput->data += options->GetString(SndOptions::s_export_msg_of);
		progressOutput->data += exportWaves.Size();
	}
	WaveInfo & wi = exportWaves[currentIndexCounter];
	ProjectWave * pw = project->WaveGet(wi.id);
	Assert(pw);
	if(pw->GetErrorCode().IsError())
	{
		//Сообщение об ошибке экспорта волны		
		isErrors = true;
		Error & error = errors[errors.Add()];
		error.code = ec_errorwaveexport;
		error.id = wi.id;
		//Процессируем следующую волну
		currentIndexCounter++;
		return false;
	}
	bool isWavesExportComplette = false;
	if(wi.exportWaves.Size() == 0)
	{
		if(pw->GetExportData(&wi.exportWaves, false, false))
		{
			isWavesExportComplette = true;
		}
	}else{
		isWavesExportComplette = true;
	}
	bool isPhonemesExportComplette = true;
	if(wi.needPhonemes)
	{		
		if(wi.exportPhonemes.Size() == 0)
		{
			isPhonemesExportComplette = false;
			if(pw->GetExportData(&wi.exportPhonemes, false, true))
			{				
				isPhonemesExportComplette = true;
			}			
		}
	}
	if(isWavesExportComplette && isPhonemesExportComplette)
	{
		currentIndexCounter++;
	}else{
		//Не готовы, поэтому отдадим ресурсы другим потокам
		Sleep(5);
	}
	return false;
}


#define SndExporter_SetPointer(type, size, align, safe) ((type *)((byte *)null + ((bytesCounter + align - 1) & ~(align - 1)))); bytesCounter = ((bytesCounter + align - 1) & ~(align - 1)) + size*sizeof(type) + safe;
#define SndExporter_FixPointer(type, ptr, base) ptr = (type *)((byte *)base + ((byte *)ptr - (byte *)null))


#include "..\..\common_h\corecmds.h"

//Разметка и выделение памяти
bool SndExporter::ProcessMemoryMaping()
{
	Assert(data.Size() == 0);
	if(progressOutput)
	{
		progressOutput->data += options->GetString(SndOptions::s_export_msg_4_1);
	}
#ifdef ProjectWaveDebugEnable
	api->ExecuteCoreCommand(CoreCommand_MemStat(cmemstat_bysize));
#endif	
	//Вынудим все используемые волны освободить память	
	const array<ProjectWave *> & waves = project->WaveArray();
	for(dword i = 0; i < waves.Size(); i++)
	{
		if(waves[i])
		{
			waves[i]->WorkUpdate(true);
		}
	}
	//Счётчики для последующего использования
	bankExtrasCount = 0;
	bankExtrasOffset = 0;
	//Размер таблицы для поиска звуков по имени
	entryTableSize = 32;
	while(entryTableSize < exportSounds.Size() && entryTableSize < 1024)
	{
		entryTableSize <<= 1;
	}
	//Расметка относительных смещений для волн
	dword phonemeDataCount = 0;
	dword phonemesPartOffset = 0;
	long waveOffsetPrtXbox = 0;
	long waveOffsetPrtPC = 0;
	long formatsOffsetPrtXbox = 0;
	long formatsOffsetPrtPC = 0;	

//	array<byte> waveFmts(_FL_);

	for(long i = 0; i < exportWaves; i++)
	{
		//Описания волны
		WaveInfo & winfo = exportWaves[i];
		Assert(winfo.exportWaves);
		ProjectWave::ExportWavesPart * exportWave = (ProjectWave::ExportWavesPart *)winfo.exportWaves.GetBuffer();

/*
		byte * formatInfoPtr = (byte *)exportWave + sizeof(ProjectWave::ExportWavesPart) + exportWave->waveformatSizeXbox + exportWave->waveSizeForXbox;
		dword formatInfoSize = exportWave->waveformatSizePC;
		long formatsOffset = -1;
		for(dword i = 0; i < waveFmts.Size(); )
		{
			dword size = (dword &)waveFmts[i];
			if(size == formatInfoSize)
			{
				byte * ptr = &waveFmts[i + sizeof(dword)];
				for(dword j = 0; j < size; j++)
				{
					if(ptr[j] != formatInfoPtr[j]) break;
				}
				if(j == size)
				{
					formatsOffset = i + sizeof(dword);
					break;
				}				
			}
			i += size + 4;
		}
		if(formatsOffset < 0)
		{
			formatsOffset = waveFmts.Size();
			waveFmts.AddElements(sizeof(dword) + formatInfoSize);
			(dword &)waveFmts[formatsOffset] = formatInfoSize;
			byte * ptr = &waveFmts[formatsOffset + sizeof(dword)];
			for(dword j = 0; j < formatInfoSize; j++)
			{
				ptr[j] = formatInfoPtr[j];
			}
		}

//  */


		

		//Расположение волн в блоке данных
		if((exportWave->exportFormatXbox & SoundBankFileWave::f_format_mask) == SoundBankFileWave::f_format_xma)
		{
			//Начало xma должно лежать на границе 2048
			waveOffsetPrtXbox = (waveOffsetPrtXbox + 2047) & ~2047;
		}
		//Прописываем смещения для Xbox
		winfo.waveOffsetXbox = waveOffsetPrtXbox;
		waveOffsetPrtXbox += (exportWave->waveSizeForXbox + 0xf) & ~0xf;
		winfo.formatOffsetXbox = formatsOffsetPrtXbox;
		formatsOffsetPrtXbox += (exportWave->waveformatSizeXbox + 0xf) & ~0xf;
		//Прописываем смещения для PC
		winfo.waveOffsetPC = waveOffsetPrtPC;
		waveOffsetPrtPC += (exportWave->waveSizeForPC + 0xf) & ~0xf;
		winfo.formatOffsetPC = formatsOffsetPrtPC;
		formatsOffsetPrtPC += (exportWave->waveformatSizePC + 0xf) & ~0xf;
		//Добавляем описание фонем
		if(winfo.exportPhonemes)
		{
			phonemeDataCount++;
			ProjectWave::ExportPhonemesPart * exportPhonemes = (ProjectWave::ExportPhonemesPart *)winfo.exportPhonemes.GetBuffer();
			winfo.phonemesOffset = phonemesPartOffset;
			dword size = SoundBankFilePhonemes::GetSize(exportPhonemes->count);
			size = (size + 0xf) & ~0xf;
			phonemesPartOffset += size;
		}
	}
	bankPCDebugTableSize = exportSounds.Size()*3 + exportWaves.Size();
	//Размечаем память относительно null
	bankPCHeader = bankXboxHeader = null;
	dword bytesCounter = sizeof(SoundBankFileHeader);
	bankPCSounds = bankXboxSounds = SndExporter_SetPointer(SoundBankFileSound, exportSounds.Size(), 16, 0);
	bankPCInfos = bankXboxInfos = SndExporter_SetPointer(SoundBankFileWaveInfo, waveInfoCount, 16, 0);
	bankPCWaves = bankXboxWaves = SndExporter_SetPointer(SoundBankFileWave, exportWaves.Size(), 16, 0);
	bankPCEntry = bankXboxEntry = SndExporter_SetPointer(SoundBankFileSound *, entryTableSize, 16, 16);
	bankPCExtras = bankXboxExtras = SndExporter_SetPointer(SoundBankFileExtra, phonemeDataCount, 4, 16);
	bankPCQueues = bankXboxQueues = SndExporter_SetPointer(byte, currentQueuePos, 16, 4);
	bankPCNames = bankXboxNames = SndExporter_SetPointer(byte, names.Size(), 16, 4);
	bankPCExtrasData = bankXboxExtrasData = SndExporter_SetPointer(byte, phonemesPartOffset, 16, 32);
	//Дальше xbox и PC расходяться по размеру содержимого
	dword wavesPartBytesCounter = bytesCounter;
	//Волновые данные для xbox
	bankXboxWavesDataFormat = SndExporter_SetPointer(byte, formatsOffsetPrtXbox, 16, 32);
	bankXboxWavesData = SndExporter_SetPointer(byte, waveOffsetPrtXbox, 2048, 2048);
	xboxWavesDataSize = bytesCounter - ((byte *)bankXboxWavesData - (byte *)null);
	xboxPartSize = bytesCounter;	
	//Волновые данные для PC
	bytesCounter = wavesPartBytesCounter;
	bankPCWavesDataFormat = SndExporter_SetPointer(byte, formatsOffsetPrtPC, 16, 32);
	bankPCWavesData = SndExporter_SetPointer(byte, waveOffsetPrtPC, 256, 256);
	pcWavesDataSize = bytesCounter - ((byte *)bankPCWavesData - (byte *)null);
	//Отладочные данные для PC части	
	bankPCDebugTable = SndExporter_SetPointer(SoundBankFileObjectId, bankPCDebugTableSize, 16, 256);
	bankPCDebugTableOffset = ((byte *)bankPCDebugTable - (byte *)null);
	pcPartSize = bytesCounter;
	//Выделяем требуемую память
	dword totalSize = sizeof(SoundBankFileId) + xboxPartSize + pcPartSize;
	//Выделяем общую память и обнуляем её
	dataSize = totalSize;
	data.AddElements(dataSize - xboxWavesDataSize - pcWavesDataSize);
	memset(data.GetBuffer(), 0, data.Size());
	//Корректируем указатели на выделенную память
	bankId = (SoundBankFileId *)data.GetBuffer();
	//Xbox
	bankXboxHeader = (SoundBankFileHeader *)(data.GetBuffer() + sizeof(SoundBankFileId));
	SndExporter_FixPointer(SoundBankFileSound, bankXboxSounds, bankXboxHeader);
	SndExporter_FixPointer(SoundBankFileWaveInfo, bankXboxInfos, bankXboxHeader);
	SndExporter_FixPointer(SoundBankFileWave, bankXboxWaves, bankXboxHeader);
	SndExporter_FixPointer(SoundBankFileSound *, bankXboxEntry, bankXboxHeader);
	SndExporter_FixPointer(SoundBankFileExtra, bankXboxExtras, bankXboxHeader);
	SndExporter_FixPointer(byte, bankXboxQueues, bankXboxHeader);
	SndExporter_FixPointer(byte, bankXboxNames, bankXboxHeader);
	SndExporter_FixPointer(byte, bankXboxExtrasData, bankXboxHeader);
	SndExporter_FixPointer(byte, bankXboxWavesDataFormat, bankXboxHeader);
	SndExporter_FixPointer(byte, bankXboxWavesData, bankXboxHeader);
	Assert((byte *)bankXboxWavesData - (byte *)bankXboxHeader == xboxPartSize - xboxWavesDataSize);
	//PC
	bankPCHeader = (SoundBankFileHeader *)bankXboxWavesData;
	SndExporter_FixPointer(SoundBankFileSound, bankPCSounds, bankPCHeader);
	SndExporter_FixPointer(SoundBankFileWaveInfo, bankPCInfos, bankPCHeader);
	SndExporter_FixPointer(SoundBankFileWave, bankPCWaves, bankPCHeader);
	SndExporter_FixPointer(SoundBankFileSound *, bankPCEntry, bankPCHeader);
	SndExporter_FixPointer(SoundBankFileExtra, bankPCExtras, bankPCHeader);
	SndExporter_FixPointer(byte, bankPCQueues, bankPCHeader);
	SndExporter_FixPointer(byte, bankPCNames, bankPCHeader);
	SndExporter_FixPointer(byte, bankPCExtrasData, bankPCHeader);	
	SndExporter_FixPointer(byte, bankPCWavesDataFormat, bankPCHeader);
	SndExporter_FixPointer(byte, bankPCWavesData, bankPCHeader);
	Assert((byte *)bankPCWavesData - (byte *)bankPCHeader == bankPCDebugTableOffset - pcWavesDataSize);
	//Таблицу отладки распалагаем на место волн, а волны сохраним прямо в файл
	//SndExporter_FixPointer(SoundBankFileObjectId, bankPCDebugTable, bankPCHeader);
	bankPCDebugTable = (SoundBankFileObjectId *)bankPCWavesData;
	//Открываем файл на запись и заполняем его 0
	ProjectSoundBank * sb = project->SoundBankGet(soundBankId);
	Assert(sb);
	filePath = sb->exportPath;
	filePath.Lower();
	filePath.AddExtention(".ssb");
	string fileDirs;
	fileDirs.GetFilePath(filePath);
	options->fileService->CreateFolder(fileDirs.c_str());
	file = options->fileService->OpenFile(filePath, file_create_always, _FL_);
	if(!file)
	{
		exportStageCount = ARRSIZE(exportSteps);
		isErrors = true;		
		Error & error = errors[errors.Add()];
		error.code = ec_filenotopen;
		error.id.Reset();
	}
	array<byte> zeroBuffer(_FL_, 1);
	zeroBuffer.AddElements(65536*256);
	memset(zeroBuffer.GetBuffer(), 0, zeroBuffer.Size());
	for(dword size = totalSize; size > 0; )
	{
		dword bytes = size;
		if(bytes > zeroBuffer.Size()) bytes = zeroBuffer.Size();
		if(file->Write(zeroBuffer.GetBuffer(), bytes) != bytes)
		{
			exportStageCount = ARRSIZE(exportSteps);
			isErrors = true;
			Error & error = errors[errors.Add()];
			error.code = ec_cantwrite;
			error.id.Reset();
			break;
		}
		size -= bytes;
	}
	return true;
}


//Инициализация подготовки звуков, waveinfo, и таблицы имён
void SndExporter::InitProcessFillTableSounds()
{
	if(progressOutput)
	{
		progressOutput->data += options->GetString(SndOptions::s_export_msg_5_1);
	}
	currentIndexCounter = 0;
	currentCounter1 = 0;
}

//Подготовка звуков, waveinfo, и таблицы имён
bool SndExporter::ProcessFillTableSounds()
{
	if(currentIndexCounter == exportSounds.Size())
	{
		//Копируем имена звуков
		memcpy(bankXboxNames, names.GetBuffer(), names.Size());
		memcpy(bankPCNames, names.GetBuffer(), names.Size());
		currentIndexCounter++;
		if(progressOutput)
		{
			progressOutput->data += options->GetString(SndOptions::s_export_msg_5_2);
		}
	}
	if(currentIndexCounter >= exportSounds.Size())
	{
		return true;
	}
	if(progressOutput)
	{
		progressOutput->data += options->GetString(SndOptions::s_export_msg_5_3);
		progressOutput->data += (currentIndexCounter + 1);
		progressOutput->data += options->GetString(SndOptions::s_export_msg_of);
		progressOutput->data += exportSounds.Size();
	}
	//Данные с которыми работаем
	SoundInfo & sinfo = exportSounds[currentIndexCounter];
	ProjectSound * sound = project->SoundGet(sinfo.id);
	Assert(sound);
	SoundBankFileSound & soundXbox = bankXboxSounds[currentIndexCounter];
	SoundBankFileSound & soundPC = bankPCSounds[currentIndexCounter];
	AddDebugRecord(sound->GetId(), &soundPC);
	//Setup
	const SoundBaseParams * sbp = sound->GetCurrentBaseParams();
	Assert(sbp);
	AddDebugRecord(sound->GetGlobalBaseParams(), &soundPC.setup);
	SoundBankFileSetup & setupXbox = soundXbox.setup;
	SoundBankFileSetup & setupPC = soundPC.setup;
	setupXbox.maxCount = sbp->maxCount;
	setupPC.maxCount = sbp->maxCount;
	long priority = sbp->priority + SoundBankFileSetup::mode_priority_base;
	Assert(priority >= SoundBankFileSetup::mode_priority_base);
	Assert(priority < SoundBankFileSetup::mode_priority_base + SoundBankFileSetup::mode_priority_range);
	dword mode = priority & SoundBankFileSetup::mode_priority_mask;
	//Методы выбора волны
	switch(sbp->selectMethod)
	{
	case sbpc_select_rnd:
		mode |= SoundBankFileSetup::mode_select_rnd;
		break;
	case sbpc_select_queue:
		mode |= SoundBankFileSetup::mode_select_queue;
		break;
	case sbpc_select_sequence:
		mode |= SoundBankFileSetup::mode_select_sequence;
		break;
	default:
		Assert(false);
	};
	//Зацикленные звуки
	switch(sbp->loopMode)
	{
	case sbpc_loop_disable:
		mode |= SoundBankFileSetup::mode_loop_diasble;
		break;
	case sbpc_loop_one_wave:
		mode |= SoundBankFileSetup::mode_loop_one_wave;
		break;
		/*
		case sbpc_loop_sel_wave_rnd:
		mode |= SoundBankFileSetup::mode_loop_sel_wave_rnd;
		break;
		case sbpc_loop_sel_wave_que:
		mode |= SoundBankFileSetup::mode_loop_sel_wave_que;
		break;
		*/
	default:
		Assert(false);
	};
	//Какие эффекты применять к звуку
	switch(sbp->fx)
	{
	case sbpc_fx_full:
		mode |= SoundBankFileSetup::mode_fx_full;
		break;
	case sbpc_fx_premaster:
		mode |= SoundBankFileSetup::mode_fx_premaster;
		break;
	case sbpc_fx_master:
		mode |= SoundBankFileSetup::mode_fx_master;
		break;
	case sbpc_fx_music:
		mode |= SoundBankFileSetup::mode_fx_music;
		break;
	default:
		Assert(false);
	};
	setupXbox.mode = mode;
	setupPC.mode = mode;
	//Attenuation
	const SoundAttGraph * satt = sound->GetCurrentAttenuation();
	AddDebugRecord(sound->GetGlobalAttenuation(), &soundPC.att);
	SoundBankFileAttenuation & attXbox = soundXbox.att;
	SoundBankFileAttenuation & attPC = soundPC.att;
	for(long j = 0; j < 4; j++)
	{
		attXbox.c[j] = satt->c[j];
		attPC.c[j] = satt->c[j];
	}
	Assert(satt->minDist <= satt->maxDist);
	float minD2 = satt->minDist*satt->minDist;
	float maxD2 = satt->maxDist*satt->maxDist;
	float delta = maxD2 - minD2;
	float kNorm2 = (delta > 1e-6f) ? 1.0f/delta : 0.0f;
	attXbox.minDist2 = minD2;
	attPC.minDist2 = minD2;
	attXbox.maxDist2 = maxD2;
	attPC.maxDist2 = maxD2;
	attXbox.kNorm2 = kNorm2;
	attPC.kNorm2 = kNorm2;
	//Waves
	dword wavesCount = sound->GetWavesCount();
	soundXbox.waves = &bankXboxInfos[currentCounter1];
	soundXbox.wavesCount = wavesCount;
	soundPC.waves = &bankPCInfos[currentCounter1];
	soundPC.wavesCount = wavesCount;
	const float minVolume = SndExporter_MinExportVolume;
	const float maxVolume = SndExporter_MaxExportVolume;
	float kNorm = sound->GetKSumWeights();
	for(dword j = 0; j < wavesCount; j++)
	{
		ProjectSound::Wave & sndWave = sound->GetWave(j);
		SoundBankFileWaveInfo & winfoXbox = bankXboxInfos[currentCounter1];
		SoundBankFileWaveInfo & winfoPC = bankPCInfos[currentCounter1];
		float soundVolume = sbp->volume;
		float waveVolume = Clampf(sndWave.volume*soundVolume, minVolume, maxVolume);
		winfoXbox.volume = waveVolume;
		winfoPC.volume = waveVolume;
		float prob = Clampf(sndWave.weight*kNorm);
		winfoXbox.probability = prob;
		winfoPC.probability = prob;
		winfoXbox.playTime = sndWave.time;
		winfoPC.playTime = sndWave.time;
		winfoXbox.wave = null;
		winfoPC.wave = null;
		if(sndWave.waveId.IsValidate())
		{
			for(long windex = 0; windex < exportWaves; windex++)
			{
				if(exportWaves[windex].id == sndWave.waveId)
				{
					winfoXbox.wave = &bankXboxWaves[windex];
					winfoPC.wave = &bankPCWaves[windex];
					break;
				}
			}
			Assert(winfoXbox.wave);
			Assert(winfoPC.wave);
		}
		currentCounter1++;
	}
	//Очереди для выборок волн
	if(sinfo.queueSelect < 0)
	{
		Assert(sbp->selectMethod != sbpc_select_queue);
		soundXbox.squeue = null;
		soundXbox.selectSequenceCount = 0;
		soundPC.squeue = null;
		soundPC.selectSequenceCount = 0;
	}else{
		Assert(sbp->selectMethod == sbpc_select_queue);
		soundXbox.squeue = (SoundBankFileWaveQueue *)(bankXboxQueues + sinfo.queueSelect);
		soundPC.squeue = (SoundBankFileWaveQueue *)(bankPCQueues + sinfo.queueSelect);
	}
	/*
	if(sinfo.queueLoop < 0)
	{
		soundXbox.lqueue = null;
		soundPC.lqueue = null;
	}else{
		soundXbox.lqueue = (SoundBankFileWaveQueue *)(bankXboxQueues + sinfo.queueLoop);
		soundPC.lqueue = (SoundBankFileWaveQueue *)(bankPCQueues + sinfo.queueLoop);
	}*/
	//Имя
	const ExtName & sname = sound->GetName();
	soundXbox.name = (char *)(bankXboxNames + sinfo.name);
	soundPC.name = (char *)(bankPCNames + sinfo.name);
	soundXbox.nameHash = sname.hash;
	soundPC.nameHash = sname.hash;
	soundXbox.nameLen = sname.len;
	soundPC.nameLen = sname.len;
	soundXbox.next = null;
	soundPC.next = null;
	//Расширенные данные
	soundXbox.extraData = null;
	soundPC.extraData = null;
	soundXbox.extraCount = 0;
	soundPC.extraCount = 0;
	//Увеличиваем индекс
	currentIndexCounter++;
	return false;
}

//Инициализация заполнения таблицы описания волн банка
void SndExporter::InitProcessFillTableWaves()
{
	if(progressOutput)
	{
		progressOutput->data += options->GetString(SndOptions::s_export_msg_8_1);
	}
	//Убедимся, что область волн выровнена
	Assert(((bankXboxWavesData - ((byte *)bankXboxHeader)) & 0xf) == 0);
	Assert(((bankPCWavesData - ((byte *)bankPCHeader)) & 0xf) == 0);
	currentIndexCounter = 0;
}

//Таблицы описания волн банка
bool SndExporter::ProcessFillTableWaves()
{	
	if(currentIndexCounter == exportWaves.Size())
	{
		if(progressOutput)
		{
			progressOutput->data += options->GetString(SndOptions::s_export_msg_8_2);
		}
		currentIndexCounter++;
	}
	if((dword)currentIndexCounter >= exportWaves.Size())
	{
		return true;
	}	
	if(progressOutput)
	{
		progressOutput->data += options->GetString(SndOptions::s_export_msg_8_3);
		progressOutput->data += (currentIndexCounter + 1);
		progressOutput->data += options->GetString(SndOptions::s_export_msg_of);
		progressOutput->data += exportWaves.Size();
	}
	//Указатели и ссылки на описывающии структуры
	dword i = currentIndexCounter;
	currentIndexCounter++;
	WaveInfo & winfo = exportWaves[i];
	ProjectWave::ExportWavesPart * exportWave = (ProjectWave::ExportWavesPart *)winfo.exportWaves.GetBuffer();
	ProjectWave::ExportPhonemesPart * exportPhonemes = (ProjectWave::ExportPhonemesPart *)winfo.exportPhonemes.GetBuffer();
	SoundBankFileWave & waveXbox = bankXboxWaves[i];
	SoundBankFileWave & wavePC = bankPCWaves[i];
	ProjectWave * prjWave = project->WaveGet(winfo.id);
	Assert(prjWave);
	AddDebugRecord(prjWave->GetId(), &wavePC);
	//Структура описывающая формат волны WAVEFORMATEX или XMA2WAVEFORMAT
	waveXbox.waveFormatInfo = bankXboxWavesDataFormat + winfo.formatOffsetXbox;
	wavePC.waveFormatInfo = bankPCWavesDataFormat + winfo.formatOffsetPC;
	//Внутреннее описания формата волны
	Assert(exportWave->exportFormatXbox);
	Assert(exportWave->exportFormatPC);
	waveXbox.format = exportWave->exportFormatXbox;
	waveXbox.samplesCount = exportWave->waveSmplsForXbox;
	waveXbox.unimportantTime = prjWave->GetUnimportantTime();
	waveXbox.maxNormalizedAmp = prjWave->GetPickAmplitude();
	wavePC.format = exportWave->exportFormatPC;
	wavePC.samplesCount = exportWave->waveSmplsForPC;
	wavePC.unimportantTime = prjWave->GetUnimportantTime();
	wavePC.maxNormalizedAmp = prjWave->GetPickAmplitude();
	//Звуковые данные волны
	waveXbox.data = bankXboxWavesData + winfo.waveOffsetXbox;
	wavePC.data = bankPCWavesData + winfo.waveOffsetPC;
	Assert(((waveXbox.data - ((byte *)bankXboxHeader)) & 0xf) == 0);
	Assert(((wavePC.data - ((byte *)bankPCHeader)) & 0xf) == 0);	
	//Копируем данные
	//xbox
	byte * waveData = ((byte *)exportWave) + sizeof(ProjectWave::ExportWavesPart);
	Assert(((byte *)waveXbox.waveFormatInfo) - data.GetBuffer() + exportWave->waveformatSizeXbox < dataSize);
	memcpy((byte *)waveXbox.waveFormatInfo, waveData, exportWave->waveformatSizeXbox);
	waveData += exportWave->waveformatSizeXbox;
	Assert(waveXbox.data - data.GetBuffer() + exportWave->waveSizeForXbox < dataSize);
	//memcpy((byte *)waveXbox.data, waveData, exportWave->waveSizeForXbox);
	dword offsetXbox = (byte *)waveXbox.data - (byte *)bankXboxHeader + sizeof(SoundBankFileId);
	SaveData(offsetXbox, waveData, exportWave->waveSizeForXbox);
	//PC
	waveData += exportWave->waveSizeForXbox;
	Assert(((byte *)wavePC.waveFormatInfo) - data.GetBuffer() + exportWave->waveformatSizePC < dataSize);
	memcpy((byte *)wavePC.waveFormatInfo, waveData, exportWave->waveformatSizePC);
	waveData += exportWave->waveformatSizePC;
	Assert(wavePC.data - data.GetBuffer() + exportWave->waveSizeForPC < dataSize);
	//memcpy((byte *)wavePC.data, waveData, exportWave->waveSizeForPC);
	dword offsetPC = (byte *)wavePC.data - (byte *)bankPCHeader + xboxPartSize + sizeof(SoundBankFileId);
	SaveData(offsetPC, waveData, exportWave->waveSizeForPC);
	//Размер звуковых данных в байтах
	waveXbox.dataSize = exportWave->waveSizeForXbox;
	wavePC.dataSize = exportWave->waveSizeForPC;
	//Фонемы
	if(winfo.exportPhonemes.Size() > 0)
	{
		waveXbox.extraData = &bankXboxExtras[bankExtrasCount];
		wavePC.extraData = &bankPCExtras[bankExtrasCount];
		waveXbox.extraCount = 1;
		wavePC.extraCount = 1;
		//Заголовок 
		bankXboxExtras[bankExtrasCount].id = sbf_extra_phonemes;
		bankPCExtras[bankExtrasCount].id = sbf_extra_phonemes;
		bankXboxExtras[bankExtrasCount].data = &bankXboxExtrasData[bankExtrasOffset];
		bankPCExtras[bankExtrasCount].data = &bankPCExtrasData[bankExtrasOffset];
		//Заполняем фонемы для обоих версий
		SoundBankFilePhonemes * xboxPhonemes = (SoundBankFilePhonemes *)bankXboxExtras[bankExtrasCount].data;
		SoundBankFilePhonemes * pcPhonemes = (SoundBankFilePhonemes *)bankPCExtras[bankExtrasCount].data;
		xboxPhonemes->version = SoundBankFilePhonemes::currentVersion;
		pcPhonemes->version = SoundBankFilePhonemes::currentVersion;
		xboxPhonemes->framesCount = exportPhonemes->count;
		pcPhonemes->framesCount = exportPhonemes->count;
		dword phonemesCount = coremax(exportPhonemes->count, 1);
		for(dword i = 0; i < phonemesCount; i++)
		{
			dword phonemeTimeInMs = exportPhonemes->phoneme[i].time;
			byte phonemeId = exportPhonemes->phoneme[i].id;
			Assert(phonemeId <= SoundBankFilePhonemes::phoneme_id_max);
			dword packPhoneme = SoundBankFilePhonemes::PackPhoneme(phonemeTimeInMs, phonemeId);
			xboxPhonemes->phoneme[i] = packPhoneme;
			pcPhonemes->phoneme[i] = packPhoneme;
		}
		//Укажем на следующее место
		bankExtrasCount++;
		bankExtrasOffset += SoundBankFilePhonemes::GetSize(exportPhonemes->count);
	}else{
		waveXbox.extraData = null;
		wavePC.extraData = null;
		waveXbox.extraCount = 0;
		wavePC.extraCount = 0;
	}
	return false;
}

//Таблица поиска звуков по имени
bool SndExporter::ProcessFillTableEntry()
{
	if(progressOutput)
	{
		progressOutput->data += options->GetString(SndOptions::s_export_msg_9_1);
	}
	for(dword i = 0; i < entryTableSize; i++)
	{
		bankXboxEntry[i] = null;
		bankPCEntry[i] = null;
	}
	dword emask = entryTableSize - 1;
	for(long i = 0; i < exportSounds; i++)
	{
		//Зановим хвох звук в таблицу
		dword eindex = bankXboxSounds[i].nameHash & emask;
		if(bankXboxEntry[eindex])
		{
			for(SoundBankFileSound * snd = bankXboxEntry[eindex]; snd->next; snd = snd->next);
			snd->next = &bankXboxSounds[i];
		}else{
			bankXboxEntry[eindex] = &bankXboxSounds[i];
		}		
		//Зановим рс звук в таблицу
		eindex = bankPCSounds[i].nameHash & emask;
		if(bankPCEntry[eindex])
		{
			for(SoundBankFileSound * snd = bankPCEntry[eindex]; snd->next; snd = snd->next);
			snd->next = &bankPCSounds[i];
		}else{
			bankPCEntry[eindex] = &bankPCSounds[i];
		}		
	}
	return true;
}

//Заполнение таблиц звуков
bool SndExporter::ProcessFillHeaders()
{
	if(progressOutput)
	{
		progressOutput->data += options->GetString(SndOptions::s_export_msg_10_1);
	}
	//Уникальный идентификатор звукового банка
	for(dword i = 0; i < 4; i++)
	{
		bankXboxHeader->uniqueId[i] = soundBankId.data[i];
		bankPCHeader->uniqueId[i] = soundBankId.data[i];
	}
	//Звуки
	bankXboxHeader->sounds = bankXboxSounds;
	bankPCHeader->sounds = bankPCSounds;
	//Количество звуков
	bankXboxHeader->soundsCount = exportSounds.Size();
	bankPCHeader->soundsCount = exportSounds.Size();
	//Описание волн подвязаных к звукам
	bankXboxHeader->winfos = bankXboxInfos;
	bankPCHeader->winfos = bankPCInfos;	
	//Размер таблицы волн
	bankXboxHeader->winfosCount = waveInfoCount;
	bankPCHeader->winfosCount = waveInfoCount;	
	//Волны, используещиеся в банке
	bankXboxHeader->waves = bankXboxWaves;
	bankPCHeader->waves = bankPCWaves;	
	//Количество волн используемых в банке
	bankXboxHeader->wavesCount = exportWaves.Size();
	bankPCHeader->wavesCount = exportWaves.Size();	
	//Входная таблица поиска звуков по имени
	bankXboxHeader->entry = bankXboxEntry;
	bankPCHeader->entry = bankPCEntry;
	//Маска входа через таблицу поиска
	bankXboxHeader->mask = entryTableSize - 1;
	bankPCHeader->mask = entryTableSize - 1;
	//Таблица отладки
	bankXboxHeader->ids = null;
	bankPCHeader->ids = bankPCDebugTable;
	bankXboxHeader->idsCount = 0;
	bankPCHeader->idsCount = bankPCDebugTableCounter;
	//Таблица расширений
	bankXboxHeader->extras = bankXboxExtras;
	bankPCHeader->extras = bankPCExtras;
	//Количество расширений
	bankXboxHeader->extrasCount = bankExtrasCount;
	bankPCHeader->extrasCount = bankExtrasCount;
	return true;
}

//Бинарная подготовка файла
bool SndExporter::ProcessPrepare()
{	
	if(progressOutput)
	{
		progressOutput->data += options->GetString(SndOptions::s_export_msg_11_1);
	}
	//Фиксим времена в волнах звуков
	for(dword i = 0; i < bankXboxHeader->winfosCount; i++)
	{
		SoundBankFileWaveInfo & winfoXbox = bankXboxInfos[i];
		if(winfoXbox.wave)
		{
			float xbFreq = (float)(winfoXbox.wave->format & SoundBankFileWave::f_freq_mask);
			Assert(xbFreq > 0.0f);
			float xbMinTime = winfoXbox.wave->samplesCount/xbFreq;
			winfoXbox.playTime = coremax(winfoXbox.playTime, xbMinTime);			
			Assert(winfoXbox.playTime > 0.0f);
		}else{
			if(winfoXbox.playTime <= 0.0f)
			{
				winfoXbox.playTime = 0.00001f;
			}
		}		
	}
	for(dword i = 0; i < bankPCHeader->winfosCount; i++)
	{
		SoundBankFileWaveInfo & winfoPC = bankPCInfos[i];
		if(winfoPC.wave)
		{
			float pcFreq = (float)(winfoPC.wave->format & SoundBankFileWave::f_freq_mask);
			Assert(pcFreq > 0.0f);
			float pcMinTime = winfoPC.wave->samplesCount/pcFreq;
			winfoPC.playTime = coremax(winfoPC.playTime, pcMinTime);
			Assert(winfoPC.playTime > 0.0f);
		}else{
			if(winfoPC.playTime <= 0.0f)
			{
				winfoPC.playTime = 0.00001f;
			}
		}		
	}
	//Фиксим указатели и тп
	bankId->Init(sizeof(SoundBankFileId) + xboxPartSize);
	bankXboxHeader->Prepare(true);
	bankPCHeader->Prepare(false);
	//Фиксим расположение отладочной таблицы
	Assert(bankXboxHeader->ids == null);
	Assert(bankXboxHeader->idsCount == 0);
	bankPCHeader->ids = (SoundBankFileObjectId *)((byte *)null + bankPCDebugTableOffset);
	return true;
}

//Сохранение в файл
bool SndExporter::ProcessSaveToFile()
{	
	if(progressOutput)
	{
		progressOutput->data += options->GetString(SndOptions::s_export_msg_12_1);
	}
	Assert(file);
	//Сохраняем информацию для xbox части и заголовок банка
	dword sizeXbox = sizeof(SoundBankFileId) + xboxPartSize - xboxWavesDataSize;
	SaveData(0, data.GetBuffer(), sizeXbox);
	//Сохраняем информацию для pc части
	dword pcOffset = sizeof(SoundBankFileId) + xboxPartSize;
	dword sizePC = bankPCDebugTableOffset - pcWavesDataSize;
	SaveData(pcOffset, bankPCHeader, sizePC);
	//Сохраняем отладочную информацию
	SaveData(pcOffset + bankPCDebugTableOffset, bankPCDebugTable, bankPCDebugTableSize*sizeof(SoundBankFileObjectId));
	//Освобождение ресурсов
	data.Free();
#ifdef GAME_RUSSIAN
	ConvertBankToPConly(file);
#endif
	file->Release();
	file = null;
	return true;
}

//Инициализация процесса бездействия
void SndExporter::InitProcessIdle()
{
}

//Процесс бездействия
bool SndExporter::ProcessIdle()
{
	Sleep(1);
	return true;
}

//Добавить индекс к массиву, если такой есть то ничего не делать
void SndExporter::AddUniqueWave(const UniqId & id, bool isNeedPhoneme)
{
	if(!id.IsValidate())
	{
		return;
	}
	dword count = exportWaves.Size();
	WaveInfo * ptr = exportWaves.GetBuffer();
	for(dword i = 0; i < count; i++)
	{
		if(ptr[i].id == id)
		{
			ptr[i].needPhonemes |= isNeedPhoneme;
			return;
		}
	}
	WaveInfo & wi = exportWaves[exportWaves.Add()];
	wi.id = id;
	wi.needPhonemes = isNeedPhoneme;
	wi.isStartExport = false;
	wi.exportWaves.Empty();
	wi.exportPhonemes.Empty();
	wi.waveOffsetXbox = -1;
	wi.formatOffsetXbox = -1;		
	wi.waveOffsetPC = -1;
	wi.formatOffsetPC = -1;	
	wi.phonemesOffset = -1;	
}

//Добавить запись в отладочную таблицу
void SndExporter::AddDebugRecord(const UniqId & id, void * ptr)
{
	Assert(bankPCDebugTable);
	SoundBankFileObjectId & element = bankPCDebugTable[bankPCDebugTableCounter++];
	Assert(bankPCDebugTableCounter <= bankPCDebugTableSize);
	Assert(((byte *)&element) + sizeof(SoundBankFileObjectId) <= data.GetBuffer() + data.Size());
	element.objectPtr = (byte *)ptr;
	element.data[0] = id.data[0];
	element.data[1] = id.data[1];
	element.data[2] = id.data[2];
	element.data[3] = id.data[3];
}

//Сохранить блоу данных в файл банка
bool SndExporter::SaveData(dword offset, void * ptr, dword size)
{
	if(!size)
	{
		return true;
	}
	Assert(file != null);
	if(file->SetPos(offset) == offset)
	{
		if(file->Write(ptr, size) == size)
		{
			return true;
		}
	}
	isErrors = true;
	Error & error = errors[errors.Add()];
	error.code = ec_cantwrite;
	error.id.Reset();
	return false;	
}

//Преобразовать банк только для РС (GAME_RUSSIAN)
void SndExporter::ConvertBankToPConly(IFile * file)
{
#ifdef GAME_RUSSIAN
	//Оставить только РС чанк
	file->SetPos(0);
	dword fileSize = file->Size();
	//Загружаем идентификационный заголовок
	SoundBankFileId sbfId;
	Verify(file->Read(&sbfId, sizeof(sbfId)) == sizeof(sbfId));
	//Загружаем РС часть
	dword dataOffset = sbfId.GetPCChankOffset();
	dword dataSize = sbfId.GetPCChankSize(fileSize);
	byte * buffer = NEW byte[dataSize];
	Verify(file->SetPos(dataOffset) == dataOffset);
	Verify(file->Read(buffer, dataSize) == dataSize);
	//Разворачиваем в памяти
	SoundBankFileHeader * header = (SoundBankFileHeader *)buffer;
	Verify(header->Restore(dataSize));
	//Удаляем отладочные данные
	if(header->ids)
	{
		header->ids = null;
		dataSize -= header->idsCount*sizeof(SoundBankFileObjectId);
	}
	//Кодируем волны
	for(dword i = 0; i < header->wavesCount; i++)
	{
		header->waves[i].EncodeWaveData();
	}
	//Подготавливаем к сохранению в файл
	header->Prepare(false);
	//Стираем файл
	Verify(file->SetPos(0) == 0);
	file->Truncate();
	//Сохраняем данные
	file->Write(buffer, dataSize);
	//Удаляем буфер
	delete buffer;
#endif
}