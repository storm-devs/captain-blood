//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// MissionLoader	
//============================================================================================
			
#include "Mission.h"
#include "MissionLoader.h"
#include "..\..\Common_h\Mission\MissionFile.h"
#include "..\..\common_h\corecmds.h"

//#define MissionLoaderEnableThread

//#define ENABLE_MEMORY_PROFILE
#define MEMORY_PROFILE_CPP_FILE		"string.h"
#define MEMORY_PROFILE_CPP_LINE		-1

//============================================================================================

#ifdef MissionLoaderEnableThread

DWORD WINAPI MissionLoader::MissionLoaderThreadProc(LPVOID lpParameter)
{
	((MissionLoader *)lpParameter)->LoadObjects();
	return 0;
}

#endif

//============================================================================================

MissionLoader::MissionLoader(Mission & mis, const byte * _data, dword _size) : mission(mis)
{
	data = _data;
	pnt = 0;
	size = _size;
	Assert(data != 0);
	Assert(size > 0);
	threadID = 0;
	threadHandle = 0;
	objectsCount = 0;
	stopLoading = false;
}

MissionLoader::~MissionLoader()
{
	stopLoading = true;
	for(long i = 0; threadHandle && i < 100; i++)
	{
		Sleep(10);
	}
#ifndef _XBOX
	if(threadHandle)
	{
		TerminateThread(threadHandle, 0);
		threadHandle = 0;
	}
#endif
}


//============================================================================================

//Начать процесс загрузки
bool MissionLoader::StartProcess(float loadProgressRange)
{
	//Проверим версию данных
	if(!CheckId()) return false;
	if(objectsCount)
	{
		loadStep = loadProgressRange/objectsCount;
	}else{
		loadStep = loadProgressRange;
	}
#ifdef MissionLoaderEnableThread
	threadHandle = CreateThread(NULL, 0, MissionLoaderThreadProc, this, 0, &threadID);
#else
	LoadObjects();
#endif
	return true;
}


//Залочить список объектов
void MissionLoader::LockObjects()
{
#ifdef MissionLoaderEnableThread
	critSection.Enter();
#endif
}

//Освободить список объектов
void MissionLoader::UnLockObjects()
{
#ifdef MissionLoaderEnableThread
	critSection.Leave();
#endif
}

//Процесс загрузки окончен
bool MissionLoader::IsDone()
{
	LockObjects();
	bool res = !IsCan();
	UnLockObjects();
	return res && (threadHandle == 0);
}

//============================================================================================
//Этапы загрузки
//============================================================================================

//Проверить версию
bool MissionLoader::CheckId()
{
	MissionFileId * hdr = (MissionFileId *)Get(sizeof(MissionFileId));
	if(!hdr) return false;
	const char * id = MISSION_FILE_ID;
	for(long i = 0; i < sizeof(hdr->id); i++) if(hdr->id[i] != id[i]) return false;
	if(SwizzleLong(hdr->ver) != MISSION_FILE_VER) return false;
	objectsCount = SwizzleLong(hdr->objectsCount);
	return true;
}

#ifdef ENABLE_MEMORY_PROFILE
struct Data
{
	enum {name_len = 80};

	dword n;
	dword m;
	dword b; char name[name_len];

	Data() : n(0),m(0),b(0) {}
};

static array<Data> table(_FL_);
#endif

//Загрузить объекты
void MissionLoader::LoadObjects()
{
#ifdef ENABLE_MEMORY_PROFILE
	CoreCommand_GetMemStat stats1(MEMORY_PROFILE_CPP_FILE, MEMORY_PROFILE_CPP_LINE);
	api->ExecuteCoreCommand(stats1);
	api->Trace("Memory state at start loading...\n  use memory: %u, blocks: %u; cpp file name: %s, cpp file line: %i", 
		stats1.totalAllocSize,
		stats1.numBlocks,
		MEMORY_PROFILE_CPP_FILE,
		MEMORY_PROFILE_CPP_LINE);

	table.DelAll();
#endif
	//Начинаем загрузку объектов миссии
	while(IsCan())
	{
		//Sleep(40);   //Тест асинхронной загрузки
		if(stopLoading) break;
		LockObjects();
		LoadObject();
		UnLockObjects();
		api->SetWatchDog();
	}
	threadHandle = 0;
	stopLoading = true;

#ifdef ENABLE_MEMORY_PROFILE
	api->Trace("\n%30s %14s %14s %14s %14s %14s","Class name","Objects count","Total memory","Total blocks","Memory","Blocks");
	api->Trace("");

	array<Data *>list(_FL_); list.AddElements(table.Size());

	for( int i = 0 ; i < table ; i++ )
	{
		Data *data = &table[i];

		list[i] = data;

		int j = i;

		while( j > 0 )
		{
			if( list[j - 1]->m < list[j]->m )
			{
				Data *t = list[j - 1];

				list[j - 1] = list[j];
				list[j] = t;
			}
			else
				break;

			j--;
		}
	}

	for( int i = 0 ; i < list ; i++ )
	{
		const Data &data = *list[i];

		api->Trace("%30s %14d %14d %14d %14d %14d",data.name,data.n,data.m,data.b,data.m/data.n,data.b/data.n);
	}

	api->Trace("");
#endif
}

//Загрузить объект
void MissionLoader::LoadObject()
{
	//Данные объекта
	dword size = 0;
	const char * objectType = GetObjectData(size);	
	if(!objectType)
	{
		api->Trace("MissionLoader::LoadObject -> can't get object information. mis file is damage?");
		return;
	}
	const void * data = Get(size);
	if(!data)
	{
		api->Trace("MissionLoader::LoadObject -> can't get data for object \"%s\"", objectType);
		return;
	}
	
	//Создаём объект
#ifdef ENABLE_MEMORY_PROFILE
	const char * calcAllocsInFile = MEMORY_PROFILE_CPP_FILE;
	const long calcAllocsInLine = MEMORY_PROFILE_CPP_LINE;
	CoreCommand_GetMemStat stats1(calcAllocsInFile, calcAllocsInLine);
	api->ExecuteCoreCommand(stats1);
	api->Trace("Object %s",objectType);
#endif
	MOPReader reader(data, size);
	if(!mission.CreateObjectEx(objectType, reader, data, size))
	{
		api->Trace("MissionLoader::LoadObject -> can't create object \"%s\" (type: \"%s\")", reader.GetObjectID(), objectType);
	}
	mission.LoadingProgress(loadStep);
#ifdef ENABLE_MEMORY_PROFILE
	CoreCommand_GetMemStat stats2(calcAllocsInFile, calcAllocsInLine);
	api->ExecuteCoreCommand(stats2);
	if( stats2.totalAllocSize != stats1.totalAllocSize || stats2.numBlocks != stats1.numBlocks )
	{
	api->Trace("Object %s, use memory: %u, blocks: %u; cpp file name: %s, cpp file line: %i", 
								objectType,
								stats2.totalAllocSize - stats1.totalAllocSize,
								stats2.numBlocks - stats1.numBlocks,
								calcAllocsInFile,
								calcAllocsInLine);

		for( int i = 0 ; i < table ; i++ )
		{
			if( string::IsEqual(table[i].name,objectType))
				break;
		}

		Data *data = null;

		if( i < table )
			data = &table[i];
		else
		{
			data = &table[table.Add()];
			strcpy_s(data->name,Data::name_len,objectType);
		}

		data->n++;
		data->m += stats2.totalAllocSize - stats1.totalAllocSize;
		data->b += stats2.numBlocks - stats1.numBlocks;
	}
#endif	
}

//Получить данные объекта
const char * MissionLoader::GetObjectData(dword & datasize)
{
	//Получаем первый символ
	const char * s = (const char *)Get(1);
	if(!s || !s[0]) return null;
	//Ищем окончание строки
	while(true)
	{
		const char * c = (const char *)Get(1);
		if(!c) return null;
		if(!c[0]) break;
	}
	//Получаем размер данных
	dword * size = (dword *)Get(sizeof(dword));
	if(!size) return null;
	datasize = SwizzleDWord(*size);
	return s;
}

//============================================================================================
//Входные данные
//============================================================================================

//Получить данные
__forceinline const void * MissionLoader::Get(dword s)
{
	if(pnt + s > size) return null;
	const void * p = data + pnt;
	pnt += s;
	return p;
}

//Окончание потока
bool MissionLoader::IsCan()
{
	return pnt < size;
}