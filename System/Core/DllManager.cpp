

#include "DllManager.h"

#ifndef _XBOX
#include <strsafe.h>
#endif


DllManager::DllManager(const char * coreModuleNameForSkip) : dlls(_FL_, 4)
{
	dlls.Reserve(32);
	//Добавляем запись об модуле ядра
	if(string::NotEmpty(coreModuleNameForSkip))
	{
		DllDesc & desc = dlls[dlls.Add()];
		desc.handle = null;
		desc.first = null;
		desc.versionId = null;
		desc.path = coreModuleNameForSkip;
	}
}

DllManager::~DllManager()
{
	Unload();
}

//Загрузить модули быстрого запуска
bool DllManager::LoadQuickModules(const char * quickModules)
{
	if(!quickModules) return true;	
	string modulesPath;
	GetModulesPath(modulesPath);
	string currentPath;	
	currentPath.Reserve(modulesPath.Len() + 64);
	while(*quickModules)
	{
		const char * name = quickModules;
		//Проверяем имя на отсутствие слэшей
		for(const char * n = name; *n; n++)
		{
			if(*n == '\\' || *n == '/')
			{
				CoreLogOut("Invalidate quick module name: \"%s\"", name);
				return false;
			}
		}
		//Загружаем модуль
		currentPath = modulesPath;
		currentPath += name;
		if(!LoadDll(currentPath.c_str()))
		{
			CoreLogOut("Error load quick module. (\"%s\")", currentPath.c_str());
			return false;
		}
		CoreLogOut("Load quick module: \"%s\"", currentPath.c_str());
		//Ищим следующее имя
		while(*quickModules) quickModules++;
		quickModules++;
	}
	return true;
}

//Загрузить модули
void DllManager::LoadModules()
{
	//Получаем текущую директорию и маску
	string modulesPath;
	GetModulesPath(modulesPath);
	string currentPath;
	currentPath.Reserve(modulesPath.Len() + 64);
	string findMask;
	findMask.Reserve(modulesPath.Len() + 8);
	findMask = modulesPath;
#ifndef _XBOX
	findMask += "*.dll";
#else
	findMask += "*.xdl";
#endif
	//Первый запрос	
	WIN32_FIND_DATA findData;
	HANDLE hFind = ::FindFirstFile(findMask.GetBuffer(), &findData);
	if(hFind == INVALID_HANDLE_VALUE) return;
	//Последовательный просмотр модулей
	do 
	{
		//Формируем имя 
		findData.cFileName[sizeof(findData.cFileName) - 1] = 0;
		currentPath = modulesPath;
		currentPath += findData.cFileName;
		//Загружаем модуль
		if(LoadDll(currentPath.c_str()))
		{
			CoreLogOut("Load module: \"%s\"", currentPath.c_str());
		}
	}while(::FindNextFile(hFind, &findData));
	//Заканчиваем поиск
	FindClose(hFind);
}

//Получить количество подключёных модулей
long DllManager::GetCount()
{
	return (long)dlls.Size();
}

//Получить первый дескриптор
StormEngine_Declarator * DllManager::GetFirst(long i)
{
	return dlls[i].first;
}

//Выгрузить все модули
void DllManager::Unload()
{
	for(long i = 0; i < dlls; i++)
	{
		if(dlls[i].handle)
		{
			FreeLibrary(dlls[i].handle);
		}
	}
	dlls.DelAll();
}

//Занести данные о загруженных модулях в базу данных
void DllManager::PutInfoToStorage(ICoreStorageFolder & root)
{
	string moduleName;
	string storagePath;
	for(long i = 0; i < dlls; i++)
	{
		if(!dlls[i].handle)
		{
			continue;
		}
		moduleName.GetFileTitle(dlls[i].path);
		storagePath = "system.core.modules.";
		storagePath += moduleName;
		storagePath += ".version";
		const char * versionId = dlls[i].versionId ? dlls[i].versionId : "unknown";
		root.SetString(storagePath.c_str(), versionId);
		storagePath = "system.core.modules.";
		storagePath += moduleName;
		storagePath += ".path";
		root.SetString(storagePath.c_str(), dlls[i].path.c_str());
	}	
}

//Описание функции инициализации
typedef void (_cdecl * StormEngine_InitDll)(ICore * core, StormEngine_Declarator * & first, const char * & verId, long init_rand);

//Загрузить с проверкой библиотеку
bool DllManager::LoadDll(const char * dllName)
{
	//CoreLogOut("Loading module \"%s\"", dllName);
	//Проверяем модуль среди загруженных
	const char * fileName = FileFileName(dllName);
	for(dword i = 0; i < dlls.Size(); i++)
	{
		const char * fn = FileFileName(dlls[i].path.c_str());
		if(string::IsEqual(fileName, fn))
		{
			//Уже загружен данный модуль, пропускаем его
			return true;
		}
	}	
	//Загружаем библиотеку
	DllDesc desc;
	desc.handle = ::LoadLibrary(dllName);	
	if(!desc.handle)
	{
#ifndef _XBOX
		LPTSTR lpszFunction = "LoadLibrary";
		DWORD dwLoadError = GetLastError();

		LPVOID lpMsgBuf;
		LPVOID lpDisplayBuf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dwLoadError, 
			          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL );

		// Display the error message and exit the process
		lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR)); 
		
		StringCchPrintf((LPTSTR)lpDisplayBuf, LocalSize(lpDisplayBuf) / sizeof(TCHAR), TEXT("%s failed with error %d: %s"), lpszFunction, dwLoadError, lpMsgBuf); 

		CoreLogOut("Core error: can't load library \"%s\", error code : 0x%08X, reason : '%s'", dllName, dwLoadError, (LPCTSTR)lpDisplayBuf);

		LocalFree(lpMsgBuf);
		LocalFree(lpDisplayBuf);
#else
		CoreLogOut("Core error: can't load library \"%s\"", dllName);
#endif
		return false;
	}
	//Получаем указатель на функцию инициализации
	StormEngine_InitDll init = (StormEngine_InitDll)GetProcAddress(desc.handle, "StormEngine_InitDll");
	if(!init)
	{
		CoreLogOut("Core error: can't get core interface function in library \"%s\"", dllName);
		FreeLibrary(desc.handle);
		return false;
	}
	//Подключаем библиотеку	
	init(api, desc.first, desc.versionId, rand());
	dlls[dlls.Add(desc)].path = dllName;
	return true;
}

//Найти указатель на имя файла в пути
const char * DllManager::FileFileName(const char * filePath)
{
	if(!filePath) return null;
	for(long i = string::Len(filePath); i >= 0; i--)
	{
		char c = filePath[i];
		if(c == '\\' || c == '/')
		{
			break;
		}
	}
	return filePath + i + 1;
}

//Получить директорию c модулями
void DllManager::GetModulesPath(string & path)
{
#ifndef _XBOX
	const long maxSize = 4096;
	char * tmpCurDir = NEW char[maxSize];
	GetCurrentDirectory(maxSize - 1, tmpCurDir);
	tmpCurDir[maxSize - 1] = 0;
	path = tmpCurDir;
	delete tmpCurDir;
	tmpCurDir = null;
	//Формируем директорию в которой модули
	path += "\\"; path += Core_DllsFolder; path += "\\";
#else
	path = "D:\\";
#endif
}
