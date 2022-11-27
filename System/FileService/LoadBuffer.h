
#ifndef _LoadBuffer_h_
#define _LoadBuffer_h_


#include "..\..\common_h\FileService.h"


//Возвращаемый файловым сервисом буффер с прочитаным файлом
class LoadBuffer : public ILoadBuffer
{
public:
	LoadBuffer(const byte * _buffer, dword _size, const char * _cppFileName, long _cppFileLine);
	virtual ~LoadBuffer();

	//Удалить сообщив об ошибке
	void ErrorRelease();

public:
	//Удалить объект
	virtual void Release();
	//Получить указатель на буффер
	virtual const byte * Buffer();
	//Получить размер буффера
	virtual dword Size();

private:
	const byte * buffer;
	dword size;
#ifndef STOP_DEBUG
	const char * cppFileName;
	long cppFileLine;
#endif
};


#endif
