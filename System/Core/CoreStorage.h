//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// CoreStorage
//============================================================================================

#include "..\..\common_h\core.h"
#include "CoreStorageFolder.h"

#ifndef _CoreStorage_h_
#define _CoreStorage_h_

class CoreStorage
{
	struct Name
	{
		dword hash;		//Хэшь-значение имени
		dword len;		//Длинна строки
		long next;		//Индекс следующей записи в таблице имён с одинаковой маской
		char name[1];	//Имя
	};

public:

//Запретим сообщение о том что будет доминировать ветка из Item
#pragma warning(push)
#pragma warning(disable : 4250)

	class Item : public virtual ICoreStorageItem
	{
	protected:
		Item();

	public:
		//Удалить интерфейс на переменную
		virtual void Release();
		//Проверить, существует ли переменная
		virtual bool IsValidate();

	public:
		//Маркировать что координаты потеряны
		void Lost();
		//Инициализировать итем
		void Init(array<long> & _path, const char * _fileName, long _fileLine);
		//Принудительное удаление
		void ForceRelease();
	
	protected:
		//Получить доступ к переменной
		bool IsAccess();
		//Проставить результат
		static void SetResult(bool * isOk, bool value);

	protected:
		CoreStorageFolder * folder;	//Указатель на папку в которой находиться поле
		long index;					//Индекc поля
		array<long> path;			//Путь до итема
		Type type;					//Тип итема
		const char * cppFileName;	//Имя файла где была создана переменная
		long cppFileLine;			//Номер строки в файле где была создана переменная
	};

	class String : public Item, public ICoreStorageString
	{
	public:
		String();
		//Установить
		virtual void Set(const char * value);
		//Получить
		virtual const char * Get(const char * def, bool * isOk);
		//Получить указатель на storage
		virtual ICoreStorageString * GetStorageItem();
	};

	class Long : public Item, public ICoreStorageLong
	{
	public:
		Long();
		//Установить
		virtual void Set(long value);
		//Получить
		virtual long Get(long def, bool * isOk);
		//Получить указатель на storage
		virtual ICoreStorageLong * GetStorageItem();
	};

	class Float : public Item, public ICoreStorageFloat
	{
	public:
		Float();
		//Установить
		virtual void Set(float value);
		//Получить
		virtual float Get(float def, bool * isOk);
		//Получить указатель на storage
		virtual ICoreStorageFloat * GetStorageItem();
	};

	class Folder : public Item, public ICoreStorageFolder
	{
	public:
		Folder();
		//Установить элементу массива строковое поле
		virtual bool SetString(const char * name, const char * value);
		//Получить строку
		virtual const char * GetString(const char * name, const char * def, bool * isOk);
		//Получить интерфейс на переменную типа string, возвращает всегра не null
		ICoreStorageString * GetItemString(const char * name, const char * fileName, long fileLine);
		//Установить целочисленное поле
		virtual bool SetLong(const char * name, long value);
		//Получить целочисленное поле
		virtual long GetLong(const char * name, long def, bool * isOk);
		//Получить интерфейс на переменную типа long, возвращает всегра не null
		ICoreStorageLong * GetItemLong(const char * name, const char * fileName, long fileLine);
		//Установить численное поле
		virtual bool SetFloat(const char * name, float value);
		//Получить численное поле
		virtual float GetFloat(const char * name, float def, bool * isOk);
		//Получить интерфейс на переменную типа long, возвращает всегра не null
		ICoreStorageFloat * GetItemFloat(const char * name, const char * fileName, long fileLine);

		//Получить папку, если такой нет, то создать, если имя не занято, возвращает всегра не null
		virtual ICoreStorageFolder * GetItemFolder(const char * name, const char * fileName, long fileLine);

		//Получить количество полей данной папки
		virtual dword GetItemsCount();
		//Получить имя поля по индексу
		virtual const char * GetNameByIndex(long index);
		//Получить тип поля по индексу
		virtual Type GetTypeByIndex(long index);

		//Удалить содержимое папки
		virtual void Delete(const char * name);

		//Вывести содержимое в строку, (перевод строки \n)
		virtual void Print(string & buffer, const char * name);

		//Сохранить состояние папки, добавив данные в data
		virtual bool Save(const char * folderName, array<byte> & data);
		//Востоновить состояние папки из data. readPointer - текущая позиция чтения из data
		virtual bool Load(const char * folderName, const void * data, dword size, dword & readPointer);

		//Получить указатель на storage
		virtual ICoreStorageFolder * GetStorageItem();

	private:
		//Получить координаты итема если он доступен
		CoreStorageFolder * FindItem(const char * name, ICoreStorageItem::Type type, bool isCreatePath, long & index);
		//Получить текущую папку
		CoreStorageFolder * ThisFolder();
	
	private:
		static const byte storgeId[4];
	};

	class RootFolder : public Folder
	{
	public:
		//Удалить интерфейс на переменную
		virtual void Release();
		//Проверить, существует ли переменная
		virtual bool IsValidate();
	};

#pragma warning(pop)


public:
	CoreStorage();
	virtual ~CoreStorage();

public:
	//Получить корневую папку
	ICoreStorageFolder & Root();
	//Удалить все переменные, сообщив об ошибках
	void ForceRelease();

public:
	//Индексировать путь по имени достраивая от заданного
	array<long> & BuildPath(const char * namePath, long * rootPath, dword sizeOfRootPath);
	//Найти итем имея полный путь
	CoreStorageFolder * FindItem(const array<long> & p, ICoreStorageItem::Type type, bool isCreatePath, long & index);
	//Добавить итем в список созданных и инициализировать его
	void AddItem(Item * item, const char * namePath, array<long> & itemRoot, const char * fileName, long fileLine);
	//Удалить итем из списка
	void RemoveItem(Item * item);
	//Промаркировать заведённые переменные что было удоление
	void InvalidateItems(CoreStorageFolder * folder);

public:
	//Преобразовать имя в идентификатор
	long SaveName(const char * name);
	//Получить имя по идентификатору
	const char * GetName(long id);

private:
	//Получить описание имени по индексу
	Name & GetNameByIndex(long index);

private:
	//Корень базы
	CoreStorageFolder root;
	RootFolder rootFolder;
	ICoreStorageFolder * rootFolderPtr;
	//База имён
	array<char> names;
	long entry[1024];
	//Заведённые переменные	
	array<Item *> items;
	//Индексированный путь
	string buffer;
	array<long> path;
};

//Получить корневую папку
inline ICoreStorageFolder & CoreStorage::Root()
{
	return *rootFolderPtr;
}

#endif
