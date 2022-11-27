//****************************************************************
//*
//*  Author : Sergey Makeev aka Joker, 2003
//*
//*
//****************************************************************

#ifndef PARTICLES_MANAGER_IMPLEMENTATION
#define PARTICLES_MANAGER_IMPLEMENTATION

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "..\..\common_h\core.h"
#include "..\..\common_h\particles\iparticlemanager.h"
#include "..\..\common_h\templates.h"


//#include "..\system\ParticleProcessor\mdl_processor.h"
#include "..\system\ParticleProcessor\bb_processor.h"

#include "..\system\particlesystem\particlesystem.h"
#include "..\system\particlesystem\systemproxy.h"

#include "SyncParams.h"


class ParticleService;
class IFileService;
class IRender;
class IBaseTexture;
class DataCacheGlobal;
class ParticleSystem;
class DataSource;
class IGMXScene;
struct IGMXEntity;






//Менеджер партикловых систем
class ParticleManager : public IParticleManager
{
	struct CacheReloadedInfo
	{
		string Name;
		bool AutoDeleted;

		IGMXScene * AttachedScene;
		GMXHANDLE AttachedToGeom;
		Matrix matWorld;
	};

	bool ShowStat;
	bool GlobalDelete;










	/*

	struct FreeSystem
	{
		dword dwNameHash;
		ParticleSystemProxy* systemPtr;
	};

	array<FreeSystem> FreeCachedSystems;
	

	ParticleSystemProxy* GetFreeSystem (const char* szFileName);
	*/

	//Процессор для партиклов плашек
	BillBoardProcessor* spriteProcessor; 


#ifndef _XBOX
	IVariable* fitToViewport;
#endif

	IVariable* DistortMap;
	IVariable* ScreenSurf;
	

	IVariable* pBaseTexture;
	IVariable* pNormalMap;
	IVariable* DirLightProj;
	IVariable* DirLightProjColor;
	IVariable* LightAmbient;

	IBaseTexture* pHazeTexture;
	IBaseTexture* pProjectTexture;
	IBaseTexture* pNormalMapProjectTexture;
	DataCacheGlobal* pDataCache;

	string ShortProjectName;
	string TextureName;

	//Сервис который родил систему
	ParticleService* pService;

  IFileService* pFS;
  IRender* pRS;

	void DeleteAllSystems ();

	float TimeFromLastStatUpdate;
	DWORD nowTickTime;
	DWORD nowUpdateTime;
	DWORD ActiveSystems;
	DWORD InactiveSystems;
	DWORD ActiveEmitters;


	array<IParticleSystem*> DeleteQuery;


protected:

 virtual ~ParticleManager ();


 __forceinline ParticleSystemProxy* CreateParticleSystemFromDataSource (DataSource* pDataSource)
 {
	 ParticleSystem* pSys = NEW ParticleSystem(getBBProcessor());
	 ParticleSystemProxy* pSysProxy = NEW ParticleSystemProxy(this, pSys);

	 //api->Trace("Born proxy: 0x%08X", pSysProxy);

	 pSys->CreateFromDataSource(pDataSource);

	 ThreadParams.SystemsCreated.Add(pSys);
	 ThreadParams.SystemsProxy.Add(pSysProxy);
	 return pSysProxy;
 }


 DWORD IteratorIndex;
 array<string> EnumUsedGeom;

 bool FindInEnumUsedGeom (const char* GeomName);

public:

	virtual bool Draw ();
	virtual bool DrawSimpleParticles (bool bSoftParticlesSupport);

//Создание/удаление 
 ParticleManager (ParticleService* service);
 virtual bool Release ();

 virtual IFileService* Files ();
/*
//Открыть проект 
 virtual bool OpenProject (const char* FileName);
//Закрыть проект 
 virtual void CloseProject ();
 */
 
//Удалить из списка ресурсов (системная)
 virtual void RemoveResource (IParticleSystem* pResource);

//Исполнить партиклы 
 virtual void Execute (float DeltaTime);

//Узнать доступна система или нет 
 virtual bool IsSystemAvailable (const char* FileName);

 /*
//Получить глобальную текстуру проекта 
 virtual IBaseTexture* GetProjectTexture ();
//Установить текстуру проекта
 virtual void SetProjectTexture (const char* FileName = NULL);
 

//Получить имя проекта 
 virtual const char* GetProjectFileName ();
 */

 virtual bool DrawAllParticles ();

 
 //Создать партикловую систему из файла (файл должен быть в проекте!!!!!)
 virtual IParticleSystem* CreateParticleSystemEx (const char* FileName, const char* File, int Line);

 virtual IParticleSystem* CreateParticleSystemEx2 (const char* FileName, const Matrix& mWorld, bool bAutoDelete, const char* File, int Line);

 //Создать пустую партикловую систему, для редактора...
 virtual IParticleSystem* CreateEmptyParticleSystemEx  (const char* FileName, int Line);

 BillBoardProcessor * getBBProcessor ();

 virtual bool ValidateSystem (IParticleSystem* pSystem);


 DWORD GetCreatedSystemCount ();
 ParticleSystemProxy* GetCreatedSystemByIndex (DWORD Index);


 void DefferedDelete (IParticleSystem* pSys);

 //virtual const char* GetProjectTextureName ();

 virtual void Editor_UpdateCachedData ();

 virtual DWORD GetProjectSystemCount ();
 virtual const char* GetProjectSystemName (DWORD Index);

 virtual const char* GetFirstGeomName (const char* FileName);
 virtual const char* GetNextGeomName ();

 

 virtual void WriteSystemCache (const char* FileName);
 virtual void WriteSystemCache (const char* FileName, MemFile* pMemFile);
 virtual void WriteSystemCacheAs (const char* FileName, const char* NewName);
 
 virtual void LoadSystemCache (const char* FileName, MemFile* pMemFile);
 virtual void CacheReloaded ();

 virtual FieldList* Editor_CreatePointEmitter (const char* SystemName, const char* EmitterName);
 virtual FieldList* Editor_CreateBillBoardParticle (const char* SystemName, const char*EmitterName, const char* ParticleName);
 virtual FieldList* Editor_CreateModelParticle (const char* SystemName, const char*EmitterName, const char* ParticleName);

 virtual void DeletePointEmitter (const char* SystemName, IEmitter* pEmitter);
 virtual void DeleteBillboard (const char* SystemName, IEmitter* pEmitter, FieldList* pParticles);
 virtual void DeleteModel (const char* SystemName, IEmitter* pEmitter, FieldList* pParticles);


 dword RegisterForceField (const char* szName, Object* _class, FORCEFIELD_PROCESS _method);
 void UnregisterForceField (Object* _class, FORCEFIELD_PROCESS _method);
 virtual void UnregisterAllForceFields(Object* _class);

 int GetForceFieldIndex (const char* szName);

 virtual dword GetForceFieldUniqueID (const char* szName);

 virtual bool ExecuteForceFields (dword dwGUID1, dword dwGUID2, dword dwGUID3, dword dwGUID4, Vector& old_pos, Vector& pos, Vector& externalforce);


 virtual void LoadAllSystems ();

 virtual void ReloadSystems ();

 virtual void AddDeleteTime (double time);


 
 bool Update ();

#ifdef ENABLE_PARTICLE_THREADS
 void Lock ();
 void Unlock ();
#endif


private:

	

	


	struct RegistredForceFields
	{
		dword dwNameHash;
		string Name;
		Object* pClass;
		FORCEFIELD_PROCESS pMethod;
		dword GUID;
	};

	array<RegistredForceFields> ForceFields;


	float m_time;
	float m_fixedFPS;
 
	
	double createTime;
	
	dword createCount;

	double deleteTime;
	dword deleteCount;


	double maxCreateTime;
	double maxDeleteTime;

	dword MaxnowTickTime;

	dword framesPassed;



	float fTime;
	float fCreatedCachedSystems;
	float fCreatedSystems;
	float fDeletedSystems;

	//Если ли в менеджере созданные системы
	BOOL bIsActiveManager;


	IRenderTarget* tempRTForDistorsion;


#ifdef _XBOX
	IRenderTargetDepth* tempRTDForSoftParticles;
	IVariable* depthMap;
	IVariable* particleSoftness;
	IVariable* mProjInverse;
	

	float fSoftness;

#endif




private:

	SyncParams ThreadParams;





#ifdef ENABLE_PARTICLE_THREADS

	CritSection sync_access_to_array;

#endif

};


#endif