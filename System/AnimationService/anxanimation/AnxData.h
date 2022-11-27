//============================================================================================
// Spirenkov Maxim, 2004
//============================================================================================
// AnxAnimation system
//============================================================================================
// AnxData
//============================================================================================

#ifndef _AnxData_h_
#define _AnxData_h_

#include "AnxFileFormat.h"

class IDataFile;

class AnxData
{
//--------------------------------------------------------------------------------------------
public:
	AnxData();
	virtual ~AnxData();

public:
	//Получить заголовок анимации
	AnxHeader & GetHeader() const;
	//Получить имя анимации
	const char * GetName();

public:
	//Загрузить данные
	bool LoadData(IDataFile * file);
	//Установить имя
	void SetName(const char * name);

	//Установить загруженные данные
	void SetData(const void * memData);
	//Получить объем памяти заннимаемый данными
	dword GetDataSize();

public:
	//Установить пользовательские данные
	void SetUserData(dword data);
	//Получить пользовательские данные
	dword GetUserData();

//--------------------------------------------------------------------------------------------
private:
	AnxHeader * header;			//Заголовок анимационных данных
	byte * data;				//Данные анимации
	string fileName;			//Имя файла
	dword userData;				//Пользовательские данные
	dword debugDataSize;		//Размер алоцированных данных
};

//Получить заголовок анимации
__forceinline AnxHeader & AnxData::GetHeader() const
{
	return *header;
}

//Получить имя анимации
__forceinline const char * AnxData::GetName()
{
	return fileName.c_str();
}

//Установить пользовательские данные
__forceinline void AnxData::SetUserData(dword data)
{
	userData = data;
}

//Получить пользовательские данные
__forceinline dword AnxData::GetUserData()
{
	return userData;
}

//Получить объем памяти заннимаемый данными
__forceinline dword AnxData::GetDataSize()
{
	return debugDataSize;
}

#endif

