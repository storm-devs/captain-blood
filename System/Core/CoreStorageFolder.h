//============================================================================================
// Spirenkov Maxim, 2004, 2008
//============================================================================================
// CoreStorageFolder
//============================================================================================

#ifndef _CoreStorageFolder_h_
#define _CoreStorageFolder_h_

#include "..\..\common_h\core.h"


class CoreStorageFolder
{
	struct Item
	{
		long nameId;						//Идентификатор имени в общем массиве имён
		ICoreStorageItem::Type type;		//Тип итема
		union								//Возможные вариации итема в зависимости от type
		{
			dword vString;					//Индекс строки в массиве строк
			long vLong;						//long
			float vFloat;					//float
			CoreStorageFolder * vFolder;	//Указатель на дочернюю папку
		};
	};

//------------------------------------------------------------------------------------
public:

	class WriteStream
	{
	public:
		WriteStream(array<byte> & _data);

		void WriteByte(const byte & b);
		void WriteDWord(const dword & dw);
		void WriteFloat(const float & fl);
		void WriteString(const char * str);
		void WriteTable(const void * buffer, dword size);
		void PinSizePosition();
		void SaveSize();

	private:
		array<byte> & data;
		long offset;
	};

	class ReadStream
	{
	public:
		ReadStream(dword & _pointer, const void * _data, dword _size);

		bool ReadByte(byte & b);
		bool ReadDWord(dword & dw);
		bool ReadFloat(float & fl);
		bool ReadString(string & str);
		bool ReadTable(void * buffer, dword size);

		bool PinSizePosition();
		bool CheckSize();

	private:
		dword & pointer;
		const byte * data;
		dword size;
		long offset;
		dword offsetSize;
	};


//------------------------------------------------------------------------------------
public:
	CoreStorageFolder(CoreStorageFolder * p, long nId);
	~CoreStorageFolder();

public:
	//Получить по пути итем, если объекты не созданы, попытаться создать
	//Если встречается на месте нужного итем другого типа то возвращает индекс этого итем и папку null
	//Если в режиме isCreatePath == false передать тип t_error, то последний итем не проверяеться на тип
	CoreStorageFolder * FindItem(const array<long> & p, ICoreStorageItem::Type type, bool isCreatePath, long & index);
	//Заполнить полный путь
	void BuildFullPath(array<long> & p, long size);

public:
	//Установить элементу массива строковое поле
	void SetString(long index, const char * value);
	//Получить строку
	const char * GetString(long index);
	//Установить целочисленное поле
	void SetLong(long index, long value);
	//Получить целочисленное поле
	long GetLong(long index);
	//Установить численное поле
	void SetFloat(long index, float value);
	//Получить численное поле
	float GetFloat(long index);
	//Получить папку по индексу
	CoreStorageFolder * GetFolder(long index);

public:
	//Получить количество полей данной групы
	dword GetCount();
	//Получить идентификатор имени поля по индексу
	long GetNameByIndex(long index);
	//Получить тип поля по индексу
	ICoreStorageItem::Type GetTypeByIndex(long index);

public:
	//Вывести содержимое в строку, (перевод строки \n)
	void Print(long tab, string & buffer, long index);
	//Сохранить группу
	void Save(long index, WriteStream & stream);
	//Прочитать группу
	bool Load(ReadStream & stream);
	//Удалить все дочернии поля
	void Delete(long index);

//------------------------------------------------------------------------------------
protected:
	//Сохранить в поток поле
	void SaveItem(Item & item, WriteStream & stream);
	//Вывести элемент в буфер
	void PrintItemToBuffer(long tab, string & buffer, Item & item);

protected:
	array<Item> items;			//Элементы папки
	array<string> strings;		//Используемые строки
	CoreStorageFolder * parent;	//Родительская папка
	long nameId;				//Идентификатор имени
};

//------------------------------------------------------------------------------------

//Установить элементу массива строковое поле
inline void CoreStorageFolder::SetString(long index, const char * value)
{
	Item & item = items[index];
	Assert(item.type == ICoreStorageItem::t_string);
	strings[item.vString] = value;
}

//Получить строку
inline const char * CoreStorageFolder::GetString(long index)
{
	Item & item = items[index];
	Assert(item.type == ICoreStorageItem::t_string);
	return strings[item.vString].c_str();
}

//Установить целочисленное поле
inline void CoreStorageFolder::SetLong(long index, long value)
{
	Item & item = items[index];
	Assert(item.type == ICoreStorageItem::t_long);
	item.vLong = value;
}

//Получить целочисленное поле
inline long CoreStorageFolder::GetLong(long index)
{
	Item & item = items[index];
	Assert(item.type == ICoreStorageItem::t_long);
	return item.vLong;
}

//Установить численное поле
inline void CoreStorageFolder::SetFloat(long index, float value)
{
	Item & item = items[index];
	Assert(item.type == ICoreStorageItem::t_float);
	item.vFloat = value;
}

//Получить численное поле
inline float CoreStorageFolder::GetFloat(long index)
{
	Item & item = items[index];
	Assert(item.type == ICoreStorageItem::t_float);
	return item.vFloat;
}

//Получить папку по индексу
inline CoreStorageFolder * CoreStorageFolder::GetFolder(long index)
{
	Item & item = items[index];
	Assert(item.type == ICoreStorageItem::t_folder);
	return item.vFolder;
}

//Получить количество полей данной групы
inline dword CoreStorageFolder::GetCount()
{
	return items.Size();
}

//Получить идентификатор имени поля по индексу
inline long CoreStorageFolder::GetNameByIndex(long index)
{
	if(index < 0 || index >= items)
	{
		return -1;
	}
	return items[index].nameId;
}

//Получить тип поля по индексу
inline ICoreStorageItem::Type CoreStorageFolder::GetTypeByIndex(long index)
{
	if(index < 0 || index >= items)
	{
		return ICoreStorageItem::t_error;
	}
	return items[index].type;
}



#endif
