//****************************************************************
//*
//*  Author : Sergey Makeev aka Joker, 2003
//*
//*
//****************************************************************

#ifndef PARTICLES_SERVICE_IMPLEMENTATION
#define PARTICLES_SERVICE_IMPLEMENTATION

#include "..\..\common_h\Particles\iparticlesservice.h"
#include "..\..\common_h\templates.h"
#include "..\..\common_h\render.h"

//#include "..\icommon\gmx_pool.h"

#include "..\system\datasource\DataGraph.h"


#ifdef ENABLE_PARTICLE_THREADS
#pragma message("MULTI-THREADED PARTICLES")
#else
#pragma message("SINGLE-THREADED PARTICLES")
#endif

class ParticleManager;
class IVBuffer;
class IIBuffer;
class DataCacheGlobal;
class BillBoardProcessor;


#define VB_FRAMES_COUNT 2

//Сколько всего может быть плашек обычных
#define MAX_BILLBOARDS_TYPE0 6000

//Сколько всего может быть плашек обычных в XZ плоскости
#define MAX_BILLBOARDS_TYPE1 1000

//Сколько всего может быть плашек distortion 
#define MAX_BILLBOARDS_TYPE2 1000

//Сколько всего может быть плашек distortion в XZ плоскости
#define MAX_BILLBOARDS_TYPE3 500

#define _MAX_BILLBOARDS (MAX_BILLBOARDS_TYPE0+MAX_BILLBOARDS_TYPE1+MAX_BILLBOARDS_TYPE2+MAX_BILLBOARDS_TYPE3)

struct RECT_VERTEX
{
	Vector  vRelativePos;
	Vector4 vColor;
	float   tu1, tv1;
	float   tu2, tv2;
	float angle;
	float BlendK;
	Vector  vParticlePos;
	float AddPowerK;
	float LightPower;
	float AlwaysAmbientK;
};



struct VbMarkup
{
	DWORD particles_count;
	DWORD particlesXZ_count;
	DWORD distortedParticles_count;
	DWORD distortedParticlesXZ_count;

	VbMarkup()
	{
		Clean();
	}

	void Clean()
	{
		particles_count = 0;
		particlesXZ_count = 0;
		distortedParticles_count = 0;
		distortedParticlesXZ_count = 0;
	}
};



struct ParticleVB
{
	IVBuffer* pParticles_VB;
	IVBuffer* pParticlesXZ_VB;

	IVBuffer* pDistortedParticles_VB;
	IVBuffer* pDistortedParticlesXZ_VB;

	RECT_VERTEX * pParticlesVerts;
	RECT_VERTEX * pParticlesXZVerts;
	RECT_VERTEX * pDistortedParticlesVerts;
	RECT_VERTEX * pDistortedParticlesXZVerts;


	VbMarkup markup;

	ParticleVB()
	{
		ClearMarkup();

		pParticlesVerts = NULL;
		pParticlesXZVerts = NULL;
		pDistortedParticlesVerts = NULL;
		pDistortedParticlesXZVerts = NULL;


		pParticles_VB = NULL;
		pParticlesXZ_VB = NULL;

		pDistortedParticles_VB = NULL;
		pDistortedParticlesXZ_VB = NULL;
	}


	void Lock()
	{
		markup.distortedParticles_count = 0;
		markup.distortedParticlesXZ_count = 0;
		markup.particles_count = 0;
		markup.particlesXZ_count = 0;



		pParticlesVerts = (RECT_VERTEX*)pParticles_VB->Lock(0, 0, LOCK_DISCARD);
		pParticlesXZVerts = (RECT_VERTEX*)pParticlesXZ_VB->Lock(0, 0, LOCK_DISCARD);

		pDistortedParticlesVerts = (RECT_VERTEX*)pDistortedParticles_VB->Lock(0, 0, LOCK_DISCARD);
		pDistortedParticlesXZVerts = (RECT_VERTEX*)pDistortedParticlesXZ_VB->Lock(0, 0, LOCK_DISCARD);

		Assert(pParticlesVerts);
		Assert(pParticlesXZVerts);
		Assert(pDistortedParticlesVerts);
		Assert(pDistortedParticlesXZVerts);

	}

	void Unlock()
	{
		pParticles_VB->Unlock();
		pParticlesXZ_VB->Unlock();

		pDistortedParticles_VB->Unlock();
		pDistortedParticlesXZ_VB->Unlock();

		pParticlesVerts = NULL;
		pParticlesXZVerts = NULL;
		pDistortedParticlesVerts = NULL;
		pDistortedParticlesXZVerts = NULL;
	}

	void ClearMarkup()
	{
		markup.Clean();
	}


	void Create(IRender *pRS, DWORD dwCreateFlags)
	{
		DWORD dwMultipler = 4;

#ifdef _XBOX
		dwMultipler = 1;
#endif


		pParticles_VB = pRS->CreateVertexBuffer(MAX_BILLBOARDS_TYPE0 * sizeof(RECT_VERTEX) * dwMultipler, sizeof(RECT_VERTEX), _FL_, dwCreateFlags, POOL_DEFAULT);
		Assert (pParticles_VB != NULL);

		pParticlesXZ_VB = pRS->CreateVertexBuffer(MAX_BILLBOARDS_TYPE1 * sizeof(RECT_VERTEX)* dwMultipler, sizeof(RECT_VERTEX), _FL_, dwCreateFlags, POOL_DEFAULT);
		Assert (pParticlesXZ_VB != NULL);

		pDistortedParticles_VB = pRS->CreateVertexBuffer(MAX_BILLBOARDS_TYPE2 * sizeof(RECT_VERTEX)* dwMultipler, sizeof(RECT_VERTEX), _FL_, dwCreateFlags, POOL_DEFAULT);
		Assert (pDistortedParticles_VB != NULL);

		pDistortedParticlesXZ_VB = pRS->CreateVertexBuffer(MAX_BILLBOARDS_TYPE3 * sizeof(RECT_VERTEX)* dwMultipler, sizeof(RECT_VERTEX), _FL_, dwCreateFlags, POOL_DEFAULT);
		Assert (pDistortedParticlesXZ_VB != NULL);
	}

	void AddRef()
	{
		if (pParticles_VB)
		{
			pParticles_VB->AddRef();
		}

		if (pParticlesXZ_VB)
		{
			pParticlesXZ_VB->AddRef();
		}

		if (pDistortedParticles_VB)
		{
			pDistortedParticles_VB->AddRef();
		}

		if (pDistortedParticlesXZ_VB)
		{
			pDistortedParticlesXZ_VB->AddRef();
		}
	}

	void Release()
	{
		if (pParticles_VB)
		{
			pParticles_VB->Release();
			pParticles_VB = NULL;
		}
		
		if (pParticlesXZ_VB)
		{
			pParticlesXZ_VB->Release();
			pParticlesXZ_VB = NULL;
		}

		if (pDistortedParticles_VB)
		{
			pDistortedParticles_VB->Release();
			pDistortedParticles_VB = NULL;
		}

		if (pDistortedParticlesXZ_VB)
		{
			pDistortedParticlesXZ_VB->Release();
			pDistortedParticlesXZ_VB = NULL;
		}
	}


	ParticleVB & operator = (const ParticleVB & src)
	{
		pParticles_VB = src.pParticles_VB;
		pParticlesXZ_VB = src.pParticlesXZ_VB;

		pDistortedParticles_VB = src.pDistortedParticles_VB;
		pDistortedParticlesXZ_VB = src.pDistortedParticlesXZ_VB;

		pParticlesVerts = src.pParticlesVerts;
		pParticlesXZVerts = src.pParticlesXZVerts;
		pDistortedParticlesVerts = src.pDistortedParticlesVerts;
		pDistortedParticlesXZVerts = src.pDistortedParticlesXZVerts;

		markup = src.markup;

        return *this;
	}

};



class ParticleService : public IParticleService
{
	DWORD dwCurrentFrame;

	

	ParticleVB vbFrames[VB_FRAMES_COUNT];

#ifndef _XBOX
	IIBuffer* pIBuffer;
#endif


	struct CreatedManager
	{
		ParticleManager* pManager;
		string FileName;
		int Line;
	};


	bool sysDelete;
	array<CreatedManager> CreatedManagers;


	DataCacheGlobal* globalDataCache;


	array<GraphVertex> allData;
	objectsPool<DataGraph, 128> DataGraphsPool;

	void systemFreeAllDataForGraphs();

	volatile bool bLowQuality;

	BillBoardProcessor * spriteProcessor;

public:

	__forceinline BillBoardProcessor * getSpriteProcessor()
	{
		return spriteProcessor;
	}


	IIBuffer * GetCurrentIndexBuffer();

	
	ParticleVB * GetUpdateData();


	DataGraph* AllocateDataGraph (FieldList* pMaster);
	void FreeDataGraph (DataGraph* dataGraph);

	array<GraphVertex> * ParticleService::AllocateDataForGraphs (long dwMinGraphSize, long dwMaxGraphSize,
		                                                           long & MinGraphDataStart, long & MinGraphDataSize,
		                                                           long & MaxGraphDataStart, long & MaxGraphDataSize);



//Конструктор / деструктор
	ParticleService ();
	virtual ~ParticleService ();

//Создать менеджер партиклов
	virtual IParticleManager* CreateManagerEx (const char* ProjectName, const char* File, int Line);

	virtual void RemoveManagerFromList (IParticleManager* pManager);

	virtual DWORD GetManagersCount ();
	virtual IParticleManager* GetManagerByIndex (DWORD Index);

#ifdef ENABLE_PARTICLE_THREADS
	bool threadUpdate();
#endif

	bool Init();

	//Исполнение в конце кадра и в начале
	void StartFrame(float dltTime);
	virtual void EndFrame(float dltTime);


	DataCacheGlobal* GetDataCache();

	void LoadAllfromDisk();

	__forceinline bool IsLowQuality()
	{
		return bLowQuality;
	}



#ifdef ENABLE_PARTICLE_THREADS

	ParticleManager * pManagerToExecute;

	//Функция для потока
	static void ThreadExecute(LPVOID lpThreadParameter);

	void StartUpdateThread(ParticleManager * pManager);
	void WaitUntilUpdateFinished();
	//void EndDraw();


	LONG volatile dwNeedCloseThread;

	//поток обновляющий частицы...
	HANDLE hUpdateThread;

	
	//Потоку можно начать обновлять партиклы и строить vertexBuffer...
	HANDLE hCanStartUpdate;


	//Поток закончил считать, можно рисовать...
	HANDLE hThreadDone;



#endif


};


#endif