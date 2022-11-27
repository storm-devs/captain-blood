//============================================================================================
// Spirenkov Maxim, 2004
//============================================================================================
// AnxAnimation system
//============================================================================================
// AnxData
//============================================================================================


#include "AnxData.h"
#include "..\..\..\Common_h\data_swizzle.h"
#include "..\..\..\Common_h\FileService.h"

//============================================================================================

AnxData::AnxData()
{
	data = null;
	header = null;
}

AnxData::~AnxData()
{
	if(data)
	{
		delete data;
	}
}

//============================================================================================

//Загрузить данные
bool AnxData::LoadData(IDataFile * file)
{
	//Удаляем текущии ресурсы
	if(data)
	{
		delete data;
		data = null;
	}
	//Загружаем данные файла
	dword size = file->Size();
	if(size < sizeof(AnxHeaderId) + sizeof(AnxHeader))
	{
		return false;
	}
	data = NEW byte[size + 16];
	debugDataSize = size + 16;
	byte * buffer = AlignPtr(data);
	AssertAling16(buffer);
	if(file->Read(buffer, size) != size)
	{
		return false;
	}
	//Инициализируем данные
	//Проверяем идентификатор файла и версию
	AnxHeaderId * anxId = (AnxHeaderId *)buffer;
	if(size < sizeof(AnxHeaderId) + sizeof(AnxHeader) || !anxId->CheckId() || !anxId->CheckVer())
	{
		delete data;
		data = null;
		header = null;
		fileName.Empty();
		return false;
	}
	//Заголовок
	header = (AnxHeader *)(buffer + sizeof(AnxHeaderId));
	AssertAling16(header);
	return header->RestoreAnimation(buffer, size);
}

//Загрузить данные
void AnxData::SetData(const void * memData)
{
	//Сохраняем только ссылку на заголовок данных
	header = (AnxHeader *)((byte *)memData + sizeof(AnxHeaderId));
	AssertAling16(header);
}

//Установить имя
void AnxData::SetName(const char * name)
{
	fileName = name;
}

