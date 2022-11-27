#ifndef _IFileService_h_
#define _IFileService_h_

/*

Создание объектов допустимо с разных потоков, но обращение к методам
объекта допустимо только из одного потока.

*/



#include "core.h"

//Файл для чтения данных
class IFileBase
{
protected:
	IFileBase(){};
	virtual ~IFileBase(){};

public:
	//Закрыть файл
	virtual void Release() = null;
	//Поулчить путь до файла
	//virtual const char * GetPath() const = null;
};



//Файл для чтения данных
class IDataFile : public IFileBase
{
protected:
	IDataFile(){};
	virtual ~IDataFile(){};

public:
	//Прочитать данные, возвращает количество прочитаных байт
	//если поинтер null то читать во внутрений буффер
	//если режим noReadAccess, то память буфера не кэшируемая
	virtual dword Read(void * buffer, dword size, bool noReadAccess = false) = null;
	//Получить размер файла
	virtual dword Size() const = null;
};

#ifndef _XBOX

//Файл для произвольного доступа
class IFile : public IFileBase
{
protected:
	IFile(){};
	virtual ~IFile(){};

public:
	enum MoveMethod
	{
		from_begin,
		from_end,
		from_current,
	};
	
public:	
	//Прочитать данные, возвращает количество прочитаных байт
	virtual dword Read(void * buffer, dword size) = null;
	//Записать данные, возвращает количество записаных байт
	virtual dword Write(const void * buffer, dword size) = null;
	//Установить текущую позицию, возвращает установленую относительно начала
	virtual dword SetPos(dword distanceToMove, MoveMethod moveMethod = from_begin) = null;
	//Получить текущую позицию относительно начала
	virtual dword GetPos() const = null;
	//Получить размер файла
	virtual dword Size() const = null;
	//Обрезать файл по текущей позиции
	virtual bool Truncate() = null;
};

#endif


//ini файл
class IIniFile : public IFileBase
{
protected:
	IIniFile(){};
	virtual ~IIniFile(){};

public:
	//Получить список секций
	virtual void GetSections(array<string> & sections) = null;
	//Добавить секцию
	virtual void AddSection(const char * section) = null;
	//Проверить наличие секции
	virtual bool IsSectionCreated(const char * section) = null;

	//Получить количество ключей
	virtual dword GetKeysCount(const char * section, const char * name) = null;
	//Проверить наличие ключа в секции
	virtual bool IsKeyCreated(const char * section, const char * name, long index = 0) = null;

	//Получить значение ключа как строку
	virtual const char * GetString(const char * section, const char * name, const char * defValue = "", long index = 0) = null;
	//Получить все значения ключа как строки
	virtual void GetStrings(const char * section, const char * name, array<string> & value) = null;

	//Получить значение ключа как long
	virtual long GetLong(const char * section, const char * name, long defValue = 0, long index = 0) = null;
	//Получить все значения ключа как long
	virtual void GetLongs(const char * section, const char * name, array<long> & value) = null;

	//Получить значение ключа как float
	virtual float GetFloat(const char * section, const char * name, float defValue = 0.0, long index = 0) = null;
	//Получить все значения ключа как float
	virtual void GetFloats(const char * section, const char * name, array<float> & value) = null;

	//Получить значение ключа как double
	virtual double GetDouble(const char * section, const char * name, double defValue = 0.0, long index = 0) = null;
	//Получить все значения ключа как double
	virtual void GetDoubles(const char * section, const char * name, array<double> & value) = null;

};

//Редактируемый ini-файл
class IEditableIniFile : public IIniFile
{
protected:
	IEditableIniFile(){};
	virtual ~IEditableIniFile(){};

public:
	//Добавить секцию
	virtual void AddSection(const char * section) = null;
	//Удалить секцию с ключами
	virtual void DelSection(const char * section) = null;

	//Удалить ключ
	virtual void DelKey(const char * section, const char * name, long index = 0) = null;

	//Установить значение ключа как строку
	virtual void SetString(const char * section, const char * name, const char * value, long index = 0) = null;
	//Установить значение ключа как long
	virtual void SetLong(const char * section, const char * name, long value, long index = 0) = null;
	//Установить значение ключа как float
	virtual void SetFloat(const char * section, const char * name, float value, long index = 0) = null;
	//Установить значение ключа как double
	virtual void SetDouble(const char * section, const char * name, double value, long index = 0) = null;

	//Сохранить изменения на диск немедленно
	virtual void Flush() = null;
};


//Поиск файлов
class IFinder
{
protected:
	IFinder(){};
	virtual ~IFinder(){};

public:
	//Удалить объект
	virtual void Release() = null;

	//Начать поиск заново, возвращает количество найденых файлов
	virtual dword Reset() = null;

	//Получить количество найденых файлов
	virtual dword Count() const = null;

	//Получить полный путь с именем файла "С:\path\name.ext"
	virtual const char * FilePath(dword index) const = null;
	//Получить полный путь до файла "С:\path\"
	virtual const char * Path(dword index) const = null;
	//Получить имя файла "name.ext"
	virtual const char * Name(dword index) const = null;
	//Получить имя файла без расширения name
	virtual const char * Title(dword index) const = null;
	//Получить расширение файла "ext"
	virtual const char * Extension(dword index) const = null;
	//true если это  "." или ".."
	virtual bool IsDot(dword index) const = null;
	//true если это папка
	virtual bool IsFolder(dword index) const = null;
	//true если этот файл найден в зеркальной директории
	virtual bool IsMirror(dword index) const = null;
};

//Пак-файл
class IPackFile
{
protected:
	IPackFile(){};
	virtual ~IPackFile(){};

public:
	//Удалить объект
	virtual void Release() = null;
	//Получить количество файлов
	virtual dword Count() const = null;
	//Получить путь до файла внутри пака
	virtual const char * LocalPath(dword index) const = null;
	//Получить полный путь до файла
	virtual const char * FullPath(dword index) const = null;
	//Получить размер пак-файла
	virtual dword Size() const = null;
};

//Зеркальный путь
class IMirrorPath
{
protected:
	IMirrorPath(){};
	virtual ~IMirrorPath(){};

public:
	//Удалить объект, закончив отражать путь
	virtual void Release() = null;
	//Получить путь который отражается
	virtual const char * From() = null;
	//Получить путь на который отражается
	virtual const char * On() = null;

};

//Возвращаемый файловым сервисом буффер с прочитаным файлом
class ILoadBuffer
{
protected:
	ILoadBuffer(){};
	virtual ~ILoadBuffer(){};

public:
	//Удалить объект
	virtual void Release() = null;	
	//Получить указатель на буффер
	virtual const byte * Buffer() = null;
	//Получить размер буффера
	virtual dword Size() = null;
};


enum IFileService_OpenMode
{
	file_create_always,			//Создавать всегда новый файл, удалив существующий
	file_create_new,			//Создать только новый файл, созданный не открывать
	file_open_always,			//Открыть созданный файл, если нет то создать новый
	file_open_existing,			//Открыть созданый файл
	file_truncate_existing,		//Открыть созданый файл и обнулить размер
	file_open_existing_for_read	//Открыть созданый файл только для чтения
};

enum IFileService_DFOpenMode
{
	file_open_default = 0,			//Открывать как определенно в системе
	file_open_fromdisk = 1,			//Открывть только с диска, игнорируя пак-файлы
	file_open_frompack = 2,			//Открывть только из пак-файлов
	file_open_any = 3,				//Сначала пробывать открыть из пак-файла, если нет то с диска
};

enum IFileService_FinderFlags
{
	find_all_files = 0,			//Рекурсивный поиск файлов по реальным и зеркальным путям
	find_no_recursive = 1,		//Не делать рекурсивный поиск
	find_dots = 2,				//Включить в список искомых точки
	find_folders = 4,			//Включить в список искомых папки
	find_no_files_from_packs = 8,//Исключить из списока искомых файлы из загруженых пак-файлов
	find_no_mirror_files = 16,	//Исключить зеркальные пути
	find_inverse_order = 32,	//Дисковые файлы сначала будут идти те что глубже по рекурсии
	find_all_files_folder = find_all_files | find_folders,
	find_all_files_no_mirrors = find_all_files | find_no_mirror_files
};

enum IFileService_PackCompressMethod
{
	pack_cmpr_method_archive,	//Упаковка c сжатием
	pack_cmpr_method_store,		//Упаковка без сжатия
};

//Файловый сервис
class IFileService : public Service
{
protected:
	IFileService(){};
	virtual ~IFileService(){};

public:
	//Открыть файл данных (файл может быть архивирован)
	virtual IDataFile * OpenDataFile(const char * fileName, IFileService_DFOpenMode mode, const char * cppFileName, long cppFileLine) = null;

	//Открыть ini файл
	virtual IIniFile * OpenIniFile(const char * fileName, const char * cppFileName, long cppFileLine) = null;
	//Загрузить пак-файл в память
	virtual IPackFile * LoadPack(const char * fileName, const char * cppFileName, long cppFileLine) = null;

	//Итератор поиска файлов
	virtual	IFinder * CreateFinder(const char * findPath, const char * findMask, dword flags, const char * cppFileName, long cppFileLine) = null;

	//Отразить один путь на другой
	virtual IMirrorPath * CreateMirrorPath(const char * from, const char * on, const char * cppFileName, long cppFileLine) = null;

	//Прочитать файл в выделеную память с помощью OpenDataFile
	virtual ILoadBuffer * LoadData(const char * fileName, const char * cppFileName, long cppFileLine) = null;

	//Получить указатель на системный ini-файл
	virtual IIniFile * SystemIni() = null;

	//Получить полный нормализованный путь до файла, возвращает ссылку на result
	virtual string & BuildPath(const char * path, string & result) = null;

	//Заблокировать доступ к файлам
	virtual void LockFileAccess(bool isLock) = null;

#ifndef _XBOX
	//Открыть ini файл для редактирования
	virtual IEditableIniFile * OpenEditableIniFile(const char * fileName, IFileService_OpenMode mode, const char * cppFileName, long cppFileLine) = null;
	//Проверить существует ли файл
	virtual bool IsExist(const char * fileName, bool checkAsDataFile = false) = null;
	//Сохранить данные в файл с помощью OpenFile, перезаписав его
	virtual bool SaveData(const char * fileName, const void * data, dword size) = null;
	//Открыть файл с диска на чтение-запись
	virtual IFile * OpenFile(const char * fileName, IFileService_OpenMode mode, const char * cppFileName, long cppFileLine) = null;
	//Создать папку (возможно указать иерархический путь)
	virtual bool CreateFolder(const char * path) = null;
	//Скопировать файл
	virtual bool Copy(const char * from, const char * to) = null;
	//Переместить файл
	virtual bool Move(const char * from, const char * to) = null;
	//Переименовать файл
	virtual bool Rename(const char * from, const char * to) = null;
	//Удалить файл
	virtual bool Delete(const char * fileName) = null;
	//Установить путь для сохранения открываемых файлов данных на чтение
	virtual void SetDrainPath(const char * path) = null;
	//Создать пак-файл из файлов находящихся в заданной папке
	virtual bool BuildPack(const char * fileName, const char * folderPath, const char * filesMask, IFileService_PackCompressMethod method) = null;
#endif

};

#endif


