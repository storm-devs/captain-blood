

#include "PhysicsService.h"
#include "PhysicsScene.h"
#include "..\Common_h\Render.h"
#include "..\Common_h\FileService.h"
#include "..\Common_h\LocStrings.h"
#include "..\Common_h\SetThreadName.h"
#include "PhysCharacter.h"
#include "PhysRigidBody.h"
#include "PhysCloth.h"
#include "PhysTriangleMesh.h"
#include "PhysRagdoll.h"
#include "NxCooking.h"
#include "ClothMeshBuilderProxy.h"
#include "IProxy.h"
#include "Deferrer.h"

//#define NOMINMAX		//suppress windows' global min,max macros.
//============================================================================================

INTERFACE_FUNCTION
CREATE_SERVICE(PhysicsService, 19)

#ifndef _XBOX
#pragma comment(lib, "PhysXLoader.lib")
#endif

#pragma comment(lib, "NxCharacter.lib")
#pragma comment(lib, "NxCooking.lib")

#ifndef STOP_DEBUG
	bool PhysicsService::m_gShowColliders = false;
	bool PhysicsService::m_gShowPhysBoxes = false;
	long PhysicsService::m_gMaterialIndex = -1;
	bool PhysicsService::m_gShowWorldPoint = false;
	Vector PhysicsService::m_gWorldPoint = 0.0f;
#endif

//============================================================================================

PhysicsService::PhysicsService() :	scenes(_FL_),
									meshes(_FL_),
									builders(_FL_),
									m_scenes2Execute(_FL_, 16)
{
	physicsSDK = null;	
	console = null;
	m_ctrManager = null;
	isHardware = false;
	isEnableDebug = false;
	isStop = false;
	isMultiThreading = false;

	m_hThread = null;
	m_hSimulateStartEvent = null;
	m_hExitEvent = null;

	MemPool::ptr = NEW MemPool(); Assert(MemPool::ptr);
}

PhysicsService::~PhysicsService()
{
	if (isMultiThreading)
	{
		SetEvent(m_hExitEvent);
		for (int i=0; i<5; i++)
		{
			if ( WaitForSingleObject(m_hThread, 2000) == WAIT_OBJECT_0 )
				break;
		
			api->Trace("PhysicsService: Killing thread problem!");
		}

		CloseHandle(m_hThread);
		CloseHandle(m_hSimulateStartEvent);
		CloseHandle(m_hExitEvent);
	}

	if (m_ctrManager)
		NxReleaseControllerManager(m_ctrManager);

	while(scenes)
	{
		scenes[0]->Release();
	}
	while(meshes)
	{
		meshes[0]->Release();
	}
	if(physicsSDK)
	{
		physicsSDK->release();
	}
	physicsSDK = null;

	DELETE(MemPool::ptr);
}

//Создать сцену
IPhysicsScene * PhysicsService::CreateScene()
{
	return NEW PhysicsScene(this);
}

//Текущий режим работы
bool PhysicsService::IsHardware()
{
	return isHardware;
}

//Использование мультипоточности
bool PhysicsService::IsMultiThreading()
{
	return isMultiThreading;
}

//Текущий режим дебаг инфы
bool PhysicsService::IsEnableDebug()
{
	return isEnableDebug;
}

//Создать сетку, основываясь на бинарных данных
IPhysTriangleMesh * PhysicsService::CreateTriangleMesh(const void * meshData, dword meshDataSize, const void * pMapData, dword pMapDataSize)
{
	MemoryReadStream memoryStream(meshData, meshDataSize);

	// останавливаем отдельный поток с физикой
	csSimulate.Enter();
		NxTriangleMesh * trgMesh = physicsSDK->createTriangleMesh(memoryStream);
	csSimulate.Leave();

	if(!trgMesh) 
		return null;

	/*if(pMapData && pMapDataSize)
	{
		NxPMap pmap;
		pmap.data = (void *)pMapData;
		pmap.dataSize = pMapDataSize;
		trgMesh->loadPMap(pmap);
	}*/
	PhysTriangleMesh * mesh = NEW PhysTriangleMesh(trgMesh, this);
	meshes.Add(mesh);
	return mesh;
}

//Создать построитель сеток ткани
IClothMeshBuilder* PhysicsService::CreateClothMeshBuilder()
{
/*#ifdef USE_THREADING
	ClothMeshBuilderProxy* pBuilder = NEW ClothMeshBuilderProxy(physicsSDK);
#else*/
	ClothMeshBuilder* pBuilder = NEW ClothMeshBuilder(physicsSDK);
/*#endif*/

	Assert(pBuilder);
	builders.Add(pBuilder);
	return pBuilder;
}

//Удаление сетки из списка
void PhysicsService::UnregistryPhysTriangleMesh(IPhysTriangleMesh * obj)
{
	NxTriangleMesh * msh = ((PhysTriangleMesh *)obj)->triangleMesh;
	NxU32 refcnt = msh->getReferenceCount();
	Assert(msh);
	physicsSDK->releaseTriangleMesh(*msh);
	meshes.Del(obj);
}

//Удаление сетки из списка
void PhysicsService::UnregistryClothMeshBuilder(IClothMeshBuilder * obj)
{
	builders.Del(obj);
}

#ifndef STOP_DEBUG
IProxy * FindProxyObject(NxActor * actor)
{
	PhysicsService * srv = (PhysicsService*)api->GetService("PhysicsService");
	return srv->FindProxyObject(actor);
}

//Ищем прокси объект во всех сценах с данным актером
IProxy * PhysicsService::FindProxyObject(NxActor * actor)
{
	for (long i=0; i<scenes; i++)
	{
		IProxy * object = scenes[i]->FindProxyObject(actor);
		if (object) 
			return object;
	}
	return null;
}
#endif

//Инициализация
bool PhysicsService::Init()
{
	IFileService * fs = (IFileService *)api->GetService("FileService");

	IIniFile * pIni = fs->SystemIni();
	NxReal enableDebug = (NxReal)0;
	isEnableDebug = false;
	isHardware = false;
	bool isEnabledWarnings = false;
	string debugHost;

	if (pIni)
	{
#ifndef STOP_DEBUG
		isEnableDebug = pIni->GetLong("Physics", "debug", 0) != 0;
		enableDebug = (isEnableDebug) ? (NxReal)1 : (NxReal)0;
		debugHost = pIni->GetString("Physics", "debughost", "");
#endif
		isHardware = pIni->GetLong("Physics", "hw", 0) != 0;
		isStop = pIni->GetLong("Physics", "stop", 0) != 0;
		isMultiThreading = (api->GetThreadingInfo() != ICore::mt_none);
		isEnabledWarnings = pIni->GetLong("Physics", "warnings", 0) != 0;

		RELEASE(pIni);
	}

	errorStream.EnableWarnings(isEnabledWarnings);

	//Создаём объект
	physicsSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, &allocator, &errorStream);

	dword version = NX_PHYSICS_SDK_VERSION;

	if (!physicsSDK) 
	{
		Error(1000200, "PhysicsService: Can't init PhysX SDK");
		return false;
	}

	m_ctrManager = NxCreateControllerManager(&allocator);

	NxInitCooking();

#ifndef STOP_DEBUG
	if(!debugHost.IsEmpty())
	{
		physicsSDK->getFoundationSDK().getRemoteDebugger()->connect(debugHost);
	}	
#endif
	
	physicsSDK->setParameter(NX_SKIN_WIDTH, 0.01f);	
	physicsSDK->setParameter(NX_DEFAULT_SLEEP_LIN_VEL_SQUARED, 0.2f*0.2f);
	physicsSDK->setParameter(NX_DEFAULT_SLEEP_ANG_VEL_SQUARED, 0.15f*0.15f);
	physicsSDK->setParameter(NX_VISUALIZATION_SCALE, 0.25f * enableDebug);
	physicsSDK->setParameter(NX_VISUALIZE_COLLISION_SHAPES, enableDebug);
	physicsSDK->setParameter(NX_VISUALIZE_JOINT_LIMITS, enableDebug);
	physicsSDK->setParameter(NX_VISUALIZE_JOINT_LOCAL_AXES, enableDebug);
	physicsSDK->setParameter(NX_STA_FRICT_SCALING, 1.0f);
	physicsSDK->setParameter(NX_DYN_FRICT_SCALING, 1.0f);

	// Set the physics parameters

	// Set the debug visualization parameters
	//physicsSDK->setParameter(NX_VISUALIZATION_SCALE, 1);
	//physicsSDK->setParameter(NX_VISUALIZE_COLLISION_SHAPES, 1);
	//physicsSDK->setParameter(NX_VISUALIZE_JOINT_LOCAL_AXES, 1);

	//physicsSDK->setParameter(NX_VISUALIZE_JOINT_FORCE, enableDebug);
	//physicsSDK->setParameter(NX_VISUALIZE_ACTOR_AXES, enableDebug);
	//physicsSDK->setParameter(NX_VISUALIZE_JOINT_WORLD_AXES, enableDebug);
	//physicsSDK->setParameter(NX_VISUALIZE_JOINT_LOCAL_AXES, enableDebug);
	//physicsSDK->setParameter( NX_VISUALIZE_JOINT_LOCAL_AXES, 1);
	//physicsSDK->setParameter(NX_VISUALIZE_JOINT_LIMITS, 1);
	//physicsSDK->setParameter(NX_VISUALIZE_COLLISION_EDGES, enableDebug);	
	//physicsSDK->setParameter(NX_CONTINUOUS_CD, 1.0f);

	api->SetStartFrameLevel(this, Core_DefaultExecuteLevel);
	api->SetEndFrameLevel(this, Core_DefaultExecuteLevel+0x1000);

	if (isMultiThreading)
	{
		m_hSimulateStartEvent = CreateEvent(NULL, FALSE, FALSE, NULL);	Assert(m_hSimulateStartEvent);
		m_hExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);			Assert(m_hExitEvent);

		// стартуем аццкий поток
		DWORD dwPhysicsThreadID = 0;
		m_hThread = CreateThread(NULL, 512 * 1024, (LPTHREAD_START_ROUTINE)&SimulationThread, this, CREATE_SUSPENDED, &dwPhysicsThreadID);
		Assert(m_hThread);
#ifdef _XBOX
		::XSetThreadProcessor(m_hThread, 2);
		::SetThreadPriority(m_hThread, THREAD_PRIORITY_HIGHEST);
#endif
		XSetThreadName(dwPhysicsThreadID, "Physics::Simulate");
		ResumeThread(m_hThread);
	}

	api->Trace("Physics started in %s mode", (isMultiThreading) ? "multithread" : "singlethread");

	return true;
}

void PhysicsService::Error(long id, const char * errorEnglish)
{
	ILocStrings * locStrings = (ILocStrings*)api->GetService("LocStrings");
	ICoreStorageString * storage = api->Storage().GetItemString("system.error", _FL_);

	const char * errorString = (locStrings) ? locStrings->GetString(id) : null;
	storage->Set((errorString) ? errorString : errorEnglish);
	storage->Release();
}

//Исполнение в начале кадра
void PhysicsService::StartFrame(float dltTime)
{
#if !defined(STOP_DEBUG) && !defined(_XBOX)
	if (!console)
	{
		console = (IConsole *)api->GetService("Console");
		if(console)
		{
			console->Register_PureC_Command("sc", "sc [mat index, -1 for all mats] Show/hide all colliders", &Console_ShowColliders);
			console->Register_PureC_Command("spb", "Show/hide all phys objects boxes", &Console_ShowPhysBoxes);
			console->Register_PureC_Command("null", "null [x,y,z] Show/hide pointer to world point(no parameters = 0, 0, 0)", &Console_ShowNull);
		}
	}
#endif

	// Очищаем очередь на выполнение в начале кадра
	csArrayUpdate.Enter();
		m_scenes2Execute.Empty();
	csArrayUpdate.Leave();

}

#ifndef STOP_DEBUG
void _cdecl PhysicsService::Console_ShowNull(const ConsoleStack & params)
{
	if (!params.GetSize())
	{
		m_gShowWorldPoint ^= 1;
		m_gWorldPoint = 0.0f;
	}
	else
	{
		if (params.GetSize() == 1)
		{
			m_gShowWorldPoint = true;
			m_gWorldPoint = float(atof(params.GetParam(0)));
		}
		else
		{
			m_gShowWorldPoint = true;
			m_gWorldPoint.x = float(atof(params.GetParam(0)));
			m_gWorldPoint.y = float(atof(params.GetParam(1)));
			m_gWorldPoint.z = float(atof(params.GetParam(2)));
		}
	}
}

void _cdecl PhysicsService::Console_ShowColliders(const ConsoleStack & params)
{
	if (!params.GetSize())
	{
		m_gShowColliders ^= 1;
		m_gMaterialIndex = -1;
	}
	else
	{
		m_gShowColliders = true;
		m_gMaterialIndex = atol(params.GetParam(0));
	}
}

void _cdecl PhysicsService::Console_ShowPhysBoxes(const ConsoleStack & params)
{
	m_gShowPhysBoxes ^= 1;
}

void PhysicsService::DrawSceneStat(unsigned int index)
{
	if (scenes.Size() <= index)
		return;

	/*IRender * rs = (IRender *)api->GetService("DX9Render");
	scenes[index]->DebugDraw(*rs);
	return;*/

	api->Trace("");
	api->Trace("-----------------------------------------------------");
	api->Trace("Physics statistics. Scene #%d", index);
	api->Trace("Name = Current, Max");

	const NxSceneStats2 * stats = scenes[index]->Scene().getStats2();
	unsigned int count = 0;
	for (unsigned int i = 0; i < stats->numStats; ++i)
	{
		if (stats->stats[i].parent == 0xFFFFFFFF )
			api->Trace("%s = %d, %d", stats->stats[i].name, stats->stats[i].curValue, stats->stats[i].maxValue);
		else
			api->Trace("    %s = %d, %d", stats->stats[i].name, stats->stats[i].curValue, stats->stats[i].maxValue);

		++count;
	}
	
	api->Trace("-----------------------------------------------------");
	api->Trace("");
}
#endif

//Исполнение в конце кадра
void PhysicsService::EndFrame(float dltTime)
{
#ifndef STOP_DEBUG
	if ( api->DebugKeyState(VK_MENU, VK_F5) )
		for (unsigned int i = 0; i < scenes.Size(); ++i)
			DrawSceneStat(i);
#endif
}

void PhysicsService::AddScene2Execute(PhysicsScene * scene)
{
	csArrayUpdate.Enter();

	m_scenes2Execute.Add(scene);

	csArrayUpdate.Leave();

	SetEvent(m_hSimulateStartEvent);
}

// ВНИМАНИЕ! функция рабочего потока
dword __stdcall PhysicsService::SimulationThread(void * ptr)
{
	PhysicsService * svc = (PhysicsService *)ptr;

	HANDLE waits[] = {svc->m_hSimulateStartEvent, svc->m_hExitEvent};
	
	while (true)
	{
		DWORD waitResult = WaitForMultipleObjects(ARRSIZE(waits), waits, FALSE, INFINITE );
		if (waitResult == (WAIT_OBJECT_0 + 1))
			break;

		svc->csSimulate.Enter();
		while (true)
		{
			svc->csArrayUpdate.Enter();

				PhysicsScene * scene = null;
				if (svc->m_scenes2Execute.Len())
				{
					scene = svc->m_scenes2Execute[ svc->m_scenes2Execute.Last() ];
					
					svc->m_scenes2Execute.Extract(svc->m_scenes2Execute.Last());
				}
				
			svc->csArrayUpdate.Leave();

			if (!scene)
				break;

			scene->Scene().simulate( scene->GetLastDeltaTime() );
			scene->Scene().flushStream();

			scene->SimulationDone();
		}
		svc->csSimulate.Leave();
	}
	return 0;
}


//============================================================================================
//PhysicsService::Allocator
//============================================================================================

void * PhysicsService::Allocator::malloc(NxU32 size)
{
	return api->Reallocate(null, size, _FL_);
}

void * PhysicsService::Allocator::mallocDEBUG(NxU32 size,const char * fileName, int line)
{
	return api->Reallocate(null, size, fileName, line);
}

void * PhysicsService::Allocator::realloc(void * memory, NxU32 size)
{
	return api->Reallocate(memory, size, _FL_);
}

void PhysicsService::Allocator::free(void * memory)
{
	api->Free(memory, _FL_);
}

