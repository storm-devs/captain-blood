
#ifndef _DataFile_h_
#define _DataFile_h_

#include "..\..\common_h\FileService.h"
#include "Archivator.h"

class PackFile;

//Файл для чтения данных
class DataFileBase : public IDataFile
{
protected:
	DataFileBase(const char * _cppFileName, long _cppFileLine);
	virtual ~DataFileBase();

public:
	//Закрыть файл сообщив об ошибке
	void ErrorRelease();

public:
	//Закрыть файл
	virtual void Release();
	//Получить размер файла
	virtual dword Size() const;

protected:
	dword size;
#ifndef STOP_DEBUG
	const char * cppFileName;
	long cppFileLine;
#endif
};


//Файл для чтения данных
class DataFile : public DataFileBase
{
public:

	DataFile(const char * cppFileName, long cppFileLine);
	virtual ~DataFile();

	//Открыть файл
	bool Open(const char * realPath);

	//Прочитать данные, возвращает количество прочитаных байт
	//если поинтер null то читать во внутрений буффер
	//если режим noReadAccess, то память буфера не кэшируемая
	virtual dword Read(void * buffer, dword size, bool noReadAccess);

private:
	HANDLE handle;
};

//Файл для чтения данных из неупакованного файла
class DataFileMemUnpack : public DataFileBase
{
public:
	DataFileMemUnpack(const char * cppFileName, long cppFileLine, const byte * data, dword fileSize);
	virtual ~DataFileMemUnpack();

public:
	//Прочитать данные, возвращает количество прочитаных байт
	//если поинтер null то читать во внутрений буффер
	//если режим noReadAccess, то память буфера не кэшируемая
	virtual dword Read(void * buffer, dword size, bool noReadAccess);

private:
	const byte * ptr;
	const byte * stopPtr;
	ClassThread
};

//Файл для чтения данных из упакованного файла
class DataFileMemPack : public DataFileBase
{
public:
	DataFileMemPack(const char * cppFileName, long cppFileLine, const byte * mem, dword packedSize, dword realSize);
	virtual ~DataFileMemPack();

public:
	//Прочитать данные, возвращает количество прочитаных байт
	//если поинтер null то читать во внутрений буффер
	//если режим noReadAccess, то память буфера не кэшируемая
	virtual dword Read(void * buffer, dword size, bool noReadAccess);

private:
	const byte * sourceData;
	dword sourceSize;
	XStreamDecompressor * decompressor;
	ClassThread
};


#endif
