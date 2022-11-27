//Spirenkov Maxim, 2008

#include "CoreStorage.h"
#include "Core.h"

//=============================================================================================
//Items
//=============================================================================================
//Item
//---------------------------------------------------------------------------------------------

CoreStorage::Item::Item() : path(_FL_, 1)
{
}

//Удалить интерфейс на переменную
void CoreStorage::Item::Release()
{
	CoreStorage & storage = ((Core *)api)->GetCoreStorage();
	storage.RemoveItem(this);
	delete this;
}

//Проверить, существует ли переменная
bool CoreStorage::Item::IsValidate()
{
	if(folder)
	{
		return true;
	}
	//Ищем путь до объекта
	CoreStorage & storage = ((Core *)api)->GetCoreStorage();
	folder = storage.FindItem(path, type, false, index);
	if(folder)
	{
		return index >= 0;
	}
	return false;
}

//Получить доступ к переменной
bool CoreStorage::Item::IsAccess()
{
	if(folder)
	{
		return true;
	}
	//Ищем путь до объекта
	CoreStorage & storage = ((Core *)api)->GetCoreStorage();
	folder = storage.FindItem(path, type, true, index);
	if(folder)
	{
		return true;
	}
	return false;	
}

//Проставить результат
inline void CoreStorage::Item::SetResult(bool * isOk, bool value)
{
	if(isOk)
	{
		*isOk = value;
	}
}

//Маркировать что координаты потеряны
inline void CoreStorage::Item::Lost()
{
	folder = null;
	index = -1;
}

//Инициализировать итем
inline void CoreStorage::Item::Init(array<long> & _path, const char * _fileName, long _fileLine)
{
	folder = null;
	index = -1;
	cppFileName = _fileName;
	cppFileLine = _fileLine;
	path.Empty();
	path.AddElements(_path);
	for(long i = 0; i < _path; i++)
	{
		path[i] = _path[i];
	}
}

//Принудительное удаление
void CoreStorage::Item::ForceRelease()
{
	api->Trace("CoreStorage: variable dont release. cpp: %s, line %i", cppFileName, cppFileLine);
	Release();
}

//---------------------------------------------------------------------------------------------
//String
//---------------------------------------------------------------------------------------------

CoreStorage::String::String()
{
	type = ICoreStorageItem::t_string;
}

//Установить
void CoreStorage::String::Set(const char * value)
{
	if(Item::IsAccess())
	{
		folder->SetString(index, value);
	}
}

//Получить
const char * CoreStorage::String::Get(const char * def, bool * isOk)
{
	if(Item::IsValidate() && folder)
	{
		SetResult(isOk, true);
		return folder->GetString(index);
	}
	SetResult(isOk, false);
	return def;
}

//Получить указатель на storage
ICoreStorageString * CoreStorage::String::GetStorageItem()
{
	return this;
}

//---------------------------------------------------------------------------------------------
//Long
//---------------------------------------------------------------------------------------------

CoreStorage::Long::Long()
{
	type = ICoreStorageItem::t_long;
}

//Установить
void CoreStorage::Long::Set(long value)
{
	if(Item::IsAccess())
	{
		folder->SetLong(index, value);
	}
}

//Получить
long CoreStorage::Long::Get(long def, bool * isOk)
{
	if(Item::IsValidate() && folder)
	{
		SetResult(isOk, true);
		return folder->GetLong(index);
	}
	SetResult(isOk, false);
	return def;
}

//Получить указатель на storage
ICoreStorageLong * CoreStorage::Long::GetStorageItem()
{
	return this;
}

//---------------------------------------------------------------------------------------------
//Float
//---------------------------------------------------------------------------------------------

CoreStorage::Float::Float()
{
	type = ICoreStorageItem::t_float;
}

//Установить
void CoreStorage::Float::Set(float value)
{
	if(Item::IsAccess())
	{
		folder->SetFloat(index, value);
	}
}

//Получить
float CoreStorage::Float::Get(float def, bool * isOk)
{
	if(Item::IsValidate() && folder)
	{
		SetResult(isOk, true);
		return folder->GetFloat(index);
	}
	SetResult(isOk, false);
	return def;
}

//Получить указатель на storage
ICoreStorageFloat * CoreStorage::Float::GetStorageItem()
{
	return this;
}

//---------------------------------------------------------------------------------------------
//Folder
//---------------------------------------------------------------------------------------------

CoreStorage::Folder::Folder()
{
	type = ICoreStorageItem::t_folder;
}

//Установить элементу массива строковое поле
bool CoreStorage::Folder::SetString(const char * name, const char * value)
{
	//Получим путь до переменной
	long index = -1;
	CoreStorageFolder * fld = FindItem(name, ICoreStorageItem::t_string, true, index);
	//Устанавливаем если нашли
	if(fld)
	{
		fld->SetString(index, value);
		return true;
	}
	return false;
}

//Получить строку
const char * CoreStorage::Folder::GetString(const char * name, const char * def, bool * isOk)
{
	//Получим путь до переменной
	long index = -1;
	CoreStorageFolder * fld = FindItem(name, ICoreStorageItem::t_string, false, index);
	//Читаем если нашли
	if(fld)
	{
		SetResult(isOk, true);
		return fld->GetString(index);
	}
	SetResult(isOk, false);
	return def;
}

//Получить интерфейс на переменную типа string
ICoreStorageString * CoreStorage::Folder::GetItemString(const char * name, const char * fileName, long fileLine)
{
	//Получим путь до переменной
	long index = -1;
	CoreStorageFolder * fld = FindItem(name, ICoreStorageItem::t_string, true, index);
	if(fld)
	{
		CoreStorage::String * v = NEW CoreStorage::String();
		CoreStorage & storage = ((Core *)api)->GetCoreStorage();
		storage.AddItem(v, name, path, fileName, fileLine);
		return v->GetStorageItem();
	}
	return null;
}

//Установить целочисленное поле
bool CoreStorage::Folder::SetLong(const char * name, long value)
{
	//Получим путь до переменной
	long index = -1;
	CoreStorageFolder * fld = FindItem(name, ICoreStorageItem::t_long, true, index);
	//Устанавливаем если нашли
	if(fld)
	{
		fld->SetLong(index, value);
		return true;
	}
	return false;
}

//Получить целочисленное поле
long CoreStorage::Folder::GetLong(const char * name, long def, bool * isOk)
{
	//Получим путь до переменной
	long index = -1;
	CoreStorageFolder * fld = FindItem(name, ICoreStorageItem::t_long, false, index);
	//Читаем если нашли
	if(fld)
	{
		SetResult(isOk, true);
		return fld->GetLong(index);
	}
	SetResult(isOk, false);
	return def;
}

//Получить интерфейс на переменную типа long
ICoreStorageLong * CoreStorage::Folder::GetItemLong(const char * name, const char * fileName, long fileLine)
{
	CoreStorage::Long * v = NEW CoreStorage::Long();
	CoreStorage & storage = ((Core *)api)->GetCoreStorage();
	storage.AddItem(v, name, path, fileName, fileLine);
	return v->GetStorageItem();
}

//Установить численное поле
bool CoreStorage::Folder::SetFloat(const char * name, float value)
{
	//Получим путь до переменной
	long index = -1;
	CoreStorageFolder * fld = FindItem(name, ICoreStorageItem::t_float, true, index);
	//Устанавливаем если нашли
	if(fld)
	{
		fld->SetFloat(index, value);
		return true;
	}
	return false;
}

//Получить численное поле
float CoreStorage::Folder::GetFloat(const char * name, float def, bool * isOk)
{
	//Получим путь до переменной
	long index = -1;
	CoreStorageFolder * fld = FindItem(name, ICoreStorageItem::t_float, false, index);
	//Читаем если нашли
	if(fld)
	{
		SetResult(isOk, true);
		return fld->GetFloat(index);
	}
	SetResult(isOk, false);
	return def;
}

//Получить интерфейс на переменную типа long
ICoreStorageFloat * CoreStorage::Folder::GetItemFloat(const char * name, const char * fileName, long fileLine)
{
	CoreStorage::Float * v = NEW CoreStorage::Float();
	CoreStorage & storage = ((Core *)api)->GetCoreStorage();		
	storage.AddItem(v, name, path, fileName, fileLine);
	return v->GetStorageItem();
}

//Получить папку, если такой нет, то создать, если имя не занято
ICoreStorageFolder * CoreStorage::Folder::GetItemFolder(const char * name, const char * fileName, long fileLine)
{
	CoreStorage & storage = ((Core *)api)->GetCoreStorage();
	CoreStorage::Folder * v = NEW CoreStorage::Folder();
	storage.AddItem(v, name, path, fileName, fileLine);
	return v->GetStorageItem();
}

//Получить количество полей данной папки
dword CoreStorage::Folder::GetItemsCount()
{
	IsValidate();
	CoreStorageFolder * fld = ThisFolder();
	if(fld)
	{
		return fld->GetCount();
	}
	return 0;
}

//Получить имя поля по индекс
const char * CoreStorage::Folder::GetNameByIndex(long index)
{
	IsValidate();
	CoreStorageFolder * fld = ThisFolder();
	if(fld)
	{
		long id = fld->GetNameByIndex(index);
		CoreStorage & storage = ((Core *)api)->GetCoreStorage();
		return storage.GetName(id);
	}
	return null;
}

//Получить тип поля по индексу
ICoreStorageItem::Type CoreStorage::Folder::GetTypeByIndex(long index)
{
	IsValidate();
	CoreStorageFolder * fld = ThisFolder();
	if(fld)
	{
		return fld->GetTypeByIndex(index);
	}
	return ICoreStorageItem::t_error;
}

//Удалить содержимое папки
void CoreStorage::Folder::Delete(const char * name)
{
	long index = -1;
	CoreStorageFolder * fld = FindItem(name, ICoreStorageItem::t_error, false, index);
	if(fld)
	{
		CoreStorage & storage = ((Core *)api)->GetCoreStorage();
		storage.InvalidateItems(fld);
		fld->Delete(index);
	}
}

//Вывести содержимое в строку, (перевод строки \n)
void CoreStorage::Folder::Print(string & buffer, const char * name)
{
	long index = -1;
	CoreStorageFolder * fld = FindItem(name, ICoreStorageItem::t_error, false, index);
	if(fld)
	{
		fld->Print(0, buffer, index);
	}else{
		buffer += "State \"";
		buffer += name;
		buffer += "\" not found\n";
	}
}

//Сохранить состояние папки, добавив данные в data
bool CoreStorage::Folder::Save(const char * folderName, array<byte> & data)
{
	CoreStorage & storage = ((Core *)api)->GetCoreStorage();
	long index = -1;
	CoreStorageFolder * fld = FindItem(folderName, ICoreStorageItem::t_error, false, index);
	if(fld)
	{
		if(fld->GetTypeByIndex(index) == ICoreStorageItem::t_folder)
		{
			fld = fld->GetFolder(index);
			index = -1;
		}
		CoreStorageFolder::WriteStream stream(data);
		stream.WriteTable(storgeId, ARRSIZE(storgeId));
		stream.PinSizePosition();
		fld->Save(index, stream);
		stream.SaveSize();
		return true;
	}
	return false;
}

//Востоновить состояние папки из data. readPointer - текущая позиция чтения из data
bool CoreStorage::Folder::Load(const char * folderName, const void * data, dword size, dword & readPointer)
{
	CoreStorage & storage = ((Core *)api)->GetCoreStorage();
	long index = -1;
	CoreStorageFolder * fld = FindItem(folderName, ICoreStorageItem::t_folder, true, index);
	if(!fld)
	{
		return false;
	}
	//Получим конечную папку указанного пути
	if(index >= 0)
	{
		fld = fld->GetFolder(index);
		index = -1;
	}
	//Поток для чтения
	CoreStorageFolder::ReadStream stream(readPointer, data, size);
	//Проверим идентификатор
	byte fileId[ARRSIZE(storgeId)];
	if(!stream.ReadTable(fileId, ARRSIZE(storgeId)))
	{
		return false;
	}
	for(long i = 0; i < ARRSIZE(storgeId); i++)
	{
		if(fileId[i] != storgeId[i])
		{
			return false;
		}
	}
	//Сохраним размер для проверки
	if(!stream.PinSizePosition())
	{
		return false;
	}
	if(!fld->Load(stream))
	{
		return false;
	}
	if(!stream.CheckSize())
	{
		return false;
	}
	return true;
}

//Получить указатель на storage
ICoreStorageFolder * CoreStorage::Folder::GetStorageItem()
{
	return this;
}

//Получить координаты итема если он доступен
CoreStorageFolder * CoreStorage::Folder::FindItem(const char * name, ICoreStorageItem::Type type, bool isCreatePath, long & index)
{
	//Заполняем свой путь если обращение на запись
	if(isCreatePath)
	{
		if(!IsAccess())
		{
			return false;
		}
	}
	CoreStorage & storage = ((Core *)api)->GetCoreStorage();
	CoreStorageFolder * fld = ThisFolder();
	if(fld)
	{
		//Относительный путь до искомого объекта
		array<long> & path = storage.BuildPath(name, null, 0);
		//Ищем объект
		return fld->FindItem(path, type, isCreatePath, index);
	}
	//Полный путь до искомого объекта
	array<long> & path = storage.BuildPath(name, Item::path.GetBuffer(), Item::path.Size());
	//Ищем объект
	return storage.FindItem(path, type, isCreatePath, index);
}

//Получить текущую папку
inline CoreStorageFolder * CoreStorage::Folder::ThisFolder()
{
	if(folder)
	{
		if(index >= 0)
		{
			return folder->GetFolder(index);
		}else{
			return folder;
		}		
	}
	return null;
}


const byte CoreStorage::Folder::storgeId[4] = {'S', 't', 'g', '>'};


//Удалить интерфейс на переменную
void CoreStorage::RootFolder::Release()
{
	Assert(false);
}

//Проверить, существует ли переменная
bool CoreStorage::RootFolder::IsValidate()
{
	return true;
}

//=============================================================================================
//CoreStorage
//=============================================================================================

CoreStorage::CoreStorage() : names(_FL_, 16384),
								items(_FL_, 256),
								path(_FL_, 256),
								root(null, 0)
{
	for(long i = 0; i < ARRSIZE(entry); i++)
	{
		entry[i] = -1;
	}
	//Сохраним имя для рутовой группы
	long index = SaveName("");
	Assert(index == 0);
	path.Empty();
	rootFolder.Init(path, _FL_);
	rootFolderPtr = rootFolder.GetStorageItem();
}

CoreStorage::~CoreStorage()
{
	while(items)
	{
		items[0]->Release();
	}
}

//Удалить все переменные, сообщив об ошибках
void CoreStorage::ForceRelease()
{
	while(items)
	{
		items[0]->ForceRelease();
	}	
}

//Индексировать путь по имени
array<long> & CoreStorage::BuildPath(const char * namePath, long * rootPath, dword sizeOfRootPath)
{
	if(!namePath) namePath = "";
	path.Empty();
	//Если есть корневой путь, то припишем его в начало
	if(rootPath)
	{
		path.AddElements(sizeOfRootPath);
		for(dword i = 0; i < sizeOfRootPath; i++)
		{
			path[i] = rootPath[i];
		}
	}
	//Парсим строку, разделённую точками
	while(*namePath != 0)
	{
		//Выделяем имя
		buffer.Empty();
		while(true)
		{
			if(*namePath == '.' || *namePath == 0)
			{
				if(*namePath == '.') namePath++;
				break;
			}
			buffer += *namePath++;
		}
		//Добавляем в массив
		long id = SaveName(buffer.c_str());
		path.Add(id);
	}
	return path;
}

//Найти итем имея полный путь
CoreStorageFolder * CoreStorage::FindItem(const array<long> & p, ICoreStorageItem::Type type, bool isCreatePath, long & index)
{
	return root.FindItem(p, type, isCreatePath, index);
}

//Добавить итем в список созданных и инициализировать его
void CoreStorage::AddItem(Item * item, const char * namePath, array<long> & itemRoot, const char * fileName, long fileLine)
{	
	array<long> & fullPath = BuildPath(namePath, itemRoot.GetBuffer(), itemRoot.Size());
	item->Init(fullPath, fileName, fileLine);
	items.Add(item);
#ifndef STOP_DEBUG
	const char * name = "statistics.maximumCoreVariables";
	long count = rootFolderPtr->GetLong(name, 0);
	if(count < (long)items.Size())
	{
		rootFolderPtr->SetLong(name, items.Size());
	}
#endif
}

//Удалить итем из списка
void CoreStorage::RemoveItem(Item * item)
{
	items.Del(item);
}

//Промаркировать заведённые переменные что было удоление
void CoreStorage::InvalidateItems(CoreStorageFolder * folder)
{
	//Если часто используемых переменных будет много то надо 
	//маркировать только те что относяться к удаляемой папке
	for(long i = 0; i < items; i++)
	{
		items[i]->Lost();
	}
}

//Преобразовать имя в идентификатор
long CoreStorage::SaveName(const char * name)
{
	if(!name)
	{
		name = "";
	}
	//Для начала ищем среди добавленых
	dword len = 0;
	dword hash = string::HashNoCase(name, len);
	long eindex = hash & (ARRSIZE(entry) - 1);
	if(eindex >= 0)
	{
		long index = entry[eindex];
		while(index >= 0)
		{
			Name & nm = GetNameByIndex(index);
			if(nm.hash == hash && nm.len == len)
			{
				if(string::IsEqual(nm.name, name))
				{
					return index;
				}
			}
			index = nm.next;
		}
	}
	//Нет такой строки, добавляем новую
	dword nameIndex = names.Size();
	names.AddElements(sizeof(Name) + len);
	Name & n = GetNameByIndex(nameIndex);
	n.hash = hash;
	n.len = len;
	n.next = -1;
	memcpy(n.name, name, len + 1);
	//Вносим в хэшь-таблицу
	long index = entry[eindex];
	if(index >= 0)
	{		
		while(true)
		{
			Name & nm = GetNameByIndex(index);
			if(nm.next < 0)
			{
				nm.next = nameIndex;
				break;
			}
			index = nm.next;
		}

	}else{
		entry[eindex] = nameIndex;
	}
	return nameIndex;
}

//Получить имя по идентификатору
const char * CoreStorage::GetName(long id)
{
	Name & nm = GetNameByIndex(id);
	return nm.name;
}

//Получить описание имени по индексу
__forceinline CoreStorage::Name & CoreStorage::GetNameByIndex(long index)
{
	Assert(index + sizeof(Name) <= names.Size());
	Name & name = (Name &)names[index];
	return name;
}

