#include "particleservice.h"
#include "..\manager\particlemanager.h"
#include "..\datacache\DataCache.h"
#include "..\..\common_h\SetThreadName.h"


INTERFACE_FUNCTION
CREATE_SERVICE(ParticleService, 30)


#define MAX_WAIT_TIME (500)


ParticleService* PService = NULL;

ParticleService::ParticleService () : CreatedManagers (_FL_), 
                                      allData (_FL_, 2048)
{
#ifdef ENABLE_PARTICLE_THREADS

	pManagerToExecute = NULL;

	dwNeedCloseThread = 0;
	hCanStartUpdate = NULL;
	hThreadDone = NULL;

	hUpdateThread = NULL;
#endif


	dwCurrentFrame = 0;

#ifndef _XBOX
	pIBuffer = NULL;
#endif

	PService = this;

	sysDelete = false;

	globalDataCache = NULL;

	bLowQuality = false;
}

ParticleService::~ParticleService ()
{

#ifdef ENABLE_PARTICLE_THREADS

	InterlockedIncrement(&dwNeedCloseThread);

	//Что бы поток отпустить...
	SetEvent(hCanStartUpdate);


	if (hUpdateThread)
	{
		for (;;)
		{
			DWORD waitResult = WaitForSingleObject(hUpdateThread, MAX_WAIT_TIME);
			if (waitResult != WAIT_TIMEOUT)
			{
				break;
			}
		}
	}

	if( hCanStartUpdate )
	{
		CloseHandle( hCanStartUpdate );
		hCanStartUpdate = NULL;
	}


	hUpdateThread = NULL;
	hCanStartUpdate = NULL;

#endif


#ifndef _XBOX
	if (pIBuffer)
	{
		pIBuffer->Release();
		pIBuffer = NULL;
	}
#endif


	for (dword i = 0; i < VB_FRAMES_COUNT; i++)
	{
		vbFrames[i].Release();
	}

	sysDelete = true;

	if (CreatedManagers.Size () > 0) 
	{
		api->Trace("Unreleased particles managers found !\n");
	}
	for (int n = 0; n < CreatedManagers; n++)
	{
		api->Trace("Manager created in %s, Line %d\n", CreatedManagers[n].FileName, CreatedManagers[n].Line);
		CreatedManagers[n].pManager->Release();
	}

	delete spriteProcessor;
	spriteProcessor = NULL;

	delete globalDataCache;
	PService = NULL;
}

#ifdef ENABLE_PARTICLE_THREADS
void ParticleService::ThreadExecute(LPVOID lpThreadParameter)
{
	ParticleService* srv = (ParticleService*)lpThreadParameter;

	while (true)
	{
		if (!srv->threadUpdate()) break;
	}

	ExitThread(0xDEAD);
}


#endif



bool ParticleService::Init()
{
	IFileService *storage = (IFileService *)api->GetService("FileService");
	Assert(storage)


	IRender * pRS = (IRender*)api->GetService("DX9Render");
	Assert(pRS)


#ifdef _XBOX
	//во write-combined память кладем партиклы на боксе
	DWORD dwFlags = 0;
#else
	DWORD dwFlags = USAGE_WRITEONLY | USAGE_DYNAMIC;
#endif

	for (dword i = 0; i < VB_FRAMES_COUNT; i++)
	{
		vbFrames[i].Create(pRS, dwFlags);
	}

#ifndef _XBOX
	pIBuffer = pRS->CreateIndexBuffer(_MAX_BILLBOARDS * 6 * sizeof(WORD), _FL_, dwFlags); 
	WORD * pTrgs = (WORD*)pIBuffer->Lock(); 
	Assert(pTrgs != NULL);

	for (long i = 0; i < _MAX_BILLBOARDS; i++)
	{
		pTrgs[i * 6 + 0] = WORD(i * 4 + 0);
		pTrgs[i * 6 + 1] = WORD(i * 4 + 1);
		pTrgs[i * 6 + 2] = WORD(i * 4 + 2);
		pTrgs[i * 6 + 3] = WORD(i * 4 + 0);
		pTrgs[i * 6 + 4] = WORD(i * 4 + 2);
		pTrgs[i * 6 + 5] = WORD(i * 4 + 3);
	}
	pIBuffer->Unlock();
#endif


	IIniFile *ini = storage->SystemIni();
	if( ini )
	{
		bLowQuality = ini->GetLong("particles", "lowquality", 0) != 0;
	}

	

	api->SetStartFrameLevel(this, Core_DefaultExecuteLevel);
	api->SetEndFrameLevel(this, Core_DefaultExecuteLevel + 0x100);

	spriteProcessor = NEW BillBoardProcessor;

	globalDataCache = NEW DataCacheGlobal();


	LoadAllfromDisk();



#ifdef ENABLE_PARTICLE_THREADS

	hCanStartUpdate = CreateEvent( null, false, false, null );
	ResetEvent(hCanStartUpdate);

	hThreadDone = CreateEvent( null, true, false, null );
	SetEvent( hThreadDone );


	DWORD dwParticlesThreadID = 0;
	hUpdateThread = CreateThread(null, 0, (LPTHREAD_START_ROUTINE)ParticleService::ThreadExecute, this, CREATE_SUSPENDED, &dwParticlesThreadID);
	
#ifdef _XBOX
	SetThreadPriority(hUpdateThread, THREAD_PRIORITY_ABOVE_NORMAL);
	XSetThreadProcessor( hUpdateThread, 5 );
#endif

	XSetThreadName(dwParticlesThreadID, "Particles::Work");
	ResumeThread(hUpdateThread);

#endif


	return true;
}



IIBuffer * ParticleService::GetCurrentIndexBuffer()
{
#ifndef _XBOX
	return pIBuffer;
#else
	return NULL;
#endif
}




IParticleManager* ParticleService::CreateManagerEx (const char* ProjectName, const char* File, int Line)
{
	ParticleManager* pManager = NEW ParticleManager (this);

	CreatedManager manager;
	manager.pManager = pManager;
	manager.Line = Line;
	manager.FileName = File;
	CreatedManagers.Add(manager);

	return pManager;
}

void ParticleService::RemoveManagerFromList (IParticleManager* pManager)
{
	if (sysDelete) return;
	for (int n = 0; n < CreatedManagers; n++)
	{
		if (CreatedManagers[n].pManager == pManager)
		{
			CreatedManagers.ExtractNoShift(n);
			return;
		}
	}
}

DWORD ParticleService::GetManagersCount ()
{
	return CreatedManagers.Size();
}

IParticleManager* ParticleService::GetManagerByIndex (DWORD Index)
{
	return CreatedManagers[Index].pManager;
}



ParticleVB * ParticleService::GetUpdateData()
{
	return &vbFrames[dwCurrentFrame];
}


void ParticleService::StartFrame(float dltTime)
{
}


void ParticleService::EndFrame(float dltTime)
{
	//OutputDebugString("----------------------\n");

	dwCurrentFrame++;
	if (dwCurrentFrame >= VB_FRAMES_COUNT)
	{
		dwCurrentFrame = 0;
	}


	if (api->DebugKeyState(VK_F3, VK_SHIFT))
	{
		for (int n = 0; n < CreatedManagers; n++)
		{
			CreatedManagers[n].pManager->ReloadSystems();
		}
	}

}

DataCacheGlobal* ParticleService::GetDataCache()
{
	return globalDataCache;
}




void ParticleService::LoadAllfromDisk()
{
	AssertCoreThread

	systemFreeAllDataForGraphs();

	IFileService* pFS = (IFileService*)api->GetService("FileService");

#ifdef USE_XML_FILE_FORMAT

	// загрузка из XML ресурсов 
	IFinder * finder = pFS->Finder("*.xml", FFND_NODIRS | FFND_NODOTS);
	if(!finder) return;

	string sFileName;
	for(finder->Begin(".\\resource\\particles\\"); !finder->IsDone(); finder->Next())
	{
		sFileName = finder->FileName();
		pDataCache->CacheSystemXML(sFileName);
	}

	finder->Release();

#else



	DWORD dwStartTime = GetTickCount();

	dword pTime;
	RDTSC_B(pTime);

#ifndef _XBOX
	//Загрузка пак-файла если есть такой
	IPackFile * pack = pFS->LoadPack("resource\\particles.pkx", _FL_);
#else
	IPackFile * pack = pFS->LoadPack("particles.pkx", _FL_);
#endif

	// загрузка из бинарных ресурсов 
	IFinder * finder = pFS->CreateFinder("resource\\particles\\", "*.xps", find_all_files_no_mirrors, _FL_);
	if(!finder) return;


	for(dword i = 0; i < finder->Count(); i++)
	{
		const char* sFileName = finder->Name(i);

		globalDataCache->CacheSystem(sFileName);
	}

	finder->Release();

	if(pack)
	{
		pack->Release();
	}

	RDTSC_E(pTime);


	DWORD dwEndTime = GetTickCount();

	api->Trace("%d particles loaded by %d ticks - %3.2f seconds", globalDataCache->GetCachedCount(), pTime , (dwEndTime - dwStartTime) / 1000.0f);


#endif
}



DataGraph* ParticleService::AllocateDataGraph (FieldList* pMaster)
{
	DataGraph* Field = DataGraphsPool.Allocate();
	Field->SetMasterField(pMaster);

	//DataGraph* Field = NEW DataGraph(pMaster);

	return Field;
}

void ParticleService::FreeDataGraph (DataGraph* dataGraph)
{
	DataGraphsPool.Free(dataGraph);
}




void ParticleService::systemFreeAllDataForGraphs()
{
	allData.DelAll();
}

array<GraphVertex> * ParticleService::AllocateDataForGraphs (long dwMinGraphSize, long dwMaxGraphSize,
											                 long &MinGraphDataStart, long &MinGraphDataSize,
											                 long &MaxGraphDataStart, long &MaxGraphDataSize)
{
	if (dwMinGraphSize >= 0)
	{
		if (dwMinGraphSize == 0)
		{
			MinGraphDataStart = -1;
			MinGraphDataSize = 0;
		} else
		{
			dword dwStartIndex = allData.Size();

			allData.AddElements(dwMinGraphSize);

			allData.Add();
			allData.Add();
			allData.Add();

			MinGraphDataStart = dwStartIndex+3;
			MinGraphDataSize = dwMinGraphSize;
		}

	}

	if (dwMaxGraphSize >= 0)
	{
		if (dwMaxGraphSize == 0)
		{
			MaxGraphDataStart = -1;
			MaxGraphDataSize = 0;
		} else
		{
			dword dwStartIndex = allData.Size();

			allData.AddElements(dwMaxGraphSize);

			allData.Add();
			allData.Add();
			allData.Add();
			allData.Add();
			allData.Add();

			MaxGraphDataStart = dwStartIndex+5;

			MaxGraphDataSize = dwMaxGraphSize;
		}

	}


	return &allData;
}

#ifdef ENABLE_PARTICLE_THREADS

void ParticleService::StartUpdateThread(ParticleManager * pManager)
{
	AssertCoreThread

	//OutputDebugString("ParticleService::StartUpdateThread\n");

	//Ждем пока поток дообновляет, если он уже обновляет данные
	WaitForSingleObject(hThreadDone, INFINITE);
	ResetEvent(hThreadDone);

	//OutputDebugString("WaitForSingleObject(hThreadDone, INFINITE) - ParticleService::StartUpdateThread\n");

	pManagerToExecute = pManager;

	ParticleVB * vbToUpdate = GetUpdateData();
	vbToUpdate->Lock();

	Assert(vbToUpdate->pParticlesVerts);
	Assert(vbToUpdate->pParticlesXZVerts);
	Assert(vbToUpdate->pDistortedParticlesVerts);
	Assert(vbToUpdate->pDistortedParticlesXZVerts);


	//Можно уже обновлять...
	//OutputDebugString("hCanStartUpdate - fired. ParticleService::StartUpdateThread()\n");
	SetEvent(hCanStartUpdate);
}

void ParticleService::WaitUntilUpdateFinished()
{
	AssertCoreThread

	//OutputDebugString("ParticleService::WaitUntilUpdateAndStartDraw\n");

	//Ждем пока поток дообновляет, если он уже обновляет данные
	WaitForSingleObject(hThreadDone, INFINITE);

	//OutputDebugString("WaitForSingleObject(hThreadDone, INFINITE) - ParticleService::WaitUntilUpdateAndStartDraw\n");

	pManagerToExecute = NULL;

	//Разлочить буффер - можно рисовать теперь...
	ParticleVB * vbToUpdate = GetUpdateData();

	if (vbToUpdate->pParticlesVerts != NULL)
	{
		vbToUpdate->Unlock();
	}
	

}



bool ParticleService::threadUpdate()
{
	
	//OutputDebugString("ParticleService::threadUpdate\n");

	//Ждем пока не разрешат обновлять...
	WaitForSingleObject(hCanStartUpdate, INFINITE);

	//Может нужно завершить поток (закрывается движок)
	if (InterlockedCompareExchange(&dwNeedCloseThread, 0, 0) != 0)
	{
		//OutputDebugString("hThreadDone - fired. ParticleService::threadUpdate()\n");
		SetEvent(hThreadDone);
		return false;
	}

	//Обновляем тексущий манагер
	Assert(pManagerToExecute);
	pManagerToExecute->Update();

	//Может нужно завершить поток (закрывается движок)
	if (InterlockedCompareExchange(&dwNeedCloseThread, 0, 0) != 0)
	{
		//OutputDebugString("hThreadDone - fired. ParticleService::threadUpdate()\n");
		SetEvent(hThreadDone);
		return false;
	}

	//OutputDebugString("hThreadDone - fired. ParticleService::threadUpdate()\n");
	SetEvent(hThreadDone);
	return true;
}
#endif

