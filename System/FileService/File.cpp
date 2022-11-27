

#include "File.h"
#include "FileService.h"

#ifndef _XBOX

File::File(const char * _cppFileName, long _cppFileLine)
{
	handle = INVALID_HANDLE_VALUE;
#ifndef STOP_DEBUG
	cppFileName = _cppFileName;
	cppFileLine = _cppFileLine;
#endif
}

File::~File()
{
	if(handle != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(handle);
		handle = INVALID_HANDLE_VALUE;
	}
}

//Закрыть файл сообщив об ошибке
void File::ErrorRelease()
{
#ifndef STOP_DEBUG
	FileService::object->Error("FileService error: IFile not release (file: %s, line %i)", cppFileName, cppFileLine);
#endif
	delete this;
}

//Закрыть файл
void File::Release()
{
	{
		SingleExClassThread(FileService::object)
		FileService::object->DeleteFile(this);
	}
	delete this;
}

//Открыть файл
bool File::Open(const char * realPath, IFileService_OpenMode mode)
{
	dword creation;
	dword access = GENERIC_WRITE | GENERIC_READ;
	dword share = FILE_SHARE_READ | FILE_SHARE_WRITE;
	switch(mode)
	{
	case file_create_always:
		creation = CREATE_ALWAYS;
		break;
	case file_create_new:
		creation = CREATE_NEW;
		break;
	case file_open_always:
		creation = OPEN_ALWAYS;
		break;
	case file_open_existing:
		creation = OPEN_EXISTING;
		break;
	case file_truncate_existing:
		creation = TRUNCATE_EXISTING;
		break;
	case file_open_existing_for_read:
		creation = OPEN_EXISTING;
		access = GENERIC_READ;
		//share = FILE_SHARE_READ;
		break;
	default:
		Assert(false);
	}
	handle = ::CreateFile(realPath, access, share, null, creation, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, null);
	if(handle == INVALID_HANDLE_VALUE)
	{
		const char * fileName = string::GetFileName(realPath);
		long len = fileName - realPath + 1;
		array<char> filePath(_FL_);
		filePath.AddElements(len);
		memcpy(filePath.GetBuffer(), realPath, len);
		filePath[len - 1] = 0;
		FileService::object->CreateFolder(filePath.GetBuffer());
		if(share & FILE_SHARE_WRITE)
		{
			dword attributes = ::GetFileAttributes(realPath);
			if(attributes != INVALID_FILE_ATTRIBUTES)
			{
				if(attributes & FILE_ATTRIBUTE_READONLY)
				{
					attributes &= ~FILE_ATTRIBUTE_READONLY;
					::SetFileAttributes(realPath, attributes);
				}
			}
		}
		handle = ::CreateFile(realPath, access, share, null, creation, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, null);
		if(handle == INVALID_HANDLE_VALUE)
		{
			dword errorCode = ::GetLastError();
			return false;
		}		
	}
	return true;
}

//Прочитать данные, возвращает количество прочитаных байт
dword File::Read(void * buffer, dword size)
{
	if(!size) return 0;
	Assert(buffer);
	Assert(handle != INVALID_HANDLE_VALUE);
	DWORD readBytes = 0;
	if(::ReadFile(handle, buffer, size, &readBytes, null))
	{
		return readBytes;
	}
	return 0;
}

//Записать данные, возвращает количество записаных байт
dword File::Write(const void * buffer, dword size)
{
	if(!size) return 0;
	Assert(buffer);
	Assert(handle != INVALID_HANDLE_VALUE);
	DWORD writeBytes = 0;
	if(::WriteFile(handle, buffer, size, &writeBytes, null))
	{
		return writeBytes;
	}
	return 0;
}

//Установить текущую позицию, возвращает установленую относительно начала
dword File::SetPos(dword distanceToMove, MoveMethod moveMethod)
{
	dword method = FILE_BEGIN;
	switch(moveMethod)
	{
	case from_current:
		method = FILE_CURRENT;
		break;
	case from_end:
		method = FILE_END;
		break;
	}
	dword newPos = ::SetFilePointer(handle, distanceToMove, null, method);
	if(newPos == INVALID_SET_FILE_POINTER)
	{
		return 0;
	}
	return newPos;
}

//Получить текущую позицию относительно начала
dword File::GetPos() const
{
	dword curPos = ::SetFilePointer(handle, 0, null, FILE_CURRENT);
	if(curPos == INVALID_SET_FILE_POINTER)
	{
		return 0;
	}
	return curPos;
}

//Получить размер файла
dword File::Size() const
{
	dword size = ::GetFileSize(handle, null);
	if(size == INVALID_FILE_SIZE)
	{
		return 0;
	}
	return size;
}

//Обрезать файл по текущей позиции
bool File::Truncate()
{
	bool res = ::SetEndOfFile(handle) != 0;
	return res;
}

#endif
