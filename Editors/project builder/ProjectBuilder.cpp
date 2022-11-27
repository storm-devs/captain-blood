
/*

-----------------------------------------------------------------------------------

Базовая секция билдера:

1. Параметр обязательный
todo = xml2mis экспортировать xml в mis, build собирать билд

2. Параметр обязаательный
build path pc = Полный путь до места, где собирать базовый билд и билд для PC
                
3. Параметр необязательный по умолчанию "", те хвох билд не собирается
build path xbox = Путь до места, где билд собирать для Xbox

4. Параметр обязательный
resources path = Относительный путь где лежат ресурсы

5. Параметр обязательный
missions path = Относительный путь где лежат миссии

6. Параметр не обязательный по умолчанию ""
backup path pc = Абсолютный путь до места куда скопировать готовый PC билд для сохранения
                 Если путь не указан, то копия билда не сохраняется

7. Параметр не обязательный по умолчанию ""
backup path xbox = Абсолютный путь до места куда скопировать готовый PC билд для сохранения
                   Если путь не указан, то копия билда не сохраняется

8. Параметр необязательный, может повторятся любое количество раз

Формы записи команд:
    command platform = path
    command platform = pathFrom>pathTo

	

command:
    copy копировать из папки исходного билда в создаваемый без рекурсии
    copy_rec копировать из папки исходного билда в создаваемый c рекурсией
    incopy копировать между папками создаваемого билда
    incopy_rec копировать между папками создаваемого билда c рекурсией, без использования '>' не имеет смысла
    delete удалить файлы в создаваемом билде без рекурсии
    delete удалить файлы в создаваемом билде c рекурсией

platform:
    pc команда для создаваемого билда [build path pc]
	xbox команда для создаваемого билда [build path xbox]


9. Параметр необязательный, может повторятся любое количество раз
skip = маска до файла(ов) которые надо пропустить при проверки на повторяемость

-----------------------------------------------------------------------------------

описание пака
[произвольное уникальное имя секции]

1.
pack_type = тип

Типы:
global pc - пак из глобальных ресурсов для PC билда
global xbox - пак из глобальных ресурсов для XBOX билда
mission pc - пак из миссионых ресурсов для PC билда
global xbox - пак из миссионых ресурсов для XBOX билда

2.
path = Путь относительный "resources path" для глобальных ресурсов или "missions path"+"mission folder" для миссионых

3.
file = имя пак файла, который будет сохранён по указоному пути
если в имени присутствует '*' то этот символ заменяеться на имя миссии (1 попавшийся)

4.
mask = маска файлов которые будут добавлены в пак

5.
delete = 1 удалять файлы, добавленые в пак, 0 оставить

6.
compress = 1 сжимать пак файл, 0 нет

*/


#include "ProjectBuilder.h"


#ifndef NO_TOOLS
#ifndef _XBOX

#pragma message("Compile project builder")

#include "..\..\MissionEditor\MissionEditorExport.h"

CREATE_SERVICE(ProjectBuilderService, 1)


#pragma warning(disable:4996)

//#include "..\..\System\XRender\X360TexConvertor.h"
//#include "..\..\System\XRender\X360TexConvertor.cpp"



ProjectBuilder::ProjectBuilder() : packs(_FL_),									
									copyFiles(_FL_),
									deleteFiles(_FL_),
									compareFilters(_FL_),
									skipMissionsTable(_FL_)
									
{
	Assert(!ProjectBuilderService::builder);
	ProjectBuilderService::builder = this;
	finder = null;
	fs = (IFileService *)api->GetService("FileService");
	Assert(fs);
	fs->SetDrainPath(null);
	currentMission = null;
}

ProjectBuilder::~ProjectBuilder()
{
	if(finder)
	{
		finder->Release();
		finder = null;
	}
	Assert(ProjectBuilderService::builder == this);
	ProjectBuilderService::builder = null;
}


//Экспорт xml миссий
bool ProjectBuilder::StartExportXMLMissions()
{	
	//Продолжим когда всё загрузится
	ProjectBuilderService::service->Execute(&ProjectBuilder::ExportXMLMissions);
	return true;
}

//Процесс сбора билда из подготовленного проекта
bool ProjectBuilder::StartBuildProcess()
{
	//Считываем конфигурацию
	IIniFile * ini = fs->SystemIni();
	Assert(ini);
	//Рабочий путь, где будем билд собирать
	workPath_PC = ini->GetString("Build options", "build path pc", "");
	if(!workPath_PC.IsEmpty())
	{
		workPath_PC += '\\';
		fs->BuildPath(workPath_PC, workPath_PC);
	}
	if(workPath_PC.IsEmpty())
	{
		Error("Build error: ini file description: Build path is empty!");
		return false;
	}
	workPath_Xbox = ini->GetString("Build options", "build path xbox", "");
	if(workPath_Xbox.IsEmpty())
	{
		Error("Build warning: ini file description: Build path for Xbox is empty, no process that build");
	}else{
		workPath_Xbox += '\\';
		fs->BuildPath(workPath_Xbox, workPath_Xbox);
	}
	//Удалить все файлы с будующего билда
	DeleteFiles(workPath_PC, "*.*", true);
	if(!workPath_Xbox.IsEmpty())
	{
		DeleteFiles(workPath_Xbox, "*.*", true);
	}
	//Путь до ресурсов
	resourcesPath = ini->GetString("Build options", "resources path", "");
	if(resourcesPath.IsEmpty())
	{
		Error("Build error: ini file description: Resources path is empty!");
		return false;
	}
	resourcesPath += '\\';
	resourcesPath.CheckPath();
	//Пути до папок бэкапов
	backupPath_PC = ini->GetString("Build options", "backup path pc", "");
	if(!backupPath_PC.IsEmpty())
	{
		backupPath_PC += '\\';
	}
	backupPath_PC.CheckPath();
	backupPath_Xbox = ini->GetString("Build options", "backup path xbox", "");
	if(!backupPath_Xbox.IsEmpty())
	{
		backupPath_Xbox += '\\';
	}
	backupPath_Xbox.CheckPath();
	//Путь до миссий
	missionsPath = ini->GetString("Build options", "missions path", "");
	if(missionsPath.IsEmpty())
	{
		Error("Build error: ini file description: Missions path is empty!");
		return false;
	}
	missionsPath += '\\';
	missionsPath.CheckPath();
	//Копируемые файлы
	AddOpFiles(copyFiles, ini, "copy pc", false, true, false);	
	AddOpFiles(copyFiles, ini, "copy_rec pc", true, true, false);	
	AddOpFiles(copyFiles, ini, "copy xbox", false, false, false);	
	AddOpFiles(copyFiles, ini, "copy_rec xbox", true, false, false);
	AddOpFiles(copyFiles, ini, "incopy pc", false, true, true);	
	AddOpFiles(copyFiles, ini, "incopy_rec pc", true, true, true);	
	AddOpFiles(copyFiles, ini, "incopy xbox", false, false, true);	
	AddOpFiles(copyFiles, ini, "incopy_rec xbox", true, false, true);
	//Удаляемые файлы
	AddOpFiles(deleteFiles, ini, "delete pc", false, true, false);
	AddOpFiles(deleteFiles, ini, "delete_rec pc", true, true, false);
	AddOpFiles(deleteFiles, ini, "delete xbox", false, false, false);
	AddOpFiles(deleteFiles, ini, "delete_rec xbox", true, false, false);
	//Миссии, которые надо пропустить
	ini->GetStrings("Build options", "skip_mission", skipMissionsTable);
	//Перебираем все секции на поиск описаний паков
	array<string> sections(_FL_);
	ini->GetSections(sections);
	packs.Reserve(sections.Size());
	bool isError = false;
	for(long i = 0; i < sections; i++)
	{
		const char * packType = ini->GetString(sections[i], "pack_type", null);
		PackOptions options;
		if(packType && string::IsEqual(packType, "global pc"))
		{
			options.type = global_pc;
		}else
			if(packType && string::IsEqual(packType, "mission pc"))
			{
				options.type = mission_pc;
			}else
				if(packType && string::IsEqual(packType, "global xbox"))
				{
					options.type = global_xbox;
				}else
					if(packType && string::IsEqual(packType, "mission xbox"))
					{
						options.type = mission_xbox;
					}else{
						continue;
					}		
					options.folderPath = ini->GetString(sections[i], "path", "");
					options.fileName = ini->GetString(sections[i], "file", "");
					options.mask = ini->GetString(sections[i], "mask", "*.*");
					options.compressRatio = ini->GetLong(sections[i], "compress", 0);
					options.isDelete = ini->GetLong(sections[i], "delete", 0) != 0;
					if(options.folderPath.IsEmpty() ||
						options.fileName.IsEmpty() ||
						options.mask.IsEmpty())
					{
						isError = true;
						Error("Build error: ini file description: Section [%s] have incorrect fields (some path is empty)", sections[i].c_str());
					}
					packs.Add(options);
	}
	ini->GetStrings("Build options", "skip", compareFilters);
	if(isError)
	{
		return false;
	}
	IMissionsManagerService * mmService = (IMissionsManagerService *)api->GetService("MissionsManagerService");
	if(mmService)
	{
		mmService->DeleteManager();
	}
	//Удалить все файлы с будующего билда
	DeleteFiles(workPath_PC, "*.*", true);
	if(!workPath_Xbox.IsEmpty())
	{
		DeleteFiles(workPath_Xbox, "*.*", true);
	}
	//Ставим режим сливания файлов
	fs->SetDrainPath(workPath_PC.c_str());
	//Запускаем дополнительную подготовку после создания всех сервисов
	ProjectBuilderService::service->Execute(&ProjectBuilder::BuildProcess);
	return true;
}

//Выгрузка миссий
void ProjectBuilder::ExportXMLMissions()
{
	//Создаём миссионый редактор
	MissionEditorExport * missionEditor = (MissionEditorExport *)api->CreateObject("MissionEditor");
	if(!missionEditor)
	{
		Error("Build error: Can't create MissionEditor!");
	}
	//Удаляем миссионые файлы
	IFinder * fnd = fs->CreateFinder("", "*.mis", find_no_mirror_files | find_no_files_from_packs, _FL_);
	for(dword i = 0; i < fnd->Count(); i++)
	{
		const char * path = fnd->FilePath(i);
		dword attributes = ::GetFileAttributes(path);
		if(attributes != INVALID_FILE_ATTRIBUTES)
		{
			attributes &= ~FILE_ATTRIBUTE_READONLY;
			attributes &= ~FILE_ATTRIBUTE_SYSTEM;
			::SetFileAttributes(path, attributes);
		}
		fs->Delete(path);
	}
	fnd->Release();
	//Экспортируем миссии
	fnd = fs->CreateFinder("MissionsSRC", "*.xml", find_no_mirror_files | find_no_recursive | find_no_files_from_packs, _FL_);
	for(dword i = 0; i < fnd->Count(); i++)
	{
		missionEditor->ConvertXML2MIS(fnd->FilePath(i));
	}
	fnd->Release();
	//Удаляем миссионый редактор
	delete missionEditor;
	missionEditor = null;
	api->Exit();
}

//Добавить процессируемые файлы
void ProjectBuilder::AddOpFiles(array<OpFiles> & arr, IIniFile * ini, const char * key, bool isRecursive, bool isPC, bool inBuildPath)
{
	array<string> cf(_FL_);
	ini->GetStrings("Build options", key, cf);
	long base = arr;
	arr.AddElements(cf.Size());
	for(long i = 0; i < cf; i++)
	{
		arr[base + i].dst = cf[i];
		arr[base + i].src = cf[i];
		for(dword j = 0; j < cf[i].Len(); j++)
		{
			if(cf[i][j] == '>')
			{
				arr[base + i].dst.Delete(0, j + 1);
				arr[base + i].src.Delete(j, arr[base + i].src.Len() - j);
				break;
			}
		}
		arr[base + i].isRecursive = isRecursive;
		arr[base + i].isPC = isPC;
		arr[base + i].inBuildPath = inBuildPath;
	}
}

//Подготовка в сливанию файлов с рабочих миссий
void ProjectBuilder::BuildProcess()
{
	//Не нужен менеджер в этом режиме
	IMissionsManagerService * mmService = (IMissionsManagerService *)api->GetService("MissionsManagerService");	
	if(mmService)
	{
		mmService->DeleteManager();
	}
	//Получаем все доступные папки миссий
	finder = fs->CreateFinder(missionsPath, "*.*", find_folders | find_no_recursive | find_no_mirror_files | find_no_files_from_packs, _FL_);
	if(finder->Count())
	{		
		//После паузы бедем выполнять миссии
		index = 0;
		ProjectBuilderService::service->Execute(&ProjectBuilder::Load);
	}else{
		Error("Build error: No missions found!");
	}
	//Проверим исходный билд на повторяемые файлы
	string currentPath;
	fs->BuildPath("", currentPath);
	CheckFilesForUnique(currentPath.c_str());
}

//Загрузка мисии
void ProjectBuilder::Load()
{
	//Ставим режим сливания файлов
	fs->SetDrainPath(workPath_PC.c_str());
	//Если всё миссии закунчились, выходим
	if(index >= (long)finder->Count())
	{
		ProjectBuilderService::service->Execute(&ProjectBuilder::Build);
		return;
	}
	//Загружаем и стартуем миссию
	const char * name = finder->Title(index++);
	currentMission = null;
	for(dword i = 0; i < skipMissionsTable.Size(); i++)
	{
		if(skipMissionsTable[i] == name)
		{
			api->Trace("Builder message: Skip mission \"%s\"", skipMissionsTable[i].c_str());
			ProjectBuilderService::service->Execute(&ProjectBuilder::Load);
			return;
		}
	}
	IMissionsManagerService * mmService = (IMissionsManagerService *)api->GetService("MissionsManagerService");
	if(mmService)
	{
		currentMission = mmService->CreateMission(name);
	}
	if(!currentMission)
	{
		Error("Build error: mission %s not loaded", name);
		ProjectBuilderService::service->Execute(&ProjectBuilder::Load);
		return;
	}
	workCountFrames = 10;
	//
	ProjectBuilderService::service->Execute(&ProjectBuilder::WorkCounter);
}

//Ожидание работы миссии
void ProjectBuilder::WorkCounter()
{
	workCountFrames--;
	if(workCountFrames <= 0)
	{
		if(currentMission)
		{
			IMissionsManagerService * mmService = (IMissionsManagerService *)api->GetService("MissionsManagerService");
			Assert(mmService);
			mmService->DeleteMission(currentMission);
			currentMission = null;
		}		
		ProjectBuilderService::service->Execute(&ProjectBuilder::Load);
		return;
	}
	ProjectBuilderService::service->Execute(&ProjectBuilder::WorkCounter);
}

//Собираем билд
void ProjectBuilder::Build()
{
	fs->SetDrainPath(null);
	//Удалим финдер
	finder->Release();
	finder = null;
	//Копируем билд для xbox
	if(!workPath_Xbox.IsEmpty())
	{
		CopyFiles(workPath_Xbox, "*.*", workPath_PC, "*.*", true);
	}
	//Копируем дополнительыне файлы
	string basePath;
	fs->BuildPath("", basePath);
	for(long i = 0; i < copyFiles; i++)
	{
		OpFiles & opf = copyFiles[i];
		const string & destBasePath = opf.isPC ? workPath_PC : workPath_Xbox;
		if(!opf.inBuildPath)
		{
			CopyFiles(destBasePath , opf.dst, basePath, opf.src, opf.isRecursive);
		}
	}
	//Cобираем паки
	BuildPacks(workPath_PC, true);	
	if(!workPath_Xbox.IsEmpty())
	{
		//Конвертируем файлы
		ConvertXBoxFiles();
		//Создаём паки для xbox
		BuildPacks(workPath_Xbox, false);
	}
	//Вторая попытка копирования, внутри билда
	fs->BuildPath("", basePath);
	for(long i = 0; i < copyFiles; i++)
	{
		OpFiles & opf = copyFiles[i];
		const string & destBasePath = opf.isPC ? workPath_PC : workPath_Xbox;
		if(opf.inBuildPath)
		{
			CopyFiles(destBasePath , opf.dst, destBasePath, opf.src, opf.isRecursive, true);
		}
	}
	//Удаляем "лишнии" файлы
	for(long i = 0; i < deleteFiles; i++)
	{
		OpFiles & opf = deleteFiles[i];
		const string & destBasePath = opf.isPC ? workPath_PC : workPath_Xbox;
		if(!destBasePath.IsEmpty())
		{
			DeleteFiles(destBasePath , opf.dst, opf.isRecursive);
		}		
	}
	//Удаляем пустые папки
	DeleteEmptyFolders(workPath_PC);
	if(!workPath_Xbox.IsEmpty())
	{
		DeleteEmptyFolders(workPath_Xbox);
	}
	//Проверяем билды на повторяемые файлы
	if(!workPath_PC.IsEmpty())
	{
		CheckFilesForUnique(workPath_PC.c_str());
	}
	if(!workPath_Xbox.IsEmpty())
	{
		CheckFilesForUnique(workPath_Xbox.c_str());
	}
	//Бэкапим билды, если указаны пути
	if(!backupPath_PC.IsEmpty())
	{
		CopyFiles(backupPath_PC, "*.*", workPath_PC, "*.*", true);
	}
	if(!backupPath_Xbox.IsEmpty())
	{
		CopyFiles(backupPath_Xbox, "*.*", workPath_Xbox, "*.*", true);
	}
	//Пишем в лок неиспользуемые строки
	api->Trace("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n##################");
	api->Trace("");
	ILocStrings * ls = (ILocStrings *)api->GetService("LocStrings");
	if(ls) ls->TraceUnuse();
	api->Trace("##################\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
}

//Проходимся по полученным миссиям и глобальным ресурсам, собираем паки
void ProjectBuilder::BuildPacks(const char * workPath, bool isPC)
{	
	//Собираем паки миссий
	string basePath = workPath;
	basePath += missionsPath;
	basePath += '\\';
	IFinder * fnd = fs->CreateFinder(basePath, "*.*", find_folders | find_no_mirror_files | find_no_recursive | find_no_files_from_packs, _FL_);
	string path;
	for(dword i = 0; i < fnd->Count(); i++)
	{
		if(!fnd->IsFolder(i))
		{
			continue;
		}
		const char * folder = fnd->Name(i);
		for(long j = 0; j < packs; j++)
		{
			path = basePath;
			path += folder;
			if(isPC && packs[j].type == mission_pc)
			{
				BuildFolderPack(packs[j], path, folder);
			}
			if(!isPC && packs[j].type == mission_xbox)
			{
				BuildFolderPack(packs[j], path, folder);
			}
		}		
	}
	//Собираем паки глобальных ресурсов
	basePath = workPath;
	basePath += '\\';
	basePath += resourcesPath;
	for(long i = 0; i < packs; i++)
	{
		if(isPC && packs[i].type == global_pc)
		{
			BuildFolderPack(packs[i], basePath, null);
		}
		if(!isPC && packs[i].type == global_xbox)
		{
			BuildFolderPack(packs[i], basePath, null);
		}
	}
}

//Создать пак-файл из папки и удалить файлы из папки
void ProjectBuilder::BuildFolderPack(PackOptions & opt, const char * packPath, const char * replaceName)
{
	//Полный путь
	string fullPath = packPath;
	fullPath += '\\';
	fullPath += opt.folderPath;
	fullPath += '\\';
	fullPath.CheckPath();
	//Собираем файлы из папки пак-файла
	IFinder * fnd = fs->CreateFinder(fullPath, opt.mask, find_no_mirror_files | find_inverse_order | find_no_files_from_packs, _FL_);
	//Создаём пак
	if(fnd->Count() > 0)
	{
		IFileService_PackCompressMethod method = opt.compressRatio ? pack_cmpr_method_archive : pack_cmpr_method_store;
		string correctFileName(opt.fileName);
		if(replaceName)
		{
			for(dword n = 0; n < correctFileName.Len(); n++)
			{
				if(correctFileName[n] == '*')
				{
					correctFileName.Delete(n, 1);
					correctFileName.Insert(n, replaceName);
					break;
				}
			}
		}
		string pckPath = packPath;
		pckPath += '\\';
		pckPath += correctFileName.c_str();
		if(fs->BuildPack(pckPath, fullPath, opt.mask, method))
		{
			if(opt.isDelete)
			{
				//Удаляем все лишнии файлы из пака			
				for(dword i = 0; i < fnd->Count(); i++)
				{
					const char * file = fnd->FilePath(i);
					fs->Delete(file);
				}
			}
		}else{
			Error("Build error: Pack %s%s not created...", fullPath.c_str(), opt.fileName.c_str());
		}
	}
	fnd->Release();
}


//Копировать файлы
void ProjectBuilder::CopyFiles(const string & destBasePath, const string & destPath, const string & srcBasePath, const string & srcMaskPath, bool isRecursive, bool noRelative)
{
	//Полный нормализованый путь до получателя
	string fullDestPathWithMask;
	MakeFullPath(destBasePath, destPath, fullDestPathWithMask);
	string fullDestPath;
	fullDestPath.GetFilePath(fullDestPathWithMask);
	//Полный нормализованый путь до источника
	string fullSrcPathWithMask;
	MakeFullPath(srcBasePath, srcMaskPath, fullSrcPathWithMask);
	string fullSrcPath;
	fullSrcPath.GetFilePath(fullSrcPathWithMask);
	//Путь и маска поиска
	string path, buffer;
	buffer.GetFileName(fullSrcPathWithMask);
	//Собираем все файлы
	dword flags = isRecursive ? 0 : find_no_recursive;
	IFinder * fnd = fs->CreateFinder(fullSrcPath, buffer, find_no_mirror_files | find_no_files_from_packs | flags, _FL_);
	//Процессируем найденые файлы
	for(dword j = 0; j < fnd->Count(); j++)
	{
		//Получаем путь куда копировать		
		if(!noRelative)
		{
			buffer = fnd->FilePath(j);
			buffer.GetRelativePath(fullSrcPath);
		}else{
			buffer = fnd->Name(j);
		}
		path = fullDestPath;
		path += '\\';
		path += buffer;
		path.CheckPath();
		//Создаём папку куда копировать
		buffer.GetFilePath(path);
		fs->CreateFolder(buffer);
		//Нормализуем путь
		fs->BuildPath(path, path);
		//Копируем файл
		if(path != fnd->FilePath(j))
		{
			dword attributes = ::GetFileAttributes(path);
			if(attributes != INVALID_FILE_ATTRIBUTES)
			{
				attributes &= ~FILE_ATTRIBUTE_READONLY;
				attributes &= ~FILE_ATTRIBUTE_SYSTEM;
				::SetFileAttributes(path, attributes);
			}
			fs->Delete(path);
			if(!fs->Copy(fnd->FilePath(j), path))
			{
				Error("Build error: can't copy %s to %s", fnd->FilePath(j), path.c_str());
			}
		}
	}
	fnd->Release();
}

//Удалить файлы
void ProjectBuilder::DeleteFiles(const string & basePath, const string & maskPath, bool isRecursive)
{
	//Полный нормализованый путь
	string fullPathWithMask;
	MakeFullPath(basePath, maskPath, fullPathWithMask);
	string fullPath;
	fullPath.GetFilePath(fullPathWithMask);
	//Путь и маска поиска
	string buffer;
	buffer.GetFileName(fullPathWithMask);
	//Собираем все файлы
	dword flags = isRecursive ? 0 : find_no_recursive;
	IFinder * fnd = fs->CreateFinder(fullPath, buffer, find_no_mirror_files | find_no_files_from_packs | flags, _FL_);
	if(!fnd->Count())
	{
		for(dword i = 0; i < maskPath.Len(); i++)
		{
			if(maskPath[i] == '*' || maskPath[i] == '?')
			{
				return;
			}
		}
		//Error("Build warning: file %s not found for delete ", fullPathWithMask.c_str());
		return;
	}
	//Процессируем найденые файлы
	for(dword j = 0; j < fnd->Count(); j++)
	{
		dword attributes = ::GetFileAttributes(fnd->FilePath(j));
		if(attributes != INVALID_FILE_ATTRIBUTES)
		{
			attributes &= ~FILE_ATTRIBUTE_READONLY;
			attributes &= ~FILE_ATTRIBUTE_SYSTEM;
			::SetFileAttributes(fnd->FilePath(j), attributes);
		}
		if(!fs->Delete(fnd->FilePath(j)))
		{
			Error("Build error: can't delete %s", fnd->FilePath(j));
		}
	}
	fnd->Release();
}

//Получить полный путь имея базовый путь и заданный
void ProjectBuilder::MakeFullPath(const string & basePath, const string & path, string & result)
{	
	//Смотрим абсолютный или относительный путь
	for(const char * p = path.GetBuffer(); *p; p++)
	{
		if(*p == ':')
		{
			result = path;
			fs->BuildPath(result, result);
			return;
		}
	}
	result = basePath;
	result += '\\';
	result += path;
	fs->BuildPath(result, result);
}

//Удалить пустые папки из билда
bool ProjectBuilder::DeleteEmptyFolders(const char * path)
{
	IFinder * fnd = fs->CreateFinder(path, "*.*", find_no_mirror_files | find_no_recursive | find_folders | find_no_files_from_packs, _FL_);
	if(fnd->Count() == 0)
	{
		fnd->Release();
		return true;
	}
	bool isDelete = true;
	for(dword i = 0; i < fnd->Count(); i++)
	{
		if(fnd->IsFolder(i))
		{
			if(DeleteEmptyFolders(fnd->FilePath(i)))
			{
				bool deleteResult = fs->Delete(fnd->FilePath(i));
				if(!deleteResult)
				{
					dword error = GetLastError();
					Error("Build error: can't delete folder %s", fnd->FilePath(i));
				}
			}else{
				isDelete = false;
			}
		}else{
			isDelete = false;
		}
	}
	fnd->Release();
	return isDelete;
}

//Проверить файлы в ресурсах
void ProjectBuilder::CheckFilesForUnique(const char * path)
{
	//Собираем файлы
	IFinder * fnd = fs->CreateFinder(path, "*.*", find_no_mirror_files | find_no_files_from_packs, _FL_);
	if(fnd->Count() == 0)
	{
		fnd->Release();
		return;
	}
	//Входная таблица
	const dword mask = 4095;
	array<long> entry(_FL_);
	entry.AddElements(mask + 1);
	for(dword i = 0; i <= mask; i++)
	{
		entry[i] = -1;
	}
	//Таблица строк
	array<char> strings(_FL_, 65536);
	strings.Reserve(fnd->Count()*32);
	strings.Add(0);
	//Таблица имён
	array<CheckFileName> namesTable(_FL_);
	namesTable.Reserve(fnd->Count());
	string compareMask;
	string compareMaskChk;
	//Перебираем все имена	
	for(dword i = 0; i < fnd->Count(); i++)
	{
		//Фильтруем имя
		const char * fpath = fnd->FilePath(i);
		for(long j = 0; j < compareFilters; j++)
		{
			compareMask = path;
			compareMask += compareFilters[j];
			fs->BuildPath(compareMask, compareMaskChk);
			if(string::IsFileMask(fpath, compareMaskChk.c_str()))
			{
				break;
			}
		}
		if(j < compareFilters)
		{
			//Пропускаем отфильтрованные файлы
			continue;
		}
		//Получаем имя и его параметры
		const char * name = fnd->Name(i);
		dword len = 0;
		dword hash = string::HashNoCase(name, len);
		long enterIndex = hash & mask;
		long lastIndex = entry[enterIndex];
		//Получаем цепочку имён
		if(lastIndex >= 0)
		{
			bool isError = false;
			while(true)
			{
				//Проверяем имя на совпадение
				CheckFileName & fn = namesTable[lastIndex];
				if(fn.hash == hash)
				{
					if(fn.len == len)
					{
						const char * n = strings.GetBuffer() + fn.nameIndex;
						if(string::IsEqual(n, name))
						{
							//Совпадение
							isError = true;
							break;
						}
					}
				}
				//Есть ли следующая запись в цепочки
				if(fn.next < 0)
				{
					break;
				}
				//Переходим на следующую запись
				lastIndex = fn.next;
			}
			if(isError)
			{
				Error("Check error: file name %s is repeat.", name);
				continue;
			}
		}
		//Добавляем новую запись
		long newRecord = namesTable.Add();
		CheckFileName & nfn = namesTable[newRecord];
		nfn.hash = hash;
		nfn.len = len;
		nfn.nameIndex = strings.Size();
		nfn.next = -1;
		//Сохраняем имя
		strings.AddElements(len + 1);
		char * dst = strings.GetBuffer() + nfn.nameIndex;
		for(dword i = 0; i <= len; i++)
		{
			*dst++ = *name++;
		}
		//Вносим в цепочку
		if(lastIndex >= 0)
		{
			namesTable[lastIndex].next = newRecord;
		}else{
			entry[enterIndex] = newRecord;
		}
	}
	//Проверяем размеры, сообщая о пустых файлах
	for(dword i = 0; i < fnd->Count(); i++)
	{
		const char * name = fnd->FilePath(i);
		IFile * file = fs->OpenFile(name, file_open_existing_for_read, _FL_);
		if(file)
		{
			dword size = file->Size();
			if(size == 0)
			{
				Error("Check warning: size of file %s is zero.", name);
			}
			file->Release();
		}else{
			Error("Check warning: can't open file %s for check size.", name);
		}
	}
	fnd->Release();
}

//Сконвертировать файлы из PC формата в Xbox формат
void ProjectBuilder::ConvertXBoxFiles()
{
	//Файлы геометрии
//	ConvertXBoxFiles("*.gmx", XboxConvert_gmx);
	//Файлы анимации
//	ConvertXBoxFiles("*.anx", XboxConvert_anx);
	//Файлы текстур
//	ConvertXBoxFiles("*.txx", XboxConvert_txx);
}

//Сконвертировать файлы из PC формата в Xbox формат заданного типа
void ProjectBuilder::ConvertXBoxFiles(const char * mask, void (* convert)(const char * from, const char * to))
{
	IFinder * fnd = fs->CreateFinder(workPath_Xbox.c_str(), mask, find_no_mirror_files | find_no_files_from_packs, _FL_);
	string destPath;
	for(dword i = 0; i < fnd->Count(); i++)
	{
		destPath = fnd->FilePath(i);
		destPath += "360";
		convert(fnd->FilePath(i), destPath.c_str());
		if(!fs->Delete(fnd->FilePath(i)))
		{
			Error("Build error: can't delete file %s", fnd->FilePath(i));
		}
	}
	fnd->Release();
}

//Вывести сообщение об ошибке
void _cdecl ProjectBuilder::Error(const char * error, ...)
{
	const char * errorFileName = "build_errors.txt";
	string errorPath = workPath_PC;
	errorPath += errorFileName;
	FILE * file = fopen(errorPath.c_str(), "ab");
	if(file)
	{
		vfprintf(file, error, (char *)(&error + 1));
		fprintf(file, "\r\n");
		fclose(file);
	}
	if(!workPath_Xbox.IsEmpty())
	{
		errorPath = workPath_Xbox;
		errorPath += errorFileName;
		FILE * file = fopen(errorPath.c_str(), "ab");
		if(file)
		{
			vfprintf(file, error, (char *)(&error + 1));
			fprintf(file, "\r\n");
			fclose(file);
		}
	}
	api->TraceData(error, &error + 1);
}

//=======================================================================================================
//ProjectBuilderService
//=======================================================================================================

ProjectBuilderService * ProjectBuilderService::service = null;
ProjectBuilder * ProjectBuilderService::builder = null;


ProjectBuilderService::ProjectBuilderService()
{
	Assert(!service);
	service = this;
	Assert(!builder);
	exeFunction = null;
	waitTimer = 0;
}

ProjectBuilderService::~ProjectBuilderService()
{
	if(builder)
	{
		delete builder;
	}
	Assert(!builder);
	Assert(service == this);
	service = null;
}

//Исполнить следующую функцию выждив несколько кадров
void ProjectBuilderService::Execute(void (ProjectBuilder:: * _exeFunction)())
{
	exeFunction = _exeFunction;
	waitTimer = 10;
}

//Инициализация
bool ProjectBuilderService::Init()
{
	//LogMissionObjects();
	//Считываем конфигурацию
	IFileService * fs = (IFileService *)api->GetService("FileService");
	Assert(fs);
	fs->SetDrainPath(null);
	IIniFile * ini = fs->SystemIni();
	if(!ini)
	{
		api->Trace("Build error: System ini file not opened!");
		return false;
	}
	if(!ini->IsSectionCreated("Build options"))
	{
		//Ничего делать ненадо, просто пропускаем всё
		return true;
	}
	const char * todo = ini->GetString("Build options", "todo", null);
	if(!todo)
	{
		api->Trace("Build error: parameter todo not asigned! (xml2mis | build)");
		return false;
	}
	bool isEnableXMLExport = false;
	if(todo[0] == 'e' || todo[0] == 'E')
	{
		isEnableXMLExport = true;
	}else
	if(todo[0] == 'b' || todo[0] == 'B')
	{
		isEnableXMLExport = false;
	}else{
		api->Trace("Build error: parameter todo has invalidate value %s! (can be: xml2mis | build)", todo);
		return false;
	}	
	//Создаём объект
	NEW ProjectBuilder();
	Assert(builder);	
	//Определяем что делать
	if(isEnableXMLExport)
	{
		//Для начала экспортируем xml в mis
		if(builder->StartExportXMLMissions())
		{
			api->SetEndFrameLevel(this, 0xffffffff);
			return true;
		}
	}else{
		if(builder->StartBuildProcess())
		{
			api->SetEndFrameLevel(this, 0xffffffff);
			return true;
		}
	}
	return false;
}

//Исполнение в конце кадра
void ProjectBuilderService::EndFrame(float dltTime)
{
	Assert(builder);
	if(!exeFunction)
	{
		//Закончили работу, выходим
		api->Exit();
		return;
	}
	if(waitTimer > 0)
	{
		waitTimer--;
		return;
	}
	void (ProjectBuilder:: * tmp)();
	tmp = exeFunction;
	exeFunction = null;
	(builder->*tmp)();	
}


//Вывести в лог форматированый список миссионых объектов и их параметров
void ProjectBuilderService::LogMissionObjects()
{
	array<string> regList(_FL_);
	api->GetRegistryObjectsList(regList);
	const char * mopId = MOP_ID;
	string moInitValue;
	for(long i = 0; i < regList; i++)
	{
		//Проверяем принадлежность объекта
		const char * objName = regList[i].c_str();
		for(long j = 0; objName[j]; j++)
		{
			if(mopId[j] != objName[j]) break;
		}
		if(mopId[j])
		{
			continue;
		}
		//Создаём объект дескриптора
		IMOParams * mop = (IMOParams *)api->CreateObject(objName);
		if(!mop)
		{
			continue;
		}
		if(mop->GetName() == null || mop->GetName()[0] == 0)
		{
			continue;
		}
		//Читаем и выводим атрибуты
		api->Trace("\n\nObject: ==%s==\n    Group: ===%s===", mop->GetName(), mop->GetGroup());
		if(mop->GetComment())
		{
			api->Trace("\n    Comment: \"%s\"", mop->GetComment());
		}
		api->Trace("\n    Attributes:\n");
		for(long j = 0; j < mop->GetNumParams(); j++)
		{
			const IMOParams::Param * p = mop->GetParam(j);
			OutputParameter("        ", p);
		}
		delete (RegObject *)mop;
	}	
}

//Вывести параметр
void ProjectBuilderService::OutputParameter(const char * shift, const IMOParams::Param * p)
{
	api->Trace("%s'''%s'''", shift, p->name);
	const char * type = "unknown";
	Vector v;
	Color c;
	char buffer[256];
	buffer[0] = 0;
	switch(p->type)
	{
	case IMOParams::t_bool:
		type = "[[bool]]";
		crt_snprintf(buffer, sizeof(buffer), "%s", ((const IMOParams::Bool *)p)->def ? "true" : "false");
		break;
	case IMOParams::t_long:
		type = "[[long]]";
		crt_snprintf(buffer, sizeof(buffer), "%i", ((const IMOParams::Long *)p)->def);
		break;
	case IMOParams::t_float:
		type = "[[float]]";
		crt_snprintf(buffer, sizeof(buffer), "%f", ((const IMOParams::Float *)p)->def);
		break;
	case IMOParams::t_string:
		type = "[[string]]";
		crt_snprintf(buffer, sizeof(buffer), "\"%s\"", ((const IMOParams::String *)p)->def);
		break;
	case IMOParams::t_locstring:
		type = "[[locstring]]";
		break;
	case IMOParams::t_position:
		type = "[[position]]";
		v = ((const IMOParams::Position *)p)->def;
		crt_snprintf(buffer, sizeof(buffer), "(x:%f, y:%f, z:%f)", v.x, v.y, v.z);
		break;
	case IMOParams::t_angles:
		type = "[[angles]]";
		v = ((const IMOParams::Angles *)p)->def;
		crt_snprintf(buffer, sizeof(buffer), "(x:%f, y:%f, z:%f)", v.x, v.y, v.z);
		break;
	case IMOParams::t_color:
		type = "[[color]]";
		c = ((const IMOParams::Colors *)p)->def;
		crt_snprintf(buffer, sizeof(buffer), "(r:%f, g:%f, b:%f, a:%f)", c.r, c.g, c.b, c.a);
		break;
	case IMOParams::t_array:
		type = null;
		api->Trace("%sType: [[array]]\n%s{", shift, shift);
		crt_snprintf(buffer, sizeof(buffer), "%s    ", shift);
		for(long i = 0; i < ((const IMOParams::Array *)p)->element; i++)
		{
			OutputParameter(buffer, ((const IMOParams::Array *)p)->element[i]);
		}		
		api->Trace("%s}", shift);
		break;
	case IMOParams::t_enum:
		type = null;
		api->Trace("%s  Type: [[enum]]", shift, shift);
		for(long i = 0; i < ((const IMOParams::Enum *)p)->element; i++)
		{
			api->Trace("%s  * %s", shift, ((const IMOParams::Enum *)p)->element[i]);
		}		
		break;
	case IMOParams::t_group:
		type = null;
		//api->Trace("%sType: group\n%s{", shift, shift);
		api->Trace("%s{", shift);
		crt_snprintf(buffer, sizeof(buffer), "%s    ", shift);
		for(long i = 0; i < ((const IMOParams::Group *)p)->element; i++)
		{
			OutputParameter(buffer, ((const IMOParams::Group *)p)->element[i]);
		}		
		api->Trace("%s}", shift);
		break;
	};
	if(type)
	{
		api->Trace("%s  Type: %s = %s", shift, type, buffer);
	}
}
#endif
#endif
