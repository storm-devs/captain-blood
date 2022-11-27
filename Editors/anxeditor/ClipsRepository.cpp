//Спиренков Максим, 2009

#include "ClipsRepository.h"



ClipsRepository::ClipsRepository() : ants(_FL_, 1024),
									translateTable(_FL_, 1024)
{

}

ClipsRepository::~ClipsRepository()
{
	Release();
}


//Добавить компресированный ант на хранение
long ClipsRepository::Add(byte * compressedAnt, dword size)
{
	//Собираем чексуму
	dword checkSum = 0;
	for(dword i = 0; i < size; i++)
	{
		checkSum += compressedAnt[i];
	}
	//Ищим такое же среди добавленых
	for(dword i = 0; i < ants.Size(); i++)
	{
		AntArchive * aa = ants[i];
		//Предварительные проверки
		if(!aa || aa->size != size || aa->checkSum != checkSum)
		{
			continue;
		}
		//Побайтовое сравнение
		for(dword j = 0; j < size; j++)
		{
			if(aa->data[j] != compressedAnt[j])
			{
				break;
			}
		}
		if(j == size)
		{
			aa->refCount++;
			return i;
		}
	}
	//Подготавливаем данные анта
	AntArchive * nant = (AntArchive *)NEW byte[size + sizeof(AntArchive) - 1];
	nant->size = size;
	nant->checkSum = checkSum;
	nant->refCount = 1;
	for(dword i = 0; i < size; i++)
	{
		nant->data[i] = compressedAnt[i];
	}
	//Добавляем новый ант в хранилище
	for(dword i = 0; i < ants.Size(); i++)
	{
		if(!ants[i])
		{
			ants[i] = nant;
			return i;
		}
	}
	long newIndex = ants.Add(nant);
	return newIndex;
}

//Удалить ант из хранилища
void ClipsRepository::Del(long id)
{
	if(id < 0)
	{
		return;
	}
	AntArchive * aa = ants[id];
	Assert(aa);
	aa->refCount--;
	Assert(aa->refCount >= 0);
	if(aa->refCount == 0)
	{
		delete ants[id];
		ants[id] = null;
	}
}

//Получить ант для использования
byte * ClipsRepository::Get(long id, dword & size)
{
	if(id < 0)
	{
		size = 0;
		return null;
	}
	AntArchive * aa = ants[id];
	Assert(aa);
	size = aa->size;
	return aa->data;
}

//Записать данные в поток
void ClipsRepository::Write(AnxStream & stream)
{
	stream.EnterWriteSection("Clips repository -> ");
	stream.WriteVersion(1);
	long size = ants.Size();
	stream << size;
	for(dword i = 0; i < ants.Size(); i++)
	{
		AntArchive * aa = ants[i];
		if(aa)
		{
			size = aa->size + sizeof(AntArchive) - 1;
			stream << size;
			stream.Write(aa, size);
		}else{
			size = 0;
			stream << size;
		}		
	}
	stream.ExitWriteSection();
}

//Прочитать данные из потока
void ClipsRepository::Read(AnxStream & stream)
{
	Release();
	string sid;
	stream.EnterReadSection(sid);
#ifdef ANX_STREAM_CATCH
	try
	{
#endif
		if(sid != "Clips repository -> ") throw "Invalidate repository section id";
		dword ver = stream.ReadVersion();
		if(ver != 1) throw "Invalidate version repository section";
		long count = 0;
		stream >> count;
		ants.AddElements(count);
		for(dword i = 0; i < ants.Size(); i++)
		{
			ants[i] = null;
		}
		for(dword i = 0; i < ants.Size(); i++)
		{
			long size = 0;
			stream >> size;
			if(size > 0)
			{
				void * ptr = null;
				dword readSize = 0;
				stream.Read(ptr, readSize);
				ants[i] = null;
				if(readSize != size)
				{
					throw "Invalidate content";
				}
				ants[i] = (AntArchive *)ptr;
			}
		}
#ifdef ANX_STREAM_CATCH
	}catch(const char * err){
		api->Trace("AnxEditor repository section: IORead graph error: %s", err ? err : "<no info>");
		throw "Can't load repository section";
	}catch(...){
		api->Trace("AnxEditor repository section: IORead graph unknow error.");
		throw "Can't load repository section";
	}
#endif
	stream.ExitReadSection();
}

//Импортировать данные из потока
void ClipsRepository::Import(AnxStream & stream)
{
	string sid;
	stream.EnterReadSection(sid);
#ifdef ANX_STREAM_CATCH
	try
	{
#endif
		if(sid != "Clips repository -> ") throw "Invalidate repository section id";
		dword ver = stream.ReadVersion();
		if(ver != 1) throw "Invalidate version repository section";
		long count = 0;
		stream >> count;
		for(long i = 0; i < count; i++)
		{
			//Добавляем запись в таблицу трансляции
			Translate & t = translateTable[translateTable.Add()];
			t.oldIndex = i;
			t.newIndex = -1;
			//Загружаем ант
			long size = 0;
			stream >> size;
			if(size > 0)
			{
				void * ptr = null;
				dword readSize = 0;
				stream.Read(ptr, readSize);
				if(readSize != size)
				{
					throw "Invalidate content";
				}
				AntArchive * aa = (AntArchive *)ptr;
				t.newIndex = Add(aa->data, aa->size);
			}
		}
#ifdef ANX_STREAM_CATCH
	}catch(const char * err){
		api->Trace("AnxEditor repository section: IORead graph error: %s", err ? err : "<no info>");
		throw "Can't load repository section";
	}catch(...){
		api->Trace("AnxEditor repository section: IORead graph unknow error.");
		throw "Can't load repository section";
	}
#endif
	stream.ExitReadSection();
}

//Удалить всё
void ClipsRepository::Release()
{
	translateTable.Empty();
	for(dword i = 0; i < ants.Size(); i++)
	{
		if(ants[i])
		{
			delete ants[i];
			ants[i] = null;
		}
	}
	ants.Empty();
}


//Подготовить таблицу индексов
void ClipsRepository::BeginTranslates()
{
	translateTable.Empty();
}

//Закончить трансляцию
void ClipsRepository::EndTranslates()
{
	for(dword i = 0; i < translateTable.Size(); i++)
	{
		Del(translateTable[i].newIndex);
	}
}

//Преобразовать старый индекс в новый
long ClipsRepository::TranslateIndex(long oldIndex)
{
	if(translateTable.Size() == 0)
	{
		return oldIndex;
	}
	for(dword i = 0; i < translateTable.Size(); i++)
	{
		if(translateTable[i].oldIndex == oldIndex)
		{
			long newIndex = translateTable[i].newIndex;
			if(newIndex >= 0)
			{
				Assert(ants[newIndex]);
				ants[newIndex]->refCount++;
			}
			return translateTable[i].newIndex;
		}
	}
	Assert(false);
	return -1;
}

//Экспортировать ант
void ClipsRepository::ExportAnt(long index, AnxStream & stream)
{
	if(index < 0) return;
	Assert((dword)index < ants.Size());
	//Сохраняем индекс для трансляции
	stream << index;
	//Сохраняем данные
	long size = 0;
	AntArchive * aa = ants[index];
	if(aa)
	{
		size = aa->size;
		stream << size;
		stream.Write(aa->data, aa->size);
	}else{
		size = 0;
		stream << size;
	}
}

//Импортировать ант
long ClipsRepository::ImportAnt(AnxStream & stream)
{
	//Добавляем запись в таблицу трансляции
	Translate & t = translateTable[translateTable.Add()];
	t.oldIndex = -1;
	t.newIndex = -1;
	//Получаем индекс	
	stream >> t.oldIndex;
	//Загружаем даненые
	long size = 0;
	stream >> size;
	if(size > 0)
	{
		void * ptr = null;
		dword readSize = 0;
		stream.Read(ptr, readSize);
		Assert(readSize == size);		
		t.newIndex = Add((byte *)ptr, readSize);
	}
	return t.newIndex;
}

