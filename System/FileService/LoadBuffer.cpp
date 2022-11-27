

#include "LoadBuffer.h"
#include "FileService.h"


LoadBuffer::LoadBuffer(const byte * _buffer, dword _size, const char * _cppFileName, long _cppFileLine)
{
	buffer = _buffer;
	size = _size;
#ifndef STOP_DEBUG
	cppFileName = _cppFileName;
	cppFileLine = _cppFileLine;
#endif
}

LoadBuffer::~LoadBuffer()
{
	if(buffer)
	{
		delete buffer;
		buffer = null;
	}
}

//Удалить сообщив об ошибке
void LoadBuffer::ErrorRelease()
{
#ifndef STOP_DEBUG
	api->Trace("FileService error: ILoadBuffer not released (file: %s, line: %i)", cppFileName, cppFileLine);
#endif
	delete this;
}

//Удалить объект
void LoadBuffer::Release()
{
	{
		SingleExClassThread(FileService::object)
		FileService::object->DeleteLoadBuffer(this);
	}
	delete this;
}

//Получить указатель на буффер
const byte * LoadBuffer::Buffer()
{
	return buffer;
}

//Получить размер буффера
dword LoadBuffer::Size()
{
	return size;
}