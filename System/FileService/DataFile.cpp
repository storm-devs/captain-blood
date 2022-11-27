

#include "DataFile.h"
#include "FileService.h"
#include "PackFile.h"

//---------------------------------------------------------------------------------------------
//DataFileBase
//---------------------------------------------------------------------------------------------

DataFileBase::DataFileBase(const char * _cppFileName, long _cppFileLine)
{
	size = 0;
#ifndef STOP_DEBUG
	cppFileName = _cppFileName;
	cppFileLine = _cppFileLine;
#endif
}

DataFileBase::~DataFileBase()
{
	
}

//Закрыть файл сообщив об ошибке
void DataFileBase::ErrorRelease()
{
#ifndef STOP_DEBUG
	FileService::object->Error("FileService error: IDataFile file not release (file: %s, line %i)", cppFileName, cppFileLine);
#endif
	delete this;
}

//Закрыть файл
void DataFileBase::Release()
{
	SingleExClassThread(FileService::object)
	FileService::object->DeleteDataFile(this);
	delete this;
}

//Получить размер файла
dword DataFileBase::Size() const
{
	return size;
}

//---------------------------------------------------------------------------------------------
//DataFile
//---------------------------------------------------------------------------------------------

DataFile::DataFile(const char * cppFileName, long cppFileLine) : DataFileBase(cppFileName, cppFileLine)
{
	handle = INVALID_HANDLE_VALUE;
}

DataFile::~DataFile()
{
	if(handle != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(handle);
		handle = INVALID_HANDLE_VALUE;
	}
}

//Открыть файл
bool DataFile::Open(const char * realPath)
{
	handle = ::CreateFile(realPath, GENERIC_READ, FILE_SHARE_READ, null, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, null);
	if(handle == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	size = ::GetFileSize(handle, null);
	if(size == INVALID_FILE_SIZE)
	{
		return false;
	}
	return true;
}

//Прочитать данные, возвращает количество прочитаных байт
//если поинтер null то читать во внутрений буффер
//если режим noReadAccess, то память буфера не кэшируемая
dword DataFile::Read(void * buffer, dword size, bool noReadAccess)
{
	if(!size) return 0;
	Assert(handle != INVALID_HANDLE_VALUE);
	if(!buffer)
	{
		dword curPos = ::SetFilePointer(handle, 0, null, FILE_CURRENT);
		dword newPos = ::SetFilePointer(handle, size, null, FILE_CURRENT);
		if(curPos == INVALID_SET_FILE_POINTER || newPos == INVALID_SET_FILE_POINTER)
		{
			return 0;
		}
		return newPos - curPos;
	}
	DWORD readBytes = 0;
	if(::ReadFile(handle, buffer, size, &readBytes, null))
	{
		return readBytes;
	}
	return 0;	
}

//---------------------------------------------------------------------------------------------
//DataFileMemUnpack
//---------------------------------------------------------------------------------------------

DataFileMemUnpack::DataFileMemUnpack(const char * cppFileName, long cppFileLine, const byte * data, dword fileSize) : DataFileBase(cppFileName, cppFileLine)
{
	ptr = data;
	stopPtr = data + fileSize;
	size = fileSize;
}

DataFileMemUnpack::~DataFileMemUnpack()
{
}

//Прочитать данные, возвращает количество прочитаных байт
//если поинтер null то читать во внутрений буффер
//если режим noReadAccess, то память буфера не кэшируемая
dword DataFileMemUnpack::Read(void * buffer, dword size, bool noReadAccess)
{
	SingleClassThread
	if(ptr + size > stopPtr) size = stopPtr - ptr;
	if(buffer)
	{		
		memcpy(buffer, ptr, size);
	}
	ptr += size;
	return size;
}

//---------------------------------------------------------------------------------------------
//DataFileMemPack
//---------------------------------------------------------------------------------------------


DataFileMemPack::DataFileMemPack(const char * cppFileName, long cppFileLine, const byte * mem, dword packedSize, dword realSize) : DataFileBase(cppFileName, cppFileLine)
{
	Assert(mem);
	Assert(packedSize < realSize);
	sourceData = mem;
	sourceSize = packedSize;
	size = realSize;
	decompressor = null;
}

DataFileMemPack::~DataFileMemPack()
{
	if(decompressor)
	{
		FileService::object->ReleaseStreamDecompressor(decompressor);
		decompressor = null;
	}
}

//Прочитать данные, возвращает количество прочитаных байт
//если поинтер null то читать во внутрений буффер
//если режим noReadAccess, то память буфера не кэшируемая
dword DataFileMemPack::Read(void * buffer, dword size, bool noReadAccess)
{
	SingleClassThread	
	if(!sourceData || !size)
	{
		return 0;
	}
	if(!decompressor)
	{
		if(size == this->size && !noReadAccess && sourceData)
		{
			//Распаковка содержимого за 1 вызов в кэшируемый буфер
			if(buffer)
			{
				if(!XCompressor::Decompress(sourceData, sourceSize, (byte *)buffer, size))
				{
					//Данные испорчены
					size = 0;
#ifndef STOP_DEBUG
					FileService::object->Error("FileService error: IDataFile file content damaged (archive in pack file) (file: %s, line %i)", cppFileName, cppFileLine);
#endif
				}
			}
			sourceData = null;
			return size;
		}
		//Компрессор для потоковой распаковки
		decompressor = FileService::object->GetStreamDecompressor();
		if(!decompressor->Prepare(sourceData, sourceSize, this->size))
		{
#ifndef STOP_DEBUG
			FileService::object->Error("FileService error: IDataFile file have got invalidate file size (archive in pack file) (file: %s, line %i)", cppFileName, cppFileLine);
#endif
			sourceData = null;
			return 0;
		}
	}
	Assert(decompressor);
	dword readBytes = 0;
	if(buffer)
	{
		if(!decompressor->Process((byte *)buffer, size, readBytes))
		{
#ifndef STOP_DEBUG
			FileService::object->Error("FileService error: IDataFile file content damaged (archive in pack file) (file: %s, line %i)", cppFileName, cppFileLine);
#endif
			sourceData = null;
			return 0;
		}
	}else{
		//Надо пропустить часть файла		
		while(size > 0)
		{
			static byte skipBuffer[16384];
			dword skipBytes = size;
			if(skipBytes > sizeof(skipBuffer)) skipBytes = sizeof(skipBuffer);
			dword rbytes = 0;
			if(!decompressor->Process((byte *)skipBuffer, skipBytes, rbytes))
			{
#ifndef STOP_DEBUG
				FileService::object->Error("FileService error: IDataFile file content damaged (archive in pack file) (file: %s, line %i)", cppFileName, cppFileLine);
#endif
				sourceData = null;
				return 0;
			}
			readBytes += rbytes;
			size -= skipBytes;
		}
	}
	return readBytes;
}

