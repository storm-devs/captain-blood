

#pragma once

#include "..\SndBase.h"


//Базовый класс для объектов проекта
class ProjectObject
{
	friend class SndProject;

	enum ErrorCode
	{
		ec_ok,
		ec_no_name,
		ec_name_to_long,
		ec_cant_create_file,
		ec_cant_closefilenow,
		ec_cant_rename,
	};


public:
	ProjectObject(const UniqId & folderId, dword maxNameLength, const char * fileExtantion, bool isBinaryFile = false);
	virtual ~ProjectObject();

public:
	//Получить имя
	const ExtName & GetName();
	//Получить уникалный идентификатор
	const UniqId & GetId();
	//Получить идентификатор папки
	const UniqId & GetFolder();
	//Получить текущий счётчик обновлений
	dword GetUpdateCounter();

public:
	//Установить объект в очередь на сохранение
	void SetToSave();

protected:
	//Создать полный путь файла для сохранения этого объекта
	virtual void BuildFilePath(string & path, const char * otherName = null);
	//Перезаписать идентификатор объекта
	virtual void ReplaceId(const UniqId & newId);

protected:
	//Сохранить объект
	virtual ErrorId OnSaveObject(const char * defaultFilePath) = null;
	//Первоначальная загрузка при старте редактора
	virtual ErrorId OnLoadObject(const char * defaultFilePath) = null;
	//Событие удаления объекта из проекта
	virtual void OnDeleteObject();


private:
	//Инициализация объекта
	ErrorCode Init(const char * newName);
	//Сохранить в файл под стандартным именем
	ErrorId SaveProcess();
	//Первоначальная загрузка при старте редактора
	ErrorId LoadProcess();
	//Переименовать объект
	ErrorCode Rename(const char * newName);
	//Можно ли переименовать объект в данное имя
	ErrorCode IsCanRename(const char * newName);
	//Удаление объекта
	ErrorCode Delete();
	//Установить имя
	ErrorCode SetName(const char * n, bool isSet = true);

private:
	UniqId id;					//Идентификатор объекта
	UniqId fldId;				//Идентификатор папки, в которой находиться объект
	ExtName name;				//Имя объекта
	char nameData[256];			//Данные имени	
	dword maxNameLen;			//Максимально допустимая длинна имени
	dword updateCounter;		//Счётчик обновлений за текущую сессию
	const char * ext;			//Расширение имени файла с точкой для ренерации полного пути
	bool isBinFile;				//Данные представлены бинарным файлом
};

//Получить имя
__forceinline const ExtName & ProjectObject::GetName()
{
	return name;
}

//Получить уникалный идентификатор
__forceinline const UniqId & ProjectObject::GetId()
{
	return id;
}

//Получить идентификатор папки
__forceinline const UniqId & ProjectObject::GetFolder()
{
	return fldId;
}

//Получить текущий счётчик обновлений
__forceinline dword ProjectObject::GetUpdateCounter()
{
	return updateCounter;
}

