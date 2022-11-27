//============================================================================================
// Spirenkov Maxim, 2004, 2008
//============================================================================================
// CoreStorageFolder
//============================================================================================

#include "CoreStorageFolder.h"
#include "CoreStorage.h"
#include "Core.h"
#include "..\..\common_h\data_swizzle.h"


//============================================================================================
//Streams
//============================================================================================
//WriteStream
//--------------------------------------------------------------------------------------------

CoreStorageFolder::WriteStream::WriteStream(array<byte> & _data) : data(_data)
{
	offset = -1;
}

void CoreStorageFolder::WriteStream::WriteByte(const byte & b)
{
	data.Add(b);
}

void CoreStorageFolder::WriteStream::WriteDWord(const dword & dw)
{
	long index = data;
	data.AddElements(sizeof(dword));
	core_unaligned dword & v = (core_unaligned dword &)data[index];
	v = SwizzleDWord(dw);
}

void CoreStorageFolder::WriteStream::WriteFloat(const float & fl)
{
	long index = data;
	data.AddElements(sizeof(float));
	core_unaligned float & v = (core_unaligned float &)data[index];
	v = SwizzleFloat(fl);	
}

void CoreStorageFolder::WriteStream::WriteString(const char * str)
{
	if(!str) str = "";
	for(long s = 0; str[s]; s++); s++;
	long index = data;
	data.AddElements(s);
	memcpy(&data[index], str, s);
}

void CoreStorageFolder::WriteStream::WriteTable(const void * buffer, dword size)
{
	if(size > 0)
	{
		long index = data;
		data.AddElements(size);
		memcpy(&data[index], buffer, size);
	}
}

void CoreStorageFolder::WriteStream::PinSizePosition()
{
	offset = data;
	data.AddElements(sizeof(dword));
}

void CoreStorageFolder::WriteStream::SaveSize()
{
	Assert(offset > 0);
	long size = data - offset;
	Assert(size > 0);
	core_unaligned dword & v = (core_unaligned dword &)data[offset];
	v = SwizzleDWord((dword)size);
}


//--------------------------------------------------------------------------------------------
//ReadStream
//--------------------------------------------------------------------------------------------

CoreStorageFolder::ReadStream::ReadStream(dword & _pointer, const void * _data, dword _size) : pointer(_pointer)
{
	data = (const byte *)_data;
	size = _size;
	offset = -1;
	offsetSize = -1;
}

bool CoreStorageFolder::ReadStream::ReadByte(byte & b)
{
	if(pointer + sizeof(byte) > size)
	{
		return false;
	}
	b = data[pointer];
	pointer += sizeof(byte);
	return true;
}

bool CoreStorageFolder::ReadStream::ReadDWord(dword & dw)
{
	if(pointer + sizeof(dword) > size)
	{
		return false;
	}
	dw = SwizzleDWord((core_unaligned dword &)data[pointer]);
	pointer += sizeof(dword);
	return true;
}

bool CoreStorageFolder::ReadStream::ReadFloat(float & fl)
{
	if(pointer + sizeof(float) > size)
	{
		return false;
	}
	fl = SwizzleFloat((core_unaligned float &)data[pointer]);
	pointer += sizeof(float);
	return true;
}

bool CoreStorageFolder::ReadStream::ReadString(string & str)
{	
	for(str.Empty(); pointer < size; pointer++)
	{
		char c = data[pointer];
		if(!c)
		{
			pointer++;
			return true;
		}
		str += c;
	}
	str.Empty();
	return false;
}

bool CoreStorageFolder::ReadStream::ReadTable(void * buffer, dword size)
{
	if(pointer + size > this->size)
	{
		return false;
	}
	if(size > 0)
	{
		memcpy(buffer, &data[pointer], size);
	}
	pointer += size;
	return true;
}

bool CoreStorageFolder::ReadStream::PinSizePosition()
{
	offset = pointer;
	return ReadDWord(offsetSize);
}

bool CoreStorageFolder::ReadStream::CheckSize()
{
	long s = pointer - offset;
	if(s < 0)
	{
		return false;
	}
	if(s != offsetSize)
	{
		return false;
	}
	return true;
}


//============================================================================================
//CoreStorageFolder
//============================================================================================

CoreStorageFolder::CoreStorageFolder(CoreStorageFolder * p, long nId) : items(_FL_),
																		strings(_FL_)
{
	parent = p;
	nameId = nId;
}

CoreStorageFolder::~CoreStorageFolder()
{
	for(long i = 0; i < items; i++)
	{
		if(items[i].type == ICoreStorageItem::t_folder)
		{
			delete items[i].vFolder;
		}
	}
	items.Empty();
}

//Получить по пути итем, если объекты не созданы, попытаться создать
//Если встречается на месте нужного итем другого типа то возвращает индекс этого итем и папку null
CoreStorageFolder * CoreStorageFolder::FindItem(const array<long> & p, ICoreStorageItem::Type type, bool isCreatePath, long & index)
{
	CoreStorageFolder * folder = this;
	for(long i = 0; i < p; i++)
	{
		//Ищем поле с заданным идентификатором
		long id = p[i];
		Item * its = folder->items.GetBuffer();
		long count = folder->items.Size();
		index = -1;
		for(long n = 0; n < count; n++)
		{
			if(its[n].nameId == id)
			{
				index = n;
				break;
			}
		}
		if(index >= 0)
		{
			//Элемент с таким идентификатором найден, проверяем тип
			Item & item = folder->items[index];
			if(i >= p - 1)
			{
				//Последний элемент
				if(type != ICoreStorageItem::t_error)
				{
					//Требуеться проверить тип переменной
					if(item.type != type)
					{
						return null;
					}
				}
				return folder;
			}
			//Не последний элемент должен быть папкой
			if(item.type != ICoreStorageItem::t_folder)
			{
				return null;
			}
			folder = item.vFolder;
			Assert(folder);
		}else{
			//Нету такого поля если в режиме создания то создаём папку
			if(!isCreatePath)
			{
				return null;
			}
			index = folder->items.Add();
			Item & item = folder->items[index];
			item.nameId = id;			
			if(i >= p - 1)
			{
				//Последний элемент
				item.type = type;
				switch(type)
				{
				case ICoreStorageItem::t_string:
					item.vString = folder->strings.Add();
					break;
				case ICoreStorageItem::t_long:
					item.vLong = 0;
					break;
				case ICoreStorageItem::t_float:
					item.vFloat = 0.0f;
					break;
				case ICoreStorageItem::t_folder:
					item.vFolder = NEW CoreStorageFolder(folder, id);
					break;
				default:
					Assert(false);
				}
				return folder;
			}			
			//Папка
			item.type = ICoreStorageItem::t_folder;
			folder = item.vFolder = NEW CoreStorageFolder(folder, id);
		}
	}
	//Указана текущая папка
	index = -1;
	if(type != ICoreStorageItem::t_error)
	{
		if(type != ICoreStorageItem::t_folder)
		{
			return null;
		}
	}
	return this;	
}

//Заполнить полный путь до папки
void CoreStorageFolder::BuildFullPath(array<long> & p, long size)
{
	if(parent)
	{
		parent->BuildFullPath(p, size + 1);
		p.Add(nameId);
	}else{
		p.Empty();
		p.Reserve(size);
	}
}

//Вывести содержимое в строку, (перевод строки \n)
void CoreStorageFolder::Print(long tab, string & buffer, long index)
{
	if(index < 0)
	{
		buffer.Reserve(items.Size()*(tab + 1024));
		for(long i = 0; i < items; i++)
		{
			PrintItemToBuffer(tab, buffer, items[i]);
		}
	}else{
		buffer.Reserve(tab + 1024);
		PrintItemToBuffer(tab, buffer, items[index]);
	}
}

//Сохранить группу
void CoreStorageFolder::Save(long index, WriteStream & stream)
{
	if(index < 0)
	{
		//Надо сохранить всю папку
		stream.WriteDWord(items.Size());		
		for(long i = 0; i < items; i++)
		{
			SaveItem(items[i], stream);
		}
	}else{
		//Надо сохранить отдельный элемент
		stream.WriteDWord(1);
		SaveItem(items[index], stream);
	}
}

//Прочитать группу
bool CoreStorageFolder::Load(ReadStream & stream)
{
	CoreStorage & storage = ((Core *)api)->GetCoreStorage();
	//Читаем количество полей
	dword count = 0;
	if(!stream.ReadDWord(count))
	{
		return false;
	}
	items.Reserve(items.Size() + count);
	//Читаем поля
	string name;
	for(dword i = 0; i < count; i++)
	{
		byte type;
		if(!stream.ReadByte(type))
		{
			return false;
		}
		if(!stream.ReadString(name))
		{
			return false;
		}
		Item item;
		item.nameId = storage.SaveName(name.c_str());
		//Проверяем такой среди добавленых
		for(long j = 0; j < items; j++)
		{
			Item & it = items[j];
			if(it.nameId == item.nameId)
			{
				//Опа, есть такое
				if(it.type != type)
				{
					//Жёпа, нельзя перезаписать значение
					return false;
				}
				break;
			}
		}		
		switch(type)
		{
		case ICoreStorageItem::t_string:
			if(j >= items)
			{
				item.type = ICoreStorageItem::t_string;
				item.vString = strings.Add();
				if(!stream.ReadString(strings[item.vString]))
				{
					strings.DelIndex(item.vString);
					return false;
				}
				items.Add(item);
			}else{
				if(!stream.ReadString(strings[items[j].vString]))
				{
					return false;
				}
			}
			break;
		case ICoreStorageItem::t_long:
			if(j >= items)
			{
				item.type = ICoreStorageItem::t_long;
				if(!stream.ReadDWord((dword &)item.vLong))
				{
					return false;
				}
				items.Add(item);
			}else{
				if(!stream.ReadDWord((dword &)items[j].vLong))
				{
					return false;
				}
			}
			break;
		case ICoreStorageItem::t_float:
			if(j >= items)
			{
				item.type = ICoreStorageItem::t_float;
				if(!stream.ReadFloat(item.vFloat))
				{
					return false;
				}
				items.Add(item);
			}else{
				if(!stream.ReadFloat(items[j].vFloat))
				{
					return false;
				}
			}
			break;
		case ICoreStorageItem::t_folder:
			if(j >= items)
			{
				item.type = ICoreStorageItem::t_folder;
				item.vFolder = NEW CoreStorageFolder(this, item.nameId);
				if(!item.vFolder->Load(stream))
				{
					delete item.vFolder;
					return false;
				}
				items.Add(item);
			}else{
				if(!items[j].vFolder->Load(stream))
				{
					return false;
				}
			}
			break;
		default:
			return false;
		}
	}
	return true;
}

//Удалить все дочернии поля
void CoreStorageFolder::Delete(long index)
{
	if(index >= 0)
	{
		if(items[index].type == ICoreStorageItem::t_folder)
		{
			delete items[index].vFolder;
		}
		items.DelIndex(index);
	}else{
		for(long i = 0; i < items; i++)
		{
			if(items[i].type == ICoreStorageItem::t_folder)
			{
				delete items[i].vFolder;
			}
		}
		items.Empty();
	}
}

//Сохранить в поток поле
void CoreStorageFolder::SaveItem(Item & item, WriteStream & stream)
{
	//Тип
	stream.WriteByte(item.type);
	//Имя
	CoreStorage & storage = ((Core *)api)->GetCoreStorage();
	const char * name = storage.GetName(item.nameId);
	stream.WriteString(name);
	//Данные
	switch(item.type)
	{
	case ICoreStorageItem::t_string:
		stream.WriteString(strings[item.vString].c_str());
		break;
	case ICoreStorageItem::t_long:
		stream.WriteDWord((dword)item.vLong);
		break;
	case ICoreStorageItem::t_float:
		stream.WriteFloat(item.vFloat);
		break;
	case ICoreStorageItem::t_folder:
		item.vFolder->Save(-1, stream);
		break;
	default:
		Assert(false);
	}
}

//Вывести элемент в буфер
void CoreStorageFolder::PrintItemToBuffer(long tab, string & buffer, Item & item)
{
	char tmp[1024];
	for(long i = 0; i < tab; i++)
	{
		buffer += ' ';
	}	
	CoreStorage & storage = ((Core *)api)->GetCoreStorage();
	const char * name = storage.GetName(item.nameId);
	switch(item.type)
	{
	case ICoreStorageItem::t_string:
		crt_snprintf(tmp, sizeof(tmp) - 1, "string %s = %s\n", name, strings[item.vString].c_str());
		buffer += tmp;
		break;
	case ICoreStorageItem::t_long:
		crt_snprintf(tmp, sizeof(tmp) - 1, "long %s = %i\n", name, item.vLong);
		buffer += tmp;
		break;
	case ICoreStorageItem::t_float:
		crt_snprintf(tmp, sizeof(tmp) - 1, "float %s = %f\n", name, item.vFloat);
		buffer += tmp;
		break;
	case ICoreStorageItem::t_folder:
		crt_snprintf(tmp, sizeof(tmp) - 1, "[%s]\n", name);
		buffer += tmp;
		item.vFolder->Print(tab + 4, buffer, -1);
		break;
	}
}

