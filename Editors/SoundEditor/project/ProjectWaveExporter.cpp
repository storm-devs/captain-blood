

#include "ProjectWaveExporter.h"
#include "..\..\..\Common_h\data_swizzle.h"
#include "..\SndOptions.h"
#include <Xaudio2.h>
#include <xma2defs.h>


ProjectWaveExporter ProjectWaveExporter::globalObject;



ProjectWaveExporter::ProjectWaveExporter() : exportQueue(_FL_)
{
	isWork = false;
	threadPCExit = false;
	threadXboxExit = false;
	threadPhonemesExit = false;
	activeWorkModeCounter = 0;
	//Создаём и запускаем потоки конвертации данных
	threadPCWaveExport = null;
	threadXboxWaveExport = null;
	threadPhonemesExport = null;
}

ProjectWaveExporter::~ProjectWaveExporter()
{
}

//Запустить экспортёр
void ProjectWaveExporter::StartExporter()
{
	AssertCoreThread
	//Разрешаем работу
	globalObject.isWork = true;
	//Очистить рабочию папку
	dword flags = find_no_recursive | find_no_mirror_files | find_no_files_from_packs;
	IFinder * finder = options->fileService->CreateFinder(options->pathTempWorkFolder.c_str(), "*.*", flags, _FL_);
	if(finder)
	{
		for(dword i = 0; i < finder->Count(); i++)
		{
			options->fileService->Delete(finder->FilePath(i));
		}
		finder->Release();
	}
	//Создаём рабочие пути
	options->fileService->CreateFolder(options->pathTempWorkFolder.c_str());
	options->fileService->CreateFolder(options->pathTempSaveFolder.c_str());
	//Проверить состояние объекта
	Assert(globalObject.threadPCWaveExport == null);
	Assert(globalObject.threadXboxWaveExport == null);
	Assert(globalObject.threadPhonemesExport == null);
	//Создаём и запускаем потоки конвертации данных
	globalObject.threadPCWaveExport = ::CreateThread(null, 0, ThreadPCWaveExport, null, CREATE_SUSPENDED, null);
	globalObject.threadXboxWaveExport = ::CreateThread(null, 0, ThreadXboxWaveExport, null, CREATE_SUSPENDED, null);
	globalObject.threadPhonemesExport = ::CreateThread(null, 0, ThreadPhonemesExport, null, CREATE_SUSPENDED, null);
	Assert(globalObject.threadPCWaveExport != null);
	Assert(globalObject.threadXboxWaveExport != null);
	Assert(globalObject.threadPhonemesExport != null);	
	globalObject.activeWorkModeCounter = 1;
	ReleaseActiveMode();
	::ResumeThread(globalObject.threadPCWaveExport);
	::ResumeThread(globalObject.threadXboxWaveExport);
	::ResumeThread(globalObject.threadPhonemesExport);
}

//Запустить экспортёр
void ProjectWaveExporter::StopExporter()
{
	AssertCoreThread
	if(globalObject.isWork)
	{
		//Ставим нормальный приоритет потокам
		SetActiveMode();
		//Указываем что надо остановить циклы выгрузки
		globalObject.queueAccessor.Enter();
		globalObject.isWork = false;
		globalObject.queueAccessor.Leave();
		if(globalObject.threadPCWaveExport != null)
		{
			Assert(globalObject.threadPCWaveExport != null);
			Assert(globalObject.threadXboxWaveExport != null);
			Assert(globalObject.threadPhonemesExport != null);
			//Очищаем очередь волн						
			while(true)
			{
				ProjectWave * wave = null;
				globalObject.queueAccessor.Enter();
				if(globalObject.exportQueue.Size() > 0)
				{
					wave = globalObject.exportQueue[0].wave;
				}
				globalObject.queueAccessor.Leave();
				if(wave)
				{
					ProjectWaveExporter::DelWaveFromQueue(wave);
					Sleep(10);
				}else{
					break;
				}
			}
			//Ждём 10 секунд в надежде что всё чистенько выйдет
			for(long i = 0; i < 1000; i++)
			{
				Sleep(10);
				globalObject.queueAccessor.Enter();
				if(globalObject.threadPCExit && globalObject.threadXboxExit && globalObject.threadPhonemesExit)
				{
					globalObject.queueAccessor.Leave();
					break;
				}
				globalObject.queueAccessor.Leave();
			}
			//...если потоки не остановились, предоставим их системе
		}else{
			Assert(globalObject.threadPCWaveExport == null);
			Assert(globalObject.threadXboxWaveExport == null);
			Assert(globalObject.threadPhonemesExport == null);
		}
	}
}

//Установить нормальный режим работы
void ProjectWaveExporter::SetActiveMode()
{
	AssertCoreThread
	if(globalObject.isWork)
	{
		if(globalObject.activeWorkModeCounter == 0)
		{		
			::SetThreadPriority(globalObject.threadPCWaveExport, THREAD_PRIORITY_NORMAL);
			::SetThreadPriority(globalObject.threadXboxWaveExport, THREAD_PRIORITY_NORMAL);
			::SetThreadPriority(globalObject.threadPhonemesExport, THREAD_PRIORITY_NORMAL);
		}
		globalObject.activeWorkModeCounter++;
	}
}

//Установить фоновый режим работы
void ProjectWaveExporter::ReleaseActiveMode()
{
	if(globalObject.isWork)
	{
		AssertCoreThread
		globalObject.activeWorkModeCounter--;
		Assert(globalObject.activeWorkModeCounter >= 0);
		if(globalObject.activeWorkModeCounter == 0)
		{
			::SetThreadPriority(globalObject.threadPCWaveExport, THREAD_PRIORITY_IDLE);
			::SetThreadPriority(globalObject.threadXboxWaveExport, THREAD_PRIORITY_IDLE);
			::SetThreadPriority(globalObject.threadPhonemesExport, THREAD_PRIORITY_IDLE);
		}
	}
}

//Экспорт волны для предварительного прослушивания
byte * ProjectWaveExporter::WavePreviewMake(ProjectWave * wave, bool isPC, ErrorId errorCode)
{
	AssertCoreThread
	WaveDataExportParams params;
	//Экспортим PC
	params.isPC = isPC;
	params.isMakePreview = true;
	dword size = 0;
	return ConvertWaveData(wave, size, &params, errorCode);
}

//Получить указатель на структуру WAVEFORMATEX в буфере предпрослушивания
WAVEFORMATEX * ProjectWaveExporter::WavePreviewGetFormat(byte * buffer)
{
	if(!buffer) return null;
	WaveDataInfo * info = (WaveDataInfo *)buffer;
	Assert(info->waveExSize >= sizeof(WAVEFORMATEX));
	return (WAVEFORMATEX *)(buffer + sizeof(WaveDataInfo));
}

//Получить указатель на данные в буфере предпрослушивания
byte * ProjectWaveExporter::WavePreviewGetData(byte * buffer, dword & dataSizeInBytes)
{	
	if(!buffer)
	{
		dataSizeInBytes = 0;
		return null;
	}
	WaveDataInfo * info = (WaveDataInfo *)buffer;
	dataSizeInBytes = info->waveDataSize;
	return buffer + sizeof(WaveDataInfo) + info->waveExSize;
}

//Получить экспортный размер посчитаный при генерации данных предпрослушивания
dword ProjectWaveExporter::WavePreviewGetDataSize(byte * buffer)
{
	if(!buffer)
	{
		return 0;
	}
	WaveDataInfo * info = (WaveDataInfo *)buffer;
	Assert(info->sizeofAdds == 4);
	return (dword &)info->adds[0];
}

//Добавить волну в очередь экспорта
void ProjectWaveExporter::AddWaveToQueue(ProjectWave * wave, bool isNeedPhonemes, bool isIdleExport)
{	
	if(!globalObject.isWork)
	{
		return;
	}
	AssertCoreThread
	Assert(globalObject.threadPCWaveExport != null);
	Assert(globalObject.threadXboxWaveExport != null);
	Assert(globalObject.threadPhonemesExport != null);
	Assert(wave);
	if(wave->GetErrorCode() != ErrorId::ok)
	{
		//Экспорт волны уже был, и закончился ошибкой
		return;
	}
	//api->Trace("ProjectWaveExporter::AddWaveToQueue Enter 1 start");
	globalObject.queueAccessor.Enter();
	//api->Trace("ProjectWaveExporter::AddWaveToQueue Enter 1 end");
	if(isIdleExport && globalObject.exportQueue.Size() > 0)
	{
		//Очередь не пустая, пропускаем попытку фонового экспорта
		//api->Trace("ProjectWaveExporter::AddWaveToQueue Leave 1 start");
		globalObject.queueAccessor.Leave();
		//api->Trace("ProjectWaveExporter::AddWaveToQueue Leave 1 end");
		return;
	}
	//Проверяем на наличие в очереди
	for(dword i = 0; i < globalObject.exportQueue.Size(); i++)
	{
		QuiueRecord & qr = globalObject.exportQueue[i];
		if(qr.wave == wave)
		{
			if(qr.markForDelete)
			{
				//Удаляем из очереди
				//api->Trace("ProjectWaveExporter::AddWaveToQueue Leave 3 start");
				globalObject.queueAccessor.Leave();
				//api->Trace("ProjectWaveExporter::AddWaveToQueue Leave 3 end");
				DelWaveFromQueue(wave);
				//api->Trace("ProjectWaveExporter::AddWaveToQueue Enter 2 start");
				globalObject.queueAccessor.Enter();
				//api->Trace("ProjectWaveExporter::AddWaveToQueue Enter 2 end");
				break;
			}
			if(isNeedPhonemes)
			{
				if(qr.exports[e_phonemes].isDone)
				{
					if(!qr.exports[e_phonemes].buffer)
					{
						qr.exports[i].inProcess = false;
						qr.exports[i].isDone = false;
						qr.exports[i].isError = false;
						qr.exports[i].buffer = null;
						qr.exports[i].size = 0;
						globalObject.queueAccessor.Leave();
						return;
					}
				}
			}
		}
	}
	//Добавляем новую запись
	QuiueRecord & qr = globalObject.exportQueue[globalObject.exportQueue.Add()];
	qr.wave = wave;
	qr.markForDelete = false;
	qr.waitToSave = false;
	qr.saveProcess = false;
	qr.errorCode = options->ErrorOut(null, false, "");
	for(dword i = 0; i < e_count; i++)
	{
		qr.exports[i].inProcess = false;
		qr.exports[i].isDone = false;
		qr.exports[i].isError = false;
		qr.exports[i].buffer = null;
		qr.exports[i].size = 0;
	}
	if(!isNeedPhonemes)
	{
		qr.exports[e_phonemes].isDone = true;
	}
	qr.wave->ProcessExportSetToQueue();
	//api->Trace("ProjectWaveExporter::AddWaveToQueue Leave 2 start");
	globalObject.queueAccessor.Leave();
	//api->Trace("ProjectWaveExporter::AddWaveToQueue Leave 2 end");
}

//Удалить волну из очереди экспорта
void ProjectWaveExporter::DelWaveFromQueue(ProjectWave * wave)
{
	Assert(wave);
	AssertCoreThread
	bool isNeedRepeat = true;
	while(true)
	{
		//api->Trace("ProjectWaveExporter::DelWaveFromQueue Enter start");
		globalObject.queueAccessor.Enter();
		//api->Trace("ProjectWaveExporter::DelWaveFromQueue Enter end");
		for(dword i = 0; i < globalObject.exportQueue.Size(); i++)
		{
			QuiueRecord & qr = globalObject.exportQueue[i];
			if(qr.wave == wave)
			{
				//Если волна не в работе, то удаляем
				for(dword j = 0; j < e_count; j++)
				{
					if(qr.exports[j].inProcess) break;
				}
				if(j >= e_count && !qr.saveProcess && !qr.waitToSave)
				{
					//Запретим начинать экспорт для данной волны
					qr.markForDelete = true;
					DeleteQueueElement(i);
					isNeedRepeat = false;
				}
			}
		}
		if(i >= globalObject.exportQueue.Size())
		{
			isNeedRepeat = false;
		}
		//api->Trace("ProjectWaveExporter::DelWaveFromQueue Leave start");
		globalObject.queueAccessor.Leave();
		//api->Trace("ProjectWaveExporter::DelWaveFromQueue Leave end");
		//Освободив секцию ждём и повторяем попытку или выходим
		if(isNeedRepeat)
		{
			Sleep(10);
		}else{
			break;
		}
	}
	
}

//Выгрузка волны в PC формате
DWORD WINAPI ProjectWaveExporter::ThreadPCWaveExport(LPVOID lpParameter)
{
	while(globalObject.isWork)
	{
		WaveDataExportParams params;
		params.isPC = true;
		params.isMakePreview = false;
		ProcessFunction(e_pcWave, &ProjectWaveExporter::ConvertWaveData, &params);
	}
	globalObject.threadPCExit = true;
	return 0;
}

//Выгрузка волны в Xbox формате
DWORD WINAPI ProjectWaveExporter::ThreadXboxWaveExport(LPVOID lpParameter)
{
	while(globalObject.isWork)
	{
		WaveDataExportParams params;
		params.isPC = false;
		params.isMakePreview = false;
		ProcessFunction(e_xboxWave, &ProjectWaveExporter::ConvertWaveData, &params);
	}
	globalObject.threadXboxExit = true;
	return 0;
}

//Подготовка фонем для волны
DWORD WINAPI ProjectWaveExporter::ThreadPhonemesExport(LPVOID lpParameter)
{
	while(globalObject.isWork)
	{
		ProcessFunction(e_phonemes, &ProjectWaveExporter::PreparePhonemes, null);
	}
	globalObject.threadPhonemesExit = true;	
	return 0;
}

//Шаг экспорта
void ProjectWaveExporter::ProcessFunction(dword exportIndex, byte * (_cdecl * ExportFunction)(ProjectWave * wave, dword & size, void * params, ErrorId errorCode), void * params)
{
	ProjectWave * waveInProcess = null;
	ErrorId workErrorCode;
	//Ищем в очереди волну для экспорта
	//api->Trace("ProjectWaveExporter::ProcessFunction(%i) Enter 1 start", exportIndex);
	globalObject.queueAccessor.Enter();
	//api->Trace("ProjectWaveExporter::ProcessFunction(%i) Enter 1 end", exportIndex);
	for(dword i = 0; i < globalObject.exportQueue.Size(); i++)
	{
		QuiueRecord & qr = globalObject.exportQueue[i];
		if(!qr.markForDelete && !qr.exports[exportIndex].isDone)
		{
			if(qr.exports[exportIndex].buffer == null)
			{
				waveInProcess = qr.wave;
				workErrorCode = qr.errorCode;
				qr.exports[exportIndex].inProcess = true;
				break;
			}
		}
	}
	//api->Trace("ProjectWaveExporter::ProcessFunction Leave 1 start");
	globalObject.queueAccessor.Leave();
	//api->Trace("ProjectWaveExporter::ProcessFunction Leave 1 end");
	//Нет работы, подчистим очередь и освободим ресурсы
	if(!waveInProcess)
	{
		IdleFunction();
		return;
	}
	//Выполняем экспорт
	dword size = 0;
	byte * data = ExportFunction(waveInProcess, size, params, workErrorCode);
	//Сохряняем результат
	//api->Trace("ProjectWaveExporter::ProcessFunction Enter 2 start");
	globalObject.queueAccessor.Enter();
	//api->Trace("ProjectWaveExporter::ProcessFunction Enter 2 end");
	for(dword i = 0; i < globalObject.exportQueue.Size(); i++)
	{
		QuiueRecord & qr = globalObject.exportQueue[i];
		if(qr.wave == waveInProcess)
		{
			Assert(!qr.markForDelete);
			Assert(qr.exports[exportIndex].inProcess);
			Assert(qr.exports[exportIndex].buffer == null);
			qr.exports[exportIndex].inProcess = false;
			qr.exports[exportIndex].isDone = true;
			qr.exports[exportIndex].isError = (data == null);
			qr.exports[exportIndex].buffer = data;
			qr.exports[exportIndex].size = size;
			break;
		}
	}
	if(i >= globalObject.exportQueue.Size() && data)
	{
		delete data;
	}
	//api->Trace("ProjectWaveExporter::ProcessFunction Leave 2 start");
	globalObject.queueAccessor.Leave();
	//api->Trace("ProjectWaveExporter::ProcessFunction Leave 2 end");
	Sleep(0);
}

//Ожидание работы
void ProjectWaveExporter::IdleFunction()
{	
	bool isIdle = true;
	//api->Trace("ProjectWaveExporter::IdleFunction Enter start");
	globalObject.queueAccessor.Enter();
	//api->Trace("ProjectWaveExporter::IdleFunction Enter end");
	dword totalTries = 0;
	string fileName;//, tmpString;
	//Закончим экспорт подготовленных волн
	for(dword i = 0; i < globalObject.exportQueue.Size() && totalTries < globalObject.exportQueue.Size()*10; i++)
	{		
		QuiueRecord & qr = globalObject.exportQueue[i];
		long j = e_count;
		if(!qr.markForDelete)
		{
			for(j = 0; j < e_count; j++)
			{
				if(!qr.exports[j].isDone || qr.exports[j].buffer != null)
				{
					break;
				}
			}
		}
		if(j == e_count)
		{
			//Удаляем из очереди запись
			qr.markForDelete = true;
			DeleteQueueElement(i);
			break;
		}
		//Смотрим, всё ли работы по волнам сделаны
		if(qr.exports[e_pcWave].isDone && qr.exports[e_xboxWave].isDone)
		{
			if(qr.exports[e_pcWave].buffer && qr.exports[e_xboxWave].buffer)
			{
				Assert(qr.exports[e_pcWave].buffer != null);
				Assert(qr.exports[e_xboxWave].buffer != null);
				//Волна готова, собираем данные и сохраняем в файл
				byte * pcData = qr.exports[e_pcWave].buffer;
				Assert(pcData);
				WaveDataInfo & wdiPC = *(WaveDataInfo *)pcData;
				byte * xboxData = qr.exports[e_xboxWave].buffer;
				Assert(xboxData);
				WaveDataInfo & wdiXbox = *(WaveDataInfo *)xboxData;
				dword wavesDataSize = sizeof(ProjectWave::ExportWavesPart);
				wavesDataSize += wdiXbox.waveExSize;
				wavesDataSize += wdiXbox.waveDataSize;
				wavesDataSize += wdiPC.waveExSize;
				wavesDataSize += wdiPC.waveDataSize;
				//Выделяем память и переносим информацию
				array<byte> buffer(_FL_);
				buffer.AddElements(wavesDataSize);
				byte * wData = buffer.GetBuffer();
				ProjectWave::ExportWavesPart & ewp = *(ProjectWave::ExportWavesPart *)wData;
				ewp.size = wavesDataSize;
				ewp.waveformatSizeXbox = wdiXbox.waveExSize;
				ewp.waveSizeForXbox = wdiXbox.waveDataSize;
				ewp.waveSmplsForXbox = wdiXbox.waveSplCount;
				ewp.exportFormatXbox = wdiXbox.waveFormat;
				Assert(wdiXbox.sizeofAdds == sizeof(ProjectWave::WaveOptions) + 2*sizeof(dword));
				ewp.exportOptsXbox = *(ProjectWave::WaveOptions *)wdiXbox.adds;
				dword xboxWaveCheck = (dword &)wdiXbox.adds[sizeof(ProjectWave::WaveOptions)];
				dword xboxWaveSize = (dword &)wdiXbox.adds[sizeof(ProjectWave::WaveOptions) + sizeof(dword)];
				ewp.waveformatSizePC = wdiPC.waveExSize;
				ewp.waveSizeForPC = wdiPC.waveDataSize;
				ewp.waveSmplsForPC = wdiPC.waveSplCount;
				ewp.exportFormatPC = wdiPC.waveFormat;
				Assert(wdiPC.sizeofAdds == sizeof(ProjectWave::WaveOptions) + 2*sizeof(dword));
				ewp.exportOptsPC = *(ProjectWave::WaveOptions *)wdiPC.adds;
				dword pcWaveCheck = (dword &)wdiPC.adds[sizeof(ProjectWave::WaveOptions)];
				dword pcWaveSize = (dword &)wdiPC.adds[sizeof(ProjectWave::WaveOptions) + sizeof(dword)];
				byte * currentPtr = wData + sizeof(ProjectWave::ExportWavesPart);
				memcpy(currentPtr, xboxData + sizeof(WaveDataInfo), wdiXbox.waveExSize + wdiXbox.waveDataSize);
				currentPtr += wdiXbox.waveExSize + wdiXbox.waveDataSize;
				memcpy(currentPtr, pcData + sizeof(WaveDataInfo), wdiPC.waveExSize + wdiPC.waveDataSize);
				currentPtr += wdiPC.waveExSize + wdiPC.waveDataSize;
				Assert(currentPtr - wData == buffer.Size());
				Assert(xboxWaveCheck == pcWaveCheck);
				Assert(xboxWaveSize == pcWaveSize);
				ewp.waveCheckSum = xboxWaveCheck;
				ewp.waveDataSize = xboxWaveSize;
				qr.wave->BuildExportPath(fileName, false);
				IFile * file = project->FileOpenBin(fileName.c_str(), SndProject::poff_isCreateNewFile, null, _FL_, 102);
				if(!file)
				{
					//Файл не удалось открыть, попробуем позже, а пока продолжим идти по очереди
					totalTries++;
					continue;
				}
				if(file->Write(buffer.GetBuffer(), buffer.GetDataSize()) != buffer.GetDataSize())
				{
					//Чтото не срослось, попробуем потом ещё раз
					totalTries++;
					file->SetPos(0);
					file->Truncate();
					project->FileCloseBin(file, 102);
					continue;
				}
				project->FileCloseBin(file, 102);
				buffer.Empty();
				isIdle = false;
				delete qr.exports[e_pcWave].buffer;
				qr.exports[e_pcWave].buffer = null;
				delete qr.exports[e_xboxWave].buffer;
				qr.exports[e_xboxWave].buffer = null;
				break;
			}else{
				if(qr.exports[e_pcWave].buffer)
				{
					delete qr.exports[e_pcWave].buffer;
					qr.exports[e_pcWave].buffer = null;
				}
				if(qr.exports[e_xboxWave].buffer)
				{					
					delete qr.exports[e_xboxWave].buffer;
					qr.exports[e_xboxWave].buffer = null;
				}
			}
		}
		if(qr.exports[e_phonemes].isDone && qr.exports[e_phonemes].buffer)
		{
			Assert(qr.exports[e_phonemes].size >= sizeof(ProjectWave::ExportPhonemesPart));
			//Открываем файл для записи данных
			qr.wave->BuildExportPath(fileName, true);
			IFile * file = project->FileOpenBin(fileName.c_str(), SndProject::poff_isCreateNewFile, null, _FL_, 103);
			if(!file)
			{
				//Файл не удалось открыть, попробуем позже, а пока продолжим идти по очереди
				totalTries++;
				continue;
			}
			if(file->Write(qr.exports[e_phonemes].buffer, qr.exports[e_phonemes].size) != qr.exports[e_phonemes].size)
			{
				//Чтото не срослось, попробуем потом ещё раз
				totalTries++;
				file->SetPos(0);
				file->Truncate();
				project->FileCloseBin(file, 103);
				continue;
			}
			project->FileCloseBin(file, 103);
			isIdle = false;
			delete qr.exports[e_phonemes].buffer;
			qr.exports[e_phonemes].buffer = null;
			break;
		}
	}
	//api->Trace("ProjectWaveExporter::IdleFunction Leave start");
	globalObject.queueAccessor.Leave();
	//api->Trace("ProjectWaveExporter::IdleFunction Leave end");
	//Освободим ресурсы для других потоков
	if(isIdle)
	{
		Sleep(100);
	}	
}

//Удалить выделенный элемент из очереди. Необходимо находиться в секции queueAccessor
void ProjectWaveExporter::DeleteQueueElement(long index)
{
	if(index < 0 || (dword)index >= globalObject.exportQueue.Size())
	{
		return;
	}
	QuiueRecord & qr = globalObject.exportQueue[index];
	if(!qr.markForDelete)
	{
		return;
	}
	bool isError = false;
	for(long i = 0; i < e_count; i++)
	{
		if(qr.exports[i].buffer)
		{
			delete qr.exports[i].buffer;
		}
		isError |= qr.exports[i].isError;
	}
	if(isError)
	{
		options->ErrorOut(&qr.errorCode, true, "");
	}else{
		options->ErrorCancel(qr.errorCode);
		qr.errorCode = ErrorId::ok;
	}
	qr.wave->ProcessExportRemoveFromQueue(qr.errorCode);
	globalObject.exportQueue.DelIndex(index);
}


//Преобразование данных волны в нужный формат
byte * _cdecl ProjectWaveExporter::ConvertWaveData(ProjectWave * wave, dword & size, void * params, ErrorId errorCode)
{
	WaveDataExportParams * exportParams = (WaveDataExportParams *)params;
	const WAVEFORMATEX & wformat = wave->GetFormat();
	dword platformOpts = exportParams->isPC ? ProjectWave::wce_platform_pc : ProjectWave::wce_platform_xbox;
	const ProjectWave::WaveOptions & exportOptions = wave->GetExportOptions(platformOpts);
	//Проконтролируем входной формат на допустимые значения
	Assert(wformat.nChannels == 1 || wformat.nChannels == 2);
	Assert(wformat.wBitsPerSample == 16);
	Assert(wformat.nSamplesPerSec >= 44100);
	dword bytesPerSample = (wformat.wBitsPerSample*wformat.nChannels)/8;
	//Проверим согласование форматов
	Assert(wformat.nChannels == exportOptions.channels);
	Assert(wformat.nSamplesPerSec >= exportOptions.samplerate);
	//Источник
	wave->LoadWaveData(ProjectWaveDebugId_cnvwavedata);
	dword rawDataSizeInBytes = 0;
	const byte * rawSource = wave->GetWaveData(rawDataSizeInBytes);
	Assert(rawSource);
	//Подготавливаем волну нужной частоты
	const byte * source = rawSource;
	dword samplesCount = rawDataSizeInBytes/bytesPerSample;
	Assert(samplesCount*bytesPerSample == rawDataSizeInBytes);
	array<byte> resampleWaveData(_FL_);
	if(wformat.nSamplesPerSec != exportOptions.samplerate)
	{
		DownSample(rawSource, samplesCount, wformat.nSamplesPerSec, wformat.nChannels, exportOptions.samplerate, resampleWaveData, samplesCount);
		source = resampleWaveData.GetBuffer();
	}
	//Подготовим описание формата
	Assert((exportOptions.samplerate & SoundBankFileWave::f_freq_mask) == exportOptions.samplerate);
	dword format = exportOptions.samplerate;
	dword quality = ((dword)exportOptions.compression) << SoundBankFileWave::f_quality_shift;
	Assert((quality & SoundBankFileWave::f_quality_mask) == quality);
	format |= quality;
	if(exportOptions.channels > 1) format |= SoundBankFileWave::f_stereo;
	//Конвертируем в нужный формат
	byte * exportData = null;
	const char * formatName = "unknown";
	switch(exportOptions.format)
	{
	case ProjectWave::wce_format_16bit:
		exportData = ConvertToPCM(exportParams->isPC, source, samplesCount, exportOptions.channels, exportOptions.samplerate, exportParams->isMakePreview);
		formatName = "PCM";
		format |= SoundBankFileWave::f_format_pcm;
		break;
	case ProjectWave::wce_format_xma:
		Assert(!exportParams->isPC);
		exportData = ConvertToXMA(source, samplesCount, exportOptions.channels, exportOptions.samplerate, exportOptions.compression, wave->ProcessExportGetName(), exportParams->isMakePreview);
		formatName = "XMA";
		format |= SoundBankFileWave::f_format_xma;
		break;
	case ProjectWave::wce_format_xwma:
		exportData = ConvertToXWMA(exportParams->isPC, source, samplesCount, exportOptions.channels, exportOptions.samplerate, exportOptions.compression, wave->ProcessExportGetName(), exportParams->isMakePreview);
		format |= SoundBankFileWave::f_format_xvma;
		formatName = "xWMA";
		break;
	}
	if(exportData)
	{
		WaveDataInfo * dataInfo = (WaveDataInfo *)exportData;
		dataInfo->waveFormat = format;
		if(!exportParams->isMakePreview)
		{
			dataInfo->sizeofAdds = sizeof(ProjectWave::WaveOptions) + 2*sizeof(dword);
			Assert(dataInfo->sizeofAdds <= sizeof(dataInfo->adds));
			memcpy(dataInfo->adds, &exportOptions, sizeof(ProjectWave::WaveOptions));
			dword * waveCheck = (dword *)&dataInfo->adds[sizeof(ProjectWave::WaveOptions)];
			waveCheck[0] = wave->GetWaveDataCheckSum();
			waveCheck[1] = wave->GetWaveDataSize();
		}		
	}else{
		options->ErrorOut(&errorCode, false, "Error try convert wave \"%s\" to %s data format.", wave->GetName().str, formatName);
	}
	wave->UnloadWaveData(ProjectWaveDebugId_cnvwavedata);
	return exportData;
}

//Подготовка волны пониженой частоты (16bit)
void ProjectWaveExporter::DownSample(const byte * sourceData, dword srcSamples, dword srcSampleRate, dword srcChannels, dword dstSampleRate, array<byte> & buffer, dword & dstSamples)
{
	Assert(dstSampleRate < srcSampleRate);
	Assert(srcSamples > 16);
	Assert(srcChannels == 1 || srcChannels == 2);
	//!!! FixMe
	//Пока что примитивным алгоритмом даунсэмплим, чтобы хоть както работало
	//Потом надо апсэмплить на кратную частоту даунсэмплированой, 
	//фильтровать НЧ фильтром по граничной частоте, 
	//и после выбирать выборки с целым шагом без интерполяции
	float step = srcSampleRate/float(dstSampleRate);	
	buffer.Reserve((long(srcSamples/step) + 64)*2*srcChannels);
	buffer.Empty();
	dstSamples = 0;
	for(float pos = 0.0f; pos < (float)srcSamples; pos += step, dstSamples++)
	{
		long samplePos1 = long(pos);
		long samplePos2 = samplePos1 + 1;
		if((dword)samplePos2 >= srcSamples) samplePos2 = srcSamples - 1;
		float kLerp = pos - samplePos1;
		if(srcChannels == 1)
		{
			short * sample1 = (short *)(sourceData + samplePos1*sizeof(short));
			short * sample2 = (short *)(sourceData + samplePos2*sizeof(short));
			float v = sample1[0] + (sample2[0] - sample1[0])*kLerp;
			long s = buffer.Size();
			buffer.AddElements(sizeof(short));
			short * dst = (short *)&buffer[s];
			dst[0] = (short)v;
		}else{
			short * sample1 = (short *)(sourceData + samplePos1*2*sizeof(short));
			short * sample2 = (short *)(sourceData + samplePos2*2*sizeof(short));
			float v0 = sample1[0] + (sample2[0] - sample1[0])*kLerp;
			float v1 = sample1[1] + (sample2[1] - sample1[1])*kLerp;
			long s = buffer.Size();
			buffer.AddElements(2*sizeof(short));
			short * dst = (short *)&buffer[s];
			dst[0] = (short)v0;
			dst[1] = (short)v1;
		}
	}
	Assert(dstSamples > 0);
	for(; dstSamples & 0x7f; dstSamples++)
	{
		if(srcChannels == 1)
		{
			long s = buffer.Size();
			buffer.AddElements(sizeof(short));
			short * dst = (short *)&buffer[s];
			dst[0] = 0;
		}else{
			long s = buffer.Size();
			buffer.AddElements(2*sizeof(short));
			short * dst = (short *)&buffer[s];
			dst[0] = 0;
			dst[1] = 0;
		}
	}
	
	//Сохраним для отладки
	//char buf[256];
	//ConvertSaveToTmpFile(buf, sizeof(buf), "downsample_debug", buffer.GetBuffer(), dstSamples, srcChannels, dstSampleRate);
}

//Волна 16 бит
byte * ProjectWaveExporter::ConvertToPCM(bool isPC, const byte * sourceData, dword samplesCount, dword channels, dword sampleRate, bool isMakePreviewMode)
{
	Assert(sizeof(short) == 2);
	//Определяем требуемый объем памяти
	dword size = sizeof(WaveDataInfo) + sizeof(WAVEFORMATEX) + samplesCount*channels*sizeof(short);
	//Выделяем память и размапливаем её
	Assert(size <= XAUDIO2_MAX_BUFFER_BYTES);
	byte * buffer = NEW byte[size];
	WaveDataInfo * info = (WaveDataInfo *)buffer;
	memset(info, 0, sizeof(WaveDataInfo));
	WAVEFORMATEX * eformat = (WAVEFORMATEX *)(buffer + sizeof(WaveDataInfo));
	short * samples = (short *)(buffer + sizeof(WAVEFORMATEX) + sizeof(WaveDataInfo));
	//Заполняем заголовок
	eformat->wFormatTag = WAVE_FORMAT_PCM;
	eformat->nChannels = channels;
	eformat->nSamplesPerSec = sampleRate;
	eformat->wBitsPerSample = 16;
	eformat->nBlockAlign = eformat->nChannels*eformat->wBitsPerSample/8;
	eformat->nAvgBytesPerSec = eformat->nBlockAlign*eformat->nSamplesPerSec;
	eformat->cbSize = 0;
	//Копирование
	const short * source = (const short *)sourceData;
	dword count = samplesCount*channels;
	if(isPC || isMakePreviewMode)
	{
		for(dword i = 0; i < count; i++)
		{
			samples[i] = source[i];
		}
	}else{
		//Надо перевернуть все поля для Xbox
		__RefDataSwizzler(eformat->wFormatTag);
		__RefDataSwizzler(eformat->nChannels);
		__RefDataSwizzler(eformat->nSamplesPerSec);
		__RefDataSwizzler(eformat->wBitsPerSample);
		__RefDataSwizzler(eformat->nBlockAlign);
		__RefDataSwizzler(eformat->nAvgBytesPerSec);
		__RefDataSwizzler(eformat->cbSize);
		for(dword i = 0; i < count; i++)
		{
			samples[i] = source[i];
			__RefDataSwizzler(samples[i]);
		}
	}
	//Информация о данных
	info->waveExSize = sizeof(WAVEFORMATEX);
	info->waveDataSize = samplesCount*2*channels;
	info->waveSplCount = samplesCount;
	info->waveFormat = 0;
	if(isMakePreviewMode)
	{
		info->sizeofAdds = 4;
		((dword &)info->adds[0]) = count*sizeof(short);
	}
	return buffer;
}

//XMA (только на XBOX)
byte * ProjectWaveExporter::ConvertToXMA(const byte * sourceData, dword samplesCount, dword channels, dword sampleRate, dword compressionRate, const char * waveName, bool isMakePreviewMode)
{
	//Папка в которой мутим файлы
	const char * workPath = options->pathTempWorkFolder.c_str();
	//Сохраняем временный файл
	char bufferName[1024];	
	ConvertSaveToTmpFile(bufferName, sizeof(bufferName), waveName, sourceData, samplesCount, channels, sampleRate, false);
	//Запускаем процесс преобразования
	char cmdLine[2048];
	if(compressionRate < 1) compressionRate = 1;
	if(compressionRate > 100) compressionRate = 100;
	if(!isMakePreviewMode)
	{
		crt_snprintf(cmdLine, sizeof(cmdLine), "\"xma2encode.exe\" \"%s\" /Quality %i", bufferName, compressionRate);
	}else{
		crt_snprintf(cmdLine, sizeof(cmdLine), "\"xma2encode.exe\" \"%s\" /Quality %i /PCMOutputFile \"%s_dec.wav\"", bufferName, compressionRate, bufferName);
	}
	if(!DoExternProcess(cmdLine))
	{
		::DeleteFile(bufferName);
		return null;
	}
	byte * buffer = null;
	if(!isMakePreviewMode)
	{
		//Загружаем файл
		crt_snprintf(cmdLine, sizeof(cmdLine), "%s.xma", bufferName);
		array<byte> format(_FL_);
		array<byte> table(_FL_);
		array<byte> data(_FL_);
		ConvertLoadTmpWavFile(cmdLine, mmioFOURCC('W', 'A', 'V', 'E'), format, table, data);
		//Проверка на ошибки, которых не должно быть
		Assert(format.Size() == sizeof(XMA2WAVEFORMATEX));
		Assert(table.Size() == 0);
		Assert(data.Size() > 0);
		//Формируем буфер для сохранения
		dword size = sizeof(WaveDataInfo) + format.Size() + data.Size();
		buffer = NEW byte[size];
		WaveDataInfo * info = (WaveDataInfo *)buffer;
		memset(info, 0, sizeof(WaveDataInfo));
		info->waveExSize = format.Size();
		info->waveDataSize = data.Size();
		info->waveSplCount = samplesCount;
		info->waveFormat = 0;
		memcpy(buffer + sizeof(WaveDataInfo), format.GetBuffer(), format.Size());
		memcpy(buffer + sizeof(WaveDataInfo) + format.Size(), data.GetBuffer(), data.Size());
		//Надо перевернуть все поля для Xbox
		XMA2WAVEFORMATEX & xma = *(XMA2WAVEFORMATEX *)(buffer + sizeof(WaveDataInfo));
		__RefDataSwizzler(xma.wfx.wFormatTag);
		__RefDataSwizzler(xma.wfx.nChannels);
		__RefDataSwizzler(xma.wfx.nSamplesPerSec);
		__RefDataSwizzler(xma.wfx.nAvgBytesPerSec);
		__RefDataSwizzler(xma.wfx.nBlockAlign);
		__RefDataSwizzler(xma.wfx.wBitsPerSample);
		__RefDataSwizzler(xma.wfx.cbSize);
		__RefDataSwizzler(xma.NumStreams);
		__RefDataSwizzler(xma.ChannelMask);
		__RefDataSwizzler(xma.SamplesEncoded);
		__RefDataSwizzler(xma.BytesPerBlock);
		__RefDataSwizzler(xma.PlayBegin);
		__RefDataSwizzler(xma.PlayLength);
		__RefDataSwizzler(xma.LoopBegin);
		__RefDataSwizzler(xma.LoopLength);
		__RefDataSwizzler(xma.LoopCount);
		__RefDataSwizzler(xma.EncoderVersion);
		__RefDataSwizzler(xma.BlockCount);
		//Удаляем временные файлы
		::DeleteFile(bufferName);
		::DeleteFile(cmdLine);
	}else{
		//Загружаем файл для прослушивания
		crt_snprintf(cmdLine, sizeof(cmdLine), "%s_dec.wav", bufferName);
		array<byte> format(_FL_);
		array<byte> table(_FL_);
		array<byte> data(_FL_);
		ConvertLoadTmpWavFile(cmdLine, mmioFOURCC('W', 'A', 'V', 'E'), format, table, data);
		//Проверка на ошибки, которых не должно быть
		Assert(format.Size() >= sizeof(WAVEFORMATEX));
		Assert(((WORD &)format[0]) == WAVE_FORMAT_PCM || ((WORD &)format[0]) == WAVE_FORMAT_EXTENSIBLE);
		Assert(table.Size() == 0);
		Assert(data.Size() > 0);
		//Формируем буфер для сохранения
		dword size = sizeof(WaveDataInfo) + sizeof(WAVEFORMATEX) + data.Size();
		buffer = NEW byte[size + 256];
		memset(buffer, 0, size + 256);
		WaveDataInfo * info = (WaveDataInfo *)buffer;
		memset(info, 0, sizeof(WaveDataInfo));
		byte * dataPtr = AlignPtr(buffer + sizeof(WaveDataInfo) + sizeof(WAVEFORMATEX) + 32);
		info->waveExSize = dataPtr - buffer - sizeof(WaveDataInfo);
		info->waveDataSize = data.Size();
		info->waveSplCount = samplesCount;
		info->waveFormat = 0;
		memcpy(buffer + sizeof(WaveDataInfo), format.GetBuffer(), sizeof(WAVEFORMATEX));
		memcpy(dataPtr, data.GetBuffer(), data.Size());
		//Удаляем временные файлы
		::DeleteFile(bufferName);
		::DeleteFile(cmdLine);		
		//Перед удалением компрессированного, посмотрим оценочные размеры данных
		crt_snprintf(cmdLine, sizeof(cmdLine), "%s.xma", bufferName);
		info->sizeofAdds = 4;
		((dword &)info->adds[0]) = 0;
		HANDLE handle = ::CreateFile(cmdLine, GENERIC_READ, FILE_SHARE_READ, null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, null);
		if(handle != INVALID_HANDLE_VALUE)
		{
			((dword &)info->adds[0]) = 0;
			DWORD hi = 0;
			((dword &)info->adds[0]) = ::GetFileSize(handle, &hi) - 8 - sizeof(WAVEFORMATEX);
			CloseHandle(handle);
		}
		::DeleteFile(cmdLine);
	}
	return buffer;
}

//xWMA
byte * ProjectWaveExporter::ConvertToXWMA(bool isPC, const byte * sourceData, dword samplesCount, dword channels, dword sampleRate, dword compressionRate, const char * waveName, bool isMakePreviewMode)
{
	//Пока что без прослушивания
//	Assert(isMakePreviewMode);
	Assert(samplesCount >= 100000);
	//Папка в которой мутим файлы
	const char * workPath = options->pathTempWorkFolder.c_str();
	//Сохраняем временный файл
	char bufferName[1024];
	ConvertSaveToTmpFile(bufferName, sizeof(bufferName), waveName, sourceData, samplesCount, channels, sampleRate, false);
	//Запускаем процесс преобразования
	char cmdLine[2048];
	if(compressionRate < 1) compressionRate = 1;
	if(compressionRate > 100) compressionRate = 100;
	crt_snprintf(cmdLine, sizeof(cmdLine), "\"xWMAEncode.exe\" \"%s\" \"%s.xwma\"", bufferName, bufferName);
	if(!DoExternProcess(cmdLine))
	{
		::DeleteFile(bufferName);
		return null;
	}
	byte * buffer = null;
	if(!isMakePreviewMode)
	{
		//Загружаем файл
		crt_snprintf(cmdLine, sizeof(cmdLine), "%s.xwma", bufferName);
		array<byte> format(_FL_);
		array<byte> table(_FL_);
		array<byte> data(_FL_);
		ConvertLoadTmpWavFile(cmdLine, mmioFOURCC('X', 'W', 'M', 'A'), format, table, data);
		//Проверка на ошибки, которых не должно быть
		Assert(format.Size() == sizeof(WAVEFORMATEX));
		Assert(table.Size() > 0);
		Assert((table.Size() & 3) == 0);
		Assert(data.Size() > 0);
		//Формируем буфер для сохранения
		dword tableCount = table.Size()/sizeof(dword);
		dword size = sizeof(WaveDataInfo) + sizeof(SoundBankFileWave::XWMAWAVEFORMAT) + (tableCount - 1)*sizeof(dword) + data.Size();
		buffer = NEW byte[size];
		WaveDataInfo * info = (WaveDataInfo *)buffer;
		memset(info, 0, sizeof(WaveDataInfo));
		info->waveExSize = sizeof(SoundBankFileWave::XWMAWAVEFORMAT) + (tableCount - 1)*sizeof(dword);
		info->waveDataSize = data.Size();
		info->waveSplCount = samplesCount;
		info->waveFormat = 0;
		SoundBankFileWave::XWMAWAVEFORMAT * xwma = (SoundBankFileWave::XWMAWAVEFORMAT *)(buffer + sizeof(WaveDataInfo));
		memcpy(&xwma->format, format.GetBuffer(), sizeof(WAVEFORMATEX));
		xwma->tableCount = tableCount;
		Assert(table.GetDataSize() == tableCount*sizeof(dword));
		memcpy(xwma->table, table.GetBuffer(), tableCount*sizeof(dword));
		memcpy(buffer + sizeof(WaveDataInfo) + sizeof(SoundBankFileWave::XWMAWAVEFORMAT) + (tableCount - 1)*sizeof(dword), data.GetBuffer(), data.Size());
		if(!isPC)
		{
			//Надо перевернуть все поля для Xbox
			__RefDataSwizzler(xwma->format.wFormatTag);
			__RefDataSwizzler(xwma->format.nChannels);
			__RefDataSwizzler(xwma->format.nSamplesPerSec);
			__RefDataSwizzler(xwma->format.wBitsPerSample);
			__RefDataSwizzler(xwma->format.nBlockAlign);
			__RefDataSwizzler(xwma->format.nAvgBytesPerSec);
			__RefDataSwizzler(xwma->format.cbSize);
			for(dword i = 0; i < xwma->tableCount; i++)
			{			
				__RefDataSwizzler(xwma->table[i]);
			}
			__RefDataSwizzler(xwma->tableCount);
		}
		//Удаляем временные файлы
		::DeleteFile(bufferName);
		::DeleteFile(cmdLine);
	}else{
		crt_snprintf(cmdLine, sizeof(cmdLine), "\"xWMAEncode.exe\" \"%s.xwma\" \"%s_xwma.wav\"", bufferName, bufferName);
		if(!DoExternProcess(cmdLine)) return null;
		//Загружаем файл
		crt_snprintf(cmdLine, sizeof(cmdLine), "%s_xwma.wav", bufferName);
		array<byte> format(_FL_);
		array<byte> table(_FL_);
		array<byte> data(_FL_);
		ConvertLoadTmpWavFile(cmdLine, mmioFOURCC('W', 'A', 'V', 'E'), format, table, data);
		//Проверка на ошибки, которых не должно быть
		Assert(format.Size() >= sizeof(WAVEFORMATEX));
		Assert(((WORD &)format[0]) == WAVE_FORMAT_PCM || ((WORD &)format[0]) == WAVE_FORMAT_EXTENSIBLE);
		Assert(table.Size() == 0);
		Assert(data.Size() > 0);
		//Формируем буфер для сохранения
		dword size = sizeof(WaveDataInfo) + sizeof(WAVEFORMATEX) + data.Size();
		buffer = NEW byte[size + 256];
		memset(buffer, 0, size + 256);
		WaveDataInfo * info = (WaveDataInfo *)buffer;
		memset(info, 0, sizeof(WaveDataInfo));
		byte * dataPtr = AlignPtr(buffer + sizeof(WaveDataInfo) + sizeof(WAVEFORMATEX) + 32);
		info->waveExSize = dataPtr - buffer - sizeof(WaveDataInfo);
		Assert(info->waveExSize > sizeof(WAVEFORMATEX));
		info->waveDataSize = data.Size();
		info->waveSplCount = samplesCount;
		info->waveFormat = 0;
		memcpy(buffer + sizeof(WaveDataInfo), format.GetBuffer(), sizeof(WAVEFORMATEX));		
		memcpy(dataPtr, data.GetBuffer(), data.Size());
		//Удаляем временные файлы
		::DeleteFile(bufferName);
		::DeleteFile(cmdLine);
		crt_snprintf(cmdLine, sizeof(cmdLine), "%s.xwma", bufferName);
		//Перед удалением компрессированного, посмотрим оценочные размеры данных
		info->sizeofAdds = 4;
		((dword &)info->adds[0]) = 0;
		HANDLE handle = ::CreateFile(cmdLine, GENERIC_READ, FILE_SHARE_READ, null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, null);
		if(handle != INVALID_HANDLE_VALUE)
		{
			((dword &)info->adds[0]) = 0;
			DWORD hi = 0;
			((dword &)info->adds[0]) = ::GetFileSize(handle, &hi) - 8 - sizeof(WAVEFORMATEX);
			CloseHandle(handle);
		}		
		::DeleteFile(cmdLine);
	}
	return buffer;
}

//Сохранить волну во временный файл RIFF PCM
void ProjectWaveExporter::ConvertSaveToTmpFile(char * name, dword nameMaxSize, const char * waveName, const byte * sourceData, dword samplesCount, dword channels, dword sampleRate, bool isSaveWithExtention)
{
	//Генерим уникальное имя
	HANDLE handle = INVALID_HANDLE_VALUE;
	long maxCount = 1000000;
	for(long i = 0; i < maxCount; i++)
	{
		crt_snprintf(name, nameMaxSize, "%s__exptmp__%s_%i%s", options->pathTempWorkFolder.c_str(), waveName, i, isSaveWithExtention ? ".wav" : "");
		dword access = GENERIC_WRITE | GENERIC_READ;
		handle = ::CreateFile(name, GENERIC_WRITE, FILE_SHARE_WRITE, null, CREATE_NEW, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, null);
		if(handle != INVALID_HANDLE_VALUE)
		{
			break;
		}
	}
	Assert(i < maxCount);
	SaveWave(null, sourceData, samplesCount, channels, sampleRate, handle);
}

//Сохранить волну во временный файл RIFF PCM
void ProjectWaveExporter::SaveWave(const char * path, const byte * sourceData, dword samplesCount, dword channels, dword sampleRate, HANDLE handle)
{
	if(handle == INVALID_HANDLE_VALUE)
	{
		handle = ::CreateFile(path, GENERIC_WRITE, FILE_SHARE_WRITE, null, CREATE_NEW, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, null);
		if(handle == INVALID_HANDLE_VALUE)
		{
			::DeleteFile(path);
			handle = ::CreateFile(path, GENERIC_WRITE, FILE_SHARE_WRITE, null, CREATE_NEW, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, null);
			if(handle == INVALID_HANDLE_VALUE)
			{
				api->Trace("Sound editor error: Can't open file \"%s\" for write new wave!");
				return;
			}
		}
	}
	//Сохраняем в новый пустой файл зоголовок
	struct EasyWav
	{
		dword riffId;
		dword totalSize;
		dword waveId;
		dword fmtId;
		dword fmtSize;
		PCMWAVEFORMAT pcm;
		dword dataId;
		dword dataSize;
	};
	dword waveSizeInBytes = samplesCount*channels*sizeof(short);
	array<byte> buffer(_FL_);
	buffer.AddElements(sizeof(EasyWav) + waveSizeInBytes);
	EasyWav & wav = (EasyWav &)buffer[0];
	wav.riffId = FOURCC_RIFF;
	wav.totalSize = sizeof(EasyWav) + waveSizeInBytes - 2*sizeof(dword);
	wav.waveId = mmioFOURCC('W', 'A', 'V', 'E');
	wav.fmtId = mmioFOURCC('f', 'm', 't', ' ');
	wav.fmtSize = sizeof(PCMWAVEFORMAT);
	wav.pcm.wf.wFormatTag = WAVE_FORMAT_PCM;
	wav.pcm.wf.nChannels = channels;
	wav.pcm.wf.nSamplesPerSec = sampleRate;
	wav.pcm.wBitsPerSample = 16;
	wav.pcm.wf.nBlockAlign = (wav.pcm.wf.nChannels*wav.pcm.wBitsPerSample)/8;
	wav.pcm.wf.nAvgBytesPerSec = (wav.pcm.wf.nChannels*wav.pcm.wBitsPerSample*wav.pcm.wf.nSamplesPerSec)/8;
	wav.dataId = mmioFOURCC('d', 'a', 't', 'a');
	wav.dataSize = waveSizeInBytes;
	memcpy(&buffer[sizeof(EasyWav)], sourceData, waveSizeInBytes);
	DWORD writeBytes = 0;
	bool ErrorSaveDataToTemporaryFile = ::WriteFile(handle, buffer.GetBuffer(), buffer.GetDataSize(), &writeBytes, null) != FALSE;
	Assert(ErrorSaveDataToTemporaryFile);
	Assert(writeBytes == buffer.GetDataSize());
	CloseHandle(handle);
}

//Загрузить из полученого RIFF файла необходимые чанки
void ProjectWaveExporter::ConvertLoadTmpWavFile(const char * path, dword id, array<byte> & format, array<byte> & table, array<byte> & data)
{
	//Подготавливаем массивы к принятию данных
	format.Empty();
	table.Empty();
	data.Empty();
	//Загружаем файл в память
	HANDLE handle = INVALID_HANDLE_VALUE;
	long maxCount = 10;
	for(long i = 0; i < maxCount; i++)
	{
		handle = ::CreateFile(path, GENERIC_READ, FILE_SHARE_READ, null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, null);
		if(handle != INVALID_HANDLE_VALUE)
		{
			break;
		}
		Sleep(10);
	}
	Assert(handle != INVALID_HANDLE_VALUE);
	dword size = ::GetFileSize(handle, null);
	Assert(size != INVALID_FILE_SIZE);
	Assert(size > 12);
	array<byte> buffer(_FL_);
	buffer.AddElements(size);
	DWORD readBytes = 0;
	bool ErrorLoadDataToTemporaryFile = ::ReadFile(handle, buffer.GetBuffer(), buffer.GetDataSize(), &readBytes, null) != FALSE;
	Assert(ErrorLoadDataToTemporaryFile);
	Assert(readBytes == buffer.GetDataSize());
	CloseHandle(handle);
	//Разбираем загруженный файл
	Assert((dword &)buffer[0] == FOURCC_RIFF);
	Assert((dword &)buffer[8] == id);
	for(dword currentChank = 12; currentChank + 8 < buffer.Size(); )
	{
		//Заголовок чанка
		dword chankId = (dword &)buffer[currentChank + 0];
		dword chankSize = (dword &)buffer[currentChank + 4];
		Assert(currentChank + chankSize + 8 <= buffer.Size());
		//Рассматриваем интересующие
		if(chankId == mmioFOURCC('f', 'm', 't', ' ') && format.Size() == 0)
		{
			//Формат			
			format.AddElements(chankSize);
			memcpy(format.GetBuffer(), &buffer[currentChank + 8], format.GetDataSize());
		}else
		if(chankId == mmioFOURCC('d', 'a', 't', 'a') && data.Size() == 0)
		{
			//Формат			
			data.AddElements(chankSize);
			memcpy(data.GetBuffer(), &buffer[currentChank + 8], data.GetDataSize());
		}else
		if(chankId == mmioFOURCC('d', 'p', 'd', 's') && table.Size() == 0)
		{
			//Формат			
			table.AddElements(chankSize);
			memcpy(table.GetBuffer(), &buffer[currentChank + 8], table.GetDataSize());
		}
		currentChank += chankSize + 8;
	}	
}

//Построение списка фонем по волне	
byte * _cdecl ProjectWaveExporter::PreparePhonemes(ProjectWave * wave, dword & size, void * params, ErrorId errorCode)
{
	size = 0;
	//Источник
	wave->LoadWaveData(ProjectWaveDebugId_genphonemes);	
	dword rawDataSizeInBytes = 0;
	const byte * source = wave->GetWaveData(rawDataSizeInBytes);
	Assert(source);
	const WAVEFORMATEX & wformat = wave->GetFormat();
	//Папка в которой мутим файлы
	const char * workPath = options->pathTempWorkFolder.c_str();
	//Проконтролируем входной формат на допустимые значения
	Assert(wformat.nChannels == 1 || wformat.nChannels == 2);
	Assert(wformat.wBitsPerSample == 16);
	Assert(wformat.nSamplesPerSec >= 44100);
	dword bytesPerSample = (wformat.wBitsPerSample*wformat.nChannels)/8;
	dword samplesCount = rawDataSizeInBytes/bytesPerSample;
	Assert(samplesCount*bytesPerSample == rawDataSizeInBytes);
	//Сохраняем временный файл
	char bufferName[1024];
	ConvertSaveToTmpFile(bufferName, sizeof(bufferName), wave->ProcessExportGetName(), source, samplesCount, wformat.nChannels, wformat.nSamplesPerSec, true);
	char cmdLine[2048];
	const char * toolPath = "C:\\private tools\\xphonemes\\xphonemes.exe";
	if(!options->isDeveloperMode)
	{
		crt_snprintf(cmdLine, sizeof(cmdLine), "\"%s\" \"%s\" \"%s.ph\"", toolPath, bufferName, bufferName);
	}else{
		crt_snprintf(cmdLine, sizeof(cmdLine), "\"%s\" \"%s\" \"%s.ph\" \"%s\"", toolPath, bufferName, bufferName, options->pathPhonemesLog.c_str());
	}
	if(!options->fileService->IsExist(toolPath))
	{
		//Файла нет, делаем фэйковый экспорт		
		ProjectWave::ExportPhonemesPart * ph = NEW ProjectWave::ExportPhonemesPart;
		ph->size = size = sizeof(ProjectWave::ExportPhonemesPart);
		ph->waveCheckSum = wave->GetWaveDataCheckSum();
		ph->waveDataSize = wave->GetWaveDataSize();
		ph->count = 0;
		ph->phoneme[0].id = 0;
		ph->phoneme[0].time = 0;
		ph->phoneme[0].itensity = 0.0f;
		api->Trace("Xphonemes tool not found. Export empty data.");
		wave->UnloadWaveData(ProjectWaveDebugId_genphonemes);
		return (byte *)ph;
	}
	if(!DoExternProcess(cmdLine))
	{
		::DeleteFile(bufferName);
		options->ErrorOut(&errorCode, false, "Error try build phonemes for wave \"%s\".", wave->GetName().str);
		wave->UnloadWaveData(ProjectWaveDebugId_genphonemes);
		return null;
	}
	//Загружаем подготовленные фонемы
	crt_snprintf(cmdLine, sizeof(cmdLine), "%s.ph", bufferName);
	//Загружаем файл в память
	byte * buffer = null;
	HANDLE handle = INVALID_HANDLE_VALUE;
	handle = ::CreateFile(cmdLine, GENERIC_READ, FILE_SHARE_READ, null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, null);
	if(handle != INVALID_HANDLE_VALUE)
	{
		while(true)	//Для удобного прерывания исполнения
		{
			//Зачитываем количество фонем
			DWORD readBytes = 0;
			long phonemesCount = 0;
			if(::ReadFile(handle, &phonemesCount, 4, &readBytes, null) == FALSE || readBytes != 4 || phonemesCount <= 0)
			{
				options->ErrorOut(&errorCode, false, "Error phonemes file format for wave \"%s\".", wave->GetName().str);
				break;
			}
			//Выделяем память
			const dword phonemesPartSize = sizeof(ProjectWave::ExportPhonemesPart);
			Assert(sizeof(ProjectWave::ExportPhonemesPart::Phoneme) == 9);
			buffer = NEW byte[phonemesPartSize + sizeof(ProjectWave::ExportPhonemesPart::Phoneme)*(phonemesCount - 1)];
			//Зачитываем фонемы
			ProjectWave::ExportPhonemesPart & part = *(ProjectWave::ExportPhonemesPart *)buffer;
			part.count = phonemesCount;
			part.size = phonemesPartSize + sizeof(ProjectWave::ExportPhonemesPart::Phoneme)*(phonemesCount - 1);
			part.waveCheckSum = wave->GetWaveDataCheckSum();
			part.waveDataSize = wave->GetWaveDataSize();
			readBytes = 0;
			if(::ReadFile(handle, part.phoneme, sizeof(ProjectWave::ExportPhonemesPart::Phoneme)*phonemesCount, &readBytes, null) == FALSE || readBytes != sizeof(ProjectWave::ExportPhonemesPart::Phoneme)*phonemesCount)
			{
				delete buffer;
				buffer = null;
				options->ErrorOut(&errorCode, false, "Error load phonemes data for wave \"%s\".", wave->GetName().str);
				break;
			}
			//Проверяем, чтобы время было всегда больше или равно чем в предыдущей фонеме и индекс фонемы
			dword curTime = 0;
			for(dword i = 0; i < part.count; i++)
			{
				if(part.phoneme[i].time < curTime || part.phoneme[i].id == 255)
				{
					delete buffer;
					buffer = null;
					options->ErrorOut(&errorCode, false, "Error phonemes data content for wave \"%s\".", wave->GetName().str);
					break;
				}
				curTime = part.phoneme[i].time;
			}
			size = part.size;
			break;
		}
		CloseHandle(handle);
	}
	//Удаляем файлы
	::DeleteFile(bufferName);
	::DeleteFile(cmdLine);
	wave->UnloadWaveData(ProjectWaveDebugId_genphonemes);
	return buffer;
}

//Выполнение внешнего конвертора
bool ProjectWaveExporter::DoExternProcess(const char * commandString)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	memset(&pi, 0, sizeof(pi));
	//Приоритет
	dword priority = NORMAL_PRIORITY_CLASS;
	if(globalObject.activeWorkModeCounter == 0)
	{
		priority = IDLE_PRIORITY_CLASS;
	}
	//Запускаем процесс конвертера
	if(!CreateProcess(null, (LPTSTR)commandString, null, null, false, priority | CREATE_NO_WINDOW, null, null, &si, &pi))
	{
		api->Trace("CreateProcess failed (GetLastError() == %d)", GetLastError());
		api->Trace("    command string: %s", commandString);
		return false;
	}
	//Ожидаем конвертации
	WaitForSingleObject(pi.hProcess, INFINITE);
	//Закрываем хэндлы
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return true;
}

