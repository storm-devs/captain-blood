
#ifndef _FileService_h_
#define _FileService_h_

#include "..\..\common_h\FileService.h"
#include "..\..\common_h\CritSection.h"
#include "FilesTree.h"


class DataFileBase;
class File;
class IniFile;
class Finder;
class LoadBuffer;
class PackFile;
class MirrorPath;
class XStreamDecompressor;


class FileService : public IFileService
{
public:
	FileService();
	virtual ~FileService();
	bool Init();

	//Удаление записи об ресурсе
	void DeleteDataFile(DataFileBase * ptr);
#ifndef _XBOX
	void DeleteFile(File * ptr);
#endif
	void DeleteIniFile(IniFile * ptr);
	void DeleteFinder(Finder * ptr);
	void DeleteLoadBuffer(LoadBuffer * ptr);
	void DeletePackFile(PackFile * ptr);
	void DeleteMirrorPath(MirrorPath * ptr);


public:
	//Открыть файл данных (файл может быть архивирован)
	virtual IDataFile * OpenDataFile(const char * fileName, IFileService_DFOpenMode mode, const char * cppFileName, long cppFileLine);
	//Открыть файл данных (файл может быть архивирован)
	IDataFile * OpenDataFile(const char * fileName, IFileService_DFOpenMode mode, bool isOutputError, const char * cppFileName, long cppFileLine);
	//Открыть ini файл
	virtual IIniFile * OpenIniFile(const char * fileName, const char * cppFileName, long cppFileLine);
	//Открыть ini файл
	IIniFile * OpenIniFile(const char * fileName, IFileService_DFOpenMode mode, const char * cppFileName, long cppFileLine);
	//Загрузить пак-файл в память
	virtual IPackFile * LoadPack(const char * fileName, const char * cppFileName, long cppFileLine);

	//Итератор поиска файлов
	virtual	IFinder * CreateFinder(const char * findPath, const char * findMask, dword flags, const char * cppFileName, long cppFileLine);

	//Отразить один путь на другой
	virtual IMirrorPath * CreateMirrorPath(const char * from, const char * on, const char * cppFileName, long cppFileLine);

	//Прочитать файл в выделеную память с помощью OpenDataFile
	virtual ILoadBuffer * LoadData(const char * fileName, const char * cppFileName, long cppFileLine);

	//Получить указатель на системный ini-файл
	virtual IIniFile * SystemIni();

	//Получить полный нормализованный путь до файла, возвращает ссылку на result
	virtual string & BuildPath(const char * path, string & result);
	//Получить полный нормализованный путь до файла
	const char * BuildPath_noSafe(const char * path);

	//Заблокировать создание файлов
	virtual void LockFileAccess(bool isLock);

#ifndef _XBOX
	//Проверить существует ли файл
	virtual bool IsExist(const char * fileName, bool checkAsDataFile);
	//Сохранить данные в файл с помощью OpenFile, перезаписав его
	virtual bool SaveData(const char * fileName, const void * data, dword size);
	//Открыть файл
	virtual IFile * OpenFile(const char * fileName, IFileService_OpenMode mode, const char * cppFileName, long cppFileLine);
	//Открыть файл
	IFile * OpenFile(const char * fileName, IFileService_OpenMode mode, bool isOutputError, const char * cppFileName, long cppFileLine);
	//Открыть ini файл для редактирования
	virtual IEditableIniFile * OpenEditableIniFile(const char * fileName, IFileService_OpenMode mode, const char * cppFileName, long cppFileLine);
	//Создать папку (возможно указать иерархический путь)
	virtual bool CreateFolder(const char * path);
	//Скопировать файл
	virtual bool Copy(const char * from, const char * to);
	//Переместить файл
	virtual bool Move(const char * from, const char * to);
	//Переименовать файл
	virtual bool Rename(const char * from, const char * to);
	//Удалить файл
	virtual bool Delete(const char * fileName);
	//Установить путь для сохранения открываемых файлов данных на чтение
	virtual void SetDrainPath(const char * path);
	//Создать пак-файл из файлов находящихся в заданной папке
	virtual bool BuildPack(const char * fileName, const char * folderPath, const char * filesMask, IFileService_PackCompressMethod method);
#endif


public:
	//Скопировать файл
	void DrainFile(const char * path);
	//Вывести в лог сообщение об ошибке
	void _cdecl Error(const char * error, ...);
	//Собрать все имена файлов с паков
	void CollectFilesFromPacks(array<const char *> & names);
	//Выделить декомпрессор для файла
	XStreamDecompressor * GetStreamDecompressor();
	//Удалить декомпрессор для файла
	void ReleaseStreamDecompressor(XStreamDecompressor * d);


private:
	//Открытые файлы
	array<DataFileBase *> dataFiles;	
	array<IniFile *> iniFiles;
	array<Finder *> finders;
	array<LoadBuffer *> buffers;
	array<PackFile *> packFiles;
	array<MirrorPath *> mirrorPaths;
#ifndef _XBOX
	array<File *> rwFiles;
#endif
	bool stopErrors;
	bool isLockFileAccess;

public:
	FilesTree filesTree;
	ClassThread

private:
	string currentPath;				//Рабочий каталог, оканчивающийся на "\"
	string fullPath;				//Буфер для генерации полного нормализованого пути
	string drainPath;				//Путь для копирования открываемых файлов данных
	XStreamDecompressor * dcmprs;	//Локальный декомпресор для открытого файла
	bool isUseDcmprs;				//Занят ли в данный момент декомпрессор
	
private:	
	IIniFile * systemIni;			//Системный ini Файл на момент создания сервиса
	IPackFile * iniPacks;			//Пак с ini файлами

public:
	static FileService * object;
};


#endif
