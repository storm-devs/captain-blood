//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// SndEditorMainWindow
//============================================================================================

#include "..\SndOptions.h"
#include "SndProject.h"
#include "ProjectWaveExporter.h"


//============================================================================================
//SoundBank
//============================================================================================

const char * ProjectSoundBank::section = "sound_bank_options";

ProjectSoundBank::ProjectSoundBank(UniqId & folderId) : ProjectObject(folderId, ProjectSoundBank::c_namelen, ".sbo")
{
}

//Получить путь по которому экспортировать банк
const char * ProjectSoundBank::GetExportPath()
{
	return exportPath.c_str();
}

//Получить папку звукового банка
const UniqId & ProjectSoundBank::GetSBFolderId()
{
	return GetFolder();
}

//Сбровить поля в начальное значение
void ProjectSoundBank::Reset(bool rebuildId)
{
	if(rebuildId)
	{
		UniqId id;
		id.Build();
		ReplaceId(id);
	}
	exportPath = "Resource\\Missions\\";
	exportPath += GetName().str;
	exportPath += "\\sounds\\";
	exportPath += GetName().str;
}

//Сохранить объект
ErrorId ProjectSoundBank::OnSaveObject(const char * defaultFilePath)
{
	IEditableIniFile * file = project->FileOpenIni(defaultFilePath, false, false, _FL_);
	if(!file)
	{
		return options->ErrorOut(null, true, "Error sound bank options save: Can't open file \"%s\"", defaultFilePath);
	}
	file->SetString(section, "id", GetId().ToString());
	file->SetString(section, "export_path", exportPath.c_str());
	project->FileCloseIni(file);
	return ErrorId::ok;
}

//Первоначальная загрузка при старте редактора
ErrorId ProjectSoundBank::OnLoadObject(const char * defaultFilePath)
{
	IEditableIniFile * file = project->FileOpenIni(defaultFilePath, false, true, _FL_);
	if(!file)
	{
		return options->ErrorOut(null, true, "Error sound bank options load: Can't open file \"%s\"", defaultFilePath);
	}
	const char * s = file->GetString(section, "id", null);
	if(!s)
	{
		project->FileCloseIni(file);
		return options->ErrorOut(null, true, "Error sound bank options load: Can't read key \"%s:id\" from file \"%s\"", section, defaultFilePath);
	}
	UniqId id;
	if(!id.FromString(s))
	{
		project->FileCloseIni(file);
		return options->ErrorOut(null, true, "Error sound bank options load: Can't restore id (%s). Key \"%s:id\". File \"%s\"", s, section, defaultFilePath);
	}
	ReplaceId(id);
	//Параметры банка
	s = file->GetString(section, "export_path", null);
	if(!s)
	{
		project->FileCloseIni(file);
		return options->ErrorOut(null, true, "Error sound bank options load: Can't read key \"%s:export_path\" from file \"%s\"", section, defaultFilePath);
	}
	exportPath = s;
	project->FileCloseIni(file);
	return ErrorId::ok;
}

//============================================================================================
//SoundBaseParamsData
//============================================================================================

ProjectSoundBaseParams::ProjectSoundBaseParams(UniqId & folderId) : ProjectObject(folderId, ProjectSoundBaseParams::c_namelen, ".txt")
{
}

//Сохранить объект
ErrorId ProjectSoundBaseParams::OnSaveObject(const char * defaultFilePath)
{
	IEditableIniFile * file = project->FileOpenIni(defaultFilePath, false, false, _FL_);
	if(!file)
	{
		return options->ErrorOut(null, true, "Error sound setup save: Can't open file \"%s\"", defaultFilePath);
	}
	file->SetString(null, "id", GetId().ToString());
	params.SaveToIni(file);
	project->FileCloseIni(file);
	return ErrorId::ok;
}

//Первоначальная загрузка при старте редактора
ErrorId ProjectSoundBaseParams::OnLoadObject(const char * defaultFilePath)
{
	IEditableIniFile * file = project->FileOpenIni(defaultFilePath, false, true, _FL_);
	if(!file)
	{
		return options->ErrorOut(null, true, "Error sound setup load: Can't open file \"%s\"", defaultFilePath);
	}
	const char * s = file->GetString(null, "id", null);
	if(!s)
	{
		project->FileCloseIni(file);
		return options->ErrorOut(null, true, "Error sound setup load: Can't read key \"id\" from file \"%s\"", defaultFilePath);
	}
	UniqId id;
	if(!id.FromString(s))
	{
		project->FileCloseIni(file);
		return options->ErrorOut(null, true, "Error sound setup load: Can't restore id (%s). Key \"id\". File \"%s\"", s, defaultFilePath);
	}
	ReplaceId(id);
	params.LoadFromIni(file);
	project->FileCloseIni(file);
	return ErrorId::ok;
}

//============================================================================================
//AttGraphData
//============================================================================================

ProjectSoundAttGraph::ProjectSoundAttGraph(UniqId & folderId) : ProjectObject(folderId, ProjectSoundAttGraph::c_namelen, ".txt")
{
}

//Сохранить объект
ErrorId ProjectSoundAttGraph::OnSaveObject(const char * defaultFilePath)
{
	IEditableIniFile * file = project->FileOpenIni(defaultFilePath, false, false, _FL_);
	if(!file)
	{
		return options->ErrorOut(null, true, "Error sound attenuation save: Can't open file \"%s\"", defaultFilePath);
	}
	file->SetString(null, "id", GetId().ToString());
	attGraph.SaveToIni(file);
	project->FileCloseIni(file);
	return ErrorId::ok;
}

//Первоначальная загрузка при старте редактора
ErrorId ProjectSoundAttGraph::OnLoadObject(const char * defaultFilePath)
{
	IEditableIniFile * file = project->FileOpenIni(defaultFilePath, false, true, _FL_);
	if(!file)
	{
		return options->ErrorOut(null, true, "Error sound attenuation load: Can't open file \"%s\"", defaultFilePath);
	}
	const char * s = file->GetString(null, "id", null);
	if(!s)
	{
		project->FileCloseIni(file);
		return options->ErrorOut(null, true, "Error sound attenuation load: Can't read key \"id\" from file \"%s\"", defaultFilePath);
	}
	UniqId id;
	if(!id.FromString(s))
	{
		project->FileCloseIni(file);
		return options->ErrorOut(null, true, "Error sound attenuation load: Can't restore id (%s). Key \"id\". File \"%s\"", s, defaultFilePath);
	}
	ReplaceId(id);
	attGraph.LoadFromIni(file);
	project->FileCloseIni(file);
	return ErrorId::ok;
}

//============================================================================================
//PrjFolder
//============================================================================================


SndProject::PrjFolder::PrjFolder() : child(_FL_),
									 objects(_FL_)
{
	parent = null;
	del = null;
	noEditable = false;
}

//============================================================================================
//SndProject
//============================================================================================

SndProject::SndProject() :	waves(_FL_, 1024),
							sounds(_FL_, 1024),
							soundBaseParams(_FL_, 256),
							attGraphs(_FL_, 256),
							folders(_FL_, 1024),
							soundBanks(_FL_, 16),
							animations(_FL_, 256),
							waitForSave(_FL_, 1024),
							folderChilds(_FL_, 256),
							openBinFiles(_FL_, 256),
							openIniFiles(_FL_, 256),
							regTable(_FL_, 4096)
{
	openBinFiles.Reserve(4096);
	openIniFiles.Reserve(4096);
	ProjectWaveExporter::StartExporter();
	Assert(project == null);
	project = this;
	waitToSave = false;
	updateTimer = 0.0f;
	projectTime = 0.0;
	waveUpdater = 0;
	waveExportPointer = 0;
	soundUpdater = 0;
	soundUpdaterPauseTimer = 15.0f;
	firstRegFree = -1;
	Assert(((ARRSIZE(entryRegTable) - 1) & ARRSIZE(entryRegTable)) == 0);
	for(dword i = 0; i < ARRSIZE(entryRegTable); i++) entryRegTable[i] = -1;
	regChainLengMax = 0;
	folderUp.Build();
	//Загрузить проект
	LoadProject();
}

SndProject::~SndProject()
{	
	AssertCoreThread
	//Останавливаем экспортёр
	ProjectWaveExporter::StopExporter();
	//Сохраняем изменения
	FlushData();
	//Удаляем объекты
	sounds.DelAllWithPointers();
	waves.DelAllWithPointers();
	soundBaseParams.DelAllWithPointers();
	attGraphs.DelAllWithPointers();
	soundBanks.DelAllWithPointers();
	animations.DelAllWithPointers();
	folders.DelAllWithPointers();
	//Закрываем бинарные файлы
	binFilesSyncro.Enter();
	FilesRelease(openBinFiles);
	binFilesSyncro.Leave();
	//Закрываем ini файлы
	FilesRelease(openIniFiles);
	//На этот момент на сохранение не должно быть желающих
	Assert(waitForSave.Size() == 0);
	//Очищаем поинтер на проект	
	Assert(project == this);
	project = null;
}



//--------------------------------------------------------------------------------------
//Глобальные объекты поиска
//--------------------------------------------------------------------------------------


//Зарегистрировать объект
bool SndProject::ObjectRegister(const UniqId & id, ObjectTypes type, void * ptr)
{
	if(type == pot_any_object)
	{
		return false;
	}
	dword hash = id.data[0] ^ id.data[1] ^ id.data[2] ^ id.data[3];
	dword mask = ARRSIZE(entryRegTable) - 1;
	long chainTail = entryRegTable[hash & mask];
	if(chainTail >= 0)
	{
		//Проверяем на повторения и заодно ищем конец хвоста
		dword chainLen = 1;
		long key = chainTail;
		while(true)
		{
			RegRecord & rr = regTable[key];
			if(rr.hash == hash)
			{
				if(rr.id == id)
				{
					//Объект с таким идентификатором уже есть
					if(rr.type == type && rr.objectPtr == ptr)
					{
						//Какогото фига повторная регистрация одного и того же
						return true;
					}
					//Попытка внести новый объект с повторным идентификатором
					return false;
				}
			}
			if(rr.next >= 0)
			{
				key = rr.next;
				chainLen++;
			}else{
				chainTail = key;
				if(chainLen > regChainLengMax)
				{
					regChainLengMax = chainLen;
				}
				break;
			}
		}
	}
	//Добавляем запись
	long index = -1;
	if(firstRegFree < 0)
	{
		index = regTable.Add();
	}else{
		index = firstRegFree;
		firstRegFree = regTable[firstRegFree].next;
	}
	//Заполняем содержимое
	RegRecord & nrr = regTable[index];
	nrr.hash = hash;
	nrr.type = type;
	nrr.id = id;
	nrr.objectPtr = ptr;
	nrr.next = -1;
	if(chainTail >= 0)
	{
		regTable[chainTail].next = index;
	}else{
		entryRegTable[hash & mask] = index;
	}
	return true;
}

//Удалить объект
bool SndProject::ObjectRemove(const UniqId & id, ObjectTypes type)
{
	dword hash = id.data[0] ^ id.data[1] ^ id.data[2] ^ id.data[3];
	dword mask = ARRSIZE(entryRegTable) - 1;
	long * chainPtr = &entryRegTable[hash & mask];
	while(*chainPtr >= 0)
	{
		RegRecord & rr = regTable[*chainPtr];
		if(rr.hash == hash)
		{
			if(rr.id == id)
			{
				long nextRegFree = firstRegFree;
				firstRegFree = *chainPtr;
				*chainPtr = rr.next;
				chainPtr = null;
				rr.hash = 0;
				rr.type = 0;
				rr.id.Reset();
				rr.objectPtr = 0;
				rr.next = nextRegFree;
				return true;
			}
		}
		chainPtr = &rr.next;
	}
	//Объект не найден
	return false;
}

//Найти объект по идентификатору
void * SndProject::ObjectFind(const UniqId & id, ObjectTypes type)
{
	dword hash = id.data[0] ^ id.data[1] ^ id.data[2] ^ id.data[3];
	dword mask = ARRSIZE(entryRegTable) - 1;
	long key = entryRegTable[hash & mask];
	while(key >= 0)
	{
		RegRecord & rr = regTable[key];
		if(rr.hash == hash)
		{			
			if(rr.id == id)
			{
				if(rr.type == type || type == pot_any_object)
				{
					return rr.objectPtr;
				}
			}
		}
		key = rr.next;
	}
	return null;
}

//--------------------------------------------------------------------------------------
//Работа с папками
//--------------------------------------------------------------------------------------

//Добавить папку
bool SndProject::FolderAdd(const UniqId & parentFolderId, const char * name, UniqId * id)
{
	if(id)
	{
		id->Reset();
	}
	if(string::IsEmpty(name))
	{
		//С пустым именем не работаем
		return false;
	}
	//Проверяем, чтобы у родительской папки небыло детей с таким же именем
	PrjFolder * parent = (PrjFolder *)ObjectFind(parentFolderId, pot_folder);
	if(!parent)
	{
		//Ненайден родитель
		return false;
	}
	PrjFolder ** childFolders = parent->child.GetBuffer();
	dword childsCount = parent->child.Size();
	ExtName ename;
	ename.Init(name);
	for(dword i = 0; i < childsCount; i++)
	{
		ExtName & fn = childFolders[i]->name;
		if(fn.hash == ename.hash && fn.len == ename.len)
		{
			if(string::IsEqual(fn.str, ename.str))
			{
				//Есть такая уже папка, делаем вид что создали
				if(id)
				{
					*id = childFolders[i]->id;
				}
				return true;
			}
		}
	}
	//Получаем полный путь новой папки	
	string fullPath = parent->fullPath;
	fullPath += name;
	fullPath += "\\";
	//Пробуем создать папку на диске
	if(!options->fileService->CreateFolder(fullPath.c_str()))
	{
		//Неудалось создать физическую папку на диске
		return false;
	}
	//Создаём запись папки в проекте
	PrjFolder * fld = NEW PrjFolder();
	fld->id.Build();
	fld->name.SetString(name);
	fld->parent = parent;
	if(parent)
	{
		parent->child.Add(fld);
	}
	fld->fullPath = fullPath;
	folders.Add(fld);
	//Регистрируем папку в реестре поиска
	ObjectRegister(fld->id, pot_folder, fld);
	//Возвращаем положительный результат
	if(id) *id = fld->id;
	return true;
}

//Удалить папку
ErrorId SndProject::FolderDelete(const UniqId & folderId)
{
	//Проверяем, чтобы у родительской папки небыло детей с таким же именем
	PrjFolder * folder = (PrjFolder *)ObjectFind(folderId, pot_folder);
	if(!folder)
	{
		//Папка не найдена
		return options->ErrorOut(null, true, "Folder delete error: Folder not found! Invalidate folderId (%s)", folderId.ToString());
	}
	//Удаляем дочернии папки
	while(folder->child.Size())
	{
		ErrorId errId = FolderDelete(folder->child[0]->id);
		if(errId.IsError())
		{
			//Неполучилось удалить дочернюю папку
			return errId;
		}
	}
	//Занимаемся удаление папки
	if(folder->noEditable)
	{
		//Эту папку нельзя удалять
		return options->ErrorOut(null, true, "Folder delete error: This folder disable for delete (%s)", folder->name.str);
	}
	//Получаем функцию удаления от себя или родителя
	FolderObjectDelete del;
	for(PrjFolder * f = folder; f; f = f->parent)
	{
		if(f->del)
		{
			del = f->del;
			break;
		}
	}
	//Удаляем содержимое папки, если есть такая опция
	if(del)
	{
		for(dword i = 0; i < folder->objects.Size(); i++)
		{
			ErrorId errId = (project->*del)(folder->objects[i]);
			if(errId.IsError())
			{
				//Ошибка удаления объекта
				return errId;
			}
		}
	}
	//Отписываемся у родительской папки
	if(folder->parent)
	{
		folder->parent->child.Del(folder);
	}
	//Удаляем саму папку
	if(!options->fileService->Delete(folder->fullPath.c_str()))
	{
		//Неудалось удалить папку с диска
		return options->ErrorOut(null, true, "Folder delete error: FIO fault for delete folder from disk (%s)", folder->fullPath.c_str());
	}
	Verify(ObjectRemove(folder->id, pot_folder));	
	folders.Del(folder);
	delete folder;
	return ErrorId::ok;
}


//Переименовать папку
ErrorId SndProject::FolderRename(const UniqId & folderId, const char * name)
{
	if(string::IsEmpty(name))
	{
		//Нельзя переименовать в пустое имя
		return options->ErrorOut(null, true, "Folder rename error: Can't rename folder to empty name");
	}
	//Папка
	PrjFolder * folder = (PrjFolder *)ObjectFind(folderId, pot_folder);
	if(!folder)
	{
		//Папка не найдена
		return options->ErrorOut(null, true, "Folder rename error: Folder not found! Invalidate folderId (%s)", folderId.ToString());
	}
	if(folder->noEditable)
	{
		//Эту папку нельзя переименовывать
		return options->ErrorOut(null, true, "Folder rename error: This folder disable for rename (%s)", folder->name.str);
	}
	//Редактировать можно только дочернии папки
	Assert(folder->parent);
	//Если имя не меняеться то и ничего не делаем
	if(string::IsEqual(folder->name.str, name))
	{
		return ErrorId::ok;
	}
	//Пытаемся переименовать папку
	string newFullPath = folder->parent->fullPath;
	newFullPath += name;
	newFullPath += "\\";
	if(!options->fileService->Rename(folder->fullPath.c_str(), newFullPath.c_str()))
	{
		//Переименовать папку неудалось
		return options->ErrorOut(null, true, "Folder rename error: FIO fault when try rename folder from \"%s\" to \"%s\"", folder->name.str, name);
	}
	folder->name.SetString(name);
	return ErrorId::ok;
}

//Получить родительскую папку
bool SndProject::FolderGetParent(const UniqId & folderId, UniqId & parentFolderId)
{
	parentFolderId.Reset();
	PrjFolder * folder = (PrjFolder *)ObjectFind(folderId, pot_folder);
	if(!folder)
	{
		return false;
	}
	if(folder->parent)
	{
		parentFolderId = folder->parent->id;
	}
	return true;
}

//Получить дочернии папки
const array<UniqId> & SndProject::FolderGetChild(const UniqId & folderId)
{
	folderChilds.Empty();
	PrjFolder * folder = (PrjFolder *)ObjectFind(folderId, pot_folder);
	if(!folder)
	{
		return folderChilds;
	}
	folderChilds.AddElements(folder->child.Size());
	for(dword i = 0; i < folder->child.Size(); i++)
	{
		folderChilds[i] = folder->child[i]->id;
	}
	return folderChilds;
}

//Получить название папки
const ExtName * SndProject::FolderGetName(const UniqId & folderId)
{
	PrjFolder * folder = (PrjFolder *)ObjectFind(folderId, pot_folder);
	if(!folder)
	{
		return null;
	}
	return &folder->name;
}


//Являеться ли папка дочерней для заданной (или текущей)
bool SndProject::FolderIsChild(const UniqId & parentFolderId, const UniqId & folderId)
{
	PrjFolder * folder = (PrjFolder *)ObjectFind(folderId, pot_folder);
	for(; folder != null; folder = folder->parent)
	{
		if(folder->id == parentFolderId)
		{
			return true;
		}
	}
	return false;
}

//Получить путь папки
const char * SndProject::FolderGetPath(const UniqId & folderId, const UniqId * stopFolderId)
{
	PrjFolder * folder = (PrjFolder *)ObjectFind(folderId, pot_folder);
	if(!folder)
	{
		return null;
	}
	//Если нужен полный путь, то ничего не вычисляем	
	PrjFolder * stopFolder = (stopFolderId != null) ? (PrjFolder *)ObjectFind(*stopFolderId, pot_folder) : null;
	if(!stopFolder)
	{
		return folder->fullPath.c_str();
	}
	if(stopFolder == folder) return "";
	//Пробегаем по папкам, собирая относительный путь
	folderPathBuffer.Empty();
	while(true)
	{
		folderPathBuffer.Insert(0, folder->name.data);
		folder = folder->parent;
		if(!folder || stopFolder == folder)
		{
			//Больше нет папок
			return folderPathBuffer.c_str();
		}
		folderPathBuffer.Insert(0, "\\");
	}
	return null;
}

//Проверить на уникальность имени среди детей данной папки
bool SndProject::FolderCheckUniqeName(const UniqId & folderId, const char * name, const UniqId * skipFolderId)
{
	//Пустые имена пропускаем
	if(string::IsEmpty(name))
	{
		return false;
	}
	//Получаем указатель на папку
	PrjFolder * folder = (PrjFolder *)ObjectFind(folderId, pot_folder);
	if(!folder)
	{
		return false;
	}
	//Ищим имя среди детей
	PrjFolder ** folders = folder->child.GetBuffer();
	dword count = folder->child.Size();
	dword nlen = 0;
	dword nhash = string::HashNoCase(name, nlen);
	for(dword i = 0; i < count; i++)
	{
		if(skipFolderId)
		{
			if(folders[i]->id == *skipFolderId)
			{
				continue;
			}
		}
		ExtNameStr & fn = folders[i]->name;
		if(fn.hash == nhash && fn.len == nlen)
		{
			if(string::IsEqual(fn.str, name))
			{
				return false;
			}
		}
	}
	//Имя уникально и корректно
	return true;
}

//Добавить объект в папку
bool SndProject::FolderObjectAdd(const UniqId & folderId, const UniqId & id)
{
	//Пустой идентификатор не добавляем
	if(!id.IsValidate())
	{
		return false;
	}
	//Папка
	PrjFolder * folder = (PrjFolder *)ObjectFind(folderId, pot_folder);
	if(!folder)
	{
		//Папка не найдена
		return false;
	}
	folder->objects.Del(id);
	folder->objects.Add(id);
	return true;
}

//Удалить объект из папку
bool SndProject::FolderObjectDel(const UniqId & folderId, const UniqId & id)
{
	//Папка
	PrjFolder * folder = (PrjFolder *)ObjectFind(folderId, pot_folder);
	if(!folder)
	{
		//Папка не найдена
		return false;
	}
	folder->objects.Del(id);
	return true;
}

//Список объектов, которые лежат в папке
const array<UniqId> & SndProject::FolderObjects(const UniqId & folderId)
{
	//Папка
	PrjFolder * folder = (PrjFolder *)ObjectFind(folderId, pot_folder);
	if(!folder)
	{
		//Папка не найдена, возвратим пустой массив
		folderChilds.Empty();
		return folderChilds;
	}
	return folder->objects;
}

//--------------------------------------------------------------------------------------
//Работа с волнами
//--------------------------------------------------------------------------------------

//Получить волну
ProjectWave * SndProject::WaveGet(const UniqId & id)
{
	ProjectWave * wave = (ProjectWave *)ObjectFind(id, pot_wave);
	return wave;
}

//Получить список волн (пользоваться им можно только сразу после получения)
const array<ProjectWave *> & SndProject::WaveArray()
{
	return waves;
}

//Импортировать волну в проект (без проверки совпадения имён)
ErrorId SndProject::WaveImport(const UniqId & folderId, const char * path, const char * name, const UniqId & replaceId, UniqId * id)
{
	ErrorId errorId;
	if(id) id->Reset();
	//Проверим что папка являеться потомком папки с волнами
	Assert(FolderIsChild(folderWaves, folderId));
	//Волна, которую надо перезаписать
	ProjectWave * replaceWave = null;
	if(replaceId.IsValidate())
	{
		replaceWave = WaveGet(replaceId);
	}
	//Проверим имя
	if(!replaceWave && !WaveCheckUniqeName(folderId, name, null))
	{
		return options->ErrorOut(null, true, "Wave import: wave name not unique for project!");
	}
	//Создаём волну
	ProjectWave * w = NEW ProjectWave(folderId);
	//Бэкапим старую волну перед импортом
	string backupOldName;
	string deleteBackupFilePath;
	if(replaceWave)
	{
		backupOldName = replaceWave->GetName().str;
		string backupNewName = backupOldName;
		backupNewName += "_replace_backup";		
		replaceWave->BuildFilePath(deleteBackupFilePath, backupNewName.c_str());
		options->fileService->Delete(deleteBackupFilePath.c_str());
		if(replaceWave->Rename(backupNewName.c_str()) != ProjectObject::ec_ok)
		{
			return options->ErrorOut(null, true, "Wave import: error backup current wave. Stop import operation...");
		}
	}
	//Создаём пустой файл для новой волны
	if(w->Init(name) != ProjectObject::ec_ok)
	{
		string wpath;
		w->BuildFilePath(wpath);
		return options->ErrorOut(null, true, "Wave import: Can't create file \"%s\"", wpath.c_str());
	}
	//Импортируем волну
	errorId = w->Import(path, replaceWave);	
	if(errorId.IsOk())
	{
		//Регистрируем волну в проекте
		if(replaceWave)
		{
			//Удалим перезаписываемую волну из списков
			Verify(ObjectRemove(replaceWave->GetId(), pot_wave))
			waves.Del(replaceWave);
			UniqId id = replaceWave->GetId();			
			delete replaceWave;
			replaceWave = null;
			w->ReplaceId(id);
			w->SetToSave();
			//Сообщаем звукам, что волна удаляеться
			for(dword i = 0; i < sounds.Size(); i++)
			{
				ProjectSound * ps = sounds[i];
				if(!ps) continue;
				ps->OnReplaceWave(id);
			}
		}
		//Добавляем новую волну
		waves.Add(w);
		Verify(ObjectRegister(w->GetId(), pot_wave, w));
		if(id) *id = w->GetId();
		options->fileService->Delete(deleteBackupFilePath.c_str());
	}else{
		//Импорт неудался
		w->Delete();
		delete w;
		//Востанавливаем перезаписываемую волну
		if(replaceWave)
		{
			//Пытаемся вернуть старое имя
			replaceWave->Rename(backupOldName.c_str());
		}
	}
	return errorId;
}

//Удалить волну
ErrorId SndProject::WaveDelete(const UniqId & waveId)
{
	ProjectWave * wave = WaveGet(waveId);
	if(!wave)
	{
		return options->ErrorOut(null, true, "Wave delete: wave with id = %s not found...", waveId.ToString());
	}
	//Надо остановить все проигрывания
	AnimationPrepareForSounds(UniqId());
	options->WavePreviewStop();
	//Сообщаем звукам, что волна удаляеться
	for(dword i = 0; i < sounds.Size(); i++)
	{
		ProjectSound * ps = sounds[i];
		if(!ps) continue;
		ps->OnDeleteWave(waveId);
	}
	//Удаляем
	Verify(ObjectRemove(waveId, pot_wave));
	waves.Del(wave);
	if(wave->Delete() != ProjectObject::ec_ok)
	{
		string tmp;
		wave->BuildFilePath(tmp);
		return options->ErrorOut(null, true, "Wave delete: error delete wave file \"%s\"", tmp.c_str());
	}
	delete wave;
	return ErrorId::ok;
}

//Переименовать волну
bool SndProject::WaveRename(const UniqId & waveId, const char * name)
{
	//Получаем указатель
	ProjectWave * wave = WaveGet(waveId);
	if(!wave)
	{
		return false;
	}
	//Пытаемся переименовать
	return (wave->Rename(name) == ProjectObject::ec_ok);
}

//Проверить имя на уникальность
bool SndProject::WaveCheckUniqeName(const UniqId & folderId, const char * name, const UniqId * skipWaveId)
{
	return NameCheckForUnique<ProjectWave>(waves, ProjectWave::c_namelen, folderId, name, skipWaveId);
}


//--------------------------------------------------------------------------------------
//Работа со звуком
//--------------------------------------------------------------------------------------

//Получить звук
ProjectSound * SndProject::SoundGet(const UniqId & id)
{
	ProjectSound * sound = (ProjectSound *)ObjectFind(id, pot_sound);
	return sound;
}

//Найти звук по имени
ProjectSound * SndProject::SoundFind(const char * name)
{
	if(string::IsEmpty(name)) return null;
	dword len = 0;
	dword hash = string::HashNoCase(name, len);
	for(dword i = 0; i < sounds.Size(); i++)
	{
		ProjectSound * sound = sounds[i];
		const ExtName & n = sound->GetName();
		if(n.hash == hash && n.len == len)
		{
			if(string::IsEqual(n.str, name))
			{
				return sound;
			}
		}		
	}
	return null;
}

//Получить список звуков (пользоваться им можно только сразу после получения)
const array<ProjectSound *> & SndProject::SoundArray()
{
	return sounds;
}


//Добавить звук
bool SndProject::SoundAdd(const UniqId & folderId, const char * name, UniqId * id)
{
	if(id) id->Reset();
	//Проверим что папка являеться потомком звуковой папки
	Assert(FolderIsChild(folderSounds, folderId));
	//Проверим имя
	if(!SoundCheckUniqeName(folderId, name, null))
	{
		return false;
	}
	//Создаём звук
	ProjectSound * sound = NEW ProjectSound(folderId);
	if(sound->Init(name) != ProjectObject::ec_ok)
	{
		return false;
	}
	sounds.Add(sound);
	Verify(ObjectRegister(sound->GetId(), pot_sound, sound));
	if(id) *id = sound->GetId();
	sound->SetToSave();
	return true;	
}

//Удалить файл
ErrorId SndProject::SoundDelete(const UniqId & soundId)
{	
	//Указатель на звук
	ProjectSound * sound = SoundGet(soundId);
	if(!sound)
	{
		return options->ErrorOut(null, true, "Sound delete: object not found");
	}
	//Удаляем объект с диска
	if(sound->Delete() != ProjectObject::ec_ok)
	{
		return options->ErrorOut(null, true, "Sound delete: sound file not delete");
	}
	//Удаляем из списков
	sounds.Del(sound);
	Verify(ObjectRemove(sound->GetId(), pot_sound));
	//Удаляем объект из памяти
	delete sound;
	return ErrorId::ok;
}

//Переименовать звук
bool SndProject::SoundRename(const UniqId & soundId, const char * name)
{
	//Получаем указатель
	ProjectSound * sound = SoundGet(soundId);
	if(!sound)
	{
		return false;
	}
	//Пытаемся переименовать
	return (sound->Rename(name) == ProjectObject::ec_ok);
}


//Проверить имя на уникальность
bool SndProject::SoundCheckUniqeName(const UniqId & folderId, const char * name, const UniqId * skipSoundId)
{
	//Ищим банк, в котором находиться звук
	PrjFolder * sbankFolder = (PrjFolder *)ObjectFind(folderId, pot_folder);
	if(!sbankFolder) return false;
	while(true)
	{
		PrjFolder * parentFolder = sbankFolder->parent;
		if(!parentFolder) return false;
		if(parentFolder->id == folderSounds) break;
		sbankFolder = parentFolder;
	}
	//Проверяем на уникальность
	return NameCheckForUnique<ProjectSound>(sounds, ProjectSound::c_namelen, folderId, name, skipSoundId, &sbankFolder->id);
}


//Получить доступ к базовым параметрам звука
ProjectSoundBaseParams * SndProject::SndBaseParamsGet(const UniqId & id)
{
	ProjectSoundBaseParams * bpd = (ProjectSoundBaseParams *)ObjectFind(id, pot_baseparams);
	return bpd;
}

//Получить массив глобальных параметров (пользоваться им можно только сразу после получения)
const array<ProjectSoundBaseParams *> & SndProject::SndBaseParamsArray()
{
	return soundBaseParams;
}

//Добавить глобальные параметры звука
bool SndProject::SndBaseParamsAdd(const char * name, UniqId * id)
{
	if(id) id->Reset();
	if(string::IsEmpty(name)) return false;
	ProjectSoundBaseParams * prms = NEW ProjectSoundBaseParams(folderSoundParams);
	if(prms->Init(name) != ProjectObject::ec_ok)
	{
		delete prms;
		return false;
	}
	soundBaseParams.Add(prms);
	Verify(ObjectRegister(prms->GetId(), pot_baseparams, prms));	
	if(id) *id = prms->GetId();
	prms->SetToSave();
	return true;
}


//Удалить глобальные параметры звука
bool SndProject::SndBaseParamsDelete(const UniqId & id)
{
	//Получаем указатель
	ProjectSoundBaseParams * sbp = SndBaseParamsGet(id);
	if(!sbp)
	{
		return false;
	}
	if(sbp->Delete() != ProjectObject::ec_ok)
	{
		return false;
	}
	//Удаляем из списков
	Verify(ObjectRemove(sbp->GetId(), pot_baseparams));
	soundBaseParams.Del(sbp);
	//Сообщаем звукам, что удаляем данный объект
	for(long i = 0; i < sounds; i++)
	{
		ProjectSound * ps = sounds[i];
		ps->OnDeleteSoundParams(sbp->GetId());
	}
	//Удаляем объект	
	delete sbp;
	sbp = null;
	return true;
}

//Переименовать глобальные параметры звука
bool SndProject::SndBaseParamsRename(const UniqId & id, const char * name)
{
	//Получаем указатель
	ProjectSoundBaseParams * sbp = SndBaseParamsGet(id);
	if(!sbp)
	{
		return false;
	}
	//Пытаемся переименовать
	return (sbp->Rename(name) == ProjectObject::ec_ok);
}

//Проверить имя на уникальность
bool SndProject::SndBaseParamsCheckUniqeName(const char * name, const UniqId * skipParamsId)
{
	return NameCheckForUnique<ProjectSoundBaseParams>(soundBaseParams, ProjectSoundBaseParams::c_namelen, UniqId::zeroId, name, skipParamsId);
}

//Получить доступ к графику затухания 
ProjectSoundAttGraph * SndProject::AttGraphGet(const UniqId & id)
{
	ProjectSoundAttGraph * agd = (ProjectSoundAttGraph *)ObjectFind(id, pot_attenuation);
	return agd;
}

//Получить массив глобальных графиков
const array<ProjectSoundAttGraph *> & SndProject::AttGraphArray()
{
	return attGraphs;
}

//Добавить график затухания
bool SndProject::AttGraphAdd(const char * name, UniqId * id)
{
	if(id) id->Reset();
	if(string::IsEmpty(name)) return false;
	ProjectSoundAttGraph * agd = NEW ProjectSoundAttGraph(folderAttenuations);
	if(agd->Init(name) != ProjectObject::ec_ok)
	{
		delete agd;
		return false;
	}
	attGraphs.Add(agd);
	Verify(ObjectRegister(agd->GetId(), pot_attenuation, agd));	
	if(id) *id = agd->GetId();
	agd->SetToSave();
	return true;
}

//Удалить график затухания
bool SndProject::AttGraphDelete(const UniqId & id)
{	
	//Получаем указатель
	ProjectSoundAttGraph * agd = AttGraphGet(id);
	if(!agd)
	{
		return false;
	}
	if(agd->Delete() != ProjectObject::ec_ok)
	{
		return false;
	}
	//Удаляем из списков
	Verify(ObjectRemove(agd->GetId(), pot_attenuation));
	attGraphs.Del(agd);
	//Сообщаем звукам, что удаляем данный объект
	for(long i = 0; i < sounds; i++)
	{
		ProjectSound * ps = sounds[i];
		ps->OnDeleteSoundParams(agd->GetId());
	}
	//Удаляем объект	
	delete agd;
	agd = null;
	return true;
}

//Переименовать график затухания
bool SndProject::AttGraphRename(const UniqId & id, const char * name)
{
	//Получаем указатель
	ProjectSoundAttGraph * agd = AttGraphGet(id);
	if(!agd)
	{
		return false;
	}
	//Пытаемся переименовать
	return (agd->Rename(name) == ProjectObject::ec_ok);
}

//Проверить имя на уникальность
bool SndProject::AttGraphCheckUniqeName(const char * name, const UniqId * skipAttId)
{
	return NameCheckForUnique<ProjectSoundAttGraph>(attGraphs, ProjectSoundAttGraph::c_namelen, UniqId::zeroId, name, skipAttId);
}

//--------------------------------------------------------------------------------------
//Работа с банками
//--------------------------------------------------------------------------------------

//Получить звуковой банк
ProjectSoundBank * SndProject::SoundBankGet(const UniqId & id)
{
	ProjectSoundBank * sb = (ProjectSoundBank *)ObjectFind(id, pot_soundbank);
	return sb;
}

//Получить звуковой банк для папки
ProjectSoundBank * SndProject::SoundBankGetByFolder(const UniqId & folderId)
{
	ProjectSoundBank ** banks = soundBanks.GetBuffer();
	dword count = soundBanks.Size();
	for(dword i = 0; i < count; i++)
	{
		if(banks[i]->GetSBFolderId() == folderId)
		{
			return banks[i];
		}
	}
	return null;
}

//Получить список звуковых банков (пользоваться им можно только сразу после получения)
const array<ProjectSoundBank *> & SndProject::SoundBankArray()
{
	return soundBanks;
}


//Добавить опции звукового банка, возвращает идентификатор описания банка
ErrorId SndProject::SoundBankAdd(const char * name, UniqId * id)
{
	if(id) id->Reset();
	ErrorId errorId;
	//Проверяем имя	
	if(!SoundBankCheckUniqeName(name))
	{
		return options->ErrorOut(null, true, "Sound bank create: invalidate name");
	}
	//Пытаемся создать папку
	UniqId folderId;
	if(!FolderAdd(folderSounds, name, &folderId))
	{
		return options->ErrorOut(null, true, "Sound bank create: can't create folder \"%s\"", name);
	}
	//Создаём объект банка
	ProjectSoundBank * bank = NEW ProjectSoundBank(folderId);
	if(bank->Init(name) != ProjectObject::ec_ok)
	{
		return options->ErrorOut(null, true, "Sound bank create: error create description file \"%s.txt\"", name);
	}
	//Установим поля по инициализации
	bank->Reset(false);
	//Регистрируем этот объект
	Verify(ObjectRegister(bank->GetId(), pot_soundbank, bank));
	soundBanks.Add(bank);
	if(id) *id = bank->GetId();
	bank->SetToSave();
	return ErrorId::ok;
}


//Удалить опции звукового банка
bool SndProject::SoundBankDelete(const UniqId & bankId)
{
	//Надо удалить папку и содержимое. Может потом, когда нибудь...
	return false;
}

//Переименовать звук
bool SndProject::SoundBankRename(const UniqId & soundId, const char * name)
{
	return false;
}

//Проверить имя на уникальность
bool SndProject::SoundBankCheckUniqeName(const char * name, const UniqId * skipSoundBankId)
{
	return NameCheckForUnique<ProjectSoundBank>(soundBanks, ProjectSoundBank::c_namelen, UniqId::zeroId, name, skipSoundBankId);
}

//--------------------------------------------------------------------------------------
//Работа с анимацией
//--------------------------------------------------------------------------------------

//Получить анимацию
ProjectAnimation * SndProject::AnimationGet(const UniqId & id)
{
	ProjectAnimation * pa = (ProjectAnimation *)ObjectFind(id, pot_animation);
	return pa;
}

//Получить список роликов
const array<ProjectAnimation *> & SndProject::AnimationArray()
{
	return animations;
}

//Добавить анимацию в проект
ErrorId SndProject::AnimationAdd(const char * anxName, UniqId * id)
{
	if(id) id->Reset();
	ErrorId errorId;
	//Проверяем на уникальность имя
	if(!AnimationCheckUniqeName(anxName))
	{
		return options->ErrorOut(null, true, "Animation create: invalidate name");
	}
	//Создаём объект и инициализируем
	ProjectAnimation * pa = NEW ProjectAnimation(folderMovies);
	if(pa->Init(anxName) != ProjectObject::ec_ok)
	{
		return options->ErrorOut(null, true, "Animation create: error create movie file \"%s.txt\"", anxName);
	}
	errorId = pa->SetAnimation(anxName);
	if(!errorId.IsOk())
	{
		delete pa;
		return errorId;
	}
	//Регистрируем готовую анимацию
	ObjectRegister(pa->GetId(), pot_animation, pa);
	animations.Add(pa);
	if(id) *id = pa->GetId();
	return ErrorId::ok;
}

//Удалить анимацию из проекта
bool SndProject::AnimationDelete(const UniqId & id)
{
	//Указатель на объект
	ProjectAnimation * pa = AnimationGet(id);
	if(!pa)
	{
		return false;
	}
	//Удалим связи со звуками
	AnimationPrepareForSounds(UniqId());
	//Удаляем файл
	if(!pa->Delete())
	{
		return false;
	}
	//Удаляем регистрационные записи
	Verify(ObjectRemove(pa->GetId(), pot_animation));
	animations.Del(pa);
	delete pa;
	pa = null;
	return true;
}

//Подготовить анимацию к поигрыванию звуков
void SndProject::AnimationPrepareForSounds(const UniqId & id)
{
	for(dword i = 0; i < animations.Size(); i++)
	{
		ProjectAnimation * pa = animations[i];
		if(pa->GetId() != id)
		{
			animations[i]->EventsDeleteSounds();
		}else{
			animations[i]->EventsPrepareSounds();
		}
	}
}

//Проверить имя на уникальность
bool SndProject::AnimationCheckUniqeName(const char * name, const UniqId * skipAniId)
{
	return NameCheckForUnique<ProjectAnimation>(animations, ProjectAnimation::c_nameLen, UniqId::zeroId, name, skipAniId);
}

//--------------------------------------------------------------------------------------
//Состояние проекта
//--------------------------------------------------------------------------------------

//Добавить обновление в очередь
void SndProject::AddToSave(ProjectObject * po)
{
	for(dword i = 0; i < waitForSave.Size(); i++)
	{
		if(waitForSave[i] == po) return;
	}
	waitForSave.Add(po);
	waitToSave = true;
}

//Удалить обновляющий объект из списка
void SndProject::RemoveFromSave(ProjectObject * po)
{
	for(dword i = 0; i < waitForSave.Size(); )
	{
		if(waitForSave[i] == po)
		{
			waitForSave.DelIndex(i);
		}else{
			i++;
		}
	}	
	if(waitForSave.Size() == 0)
	{
		waitToSave = false;
	}
}

//Сохранить все изменения
void SndProject::FlushData()
{
	for(dword i = 0; i < waitForSave.Size(); i++)
	{
		waitForSave[i]->SaveProcess();
	}
	waitForSave.Empty();
	waitToSave = false;
}

//Получить текущее время проекта
double SndProject::GetProjectTime()
{
	return projectTime;
}

//--------------------------------------------------------------------------------------
//Системное
//--------------------------------------------------------------------------------------

//Открыть бинарный файл на полный доступ
IFile * SndProject::FileOpenBin(const char * path, dword flags, bool * isLock, const char * cppFile, long cppLine, long debugId)
{
	binFilesSyncro.Enter();
	string fullPath;
	//Получим полный путь
	options->fileService->BuildPath(path, fullPath);
	//Цикл ожидания открытия занятого файла	
	dword tryesCount = 0;
	while(true)
	{
		//Ищем среди открытых	
		long index = FileFindFile(openBinFiles, fullPath);
		if(index < 0)
		{
			//Файла нет, выходим из цикла
			break;
		}
		//Смотрим на состояние открытого файла
		PrjOpenFile & of = openBinFiles[index];
		if((of.flags & ffex_isOpen) == 0)
		{
			//Файл не заблокирован, проверяем режим чтения
			if(of.flags & poff_isOpenForRead)
			{
				//Файл был открыт на чтение
				if((flags & poff_isOpenForRead) == 0)
				{
					//Нужен полный доступ к файлу, закрываем этот и открываем новый
					Verify(FileForceRelease(openBinFiles, index));
					break;
				}
			}
			((IFile *)of.file)->SetPos(0);
			if(flags & poff_isCreateNewFile)
			{
				Assert((flags & poff_isOpenForRead) == 0);
				((IFile *)of.file)->Truncate();
			}
			of.waitTime = 0.0f;			
			of.cppFile = cppFile;
			of.cppLine = cppLine;
			of.debugId = debugId;
			of.flags |= ffex_isOpen;
			if(isLock) *isLock = false;
			binFilesSyncro.Leave();
			return (IFile *)of.file;
		}
		//Файл в данный момент используеться, недаём к нему доступ
		if((flags & poff_isWaitWhenOpen) && tryesCount < 100)
		{
			//Ожидаем некоторое время
			binFilesSyncro.Leave();
			Sleep(20);
			binFilesSyncro.Enter();
			tryesCount++;
		}else{
			//Ждать ненадо, сообщаем что файл заблокирован
			if(isLock) *isLock = true;
			binFilesSyncro.Leave();
			return null;
		}
	}
	//Открытого файла в проекте нет, открываем новый файл
	IFileService_OpenMode mode = file_open_existing;
	if(flags & poff_isCreateNewFile) mode = file_create_always;
	if(flags & poff_isOpenForRead) mode = file_open_existing_for_read;
	//Цикл открытия с ожиданием системы
	IFile * file = null;
	for(dword i = 0; i < 100; i++)
	{
		file = options->fileService->OpenFile(fullPath.c_str(), mode, _FL_);
		if(file) break;
		//Если файл ещё неуспел освободиться, подождём
		DWORD error = ::GetLastError();
		if(error != ERROR_SHARING_VIOLATION)
		{
			break;
		}
		Sleep(10);
	}
	if(!file)
	{
		if(isLock) *isLock = false;
		binFilesSyncro.Leave();
		return null;
	}
	//Добавляем запись о файле
	PrjOpenFile & of = openBinFiles[openBinFiles.Add()];	
	of.file = file;
	of.flags = ffex_isOpen | (flags & poff_isOpenForRead);
	of.waitTime = 0.0f;
	of.cppFile = cppFile;
	of.cppLine = cppLine;
	of.debugId = debugId;
	if(isLock) *isLock = false;
	//Почистим массив на случай накопления большого количества подвисших файлов
	FilesCloseProcess(openBinFiles, 0.0f);
	binFilesSyncro.Leave();
	return file;
}

//Закрыть бинарный файл
void SndProject::FileCloseBin(IFile * file, long debugId)
{
	SyncroCode syncFileOp(binFilesSyncro);
	for(dword i = 0; i < openBinFiles.Size(); i++)
	{
		PrjOpenFile & of = openBinFiles[i];
		if(of.file == file)
		{
			Assert(of.debugId == debugId);
			Assert(of.flags & ffex_isOpen);
			of.flags &= ~ffex_isOpen;
			of.waitTime = 0.0f;
			return;
		}
	}
}

//Принудительно закрыть бинарный файл с данным именем
bool SndProject::FileForceReleaseBin(const char * path)
{
	SyncroCode syncFileOp(binFilesSyncro);
	string fullPath;
	return FileForceRelease(openBinFiles, path, fullPath);
}

//Открыть ini файл
IEditableIniFile * SndProject::FileOpenIni(const char * path, bool isNewFile, bool isReadMode, const char * cppFile, long cppLine)
{
	AssertCoreThread
	//Получим полный путь
	options->fileService->BuildPath(path, openIniFileBuffer);
	//Ищем среди открытых	
	long index = FileFindFile(openIniFiles, openIniFileBuffer);
	if(index >= 0)
	{
		PrjOpenFile & of = openIniFiles[index];
		if(of.flags & ffex_isOpen)
		{
			return null;
		}
		if(isNewFile)
		{
			Assert(!isReadMode);
			of.file->Release();
			of.file = null;
			openIniFiles.DelIndex(index);
			options->fileService->Delete(openIniFileBuffer.c_str());
		}else{
			if(isReadMode || !(of.flags & poff_isOpenForRead))
			{
				of.flags = ffex_isOpen;
				of.waitTime = 0.0f;
				of.cppFile = cppFile;
				of.cppLine = cppLine;
				return (IEditableIniFile *)of.file;
			}
			//Нужен полный доступ к файлу, закрываем этот и открываем новый
			Verify(FileForceRelease(openIniFiles, index));
		}
	}
	//Открываем новый файл	
	IFileService_OpenMode mode = isNewFile ? file_create_new : file_open_existing;
	if(isReadMode) mode = file_open_existing_for_read;
	IEditableIniFile * iniFile = null;
	for(dword i = 0; i < 100; i++)
	{
		iniFile = options->fileService->OpenEditableIniFile(openIniFileBuffer.c_str(), mode, _FL_);
		if(iniFile) break;
		//Если файл ещё неуспел освободиться, подождём
		DWORD error = ::GetLastError();
		if(error != ERROR_SHARING_VIOLATION)
		{
			break;
		}
		Sleep(10);
	}	
	if(!iniFile) return null;
	PrjOpenFile & of = openIniFiles[openIniFiles.Add()];
	of.file = iniFile;
	of.flags = ffex_isOpen | poff_isOpenForRead;
	of.waitTime = 0.0f;
	of.cppFile = cppFile;
	of.cppLine = cppLine;
	//Почистим массив на случай накопления большого количества подвисших файлов
	FilesCloseProcess(openIniFiles, 0.0f);
	return iniFile;
}

//Закрыть ini файл
void SndProject::FileCloseIni(IEditableIniFile * ini)
{
	AssertCoreThread
	for(dword i = 0; i < openIniFiles.Size(); i++)
	{
		PrjOpenFile & of = openIniFiles[i];
		if(of.file == ini)
		{
			Assert(of.flags & ffex_isOpen);			
			if((of.flags & poff_isOpenForRead) == 0)
			{
				((IEditableIniFile *)of.file)->Flush();
			}
			of.waitTime = 0.0f;
			of.flags &= ~ffex_isOpen;
			return;
		}
	}
}

//Принудительно закрыть ini файл с данным именем
bool SndProject::FileForceReleaseIni(const char * path)
{
	AssertCoreThread
	return FileForceRelease(openIniFiles, path, openIniFileBuffer);
}

//Найти открытый файл по имени
long SndProject::FileFindFile(array<PrjOpenFile> & files, string & fullPath)
{
	//Проходимся по массиву
	for(dword i = 0; i < files.Size(); i++)
	{
		PrjOpenFile & of = files[i];
		if(fullPath == of.path)
		{
			return i;
		}
	}
	return -1;
}

//Принудительно закрыть файл с данным именем
bool SndProject::FileForceRelease(array<PrjOpenFile> & files, const char * path, string & fullPathBuffer)
{
	//Получим полный путь
	options->fileService->BuildPath(path, fullPathBuffer);
	//Ищем открытый файл
	long index = FileFindFile(files, fullPathBuffer);
	if(index < 0)
	{
		//Файла нет, это значит он закрыт уже
		return true;
	}
	//Закрыть файл
	return FileForceRelease(files, index);
}

//Принудительно закрыть файл с данным индексом
bool SndProject::FileForceRelease(array<PrjOpenFile> & files, long index)
{
	PrjOpenFile & of = files[index];
	if(of.flags & ffex_isOpen)
	{
		//Файл используеться, поэтому неможем его закрыть
		return false;
	}
	of.file->Release();
	of.file = null;
	of.flags = poff_noflags;
	files.DelIndex(index);
	return true;
}

//Проверка на закрытие неиспольуемых файлов
void SndProject::FilesCloseProcess(array<PrjOpenFile> & files, float dltTime)
{
	//Проверка подвисших во времени файлов
	if(dltTime >= 0)
	{
		for(dword i = 0; i < files.Size(); i++)
		{
			PrjOpenFile & of = files[i];
			if((of.flags & ffex_isOpen) == 0)
			{
				of.waitTime += dltTime;
				if(of.waitTime >= c_waitTimeForCloseFileInMs*0.001f)
				{
					of.file->Release();
					of.file = null;
					of.flags = poff_noflags;
					files.DelIndex(i);
					i--;
				}
			}
		}
	}
	//Проверка по количеству одновременно открытых файлов
	while(files.Size() > 32)
	{
		//Пора принудительно чистить массив
		float maxWaitTime = -1.0f;
		long selIndex = -1;
		for(dword i = 0; i < files.Size(); i++)
		{
			PrjOpenFile & of = files[i];
			if((of.flags & ffex_isOpen) == 0)
			{
				if(of.waitTime >= maxWaitTime)
				{
					maxWaitTime = of.waitTime;
					selIndex = i;
				}
			}
		}
		//Если больше нечего удалять, выходим из цикла
		if(selIndex < 0) break;
		//Удаляем выбраный элемент и повторяем действие
		PrjOpenFile & of = files[selIndex];
		of.file->Release();
		of.file = null;
		of.flags = poff_noflags;
		files.DelIndex(selIndex);
	}
	if(files.Size() > 64)
	{
		//Чтото неправильно, проспамим лог
		api->Trace("Sound project warning! Too many open file at one time: %u", files.Size());
		for(dword i = 0; i < files.Size(); i++)
		{
			api->Trace("    %u: %s", i, files[i].path.c_str());
		}
	}
}

//Удалить все файлы игнорируя состояние
void SndProject::FilesRelease(array<PrjOpenFile> & files)
{
	for(dword i = 0; i < files.Size(); i++)
	{
		PrjOpenFile & of = files[i];
		of.file->Release();
		of.file = null;
	}
	files.Empty();
}

//--------------------------------------------------------------------------------------
//Обновление
//--------------------------------------------------------------------------------------

//Обносить состояние проекта
void SndProject::Update(float dltTime)
{
	//Обновляем счётчики
	projectTime += (double)dltTime;
	if(projectTime > 60.0*60.0*24.0*31.0)
	{
		projectTime = 0.0;
	}
	updateTimer += dltTime;
	if(updateTimer >= c_updateTimeInMs*0.001f)
	{
		updateTimer = 0.0f;
		FlushData();
	}
	//Закрываем долго висящие файлы
	binFilesSyncro.Enter();
	FilesCloseProcess(openBinFiles, dltTime);
	binFilesSyncro.Leave();
	FilesCloseProcess(openIniFiles, dltTime);
	//Процесс распределённого обновления объектов проекта
	dword count = waves.Size()/128;
	for(dword i = 0; i <= count; i++)
	{
		if(waveUpdater < waves.Size())
		{
			if(waves[waveUpdater])
			{
				waves[waveUpdater]->WorkUpdate(false);
			}
		}
		waveUpdater++;
		if(waveUpdater >= waves.Size())
		{
			waveUpdater = 0;
		}
	}
	if(soundUpdaterPauseTimer <= 0.0f)
	{
		count = sounds.Size()/256;
		for(dword i = 0; i <= count; i++)
		{
			if(soundUpdater < sounds.Size())
			{
				if(sounds[soundUpdater])
				{
					if(sounds[soundUpdater]->WorkUpdate())
					{
						soundUpdaterPauseTimer = 1.0f;
					}
				}
			}
			soundUpdater++;
			if(soundUpdater >= sounds.Size())
			{
				soundUpdater = 0;
			}
		}
	}else{
		soundUpdaterPauseTimer -= dltTime;
	}
}

//--------------------------------------------------------------------------------------
//Загрузка проекта
//--------------------------------------------------------------------------------------

//Загрузить проект
void SndProject::LoadProject()
{
	//Проверяем, что проект чист	
	Assert(!folderRoot.IsValidate());
	Assert(!folderWaves.IsValidate());
	Assert(!folderSounds.IsValidate());
	Assert(!folderMovies.IsValidate());
	Assert(!folderSoundParams.IsValidate());
	Assert(!folderAttenuations.IsValidate());
	Assert(waves.Size() == 0);
	Assert(sounds.Size() == 0);
	Assert(soundBaseParams.Size() == 0);
	Assert(attGraphs.Size() == 0);
	Assert(folders.Size() == 0);
	Assert(soundBanks.Size() == 0);
	Assert(animations.Size() == 0);
	Assert(waitForSave.Size() == 0);
	//Формируем корневую папку
	PrjFolder * fld = NEW PrjFolder();
	fld->id.Build();
	fld->name.SetString("Project");
	fld->parent = null;
	fld->fullPath = options->pathProject;
	fld->noEditable = true;
	folders.Add(fld);
	ObjectRegister(fld->id, pot_folder, fld);
	folderRoot = fld->id;
	//Зачитываем базовые параметры звуков
	LoadLinearData<ProjectSoundBaseParams>("setup", pot_baseparams, folderSoundParams, soundBaseParams, null);
	//Зачитываем графики затухания
	LoadLinearData<ProjectSoundAttGraph>("attenuation", pot_attenuation, folderAttenuations, attGraphs, null);
	//Зачитываем волны
	LoadRecursiveData<ProjectWave>("Wave", "pwv", pot_wave, folderWaves, waves, false, &SndProject::WaveDelete);
	//Зачитываем банки и звуки
	LoadRecursiveData<ProjectSound>("Sound", "txt", pot_sound, folderSounds, sounds, true, &SndProject::SoundDelete);
	//Зачитать параметры звуковых банков
	LoadSoundBanksParams();
	//Загрузить анимационные проекты
	LoadLinearData<ProjectAnimation>("Movie", pot_animation, folderMovies, animations, null);
	//Контрольная проверка памяти
	api->ExecuteCoreCommand(CoreCommand_CheckMemory());
}

//Зачитать глобальные параметры звука
template<class T> void SndProject::LoadLinearData(const char * resourceName, ObjectTypes type, UniqId & folderId, array<T *> & arr, FolderObjectDelete delFunc)
{
	//Корневая папка для данного типа данных
	options->tmp_Project_LoadProcess = resourceName;
	options->tmp_Project_LoadProcess += "s";
	Verify(FolderAdd(folderRoot, options->tmp_Project_LoadProcess.c_str(), &folderId));
	PrjFolder * folder = (PrjFolder *)ObjectFind(folderId, pot_folder);
	Assert(folder);
	folder->del = delFunc;
	folder->noEditable = true;
	//Собираем в ней все объекты
	IFinder * finder = options->fileService->CreateFinder(FolderGetPath(folderId), "*.txt", find_no_recursive | find_no_mirror_files | find_no_files_from_packs, _FL_);
	Assert(finder);
	//Загружаем параметры
	for(dword i = 0; i < finder->Count(); i++)
	{
		Assert(!finder->IsFolder(i));
		T * obj = NEW T(folderId);
		//Установим имя
		if(obj->SetName(finder->Title(i)) != ProjectObject::ec_ok)
		{
			options->ErrorOut(null, true, "%s load error! Name to long: \"%s\"", resourceName, finder->Title(i));
			delete obj;
			continue;
		}
		//Загружаем данные
		ErrorId errorId = obj->LoadProcess();
		if(!errorId.IsOk())
		{
			delete obj;
			continue;
		}
		//Регистрируем
		if(!ObjectRegister(obj->GetId(), type, obj))
		{
			options->ErrorOut(null, true, "%s load error! Id is repeat. File name: \"%s\"", resourceName, finder->Name(i));
			delete obj;
			continue;
		}
		arr.Add(obj);
	}
	finder->Release();
}

//Зачитать рекурсивные данные
template<class T> void SndProject::LoadRecursiveData(const char * resourceName, const char * extention, ObjectTypes type, UniqId & folderId, array<T *> & arr, bool skipParentFolderObjects, FolderObjectDelete delFunc)
{
	//Корневая папка для данного типа данных
	options->tmp_Project_LoadProcess = resourceName;
	options->tmp_Project_LoadProcess += "s";
	Verify(FolderAdd(folderRoot, options->tmp_Project_LoadProcess.c_str(), &folderId));
	PrjFolder * folder = (PrjFolder *)ObjectFind(folderId, pot_folder);
	Assert(folder);
	folder->del = delFunc;
	folder->noEditable = true;
	//Собираем в ней все объекты
	IFinder * finder = options->fileService->CreateFinder(FolderGetPath(folderId), "*.*", find_no_mirror_files | find_folders | find_no_files_from_packs, _FL_);
	Assert(finder);
	//Создаём аналогичную структуру в проекте
	for(dword i = 0; i < finder->Count(); i++)
	{
		//Папка в проекте, куда положим объект
		UniqId fldId;
		LoadMakePath(finder->Path(i), folderId, fldId);
		if(skipParentFolderObjects && folderId == fldId)
		{
			continue;
		}
		if(finder->IsFolder(i))
		{
			continue;
		}
		//Проверяем расширение
		if(!string::IsEqual(finder->Extension(i), extention))
		{
			continue;
		}
		//Создаём объект
		T * resource = NEW T(fldId);
		//Установим объекту имя
		if(resource->SetName(finder->Title(i)) != ProjectObject::ec_ok)
		{
			options->ErrorOut(null, true, "%s load error! Name to long: \"%s\"", resourceName, finder->Title(i));
			delete resource;
			continue;
		}
		//Загружаем объект
		ErrorId errorId = resource->LoadProcess();
		if(!errorId.IsOk())
		{
			delete resource;
			continue;
		}
		//Регистрируем
		if(!ObjectRegister(resource->GetId(), type, resource))
		{
			options->ErrorOut(null, true, "%s load error! Id is repeat. File name: \"%s\"", resourceName, finder->Name(i));
			delete resource;
			continue;
		}
		arr.Add(resource);
	}
	finder->Release();
}

//Создать группу папок соответствующих относительному пути родителя
void SndProject::LoadMakePath(const char * path, const UniqId & parentFolderId, UniqId & folderId)
{
	folderId.Reset();
	//Корневая папка для данной ветки
	PrjFolder * parentFolder = (PrjFolder *)ObjectFind(parentFolderId, pot_folder);
	Assert(parentFolder);
	//Получаем чищеный относительный путь
	string locPath = path;
	locPath.GetRelativePath(parentFolder->fullPath);
	locPath.CheckPath();
	if(locPath.Last() == '\\') locPath.Delete(locPath.Len() - 1, 1);
	//Если относительный путь пуст, то родительская папка и есть искомая
	if(locPath.IsEmpty())
	{
		folderId = parentFolderId;
		return;
	}
	//Имеем относительный путь, оканчивающийся на "\", проходим по именам между слэшами
	char * name = locPath.GetDataBuffer();
	char * current = name;	
	while(true)
	{
		if(*current == '\\' || *current == 0)
		{
			bool isStop = (*current == 0);
			*current = 0;
			Verify(FolderAdd(parentFolder->id, name, &folderId));
			parentFolder = (PrjFolder *)ObjectFind(folderId, pot_folder);
			if(isStop) break;
			current++;
			name = current;			
		}else{
			current++;
		}
	}
}


//Зачитать параметры звуковых банков
void SndProject::LoadSoundBanksParams()
{
	PrjFolder * folder = (PrjFolder *)ObjectFind(folderSounds, pot_folder);
	Assert(folder);
	//Собираем в ней все объекты
	for(dword i = 0; i < folder->child.Size(); i++)
	{
		//Папка звукового банка
		PrjFolder * sbankFolder = folder->child[i];
		//Описание звукового банка
		ProjectSoundBank * sb = NEW ProjectSoundBank(sbankFolder->id);
		Verify(sb->SetName(sbankFolder->name.str) == ProjectObject::ec_ok);
		ErrorId errorId = sb->LoadProcess();		
		if(errorId.IsError())
		{
			delete sb;
			sb = null;
			UniqId id;			
			SoundBankAdd(sbankFolder->name.str, &id);
			sb = SoundBankGet(id);
		}else{
			if(!ObjectRegister(sb->GetId(), pot_soundbank, sb))
			{
					options->ErrorOut(null, true, "Sound bank load error! Id is repeat. Bank name: \"%s\"", sb->GetName().str);
					UniqId id;
					id.Build();
					sb->ReplaceId(id);
					Verify(ObjectRegister(sb->GetId(), pot_soundbank, sb));
			}
			soundBanks.Add(sb);
		}
	}
}

//Проверить имя на уникальность
template<class ProjectObjectBranch> bool SndProject::NameCheckForUnique(array<ProjectObjectBranch *> & objects, dword maxNameLen, const UniqId & folderId, const char * name, const UniqId * skipId, const UniqId * parentFolderForCheck)
{
	//Пустое имя быть не может
	if(string::IsEmpty(name))
	{
		return false;
	}
	//Проверяем длинну имени
	dword len = 0;
	dword hash = string::HashNoCase(name, len);
	if(len >= maxNameLen)
	{
		return false;
	}
	//Проверяем имя на уникальность среди остальных
	for(dword i = 0; i < objects.Size(); i++)
	{
		ProjectObject * po = objects[i];
		if(skipId)
		{
			if(po->GetId() == *skipId)
			{
				//Пропустим указанный объект (используеться при переименовании)
				continue;
			}
		}
		if(parentFolderForCheck)
		{
			const UniqId & folder =  po->GetFolder();
			if(!FolderIsChild(*parentFolderForCheck, folder))
			{
				//Пропустим указанный объект не из указанной папки
				continue;
			}
		}
		const ExtName & poname = po->GetName();
		if(poname.hash == hash && poname.len == len)
		{
			if(string::IsEqual(poname.str, name))
			{
				return false;
			}
		}
	}
	//Проверяем папки
	if(folderId.IsValidate())
	{
		//Указатель на папку
		PrjFolder * folder = (PrjFolder *)ObjectFind(folderId, pot_folder);
		Verify(folder);
		array<PrjFolder *> & child = folder->child;
		for(dword i = 0; i < child.Size(); i++)
		{
			const ExtName & fname = child[i]->name;
			if(fname.hash == hash && fname.len == len)
			{
				if(string::IsEqual(fname.str, name))
				{
					return false;
				}
			}
		}
	}
	return true;
}

