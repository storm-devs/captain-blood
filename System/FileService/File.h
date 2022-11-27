
#ifndef _File_h_
#define _File_h_

#ifndef _XBOX

#include "..\..\common_h\FileService.h"


class FileService;

//Файл для произвольного доступа
class File : public IFile
{
public:
	File(const char * _cppFileName, long _cppFileLine);
	virtual ~File();

	//Открыть файл
	bool Open(const char * realPath, IFileService_OpenMode mode);
	//Закрыть файл сообщив об ошибке
	void ErrorRelease();

public:	
	//Закрыть файл
	virtual void Release();
	//Прочитать данные, возвращает количество прочитаных байт
	virtual dword Read(void * buffer, dword size);
	//Записать данные, возвращает количество записаных байт
	virtual dword Write(const void * buffer, dword size);
	//Установить текущую позицию, возвращает установленую относительно начала
	virtual dword SetPos(dword distanceToMove, MoveMethod moveMethod);
	//Получить текущую позицию относительно начала
	virtual dword GetPos() const;
	//Получить размер файла
	virtual dword Size() const;
	//Обрезать файл по текущей позиции
	virtual bool Truncate();

private:
	HANDLE handle;
#ifndef STOP_DEBUG
	const char * cppFileName;
	long cppFileLine;
#endif
};

#endif

#endif

