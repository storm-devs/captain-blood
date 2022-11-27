

#include "PackFile.h"
#include "FileService.h"
#include "FilesTree.h"
#include "DataFile.h"
#include "Archivator.h"
#include "..\..\common_h\data_swizzle.h"


const char PackFile::id[8] = {'S','t','o','r','m','P','k','x'};
const char PackFile::ver[4] = {'2','.','1','0'};


PackFile::PackFile(const char * path, const char * _cppFileName, long _cppFileLine) : data(_FL_, 1)
{
	const char * name = string::GetFileName(path);
	for(long i = 0; i < sizeof(fileName); i++, name++)
	{
		fileName[i] = *name;
		if(*name == 0) break;
	}
	fileName[sizeof(fileName) - 1] = 0;
	refCounter = 1;
#ifndef STOP_DEBUG
	cppFileName = _cppFileName;
	cppFileLine = _cppFileLine;
#endif
}

PackFile::~PackFile()
{
}

//Загрузить файл в память
bool PackFile::Load(IDataFile * df)
{
	//Проверяем размер
	dword size = df->Size();
	if(size < sizeof(Header) + sizeof(Element) + 1)
	{
labelErrorFile:
#ifndef STOP_DEBUG
		api->Error("FileService error: can't read pack file \"%s\", invalidate file content (file: %s, line: %i -> LoadPak)", fileName, cppFileName, cppFileLine);
#endif
		return false;
	}
	//Загружаем файл
	data.AddElements(size);
	if(df->Read(data.GetBuffer(), size) != size)
	{
#ifndef STOP_DEBUG
		api->Error("FileService error: can't read pack file \"%s\", io error (file: %s, line: %i -> LoadPak)", fileName, cppFileName, cppFileLine);
#endif
		return false;
	}
	const byte * infoAddress = data.GetBuffer();
	Header & header = *(Header *)infoAddress;
	//Проверяем идентификатор и версию
	Assert(ARRSIZE(header.id) == ARRSIZE(id));
	for(dword i = 0; i < ARRSIZE(header.id); i++) 
	{
		if(header.id[i] != id[i])
		{
			goto labelErrorFile;
		}
	}
	Assert(ARRSIZE(header.version) == ARRSIZE(ver));
	for(dword i = 0; i < ARRSIZE(header.version); i++)
	{
		if(header.version[i] != ver[i])
		{
			goto labelErrorFile;
		}
	}
	//Глобальные параметры файла
	if(!__SwizzleDetect())
	{
		__RefDataSwizzler(header.filesCount);
		__RefDataSwizzler(header.entryMask);
	}
	if(sizeof(Header) + header.entryMask*sizeof(Element *) + header.filesCount*sizeof(Element) > size)
	{
		goto labelErrorFile;
	}	
	if(!__SwizzleDetect())
	{
		for(dword i = 0; i <= header.entryMask; i++)
		{
			__RefDataSwizzler(header.entry[i]);
		}
	}
	Element * elements = (Element *)&data[sizeof(Header) + header.entryMask*sizeof(Element *)];
	if((byte *)(elements + header.filesCount) >= infoAddress + size)
	{
		goto labelErrorFile;
	}
	//Входная таблица
	for(dword i = 0; i <= header.entryMask; i++)
	{
		if(header.entry[i])
		{
			Element * ptr = (Element *)(infoAddress + ((byte *)header.entry[i] - (byte *)0));
			if(ptr < elements || ptr > elements + header.filesCount)
			{
				goto labelErrorFile;
			}
			header.entry[i] = ptr;
		}		
	}
	//Таблица файловых дескрипторов
	if(!__SwizzleDetect())
	{
		for(dword i = 0; i < header.filesCount; i++)
		{
			__RefDataSwizzler(elements[i].name);
			__RefDataSwizzler(elements[i].hash);
			__RefDataSwizzler(elements[i].len);
			__RefDataSwizzler(elements[i].data);
			__RefDataSwizzler(elements[i].fileSize);
			__RefDataSwizzler(elements[i].packSize);
			__RefDataSwizzler(elements[i].next);
		}
	}
	for(dword i = 0; i < header.filesCount; i++)
	{
		Element & el = elements[i];
		el.name = (const char *)(infoAddress + ((byte *)el.name - (byte *)0));
		if((byte *)el.name < infoAddress || (byte *)el.name >= infoAddress + size)
		{
			goto labelErrorFile;
		}
		el.data = (const byte *)(infoAddress + ((byte *)el.data - (byte *)0));
		if((byte *)el.data < (byte *)(elements + header.filesCount) || (byte *)el.data >= infoAddress + size)
		{
			goto labelErrorFile;
		}
		if(el.next)
		{
			el.next = (Element *)(infoAddress + ((byte *)el.next - (byte *)0));
			if(el.next < elements || el.next > elements + header.filesCount)
			{
				goto labelErrorFile;
			}
		}
	}
	return true;
}

//Увеличить счётчик ссылок
void PackFile::AddRefCount()
{
	refCounter++;
}

//Удалить объект, закончив отражать путь
void PackFile::ErrorRelease()
{
#ifndef STOP_DEBUG
	api->Error("FileService error: IPackFile file \"%s\" not release (file: %s, line %i)", fileName, cppFileName, cppFileLine);
#endif
	delete this;
}

//Удалить объект
void PackFile::Release()
{
	SingleExClassThread(FileService::object)
	refCounter--;
	if(refCounter > 0) return;
	//Удаляем себя
	FileService::object->DeletePackFile(this);
	delete this;
}

//Получить количество файлов
dword PackFile::Count() const
{
	Header & hdr = (Header &)data[0];
	return hdr.filesCount;
}

//Получить путь до файла внутри пака
const char * PackFile::LocalPath(dword index) const
{
	Header & hdr = (Header &)data[0];
	Element * elements = (Element *)&data[sizeof(Header) + hdr.entryMask*sizeof(Element *)];
	Assert(index < hdr.filesCount);
	return elements[index].name;
}

//Получить полный путь до файла
const char * PackFile::FullPath(dword index) const
{
	return LocalPath(index);
}

//Получить размер пак-файла
dword PackFile::Size() const
{
	return data.GetDataSize();
}

//Добавить все имена файлов
void PackFile::CollectFiles(array<const char *> & names)
{
	Header & hdr = (Header &)data[0];
	Element * elements = (Element *)&data[sizeof(Header) + hdr.entryMask*sizeof(Element *)];
	names.Reserve(names.Size() + hdr.filesCount);
	for(dword i = 0; i < hdr.filesCount; i++)
	{
		names.Add(elements[i].name);
	}
}


#ifndef _XBOX

//Сохранить файлы в паке
bool PackFile::SaveToPack(const char * path, PackArchivator & compressor)
{
	Assert(!__SwizzleDetect());
	dword filesCount = compressor.GetFilesCount();
	if(!filesCount)
	{
		return false;
	}
	Assert(compressor.GetDataSize() > 0);
	//Вычисляем требуемый размер хэша
	dword hashSize = (filesCount < 80) ? filesCount*2 : filesCount;
	//Находим маску
	dword hashMask = 0x80000000;
	while((hashMask & hashSize) == 0)
	{
		hashMask >>= 1;
		Assert(hashMask != 0);
	}
	hashSize = hashMask;
	if(hashSize < 4) hashSize = 4;
	if(hashSize > 1024) hashSize = 1024;
	hashMask = hashSize - 1;
	//Подготавливаем описывающую структуру
	dword namesSize = compressor.GetNamesSize();
	array<byte> info(_FL_);
	dword infoSize = sizeof(Header) + hashMask*sizeof(Element *) + sizeof(Element)*filesCount + namesSize;
	info.AddElements(infoSize);
	byte * infoAddress = info.GetBuffer();
	Assert(info.GetDataSize() == infoSize);
	memset(infoAddress, 0, infoSize);
	Header & hdr = (Header &)info[0];
	Element * elements = (Element *)&info[sizeof(Header) + hashMask*sizeof(Element *)];
	char * names = (char *)&elements[filesCount];
	Assert(ARRSIZE(hdr.id) == ARRSIZE(id));
	for(dword i = 0; i < ARRSIZE(hdr.id); i++) hdr.id[i] = id[i];
	Assert(ARRSIZE(hdr.version) == ARRSIZE(ver));
	for(dword i = 0; i < ARRSIZE(hdr.version); i++) hdr.version[i] = ver[i];
	hdr.filesCount = filesCount;
	hdr.entryMask = hashMask;
	for(dword i = 0; i < filesCount; i++)
	{
		Element & el = elements[i];
		//Имя
		el.name = names;
		const char * n = compressor.GetFileName(i, el.len, el.hash);
		el.next = null;
		memcpy(names, n, el.len + 1);
		names += el.len + 1;
		Element ** chEl = &hdr.entry[el.hash & hdr.entryMask];
		while(*chEl != null) chEl = &(*chEl)->next;
		*chEl = &el;
		//Данные
		el.data = (byte *)0 + infoSize + compressor.GetFileOffset(i);
		el.fileSize = compressor.GetFileSize(i);
		el.packSize = compressor.GetFileCompressedSize(i);
	}
	//Приводим в сохраняемый формат с порядком байт под xbox
	__RefDataSwizzler(hdr.filesCount);
	__RefDataSwizzler(hdr.entryMask);	
	for(dword i = 0; i < hashSize; i++)
	{
		if(hdr.entry[i])
		{
			hdr.entry[i] = (Element *)((byte *)0 + ((byte *)hdr.entry[i] - infoAddress));
		}
		__RefDataSwizzler(hdr.entry[i]);
	}
	for(dword i = 0; i < filesCount; i++)
	{
		Element & el = elements[i];
		Assert(el.name);
		el.name = (const char *)((byte *)0 + ((byte *)el.name - infoAddress));
		__RefDataSwizzler(el.name);
		__RefDataSwizzler(el.hash);
		__RefDataSwizzler(el.len);
		__RefDataSwizzler(el.data);
		__RefDataSwizzler(el.fileSize);
		__RefDataSwizzler(el.packSize);
		if(el.next)
		{
			el.next = (Element *)((byte *)0 + ((byte *)el.next - infoAddress));
		}
		__RefDataSwizzler(el.next);
	}
	//Сохраняем файл
	IFile * file = FileService::object->OpenFile(path, file_create_always, _FL_);
	if(!file)
	{
		return false;
	}
	bool isError = (file->Write(infoAddress, infoSize) != infoSize);
	isError |= (file->Write(compressor.GetDataBuffer(), compressor.GetDataSize()) != compressor.GetDataSize());
	file->Release();
	if(isError)
	{
		FileService::object->Delete(path);
		return false;
	}
	return true;	
}

#endif

