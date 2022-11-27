
#ifndef _Finder_h_
#define _Finder_h_

#include "..\..\common_h\FileService.h"

//Поиск файлов
class Finder : public IFinder
{
	enum Flags
	{
		f_normal = 0,
		f_mirror = 1,
		f_pack = 2,
		f_folder = 4,
	};


	struct FindFile
	{
		string path;
		long fileNameIndex;
		dword fileNameHash;
		dword flags;
	};

public:
	Finder(const char * fullPath, const char * findMask, dword flags, const char * _cppFileName, long _cppFileLine);
	virtual ~Finder();

	//Удалить сообщив об ошибке
	void ErrorRelease();

public:
	//Удалить объект
	virtual void Release();

	//Начать поиск заново, возвращает количество найденых файлов
	virtual dword Reset();

	//Получить количество найденых файлов
	virtual dword Count() const;

	//Получить полный путь с именем файла "С:\path\name.ext"
	virtual const char * FilePath(dword index) const;
	//Получить полный путь до файла "С:\path\"
	virtual const char * Path(dword index) const;
	//Получить имя файла "name.ext"
	virtual const char * Name(dword index) const;
	//Получить имя файла без расширения name
	virtual const char * Title(dword index) const;
	//Получить расширение файла "ext"
	virtual const char * Extension(dword index) const;
	//true если это  "." или ".."
	virtual bool IsDot(dword index) const;
	//true если это папка
	virtual bool IsFolder(dword index) const;
	//true если этот файл найден в зеркальной директории
	virtual bool IsMirror(dword index) const;

private:
	//Поиск файлов на диске
	void FindOnDisk(const char * findPath, const char * mirrorPath);
	//Рекурсивный поиск на диске
	void FindOnDiskRecursive(const char * findPath, const char * mirrorPath);
	//Поиск файлов в загруженых пак-файлах
	void FindInPacks();
	//Проверить точки ли это
	static bool IsDots(const char * str);

private:
	string path;
	string mask;
	bool isRecursive;
	bool isDots;
	bool isAddFolders;
	bool isFindInLoadedPacks;
	bool isFindInMirrors;
	bool isInverseOrder;
	array<FindFile> files;
	mutable string buffer;
#ifndef STOP_DEBUG
	const char * cppFileName;
	long cppFileLine;
#endif
};

#endif
