
#ifndef _DllManager_h_
#define _DllManager_h_

#include "CoreBase.h"

class DllManager
{
	struct DllDesc
	{
		HMODULE handle;
		StormEngine_Declarator * first;
		const char * versionId;
		string path;
	};

public:
	DllManager(const char * coreModuleNameForSkip);
	~DllManager();

//-------------------------------------------------------------------------------------------------------
//Работа с менеджером dll
//-------------------------------------------------------------------------------------------------------
public:
	//Загрузить модули быстрого запуска
	bool LoadQuickModules(const char * quickModules);
	//Загрузить модули
	void LoadModules();
	//Получить количество подключёных модулей
	long GetCount();
	//Получить первый дескриптор
	StormEngine_Declarator * GetFirst(long i);
	//Выгрузить все модули
	void Unload();
	//Занести данные о загруженных модулях в базу данных
	void PutInfoToStorage(ICoreStorageFolder & root);

//-------------------------------------------------------------------------------------------------------
//Внутреннее
//-------------------------------------------------------------------------------------------------------
protected:
	//Загрузить с проверкой библиотеку
	bool LoadDll(const char * dllName);
	//Найти указатель на имя файла в пути
	const char * FileFileName(const char * filePath);
	//Получить директорию c модулями
	void GetModulesPath(string & path);
	

//-------------------------------------------------------------------------------------------------------
protected:
	array<DllDesc> dlls;
};




#endif

