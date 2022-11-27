

#include "FileService.h"
#include "DataFile.h"
#include "File.h"
#include "IniFile.h"
#include "Finder.h"
#include "LoadBuffer.h"
#include "PackFile.h"
#include "MirrorPath.h"


CREATE_SERVICE(FileService, 0)


FileService * FileService::object = null;


FileService::FileService() : dataFiles(_FL_),								
								iniFiles(_FL_),
								finders(_FL_),
								buffers(_FL_),
								packFiles(_FL_),
								mirrorPaths(_FL_)
#ifndef _XBOX
								, rwFiles(_FL_)
#endif
{
	fullPath.Reserve(MAX_PATH*4);
	object = this;
	dcmprs = NEW XStreamDecompressor();
	isUseDcmprs = false;
#ifndef _XBOX
	dword size = MAX_PATH + 4096;
	char * buffer = NEW char[size];
	memset(buffer, 0, size);
	HMODULE module = GetModuleHandle(null);
	GetModuleFileName(module, buffer, size);
	buffer[size - 1] = 0;		
	currentPath = buffer;
	currentPath.GetFilePath(string(buffer));
	delete buffer;
	if(currentPath.Len() > 0)
	{
		currentPath.CheckPath();
		if(currentPath.Last() != '\\')
		{
			currentPath += '\\';
		}
	}
#else
	currentPath = "game:\\";
#endif
	iniPacks = null;
	systemIni = null;
	stopErrors = false;
	isLockFileAccess = false;
}

FileService::~FileService()
{
	if(systemIni)
	{
		((IniFile *)systemIni)->LockRelease(false);
		systemIni->Release();
		systemIni = null;
	}
	for(long i = 0; i < dataFiles; i++)
	{
		dataFiles[i]->ErrorRelease();
	}
#ifndef _XBOX
	for(long i = 0; i < rwFiles; i++)
	{
		rwFiles[i]->ErrorRelease();
	}
#endif
	for(long i = 0; i < iniFiles; i++)
	{
		iniFiles[i]->ErrorRelease();
	}
	for(long i = 0; i < finders; i++)
	{
		finders[i]->ErrorRelease();
	}
	for(long i = 0; i < buffers; i++)
	{
		buffers[i]->ErrorRelease();
	}
	if(iniPacks)
	{
		iniPacks->Release();
	}
	for(long i = 0; i < packFiles; i++)
	{
		packFiles[i]->ErrorRelease();
	}	
	for(long i = 0; i < mirrorPaths; i++)
	{
		mirrorPaths[i]->ErrorRelease();
	}
	delete dcmprs;
}

bool FileService::Init()
{
#ifndef _XBOX
	iniPacks = LoadPack("resource\\ini.pkx", _FL_);
#else
	iniPacks = LoadPack("ini.pkx", _FL_);
#endif
	IIniFile * ini = SystemIni();
	if(ini)
	{
		//drainPath = systemIni->GetString("fileservice", "drainpath", "");
		stopErrors = systemIni->GetLong("fileservice", "trace", 1) == 0;
	}
	return true;
}

//Удаление записи об ресурсе
void FileService::DeleteDataFile(DataFileBase * ptr)
{
	dataFiles.Del(ptr);
}

#ifndef _XBOX
void FileService::DeleteFile(File * ptr)
{
	rwFiles.Del(ptr);
}
#endif

void FileService::DeleteIniFile(IniFile * ptr)
{
	iniFiles.Del(ptr);
}

void FileService::DeleteFinder(Finder * ptr)
{
	finders.Del(ptr);
}

void FileService::DeleteLoadBuffer(LoadBuffer * ptr)
{
	buffers.Del(ptr);
}

void FileService::DeletePackFile(PackFile * ptr)
{
	packFiles.Del(ptr);
}

void FileService::DeleteMirrorPath(MirrorPath * ptr)
{
	filesTree.DelMirrorPath(ptr);
	mirrorPaths.Del(ptr);
}

//Открыть файл данных (файл может быть архивирован)
IDataFile * FileService::OpenDataFile(const char * fileName, IFileService_DFOpenMode mode, const char * cppFileName, long cppFileLine)
{
	return OpenDataFile(fileName, mode, true, cppFileName, cppFileLine);
}

//Открыть файл данных (файл может быть архивирован)
IDataFile * FileService::OpenDataFile(const char * fileName, IFileService_DFOpenMode mode, bool isOutputError, const char * cppFileName, long cppFileLine)
{
	if(isLockFileAccess)
	{
		return null;
	}
	SingleClassThread
	//Режим открытия файла
	if(mode == file_open_default)
	{
#ifndef _XBOX
#ifndef STOP_DEBUG
		mode = file_open_any;
#else
		mode = file_open_frompack;
#endif
#else
		//XBOX
		mode = file_open_frompack;
#endif
	}
	Assert((mode & 3) != 0);
	//Смотрим файл в загруженных паках
	if(packFiles.Size() > 0 && (mode & file_open_frompack))
	{
		const char * name = string::GetFileName(fileName);
		dword len = 0;
		dword hash = string::HashNoCase(name, len);
		dword packedSize = 0;
		dword fileSize = 0;
		for(dword i = 0; i < packFiles.Size(); i++)
		{
			const byte * ptr = packFiles[i]->Find(name, hash, len, packedSize, fileSize);
			if(ptr)
			{
				//Открываем из пак файла
				DataFileBase * mfile = null;
				if(packedSize < fileSize)
				{
					mfile = NEW DataFileMemPack(cppFileName, cppFileLine, ptr, packedSize, fileSize);
				}else{
					mfile = NEW DataFileMemUnpack(cppFileName, cppFileLine, ptr, fileSize);
				}
				dataFiles.Add(mfile);
				fullPath = name;
				return mfile;
			}
		}
	}
	//Получаем полный путь
	const char * path = null;
	if((mode & file_open_fromdisk) == 0 || (path = BuildPath_noSafe(fileName)) == null)
	{
		if(!isOutputError)
		{
			Error("FileService error: can't open file \"%s\" (file: %s, line: %i -> OpenDataFile)", fileName, cppFileName, cppFileLine);
		}
		return null;
	}
	//Пытаемся найти файл в зеркальных паках
	dword count = filesTree.FindPaths(path);
	//Открываем с диска файл
	DataFile * file = NEW DataFile(cppFileName, cppFileLine);
	//Пытаемся найти файл в зеркальных папках
	for(dword i = 0; i < count; i++)
	{
		const char * p = filesTree.GetPath(i);
		if(file->Open(p))
		{
			fullPath = p;
			dataFiles.Add(file);
			DrainFile(fullPath.c_str());
			return file;
		}
	}
	//Пробуем открыть по прямому пути
	if(file->Open(path))
	{
		DrainFile(path);
		dataFiles.Add(file);
		return file;
	}
	delete file;
	if(!isOutputError)
	{
		Error("FileService error: can't open file \"%s\" (file: %s, line: %i -> OpenDataFile)", fileName, cppFileName, cppFileLine);
	}
	return null;
}

#ifndef _XBOX

//Открыть файл
IFile * FileService::OpenFile(const char * fileName, IFileService_OpenMode mode, const char * cppFileName, long cppFileLine)
{
	return OpenFile(fileName, mode, true, cppFileName, cppFileLine);
}

//Открыть файл
IFile * FileService::OpenFile(const char * fileName, IFileService_OpenMode mode, bool isOutputError, const char * cppFileName, long cppFileLine)
{
	if(isLockFileAccess)
	{
		return null;
	}
	//Получаем полный путь
	const char * path = BuildPath_noSafe(fileName);
	if(path)
	{
		File * file = NEW File(cppFileName, cppFileLine);
		if(file->Open(path, mode))
		{
			SingleClassThread
			rwFiles.Add(file);
			return file;
		}
		delete file;
	}
	if(!isOutputError)
	{
		Error("FileService error: can't open file \"%s\" (file: %s, line: %i -> OpenFile)", fileName, cppFileName, cppFileLine);
	}	
	return null;
}

//Открыть ini файл для редактирования
IEditableIniFile * FileService::OpenEditableIniFile(const char * fileName, IFileService_OpenMode mode, const char * cppFileName, long cppFileLine)
{
	if(isLockFileAccess)
	{
		return null;
	}
	IniFile * ini = NEW IniFile(fileName, false, cppFileName, cppFileLine);
	if(!ini->Open(mode))
	{
		delete ini;
		return null;
	}
	SingleClassThread
		iniFiles.Add(ini);
	return ini;
}

#endif


//Открыть ini файл
IIniFile * FileService::OpenIniFile(const char * fileName, const char * cppFileName, long cppFileLine)
{
	return OpenIniFile(fileName, file_open_default, cppFileName, cppFileLine);
}

//Открыть ini файл
IIniFile * FileService::OpenIniFile(const char * fileName, IFileService_DFOpenMode mode, const char * cppFileName, long cppFileLine)
{
	if(isLockFileAccess)
	{
		return null;
	}
	IniFile * ini = NEW IniFile(fileName, true, cppFileName, cppFileLine);
	if(!ini->Open(mode))
	{
		delete ini;
		return null;
	}
	SingleClassThread
	iniFiles.Add(ini);
	return ini;
}

//Итератор поиска файлов
IFinder * FileService::CreateFinder(const char * findPath, const char * findMask, dword flags, const char * cppFileName, long cppFileLine)
{
	if(isLockFileAccess)
	{
		return null;
	}
	SingleClassThread
	//Получаем полный путь
	const char * path = BuildPath_noSafe(findPath);
	if(!path)
	{
		path = currentPath;
	}
	Finder * finder = NEW Finder(path, findMask, flags, cppFileName, cppFileLine);
	finders.Add(finder);
	return finder;
}

//Загрузить пак-файл в память
IPackFile * FileService::LoadPack(const char * fileName, const char * cppFileName, long cppFileLine)
{
	if(isLockFileAccess)
	{
		return null;
	}
	const char * name = string::GetFileName(fileName);
	ClassThreadLock
	//Просматриваем открытые
	for(long i = 0; i < packFiles; i++)
	{
		if(packFiles[i]->IsThis(fullPath.c_str()))
		{
			packFiles[i]->AddRefCount();
			ClassThreadUnlock
			return packFiles[i];
		}
	}
	ClassThreadUnlock
	//Открываем как файл данных
	IDataFile * df = OpenDataFile(fileName, file_open_any, _FL_);
	if(!df)
	{
		Error("FileService error: can't open pack file \"%s\" (file: %s, line: %i -> LoadPak)", fileName, cppFileName, cppFileLine);
		return false;
	}	
	PackFile * pf = NEW PackFile(name, cppFileName, cppFileLine);
	if(pf->Load(df))
	{
		ClassThreadLock
		packFiles.Add(pf);
		ClassThreadUnlock
		df->Release();
		return pf;
	}
	df->Release();
	delete pf;
	return null;
}


//Отразить один путь на другой
IMirrorPath * FileService::CreateMirrorPath(const char * from, const char * on, const char * cppFileName, long cppFileLine)
{
	if(isLockFileAccess)
	{
		return null;
	}
	SingleClassThread
	//Получаем полные нормализованые пути
	const char * path = BuildPath_noSafe(from);
	if(!path)
	{
		path = currentPath;
	}
	string path_from = path;
	if(path_from.Len() > 0)
	{
		if(path_from.Last() != '\\')
		{
			path_from += '\\';
		}
	}
	path = BuildPath_noSafe(on);
	if(!path)
	{
		path = currentPath;
	}
	string path_on = path;
	if(path_on.Last() != '\\')
	{
		path_on += '\\';
	}
	if(path_from == path_on)
	{
		return null;
	}
	//Ищем среди созданых
	for(long i = 0; i < mirrorPaths; i++)
	{
		if(mirrorPaths[i]->IsThis(path_from, path_on))
		{
			mirrorPaths[i]->AddRefCount();
			return mirrorPaths[i];
		}
	}
	//Создаём новый объект и регистрируем его
	MirrorPath * mpath = NEW MirrorPath(from, on, path_from, path_on, cppFileName, cppFileLine);
	mirrorPaths.Add(mpath);
	filesTree.AddMirrorPath(mpath, path_on);
	return mpath;
}

//Прочитать файл в выделеную память с помощью OpenDataFile
ILoadBuffer * FileService::LoadData(const char * fileName, const char * cppFileName, long cppFileLine)
{
	IDataFile * file = OpenDataFile(fileName, file_open_default, _FL_);
	if(!file) return null;
	dword size = file->Size();
	byte * buffer = new(cppFileName, cppFileLine) byte[size];
	if(file->Read(buffer, size) != size)
	{
		Error("FileService error: read error from file \"%s\" (file: %s, line: %i -> LoadData)", fileName, cppFileName, cppFileLine);
		delete buffer;
		file->Release();
		return null;
	}
	file->Release();
	file = null;
	LoadBuffer * buf = NEW LoadBuffer(buffer, size, cppFileName, cppFileLine);
	SingleClassThread
	buffers.Add(buf);
	return buf;
}

//Заблокировать создание файлов
void FileService::LockFileAccess(bool isLock)
{
	isLockFileAccess = isLock;
}

#ifndef _XBOX
//Сохранить данные в файл с помощью OpenFile, перезаписав его
bool FileService::SaveData(const char * fileName, const void * data, dword size)
{
	AssertCoreThread
	IFile * file = OpenFile(fileName, file_create_always, _FL_);
	if(!file)
	{
		return false;
	}
	if(file->Write(data, size) != size)
	{
		Error("FileService error: write error to file \"%s\" (file: %s)", fileName);
		return false;
	}
	file->Release();
	return true;
}

//Проверить существует ли файл
bool FileService::IsExist(const char * fileName, bool checkAsDataFile)
{
	if(checkAsDataFile)
	{
		IDataFile * file = OpenDataFile(fileName, file_open_default, false, _FL_);
		if(file)
		{
			file->Release();
			return true;
		}
	}else{
		//Получаем полный путь
		SingleClassThread
		const char * path = BuildPath_noSafe(fileName);
		if(GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES)
		{
			return true;
		}
	}
	return false;
}

#endif

//Получить указатель на системный ini-файл
IIniFile * FileService::SystemIni()
{
	//Если файл уже открыт, то возвращаем указатель
	if(systemIni)
	{
		return systemIni;
	}
	//Необходимо загрузить ini файл
	systemIni = OpenIniFile(api->Storage().GetString("system.ini"), file_open_any, _FL_);
	if(systemIni)
	{
		((IniFile *)systemIni)->LockRelease(true);
	}
	return systemIni;
}


#ifndef _XBOX
//Создать папку (возможно указать иерархический путь)
bool FileService::CreateFolder(const char * path)
{
	if(isLockFileAccess)
	{
		return false;
	}
	AssertCoreThread
	if(!path)
	{
		return false;
	}
	string tmp;
	while(*path)
	{
		char c = *path++;
		if(c == '/') c = '\\';
		if(c == '\\')
		{
			if(tmp.Len() && tmp[tmp.Len() - 1] != '\\')
			{
				if(tmp[tmp.Len() - 1] != ':')
				{
					if(::CreateDirectory(tmp.c_str(), null) == 0)
					{
						if(::GetLastError() != ERROR_ALREADY_EXISTS)
						{
							return false;
						}
					}
				}
				tmp += '\\';
			}
		}else{
			tmp += c;
		}
	}
	if(::CreateDirectory(tmp.c_str(), null) == 0)
	{
		if(::GetLastError() != ERROR_ALREADY_EXISTS)
		{
			return false;
		}
	}
	return true;
}

//Скопировать файл
bool FileService::Copy(const char * from, const char * to)
{
	AssertCoreThread
	return ::CopyFile(from, to, false) != 0;
}

//Переместить файл
bool FileService::Move(const char * from, const char * to)
{
	AssertCoreThread
	return ::MoveFile(from, to) != 0;
}

//Переименовать файл
bool FileService::Rename(const char * from, const char * to)
{
	AssertCoreThread
	return ::MoveFile(from, to) != 0;
}

//Удалить файл
bool FileService::Delete(const char * fileName)
{
	AssertCoreThread
	dword attributes = ::GetFileAttributes(fileName);
	if(attributes != INVALID_FILE_ATTRIBUTES)
	{
		if(attributes & FILE_ATTRIBUTE_READONLY)
		{
			attributes &= ~FILE_ATTRIBUTE_READONLY;
			::SetFileAttributes(fileName, attributes);
		}
		if(attributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			return ::RemoveDirectory(fileName) != 0;
		}
	}
	return ::DeleteFile(fileName) != 0;
}

//Установить путь для сохранения открываемых файлов данных на чтение
void FileService::SetDrainPath(const char * path)
{
	AssertCoreThread
	drainPath = path;
}

//Создать пак-файл из файлов находящихся в заданной папке
bool FileService::BuildPack(const char * fileName, const char * folderPath, const char * filesMask, IFileService_PackCompressMethod method)
{
	AssertCoreThread
	//Перебираем все реальные файлы, загружая в память
	IFinder * finder = CreateFinder(folderPath, filesMask, find_all_files_no_mirrors, _FL_);
	if(!finder) return false;
	string filePath;
	PackArchivator compressor;
	for(dword i = 0; i < finder->Count(); i++)
	{
		//Открываем файл
		filePath = finder->FilePath(i);
		IFile * file = OpenFile(filePath, file_open_existing_for_read, _FL_);
		if(!file)
		{
			finder->Release();
			return false;
		}
		//Проверяем размер файла
		dword size = file->Size();
		if (size == 0)
		{
			api->Trace("File '%s' incorrect file size : %d bytes - delete from pack \"%s\"", filePath.c_str(), size, fileName);
			file->Release();
			file = NULL;
			continue;
		}
		//Загружаем файл в память
		void * data = NEW byte[size];
		if(file->Read(data, size) != size)
		{
			api->Trace("IO error, can't load data from file \"%s\". Stop building pack \"%s\"", filePath.c_str(), fileName);
			file->Release();
			finder->Release();
			return false;
		}
		file->Release();
		compressor.AddFile(filePath, data, size);
	}
	finder->Release();
	finder = null;
	PackArchivator::Method pam = PackArchivator::m_archive;
	switch(method)
	{
	case pack_cmpr_method_archive:
		pam = PackArchivator::m_archive;
		break;
	case pack_cmpr_method_store:
		pam = PackArchivator::m_store;
		break;
	default:
		Assert(false);
	}
	compressor.Process(pam);
	//Сохраняем полученные данные в файл
	if(!PackFile::SaveToPack(fileName, compressor))
	{
		api->Trace("IO error, can't save pack file \"%s\"", fileName);
		return false;
	}
	return true;
}

#endif


//Получить полный нормализованный путь до файла, возвращает ссылку на result
string & FileService::BuildPath(const char * path, string & result)
{
	SingleClassThread
	path = BuildPath_noSafe(path);
	if(path)
	{
		result = path;
	}else{
		result = currentPath;
	}
	return result;
}

//Получить полный нормализованный путь до файла
const char * FileService::BuildPath_noSafe(const char * path)
{
	if(!path || !path[0])
	{
		return null;
	}
	//Смотрим по наличию диска относительный или полный путь
	for(long i = 0; path[i]; i++)
	{
		if(path[i] == '\\' || path[i] == '/')
		{
			if(i > 0 && path[i - 1] == ':')
			{
				//Первым задано имя диска, значит путь полный
				break;
			}
		}
	}
	//Получаем полный ненармализованый путь
	if(path[i])
	{
		fullPath = path;
	}else{
		fullPath = currentPath;
		fullPath += path;
	}
	//Убираем лишние слеши, выправляем кривые, убираем точки
	fullPath.Reserve(fullPath.Len() + MAX_PATH);
	char * src = fullPath.GetDataBuffer();
	char * dst = src;
	char * start = src;
	while(*src)
	{
		if(*src == '\\' || *src == '/')
		{
			//Пропускаем последующие слэши
			for(src++; *src == '\\' || *src == '/'; src++);
			//Смотрим что за слешом			
			char * bookmark = src;
			for(long up = 0; *src == '.'; up++, src++);
			if(up)
			{
				if(*src == '\\' || *src == '/')
				{
					//Надо поднятся на up уровней вверх
					for(; dst >= start; dst--)
					{
						if(*dst == '\\')
						{
							//Уровни закончились, заканчиваем
							if(--up == 0) break;
							if(dst > start && dst[-1] == ':')
							{
								//Дошли до диска, дальше не поднимаемся
								break;
							}
						}
					}
					continue;
				}
			}
			*dst++ = '\\';
			src = bookmark;
			if(!*src) break;
		}
		char c = *src++;
		//Приведём символ к одному регистру
		if(c >= 'A' && c <= 'Z')
		{
			c += 'a' - 'A';
		}
		//Сохраняем символ
		*dst++ = c;
	}
	*dst = 0;
	return fullPath.c_str();
}

//Скопировать файл
__forceinline void FileService::DrainFile(const char * path)
{
#ifndef _XBOX
	if(drainPath.Len() > 0)
	{
		string relPath = path;
		relPath.GetRelativePath(currentPath);
		string drain = drainPath;
		if(drain.Last() != '\\' || drain.Last() != '/')
		{
			drain += '\\';
		}
		drain += relPath;
		//Error("From: %s\nTo: %s\n", path, drain.c_str());
		string filePath;
		filePath.GetFilePath(drain);		
		CreateFolder(filePath.c_str());
		if(!Copy(path, drain.c_str()))
		{
			if(!IsExist(drain.c_str(), false))
			{
				Error("Can't drain file %s to %s", path, drain.c_str());
			}			
		}
	}
#endif
}

//Вывести в лог сообщение об ошибке
void _cdecl FileService::Error(const char * error, ...)
{
	if(!stopErrors)
	{
		SingleClassThread
		api->TraceData(error, &error + 1);
	}
}

//Собрать все имена файлов с паков
void FileService::CollectFilesFromPacks(array<const char *> & names)
{
	names.Empty();
	for(dword i = 0; i < packFiles.Size(); i++)
	{
		packFiles[i]->CollectFiles(names);
	}
}

//Выделить декомпрессор для файла
XStreamDecompressor * FileService::GetStreamDecompressor()
{
	AssertCoreThread
	if(isUseDcmprs)
	{
		return NEW XStreamDecompressor();
	}
	isUseDcmprs = true;
	return dcmprs;
}

//Удалить декомпрессор для файла
void FileService::ReleaseStreamDecompressor(XStreamDecompressor * d)
{
	AssertCoreThread
	if(d == dcmprs)
	{
		isUseDcmprs = false;		
	}else{
		delete d;
	}
}








