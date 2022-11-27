#include "particlemanager.h"
#include "..\..\common_h\render.h"
#include "..\..\Common_h\FileService.h"
#include "..\..\common_h\core.h"
#include "..\..\common_h\corecmds.h"

#include "..\datacache\datacache.h"
#include "..\service\particleservice.h"

#include "..\system\particleprocessor\nameparser.h"
#include "..\icommon\iemitter.h"
#include "..\system\datasource\datastring.h"

#include "..\TextFile.h"




// это надо добавить в свойства проекта !!!!
//#define USE_XML_FILE_FORMAT


DWORD GraphRead = 0;
DWORD OptGraphRead = 0;

DWORD GlobalUniqueForceFieldIndex = 0;


#define DONT_COLLECT_STATS_FRAMES 30




#ifdef _XBOX
#pragma comment(lib, "Xapilibi")
#pragma comment(lib, "Xbdm")

#endif





ParticleManager::ParticleManager (ParticleService* service) : IParticleManager (service),
                                                              DeleteQuery (_FL_),
   															  EnumUsedGeom(_FL_),
															  ForceFields(_FL_)
{
	AssertCoreThread

	bIsActiveManager = false;
	fTime = 0.0f;
	fCreatedSystems = 0.0f;
	fDeletedSystems = 0.0f;
	fCreatedCachedSystems = 0.0f;

	framesPassed = 0;
	MaxnowTickTime = 0;
	maxCreateTime = 0.0;
	maxDeleteTime = 0.0;

	deleteTime = 0.0;
	deleteCount = 0;


	createTime = 0.0;
	createCount = 0;

	m_fixedFPS = 1.0f / 30.0f;
	m_time = 0.0f;
	pService = service;
	ShowStat = false;
	
	spriteProcessor = service->getSpriteProcessor(); 


	GlobalDelete = false;
	TimeFromLastStatUpdate = 100.0f;


	pFS = (IFileService*)api->GetService("FileService");
	Assert (pFS != NULL);
	pRS = (IRender*)api->GetService("DX9Render");
	Assert (pRS != NULL);

	pDataCache = service->GetDataCache();
	
	pProjectTexture = NULL;
	pHazeTexture = NULL;
	pNormalMapProjectTexture = NULL;


	string FileName = "particles";


	tempRTForDistorsion = pRS->CreateTempRenderTarget(TRS_SCREEN_FULL_3D, TRC_FIXED_RGBA_8, _FL_);

#ifdef _XBOX
	tempRTDForSoftParticles = pRS->CreateTempDepthStencil(TRS_SCREEN_FULL_3D, _FL_, 0);
	depthMap = pRS->GetTechniqueGlobalVariable("DepthMap", _FL_);
	particleSoftness = pRS->GetTechniqueGlobalVariable("fParticleSoftness", _FL_);
	mProjInverse = pRS->GetTechniqueGlobalVariable("mProjInverse", _FL_);
	fSoftness = 1.0f;
#else
	fitToViewport = pRS->GetTechniqueGlobalVariable("fitToViewport", _FL_);
#endif

	
	
	pProjectTexture = pRS->CreateTexture(_FL_, FileName.c_str());
	pNormalMapProjectTexture = pRS->CreateTexture(_FL_, "%snm", FileName.c_str());
	pHazeTexture = pRS->CreateTexture(_FL_, "haze");
	
	TextureName = FileName;


	pBaseTexture = pRS->GetTechniqueGlobalVariable("tParticlesTexture", _FL_);
	pNormalMap = pRS->GetTechniqueGlobalVariable("tParticlesNormalMap", _FL_);

	DistortMap = pRS->GetTechniqueGlobalVariable("tDistortMap", _FL_);
	ScreenSurf = pRS->GetTechniqueGlobalVariable("tScreenSurf", _FL_);


	DirLightProj = pRS->GetTechniqueGlobalVariable("LightDirProjection", _FL_);
	DirLightProjColor = pRS->GetTechniqueGlobalVariable("LightDirProjectionColor", _FL_);

	LightAmbient = pRS->GetTechniqueGlobalVariable("LightAmbientParticles", _FL_);


	for (dword n = 0; n < 32; n++)
	{
		ThreadParams.planes[n] = Plane(0.0f);
	}
	ThreadParams.fDeltaTime = 0.0f;

#ifdef _XBOX
	OutputDebugString("ParticleManager ctor\n");
#endif
	
}




ParticleManager::~ParticleManager ()
{
	AssertCoreThread



		

	OutputDebugString("ParticleManager::~ParticleManager ---------------------------------\n");
	
#ifdef _XBOX
	if (tempRTDForSoftParticles)
	{
		tempRTDForSoftParticles->Release();
		tempRTDForSoftParticles = NULL;
	}

#endif




	if (tempRTForDistorsion)
	{
		tempRTForDistorsion->Release();
		tempRTForDistorsion = NULL;
	}

	if (fTime > 0.001f)
	{
		api->Trace("Particles cache create(%f)  freq: %f systems per second", fCreatedCachedSystems, fCreatedCachedSystems / fTime);
		api->Trace("Particles create(%f)  freq: %f systems per second", fCreatedSystems, fCreatedSystems / fTime);
		api->Trace("Particles destroy(%f) freq: %f systems per second", fDeletedSystems, fDeletedSystems / fTime);
	}

	pBaseTexture = NULL;
	pNormalMap = NULL;
	DistortMap = NULL;
	ScreenSurf = NULL;
	DirLightProj = NULL;
	LightAmbient = NULL;
	DirLightProjColor = NULL;
	
	DeleteAllSystems ();
	if (pProjectTexture) pProjectTexture->Release();
	pProjectTexture = NULL;

	if (pHazeTexture) pHazeTexture->Release();
	pHazeTexture = NULL;


	if (pNormalMapProjectTexture) pNormalMapProjectTexture->Release();
	pNormalMapProjectTexture = NULL;
	

	pDataCache = NULL;

	spriteProcessor = NULL;

	pService->RemoveManagerFromList(this);
}

#ifdef ENABLE_PARTICLE_THREADS

void ParticleManager::Lock ()
{
	sync_access_to_array.Enter();
}

void ParticleManager::Unlock ()
{
	sync_access_to_array.Leave();
}

#endif


bool ParticleManager::Release ()
{
	AssertCoreThread

	delete this;
	return true;
}

IFileService* ParticleManager::Files ()
{
	AssertCoreThread

	return pFS;
}

//Установить текстуру проекта
/*
void ParticleManager::SetProjectTexture (const char* FileName)
{
	if (pProjectTexture)
	{
		pProjectTexture->Release();
		pProjectTexture = NULL;
	}

	if (pNormalMapProjectTexture)
	{
		pNormalMapProjectTexture->Release();
		pNormalMapProjectTexture = NULL;
	}

	pProjectTexture = pRS->CreateTexture(_FL_, FileName);

	
	string a;
	a.GetFileTitle(FileName);
	pNormalMapProjectTexture = pRS->CreateTexture(_FL_, "%snm", a.c_str());
	TextureName = FileName;
}

const char* ParticleManager::GetProjectTextureName ()
{
	return TextureName.GetBuffer();
}
*/

//Открыть проект 
/*
bool ParticleManager::OpenProject (const char* FileName)
{
  CloseProject ();
	ShortProjectName = FileName;

	string LongFileName = "resource\\particles\\";
	LongFileName += FileName;
	LongFileName.AddExtention(".prj");

	IIniFile* IniFile = pFS->OpenIniFile(LongFileName.GetBuffer(), _FL_);
	if (!IniFile)
	{
		api->Trace("Can't find project '%s'", LongFileName.GetBuffer());
		return false;
	}


	//Устанавливаем текстуру проекта...
	const char* szProjTexture = IniFile->GetString("Textures", "MainTexture", "none");
	//api->Trace("Manager use texture: %s", IniStringBuffer);
	SetProjectTexture (szProjTexture);

	//Загружаем данные 
	for (int n = 0; n < 9999; n++)
	{
		string Section;
		Section.Format("System_%04d", n);
		const char* szSys = IniFile->GetString("Manager", (char*)Section.GetBuffer(), NULL);
		if (!szSys) break;
		pDataCache->CacheSystem(szSys);
	}

	IniFile->Release();

	//FIX ME !
	//Если будет асинхронная загрузка это неверно...
	CreateGeomCache();

	return true;
}
*/

/*
//Получить имя проекта 
const char* ParticleManager::GetProjectFileName ()
{
	return ShortProjectName.GetBuffer();
}


//Закрыть проект 
void ParticleManager::CloseProject ()
{
	BB_Processor->Clear ();
	MDL_Processor->Clear ();
	DeleteAllSystems ();
	if (pProjectTexture) pProjectTexture->Release();
	if (pNormalMapProjectTexture) pNormalMapProjectTexture->Release();
	pProjectTexture = NULL;
	pNormalMapProjectTexture = NULL;
	pDataCache->ResetCache();
	pGeomCache->ResetCache();
}
*/
 
//Удалить из списка ресурсов (системная)
void ParticleManager::RemoveResource (IParticleSystem* pResource)
{
	AssertCoreThread

	fDeletedSystems += 1.0f;


	if (GlobalDelete) return;
	
	for (DWORD n = 0; n < ThreadParams.SystemsProxy.Size(); n++)
	{
		ParticleSystemProxy* sys = ThreadParams.SystemsProxy[n];
		if (sys == pResource)
		{
			ParticleSystemProxy* ptrToDelete = sys;

			ParticleSystem* guid = ptrToDelete->system_GetGUID_dontUSE_for_WORK_use_LOCK();

			//Вдруг ее создали на этом кадре и убили, до конца кадра - надо ее почистить
			//================================================================================
			for (dword q = 0; q < ThreadParams.SystemsCreated.Size(); q++)
			{
				if (ThreadParams.SystemsCreated[q] == guid)
				{
					ThreadParams.SystemsCreated.ExtractNoShift(q);
					break;
				}
			}
			//================================================================================

			ThreadParams.SystemsDeleted.Add(guid);
			ThreadParams.SystemsProxy.ExtractNoShift(n);

			delete ptrToDelete;

			return;
		}
	}

#ifdef ENABLE_PARTICLE_THREADS
	Unlock();
#endif
	Assert(!"Can't find resource for delete !!!");
}

bool ParticleManager::Draw ()
{
	AssertCoreThread

	if (!bIsActiveManager)
	{
		return false;
	}


#ifdef ENABLE_PARTICLE_THREADS
	pService->WaitUntilUpdateFinished();
#endif

	bool bRes = DrawSimpleParticles(false);




	return bRes;
}


bool ParticleManager::DrawAllParticles ()
{
	AssertCoreThread

	if (!bIsActiveManager)
	{
		return false;
	}


#ifdef ENABLE_PARTICLE_THREADS
	pService->WaitUntilUpdateFinished();
#endif


/*
	ParticleVB * renderData = pService->GetUpdateData();
	pRS->Print(0, 350, 0xFFFFFFFF, "p: %d", renderData->markup.particles_count);
*/
	

	bool bHaveDistorsion = DrawSimpleParticles(true);

	if (bHaveDistorsion)
	{
		//завершаем текущую сцену и копируем содержимое в distortTex
		//нужна temp текстура с экран
		IBaseTexture* tempTexture = tempRTForDistorsion->AsTexture ();
		pRS->ResolveColor(tempTexture);

		dword dwPartIDDist = pRS->pixBeginEvent(_FL_, "ParticleManager::DrawDistorsion");

		DistortMap->SetTexture(pHazeTexture);
		ScreenSurf->SetTexture(tempTexture);
		pBaseTexture->SetTexture(pHazeTexture);
		pNormalMap->SetTexture(pHazeTexture);

		RENDERSCREEN scr = pRS->GetScreenInfo3D();
		float fWidth = (float)scr.dwWidth;
		float fHeight = (float)scr.dwHeight;

		RENDERVIEWPORT rvp = pRS->GetViewport();
		Vector4 var;

		var.x = rvp.X / fWidth;
		var.y = rvp.Y / fHeight;
		var.z = rvp.Width / fWidth;
		var.w = rvp.Height / fHeight;


		//Матрица масштабирования
		Matrix mInvertY;
		mInvertY.BuildScale(Vector(1.0, -1.0, 1.0f));

		//Матрица приводящая к диапазону 0..1
		Matrix mNormalization;
		mNormalization.BuildScale(Vector(0.5f, 0.5f, 1.0f));
		mNormalization = (mInvertY * mNormalization) * Matrix(Vector(0.0f), Vector(0.5f, 0.5f, 1.0f));

		Matrix mFitToViewport;
		mFitToViewport.BuildScale(Vector(var.z, var.w, 1.0f));
		mFitToViewport = mFitToViewport * Matrix(Vector(0.0f), Vector(var.x, var.y, 1.0f));

		ParticleVB * renderData = pService->GetUpdateData();
		IIBuffer * pIndexBuffer = pService->GetCurrentIndexBuffer();
		spriteProcessor->DrawDistorted(*renderData, pIndexBuffer);


		pRS->pixEndEvent(_FL_, dwPartIDDist);
	} //draw distorted particles


	TimeFromLastStatUpdate += api->GetDeltaTime();
	if (TimeFromLastStatUpdate > 1.0f)
	{
		TimeFromLastStatUpdate = 0.0f;
		nowTickTime = 0;
		nowUpdateTime = 0;
	}

	if (framesPassed > DONT_COLLECT_STATS_FRAMES)
	{
		MaxnowTickTime = coremax (MaxnowTickTime, nowTickTime);
	}

	if (ShowStat)
	{
		/*
		IFont* pSysFont = pRS->GetSystemFont ();
		float Width = pSysFont->GetLength("Graph read count - %d", GraphRead);
		pSysFont->Release();
		*/

		RS_SPRITE spr[4];
		float x1 = -1.0f;
		float x2 = ((290.0f / (float)pRS->GetViewport().Width) * 2) - 1.0f;
		float y1 = 1.0f - ((16.0f / (float)pRS->GetViewport().Height)*2.0f);
		float y2 = 1.0f - ((150.0f / (float)pRS->GetViewport().Height)* 2.0f);
		spr[0].vPos = Vector(x1, y1, 0.2f);
		spr[1].vPos = Vector(x2, y1, 0.2f);
		spr[2].vPos = Vector(x2, y2, 0.2f);
		spr[3].vPos = Vector(x1, y2, 0.2f);
		pRS->DrawSprites(NULL, spr, 1, "dbgInfoSprite");

		float posY = 100;

		ParticleVB * renderData = pService->GetUpdateData();
		//pRS->Print(0, 350, 0xFFFFFFFF, "p: %d", renderData->markup.particles_count);


		

		pRS->Print(0, posY+0, 0xFFFFFFFF, "PCount:%d", renderData->markup.particles_count);
		pRS->Print(0, posY+16, 0xFFFFFFFF, "Systems - act:%d inact:%d", ActiveSystems, InactiveSystems);
		pRS->Print(0, posY+32, 0xFFFFFFFF, "Emitters - %d", ActiveEmitters);
		pRS->Print(0, posY+80, 0xFFFFFFFF, "Total time - %d, max: %d", nowTickTime, MaxnowTickTime);
		pRS->Print(0, posY+96, 0xFFFFFFFF, "Update time - %d, Solver Iterations - %d", nowUpdateTime, 0);
		if (createCount > 0) pRS->Print(0, posY+144, 0xFFFFFFFF, "Max create time %3.2f, Average create time %3.2f, create count %d", maxCreateTime, createTime / (double)createCount, createCount);
		if (deleteCount > 0) pRS->Print(0, posY+160, 0xFFFFFFFF, "Max delete time %3.2f, Average delete time %3.2f, delete count %d", maxDeleteTime, deleteTime / (double)deleteCount, deleteCount);



		float ReadsPerParticle = 0.0f;
		pRS->Print(0, posY+112, 0xFFFFFFFF, "Graph read count - %d [%d], read per particle %3.2f", GraphRead, OptGraphRead, ReadsPerParticle);

		if (GraphRead != 0)
		{
			float AverageReadTime = (float)nowUpdateTime / (float)(GraphRead + OptGraphRead);
			pRS->Print(0, posY+128, 0xFFFFFFFF, "Average read time - %3.2f", AverageReadTime);
		}
	}

	if (api->DebugKeyState(VK_F3, VK_CONTROL))
	{
		ShowStat = !ShowStat;
		Sleep(100);
	}

	return bHaveDistorsion;
}


bool ParticleManager::DrawSimpleParticles (bool bSoftParticlesSupport)
{
	AssertCoreThread

	dword dwPartID = pRS->pixBeginEvent(_FL_, "ParticleManager::Draw");


#ifndef _XBOX
	RENDERVIEWPORT rvp = pRS->GetViewport();
	RENDERSCREEN scr = pRS->GetScreenInfo3D();
	float fWidth = (float)scr.dwWidth;
	float fHeight = (float)scr.dwHeight;

	Vector4 fitToViewPort;
	fitToViewPort.x = rvp.X / fWidth;
	fitToViewPort.y = rvp.Y / fHeight;
	fitToViewPort.z = rvp.Width / fWidth;
	fitToViewPort.w = rvp.Height / fHeight;

	fitToViewport->SetVector4(fitToViewPort);
#endif



#ifdef _XBOX

	if (bSoftParticlesSupport)
	{
		pRS->ResolveDepth(tempRTDForSoftParticles);
		depthMap->SetTexture(tempRTDForSoftParticles->AsTexture ());
		particleSoftness->SetFloat(fSoftness);

		Matrix mProjInv = pRS->GetProjection();
		mProjInv.InverseComplette4X4();
		mProjInverse->SetMatrix(mProjInv);

		/*
		XINPUT_STATE inputState;
		XInputGetState(0, &inputState);


		if (inputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP)
		{
		fSoftness += 0.001f;
		}

		if (inputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
		{
		fSoftness -= 0.001f;
		}


		if (inputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
		{
		fSoftness += 0.01f;
		}

		if (inputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
		{
		fSoftness -= 0.01f;
		}
		*/
	}




#endif

		pBaseTexture->SetTexture(pProjectTexture);
		pNormalMap->SetTexture(pNormalMapProjectTexture);

		Vector vDir = pRS->GetGlobalLightDirection();
		Matrix mCurView = pRS->GetView();

		Vector vProjDir = mCurView.MulNormal(vDir);
		vProjDir.Normalize();
		vProjDir.z = -vProjDir.z;

		DirLightProj->SetVector(vProjDir);
		DirLightProjColor->SetVector4(pRS->GetGlobalLightColor().v4);


		Vector ambColor = pRS->GetAmbient().v4.v;
		LightAmbient->SetVector(ambColor);

		bool bHaveDistorsion = false;
		ParticleVB * renderData = pService->GetUpdateData();
		IIBuffer * pIndexBuffer = pService->GetCurrentIndexBuffer();
		bHaveDistorsion = spriteProcessor->Draw(*renderData, pIndexBuffer, bSoftParticlesSupport);

		pRS->pixEndEvent(_FL_, dwPartID);

		return bHaveDistorsion;
}




bool ParticleManager::Update ()
{

#ifdef ENABLE_PARTICLE_THREADS
	sync_access_to_array.Enter();
#endif

	framesPassed++;
	GraphRead = 0;
	OptGraphRead = 0;
	ActiveSystems = 0;
	InactiveSystems = 0;
	ActiveEmitters = 0;


	dword dwIterCount = 0;
	m_time = m_time + ThreadParams.fDeltaTime;

	if (m_time >= m_fixedFPS)
	{
		for (;;)
		{
			ActiveSystems = 0;
			InactiveSystems = 0;

			for (DWORD n = 0; n < ThreadParams.SystemsForThread.Size(); n++)
			{
				ParticleSystem* pSystem = ThreadParams.SystemsForThread[n];

				if (pSystem->IsActive())
				{
					ActiveSystems++;
					ActiveEmitters += pSystem->Execute(m_fixedFPS);
				}
				else
				{
					InactiveSystems++;
				}
			}

			spriteProcessor->UpdateParticles(m_fixedFPS);
			spriteProcessor->DeleteDeadParticles();

			m_time = m_time - m_fixedFPS;
			dwIterCount++;
			if (m_time < m_fixedFPS) break;
		}
	}


	spriteProcessor->BuildVertexBuffers(ThreadParams);


#ifdef ENABLE_PARTICLE_THREADS
	sync_access_to_array.Leave();
#endif

	return true;
}

//Исполнить партиклы 
void ParticleManager::Execute (float DeltaTime)
{
	AssertCoreThread

	if (!bIsActiveManager)
	{
		return;
	}


	fTime += DeltaTime;

	//------------------
#ifdef ENABLE_PARTICLE_THREADS
	sync_access_to_array.Enter();
#endif

	ThreadParams.updateData = pService->GetUpdateData();
	ThreadParams.mView = pRS->GetView();
	ThreadParams.fDeltaTime = DeltaTime;
	memcpy (ThreadParams.planes, pRS->GetFrustum(), sizeof(Plane) * pRS->GetNumFrustumPlanes());

	for (dword n = 0; n < ThreadParams.SystemsProxy.Size(); n++)
	{
		ThreadParams.SystemsProxy[n]->SendReceivePackets();
	}

	//Удаляем умершие системы...
	for (dword j = 0; j < ThreadParams.SystemsDeleted.Size(); j++)
	{
		for (dword n = 0; n < ThreadParams.SystemsForThread.Size(); n++)
		{
			ParticleSystem* toDelete = ThreadParams.SystemsForThread[n];
			ParticleSystem* pSys = ThreadParams.SystemsDeleted[j];

			if (pSys == toDelete)
			{
				ThreadParams.SystemsForThread.ExtractNoShift(n);
				toDelete->Release();
				continue;
			}
		}
	}
	ThreadParams.SystemsDeleted.DelAll();

	//Добавляем новые...
	for (dword n = 0; n < ThreadParams.SystemsCreated.Size(); n++)
	{
		ThreadParams.SystemsForThread.Add(ThreadParams.SystemsCreated[n]);
	}
	ThreadParams.SystemsCreated.DelAll();


#ifdef ENABLE_PARTICLE_THREADS
		sync_access_to_array.Leave();
		pService->StartUpdateThread(this);
#else
		ThreadParams.updateData->Lock();
		Update();
		ThreadParams.updateData->Unlock();
#endif


	//---- это из основного ---------------
	for (DWORD n = 0; n < DeleteQuery.Size(); n++)
	{
		DeleteQuery[n]->Release();
	}
	DeleteQuery.DelAll();

}

//Узнать доступна система или нет 
bool ParticleManager::IsSystemAvailable (const char* FileName)
{
	AssertCoreThread

	if (pDataCache->GetParticleSystemDataSource (FileName)) return true;
	return false;
}


/*
//Получить глобальную текстуру проекта 
IBaseTexture* ParticleManager::GetProjectTexture ()
{
	return pProjectTexture;
}
 */
 
void ParticleManager::AddDeleteTime (double time)
{
	AssertCoreThread

	deleteTime += time;
	deleteCount++;

	maxDeleteTime = coremax(time, maxDeleteTime);
}

/*
ParticleSystemProxy* ParticleManager::GetFreeSystem (const char* szFileName)
{
	return NULL;

	dword dwSearchHash = string::HashNoCase(szFileName);

	for (dword n = 0; n < FreeCachedSystems.Size(); n++)
	{
		if (FreeCachedSystems[n].dwNameHash == dwSearchHash)
		{
			if (crt_stricmp(FreeCachedSystems[n].systemPtr->GetName(), szFileName) == 0)
			{
				ParticleSystemProxy* proxy = FreeCachedSystems[n].systemPtr;
				FreeCachedSystems.ExtractNoShift(n);
				return proxy;
			}
		}
	}

	return NULL;
}
*/

//Создать партикловую систему из файла (файл должен быть в проекте!!!!!)
IParticleSystem* ParticleManager::CreateParticleSystemEx (const char* FileName, const char* File, int Line)
{
	AssertCoreThread

	fCreatedSystems += 1.0f;
	bIsActiveManager = true;

	CoreCommand_GetMemStat memStatInit;
	api->ExecuteCoreCommand(memStatInit);
	ProfileTimer pTimer;
	ProfileTimer pTimer2;

	DataSource* pDataSource = pDataCache->GetParticleSystemDataSource (FileName);
	if (!pDataSource)
	{
		api->Error("Particle system '%s' can't loading. Reason: Not found in cache", FileName);
		return NULL;
	}


	pTimer2.Stop();
	ProfileTimer pTimer3;
	ProfileTimer pTimer4;

	ParticleSystemProxy* pSys = CreateParticleSystemFromDataSource (pDataSource);

	pTimer4.Stop();

	pSys->SetName(FileName);

	pTimer3.Stop();
	pTimer.Stop();
	createTime += pTimer.GetTime();
	createCount++;
	
	if (framesPassed > DONT_COLLECT_STATS_FRAMES)
	{
		maxCreateTime = coremax(pTimer.GetTime(), maxCreateTime);
	}

	return pSys;
}

void ParticleManager::DeleteAllSystems ()
{
	AssertCoreThread

#ifdef ENABLE_PARTICLE_THREADS
		sync_access_to_array.Enter();
#endif

	GlobalDelete = true;
	for (DWORD n = 0; n < ThreadParams.SystemsProxy.Size(); n++)
	{
		ParticleSystemProxy* proxyToDelete = ThreadParams.SystemsProxy[n];
		delete proxyToDelete;
	}

#ifdef ENABLE_PARTICLE_THREADS

	for (DWORD n = 0; n < ThreadParams.SystemsForThread.Size(); n++)
	{
		ThreadParams.SystemsForThread[n]->Release();
	}
#endif

	for (DWORD n = 0; n < ThreadParams.SystemsCreated.Size(); n++)
	{
		ThreadParams.SystemsCreated[n]->Release();
	}

	ThreadParams.SystemsProxy.DelAll();
	ThreadParams.SystemsForThread.DelAll();
	ThreadParams.SystemsCreated.DelAll();
	ThreadParams.SystemsDeleted.DelAll();

#ifdef ENABLE_PARTICLE_THREADS
	sync_access_to_array.Leave();
#endif



	GlobalDelete = false;
}


BillBoardProcessor* ParticleManager::getBBProcessor ()
{
	return spriteProcessor;
}



//Создать пустую партикловую систему, для редактора...
IParticleSystem* ParticleManager::CreateEmptyParticleSystemEx  (const char* FileName, int Line)
{
	AssertCoreThread

	return NULL;
}

bool ParticleManager::ValidateSystem (IParticleSystem* pSystem)
{
	AssertCoreThread

	for (DWORD n = 0; n < ThreadParams.SystemsProxy.Size(); n++)
	{
		if (ThreadParams.SystemsProxy[n] == pSystem) return true;
	}
	return false;
}


DWORD ParticleManager::GetCreatedSystemCount ()
{
	AssertCoreThread

	return ThreadParams.SystemsProxy.Size();
}

ParticleSystemProxy* ParticleManager::GetCreatedSystemByIndex (DWORD Index)
{
	AssertCoreThread

	return ThreadParams.SystemsProxy[Index];
}


void ParticleManager::DefferedDelete (IParticleSystem* pSys)
{
	AssertCoreThread

	for (dword n = 0; n < DeleteQuery.Size(); n++)
	{
		if (DeleteQuery[n] == pSys)
		{
			api->Trace("[Need to fix this] Try to deffered delete, system that already defered deleted !!!");
			return;
		}
	}

	DeleteQuery.Add(pSys);
}

void ParticleManager::Editor_UpdateCachedData ()
{
	AssertCoreThread

	for (DWORD n = 0; n < ThreadParams.SystemsProxy.Size(); n++)
	{
		ParticleSystem* sys = ThreadParams.SystemsProxy[n]->Lock();

		sys->Editor_UpdateCachedData ();

		ThreadParams.SystemsProxy[n]->Unlock();
	}

}

DWORD ParticleManager::GetProjectSystemCount ()
{
	AssertCoreThread

	return pDataCache->GetCachedCount();
}

const char* ParticleManager::GetProjectSystemName (DWORD Index)
{
	AssertCoreThread

	return pDataCache->GetCachedNameByIndex(Index);
}

const char* ParticleManager::GetFirstGeomName (const char* FileName)
{
	AssertCoreThread

	IteratorIndex = 0;
	GeomNameParser Parser;
	EnumUsedGeom.DelAll ();
	DataSource* pDataSource = pDataCache->GetParticleSystemDataSource (FileName);
	if (!pDataSource) return NULL;

	DWORD count = pDataSource->GetEmitterCount();

	for (DWORD n = 0; n < count; n++)
	{
		DataSource::EmitterDesc* pDesc = pDataSource->GetEmitterDesc(n);
		for (DWORD i = 0; i < pDesc->Particles.Size(); i++)
		{
			DataString* pModelNames = pDesc->Particles[i].Fields.FindStringByGUID(GUID_PARTICLE_GEOM_NAMES);
			if (!pModelNames) continue;
			
			Parser.Tokenize(pModelNames->GetValue());

      DWORD TCount = Parser.GetTokensCount();
			if (TCount > 0)
			{
				for (DWORD j = 0; j < TCount; j++)
				{
					const char* GeomName = Parser.GetTokenByIndex(j);
					if (!FindInEnumUsedGeom(GeomName)) EnumUsedGeom.Add(string(GeomName));
				}
			}
		}
		
	}

	if (EnumUsedGeom.Size() == 0) return NULL;
	const char* FirstName = EnumUsedGeom[IteratorIndex].GetBuffer();
	IteratorIndex++;
	return FirstName;
	
}

const char* ParticleManager::GetNextGeomName ()
{
	AssertCoreThread

	if (IteratorIndex >= EnumUsedGeom.Size()) return NULL;
	const char* FirstName = EnumUsedGeom[IteratorIndex].GetBuffer();
	IteratorIndex++;
	return FirstName;
}

bool ParticleManager::FindInEnumUsedGeom (const char* GeomName)
{
	AssertCoreThread

	for (DWORD n = 0; n < EnumUsedGeom.Size(); n++)
	{
		const char* StoredGeomName = EnumUsedGeom[n].GetBuffer();
		if (crt_stricmp (StoredGeomName, GeomName) == 0) return true;
	}
	return false;
}


void ParticleManager::WriteSystemCache (const char* FileName)
{
#ifndef _XBOX
	AssertCoreThread

	DataSource* pDataSource = pDataCache->GetParticleSystemDataSource (FileName);
	if (!pDataSource)
	{
		api->Error("Particle system '%s' can't save. Reason: Not found in cache", FileName);
		return;
	}


	MemFile pMemSave;
	pMemSave.OpenWrite(1048576);
	pDataSource->Write(&pMemSave);
	
	string LongFileName = "resource\\particles\\";
	LongFileName+=FileName;
	LongFileName.AddExtention(".xps");
	
	
	bool res = pFS->SaveData(LongFileName.GetBuffer(), pMemSave.GetBuffer(), pMemSave.GetLength());

	if (!res)
	{

		::MessageBox(NULL, "Can't save file !!!!\nWriteprotection ?", "Save error", MB_OK);

	}

	pMemSave.Close();

	LongFileName = "resource\\particles\\";
	LongFileName+=FileName;

	LongFileName.DeleteLastSymbol('s');
	LongFileName.DeleteLastSymbol('p');
	LongFileName.DeleteLastSymbol('x');
	LongFileName.DeleteLastSymbol('.');


	LongFileName.AddExtention(".xml");
	
	TextFile xmlFile(LongFileName.c_str());

	xmlFile.Write(0, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");

	pDataSource->WriteXML(&xmlFile, 0);



	api->Error("Particle system '%s' saved.", FileName);

#endif
}

void ParticleManager::WriteSystemCacheAs (const char* FileName, const char* NewName)
{
#ifndef _XBOX
	AssertCoreThread

	DataSource* pDataSource = pDataCache->GetParticleSystemDataSource (FileName);
	if (!pDataSource)
	{
		api->Error("Particle system '%s' can't save. Reason: Not found in cache", FileName);
		return;
	}

	MemFile pMemSave;
	pMemSave.OpenWrite(1048576);
	pDataSource->Write(&pMemSave);

	pFS->SaveData(NewName, pMemSave.GetBuffer(), pMemSave.GetLength());

	pMemSave.Close();


	string LongFileName = NewName;

	LongFileName.DeleteLastSymbol('s');
	LongFileName.DeleteLastSymbol('p');
	LongFileName.DeleteLastSymbol('x');
	LongFileName.DeleteLastSymbol('.');


	LongFileName.AddExtention(".xml");

	TextFile xmlFile(LongFileName.c_str());

	xmlFile.Write(0, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");

	pDataSource->WriteXML(&xmlFile, 0);

	api->Trace("Particle system '%s' saved.", NewName);
#endif
}

void ParticleManager::WriteSystemCache (const char* FileName, MemFile* pMemFile)
{
	AssertCoreThread

	DataSource* pDataSource = pDataCache->GetParticleSystemDataSource (FileName);
	if (!pDataSource)
	{
		api->Error("Particle system '%s' can't save. Reason: Not found in cache", FileName);
		return;
	}

	pDataSource->Write(pMemFile);
}

void ParticleManager::LoadSystemCache (const char* FileName, MemFile* pMemFile)
{
	AssertCoreThread

	DataSource* pDataSource = pDataCache->GetParticleSystemDataSource (FileName);
	if (!pDataSource)
	{
		api->Error("Particle system '%s' can't load. Reason: Not found in cache", FileName);
		return;
	}

	pDataSource->Destroy();
	pDataSource->Load(pMemFile);

	CacheReloaded ();
}

void ParticleManager::CacheReloaded ()
{
	AssertCoreThread


	DWORD n = 0;
	array<CacheReloadedInfo> UsedSystems(_FL_);
	for (n = 0; n < ThreadParams.SystemsProxy.Size(); n++)
	{
		ParticleSystemProxy* pSystem = ThreadParams.SystemsProxy[n];

		CacheReloadedInfo Info;
		Info.Name = pSystem->GetName();
		Info.AutoDeleted = pSystem->IsAutoDeleted();

		ParticleSystem* sys = pSystem->Lock();

		Info.AttachedScene = pSystem->GetAttachedSceneToGeom();
		Info.AttachedToGeom = pSystem->GetAttachedToGeom();
		sys->GetTransform(Info.matWorld);

		pSystem->Unlock();

		UsedSystems.Add(Info);
	}


	DeleteAllSystems ();

	spriteProcessor->Clear();


	for (n = 0; n < UsedSystems.Size(); n++)
	{
		IParticleSystem* pSystem = CreateParticleSystemEx (UsedSystems[n].Name.GetBuffer(), _FL_);
		pSystem->AutoDelete(UsedSystems[n].AutoDeleted);
		pSystem->SetTransform(UsedSystems[n].matWorld);

		if (UsedSystems[n].AttachedToGeom.isValid())
		{
			pSystem->AttachTo(UsedSystems[n].AttachedScene, UsedSystems[n].AttachedToGeom, true);
		}
		
	}
}


FieldList* ParticleManager::Editor_CreatePointEmitter (const char* SystemName, const char* EmitterName)
{
	AssertCoreThread

	DataSource* pDataSource = pDataCache->GetParticleSystemDataSource (SystemName);
	if (!pDataSource) return NULL;

	FieldList* pData = pDataSource->CreateEmptyPointEmitter(EmitterName);

	CacheReloaded ();

	return pData;
}

FieldList* ParticleManager::Editor_CreateBillBoardParticle (const char* SystemName, const char*EmitterName, const char* ParticleName)
{
	AssertCoreThread

	DataSource* pDataSource = pDataCache->GetParticleSystemDataSource (SystemName);
	if (!pDataSource) return NULL;

	FieldList* pData = pDataSource->CreateBillBoardParticle (ParticleName, EmitterName);

	CacheReloaded ();

	return pData;
}

FieldList* ParticleManager::Editor_CreateModelParticle (const char* SystemName, const char*EmitterName, const char* ParticleName)
{
	AssertCoreThread

	DataSource* pDataSource = pDataCache->GetParticleSystemDataSource (SystemName);
	if (!pDataSource) return NULL;

	FieldList* pData = pDataSource->CreateModelParticle (ParticleName, EmitterName);

	CacheReloaded ();

	return pData;
}

void ParticleManager::DeletePointEmitter (const char* SystemName, IEmitter* pEmitter)
{
	AssertCoreThread

	DataSource* pDataSource = pDataCache->GetParticleSystemDataSource (SystemName);
	if (!pDataSource) return;

	FieldList* pEmitterData = pEmitter->GetData();

	pDataSource->DeletePointEmitter (pEmitterData);

	CacheReloaded ();
}

void ParticleManager::DeleteBillboard (const char* SystemName, IEmitter* pEmitter, FieldList* pParticles)
{
	AssertCoreThread

	DataSource* pDataSource = pDataCache->GetParticleSystemDataSource (SystemName);
	if (!pDataSource) return;

	FieldList* pEmitterData = pEmitter->GetData();

	pDataSource->DeleteBillboard (pEmitterData, pParticles);

	CacheReloaded ();
}

void ParticleManager::DeleteModel (const char* SystemName, IEmitter* pEmitter, FieldList* pParticles)
{
	AssertCoreThread

	DataSource* pDataSource = pDataCache->GetParticleSystemDataSource (SystemName);
	if (!pDataSource) return;

	FieldList* pEmitterData = pEmitter->GetData();

	pDataSource->DeleteModel (pEmitterData, pParticles);

	CacheReloaded ();
}

IParticleSystem* ParticleManager::CreateParticleSystemEx2 (const char* FileName, const Matrix& mWorld, bool bAutoDelete, const char* File, int Line)
{
	AssertCoreThread

	bIsActiveManager = true;

	fCreatedSystems += 1.0f;

	ProfileTimer pTimer;

	IParticleSystem* pSys = CreateParticleSystemEx(FileName, File, Line);
	if (pSys)
	{
		pSys->SetTransform(mWorld);
		pSys->AutoDelete(bAutoDelete);
	}

	pTimer.Stop();
	createTime += pTimer.GetTime();
	createCount++;

	if (framesPassed > DONT_COLLECT_STATS_FRAMES)
	{
		maxCreateTime = coremax(pTimer.GetTime(), maxCreateTime);
	}

	if (pTimer.GetTime() > 100000.0f)
	{
		//api->Trace("CreateParticleSystemEx2::Slow create '%s', %f", FileName, (double)pTimer.time64);
	}

	if (bAutoDelete)
	{
		return NULL;
	}

	return pSys;
}

dword ParticleManager::RegisterForceField (const char* szName, Object* _class, FORCEFIELD_PROCESS _method)
{
	AssertCoreThread

	//Такой force field уже есть...
	int idx = GetForceFieldIndex(szName);
	if (idx >= 0) return ForceFields[idx].GUID;
	RegistredForceFields* pField = &ForceFields[ForceFields.Add()];

	pField->dwNameHash = string::HashNoCase(szName);
	pField->Name = szName;
	pField->pClass = _class;
	pField->pMethod = _method;
	pField->GUID = GlobalUniqueForceFieldIndex;
	GlobalUniqueForceFieldIndex++;

	pDataCache->NewForceFieldRegistred();

	return pField->GUID;
}

void ParticleManager::UnregisterAllForceFields(Object* _class)
{
	AssertCoreThread

	for (dword i = 0; i < ForceFields.Size(); i++)
	{
		if (ForceFields[i].pClass == _class)
		{
			ForceFields.ExtractNoShift (i);
			i--;
		}
	}
}


void ParticleManager::UnregisterForceField (Object* _class, FORCEFIELD_PROCESS _method)
{
	AssertCoreThread

	for (dword i = 0; i < ForceFields.Size(); i++)
	{
		if (ForceFields[i].pClass == _class && ForceFields[i].pMethod == _method)
		{
			ForceFields.ExtractNoShift (i);
			return;
		}
	}

}

int ParticleManager::GetForceFieldIndex (const char* szName)
{
	AssertCoreThread

	dword dwSearchedHash = string::HashNoCase(szName);
	for (dword i = 0; i < ForceFields.Size(); i++)
	{
		if (ForceFields[i].dwNameHash == dwSearchedHash)
		{
			if (ForceFields[i].Name == szName) return i;
		}
	}
	return -1;
}


dword ParticleManager::GetForceFieldUniqueID (const char* szName)
{
	AssertCoreThread

	int idx = GetForceFieldIndex(szName);
	if (idx >= 0) return ForceFields[idx].GUID;
	return 0xFFFFFFFF;
}

bool ParticleManager::ExecuteForceFields (dword dwGUID1, dword dwGUID2, dword dwGUID3, dword dwGUID4, Vector& old_pos, Vector& pos, Vector& externalforce)
{
	AssertCoreThread

	dword dwForceFieldsCount = 0;
	if (dwGUID1 != 0xFFFFFFFF) dwForceFieldsCount++;
	if (dwGUID2 != 0xFFFFFFFF) dwForceFieldsCount++;
	if (dwGUID3 != 0xFFFFFFFF) dwForceFieldsCount++;
	if (dwGUID4 != 0xFFFFFFFF) dwForceFieldsCount++;

	dword dwProcessedForceFields = 0;

	for (dword i = 0; i < ForceFields.Size(); i++)
	{
		if (dwGUID1 != 0xFFFFFFFF)
		{
			if (ForceFields[i].GUID == dwGUID1)
			{
				bool bRes = (ForceFields[i].pClass->*ForceFields[i].pMethod) (ForceFields[i].GUID, old_pos, pos, externalforce);
				if (bRes == false) return true;
				dwProcessedForceFields++;
				if (dwProcessedForceFields >= dwForceFieldsCount) return false;
			}
		}

		if (dwGUID2 != 0xFFFFFFFF)
		{
			if (ForceFields[i].GUID == dwGUID2)
			{
				bool bRes = (ForceFields[i].pClass->*ForceFields[i].pMethod) (ForceFields[i].GUID, old_pos, pos, externalforce);
				if (bRes == false) return true;
				dwProcessedForceFields++;
				if (dwProcessedForceFields >= dwForceFieldsCount) return false;
			}
		}
		
		if (dwGUID3 != 0xFFFFFFFF)
		{
			if (ForceFields[i].GUID == dwGUID3)
			{
				bool bRes = (ForceFields[i].pClass->*ForceFields[i].pMethod) (ForceFields[i].GUID, old_pos, pos, externalforce);
				if (bRes == false) return true;
				dwProcessedForceFields++;
				if (dwProcessedForceFields >= dwForceFieldsCount) return false;
			}
		}
		
		if (dwGUID4 != 0xFFFFFFFF)
		{
			if (ForceFields[i].GUID == dwGUID4)
			{
				bool bRes = (ForceFields[i].pClass->*ForceFields[i].pMethod) (ForceFields[i].GUID, old_pos, pos, externalforce);
				if (bRes == false) return true;
				dwProcessedForceFields++;
				if (dwProcessedForceFields >= dwForceFieldsCount) return false;
			}
		}

		
	}


	return false;
}





void ParticleManager::LoadAllSystems ()
{
	AssertCoreThread

	spriteProcessor->Clear ();
	DeleteAllSystems ();
	pDataCache->ResetCache();


	pService->LoadAllfromDisk();
}


void ParticleManager::ReloadSystems ()
{
	AssertCoreThread


	spriteProcessor->Clear ();


	pDataCache->ResetCache();

	pService->LoadAllfromDisk();


	string DataSourceName;
	string ShortDataSourceName;


	for (DWORD n = 0; n < ThreadParams.SystemsProxy.Size(); n++)
	{
		DataSourceName = ThreadParams.SystemsProxy[n]->GetName();
		ShortDataSourceName.GetFileName(DataSourceName);

		

		DataSource* pDataSource = pDataCache->GetParticleSystemDataSource (ShortDataSourceName.c_str());

		ParticleSystem* sys = ThreadParams.SystemsProxy[n]->Lock();
		sys->ReatachToDataSource(pDataSource);
		ThreadParams.SystemsProxy[n]->Unlock();
	}
}



