
#ifndef _PakFile_h_
#define _PakFile_h_

#include "..\..\common_h\FileService.h"

class PackArchivator;
class DataFileMem;

//Пак-файл
class PackFile : public IPackFile
{
private:
	/*
	Формат пак файла

	Header
	Element[Header.filesCount]
	strings table
	data
	*/

#pragma pack(push, 1)

	struct Element
	{
		const char * name;		//Имя файла
		dword hash;				//Хэшь значение файла
		dword len;				//Длинна имени файла
		const byte * data;		//Смещение начала относительно начала данных
		dword fileSize;			//Реальный размер файла
		dword packSize;			//Размер файла в паке
		Element * next;			//Следующий в цепочке хэша
	};

	struct Header
	{
		byte id[8];				//Идентификатор
		byte version[4];		//Версия
		dword filesCount;		//Количество файлов
		dword entryMask;		//Маска входной таблицы
		Element * entry[1];		//Входная таблица поиска размера entryMask + 1
	};

#pragma pack(pop)

public:
	PackFile(const char * path, const char * _cppFileName, long _cppFileLine);
	virtual ~PackFile();

	//Загрузить файл в память
	bool Load(IDataFile * df);
	//Увеличить счётчик ссылок
	void AddRefCount();
	//Закрыть файл сообщив об ошибке
	void ErrorRelease();
	//Проверить имя пака на совпадение
	bool IsThis(const char * _fileName);

#ifndef _XBOX
	//Сохранить файлы в паке
	static bool SaveToPack(const char * path, PackArchivator & compressor);
#endif

public:
	//Удалить объект
	virtual void Release();
	//Получить количество файлов
	virtual dword Count() const;
	//Получить путь до файла внутри пака
	virtual const char * LocalPath(dword index) const;
	//Получить полный путь до файла
	virtual const char * FullPath(dword index) const;
	//Получить размер пак-файла
	virtual dword Size() const;
	//Найти в паке файл
	const byte * Find(const char * fileName, dword hash, dword len, dword & packedSize, dword & fileSize);
	//Добавить все имена файлов
	void CollectFiles(array<const char *> & names);


private:
	array<byte> data;			//Данные пака
	long refCounter;			//Счётчик пользователей файла
	char fileName[256];			//Имя файла без пути
#ifndef STOP_DEBUG
	const char * cppFileName;	//Исходный файл откуда был открыт пак-файл
	long cppFileLine;			//Строка исходного файла, откуда был открыт пак-файл
#endif

	static const char id[8];	//Идентификатор пак-файлов
	static const char ver[4];	//Текущая версия пак-файла
};

//Проверить имя пака на совпадение
__forceinline bool PackFile::IsThis(const char * _fileName)
{
	return string::IsEqual(fileName, _fileName);
}

//Найти в паке файл
__forceinline const byte * PackFile::Find(const char * fileName, dword hash, dword len, dword & packedSize, dword & fileSize)
{
	Header & hdr = (Header &)data[0];
	for(Element * el = hdr.entry[hash & hdr.entryMask]; el; el = el->next)
	{
		if(el->hash == hash && el->len == len)
		{
			if(string::IsEqual(el->name, fileName))
			{
				packedSize = el->packSize;
				fileSize = el->fileSize;
				return el->data;
			}
		}
	}
	return null;
}


#endif
