#include "gmxService.h"
#include "meshContainer.h"
#include "meshInstance.h"

#include "..\..\common_h\corecmds.h"


//INTERFACE_FUNCTION
CREATE_SERVICE(GMXService, 60)

#define GEOM_FOLDER "Resource\\Models\\"
#define DEFAULT_ALPHA_REF 0.5f

GMXService * GMXService::pGS = NULL;
MeshInstance * GMXService::pHead = NULL;
IPhysics * GMXService::pPhysic = NULL;


#ifndef STOP_DEBUG
IConsole* GMXService::pConsole = NULL;
dword GMXService::bBlendShapeDebug = FALSE;
dword GMXService::bGeometryDebug = FALSE;
const char* hm_desc[] = { "None", "Show batches", "Show overdraw", "Show Diffuse", "Show Normal Map", "Show Parralax", "Show Shinines", "Show Gloss", "Show Occlusion", "Show normals", "Show binormal", "Show tangent", "Light Complex"};
#endif



GMXService::GMXService() : meshesFiles(_FL_)
{
	mode = HM_OFF;

#ifndef STOP_DEBUG
	pEmptyNormalMapTexture = NULL;
	show_mode = GMXSM_ALL;
	debugShaders = (IGMXService::RenderModeShaderId*)malloc(GMXService::HM_MAX * sizeof(IGMXService::RenderModeShaderId));
#endif

	texturesPath[0] = 0;

	rmode_enabled = false;


	gmxAlphaRef = NULL;

	UserColorFromSceneDenied = false;

	pShadowService = NULL;

	pNoShadowTexture = NULL;
	bGlobalFrustumCull = true;

#ifndef _XBOX
	skinBones = NULL;
#else
	dwCurrentPoolIndex = 0;
	for (dword q = 0; q < VERTEXSTREAMS_WITH_CONSTANTS_POOL; q++)
	{
		bonesArrayPool[q] = NULL;
	}
#endif

	GMX_ShadowColor = NULL;
	GMX_ShadowQuad = NULL;

	pUserColor = NULL;
	dwCacheFrameNum = 1;
	SetRenderMode(NULL);
}

bool GMXService::Init()
{
	GMXService::pGS = this;

	pFS = (IFileService*)api->GetService("FileService");
	Assert(pFS);

	pRS = (IRender*)api->GetService("DX9Render");
	Assert(pRS);



#ifndef STOP_DEBUG
	GetRenderModeShaderId("gmx_ShowDiffuse", debugShaders[GMXService::HM_SHOW_DIFFUSE]);
	GetRenderModeShaderId("gmx_ShowBatches", debugShaders[GMXService::HM_SHOW_BATCHES]);
	GetRenderModeShaderId("gmx_ShowOverDraw", debugShaders[GMXService::HM_SHOW_OVERDRAW]);
	GetRenderModeShaderId("gmx_ShowNormalMap", debugShaders[GMXService::HM_SHOW_NORMAL_MAP]);
	GetRenderModeShaderId("gmx_ShowParralax", debugShaders[GMXService::HM_SHOW_PARALLAX]);
	GetRenderModeShaderId("gmx_ShowShinines", debugShaders[GMXService::HM_SHOW_SHININES]);
	GetRenderModeShaderId("gmx_ShowGloss", debugShaders[GMXService::HM_SHOW_GLOSS]);
	GetRenderModeShaderId("gmx_ShowOcclusion", debugShaders[GMXService::HM_SHOW_OCCLUSION]);
	GetRenderModeShaderId("gmx_ShowNormals", debugShaders[GMXService::HM_SHOW_NORMALS]);
	GetRenderModeShaderId("gmx_ShowBiNormals", debugShaders[GMXService::HM_SHOW_BINORMALS]);
	GetRenderModeShaderId("gmx_ShowTangents", debugShaders[GMXService::HM_SHOW_TANGENTS]);
	GetRenderModeShaderId("gmx_ShowLightComplex", debugShaders[GMXService::HM_SHOWLIGHTCOMPLEX]);
#endif



	pUserColor = NULL;

	pUserColor = pRS->GetTechniqueGlobalVariable("GMX_ObjectsUserColor", _FL_);
	if (pUserColor)
	{
		pUserColor->SetVector4(Color(0.0f, 0.0f, 0.0f).v4);
	}

#ifndef _XBOX
	skinBones = pRS->GetTechniqueGlobalVariable("GMX_skinBones", _FL_);
#else
	const dword maxBonesPerPass = 75;
	//const dword maxBonesPerPass = 255;
	dwCurrentPoolIndex = 0;
	for (dword q = 0; q < VERTEXSTREAMS_WITH_CONSTANTS_POOL; q++)
	{
		bonesArrayPool[q] = pRS->CreateVertexBuffer(sizeof(Vector4) * 3 * maxBonesPerPass, sizeof(Vector4) * 3, _FL_);

		byte* memToClear = (byte*)bonesArrayPool[q]->Lock();
		memset (memToClear, 0, sizeof(Vector4) * 3 * maxBonesPerPass);
		bonesArrayPool[q]->Unlock();

	}
#endif

	GMX_ShadowQuad = pRS->GetTechniqueGlobalVariable("GMX_ShadowQuad", _FL_);
	GMX_ShadowColor = pRS->GetTechniqueGlobalVariable("GMX_ShadowColor", _FL_);
	gmxAlphaRef = pRS->GetTechniqueGlobalVariable("gmxAlphaRef", _FL_);


	if (gmxAlphaRef)
	{
		gmxAlphaRef->SetFloat(DEFAULT_ALPHA_REF);
	}


	api->SetEndFrameLevel(this, Core_DefaultExecuteLevel);

	RENDERLOCKED_RECT lr;
	pNoShadowTexture = pRS->CreateTexture(4, 4, 1, 0, FMT_DXT1, _FL_);
	Assert(pNoShadowTexture != NULL);
	if (pNoShadowTexture->LockRect(0, &lr, null, 0))
	{
		((dword*)lr.pBits)[0] = 0xFFFFFFFF;
		((dword*)lr.pBits)[1] = 0x00;
		pNoShadowTexture->UnlockRect(0);
	}

#ifndef STOP_DEBUG
	pEmptyNormalMapTexture = pRS->CreateTexture(4, 4, 1, 0, FMT_DXT1, _FL_);
	Assert(pEmptyNormalMapTexture != NULL);
	if (pEmptyNormalMapTexture->LockRect(0, &lr, null, 0))
	{
		((dword*)lr.pBits)[0] = 0x841F841F;
		((dword*)lr.pBits)[1] = 0x00;
		pEmptyNormalMapTexture->UnlockRect(0);
	}
#endif

	return true;
};


GMXService::~GMXService()
{
	dumpMeshLeaksAndCleanup();
#ifndef STOP_DEBUG
	free(debugShaders);
#endif
}

void GMXService::SetHackMode (IGMXService::HackMode _mode)
{
	mode = _mode;
}

IGMXService::HackMode GMXService::GetHackMode ()
{
	return mode;
}

void GMXService::SetTexturePath(const char* szPath)
{
	crt_strcpy(texturesPath, (MAX_PATH-1), szPath);
}

const char* GMXService::GetTexturePath()
{
	return texturesPath;
}


IRender* GMXService::Render()
{
	return pRS;
}

void GMXService::SetRenderMode (IGMXService::RenderModeShaderId * new_shaders, IGMXService::RenderModeShaderId * new_shaders_without_alpha)
{
	if (new_shaders == NULL)
	{
		rmode_enabled = false;
		return;

	}

	rmode_enabled = true;
	rmode = *new_shaders;

	if (new_shaders_without_alpha != NULL)
	{
		rmode_noalpha = *new_shaders_without_alpha;
	} else
	{
		rmode_noalpha = *new_shaders;
	}
}


IGMXService::RenderModeShaderId * GMXService::GetRenderMode ()
{
	return GetRenderMode_Fast();
}

IGMXService::RenderModeShaderId * GMXService::GetRenderModeNoAlpha ()
{
	return GetRenderModeNoAlpha_Fast();
}


const Color& GMXService::GetUserColor ()
{
	return serviceUserColor;
}

void GMXService::SetUserColor (const Color& clr)
{
	serviceUserColor = clr;

	if (!pUserColor) return;

	pUserColor->SetVector4(clr.v4);

	float alphaRef = DEFAULT_ALPHA_REF * clr.v4.w;
	Clampf(alphaRef);
	gmxAlphaRef->SetFloat(alphaRef);
}

void GMXService::SetCustomAlphaRef(float alphaRef)
{
	Clampf(alphaRef);
	gmxAlphaRef->SetFloat(alphaRef);
}

void GMXService::SuppressSceneSetUserColor (bool bDenied)
{
	UserColorFromSceneDenied = bDenied;
}

bool GMXService::IsSuppressSceneSetUserColor ()
{
	return UserColorFromSceneDenied;
}

bool GMXService::GetGlobalFrustumCullState ()
{
	return GetGlobalFrustumCullStateFast();
}

void GMXService::SetGlobalFrustumCullState (bool bEnable)
{
	bGlobalFrustumCull = bEnable;
}


void GMXService::GetRenderModeShaderId (const char * szShaderName, IGMXService::RenderModeShaderId & shaders_id)
{
	static char temp[8192];

	pRS->GetShaderLightingId(szShaderName, shaders_id.bones[0]);

	crt_snprintf(temp, 8000, "%s_anim_1", szShaderName);
	pRS->GetShaderLightingId(temp, shaders_id.bones[1]);

	crt_snprintf(temp, 8000, "%s_anim_2", szShaderName);
	pRS->GetShaderLightingId(temp, shaders_id.bones[2]);

	crt_snprintf(temp, 8000, "%s_anim_3", szShaderName);
	pRS->GetShaderLightingId(temp, shaders_id.bones[3]);

}

#ifdef _XBOX
IVBuffer* GMXService::GetSkinBones_Stream()
{
	IVBuffer* ptr = bonesArrayPool[dwCurrentPoolIndex];

	dwCurrentPoolIndex++;
	if (dwCurrentPoolIndex >= VERTEXSTREAMS_WITH_CONSTANTS_POOL)
	{
		dwCurrentPoolIndex = 0;
	}

	return ptr;
}
#else
IVariable* GMXService::GetSkinBones_Variable()
{
	return skinBones;
}
#endif


void GMXService::reportMeshContainerRelease(MeshContainer * meshFile)
{
	htable<MeshContainer *>::iterator iter(meshesFiles);
	for (iter.Begin() ; iter.IsDone() == false ; iter.Next())
	{
		MeshContainer * & item = iter.Get();
		if (item == meshFile)
		{
			iter.DeleteCurrentItem();
			return;
		}
	}
}

IGMXScene* GMXService::CreateScene (const char* szFileName, IAnimationScene * animationScene, IParticleManager* pManager, ISoundScene * pSound, const char* pSourceFile, int Line)
{
	if (szFileName == NULL || szFileName[0] == 0)
	{
		return NULL;
	}

	//из за quick модулей можем загрузиться раньше этих служб....
	//--------------------------------------------
	if (pPhysic == NULL)
	{
		pPhysic = (IPhysics*)api->GetService("PhysicsService");
	}

	if (pShadowService == NULL)
	{
		pShadowService = (IShadowsService*)api->GetService("ShadowsService");
	}

	//физика нужна по любому для MeshContainer::CreateCollision()
	Assert(pPhysic);

	//Без теней проживем
	//Assert(pShadowService);
	//--------------------------------------------


	strBuffer1 = szFileName;
	strBuffer2.GetFileTitle(strBuffer1);

	MeshContainer * const  *meshContainer = meshesFiles.FindElement(strBuffer2.c_str());
	if (meshContainer)
	{
		(*meshContainer)->AddRef();
		MeshInstance * mesh = allocMeshInstance(*meshContainer, strBuffer2.c_str(), pSourceFile, Line);
		mesh->SetManagers(animationScene, pManager, pSound);
		return mesh;
	}

	//надо загрузить...
	string & longFileName = strBuffer1;
	longFileName = GEOM_FOLDER;
	longFileName += szFileName;
	longFileName.AddExtention(".gmx");

	IDataFile * pFile = pFS->OpenDataFile(longFileName.c_str(), file_open_default, _FL_);
	if (!pFile)
	{
		api->Error("GMX: '%s' File not found !!! (%s, %d)", longFileName.GetBuffer(), pSourceFile, Line);
		return NULL;
	}

	//---- создаем из файла ----

	MeshContainer * meshFile = NEW MeshContainer(pRS);
	bool bLoadResult = meshFile->Load(pFile, pFS, longFileName.c_str());

	

	RELEASE(pFile);

	if (bLoadResult == false)
	{
		return NULL;
	}

	MeshInstance * mesh = allocMeshInstance(meshFile, strBuffer2.c_str(), pSourceFile, Line);
	mesh->SetManagers(animationScene, pManager, pSound);
	meshesFiles.Add(strBuffer2, meshFile);

	return mesh;
}


void GMXService::dumpMeshLeaksAndCleanup()
{
	api->Trace("GMXService - dtor");

	if (pNoShadowTexture)
	{
		pNoShadowTexture->Release();
		pNoShadowTexture = NULL;
	}

#ifndef STOP_DEBUG
	if (pEmptyNormalMapTexture)
	{
		pEmptyNormalMapTexture->Release();
		pEmptyNormalMapTexture = NULL;
	}
#endif

	MeshInstance * m = GMXService::pHead;
	if (m)
	{
		api->Trace("[mesh leaks]\n=========================================================================\n");
	}

	while(m)
	{
		api->Trace("Mesh leaked, created from '%s', %d", m->getSourceFile(), m->getSourceLine());
		MeshInstance * meshToDelete = m;
		m = m->next();
		meshToDelete->ForceRelease();
	}


}

void GMXService::EndFrame(float dltTime)
{
	dwCacheFrameNum++;

#ifndef STOP_DEBUG
	if (pConsole == NULL)
	{
		pConsole = (IConsole*)api->GetService("Console");
		if (pConsole != NULL)
		{
			pConsole->RegisterCommand("bs", "Переключает режим отладки блендшейпов геометрии", this, (CONSOLE_COMMAND)&GMXService::BlendShapesDebugSwitch);
			pConsole->RegisterCommand("gd", "Переключает режим отладки геометрии", this, (CONSOLE_COMMAND)&GMXService::GeometryDebugModeSwitch);
		}
	}


	if (api->DebugKeyState(VK_CONTROL, '2') || api->DebugKeyState(ICore::xb_right_thumb))
	{
		dword dMode = (dword)show_mode;
		dMode++;
		if (dMode == GMXSM_MAX)
		{
			dMode = GMXSM_ALL;
		}

		show_mode = (GMX_ShowMode)dMode;

		Sleep(200);
	}

	if (api->DebugKeyState(VK_SHIFT, '2'))
	{
		dword dMode = (dword)show_mode;
		if (dMode <= GMXSM_ALL)
		{
			dMode = GMXSM_MAX-1;
		} else
		{
			dMode--;
		}
		show_mode = (GMX_ShowMode)dMode;

		Sleep(200);
	}



	if (api->DebugKeyState(VK_CONTROL, '3') || api->DebugKeyState(ICore::xb_dpad_up))
	{
		dword dMode = (dword)mode;
		dMode++;
		if (dMode >= HM_MAX)
		{
			dMode = HM_OFF;
		}

		mode = (HackMode)dMode;

		Sleep(200);
	}

	if (api->DebugKeyState(VK_SHIFT, '3'))
	{
		dword dMode = (dword)mode;
		if (dMode <= HM_OFF)
		{
			dMode = HM_MAX-1;
		} else
		{
			dMode--;
		}



		mode = (HackMode)dMode;

		Sleep(200);
	}



	if (show_mode != GMXSM_ALL || mode != HM_OFF)
	{
		float fHeight = (float)pRS->GetScreenInfo2D().dwHeight - 64.0f;

		const char * szMode = "Normal";

		if (show_mode == GMXSM_NODIFFUSE)
		{
			szMode = "Diffuse : disabled, Normals : enabled";
		}

		if (show_mode == GMXSM_NONORMAL)
		{
			szMode = "Diffuse : enabled, Normals : disabled";
		}

		if (show_mode == GMXSM_NODIFFUSE_NONORMAL)
		{
			szMode = "Diffuse : disabled, Normals : disabled";
		}

		if (show_mode == GMXSM_MAX)
		{
			szMode = "WTF ???!!";
		}


		RS_SPRITE spr[4];
		spr[0].vPos = Vector(-1.0f, -0.8f, 0.2f);       spr[1].vPos = Vector(1.0f, -0.8f, 0.2f);
		spr[2].vPos = Vector(1.0f, -1.0f, 0.2f);        spr[3].vPos = Vector(-1.0f, -1.0f, 0.2f);
		pRS->DrawSprites(NULL, spr, 1, "dbgInfoSprite");


		pRS->Print(0, fHeight, 0xFFFFFFFF, "Texture mode: '%s'", szMode);
		pRS->Print(0, fHeight + pRS->GetSystemFont()->GetHeight() + 2.0f, 0xFFFFFFFF, "Shader mode: '%s'", hm_desc[mode]);
	}

#endif


}

MeshInstance * GMXService::allocMeshInstance(MeshContainer * _container, const char * debugMeshName, const char* pSourceFile, int Line)
{
	dword dwBonesCount = _container->getBonesCount();
	dword dwDynamicTransformsCount = _container->getDynamicTransformsCount();
	dword dwLocatorsWithParticlesCount = _container->getLocatorsWithParticlesCount();
	dword dwBlendShapeBonesCount = _container->getBlendShapesBonesCount();
	dword dwBlendShapeTargetsCount = _container->getBlendShapesMorphTargetsCount();

	//-------------------

	dword _dwBonesGPUDataSize = sizeof(boneGpuData) * dwBonesCount;
	dword dwBonesGPUDataSize_Aligned = ((_dwBonesGPUDataSize + 15) & ~15);

	dword _dwGeomToAnim_TranslateTableSize = (dwBonesCount * sizeof(long));
	dword dwGeomToAnim_TranslateTableSize_Aligned = ((_dwGeomToAnim_TranslateTableSize + 15) & ~15);
	

	dword _dwAnimToGeom_TranslateTableSize = (MAX_ANIMATION_BONES * sizeof(long));
	dword dwAnimToGeom_TranslateTableSize_Aligned = ((_dwAnimToGeom_TranslateTableSize + 15) & ~15);

	//-------------------



	dword _dwParticlesPtrCacheSize = (dwLocatorsWithParticlesCount * sizeof(MeshParticleOnLocator));
	dword dwParticlesPtrCacheSize_Aligned = ((_dwParticlesPtrCacheSize + 15) & ~15);



	//-------------------

	dword _dwMorphTargetsWeightTable = (dwBlendShapeTargetsCount * sizeof(float));
	dword dwMorphTargetsWeightTable_Aligned = ((_dwMorphTargetsWeightTable + 15) & ~15);


	dword _dwAnimToGeomBlendShapes_TranslateTableSize = (MAX_ANIMATION_BONES * sizeof(long));
	dword dwAnimToGeomBlendShapes_TranslateTableSize_Aligned = ((_dwAnimToGeomBlendShapes_TranslateTableSize + 15) & ~15);


	//-------------------


	dword _dwTransofrmIDSize = (dwDynamicTransformsCount * sizeof(long));
	dword dwTransofrmIDSize_Aligned = ((_dwTransofrmIDSize + 15) & ~15);


	dword dwTransofrmMtxCacheSize = (dwDynamicTransformsCount * sizeof(Matrix));

	//-------------------

	


	
	
	dword dwAdditionalDataSize = dwBonesGPUDataSize_Aligned + dwGeomToAnim_TranslateTableSize_Aligned + dwAnimToGeom_TranslateTableSize_Aligned + dwParticlesPtrCacheSize_Aligned + dwTransofrmIDSize_Aligned + dwParticlesPtrCacheSize_Aligned + dwMorphTargetsWeightTable_Aligned + dwAnimToGeomBlendShapes_TranslateTableSize_Aligned + dwTransofrmMtxCacheSize;
	dword dwNeedBytes = ((sizeof(MeshInstance) + 16) + dwAdditionalDataSize);

	byte * myCustomMemory = (byte *)meshAlloc(dwNeedBytes, _FL_);
	byte * myCustomMemoryEnd = ((myCustomMemory + dwNeedBytes) - 1);
	
	MeshInstance* mesh = (MeshInstance*)myCustomMemory;


	byte* dataStart = AlignPtr((myCustomMemory + sizeof(MeshInstance)));
	MESH_ALIGN_16_CHECK(dataStart);

	boneGpuData * pBonesGPUdata = NULL;
	long * pGeomToAnim_TranslateTable = NULL;
	long * pAnimToGeom_TranslateTable = NULL;
	if (dwBonesCount > 0)
	{
		pBonesGPUdata = (boneGpuData*)dataStart;
		MESH_ALIGN_16_CHECK(pBonesGPUdata);

		pGeomToAnim_TranslateTable = (long*)(dataStart + dwBonesGPUDataSize_Aligned);
		MESH_ALIGN_16_CHECK(pGeomToAnim_TranslateTable);

		pAnimToGeom_TranslateTable = (long*)(dataStart + dwBonesGPUDataSize_Aligned + dwGeomToAnim_TranslateTableSize_Aligned);
		MESH_ALIGN_16_CHECK(pAnimToGeom_TranslateTable);
	}

	MeshParticleOnLocator* pParticlesPtrCache = NULL;
	if (dwLocatorsWithParticlesCount > 0)
	{
		pParticlesPtrCache = (MeshParticleOnLocator*)(dataStart + dwBonesGPUDataSize_Aligned + dwGeomToAnim_TranslateTableSize_Aligned + dwAnimToGeom_TranslateTableSize_Aligned);
		MESH_ALIGN_16_CHECK(pParticlesPtrCache);
	}


	float * pMorphTargetsWeights = NULL;
	long * pAnimToGeomBlendShapes_TranslateTable = NULL;
	if (dwBlendShapeBonesCount > 0)
	{
		pMorphTargetsWeights = (float*)(dataStart + dwBonesGPUDataSize_Aligned + dwGeomToAnim_TranslateTableSize_Aligned + dwAnimToGeom_TranslateTableSize_Aligned + dwParticlesPtrCacheSize_Aligned);
		MESH_ALIGN_16_CHECK(pMorphTargetsWeights);

		pAnimToGeomBlendShapes_TranslateTable = (long*)(dataStart + dwBonesGPUDataSize_Aligned + dwGeomToAnim_TranslateTableSize_Aligned + dwAnimToGeom_TranslateTableSize_Aligned + dwParticlesPtrCacheSize_Aligned + dwMorphTargetsWeightTable_Aligned);
		MESH_ALIGN_16_CHECK(pAnimToGeomBlendShapes_TranslateTable);
	}


	


	long * pDynamicTransofrmsBonesID = NULL;
	Matrix * pDynamicTransofrmsBonesMatrixCache = NULL;
	if (dwDynamicTransformsCount > 0)
	{
		pDynamicTransofrmsBonesID = (long*)(dataStart + dwBonesGPUDataSize_Aligned + dwGeomToAnim_TranslateTableSize_Aligned + dwAnimToGeom_TranslateTableSize_Aligned + dwParticlesPtrCacheSize_Aligned + dwMorphTargetsWeightTable_Aligned + dwAnimToGeomBlendShapes_TranslateTableSize_Aligned);
		MESH_ALIGN_16_CHECK(pDynamicTransofrmsBonesID);

		pDynamicTransofrmsBonesMatrixCache = (Matrix*)(dataStart + dwBonesGPUDataSize_Aligned + dwGeomToAnim_TranslateTableSize_Aligned + dwAnimToGeom_TranslateTableSize_Aligned + dwParticlesPtrCacheSize_Aligned + dwTransofrmIDSize_Aligned + dwMorphTargetsWeightTable_Aligned + dwAnimToGeomBlendShapes_TranslateTableSize_Aligned);
		MESH_ALIGN_16_CHECK(pDynamicTransofrmsBonesMatrixCache);

		byte * checkPtr = (byte*)&pDynamicTransofrmsBonesMatrixCache[dwDynamicTransformsCount - 1];
		Assert((checkPtr + sizeof(Matrix)-1) < (myCustomMemory + dwNeedBytes));
	}



	
	

	//инплейс коструктор вызываем...
	new('a', mesh) MeshInstance(pRS, _container, debugMeshName, pSourceFile, Line, pBonesGPUdata, pGeomToAnim_TranslateTable, pAnimToGeom_TranslateTable, pDynamicTransofrmsBonesID, pDynamicTransofrmsBonesMatrixCache, pParticlesPtrCache, pMorphTargetsWeights, pAnimToGeomBlendShapes_TranslateTable);


	//инициализируем данные...
	float fDefaultAlpha = 1.0f;
	float fDefaultScale = 1.0f;
	for (dword i = 0; i < dwBonesCount; i++)
	{
		pAnimToGeom_TranslateTable[i] = -1;
		pGeomToAnim_TranslateTable[i] = -1;
		pBonesGPUdata[i].data[2] = Vector4(fDefaultAlpha, fDefaultScale, 0.0f, 0.0f);
	}



	return mesh;


/*
	MeshInstance * mesh = NEW MeshInstance(pRS, _container, pSourceFile, Line);
	return mesh;
*/
}

void GMXService::freeMeshInstance(MeshInstance * meshInst)
{
	meshInst->~MeshInstance();
	meshFree(meshInst, _FL_);
	//delete meshInst;
}



#ifndef STOP_DEBUG

void _cdecl GMXService::GeometryDebugModeSwitch(const ConsoleStack& stack)
{
	bGeometryDebug = !bGeometryDebug;
	if (bGeometryDebug)
	{
		pConsole->Trace(COL_ALL, "GeometryDebug - enabled");
	} else
	{
		pConsole->Trace(COL_ALL, "GeometryDebug - disabled");
	}

}

void _cdecl GMXService::BlendShapesDebugSwitch(const ConsoleStack& stack)
{
	bBlendShapeDebug = !bBlendShapeDebug;


	if (bBlendShapeDebug)
	{
		pConsole->Trace(COL_ALL, "BlendShapesDebug - enabled");
	} else
	{
		pConsole->Trace(COL_ALL, "BlendShapesDebug - disabled");
	}
}
#endif
