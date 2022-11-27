

#include "Finder.h"
#include "FileService.h"


Finder::Finder(const char * fullPath, const char * findMask, dword flags, const char * _cppFileName, long _cppFileLine) : files(_FL_)
{
	path = fullPath;
	if(path.Len() > 0 && path[path.Len() - 1] != '\\')
	{
		path += '\\';
	}
	mask = findMask;
	if(mask == "*")
	{
		mask = "*.*";
	}
	isRecursive = ((flags & find_no_recursive) == 0);
	isDots = ((flags & find_dots) != 0);
	isAddFolders = ((flags & find_folders) != 0);
	isFindInLoadedPacks = ((flags & find_no_files_from_packs) == 0);
	isFindInMirrors = ((flags & find_no_mirror_files) == 0);	
	isInverseOrder = ((flags & find_inverse_order) != 0);
#ifndef STOP_DEBUG
	cppFileName = _cppFileName;
	cppFileLine = _cppFileLine;
#endif
	Reset();
}

Finder::~Finder()
{
}

//Удалить сообщив об ошибке
void Finder::ErrorRelease()
{
#ifndef STOP_DEBUG
	FileService::object->Error("FileService error: IFinder not release (file: %s, line %i)", cppFileName, cppFileLine);
#endif
	delete this;
}

//Удалить объект
void Finder::Release()
{
	{
		SingleExClassThread(FileService::object)
		FileService::object->DeleteFinder(this);
	}
	delete this;
}

//Начать поиск заново, возвращает количество найденых файлов
dword Finder::Reset()
{
	SingleExClassThread(FileService::object)
	//Перебираем все файлы по реальному пути
	FindOnDisk(path, null);
	//Ищем в паках
	if(isFindInLoadedPacks)
	{
		FindInPacks();
	}
	//Просматриваем зеркальные пути
	if(isFindInMirrors)
	{
		dword count = FileService::object->filesTree.FindPaths(path);
		for(dword i = 0; i < count; i++)
		{
			//Зеркальный путь
			const char * mirrorPath = FileService::object->filesTree.GetPath(i);
			//Смотрим на диске
			FindOnDisk(mirrorPath, mirrorPath);
		}
	}
	for(dword i = 0; i < files.Size(); i++)
	{
		FindFile & fl = files[i];
		fl.fileNameIndex = string::GetFileName(fl.path.c_str()) - fl.path.c_str();
		fl.fileNameHash = string::HashNoCase(&fl.path[fl.fileNameIndex]);
	}
	//Замещяем одинаковые файлы файлами с большим приоритетом
	for(dword i = 0; i < files.Size(); i++)
	{
		if(!(files[i].flags & (f_mirror | f_pack))) continue;
		for(dword j = 0; j < files.Size(); j++)
		{
			//Пропускаем значения с разным хэшем
			FindFile & cur = files[i];
			FindFile & ff = files[j];
			if(ff.fileNameHash != cur.fileNameHash || (ff.flags & f_folder) != 0 || i == j) continue;
			if(!string::IsEqual(&ff.path[ff.fileNameIndex], &cur.path[cur.fileNameIndex])) continue;
			//Имена совпадают, решаем что делать
			if(cur.flags > ff.flags)
			{
				files.DelIndex(j);
				Assert(j != i);
				if(j < i) i--;
				j--;
			}
		}		
	}

/*
	api->Trace("-------------------------------------------------");
	api->Trace("Finder path: %s", path.c_str());
	api->Trace("Finder mask: %s", mask.c_str());
	for(long i = 0; i < files; i++)
	{
		api->Trace("%s", files[i].path.c_str());
	}
	api->Trace("-------------------------------------------------");
*/
	return files.Size();
}

//Получить количество найденых файлов
dword Finder::Count() const
{
	return files.Size();
}

//Получить полный путь с именем файла "С:\path\name.ext"
const char * Finder::FilePath(dword index) const
{
	if(index < files.Size())
	{
		return files[index].path.c_str();
	}
	return null;
}
//Получить полный путь до файла "С:\path\"
const char * Finder::Path(dword index) const
{
	if(index < files.Size())
	{
		if((files[index].flags & f_folder) == 0)
		{
			return buffer.GetFilePath(files[index].path);
		}else{
			return files[index].path.c_str();
		}
	}
	return null;
}

//Получить имя файла "name.ext"
const char * Finder::Name(dword index) const
{
	if(index < files.Size())
	{
		return buffer.GetFileName(files[index].path);
	}
	return null;
}

//Получить имя файла без расширения name
const char * Finder::Title(dword index) const
{
	if(index < files.Size())
	{
		return buffer.GetFileTitle(files[index].path);
	}
	return null;
}

//Получить расширение файла "ext"
const char * Finder::Extension(dword index) const
{
	if(index < files.Size())
	{
		return buffer.GetFileExt(files[index].path);
	}
	return null;
}

//true если найдены  "." или ".."
bool Finder::IsDot(dword index) const
{
	if(index < files.Size())
	{
		buffer.GetFileName(files[index].path);
		return IsDots(buffer.c_str());
	}
	return false;
}

//true если найдена папка
bool Finder::IsFolder(dword index) const
{
	if(index < files.Size())
	{
		return (files[index].flags & f_folder) != 0;
	}
	return false;
}

//true если этот файл найден в зеркальной директории
bool Finder::IsMirror(dword index) const
{
	if(index < files.Size())
	{
		return (files[index].flags & f_mirror) != 0;
	}
	return false;
}

//Поиск файлов на диске
void Finder::FindOnDisk(const char * findPath, const char * mirrorPath)
{
	//Перебираем папки для рекурсивного поиска
	if(isInverseOrder && isRecursive)
	{
		FindOnDiskRecursive(findPath, mirrorPath);
	}
	WIN32_FIND_DATA wfd;
	//Ищем в текущей папке файлы
	buffer = findPath;
	if(buffer.Len() > 0)
	{
		Assert(buffer[buffer.Len() - 1] == '\\');
	}
	buffer += mask;
	HANDLE handle = ::FindFirstFile(buffer, &wfd);
	if(handle != INVALID_HANDLE_VALUE)
	{
		do 
		{
			bool isFolder = ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
			if(isFolder)
			{
				if(!isAddFolders)
				{
					continue;
				}
				if(!isDots)
				{
					if(IsDots(wfd.cFileName))
					{
						continue;
					}
				}
			}
			FindFile & file = files[files.Add()];
			if(mirrorPath)
			{
				file.path = mirrorPath;
				buffer = findPath;
				buffer.GetRelativePath(file.path);
				file.path = path;
				file.path += buffer;
			}else{
				file.path = findPath;
			}
			file.path += wfd.cFileName;
			file.fileNameIndex = string::GetFileName(file.path.c_str()) - file.path.c_str();
			file.fileNameHash = string::HashNoCase(&file.path[file.fileNameIndex]);
			file.flags = (isFolder ? f_folder : 0) | ((mirrorPath != null) ? f_mirror : 0);
		}while(::FindNextFile(handle, &wfd));
		::FindClose(handle);
	}
	//Перебираем папки для рекурсивного поиска
	if(!isInverseOrder && isRecursive)
	{
		FindOnDiskRecursive(findPath, mirrorPath);
	}
}

//Рекурсивный поиск на диске
void Finder::FindOnDiskRecursive(const char * findPath, const char * mirrorPath)
{
	WIN32_FIND_DATA wfd;
	buffer = findPath;
	if(buffer.Len() > 0)
	{
		Assert(buffer[buffer.Len() - 1] == '\\');
	}
	buffer += "*.*";
	string path;
	HANDLE handle = ::FindFirstFile(buffer, &wfd);
	if(handle != INVALID_HANDLE_VALUE)
	{
		do 
		{
			bool isFolder = ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);				
			if(isFolder)
			{
				if(!IsDots(wfd.cFileName))
				{
					path = findPath;
					path += wfd.cFileName;
					path += '\\';
					FindOnDisk(path.c_str(), mirrorPath);
				}
			}
		}while(::FindNextFile(handle, &wfd));
		::FindClose(handle);
	}
}

//Поиск файлов в загруженых пак-файлах
void Finder::FindInPacks()
{
	array<const char *> packsFiles(_FL_, 256);
	FileService::object->CollectFilesFromPacks(packsFiles);	
	dword count = packsFiles.Size();
	for(dword i = 0; i < count; i++)
	{
		//Сравнивайм имя файла с маской
		buffer = packsFiles[i];
		if(buffer.IsFileMask(mask))
		{
			FindFile & file = files[files.Add()];
			file.path = packsFiles[i];
			file.fileNameIndex = string::GetFileName(file.path.c_str()) - file.path.c_str();
			file.fileNameHash = string::HashNoCase(&file.path[file.fileNameIndex]);
			file.flags = f_pack;
		}
	}
	buffer.Empty();
}

//Проверить точки ли это
bool Finder::IsDots(const char * str)
{
	if(!str) return false;
	for(const char * name = str; *str; str++)
	{
		if(*str == '\\' || *str == '/') name = str + 1;
	}
	if(name[0] == '.')
	{
		if(name[1] == 0)
		{
			return true;
		}
		if(name[1] == '.')
		{
			if(name[2] == 0)
			{
				return true;
			}			
		}
	}
	return false;
}
