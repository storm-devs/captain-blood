
#ifndef _Archivator_h_
#define _Archivator_h_

#include "..\..\common_h\FileService.h"
#include "..\..\common_h\Compression\XCompressor.h"


class PackArchivator
{
	struct FileInfo
	{
		FileInfo();
		~FileInfo();

		string name;
		dword hash;
		byte * data;
		dword size;
		dword offset;
		dword compressedSize;
	};

public:
	enum Method
	{
		m_archive,
		m_store,
	};

public:
	PackArchivator();
	~PackArchivator();

	//Добавить файл
	void AddFile(const char * path, void * buffer, dword size);
	//Процес подготовки данных
	void Process(Method method);
	
	//Получить количество файлов
	dword GetFilesCount();
	//Получить общий размер таблицы имён
	dword GetNamesSize();
	//Получить путь архивированого файла
	const char * GetFileName(dword index, dword & len, dword & hash);
	//Получить смещение от начала массива данных
	dword GetFileOffset(dword index);
	//Получить размер несжатого файла
	dword GetFileSize(dword index);
	//Получить размер сжатого файла
	dword GetFileCompressedSize(dword index);
	
	//Получить буфер с данными
	const void * GetDataBuffer();
	//Получить размер данных
	dword GetDataSize();

private:
	array<FileInfo> files;
	array<byte> packedData;
	XCompressor compressor;
};


#endif
