//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// SndEditorMainWindow
//============================================================================================



#include "SndOptions.h"
#include "..\..\Common_h\corecmds.h"
#include "..\..\Common_h\InputSrvCmds.h"

SndOptions * options = null;
SndProject * project = null;


ErrorId ErrorId::ok;


SndOptions::SndOptions() : errorCodes(_FL_, 4096),
							saveBuffer(_FL_, 4096),
							collection(_FL_, 256)
{
	Assert(!options);
	errorCounter = 0;
	options = this;
	//Службы
	project = null;
	fileService = (IFileService * )api->GetService("FileService");
	soundService = (ISoundService *)api->GetService("SoundService");
	sa = (ISoundEditorAccessor *)api->GetService("SoundEditorAccessor");
	render = (IRender * )api->GetService("DX9Render");
	screenWidth = render->GetScreenInfo3D().dwWidth;
	screenHeight = render->GetScreenInfo3D().dwHeight;
	kbFocus = null;
	dndControl = null;
	gui_manager = null;
	Assert(render);
	controlService = (IControlsService *)api->GetService("ControlsService");
	controls = controlService->CreateInstance(_FL_);
	Assert(controls)
	IAnimationService * ass = (IAnimationService *)api->GetService("AnimationService");
	Assert(ass);
	aniScene = ass->CreateScene(_FL_);
	Assert(aniScene);
	//Системная информация
	char buffer[MAX_PATH + 128];
	::GetCurrentDirectory(MAX_PATH + 127, buffer);
	buffer[MAX_PATH + 127] = 0;
	pathProject = buffer;
	pathProject += "\\sound project\\";
	pathProject.CheckPath();
	fileService->BuildPath(pathProject.c_str(), pathProject);
	pathProjectXmlz = buffer;
	pathProjectXmlz += "\\MissionsSRC\\sound project\\";
	pathTempWorkFolder = buffer;
	pathTempWorkFolder += "\\sound project local\\";
	pathGlobalSettings = pathProject;
	pathGlobalSettings += "settings\\";
	pathLocalSettings = pathTempWorkFolder;
	pathLocalSettings += "settings\\";
	pathTempSaveFolder = pathTempWorkFolder;
	pathTempSaveFolder += "waves\\";
	pathTempLogs = pathTempWorkFolder;
	pathTempLogs += "logs\\";
	pathExportEvents = buffer;
	pathExportEvents += "\\AnimationProject\\SoundEvents\\";
	pathStageLog = pathTempLogs;
	pathStageLog += "MovieStage_log.txt";
	pathPhonemesLog = pathTempLogs;
	pathPhonemesLog += "Phonemes_export_log.txt";
	pathLeakWaves = pathTempWorkFolder;
	pathLeakWaves += "tmpwaves\\";
	pathGameMissionsIni = pathGlobalSettings;
	pathGameMissionsIni += "missions.v10";
	pathVersionFiles = pathTempWorkFolder;
	pathVersionFiles += "ver\\";
	pathProjectPolygonOptions = pathLocalSettings;
	pathProjectPolygonOptions += "polygon.v10";
	//Политики доступа
	IIniFile * ini = fileService->SystemIni();
	policy = policy_sounder;
	isDeveloperMode = false;
	isEnableSwitchPolicy = false;
	leakWavesMode = 0;
	if(ini)
	{
		long magic = ini->GetLong(null, "plmgcn", 0);
		if(magic == policy_magic_designer)
		{
			policy = policy_designer;
			isEnableSwitchPolicy = true;
		}else
		if(magic == policy_magic_developer_d)
		{
			policy = policy_designer;
			isDeveloperMode = true;
			isEnableSwitchPolicy = true;
		}else
		if(magic == policy_magic_developer_s)
		{
			policy = policy_sounder;
			isDeveloperMode = true;
			isEnableSwitchPolicy = true;
		}
		leakWavesMode = ini->GetLong(null, "savemode", 0);
	}
	//Проверяем наличие папок
	fileService->CreateFolder(pathProject.c_str());
	fileService->CreateFolder(pathTempWorkFolder.c_str());
	fileService->CreateFolder(pathGlobalSettings.c_str());
	fileService->CreateFolder(pathLocalSettings.c_str());
	fileService->CreateFolder(pathTempSaveFolder.c_str());
	fileService->CreateFolder(pathProjectXmlz.c_str());	
	fileService->CreateFolder(pathExportEvents.c_str());
	fileService->CreateFolder(pathTempLogs.c_str());
	fileService->CreateFolder(pathVersionFiles.c_str());
	if(leakWavesMode)
	{
		fileService->CreateFolder(pathLeakWaves.c_str());
	}
	//Шрифты
	uiFont = render->GetSystemFont();
	Assert(uiFont);	
	//Цвета
	bkg = 0xffb0b0b0;
	colorTextHi = 0xffffffff;
	colorTextLo	= 0xff000000;
	colorSelect = 0x400070ff;
	Color cbkg(bkg);
	Color white(1.0f, 1.0f, 1.0f, 1.0f);
	for(long i = 0; i < 8; i++)
	{
		float k = i/8.0f;
		Color b2b = cbkg*k;
		black2Bkg[i] = b2b.GetDword();
		Color b2w = cbkg*k + white*(1.0f - k);
		bkg2White[7 - i] = b2w.GetDword();
	}
	//Стандартные картинки
	imageCloseEditor.Load("sndeditor\\closeeditor.txx");			//Картинка закрытия редактора
	imageWave.Load("sndeditor\\wave.txx");							//Волновые данные
	imageWaveRootFolder.Load("sndeditor\\waverootfolder.txx");		//Волновые данные корневая папка
	imageWaveOpenFolder.Load("sndeditor\\waveopenfolder.txx");		//Волновые данные открытая папка
	imageSound.Load("sndeditor\\sound.txx");						//Звук
	imageSoundRootFolder.Load("sndeditor\\soundrootfolder.txx");	//Звуковые данные корневая папка
	imageSoundOpenFolder.Load("sndeditor\\soundopenfolder.txx");	//Звуковые данные открытая папка
	imageCloseFolder.Load("sndeditor\\closefolder.txx");			//Закрытая папка
	imageOpenFolder.Load("sndeditor\\openfolder.txx");				//Открытая папка
	imageUp.Load("sndeditor\\toup.txx");							//Подняться вверх по иерархии
	imageComputer.Load("sndeditor\\computer.txx");					//Компьютер
	imageXbox.Load("sndeditor\\xbox.txx");							//Xbox
	imagePCM.Load("sndeditor\\formatpcm.txx");						//Формат PCM
	imageXWma.Load("sndeditor\\formatxwma.txx");					//Формат XWma (ноты)
	imageXma.Load("sndeditor\\formatxma.txx");						//Формат Xma
	imageWaitToSave.Load("sndeditor\\waittosave.txx");				//Ждёт записи
	imageAddNewBank.Load("sndeditor\\addnewbank.txx");				//Добавить новый звуковой банк
	imageSoundBanks.Load("sndeditor\\soundbanks.txx");				//Звуковые банки				
	imageSoundBank.Load("sndeditor\\soundbank.txx");				//Звуковой банк
	imageSoundBankExp.Load("sndeditor\\soundbankexp.txx");			//Экспортировать звуковой банк
	imageSoundBankOpts.Load("sndeditor\\soundbankopts.txx");		//Опции звукового банка
	imageEar.Load("sndeditor\\ear.txx");							//Значёк уха
	imageAttenuations.Load("sndeditor\\attenuations.txx");			//Значёк графиков затухания от дистанции
	imageSoundSetup.Load("sndeditor\\soundsetup.txx");				//Значёк настроек звука
	imageChanalTime.Load("sndeditor\\chanaltime.txx");				//Значёк секундомера (время занимающее канал)	
	imageAniEvtGlb.Load("sndeditor\\anievtglb.txx");				//Картинка для анимационного эвента глобального звука
	imageAniEvtGlbSel.Load("sndeditor\\anievtglbsel.txx");			//Картинка для анимационного эвента глобального звука с выделением
	imageAniEvtLoc.Load("sndeditor\\anievtloc.txx");				//Картинка для анимационного эвента звука в локаторе
	imageAniEvtLocSel.Load("sndeditor\\anievtlocsel.txx");			//Картинка для анимационного эвента звука в локаторе с выделением
	imageAnimations.Load("sndeditor\\animations.txx");				//Картинка для списка анимаций
	imageAnimation.Load("sndeditor\\animation.txx");				//Картинка анимации
	imageAniMovie.Load("sndeditor\\animovie.txx");					//Картинка ролика
	imageLocator.Load("sndeditor\\locator.txx");					//Картинка для списка локаторов	
	imagePlay.Load("sndeditor\\play.txx");							//Проиграть
	imagePlayNode.Load("sndeditor\\playnode.txx");					//Проиграть текущий нод
	imagePlayVol.Load("sndeditor\\playvol.txx");					//Проиграть с применением громкости
	imageStop.Load("sndeditor\\stop.txx");							//Остановить проигрывание
	imageMoveToStart.Load("sndeditor\\start.txx");					//На начало
	imageMoveToStartNode.Load("sndeditor\\startnode.txx");			//На начало нода
	imageMoveToEnd.Load("sndeditor\\end.txx");						//В конец
	imageMoveToEndNode.Load("sndeditor\\endnode.txx");				//В конец нода
	imageMoveToPrev.Load("sndeditor\\prev.txx");					//В предыдущую позицию
	imageMoveToNext.Load("sndeditor\\next.txx");					//В следующую позицию	
	imageAddSound.Load("sndeditor\\soundadd.txx");					//Добавить звук
	imageAddWave.Load("sndeditor\\waveadd.txx");					//Добавить волну
	imageAddSilence.Load("sndeditor\\silenceadd.txx");				//Добавить слот тишины
	imageImportWave.Load("sndeditor\\waveimport.txx");				//Импортировать волны
	imageAddFolder.Load("sndeditor\\folderadd.txx");				//Добавить папку
	imageRename.Load("sndeditor\\rename.txx");						//Переименовать
	imageDelete.Load("sndeditor\\delete.txx");						//Удалить
	imageAddSetup.Load("sndeditor\\setupadd.txx");					//Добавить настройки звука
	imageAddAtts.Load("sndeditor\\attadd.txx");						//Добавить график затухания
	imageStageMovie.Load("sndeditor\\stagemovie.txx");				//Состояние редактирования клипа
	imageStageEvents.Load("sndeditor\\stageevents.txx");			//Состояние расстановки событий
	imageStageApproval.Load("sndeditor\\stageapproval.txx");		//Состояние приёмки
	imageStageDone.Load("sndeditor\\stagedone.txx");				//Законченный клип
	imageStageCheck.Load("sndeditor\\stagecheck.txx");				//Были изменения, требуеться проверка
	imageStageDamage.Load("sndeditor\\stagedamage.txx");			//Были разрушены данные в файле, требуеться детальное рассмотрение
	imageExportError.Load("sndeditor\\exporterror.txx");			//Входное сообщение об ошибке
	imageLineArrow.Load("sndeditor\\linearrow.txx");				//Стрелка вправо
	imageModel.Load("sndeditor\\model.txx");						//Моделька
	imageCamera.Load("sndeditor\\camera.txx");						//Картинка камеры
	//Строки для UI	
	for(long i = 0; i < ARRSIZE(strings); i++)
	{
		static const char * emptyString = "";
		strings[i] = emptyString;
	}
	InitStrings();
	//Предзагрузочные утилиты
	PreloadUtilites();
	//Проект
	NEW SndProject();	
	//project->Load();
}

SndOptions::~SndOptions()
{
	WavePreviewStop();
	if(project) delete project;
	Assert(options == this);
	options = null;	
	controls->Release();
	controls = null;
}

//Получить строку редактора
const char * SndOptions::GetString(dword id)
{
	Assert(id < ARRSIZE(strings));
	return strings[id];
}

//Сгенерировать законченную ошибку
ErrorId _cdecl SndOptions::ErrorOut(ErrorId * id, bool isReady, const char * format, ...)
{
	errorCodeAccessor.Enter();
	//Печатаем в буфер строку
	static char buffer[4096];	
	crt_vsnprintf(buffer, sizeof(buffer) - 1, format, (va_list)((&format) + 1));
	//Новая ошибка
	if(!id || id->IsOk())
	{
		//Выделяем запись для ошибки
		for(dword i = 0; i < errorCodes.Size(); i++)
		{
			if(errorCodes[i].id < 0) break;
		}
		if(i >= errorCodes.Size())
		{
			i = errorCodes.Add();
		}
		//Проверяем идентификатор для работы с ошибкой
		if(errorCounter > 100000000)
		{
			//Странно, так не должно быть, очищаем всё
			errorCounter = 0;
			for(dword j = 0; j < errorCodes.Size(); j++)
			{
				errorCodes[i].id = -1;
				errorCodes[i].str.Empty();
			}
		}	
		//Заполняем запись
		errorCodes[i].id = errorCounter++;
		errorCodes[i].str = buffer;
		errorCodes[i].isDone = isReady;
		ErrorId retId;
		retId.id = errorCodes[i].id;
		errorCodeAccessor.Leave();
		return retId;
	}
	//Ищим среди добавленных
	Assert(id);
	for(dword i = 0; i < errorCodes.Size(); i++)
	{
		if(errorCodes[i].id == id->id && !errorCodes[i].isDone)
		{
			errorCodes[i].str = buffer;
			if(isReady)
			{
				errorCodes[i].isDone = true;
			}
			break;
		}
	}
	errorCodeAccessor.Leave();
	return *id;
}

//Отменить ошибку
void SndOptions::ErrorCancel(ErrorId id)
{
	errorCodeAccessor.Enter();
	for(dword i = 0; i < errorCodes.Size(); i++)
	{
		if(errorCodes[i].id == id.id)
		{
			errorCodes[i].id = -1;
			errorCodes[i].str.Empty();
			errorCodes[i].isDone = true;
			break;
		}
	}
	errorCodeAccessor.Leave();
}

//Получить ошибку
void SndOptions::ErrorGet(ErrorId id, string & error, bool isDelete)
{
	error.Empty();
	errorCodeAccessor.Enter();
	for(dword i = 0; i < errorCodes.Size(); i++)
	{
		if(errorCodes[i].id == id.id)
		{
			if(errorCodes[i].isDone)
			{
				error = errorCodes[i].str;
				if(isDelete)
				{
					errorCodes[i].id = -1;
					errorCodes[i].str.Empty();
					errorCodes[i].isDone = true;
				}
			}
			break;
		}
	}
	errorCodeAccessor.Leave();
}

//Получить лог ошибок
void SndOptions::ErrorGetLog(array<string> & errors)
{
	errorCodeAccessor.Enter();
	errors.DelAll();
	errors.Reserve(errorCodes.Size());
	for(dword i = 0; i < errorCodes.Size(); i++)
	{
		ErrorCode & ecode = errorCodes[i];
		if(ecode.id >= 0 && ecode.isDone)
		{
			errors.Add(ecode.str);
		}
	}
	errorCodeAccessor.Leave();
}

//Очистить лог ошибок
void SndOptions::ErrorClear()
{
	errorCodeAccessor.Enter();
	errorCodes.DelAll();
	errorCodeAccessor.Leave();	
}

//Надо ли подсветить контрол под курсором мыши
bool SndOptions::IsHighlight(GUIControl * ctrl)
{	
	if(!ctrl || !ctrl->Enabled) return false;
	if(gui_manager->GetElementUnderCursor() == ctrl)
	{
		GUIControl * c = gui_manager->GetTopModal();
		if(c != null)
		{
			for(GUIControl * look = ctrl; look; look = look->GetParent())
			{
				if(look == c)
				{
					return true;
				}
			}
		}else{
			return true;
		}
	}
	return false;
}

//Идут ли прослушивание волны
bool SndOptions::WaveIsPreview()
{
#ifndef NO_TOOLS
	static UniqId zeroId;
	return options->sa->EditPrewiewIsPlay(zeroId.data);
#else
	return false;
#endif
}

//Остановить прослушивание
void SndOptions::WavePreviewStop()
{
#ifndef NO_TOOLS
	static UniqId zeroId;
	options->sa->EditPrewiewStop(zeroId.data);
	options->sa->EditStopAllSounds(SoundSceneName);
#endif
}

//Установить в движке настройки ввода
void SndOptions::SetInputOptions()
{
	api->ExecuteCoreCommand(CoreCommand_LockDebugKeys(true));	
	long num = controlService->GetControlGroupsQuantity();
	for(long i = 0; i < num; i++)
	{
		controlService->EnableControlGroup(i, false);
	}	
	controlService->EnableControlGroup("GUI", true);
	controlService->ExecuteCommand(InputSrvLockMouse(false));
}

//Предзагрузочные утилиты
void SndOptions::PreloadUtilites()
{
//	IIniFile * ini = fileService->SystemIni();
//	if(!ini) return;
	//Назначить рандомный выбор волны для звуков с 1ной волной
	//SetSelectModeForSound();
	//Проверяем необходимость копирования звукового банка
	IIniFile * ini = fileService->OpenIniFile("SE_duplicatebank.ini", _FL_);
	if(ini)
	{
		api->Trace("===========================================================================");
		api->Trace("Sound editor PreloadUtilites::DuplicateSoundBank");
		api->Trace("===========================================================================");
		bool isRename = DuplicateSoundBank(ini);
		ini->Release();
		ini = null;
		if(isRename)
		{
			fileService->Rename("SE_duplicatebank.ini", "SE_duplicatebank.bak");
			api->Trace("Sound editor PreloadUtilites::DuplicateSoundBank saccesful.");
		}else{
			api->Trace("Sound editor PreloadUtilites::DuplicateSoundBank fail. Look log up for details...");
		}
		api->Trace("===========================================================================");
	}
}



//Сдублировать звуковой банк
bool SndOptions::DuplicateSoundBank(IIniFile * ini)
{
	//Зачитываем параметры дублирования
	Assert(ini);
	const char * tmpStr = ini->GetString(null, "source bank", null);
	if(string::IsEmpty(tmpStr))
	{
		api->Trace("Sound editor PreloadUtilites::DuplicateSoundBank fail. Not set \"source bank\"");
		return false;
	}
	string sourcePath = pathProject;
	sourcePath += "Sounds\\";
	sourcePath += tmpStr;
	sourcePath += "\\";	
	tmpStr = ini->GetString(null, "duplicate bank", null);
	if(string::IsEmpty(tmpStr))
	{
		api->Trace("Sound editor PreloadUtilites::DuplicateSoundBank fail. Not set \"duplicate bank\"");
		return false;
	}
	string duplicatePath = pathProject;
	duplicatePath += "Sounds\\";
	duplicatePath += tmpStr;
	duplicatePath += "\\";
	tmpStr = ini->GetString(null, "waves folder", null);
	if(string::IsEmpty(tmpStr))
	{
		api->Trace("Sound editor PreloadUtilites::DuplicateSoundBank fail. Not set \"waves folder\"");
		return false;
	}
	string copyWavesPath = pathProject;
	copyWavesPath += "Waves\\";
	copyWavesPath += tmpStr;
	copyWavesPath += "\\";
	tmpStr = ini->GetString(null, "waves prefix", null);
	if(string::IsEmpty(tmpStr))
	{
		api->Trace("Sound editor PreloadUtilites::DuplicateSoundBank fail. Not set \"waves prefix\"");
		return false;
	}
	string wavesPrefix = tmpStr;
	tmpStr = null;
	//Проверяем места назначения (не должно быть таких объектов)
	if(fileService->IsExist(duplicatePath.c_str()))
	{
		api->Trace("Sound editor PreloadUtilites::DuplicateSoundBank fail. File object \"%s\" already exist! Delete it before duplicate.", duplicatePath.c_str());
		return false;
	}
	if(fileService->IsExist(copyWavesPath.c_str()))
	{
		api->Trace("Sound editor PreloadUtilites::DuplicateSoundBank fail. File object \"%s\" already exist! Delete it before duplicate.", copyWavesPath.c_str());
		return false;
	}
	//Копируем файлы звуков
	IFinder * finder = fileService->CreateFinder(sourcePath.c_str(), "*.*", find_no_mirror_files | find_no_files_from_packs, _FL_);
	Assert(finder);	
	string dstPath;
	string tmpPath;
	bool isErrors = false;
	for(dword i = 0; i < finder->Count(); i++)
	{
		if(!string::IsEqual(finder->Extension(i), "txt"))
		{
			continue;
		}
		//Создаём аналогичную папку внутри нового банка
		tmpPath = finder->Path(i);
		tmpPath.GetRelativePath(sourcePath);
		dstPath = duplicatePath;
		dstPath += tmpPath;
		fileService->CreateFolder(dstPath.c_str());
		//Копируем файл
		dstPath += finder->Name(i);
		if(!fileService->Copy(finder->FilePath(i), dstPath.c_str()))
		{
			isErrors = true;
			api->Trace("Sound editor PreloadUtilites::DuplicateSoundBank fail. Error copy \"%s\" to \"%s\"", finder->FilePath(i), dstPath.c_str());
		}
	}
	finder->Release();
	if(isErrors)
	{
		return false;
	}
	//Собираем таблицы соответствий волн и идентификаторов
	string wavesPath = pathProject;
	wavesPath += "Waves\\";
	finder = fileService->CreateFinder(wavesPath.c_str(), "*.*", find_no_mirror_files | find_no_files_from_packs, _FL_);
	Assert(finder);
	array<string> wavesTable(_FL_);	
	wavesTable.AddElements(finder->Count());
	array<UniqId> wavesIds(_FL_);
	wavesIds.AddElements(finder->Count());
	UniqId tmpId;
	for(dword i = 0; i < finder->Count(); i++)
	{
		if(!string::IsEqual(finder->Extension(i), "pwv"))
		{
			continue;
		}
		IDataFile * file = fileService->OpenDataFile(finder->FilePath(i), file_open_fromdisk, _FL_);
		char buf[64];
		memset(buf, 0, sizeof(buf));
		file->Read(buf, sizeof(buf) - 1);
		file->Release();
		if(!tmpId.FromString(buf))
		{			
			api->Trace("Sound editor PreloadUtilites::DuplicateSoundBank message. Can't read id from wave \"%s\".", finder->FilePath(i));
			wavesIds[i].Reset();
			continue;
		}
		wavesTable[i] = finder->FilePath(i);
		wavesIds[i] = tmpId;
	}
	//Папка для копирования волн
	fileService->CreateFolder(copyWavesPath.c_str());
	//Обрабатываем звуки в новом банке
	finder = fileService->CreateFinder(duplicatePath.c_str(), "*.txt", find_no_mirror_files | find_no_files_from_packs, _FL_);
	Assert(finder);	
	for(dword i = 0; i < finder->Count(); i++)
	{
		IEditableIniFile * snd = fileService->OpenEditableIniFile(finder->FilePath(i), file_open_existing, _FL_);
		if(!snd)
		{
			isErrors = true;
			api->Trace("Sound editor PreloadUtilites::DuplicateSoundBank fail. Can't open \"%s\" for modify.", finder->FilePath(i));
			continue;
		}
		//Изменяем идентификатор
		const char * oldId = snd->GetString("Sound", "id", null);
		if(string::IsEmpty(oldId))
		{
			snd->Release();
			isErrors = true;
			api->Trace("Sound editor PreloadUtilites::DuplicateSoundBank fail. Can't open \"%s\" for modify.", finder->FilePath(i));			
			continue;
		}
		tmpId.Build();
		snd->SetString("Sound", "id", tmpId.ToString());
		//Пробегаемся по волнам, копируя их и назначая
		long wavesCount = snd->GetLong("Sound", "wavescount", 0);
		if(wavesCount <= 0)
		{
			snd->Release();
			isErrors = true;
			api->Trace("Sound editor PreloadUtilites::DuplicateSoundBank fail. Sound \"%s\" have got incorrect count of waves (%i)", finder->FilePath(i), wavesCount);
			continue;
		}
		for(long j = 0; j < wavesCount; j++)
		{
			char section[64];
			crt_snprintf(section, sizeof(section), "wave_%i", j);
			const char * waveId = snd->GetString(section, "id", null);
			long index = -1;
			UniqId wId;
			if(wId.FromString(waveId))
			{
				if(!wId.IsValidate())
				{
					//Тишина
					continue;
				}
				for(index = 0; index < (long)wavesIds.Size(); index++)
				{
					if(wavesIds[index] == wId)
					{
						break;
					}
				}
				if(index >= (long)wavesIds.Size())
				{
					index = -1;
					isErrors = true;
					api->Trace("Sound editor PreloadUtilites::DuplicateSoundBank fail. Sound \"%s\" have got incorrect wave id  (wave %s not found)", finder->FilePath(i), waveId);
				}
			}else{
				isErrors = true;
				api->Trace("Sound editor PreloadUtilites::DuplicateSoundBank fail. Sound \"%s\" have got incorrect wave description (%s)", finder->FilePath(i), waveId);
			}
			tmpId.Build();
			if(index >= 0)
			{
				//Копируем волну
				tmpPath.GetFileName(wavesTable[index]);
				dstPath = copyWavesPath;
				dstPath += wavesPrefix;
				dstPath += tmpPath;
				if(!fileService->Copy(wavesTable[index].c_str(), dstPath.c_str()))
				{
					isErrors = true;
					api->Trace("Sound editor PreloadUtilites::DuplicateSoundBank fail. Error copy \"%s\" to \"%s\"", wavesTable[index].c_str(), dstPath.c_str());
				}
				//Дадим поработать другим потокам
				Sleep(1);
				//Изменяем идентификатор
				IFile * file = fileService->OpenFile(dstPath.c_str(), file_open_existing, _FL_);
				if(file)
				{
					const char * sId = tmpId.ToString();
					dword sizeForWrite = string::Len(sId);
					if(!file->Write(sId, sizeForWrite))
					{
						isErrors = true;
						api->Trace("Sound editor PreloadUtilites::DuplicateSoundBank fail. Error write data to wave file \"%s\"", dstPath.c_str());
					}
					file->Release();
				}else{
					isErrors = true;
					api->Trace("Sound editor PreloadUtilites::DuplicateSoundBank fail. Can't open new wave file \"%s\"", dstPath.c_str());
				}
			}
			snd->SetString(section, "id", tmpId.ToString());
		}
		snd->Release();
	}
	finder->Release();
	return !isErrors;
}


//Установить режим выбора волн в звуках с 1 волной
void SndOptions::SetSelectModeForSound()
{
	string sourcePath = pathProject;
	sourcePath += "Sounds\\";
	IFinder * finder = fileService->CreateFinder(sourcePath.c_str(), "*.txt", find_no_mirror_files | find_no_files_from_packs, _FL_);
	Assert(finder);	
	for(dword i = 0; i < finder->Count(); i++)
	{
		IEditableIniFile * snd = fileService->OpenEditableIniFile(finder->FilePath(i), file_open_existing, _FL_);
		long count = snd->GetLong("Sound", "wavescount", 0);
		if(count == 1)
		{
			long selector = snd->GetLong("Setup", "selector", sbpc_select_rnd);
			if(selector != sbpc_select_rnd)
			{
				snd->SetLong("Setup", "selector", sbpc_select_rnd);
			}
		}
		snd->Release();
	}
}




void ExtName::UpdateParams()
{
	if(str)
	{
		hash = string::HashNoCase(str, len);
		w = long(options->uiFont->GetLength(str) + 0.4999f);
		h = long(options->uiFont->GetHeight(str) + 0.4999f);
	}else{
		len = 0;
		hash = 0;
		w = 0;
		h = 0;
	}
}

#ifndef NO_TOOLS
#include <Rpc.h>
#pragma comment(lib, "Rpcrt4")
#endif

//Сгенерировать уникальный идентификатор
void UniqId::Build()
{
#ifndef NO_TOOLS
	UUID uid;
	RPC_STATUS retUuidCreate = UuidCreate(&uid);
	Assert(retUuidCreate == RPC_S_OK);
	Assert(sizeof(uid) == sizeof(UniqId));
	memcpy(&data, &uid, sizeof(UniqId));
	Assert(IsValidate());
#else
//Сюда не должно попадать
Assert(false);
#endif
}


UniqId UniqId::zeroId;

//Сконвертировать идентификатор в строку, оканчивающуюся на 0 (забирать сразу в свой буфер)
const char * UniqId::ToString() const
{
	static char buffer[(sizeof(data)*2 + 3 + 1) + 16];
	static const char * hex = "0123456789ABCDEF\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
	const byte * src = (const byte *)&data;	
	char * dst = buffer;
	for(dword i = 0; i < sizeof(data); i++)
	{
		if(i == 2 || i == 4 || i == 8)
		{
			*dst = '-';
			dst++;
		}
		byte b = *src;
		dst[0] = hex[(b >> 4) & 0xf];
		dst[1] = hex[b & 0xf];
		src++;
		dst+= 2;
	}
	*dst = 0;
	return buffer;
}

//Востановить идентификатор из строки
bool UniqId::FromString(const char * s)
{
	data[0] = data[1] = data[2] = data[3] = 0;
	if(!s) return false;
	dword buf[4];
	buf[0] = buf[1] = buf[2] = buf[3] = 0;
	const char * src = s;
	byte * dst = (byte *)buf;
	for(dword i = 0; i < sizeof(data)*2; i++, src++)
	{
		if(i == 2*2 || i == 4*2 || i == 8*2)
		{
			if(*src != '-')
			{
				//Ошибка формата
				return false;
			}
			src++;
		}
		dword hex = 0;
		switch(*src)
		{
		case '0': hex = 0; break;
		case '1': hex = 1; break;
		case '2': hex = 2; break;
		case '3': hex = 3; break;
		case '4': hex = 4; break;
		case '5': hex = 5; break;
		case '6': hex = 6; break;
		case '7': hex = 7; break;
		case '8': hex = 8; break;
		case '9': hex = 9; break;
		case 'A': 
		case 'a': hex = 10; break;
		case 'B': 
		case 'b': hex = 11; break;
		case 'C': 
		case 'c': hex = 12; break;
		case 'D': 
		case 'd': hex = 13; break;
		case 'E': 
		case 'e': hex = 14; break;
		case 'F': 
		case 'f': hex = 15; break;
		default:
			//Ошибка формата
			return false;
		};
		if((i & 1) == 0)
		{
			*dst = hex << 4;			
		}else{
			*dst |= hex;
			dst++;
		}
	}
	data[0] = buf[0];
	data[1] = buf[1];
	data[2] = buf[2];
	data[3] = buf[3];
	return true;
}


