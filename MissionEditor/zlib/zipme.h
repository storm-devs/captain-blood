//Zip tool's files

#ifndef _ZIPME_H_
#define _ZIPME_H_

#include "..\..\common_h\FileService.h"

class ZipMe
{
public:
	ZipMe();
	~ZipMe();

	//Записать файл с архивацией, добавляеться последний символ в конец имени 'z'
	static bool WriteFile(const char * fileName, const void * data, dword dataSize, bool isCompress);
	
	//Прочитать файл в память, если последний яимвол 'z' то подрузамеваеться что файл с компрессией
	bool ReadFile(const char * fileName);

	//Получить буфер
	const byte * Data();
	//Размер буфера
	dword Size();

	//Заменить в загруженном буфере все \r на \n и убрать двойные \n
	void ClearText();

private:
	array<byte> data;
};


#endif