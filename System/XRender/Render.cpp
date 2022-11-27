#include <io.h>
#include <stdio.h>


#include "Render.h"
#include "D3DConvertor.h"
#include "..\..\common_h\SetThreadName.h"


#define HINT_CHANGE_TIME (6.0f)


#ifndef _XBOX

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#include <gdiplus.h>
#include "Device/PC_Device.h"
#include "Batch/PrecompiledBatch.h"
#include <delayimp.h>
#else
#include "Device/X360_Device.h"
#include <Xbdm.h>
#endif

#include "D3DConvertor.h"


#include "TXXLoader.h"
//#include "Lights/Light.h"
#include "RenderTargets/RenderTarget.h"
#include "RenderTargets/RenderTargetDepth.h"

#include "Fonts/Font.h"


#include "DefaultStates/DefaultStates.h"
#include "Capsule/Capsule.h"
#include "Query/OcclusionQuery.h"

#include "Shaders/ChangeList/ChangeList.h"

#include "..\..\common_h\corecmds.h"
#include "..\..\common_h\locstrings.h"

#ifndef _XBOX
#include "XLiveShower\XLiveShowerPC.h"
#endif


#include "Shaders/Shader.h"
#include "Shaders/ShaderVar.h"

#ifndef _XBOX
#include "VideoCodec\wmvcodec.h"
#else
#include "VideoCodec\xmvcodec.h"
#include "XMP.h"
#endif



#ifndef COMPILE_PROPS_INCLUDED
#error CompileProps.h must be included
#endif


#define LOADING_HINT_ID 799999
#define DYNAMIC_HINT_FIRST_ID 800000
#define DYNAMIC_HINT_LAST_ID 800010


CREATE_SERVICE_NAMED("DX9Render", NGRender, 20)



NGRender* NGRender::pThreadRS = null;

NGRender* NGRender::pRS = null;
IDirect3DDevice9* CDX8IBuffer::pD3D8 = null;
IDirect3DDevice9* CDX8VBuffer::pD3D8 = null;
IDirect3DDevice9* CDX8Texture::pD3D8 = null;


#if !(defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER))
IDirect3DDevice9* SHADER::pD3D8 = null;
#endif

IDirect3DDevice9* StateFilter::pD3D8 = null;



NGRender::SolidBoxVertex NGRender::DX8SBVertex[] = 
{       Vector(-0.50, -0.50, 0.50), Vector (0.00, 0.00, 1.00),
Vector(0.50, -0.50, 0.50), Vector (0.00, 0.00, 1.00),
Vector(-0.50, 0.50, 0.50), Vector (0.00, 0.00, 1.00),
Vector(0.50, 0.50, 0.50), Vector (0.00, 0.00, 1.00),
Vector(-0.50, 0.50, 0.50), Vector (0.00, 1.00, 0.00),
Vector(0.50, 0.50, 0.50), Vector (0.00, 1.00, 0.00),
Vector(-0.50, 0.50, -0.50), Vector (0.00, 1.00, 0.00),
Vector(0.50, 0.50, -0.50), Vector (0.00, 1.00, 0.00),
Vector(-0.50, 0.50, -0.50), Vector (0.00, 0.00, -1.00),
Vector(0.50, 0.50, -0.50), Vector (0.00, 0.00, -1.00),
Vector(-0.50, -0.50, -0.50), Vector (0.00, 0.00, -1.00),
Vector(0.50, -0.50, -0.50), Vector (0.00, 0.00, -1.00),
Vector(-0.50, -0.50, -0.50), Vector (0.00, -1.00, 0.00),
Vector(0.50, -0.50, -0.50), Vector (0.00, -1.00, 0.00),
Vector(-0.50, -0.50, 0.50), Vector (0.00, -1.00, 0.00),
Vector(0.50, -0.50, 0.50), Vector (0.00, -1.00, 0.00),
Vector(0.50, -0.50, 0.50), Vector (1.00, 0.00, 0.00),
Vector(0.50, -0.50, -0.50), Vector (1.00, 0.00, 0.00),
Vector(0.50, 0.50, 0.50), Vector (1.00, 0.00, 0.00),
Vector(0.50, 0.50, -0.50), Vector (1.00, 0.00, 0.00),
Vector(-0.50, -0.50, -0.50), Vector (-1.00, 0.00, 0.00),
Vector(-0.50, -0.50, 0.50), Vector (-1.00, 0.00, 0.00),
Vector(-0.50, 0.50, -0.50), Vector (-1.00, 0.00, 0.00),
Vector(-0.50, 0.50, 0.50), Vector (-1.00, 0.00, 0.00) };

short NGRender::DX8SBIndices[] = 
{       0, 1, 2, 1, 3, 2, 4, 5, 6, 5, 7, 6, 8, 9, 
10, 9, 11, 10, 12, 13, 14, 13, 15, 14, 16, 
17, 18, 17, 19, 18, 20, 21, 22, 21, 23, 22 };


enum MemInfoType
{
	MIT_VERTEX_BUFFER = 0,
	MIT_INDEX_BUFFER,
	MIT_TEXTURE,
	MIT_RENDER_TARGET,
	MIT_RENDER_TARGET_DEPTH,

	MIT_FORCE_DWORD = 0x7fffffff
};

struct MemInfo
{
	MemInfoType t;
	dword dwMemCount;
	const char* srcFile;
	long srcLine;
	const char* additional;

	MemInfo()
	{
		t = MIT_FORCE_DWORD;
		srcFile = NULL;
		dwMemCount = 0x0;
		additional = NULL;
		srcLine = -1;
	}


	MemInfo(MemInfoType _t, dword mem, const char* _file, long _line)
	{
		t = _t;
		dwMemCount = mem;
		srcFile = _file;
		srcLine = _line;
		additional = NULL;
	}

	MemInfo(MemInfoType _t, dword mem, const char* _file, long _line, const char* _add)
	{
		t = _t;
		additional = _add;
		dwMemCount = mem;
		srcFile = _file;
		srcLine = _line;
	}
};






bool MemoryDumpCompare(const MemInfo &a, const MemInfo &b)
{
	if (b.dwMemCount < a.dwMemCount) return true;
	return false;
}


void LockRes (const char* szSrcFile, long SrcLine, const CritSection& sec, const char* the_text)
{
	sec.Enter();

	/*
	char text[16384];
	crt_snprintf(text, 10000, "(%s, %d), id : 0x%08X - %s::Lock()\n", szSrcFile, SrcLine, GetCurrentThreadId(), the_text);
	OutputDebugString(text);
	*/
#ifdef _XBOX
	NGRender::pRS->D3D()->AcquireThreadOwnership();
#endif
}

void UnlockRes (const char* szSrcFile, long SrcLine, const CritSection& sec, const char* the_text)
{
#ifdef _XBOX
	//Мы из основного потока...
	if (api->GetThreadId() == ::GetCurrentThreadId())
	{
		//Идет процесс загрузки, нужно освободить устройство....
		if (NGRender::pRS->loadingScreenThread)
		{
			NGRender::pRS->D3D()->ReleaseThreadOwnership();
		}
	}
#endif


	sec.Leave();
	/*
	char text[16384];
	crt_snprintf(text, 10000, "(%s, %d), id : 0x%08X - %s::Unlock()\n", szSrcFile, SrcLine, GetCurrentThreadId(), the_text);
	OutputDebugString(text);
	*/
}



NGRender::NGRender() : aTextures (_FL_),
aRenderTargets(_FL_),
aRenderTargetsDepth(_FL_),
aVBuffers(_FL_),
aIBuffers(_FL_),
aLights(_FL_),
passLights(_FL_, 32),
aFonts(_FL_),

stViewports(_FL_, 128),
stRenderTarget(_FL_, 128),

aRenderFilters(_FL_, 128),

aBufferedPrints(_FL_, 512),
aBufferedLines(_FL_, 512),
aPolygons(_FL_, 512),

perfGraphs(_FL_, 128),

EnabledFilters(_FL_, 32),

tempRenderTargetsC(_FL_, 32),
tempRenderTargetsD(_FL_, 32),                        
currentEvents(_FL_, 4096),
activeLights(_FL_, 32),

loadingHints(_FL_, 64),
legalVideos(_FL_, 8),

queries(_FL_, 16),

strTitle(_MAX_PATH),
strPath(_MAX_PATH),
strFullPath(_MAX_PATH),
strTexName(_MAX_PATH)

{
#ifdef RENDER_DEBUG_DRAW_SKIP
	curSkip = -1;
	curSkip100 = -100;
#endif

	AssertCoreThread;

	pak = NULL;
	mirror = NULL;

	x360_played_legal = NULL;

	bInsideFrame = false;

	activeHintTime = 0.0f;
	activeHintID = 0;

	fCompassArrowAngle = 0.0f;
	bDepthDisabled = true;

	storageBrightness = null;

	vertexStreams[0].vertexData = NULL;
	vertexStreams[0].dwStride = 0;
	vertexStreams[0].dwOffsetFromStartInBytes = 0;

	vertexStreams[1].vertexData = NULL;
	vertexStreams[1].dwStride = 0;
	vertexStreams[1].dwOffsetFromStartInBytes = 0;

	vertexStreams[2].vertexData = NULL;
	vertexStreams[2].dwStride = 0;
	vertexStreams[2].dwOffsetFromStartInBytes = 0;

	deviceIndex = 0;

	dwInitTime = 0;
	dwInitTimeShaders = 0;


	dwLastTimeWhenTooltipChange = 0;

	bConsoleRegistred = false;

	dwPixGUID = 1;

	fLoaderFontScale = 1.0f;

	mtxWorldView = NULL;
	mtxWorldViewProj = NULL;
	gmxFinalShdwMatrix = NULL;

	bExpand2FullScreen = false;
	bMaximize2FullScreen = false;
	winPosX = 10;
	winPosY = 10;


	frustumAligned = AlignPtr((Plane *)&frustumRaw[0]);
	//ALIGN_16_CHECK(frustumAligned);


	dword ptr = (dword)((byte*)&matrixArray[0]);

	byte* alignedPtr = (byte*)(ptr + 15 & ~15);

	mtxWorldView = (Matrix*)alignedPtr;
	alignedPtr += sizeof(Matrix);

	mtxWorldViewProj = (Matrix*)alignedPtr;
	alignedPtr += sizeof(Matrix);

	gmxFinalShdwMatrix = (Matrix*)alignedPtr;
	alignedPtr += sizeof(Matrix);

	mtxView = (Matrix*)alignedPtr;
	alignedPtr += sizeof(Matrix);

	mtxWorld = (Matrix*)alignedPtr;
	alignedPtr += sizeof(Matrix);

	mtxProjection = (Matrix*)alignedPtr;
	alignedPtr += sizeof(Matrix);

	mShadow = (Matrix*)alignedPtr;
	alignedPtr += sizeof(Matrix);


	stdVariables.mWorldViewProj = mtxWorldViewProj;
	stdVariables.mWorld = mtxWorld;
	stdVariables.mView = mtxView;
	stdVariables.mProjection = mtxProjection;
	stdVariables.mInverseView = &mtxViewInv;
	stdVariables.mInverseWorld = &mtxWorldInv;
	stdVariables.vCamPosRelativeWorld = &vCameraPosition_WorldSpace;
	stdVariables.vCamDirRelativeWorld = &vCameraDirection_WorldSpace;









	passLights.Reserve(32);


#ifdef SHOW_HISTOGRAM_DELETE_ME_BEFORE_RELEASE
	histogram_update_chunk = 256;
	bShowHistogram = false;
	histogramm_bin = 0;
	histogram = NULL;
	histogram_source = NULL;
	histogram_min = NULL;
	histogram_max = NULL;
	_histogram_query = NULL;
	histogram_totalPixelsInScreen = 1.0f;

	for (dword i = 0; i < 255; i++)
	{
		histogram_data[i] = 0.0f;
	}
#endif

	stdLightVariables.ambientLight.vAmbientLight.a = 1.0f;

	pTextureProgress = NULL;
	pTextureArrow = NULL;


	bForceWideScreen = -1;
	_finalClean = atomicI32::False;
	_bNeedCloseThread = atomicI32::False;

	pLastIndices = NULL;
	lastIBUsageFlag = 0;
	lastVBUsageFlag = 0;

	GMX_ShadowLimiters = NULL;
	GMX_ShadowMatrix = NULL;


	width_div_height_k = 4.0f / 3.0f;

	fWidthAspectMultipler = 1.0f;
	multiSample = 0;
	bDeviceReseted = false;
	shaderTime = NULL;
	fTimeToShader = 0.0f;

	bNeedResetDevice = false;

	bCantRender = false;

	LoadingScreenThreadId = 0;

	varColor = NULL;
	varTexture = NULL;

	largeshot = null;


	pReloadingTexture = NULL;

	loading_percents = 0.0f;
	loading_percents_total = 0.0f;


	//bIsWideScreen = false;

	aspect = ASPECT_4_TO_3;


	bDisableHacks = false;
	bPostProcessAlreadyDone = false;

	loadingScreenThread = NULL;

	bDontShowReloading = false;
	waitGPUFrames = -1;

	pMemVB = NULL;
	pMemIB = NULL;

	bNeedReplaceDP_to_DPUP = false;
	NGRender::pRS = this;
	NGRender::pThreadRS = this;





	for (dword n = 0 ; n < SH_MAX; n++)
	{
		hacks[n].enabled = false;
		hacks[n].v = -1;
	}





	passLights.Reserve(128);

	bDrawStats = true;

#ifndef _XBOX
	bShowStatistic = false;
#else

	bShowStatistic = false;

#endif

	bPostProcessEnabled = true;
	dwTextureDegradation = 0;

	redundantStatesFilter = NULL;

	m_WhiteEmptyTexture = NULL;
	pSpecularTexture = NULL;
	dwNumSphereTrgs = 0;
	pVSphereBuffer = null;
	pVSolidBox = null;
	pISolidBox = null;

	pSpritesVBuffer = null;
	pRectsVBuffer = null;
	pRectsIBuffer = null;

	capsuleRender = NULL;
	pFonts = NULL;
	pAdvFonts = NULL;
	pSystemFont = NULL;
	//        pLoaderFont = NULL;
	pAdvLoaderFont = NULL;


	bNeedRecalculateFrustum = true;

	m_TextureDirectory = "resource\\textures\\";

	iScreenshotFrameIndex = 0;
	dwCurrentBaseVertexIndex = 0;

	m_TXXLoader = NULL;
	bInsideBeginScene = false;
	m_backColor = Color(0.4f, 0.4f, 0.4f);

	m_Screen2DInfo.BackBufferFormat = FMT_X8R8G8B8;
	m_Screen2DInfo.StencilFormat = FMT_D24S8;
	m_Screen2DInfo.bWindowed = true;
	m_Screen2DInfo.dwWidth = 1024;
	m_Screen2DInfo.dwHeight = 768;
	m_Screen2DInfo.fFOV = 1.0f;


	pDevice = NULL;


	float fNearV = 1.0f;
	float fFarV = 0.0f;
	float fNearU = 0.0f;
	float fFarU = 1.0f;

#ifdef _XBOX
	PostProcessQuad[0].v0 = fNearV;
	PostProcessQuad[0].u0 = fNearU; 

	PostProcessQuad[1].v0 = fNearV;
	PostProcessQuad[1].u0 = fFarU; 

	PostProcessQuad[2].v0 = fFarV;
	PostProcessQuad[2].u0 = fFarU; 

	PostProcessQuad[3].v0 = fFarV;
	PostProcessQuad[3].u0 = fNearU; 

#else

	PostProcessQuad[0].v0 = fFarV;
	PostProcessQuad[0].u0 = fNearU; 

	PostProcessQuad[1].v0 = fNearV;
	PostProcessQuad[1].u0 = fNearU; 

	PostProcessQuad[2].v0 = fFarV;
	PostProcessQuad[2].u0 = fFarU; 

	PostProcessQuad[3].v0 = fNearV;
	PostProcessQuad[3].u0 = fFarU; 


#endif

	evtLegalVideosDone = CreateEvent( null, true, false, null );



	m_pXLiveShower = null;
}


bool NGRender::IsFinalCleanup()
{
	return _finalClean != atomicI32::False;
}

void NGRender::LoadLegalVideos(bool bFirstTime)
{
	if (bFirstTime)
	{
		dwLegalVideoIdx = 0;
		bNeedToStartVideoPlayer = FALSE;
	} else
	{
		if (bShowLegalVideos == false)
		{
			return;
		}
	}


	if (bDontShowReloading)
	{
		return;
	}

	LockRes(_FL_, resource_creation_sc, "LoadLegalVideos");


	// имя видеоролика для текущего видео
	static char startupVideoFullname[MAX_PATH];

	int video_startIdx = 0;
	int video_maxIdx = 1;
	if (bFirstTime == false)
	{
		video_startIdx = 1;
		video_maxIdx = 5;
	}

	for (int legalVideoIdx = video_startIdx; legalVideoIdx < video_maxIdx; legalVideoIdx++)
	{
#ifdef _XBOX
		crt_snprintf(startupVideoFullname, sizeof(startupVideoFullname), "GAME:\\legal%02d.wmv", legalVideoIdx);
#else
		crt_snprintf(startupVideoFullname, sizeof(startupVideoFullname), "resource\\videos\\legal%02d.wmv", legalVideoIdx);
#endif

		IVideoCodec* pLegalVideo = CreateVideoCodec(false);

		if (pLegalVideo)
		{
			bool bLoadedDone = pLegalVideo->OpenFile(startupVideoFullname);
			if (bLoadedDone)
			{
				pLegalVideo->SetViewPosition(0.f,0.f,1.f,1.f);

				x360_vidArray.Enter();

				legalVideos.Add(pLegalVideo);

				x360_vidArray.Leave();
			} else
			{
				DELETE(pLegalVideo);
			}
		}
	}

	UnlockRes(_FL_, resource_creation_sc, "LoadLegalVideos");

	x360_vidArray.Enter();

	if (bFirstTime)
	{
		if (legalVideos.Size() > 0)
		{
			bShowLegalVideos = TRUE;
			bNeedToStartVideoPlayer = TRUE;

			x360_played_legal = legalVideos[0];
			//legalVideos[dwLegalVideoIdx]->Start();                        
		} else
		{
			//иначе зависнет, когда видео нет...
			SetEvent( evtLegalVideosDone );
			bShowLegalVideos = FALSE;
			bNeedToStartVideoPlayer = FALSE;
		}
	}

	x360_vidArray.Leave();

}

void NGRender::UnloadLegalVideos()
{
	x360_vidArray.Enter();

	if (legalVideos.Size() <= 0)
	{
		return;
	}


	LockResourceCreation("unload videos");


#ifdef _XBOX
	if( legalVideos.Size() > 0 )
		XMPRestoreBackgroundMusic();
#endif

	bShowLegalVideos = FALSE;
	for (dword i = 0; i < legalVideos.Size(); i++)
	{
		DELETE(legalVideos[i]);
	}

	legalVideos.DelAll();

	UnLockResourceCreation("unload videos");

	x360_vidArray.Leave();

}

NGRender::~NGRender()
{
	api->Trace("\nNGRender::~NGRender()\n");

	AssertCoreThread;

	SetEvent( evtLegalVideosDone );

	RELEASE(pak);
	RELEASE(mirror);


	RELEASE(storageBrightness);


	UnloadLegalVideos();

	//Убиваем поток с загрузочным экраном (если он есть)
	if (loadingScreenThread != NULL)
	{
		api->Trace("\nNGRender - delete loading screen thread...start\n");

		_bNeedCloseThread = atomicI32::True;

		WaitForSingleObject(loadingScreenThread, INFINITE);


		CloseHandle(loadingScreenThread);
		loadingScreenThread = NULL;

		api->Trace("\nNGRender - delete loading screen thread...done\n");
	}



	IFileService * fs = (IFileService *)api->GetService("FileService");


#ifndef _XBOX
	HWND winHandle = (HWND)api->Storage().GetLong("system.hwnd");
	DWORD dwFlags = GetWindowLong(winHandle, GWL_STYLE);
	if ((fs && d3dpp.Windowed) && (bExpand2FullScreen == false && bMaximize2FullScreen == false))
	{
		IEditableIniFile * ini = fs->OpenEditableIniFile(api->Storage().GetString("system.ini"), file_open_always, _FL_);
		if(ini)
		{
			long px = api->Storage().GetLong("system.winPosX", 10);
			long py = api->Storage().GetLong("system.winPosY", 10);

			ini->SetLong("", "windowPosX", px);
			ini->SetLong("", "windowPosY", py);

			ini->Release();
		}
	}
#endif

	_finalClean = atomicI32::True;
	_bNeedCloseThread = atomicI32::True;

	DELETE( m_pXLiveShower );


#ifdef _XBOX
	NGRender::pRS->D3D()->UnsetAll();
#endif

	api->Trace("NGRender::Shutdown begin\n");

	if (pDevice)
	{
		pDevice->Cleanup();
	}

	RELEASE(largeshot);

	shaderTime = NULL;

	RELEASE(pSystemFont);
	RELEASE(pAdvLoaderFont);


	GMX_ShadowLimiters = NULL;
	GMX_ShadowMatrix = NULL;


	varColor = NULL;
	varTexture = NULL;




	RELEASE(pTextureProgress);
	RELEASE(pTextureArrow);



#ifdef SHOW_HISTOGRAM_DELETE_ME_BEFORE_RELEASE
	RELEASE(histogram);
	histogram_source = NULL;

	RELEASE(_histogram_query);

	histogram_min = NULL;
	histogram_max = NULL;
#endif



	d3d9Sync.Release();


	DELETE (pAdvFonts);
	DELETE(pFonts);


	RELEASE(pReloadingTexture);

	RELEASE(m_WhiteEmptyTexture);
	RELEASE(pSpecularTexture);

	RELEASE(pSpritesVBuffer);
	RELEASE(pRectsVBuffer);
	RELEASE(pRectsIBuffer);
	RELEASE(pVSphereBuffer);

	DELETE (capsuleRender);

	RELEASE(pVSolidBox);
	RELEASE(pISolidBox);


	DELETE (m_TXXLoader);

	api->Trace("NGRender::Shutdown shader manager\n");

	m_ShaderManager.Destroy();



	dword i = 0;
	dword dwLeaks = 0;
	for (i=0; i<aVBuffers.Size(); i++)
	{
		IVBuffer* pB = aVBuffers[i];
		api->Trace("DX8: Unreleased vertex buffer with size: %d", pB->GetSize());
		api->Trace("File: %s, line: %d", pB->GetFileName(), pB->GetFileLine());
		api->Trace("");
		pB->ForceRelease();
		dwLeaks++;
		i--;
	}

	for (i=0; i<aRenderTargets.Size(); i++)
	{
		api->Trace("DX8: Unreleased render target with size %d", aRenderTargets[i]->GetSize());
		api->Trace("File: %s, line: %d", aRenderTargets[i]->GetFileName(), aRenderTargets[i]->GetFileLine());
		api->Trace("");
		aRenderTargets[i]->ForceRelease();
		dwLeaks++;
		i--;
	}

	for (i=0; i<aRenderTargetsDepth.Size(); i++)
	{
		api->Trace("DX8: Unreleased render target depth with size %d", aRenderTargetsDepth[i]->GetSize());
		api->Trace("File: %s, line: %d", aRenderTargetsDepth[i]->GetFileName(), aRenderTargetsDepth[i]->GetFileLine());
		api->Trace("");
		aRenderTargetsDepth[i]->ForceRelease();
		dwLeaks++;
		i--;
	}


	for (i=0; i<aIBuffers.Size(); i++)
	{
		CDX8IBuffer * pB = (CDX8IBuffer*)aIBuffers[i];
		api->Trace("DX8: Unreleased index buffer with size: %d", pB->GetSize());
		api->Trace("File: %s, line: %d", pB->GetFileName(), pB->GetFileLine());
		api->Trace("");
		pB->ForceRelease();
		dwLeaks++;
		i--;
	}


	for (i=0; i<aLights.Size(); i++)
	{
		ILight * pL = aLights[i];
		api->Trace("DX8: Unreleased light, Pos = %.3f, %.3f, %.3f, Radius = %.3f", pL->GetPos().x, pL->GetPos().y, pL->GetPos().z, pL->GetRadius());
		//api->Trace("File: %s, line: %d", pL->GetFileName(), pL->GetFileLine());
		api->Trace("");
		delete (aLights[i]);
		dwLeaks++;
		i--;
	}

	for (i = 0; i<aTextures.Size(); i++)
	{
		CDX8Texture* pT = aTextures[i];

		api->Trace("DX8: Unreleased texture with size %d, refcount %d, texname \"%s\"", pT->GetSize(), pT->GetRef(), (pT->GetName()) ? pT->GetName() : "");
		api->Trace("File: %s, line: %d", pT->GetFileName(), pT->GetFileLine());
		api->Trace("");
		pT->ForceRelease();
		dwLeaks++;
		i--;
	}


	for (i = 0; i < queries.Size(); i++)
	{
		OcclusionQuery* q = queries[i];
		api->Trace("DX8: Unreleased query 0x%08X", q);
		q->Release();
	}



	queries.DelAll();
	aTextures.DelAll();
	aRenderTargets.DelAll();
	aRenderTargetsDepth.DelAll();
	aVBuffers.DelAll();
	aIBuffers.DelAll();
	aLights.DelAll();


	DELETE (redundantStatesFilter);


	if (!dwLeaks)
		api->Trace("Render released successfully.");
	else
		api->Trace("Render released with %d resource leaks.", dwLeaks);





	DELETE (pDevice);
	RELEASE(pD3D);

	NGRender::pRS = null;

	api->Trace("NGRender::Shutdown finished\n");
}

IRenderDevice* NGRender::Device()
{
	return pDevice;
}

void NGRender::RestoreIfNeed ()
{
	if (::GetCurrentThreadId() == api->GetThreadId())
	{
		if (bNeedResetDevice)
		{
			OnResetDevice();
		}
	}
}




ITexture* NGRender::getSpecularTexture()
{
	/*
	LockRes(_FL_, resource_creation_sc, "getSpecularTexture");
	UnlockRes(_FL_, resource_creation_sc, "getSpecularTexture");
	*/
	return pSpecularTexture;
}



ITexture* NGRender::getWhiteTexture()
{
	/*
	LockRes(_FL_, resource_creation_sc, "getWhiteTexture");
	UnlockRes(_FL_, resource_creation_sc, "getWhiteTexture");
	*/
	Assert(m_WhiteEmptyTexture);
	return m_WhiteEmptyTexture;

}

#ifndef _XBOX

//============================================================================
// Structured Exception Handler for delay loaded DLLs
static LONG WINAPI DelayLoadDllExceptionFilter(PEXCEPTION_POINTERS pep, int& error)
{
	// If this is a Delay-load problem, ExceptionInformation[0] points 
	// to a DelayLoadInfo structure that has detailed error info
	PDelayLoadInfo pdli = PDelayLoadInfo(pep->ExceptionRecord->ExceptionInformation[0]);

	switch(pep->ExceptionRecord->ExceptionCode)
	{
	case VcppException(ERROR_SEVERITY_ERROR, ERROR_MOD_NOT_FOUND):
		error = 1;
		return EXCEPTION_EXECUTE_HANDLER;

	case VcppException(ERROR_SEVERITY_ERROR, ERROR_PROC_NOT_FOUND):
		// The DLL was found but it doesn't contain the function
		error = 2;
		return EXCEPTION_EXECUTE_HANDLER;

	default:
		// We don't recognize this exception
		error = 3;
		return EXCEPTION_CONTINUE_SEARCH;
		break;
	}
}

static bool TryInitD3DX()
{
	int errorCode = 0;
	bool bRet = true;
	__try
	{
		// Do a simple matrix multiply
		D3DXMATRIXA16 m1, m2, m3;
		memset(&m1, 0, sizeof(m1));
		memset(&m2, 0, sizeof(m2));
		D3DXMatrixMultiply(&m3, &m1, &m2);
	}
	__except (DelayLoadDllExceptionFilter(GetExceptionInformation(), errorCode))
	{
		bRet = false;
	}

	return bRet;
}
#endif


bool NGRender::Init()
{
	AssertCoreThread;

	DWORD dwStartInit = GetTickCount();

	IFileService * fs = (IFileService *)api->GetService("FileService");

	RELEASE(pak);
	RELEASE(mirror);

#ifndef _XBOX
	pak = fs->LoadPack("resource\\InitTextures.pkx", _FL_);
#else
	pak = fs->LoadPack("InitTextures.pkx", _FL_);
#endif
	//pak = fs->LoadPack("resource\\InitTextures\\InitTextures.pkx", _FL_);
	mirror = fs->CreateMirrorPath("resource\\InitTextures", "resource\\textures", _FL_);




#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)
	precompiledBatch::DeleteTraceFile();
#endif

	pDevice = NULL;
	pD3D = NULL;


	int errorCode = 0;

#ifndef _XBOX
	if (TryInitD3DX() == false)
	{
		//bRet = false;
		MakeError(ERR_D3DX);

		RELEASE(pak);
		RELEASE(mirror);
		return false;
	}
#endif



	//ConvertFX2FXL("win32shaders20.fx", "win32shaders20.fxl");

	largeshot = api->Storage().GetItemLong("system.screenshot.Largeshot", _FL_);

	ReadIniParams();

	if (!CreateDevice())
	{
		api->Trace("RenderService::Init() - Can't create device !!!");
		RELEASE(pak);
		RELEASE(mirror);
		return false;
	}



	//CompilePCShaders();



	D3DCAPS9 caps;
	D3D()->GetDeviceCaps(&caps);
	if (caps.PixelShaderVersion < D3DPS_VERSION(3,0))
	{
		MakeError(ERR_SHADERS30);
		RELEASE(pak);
		RELEASE(mirror);
		return false;
	} 




	OutputDebugString("Shader creation start\n");




	// white texture
	RENDERLOCKED_RECT lr;
	m_WhiteEmptyTexture = CreateTexture(4, 4, 1, 0, FMT_DXT1, _FL_);
	if (!m_WhiteEmptyTexture)
	{
		MakeError(ERR_INIT);
		RELEASE(pak);
		RELEASE(mirror);
		return false;
	}

	if (m_WhiteEmptyTexture->LockRect(0, &lr, null, 0))
	{
		((dword*)lr.pBits)[0] = 0xFFFFFFFF;
		((dword*)lr.pBits)[1] = 0x00;
		m_WhiteEmptyTexture->UnlockRect(0);
	} else
	{
		MakeError(ERR_INIT);
		RELEASE(pak);
		RELEASE(mirror);
		return false;
	}

	m_FullScreenViewPort_2D.X      = 0;
	m_FullScreenViewPort_2D.Y      = 0;
	m_FullScreenViewPort_2D.Width  = GetScreenInfo2D().dwWidth;
	m_FullScreenViewPort_2D.Height = GetScreenInfo2D().dwHeight;
	m_FullScreenViewPort_2D.MinZ   = 0.0f;
	m_FullScreenViewPort_2D.MaxZ   = 1.0f;

	m_FullScreenViewPort_3D.X      = 0;
	m_FullScreenViewPort_3D.Y      = 0;
	m_FullScreenViewPort_3D.Width  = GetScreenInfo3D().dwWidth;
	m_FullScreenViewPort_3D.Height = GetScreenInfo3D().dwHeight;
	m_FullScreenViewPort_3D.MinZ   = 0.0f;
	m_FullScreenViewPort_3D.MaxZ   = 1.0f;

	SetViewport(m_FullScreenViewPort_3D);
	SetPerspective(GetScreenInfo3D().fFOV);



	mIdentity.SetIdentity();
	SetCamera(Vector(1.0f, 2.0f, 2.0f), 0.0f, Vector(0.0f, 1.0f, 0.0f));
	SetPerspective(GetScreenInfo2D().fFOV);

	DWORD dwShaderStartTime = GetTickCount();


	redundantStatesFilter = NEW StateFilter;
	redundantStatesFilter->SetDefaultStates();

	BuildHintsList();

	activeHintTime = 0.0f;
	activeHintID = 0;




#ifdef _XBOX
	//На боксе перед шейдерами включаем лоадер, на PC потом
	XMPOverrideBackgroundMusic();
	LoadLegalVideos(true);
	EnableLoadingScreen(true);
	LoadLegalVideos(false);
#endif



	m_ShaderManager.SetD3D(D3D());
	m_ShaderManager.Build();

	DWORD dwShaderEndTime = GetTickCount();

	dwInitTimeShaders = dwShaderEndTime-dwShaderStartTime;

	OutputDebugString("Shader created done... continue execution.\n");


	m_TXXLoader = NEW TXXLoader(this);


	pDevice->Init();



	LockResourceCreation("Fonts creation");

	redundantStatesFilter->SetDefaultStates();

	pFonts = NEW CDX8Fonts(this);
	pSystemFont = pFonts->CreateFont("Arial", -1.0f, 0xFFFFFFFF);

#ifdef _XBOX
	D3D()->AcquireThreadOwnership();
#endif
	UnLockResourceCreation("Fonts creation");





	fLoaderFontScale = (1.0f / ((16.0f / 9.0f) / width_div_height_k));
	if (fLoaderFontScale > 1.0f)
	{
		fLoaderFontScale = 1.0f;// / fLoaderFontScale;
	}

	api->Trace("Loader font scale : %f\n", fLoaderFontScale);
	api->Trace("Render: Resolution3D %dx%d, Resolution2D %dx%d", GetScreenInfo3D().dwWidth, GetScreenInfo3D().dwHeight, GetScreenInfo2D().dwWidth, GetScreenInfo2D().dwHeight);



	// sprites
	pSpritesVBuffer = CreateVertexBuffer(MAX_RECTS_NUM * sizeof(RS_SPRITE) * 4, sizeof(RS_SPRITE), _FL_, USAGE_WRITEONLY | USAGE_DYNAMIC, POOL_DEFAULT);

	pVSolidBox = CreateVertexBuffer(sizeof(DX8SBVertex), sizeof(SolidBoxVertex), _FL_);
	pISolidBox = CreateIndexBuffer(sizeof(DX8SBIndices), _FL_);
	pVSolidBox->Copy(DX8SBVertex, sizeof(DX8SBVertex));
	pISolidBox->Copy(DX8SBIndices, sizeof(DX8SBIndices));

	// rects 
	pRectsVBuffer = CreateVertexBuffer(MAX_RECTS_NUM * sizeof(RECT_VERTEX) * 4, sizeof(RECT_VERTEX), _FL_, USAGE_WRITEONLY | USAGE_DYNAMIC, POOL_DEFAULT);
	pRectsIBuffer = CreateIndexBuffer(MAX_RECTS_NUM * 6 * sizeof(WORD), _FL_, USAGE_WRITEONLY | USAGE_DYNAMIC);
	WORD * pTrgs = (WORD*)pRectsIBuffer->Lock(); 
	if (pTrgs)
	{
		for (dword i = 0; i < MAX_RECTS_NUM; i++)
		{
			pTrgs[i * 6 + 0] = WORD(i * 4 + 0);
			pTrgs[i * 6 + 1] = WORD(i * 4 + 1);
			pTrgs[i * 6 + 2] = WORD(i * 4 + 2);
			pTrgs[i * 6 + 3] = WORD(i * 4 + 0);
			pTrgs[i * 6 + 4] = WORD(i * 4 + 2);
			pTrgs[i * 6 + 5] = WORD(i * 4 + 3);
		}
	}

	pRectsIBuffer->Unlock();

	CreateSphere();
	capsuleRender = NEW TCapsule(this);


	// specular texture
	dword dwTextureSize = 32;
	pSpecularTexture = CreateTexture(dwTextureSize, dwTextureSize, 1, 0, FMT_A8R8G8B8, _FL_);

	if (!pSpecularTexture)
	{
		MakeError(ERR_INIT);
		RELEASE(pak);
		RELEASE(mirror);
		return false;
	}

	if (pSpecularTexture->LockRect(0, &lr, null, 0))
	{
		for (dword lDOTn = 0; lDOTn < dwTextureSize; lDOTn++)
		{
			for (dword PowK = 0; PowK < dwTextureSize; PowK++)
			{
				float rDotL = (lDOTn / (float)(dwTextureSize-1));
				float Shininess = ((PowK / (float)(dwTextureSize-1)));

				Shininess = Shininess * 32.0f;
				float PowRes = Clampf(pow(rDotL, Shininess));

				byte v = (byte)(PowRes * 255.0f);
				/*
				((byte*)lr.pBits)[((dwTextureSize-1) - PowK) * lr.Pitch + (lDOTn*4) + 0] = v;
				((byte*)lr.pBits)[((dwTextureSize-1) - PowK) * lr.Pitch + (lDOTn*4) + 1] = v;
				((byte*)lr.pBits)[((dwTextureSize-1) - PowK) * lr.Pitch + (lDOTn*4) + 2] = v;
				((byte*)lr.pBits)[((dwTextureSize-1) - PowK) * lr.Pitch + (lDOTn*4) + 3] = v;
				*/
				((byte*)lr.pBits)[PowK * lr.Pitch + (lDOTn*4) + 0] = v;
				((byte*)lr.pBits)[PowK * lr.Pitch + (lDOTn*4) + 1] = v;
				((byte*)lr.pBits)[PowK * lr.Pitch + (lDOTn*4) + 2] = v;
				((byte*)lr.pBits)[PowK * lr.Pitch + (lDOTn*4) + 3] = v;

			}
		}
		pSpecularTexture->UnlockRect(0);

		//SaveTexture2File(pSpecularTexture, "c:\\specular_test.dds");
	}








	stdLightVariables.ambientLight.vAmbientLight = Color (1.0f, 1.0f, 1.0f);
	stdLightVariables.ambientLight.vAmbientLight.a = 1.0f;

	stdLightVariables.ambientLight.vAmbientLightSpecular = Color (1.0f, 1.0f, 1.0f);
	stdLightVariables.ambientLight.vAmbientLightSpecular.a = 1.0f;



	SetGlobalLight(Vector(0.0f, 1.0f, 0.0f), false, Color(3.0f, 3.0f, 3.0f), Color(0.01f, 0.01f, 0.01f));
	setFogParams(0.0f, 0.0f, 100.0f, 0.0f, 0.0f, 100.0f, Color(0.0f));



	LockResourceCreation("AdvFonts creation");
	pAdvFonts = NEW AdvancedFonts();

	float m_hk = GetFullScreenViewPort_2D().Height / 1024.0f;
	pAdvLoaderFont = pAdvFonts->CreateAdvancedFont("Blood", m_hk * 43.0f, 0xFFF0F0F0, "ls_dbgFontAlpha", "ls_Circular", "ls_Circular_shadow");

#ifdef _XBOX
	D3D()->AcquireThreadOwnership();
#endif

	redundantStatesFilter->SetDefaultStates();
	d3d9Sync.Init(D3D(), waitGPUFrames);



	UnLockResourceCreation("AdvFonts creation");



	api->SetStartFrameLevel(this, Core_DefaultExecuteLevel);
	api->SetEndFrameLevel(this, Core_DefaultExecuteLevel + 0x1000);


	/*
	IVariable* guiMtx = GetTechniqueGlobalVariable("GUI_CliperMatrix", _FL_);
	if (guiMtx)
	{
	guiMtx->SetMatrix(Matrix());
	guiMtx->Release();
	}
	*/


	/*

	IVariable* temp0 = GetTechniqueGlobalVariable("vLightDir", _FL_);
	IVariable* temp1 = GetTechniqueGlobalVariable("vCenter", _FL_);
	IVariable* temp2 = GetTechniqueGlobalVariable("vScale", _FL_);
	IVariable* temp3 = GetTechniqueGlobalVariable("vColor", _FL_);
	IVariable* temp4 = GetTechniqueGlobalVariable("SphereColor", _FL_);
	IVariable* temp5 = GetTechniqueGlobalVariable("RectTexture", _FL_);

	*/






	/*
	pFreeCamera = (IFreeCamera*)api->CreateObject("FreeCamera");
	pFreeCamera->SetPosition(Vector (9.0f, 7.0f, -12.0f));
	pFreeCamera->SetTarget(Vector (0.0f, 0.0f, 0.0f));
	*/



	pTextureProgress = CreateTextureFullQuality(_FL_, "i_progress");
	pTextureArrow = CreateTextureFullQuality(_FL_, "i_compass_arrow");

#ifdef GAME_RUSSIAN
	InitProtectData(dword(this), dword(fs));
#endif

	IIniFile * iniFile = fs->SystemIni();
	Assert(iniFile);

	const char * localeSetting = iniFile->GetString(null, "localization", null);

#ifdef _XBOX
	DWORD dwXboxDashLanguage = XGetLanguage();
	switch( XGetLanguage() )
	{
		
		case XC_LANGUAGE_ENGLISH: localeSetting = "eng"; break;
			//case XC_LANGUAGE_JAPANESE:
			//case XC_LANGUAGE_GERMAN German 
			//case XC_LANGUAGE_FRENCH French 
			//case XC_LANGUAGE_SPANISH Spanish 
			//case XC_LANGUAGE_ITALIAN Italian 
			//case XC_LANGUAGE_KOREAN Korean 
			//case XC_LANGUAGE_TCHINESE Chinese (Traditional) 
			//case XC_LANGUAGE_PORTUGUESE Portuguese 
			//case XC_LANGUAGE_SCHINESE Chinese (Simplified) 
		case XC_LANGUAGE_POLISH: localeSetting = "pol"; break;
		case XC_LANGUAGE_RUSSIAN: localeSetting = "rus"; break;
	}
#endif

	if (localeSetting == NULL)
	{
		localeSetting = "eng";
	}


	static char tmpImageName[64];
	crt_snprintf(tmpImageName, 64, "%s_i_loading_default", localeSetting);



	SetLoadingImage(tmpImageName);




	/*
	dword dwLocId = 2000003;
	const char * message = "Loading...";
	if (m_ShaderManager.IsNeedBuild())
	{
	dwLocId = 2000004;
	message = "Compiling shaders...";
	} 

	ShowStartupScreen(dwLocId, message);
	*/





	shaderTime = GetTechniqueGlobalVariable("time", _FL_);



	varColor = GetTechniqueGlobalVariable("postProcess_FillScreen_Color", _FL_);
	varTexture = GetTechniqueGlobalVariable("postProcess_FillScreen_Texture", _FL_);



	InitShadowLimiters();

#ifndef _XBOX
	globalHWND = (HWND)api->Storage().GetLong("system.hwnd");
#endif

	if( !m_pXLiveShower )
	{
#ifndef _XBOX
		m_pXLiveShower = NEW XLiveShowerPC( D3D(), &d3dpp );
#endif
	}


#ifdef SHOW_HISTOGRAM_DELETE_ME_BEFORE_RELEASE

	api->Trace("Debug histogram included to build!!!");

	histogram = CreateTempRenderTarget(TRS_SCREEN_FULL_3D, TRC_FIXED_RGBA_8, _FL_);
	histogram_source = GetTechniqueGlobalVariable("histogram_source", _FL_);

	histogram_min = GetTechniqueGlobalVariable("histogram_min", _FL_);
	histogram_max = GetTechniqueGlobalVariable("histogram_max", _FL_);

	if (!histogram)
	{
		api->Trace("Histogram: can't create histogram RT");
	}


	if (!histogram_source)
	{
		api->Trace("Histogram: can't find histogram_source");
	}


	if (!histogram_min)
	{
		api->Trace("Histogram: can't find histogram_min");
	}


	if (!histogram_max)
	{
		api->Trace("Histogram: can't find histogram_max");
	}


	_histogram_query = CreateOcclusionQuery(_FL_);
	if (!_histogram_query)
	{
		api->Trace("Histogram: can't create occlusion query!!");
	}


	histogram_totalPixelsInScreen = (float)m_Screen3DInfo.dwWidth * (float)m_Screen3DInfo.dwHeight;
	histogramm_bin = 0;

#endif



	// Вано: переменные и техники для гамма коррекции
	GetShaderId("GammaCorrectionTech", gammaShader);
	gammaValue = GetTechniqueGlobalVariable("GammaValue", _FL_);
	preGammaTexture = GetTechniqueGlobalVariable("preGammaTexture", _FL_);

	storageBrightness = api->Storage().GetItemFloat("Options.Brightness", _FL_);
	gammaValue->SetFloat(1.0f);

	DWORD dwEndInit = GetTickCount();
	dwInitTime = dwEndInit - dwStartInit;
	api->Trace("Render init time %d ms\n", dwInitTime);


#ifndef _XBOX
	//На PC просто загружаем legal videos, потом их запустим уже...
	LoadLegalVideos(true);
	LoadLegalVideos(false);
#endif


#ifndef _XBOX
	((DX9RenderDevice*)pDevice)->AdjustWindow(winPosX, winPosY, bExpand2FullScreen, bMaximize2FullScreen, d3dpp);
#endif

	return true;
}


void NGRender::BuildHintsList()
{
	loadingHints.DelAll();
	loadingMessage = "Loading...";
	ILocStrings* pLocStr = (ILocStrings*)api->GetService("LocStrings");
	if (pLocStr)
	{
		const char* loadingValue = pLocStr->GetString(LOADING_HINT_ID);

		if (loadingValue)
		{
			loadingMessage = loadingValue;
		}

		for (DWORD hintID = DYNAMIC_HINT_FIRST_ID; hintID <= DYNAMIC_HINT_LAST_ID; hintID++)
		{
			const char* hintValue = pLocStr->GetString(hintID);
			if (!hintValue)
			{
				break;
			}

			loadingHints.Add(hintValue);
		}
	}
}



CDX8Fonts* NGRender::getFontsManager()
{
	RestoreIfNeed();
	AssertCoreThread;

	return pFonts;
}

AdvancedFonts* NGRender::getAdvancedFontsManager()
{
	RestoreIfNeed();
	AssertCoreThread;

	return pAdvFonts;
}


//bool NGRender::IsDeviceLost()

//Нужно вызывать когда окошко получает фокус
void NGRender::AcquireFocus ()
{
	AssertCoreThread;

	loading_screen_sc.Enter();

#ifndef _XBOX
	switch (pDevice->D3D()->TestCooperativeLevel())
	{
		//
	case D3DERR_DEVICENOTRESET:
		OnResetDevice();
		break;

		//
	case D3DERR_DEVICELOST: 
		bCantRender = true;
		bInsideBeginScene = false;
		break;
	}
#endif

	loading_screen_sc.Leave();


}


void NGRender::StartFrame(float dltTime)
{
#ifdef RENDER_DEBUG_DRAW_SKIP
	curDrawIndex = 0;
#endif

	AssertCoreThread;

	if (loadingScreenThread != NULL)
	{
		return;
	}

	loading_screen_sc.Enter();


	/*
	#ifdef _XBOX
	D3D()->UnsetAll();
	#endif
	*/

	for (dword i=0; i < aLights.Size(); i++)
	{
		aLights[i]->debugSetValue(0);
	}


	bInsideFrame = true;


	loading_screen_sc.Leave();

#ifndef _XBOX
	AcquireFocus();
#endif



	bPostProcessAlreadyDone = false;
	/*      
	if (GetAsyncKeyState('1') < 0)
	{
	EnableLoadingScreen(true);
	Sleep(1000);
	EnableLoadingScreen(false);
	}
	*/




	redundantStatesFilter->ResetInfo();
	//redundantStatesFilter->SetDefaultStates();

	stats_saved = stats;
	stats.Reset();




	m_ShaderManager.StartFrame();

	/* 
	Анимированных текстур больше нет, все равно - а это кэш миссы аццкие

	for (dword i = 0; i < aTextures.Size(); i++)
	{
	IBaseTexture* tx = aTextures[i];
	DX8RESOURCETYPE type = tx->GetType();
	if (type == DX8TYPE_ANIMTEXTURE)
	{
	((IAnimTexture*)tx)->Update(dltTime);
	}
	}
	*/

#ifdef _XBOX
	D3D()->SetShaderGPRAllocation(0, 32, 96);
#endif

	BeginScene();

	RENDERVIEWPORT vp3D;
	vp3D.X      = 0;
	vp3D.Y      = 0;
	vp3D.Width  = GetScreenInfo3D().dwWidth;
	vp3D.Height = GetScreenInfo3D().dwHeight;
	vp3D.MinZ   = 0.0f;
	vp3D.MaxZ   = 1.0f;
	SetViewport(vp3D);

	//        SetViewport(m_CurrentViewPort);

	//dword dwColor = m_backColor.GetDword();
	//Clear(0, null, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, dwColor, 1.0, 0);


	if (shaderTime)
	{
		shaderTime->SetFloat(fTimeToShader * 0.04f);
	}
}


void NGRender::ScreenToD3D (float sX, float sY, float &d3dX, float &d3dY)
{
	AssertCoreThread;

	float fScrX = float(GetScreenInfo2D().dwWidth) / 2.0f;
	float fScrY = float(GetScreenInfo2D().dwHeight) / 2.0f;

	d3dX = (float)sX / fScrX - 1.0f;
	d3dY = -((float)sY / fScrY - 1.0f);
}


void NGRender::Draw2DLinePerfomanace (float pX, float pY, float tX, float tY, dword color)
{
	AssertCoreThread;



	static Line2DVertex vrx[2];

	Vector From = Vector (0.0f);
	Vector To = Vector (0.0f);

	ScreenToD3D (pX, pY, From.x, From.y);
	ScreenToD3D (tX, tY, To.x, To.y);


	//api->Trace("Draw2DLinePerfomanace %fx%f to %fx%f", pX,pY, tX, tY);


	vrx[0].p = From;
	vrx[1].p = To;

	for (int r = 0; r < 2; r++)
	{
		vrx[r].color = color;
	}

	for (r = 0; r < 2; r++)
	{
		vrx[r].p.z = 0.0f;
		vrx[r].p.y = Clampf(vrx[r].p.y, -0.9f, 0.9f);
	}

	//api->Trace("Draw2DLinePerfomanace D3D %fx%f to %fx%f", From.x, From.y, To.x, To.y);

	ShaderId id;
	GetShaderId("RenderLine", id);
	if (!DrawPrimitiveUP (id, PT_LINELIST, 1, &vrx, sizeof (Line2DVertex)))
	{
		api->Trace("Can't Draw2DLinePerfomanace");
	}

}


void  NGRender::GetPerfomanceInfo (IRender::PerfomanceInfo& info)
{
	info.dwBatchCount = stats_saved.dwBatchCount;
	info.dwPolyCount = stats_saved.dwPolyCount;
}


bool CompareLightsByUsing(ILight* const & a1, ILight* const & a2)
{
	if (a2->debugGetValue() < a1->debugGetValue())
	{
		return true;
	}

	return false;
}


void NGRender::CheckAvailMemory()
{

#if defined(_XBOX) && (defined (_DEBUG) || defined(PROFILE))


	LockRes(_FL_, resource_creation_sc, "CheckAvailMemory");

	MEMORYSTATUS memStat;
	GlobalMemoryStatus(&memStat);

	//less than 4Mb
	if (memStat.dwAvailPhys < (4 * 1024 * 1024))
	{
		static char memoryInfo[4096];
		crt_snprintf(memoryInfo, 4096, "Xbox360 Physical memory low : %f Mb\n", memStat.dwAvailPhys / 1024.0f / 1024.0f);
		OutputDebugString(memoryInfo);

		D3D()->Suspend();


		LPCWSTR g_pwstrButtons[2] =
		{
			L"OK",
			L"Dump memory log"
		};

		MESSAGEBOX_RESULT m_Result;

		DWORD dwRet;
		XOVERLAPPED m_Overlapped;
		ZeroMemory( &m_Overlapped, sizeof(XOVERLAPPED));



		dword dwTotalTexturesSize = 0;
		for (long i = 0; i< aTextures.Size(); i++)
		{
			IBaseTexture* tx = aTextures[i];
			dwTotalTexturesSize += tx->GetSize();
		}

		CoreCommand_GetMemStat animMemStat("anxdata.cpp");
		api->ExecuteCoreCommand(animMemStat);

		CoreCommand_GetMemStat codeMemState;
		api->ExecuteCoreCommand(codeMemState);
		codeMemState.totalAllocSize = codeMemState.totalAllocSize - animMemStat.totalAllocSize;


		DWORD systemMemSize = ((512*1024*1024) - memStat.dwAvailPhys - dwTotalTexturesSize - animMemStat.totalAllocSize - codeMemState.totalAllocSize);


		static wchar_t messageBoxText[4096];
		swprintf(messageBoxText, 4000, L"Free physical memory is low (less than 4Mb)\nUnknown (OS:32Mb): %3.2f Mb\nTextures : %3.2f Mb\nAnimations : %3.2f Mb\nCode : %3.2f Mb\n\n\nConsole now is rebooted", systemMemSize / 1024.0f / 1024.0f, dwTotalTexturesSize / 1024.0f / 1024.0f, animMemStat.totalAllocSize / 1024.0f / 1024.0f, codeMemState.totalAllocSize / 1024.0f / 1024.0f);



		dwRet = XShowMessageBoxUI(
			XUSER_INDEX_ANY,
			L"Captain Blood",                   // Message box title
			messageBoxText,
			ARRAYSIZE( g_pwstrButtons ),// Number of buttons
			g_pwstrButtons,             // Button captions
			0,                          // Button that gets focus
			XMB_ERRORICON,              // Icon to display
			&m_Result,                  // Button pressed result
			&m_Overlapped );

		assert( dwRet == ERROR_IO_PENDING );

		for (;;)
		{
			if( XHasOverlappedIoCompleted( &m_Overlapped ) )
			{
				DWORD dwResult = XGetOverlappedResult( &m_Overlapped, NULL, TRUE );
				if( dwResult == ERROR_SUCCESS )
				{
					api->Trace("Message box result = %d\n", m_Result.dwButtonPressed);

					if (m_Result.dwButtonPressed == 1)
					{
						DumpAllMemoryUsageToLog();
					}
				}

				break;
			}
		}

		D3D()->Resume();

#ifdef _DEBUG
		DmReboot(DMBOOT_COLD);
#else
		Assert(false);
#endif
	}

	UnlockRes(_FL_, resource_creation_sc, "CheckAvailMemory");


#endif
}


#ifdef _XBOX    

void  NGRender::FlushHiZStencil (bool bAsynchronous)
{
	//FlushHiZStencil 

	D3DFHZS_FLUSHTYPE type = D3DFHZS_SYNCHRONOUS;
	if (bAsynchronous)
	{
		type = D3DFHZS_ASYNCHRONOUS;
	}
	D3D()->FlushHiZStencil(type);
}

#ifndef STOP_DEBUG




void NGRender::PrintMemoryStatistics()
{
	//      PrintShadowed(30, 30, 0xFFFFFFFF, "Init total %d ms, Shaders %d ms, PCB %d ms", dwInitTime, dwInitTimeShaders, m_ShaderManager.dwPCBtime);



	//return;

	MEMORYSTATUS memStatus;
	GlobalMemoryStatus(&memStatus);


	CoreCommand_GetMemStat coreMemStat;
	coreMemStat.totalAllocSize = 0;
	api->ExecuteCoreCommand(coreMemStat);

	dword dwTotalTexturesSize = 0;
	for (long i = 0; i< aTextures.Size(); i++)
	{
		IBaseTexture* tx = aTextures[i];
		dwTotalTexturesSize += tx->GetSize();
	}

	dword dwTotalVBSize = 0;
	for (long i=0; i<aVBuffers; i++)
	{
		IVBuffer* obj = aVBuffers[i];
		dwTotalVBSize += obj->GetSize();
	}

	dword dwTotalIBSize = 0;
	for (long i=0; i<aIBuffers; i++)
	{
		IIBuffer* obj = aIBuffers[i];
		dwTotalIBSize += obj->GetSize();
	}

	dword dwTotalRTSize = 0;
	for (long i=0; i<aRenderTargets; i++)
	{
		IRenderTarget* obj = aRenderTargets[i];
		dwTotalRTSize += obj->GetSize();
	}

	dword dwTotalRTDSize = 0;
	for (long i=0; i<aRenderTargetsDepth; i++)
	{
		IRenderTargetDepth* obj = aRenderTargetsDepth[i];
		dwTotalRTDSize += obj->GetSize();
	}


	//CoreCommand_GetMemStat animMemStat("anxdata.cpp");
	//api->ExecuteCoreCommand(animMemStat);

	CoreCommand_GetMemManagerStat managerStats;
	api->ExecuteCoreCommand(managerStats);



	IAnimationService * animService = (IAnimationService *)api->GetService("AnimationService");


	DWORD dwAnimMemory = 0;


	if (animService)
	{
		dwAnimMemory = animService->GetUsageMemory();
	}

	coreMemStat.totalAllocSize = coreMemStat.totalAllocSize - dwAnimMemory;



	DWORD dwDebugMemory = managerStats.totalSizeUseForDebug;

	//dwTotalTexturesSize + 
	DWORD dwVertexIndexDataVram = dwTotalVBSize + dwTotalIBSize;
	DWORD dwArtistVram = dwTotalTexturesSize + dwVertexIndexDataVram;


	//DWORD dwCodeVram = dwTotalRTSize + dwTotalRTDSize;
	DWORD dwCoreControlledMem = coreMemStat.totalAllocSize;
	DWORD dwUnknownMem = (memStatus.dwTotalPhys-memStatus.dwAvailPhys) - (dwArtistVram+dwCoreControlledMem+dwAnimMemory+dwDebugMemory);


	float fTotalMb = memStatus.dwTotalPhys / 1024.0f / 1024.0f;
	float fFreeMb = memStatus.dwAvailPhys / 1024.0f / 1024.0f;

	float fArtVramMb = dwArtistVram / 1024.0f / 1024.0f;
	float fVertexIndexVramMb = dwVertexIndexDataVram / 1024.0f / 1024.0f;
	float fTexturesVramMb = dwTotalTexturesSize / 1024.0f / 1024.0f;

	//float fCodeVramMb = dwCodeVram / 1024.0f / 1024.0f;
	float fCodeRamMb = dwCoreControlledMem / 1024.0f / 1024.0f;
	float fAnimRamMb = dwAnimMemory / 1024.0f / 1024.0f;
	float fDebugMemoryMb = dwDebugMemory / 1024.0f / 1024.0f;


	float fUnknownMb = dwUnknownMem / 1024.0f / 1024.0f;


	/*
	Print(posX, posY, 0xFFFFFFFF, "Draw time %d, batch cost %.2f", stats.dwDrawTime, stats.dwDrawTime / (float)stats.dwBatchCount);
	posY+=16;
	*/




	float fFps = api->EngineFps();


	/*
	PrintShadowed(0, 0, 0xFFFFFFFF, "VRAM: %3.2f, Anim: %3.2f, Code: %3.2f, Debug: %3.2f, Un(OS:32Mb): %3.2f), free: %3.2f          FPS:%3.2f", fArtVramMb, fAnimRamMb, fCodeRamMb, fDebugMemoryMb, fUnknownMb, fFreeMb, fFps);
	*/

	PIXAddNamedCounter(fArtVramMb, "RAM(Mb):Txtr + Vrtx + Idx");
	PIXAddNamedCounter(fVertexIndexVramMb, "RAM(Mb):Vrtx + Idx");
	PIXAddNamedCounter(fTexturesVramMb, "RAM(Mb):Txtr");
	PIXAddNamedCounter(fAnimRamMb, "RAM(Mb):Anim");
	PIXAddNamedCounter(fCodeRamMb, "RAM(Mb):Code");
	PIXAddNamedCounter(fDebugMemoryMb, "RAM(Mb):Debug");
	PIXAddNamedCounter(fUnknownMb, "RAM(Mb):Und (OS:32,XEX:55)");
	PIXAddNamedCounter(fFreeMb, "RAM(Mb):Free");



	float fAllBatchesCostMicroSec = (1000000.0f / (50000000.0f / stats.dwDrawTime));
	float fAllUPBatchesCostMicroSec = (1000000.0f / (50000000.0f / stats.dwDrawTimeUP));
	float fFrameTimeMicroSec = ((1/fFps) * 1000000.0f);

	float percentDPUp = (fAllUPBatchesCostMicroSec / fFrameTimeMicroSec) * 100.0f;
	float percentDP = (fAllBatchesCostMicroSec / fFrameTimeMicroSec) * 100.0f;


	DWORD dwColor = 0xFFFFFFFF;
#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)
	dwColor = 0xFFFF0000;
#endif

	/*
	PrintShadowed(0, 0+16, dwColor, "Dp[%d]:%3.2f mks (%3.2f%%), avg %3.2f mks, DpUp[%d]:%3.2f mks (%3.2f%%), avg %3.2f mks, frame time : %3.2fmks", 
	stats.dwBatchCount, fAllBatchesCostMicroSec, percentDP, (fAllBatchesCostMicroSec / (float)stats.dwBatchCount), 
	stats.dwBatchCountUP, fAllUPBatchesCostMicroSec, percentDPUp, fAllUPBatchesCostMicroSec / (float)stats.dwBatchCountUP, 
	fFrameTimeMicroSec);





	PrintShadowed(0, 0+32+80, 0xFFFFFFFF, "DpUp %3.2fMb/frame, %3.2fMb/sec      top shader [%3.2fMb]", stats.dwDrawUpBytes / 1024.0f / 1024.0f, (stats.dwDrawUpBytes*fFps) / 1024.0f / 1024.0f, stats.dwMaxDpUpSize / 1024.0f / 1024.0f);

	PrintShadowed(0, 0+48+80, 0xFFFFFFFF, "Constants[%d] %3.2fMb/frame, %3.2fMb/sec", stats.dwConstCount, stats.dwConstBytes / 1024.0f / 1024.0f, (stats.dwConstBytes*fFps) / 1024.0f / 1024.0f);

	PrintShadowed(0, 0+64+80, 0xFFFFFFFF, "WriteCombined %3.2fMb/frame, %3.2fMb/sec", stats.dwWCBytesTraffic / 1024.0f / 1024.0f, (stats.dwWCBytesTraffic*fFps) / 1024.0f / 1024.0f);

	DWORD dwTraffic = (stats.dwWCBytesTraffic + stats.dwConstBytes + stats.dwDrawUpBytes);
	PrintShadowed(0, 0+80+85, 0xFFFFFFFF, "Total %3.2fMb/frame, %3.2fMb/sec", dwTraffic / 1024.0f / 1024.0f, (dwTraffic*fFps) / 1024.0f / 1024.0f);


	PrintShadowed(0, 64+80+85, 0xFFFFFFFF, "%d Poly/frame, %3.2fM Poly/sec", stats.dwPolyCount, ((float)stats.dwPolyCount * fFps)/1000000.0f);
	*/


}
#endif
#endif



void NGRender::DumpAllMemoryUsageToLog()
{
	api->ExecuteCoreCommand(CoreCommand_MemStat(cmemstat_bysize));

	array<MemInfo> info(_FL_);

	dword dwTotalTexSize = 0;
	for (long i=0; i<aTextures; i++)
	{
		IBaseTexture* tx = aTextures[i];
		const char* szName = tx->GetName();

		//api->Trace("Tex name: '%s'", szName);
		info.Add(MemInfo(MIT_TEXTURE, tx->GetSize(), tx->GetFileName(), tx->GetFileLine(), szName));
	}

	dword dwTotalVBSize = 0;
	for (long i=0; i<aVBuffers; i++)
	{
		IVBuffer* obj = aVBuffers[i];
		info.Add(MemInfo(MIT_VERTEX_BUFFER, obj->GetSize(), obj->GetFileName(), obj->GetFileLine()));
	}

	dword dwTotalIBSize = 0;
	for (long i=0; i<aIBuffers; i++)
	{
		IIBuffer* obj = aIBuffers[i];
		info.Add(MemInfo(MIT_INDEX_BUFFER, obj->GetSize(), obj->GetFileName(), obj->GetFileLine()));
	}

	dword dwTotalRTSize = 0;
	for (long i=0; i<aRenderTargets; i++)
	{
		IRenderTarget* obj = aRenderTargets[i];
		info.Add(MemInfo(MIT_RENDER_TARGET, obj->GetSize(), obj->GetFileName(), obj->GetFileLine()));
	}

	dword dwTotalRTDSize = 0;
	for (long i=0; i<aRenderTargetsDepth; i++)
	{
		IRenderTargetDepth* obj = aRenderTargetsDepth[i];
		info.Add(MemInfo(MIT_RENDER_TARGET_DEPTH, obj->GetSize(), obj->GetFileName(), obj->GetFileLine()));
	}

	info.QSort(MemoryDumpCompare);

	for (dword n = 0; n < info.Size(); n++)
	{
		const char* type = "Unknown";

		switch (info[n].t)
		{
		case MIT_TEXTURE:
			type = "Texture";
			break;
		case MIT_VERTEX_BUFFER:
			type = "VertexBuffer";
			break;
		case MIT_INDEX_BUFFER:
			type = "IndexBuffer";
			break;
		case MIT_RENDER_TARGET:
			type = "RenderTarget";
			break;
		case MIT_RENDER_TARGET_DEPTH:
			type = "RenderTargetDepth";
			break;
		}

		if (info[n].additional)
		{
			api->Trace("    size: %8u, type: %s, file: %s, line: %i, filename: %s", info[n].dwMemCount, type, info[n].srcFile, info[n].srcLine, info[n].additional);
		} else
		{
			api->Trace("    size: %8u, type: %s,  file: %s, line: %i", info[n].dwMemCount, type, info[n].srcFile, info[n].srcLine);
		}
	}

}

void _cdecl NGRender::DebugMemory(const ConsoleStack & params)
{
	IConsole* pConsole = (IConsole*)api->GetService("Console");

	if (pConsole)
	{
		pConsole->Trace(COL_ALL, "Start dump memory usage to 'system.log'");
	}

	DumpAllMemoryUsageToLog();

	if (pConsole)
	{
		pConsole->Trace(COL_ALL, "done.");
	}

}


void NGRender::EndFrame(float dltTime)
{
#ifdef RENDER_DEBUG_DRAW_SKIP
	if (GetAsyncKeyState('1') < 0)
	{
		curSkip = -1;
		curSkip100 = -100;
		Sleep(300);
	}

	if (GetAsyncKeyState('2') < 0)
	{
		curSkip = -1;
		Sleep(300);
	}

	if (GetAsyncKeyState('5') < 0)
	{
		curSkip100 += 100;
		Sleep(300);
	}

	if (GetAsyncKeyState('6') < 0)
	{
		curSkip += 1;
		Sleep(300);
	}

	if (GetAsyncKeyState('7') < 0)
	{
		curSkip -= 1;
		Sleep(300);
	}

	if (curSkip < 0)
		Print(10.0f, 30.0f, 0xFF00FF00, "Skip range = %d .. %d", curSkip100, curSkip100 + 99);
	else
		Print(10.0f, 30.0f, 0xFF00FF00, "Skip draw = %d", curSkip + curSkip100);

#endif

	if (loadingScreenThread != NULL)
	{
		return;
	}

	if (bInsideFrame == false)
	{
		return;
	}
	bInsideFrame = false;

	CheckAvailMemory();

	if (bConsoleRegistred == false)
	{
		IConsole* pConsole = (IConsole*)api->GetService("Console");

		if (pConsole)
		{
			pConsole->RegisterCommand("DebugMemory", "dump all memory usage to system.log", this, (CONSOLE_COMMAND)&NGRender::DebugMemory);

			bConsoleRegistred = true;
		}
	}




	fTimeToShader += dltTime;

	// обновим интерфейсы от Games for Windows - LIVE (если они есть)
	if( m_pXLiveShower )
		m_pXLiveShower->RenderFrame();

	loading_screen_sc.Enter();

	if (IsDeviceLost())
	{
		loading_screen_sc.Leave();
		return;
	}




	//Если в режиме загрузки...
	if (loadingScreenThread != NULL)
	{
		loading_screen_sc.Leave();
		return;
	}

	loading_screen_sc.Leave();

	AssertCoreThread;



#ifdef _XBOX
	MEMORYSTATUS memStatus;
	GlobalMemoryStatus(&memStatus);
	dword s = dword(memStatus.dwTotalPhys - memStatus.dwAvailPhys);
	api->SetPerformanceCounter("Memory : total phys MB", ((float)memStatus.dwTotalPhys) / 1024.0f / 1024.0f);
	api->SetPerformanceCounter("Memory : avail phys MB", ((float)memStatus.dwAvailPhys) / 1024.0f / 1024.0f);
	api->SetPerformanceCounter("Memory : used MB", ((float)s) / 1024.0f / 1024.0f);
#endif

	api->SetPerformanceCounter("DeltaTime", dltTime);
	//api->SetPerformanceCounter("DeltaTime2", FRAND(1.0f));

	FlushBufferedLines();


	D3D()->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

	RS_SPRITE spr[4];
	float fScrX = float(GetViewport().Width) / 2.0f;
	float fScrY = float(GetViewport().Height) / 2.0f;




	if (bDrawStats)
	{
		for (dword n = 0; n < perfGraphs.Size(); n++)
		{
			float val = api->GetPerformanceCounter(perfGraphs[n].dwID);

			if (perfGraphs[n].min > val) perfGraphs[n].min = val; 
			if (perfGraphs[n].max < val) perfGraphs[n].max = val; 


			if (perfGraphs[n].avgInited)
			{
				perfGraphs[n].avg += val;
				perfGraphs[n].avg *= 0.5f;
			} else
			{
				perfGraphs[n].avg = val;
			}


			perfGraphs[n].graphData.Add(val);

			if (perfGraphs[n].graphData.Size() > 300)
			{
				perfGraphs[n].graphData.DelIndex(0);
			}
		}


		spr[0].vPos = Vector(-1.0, -1.0f, 0.2f);        spr[1].vPos = Vector(1.0f, -1.0f, 0.2f);
		spr[2].vPos = Vector(1.0f, 1.0f, 0.2f); spr[3].vPos = Vector(-1.0, 1.0f, 0.2f);
		DrawSprites(NULL, spr, 1, "dbgInfoSprite");


		float scrollY = 0.0f;
		for (dword n = 0; n < perfGraphs.Size(); n++)
		{
			float val = api->GetPerformanceCounter(perfGraphs[n].dwID);

			Print(10.0f, scrollY, perfGraphs[n].dwColor, "[n:%f][s:%f][v:%f] [min: %f] [max: %f] [avg: %f] : %s", val, perfGraphs[n].scale, val * perfGraphs[n].scale, perfGraphs[n].min, perfGraphs[n].max, perfGraphs[n].avg, perfGraphs[n].name.c_str());
			Print(11.0f, scrollY, perfGraphs[n].dwColor, "[n:%f][s:%f][v:%f] [min: %f] [max: %f] [avg: %f] : %s", val, perfGraphs[n].scale, val * perfGraphs[n].scale, perfGraphs[n].min, perfGraphs[n].max, perfGraphs[n].avg, perfGraphs[n].name.c_str());
			scrollY+=16.0f;
		}


		float yPos = (float)(GetScreenInfo2D().dwHeight - 20);
		Print(3.0f, yPos, 0xFFFFFFFFL, "Perfomance graphs, active [%d]", perfGraphs.Size());
		Print(4.0f, yPos, 0xFFFFFFFFL, "Perfomance graphs, active [%d]", perfGraphs.Size());



		float ScreenPos = (float)GetScreenInfo2D().dwWidth;
		Draw2DLinePerfomanace(0, yPos+2, ScreenPos, yPos+2, 0xFFFFFFFFL);
		Draw2DLinePerfomanace(0, yPos+3, ScreenPos, yPos+3, 0xFFFFFFFFL);
		Draw2DLinePerfomanace(0, yPos+4, ScreenPos, yPos+4, 0xFFFFFFFFL);

		for (dword q = 0; q < perfGraphs.Size(); q++)
		{
			ScreenPos = (float)GetScreenInfo2D().dwWidth;

			if (perfGraphs[q].graphData.Size() >= 2)
			{
				float XStep = ScreenPos / 300.0f;

				for (long n = (perfGraphs[q].graphData.Size()-1); n > 0 ; n--)
				{

					float f1 = (perfGraphs[q].graphData[n] * perfGraphs[q].scale);
					float f2 = (perfGraphs[q].graphData[n-1] * perfGraphs[q].scale);

					float a1 = yPos - f1;
					float a2 = yPos - f2;

					if (a1 < 20.0f) a1 = 20.0f;
					if (a2 < 20.0f) a2 = 20.0f;


					Draw2DLinePerfomanace(ScreenPos+1, a1-1, (ScreenPos-XStep+1), a2-1, perfGraphs[q].dwColor);
					Draw2DLinePerfomanace(ScreenPos, a1, (ScreenPos-XStep), a2, perfGraphs[q].dwColor);

					ScreenPos -= XStep;

				}
			}
		}


	}




	if (!IsRenderDisabled())
	{


#ifdef SHOW_HISTOGRAM_DELETE_ME_BEFORE_RELEASE
		if (api->DebugKeyState(VK_CONTROL, 'H'))
		{
			bShowHistogram = !bShowHistogram;
			histogram_update_chunk = 256;
			Sleep(500);
		}

		if (api->DebugKeyState(VK_MENU, 'H'))
		{
			bShowHistogram = !bShowHistogram;
			histogram_update_chunk = 16;
			Sleep(500);
		}

#endif


		if (api->DebugKeyState(VK_CONTROL, VK_SHIFT, 'G'))
		{
			bDrawStats = !bDrawStats;
			initPerfGraphs();
			Sleep(500);
		}


		if (api->DebugKeyState(VK_CONTROL, VK_SHIFT, 'D'))
		{
			DumpMemStat();
			Sleep(500);
		}


		if(api->DebugKeyState(ICore::xb_dpad_down))
		{
			bShowStatistic = !bShowStatistic;
			Sleep(500);
		}


		if (api->DebugKeyState(VK_CONTROL, VK_SHIFT, 'T'))
		{
			bShowStatistic = !bShowStatistic;
			Sleep(500);
		}

		float fpsCount = api->EngineFps();
		dword dwColor = 0xFF00FF00;
		if (fpsCount < 24.0f)
		{
			dwColor = 0xFFFFFF00;
		}

		if (fpsCount < 15.0f)
		{
			dwColor = 0xFFFF0000;

#ifdef _XBOX
			//CoreCommand_GetMemStat memStat;
			//api->ExecuteCoreCommand(memStat);

			//api->Trace("TCR not passed, FPS %f is too low (%d allocs %d deletes per this frame)", fpsCount, memStat.allocsPerFrame, memStat.deletesPerFrame);
#endif
		}


#ifdef _XBOX
#ifndef STOP_DEBUG
		PrintMemoryStatistics();
#endif
#endif



		if (api->DebugKeyState(VK_MULTIPLY))
		{
			CoreCommand_GetMemStat memStat;
			api->ExecuteCoreCommand(memStat);
			static char dataStr[256];
			if(!api->DebugKeyState(VK_SHIFT))
			{
				crt_snprintf(dataStr, sizeof(dataStr), " %.2f", api->EngineFps());
			}else{
				crt_snprintf(dataStr, sizeof(dataStr), " %.2f, mem -> alcs: %u, dels: %u, total: %u, blocks: %u", api->EngineFps(), memStat.allocsPerFrame, memStat.deletesPerFrame, memStat.totalAllocSize, memStat.numBlocks);
			}
			float fLen = pSystemFont->GetLength(dataStr);
			float x1 = -1.0f;       float x2 = fLen / fScrX - 1.0f;
			float y1 = 1.0f;        float y2 = 1.0f - 20.0f / fScrY;
			spr[0].vPos = Vector(x1, y1, 0.2f);     spr[1].vPos = Vector(x2, y1, 0.2f);
			spr[2].vPos = Vector(x2, y2, 0.2f);     spr[3].vPos = Vector(x1, y2, 0.2f);
			DrawSprites(NULL, spr, 1, "dbgInfoSprite");

			Print(0, 0, dwColor, dataStr);

			array<ILight*> activeLights(_FL_); 

			float dwYoffset = 64.0f;
			for (dword i=0; i < aLights.Size(); i++)
			{
				dword dwVal = aLights[i]->debugGetValue();
				if (dwVal <= 0 ) continue;

				activeLights.Add(aLights[i]);
			}

			activeLights.QSort(CompareLightsByUsing);

			for (dword i=0; i < activeLights.Size(); i++)
			{
				dword dwVal = activeLights[i]->debugGetValue();
				const char* szName = activeLights[i]->debugGetName();
				Print(0, dwYoffset, dwColor, "%d. %s - %d", i, szName, dwVal);
				dwYoffset += 16.0f;
			}


		} 

		/*
		#ifdef _XBOX
		PIXAddNamedCounter((float)stats.dwBatchCount, "STORM3 Batch count");

		PIXAddNamedCounter((float)stats.dwDrawTime, "STORM3 Time in DP");

		#endif
		*/
#ifdef SHOW_HISTOGRAM_DELETE_ME_BEFORE_RELEASE
		if (bShowHistogram)
		{
			float offsetX = 50.0f;
			float offsetY = 50.0f;

			float sum = 0.0f;
			float max = 0.0f;

			long startPoint = -1;
			long endPoint = -1;

			long centerPoint = -1;
			float fSumm = 0.0f;

			float Dark = 0.0f;
			float Medium = 0.0f;
			float Bright = 0.0f;

			for (long pix = 0; pix < 255; pix++)
			{
				float data = histogram_data[pix];
				float data2 = histogram_data[255-pix];


				if (data >= 0.001f && startPoint < 0)
				{
					startPoint = pix;
				}

				if (data2 >= 0.001f && endPoint < 0)
				{
					endPoint = (255-pix);
				}


				max = coremax(data, max);
				fSumm += data;

				if (fSumm > 0.5f && centerPoint < 0)
				{
					centerPoint = pix;
				}

				if (pix <= 85)
				{
					Dark += data;
				} else
				{
					if (pix > 170)
					{
						Bright += data;
					} else
					{
						Medium += data;
					}
				}
			}




			if (centerPoint < 0)
			{
				centerPoint = 0;
			}

			if (startPoint < 0)
			{
				startPoint = 0;
			}

			if (endPoint < 0)
			{
				endPoint = 0;
			}

			long centerValues = startPoint + ((endPoint - startPoint) >> 1);



			float scale = 1.0f;

			if (max > 0.000001f)
			{
				scale = 1.0f / max;
			}


			spr[0].vPos = Vector(-1.0, -1.0f, 0.2f);        spr[1].vPos = Vector(1.0f, -1.0f, 0.2f);
			spr[2].vPos = Vector(1.0f, 1.0f, 0.2f); spr[3].vPos = Vector(-1.0, 1.0f, 0.2f);


			ScreenToD3D(-1.0f+offsetX, -1.0f+offsetY, spr[0].vPos.x, spr[0].vPos.y);
			ScreenToD3D(256.0f+offsetX, -1.0f+offsetY, spr[1].vPos.x, spr[1].vPos.y);

			ScreenToD3D(256.0f+offsetX, 513.0f+offsetY, spr[2].vPos.x, spr[2].vPos.y);
			ScreenToD3D(-1.0f+offsetX, 513.0f+offsetY, spr[3].vPos.x, spr[3].vPos.y);


			DrawSprites(NULL, spr, 1, "dbgInfoSprite");

			Draw2DLinePerfomanace(85.0f+offsetX, -1.0f+offsetY, 85.0f+offsetX, 513.0f+offsetY, 0xFF00FFFFL);
			Draw2DLinePerfomanace(170.0f+offsetX, -1.0f+offsetY, 170.0f+offsetX, 513.0f+offsetY, 0xFF00FFFFL);


			for (long pix = 0; pix < 255; pix++)
			{
				Draw2DLinePerfomanace((float)pix+offsetX, 512.0f+offsetY, (float)pix+offsetX, (512.0f - ((histogram_data[pix]*scale)*512.0f))+offsetY , 0xFFA0A0A0L);

				sum += histogram_data[pix];
			}

			float p = max;
			float step = p / 10.0f;
			for (long piy = 0; piy <= 10; piy++)
			{
				Print(offsetX-32.0f, (offsetY+512.0f-((float)(10-piy)*51.2f))-8.0f, 0xFFFFFFFF, "%3.1f", p*100.0f);
				p-=step;
				if (p < 0.00001f)
				{
					p = 0.0f;
				}
			}


			Draw2DLinePerfomanace(-1.0f+offsetX, -1.0f+offsetY, 256.0f+offsetX, -1.0f+offsetY, 0xFF00FFFFL);
			Draw2DLinePerfomanace(-1.0f+offsetX, 513.0f+offsetY, 256.0f+offsetX, 513.0f+offsetY, 0xFF00FFFFL);
			Draw2DLinePerfomanace(-1.0f+offsetX, -1.0f+offsetY, -1.0f+offsetX, 513.0f+offsetY, 0xFF00FFFFL);
			Draw2DLinePerfomanace(256.0f+offsetX, -1.0f+offsetY, 256.0f+offsetX, 513.0f+offsetY, 0xFF00FFFFL);


			Draw2DLinePerfomanace(centerPoint+offsetX, 502.0f+offsetY, centerPoint+offsetX, 522.0f+offsetY, 0xFF0000FFL);
			Draw2DLinePerfomanace(centerValues+offsetX, 502.0f+offsetY, centerValues+offsetX, 522.0f+offsetY, 0xFFFF0000L);


			Draw2DLinePerfomanace(-1.0f+offsetX+39.0f, -15.0f+offsetY, -1.0f+offsetX+39.0f, 513.0f+offsetY+15.0f, 0xFFFF0000L);
			Draw2DLinePerfomanace(256.0f+offsetX-39.0f, -15.0f+offsetY, 256.0f+offsetX-39.0f, 513.0f+offsetY+15.0f, 0xFFFF0000L);



			Print(offsetX, offsetY, 0xFFFFFFFF, "Realtime image histogram (%d)", histogram_update_chunk);
			Print(offsetX, offsetY+16, 0xFFFFFFFF, "d:%3.2f, m:%3.2f, l:%3.2f", Dark*100.0f, Medium*100.0f, Bright*100.0f);
			Print(offsetX, offsetY+32, 0xFFFFFFFF, "'D' - show dark areas, 'L' - show light areas");

		}
#endif


		if (bShowStatistic)
		{
#ifndef _XBOX

			spr[0].vPos = Vector(-1.0, -1.0f, 0.2f);        spr[1].vPos = Vector(1.0f, -1.0f, 0.2f);
			spr[2].vPos = Vector(1.0f, 1.0f, 0.2f); spr[3].vPos = Vector(-1.0, 1.0f, 0.2f);
			DrawSprites(NULL, spr, 1, "dbgInfoSprite");
#endif

			float posY = 32.0f;
			float posX = 32.0f;

			Print(posX, posY, dwColor, "FPS %.2f, Cam: %f, %f, %f", api->EngineFps(), vCameraPosition_WorldSpace.x, vCameraPosition_WorldSpace.y, vCameraPosition_WorldSpace.z);
			posY+=16;

			//7950GX2, 68M Poly sec unoptimized, 120M Poly sec cache optimized  (28.3 Bloods rendered :( )
			Print(posX, posY, 0xFFFFFFFF, "Batch count: %d, Poly count: %d (%d lights poly), %.2fM Poly sec", stats.dwBatchCount, stats.dwPolyCount, stats.dwLightsPolyCount, (stats.dwPolyCount * api->EngineFps()) / 1000000.0f);
			posY+=16;
			Print(posX, posY, 0xFFFFFFFF, "Draw time %d, batch cost %.2f", stats.dwDrawTime, stats.dwDrawTime / (float)stats.dwBatchCount);
			posY+=16;





			StateFilter::Info info;
			redundantStatesFilter->GetInfo(info);

			dword totalChanges = info.renderstate_changes_count +
				info.samplerstate_changes_count +
				info.texture_changes_count +
				info.vertexshader_changes_count +
				info.pixelshader_changes_count +
				info.setvsconstF_changes_count +
				info.setvsconstI_changes_count +
				info.setvsconstB_changes_count +
				info.setpsconstF_changes_count +
				info.setpsconstI_changes_count +
				info.setpsconstB_changes_count;


			Print(posX, posY, 0xFFFFFFFF, "Try %d, Saved: %d", info.totalRequestToChangeStates, info.changeStates_saved);
			posY+=16;
			Print(posX, posY, 0xFFFFFFFF, "rs %d, ss: %d", info.renderstate_changes_count, info.samplerstate_changes_count);
			posY+=16;
			Print(posX, posY, 0xFFFFFFFF, "vs %d, ps: %d", info.vertexshader_changes_count, info.pixelshader_changes_count);
			posY+=16;
			Print(posX, posY, 0xFFFFFFFF, "vs F:%d (%d), I:%d, B:%d", info.setvsconstF_changes_count, info.setvsconstF_saved_count, info.setvsconstI_changes_count, info.setvsconstB_changes_count);
			posY+=16;
			Print(posX, posY, 0xFFFFFFFF, "ps F:%d (%d), I:%d, B:%d", info.setpsconstF_changes_count, info.setpsconstF_saved_count, info.setpsconstI_changes_count, info.setpsconstB_changes_count);
			posY+=16;
			Print(posX, posY, 0xFFFFFFFF, "vs const time(%d): %d, ps const time(%d): %d", info.vs_const_count, info.vs_const_time, info.ps_const_count, info.ps_const_time);
			posY+=16;



			dword dwTotalTexSize = 0;
			for (long i=0; i<aTextures; i++)
			{
				IBaseTexture* tx = aTextures[i];
				dwTotalTexSize += tx->GetSize();
			}

			dword dwTotalVBSize = 0;
			for (long i=0; i<aVBuffers; i++)
			{
				IVBuffer* obj = aVBuffers[i];
				dwTotalVBSize += obj->GetSize();
			}

			dword dwTotalIBSize = 0;
			for (long i=0; i<aIBuffers; i++)
			{
				IIBuffer* obj = aIBuffers[i];
				dwTotalIBSize += obj->GetSize();
			}

			dword dwTotalRTSize = 0;
			for (long i=0; i<aRenderTargets; i++)
			{
				IRenderTarget* obj = aRenderTargets[i];
				dwTotalRTSize += obj->GetSize();
			}

			dword dwTotalRTDSize = 0;
			for (long i=0; i<aRenderTargetsDepth; i++)
			{
				IRenderTargetDepth* obj = aRenderTargetsDepth[i];
				dwTotalRTDSize += obj->GetSize();
			}

			CoreCommand_GetMemStat memStat;
			api->ExecuteCoreCommand(memStat);



			Print(posX, posY, 0xFFFFFFFF, "Memory: system %3.4f Mb size", memStat.totalAllocSize/1024.0f/1024.0f);
			posY+=16;

			Print(posX, posY, 0xFFFFFFFF, "        %d textures, %3.4f Mb size", aTextures.Size(), dwTotalTexSize/1024.0f/1024.0f);
			posY+=16;

			Print(posX, posY, 0xFFFFFFFF, "        %d vertex_buf, %3.4f Mb size", aVBuffers.Size(), dwTotalVBSize/1024.0f/1024.0f);
			posY+=16;

			Print(posX, posY, 0xFFFFFFFF, "        %d index_buf, %3.4f Mb size", aIBuffers.Size(), dwTotalIBSize/1024.0f/1024.0f);
			posY+=16;

			Print(posX, posY, 0xFFFFFFFF, "        %d (%d shared) rendertargets_color, %3.4f Mb size", aRenderTargets.Size(), tempRenderTargetsC.Size(), dwTotalRTSize/1024.0f/1024.0f);
			posY+=16;

			Print(posX, posY, 0xFFFFFFFF, "        %d (%d shared) rendertargets_depth, %3.4f Mb size", aRenderTargetsDepth.Size(), tempRenderTargetsD.Size(), dwTotalRTDSize/1024.0f/1024.0f);
			posY+=16;

			dword dwBackSize = GetScreenInfo2D().dwWidth*GetScreenInfo2D().dwHeight*9;
			Print(posX, posY, 0xFFFFFFFF, "        BackBuffers, %3.4f Mb size", dwBackSize/1024.0f/1024.0f);
			posY+=16;


			dword dwTotalMem = 0;

			dwTotalMem = dwBackSize + dwTotalTexSize + dwTotalVBSize + dwTotalIBSize + dwTotalRTSize + dwTotalRTDSize + memStat.totalAllocSize;

			dword dwTotalMemVideo = 0;
			dwTotalMemVideo = dwBackSize + dwTotalTexSize + dwTotalVBSize + dwTotalIBSize + dwTotalRTSize + dwTotalRTDSize;

			Print(posX, posY, 0xFFFFFFFF, "        Total: %3.4f Mb size, Video: %3.4f Mb size", dwTotalMem/1024.0f/1024.0f, dwTotalMemVideo/1024.0f/1024.0f);
			posY+=16;



		}

	}

	//Нам depth не нужно ресолвить, только color все уже сделали...
	EndScene(NULL, false, true);

	// Вано: Подкручиваем гамму
	GammaCorrection();


#ifndef _XBOX

	d3d9Sync.Sync();

#endif

	Assert (stRenderTarget.Size() == 0);

	pDevice->Present();

	bDeviceReseted = false;
}

void NGRender::GammaCorrection()
{
	const float small_delta = 0.025f;

	float gamma = storageBrightness->Get(0.5f);
	float delta = fabs(gamma - 0.5f);
	if (delta <= small_delta)
		return; // выходим если нету гамма коррекции
	else
	{
		if (gamma < 0.5f)
			gamma = 1.0f + (0.5f - small_delta - gamma) * 6.0f;
		else
			gamma = 1.0f - (gamma - (0.5f + small_delta)) * 1.5f;
	}

	IRenderTarget * curRT = GetRenderTarget(_FL_);
	IRenderTargetDepth * curRTDepth = GetDepthStencil(_FL_);
	const RENDERVIEWPORT & fsvp = GetFullScreenViewPort_2D();

	gammaValue->SetFloat(gamma);
	preGammaTexture->SetTexture(curRT->AsTexture());

	// используем curRT потому что в нем есть и render_surface и texture
	SetRenderTarget(RTO_DONTOCH_CONTEXT, curRT, curRTDepth);
	SetViewport(fsvp);

	BeginScene();

	DrawFullScreenQuad(float(fsvp.Width), float(fsvp.Height), gammaShader);

	EndScene(null, false, true);
}


bool NGRender::Release(IResource* pResource)
{
	RestoreIfNeed();

	AssertCoreThread;

	if (!pResource)
	{
		return false;
	}

	if (!pResource->Release()) return false;
	return true;
}

const RENDERSCREEN& NGRender::GetScreenInfo2D() const
{
	return m_Screen2DInfo;
}

const RENDERSCREEN& NGRender::GetScreenInfo3D() const
{
	return m_Screen3DInfo;
}




// Vertex/Index buffers section
IVBuffer* NGRender::CreateVertexBuffer(dword Length, dword Stride, const char * pFileName, long iLine, dword Usage, RENDERPOOL Pool)
{
	RestoreIfNeed();

	LockRes(_FL_, resource_creation_sc, "CreateVertexBuffer");

	CDX8VBuffer * pV = NEW CDX8VBuffer();
	if (pV) 
	{
		pV->SetFileLine(pFileName, iLine);
		pV->Create(Length, Usage, Pool, Stride);
		aVBuffers.Add(pV);
	}

	UnlockRes(_FL_, resource_creation_sc, "CreateVertexBuffer");

	return pV;
}

IIBuffer* NGRender::CreateIndexBuffer(dword Length, const char * pFileName, long iLine, dword Usage, RENDERFORMAT Format, RENDERPOOL Pool)
{
	RestoreIfNeed();

	LockRes(_FL_, resource_creation_sc, "CreateIndexBuffer");

	CDX8IBuffer * pI = NEW CDX8IBuffer();
	if (pI)
	{
		pI->SetFileLine(pFileName, iLine);
		pI->Create(Length, Usage, Format, Pool);
		aIBuffers.Add(pI);
	}

	UnlockRes(_FL_, resource_creation_sc, "CreateIndexBuffer");
	return pI;
}



IBaseTexture* _cdecl NGRender::CreateTextureFullQuality(const char * pFileName, long iLine, const char * pFormat, ...)
{
	static char cFileName[_MAX_PATH * 2];

	va_list args;
	va_start(args, pFormat);
	crt_vsnprintf(cFileName, sizeof(cFileName), pFormat, args);
	va_end(args);

	m_TXXLoader->IgnoreDegradation(true);

	IBaseTexture* t = CreateTexture(pFileName, iLine, "%s", cFileName);

	m_TXXLoader->IgnoreDegradation(false);

	if (t == NULL)
	{
		t = getWhiteTexture();
		t->AddRef();
	}

	return t;

}

IBaseTexture* _cdecl NGRender::CreateTexture(const char * pFileName, long iLine, const char * pFormat, ...)
{
	RestoreIfNeed();

	LockRes(_FL_, resource_creation_sc, "CreateTexture (from file)");

	static char cFileName[_MAX_PATH * 2];
	va_list args;
	va_start(args, pFormat);
	crt_vsnprintf(cFileName, sizeof(cFileName), pFormat, args);
	va_end(args);

	if (cFileName == NULL)
	{
		IBaseTexture * t = getWhiteTexture();
		t->AddRef();

		UnlockRes(_FL_, resource_creation_sc, "CreateTexture (from file)");
		return t;
	}

	if (cFileName[0] == 0)
	{
		IBaseTexture * t = getWhiteTexture();
		t->AddRef();

		UnlockRes(_FL_, resource_creation_sc, "CreateTexture (from file)");
		return t;
	}


	dword i;
	IBaseTexture* pTexture = null;

	strTexName = m_TextureDirectory;
	strTexName += cFileName;
	strTexName.Lower();

	strTitle.GetFileTitle(strTexName);
	strPath.GetFilePath(strTexName);
	strFullPath = strPath;
	strFullPath += strTitle;
	strFullPath.CheckPath();
	strTexName = strFullPath;
	dword NeedFileNameHash = string::HashNoCase(strTitle.c_str());

	for (i = 0; i < aTextures.Size(); i++)
	{
		IBaseTexture* tex = aTextures[i];
		const char* name = tex->GetName();

		if (name == NULL) continue;

		dword dwSearchHash = tex->GetNameHash();

		if (dwSearchHash == NeedFileNameHash)
		{
			if (strTitle == name)
			{
				tex->AddRef();
				UnlockRes(_FL_, resource_creation_sc, "CreateTexture (from file)");
				return tex;
			}
		}
	}

	IBaseTexture* base = m_TXXLoader->Load_TXX(strTexName.c_str(), pFileName, iLine);

	if (base)
	{
		base->SetName(strTitle.c_str());
	}

	if (base == NULL)
	{
		base = getWhiteTexture();
		base->AddRef();
	}


	UnlockRes(_FL_, resource_creation_sc, "CreateTexture (from file)");
	return base;
}

IBaseTexture* _cdecl NGRender::CreateTextureUseD3DX(const char * pFileName, long iLine, const char * pFormat, ...)
{
	RestoreIfNeed();

	LockRes(_FL_, resource_creation_sc, "CreateTextureD3DX");

	dword dwStart = GetTickCount();

	static char cFileName[_MAX_PATH * 2];
	dword i;

	va_list args;
	va_start(args, pFormat);
	crt_vsnprintf(cFileName, sizeof(cFileName), pFormat, args);
	va_end(args);

	//string sTexName = cFileName;//(m_TextureDirectory + cFileName).Lower();
	//sTexName.AddExtention(".tga");
	//sTexName.CheckPath();

	for (i = 0; i < aTextures.Size(); i++)
	{
		IBaseTexture* tex =aTextures[i];
		if (string::IsEqual (cFileName, tex->GetName()))
		{
			//if (tex->IsRenderTarget()) { break; }
			tex->AddRef();

			dword dwEnd = GetTickCount();
			//api->Trace("TGA (%s) referenced by %f sec.", sTexName.c_str(), (dwEnd - dwStart) / 1000.0f);

			return tex;
		}
	}

	CDX8Texture* tex = NEW CDX8Texture();
	if (!tex->CreateUseD3DX(cFileName))
	{
		tex->Release();
		UnlockRes(_FL_, resource_creation_sc, "CreateTextureD3DX");
		return NULL;
	}

	tex->SetName(cFileName);
	tex->SetFileLine(pFileName, iLine);
	
	if (tex)
	{
		aTextures.Add(tex);
	}

	dword dwEnd = GetTickCount();
	//api->Trace("TGA (%s) loaded by %f sec.", sTexName.c_str(), (dwEnd - dwStart) / 1000.0f);

	UnlockRes(_FL_, resource_creation_sc, "CreateTextureD3DX");

	return tex;
}


ITexture* NGRender::AddTexture(const char * pFileName, long iLine)
{
	//LockRes(_FL_, resource_creation_sc, "AddTexture");

	CDX8Texture * pTex = NEW CDX8Texture();
	if (pTex) 
	{
		pTex->SetFileLine(pFileName, iLine);
		aTextures.Add(pTex);
	}

	//UnlockRes(_FL_, resource_creation_sc, "AddTexture");

	return pTex;
}

ITexture* NGRender::CreateTexture(dword Width, dword Height, dword Levels, dword Usage, RENDERFORMAT Format, const char * pFileName, long iLine, RENDERPOOL Pool)
{
	if(!Width || !Height)
	{
		return null;
	}


	RestoreIfNeed();

	LockRes(_FL_, resource_creation_sc, "CreateTexture");

	CDX8Texture * pTex = NEW CDX8Texture();
	if (pTex) 
	{
		pTex->SetFileLine(pFileName, iLine);
		if (!pTex->CreateLinear(Width , Height, Levels, Usage, Format, Pool))
		{
			SetErrorSignal();
			api->Trace("DX8Error: Can't create 2D texture, cpp: %s, %d", pFileName, iLine);
			DELETE(pTex);

			UnlockRes(_FL_, resource_creation_sc, "CreateTexture");
			return null;
		}

#ifdef _XBOX
		PIXSetTextureName((IDirect3DBaseTexture9*)pTex->GetBaseTexture(), pFileName);
#endif
		aTextures.Add(pTex);
	}

	UnlockRes(_FL_, resource_creation_sc, "CreateTexture");

	return pTex;
}


void NGRender::NotifyDeleteRT(IRenderTarget * rt)
{
	RestoreIfNeed();

	AssertCoreThread;


	aRenderTargets.Del(rt);

	for (dword i = 0; i < tempRenderTargetsC.Size(); i++)
	{
		if (tempRenderTargetsC[i].rt == rt)
		{
			tempRenderTargetsC.ExtractNoShift(i);
			break;
		}
	}
}

void NGRender::NotifyDeleteRTD(IRenderTargetDepth * rt)
{
	RestoreIfNeed();

	AssertCoreThread;


	aRenderTargetsDepth.Del(rt);

	for (dword i = 0; i < tempRenderTargetsD.Size(); i++)
	{
		if (tempRenderTargetsD[i].rt == rt)
		{
			tempRenderTargetsD.ExtractNoShift(i);
			break;
		}
	}

}


void NGRender::ConvertTRS2Size (TempRenderTargetSize size, dword & width, dword & height)
{
	width = GetScreenInfo3D().dwWidth;
	height = GetScreenInfo3D().dwHeight;

	switch (size)
	{
	case TRS_SCREEN_FULL_3D:
		width = GetScreenInfo3D().dwWidth;
		height = GetScreenInfo3D().dwHeight;
		break;
	case TRS_SCREEN_HALF_3D:
		width = GetScreenInfo3D().dwWidth >> 1;
		height = GetScreenInfo3D().dwHeight >> 1;
		break;
	case TRS_SCREEN_QUARTER_3D:
		width = GetScreenInfo3D().dwWidth >> 2;
		height = GetScreenInfo3D().dwHeight >> 2;
		break;
	case TRS_SCREEN_FULL_2D:
		width = GetScreenInfo2D().dwWidth;
		height = GetScreenInfo2D().dwHeight;
		break;
	case TRS_SCREEN_HALF_2D:
		width = GetScreenInfo2D().dwWidth >> 1;
		height = GetScreenInfo2D().dwHeight >> 1;
		break;
	case TRS_SCREEN_QUARTER_2D:
		width = GetScreenInfo2D().dwWidth >> 2;
		height = GetScreenInfo2D().dwHeight >> 2;
		break;
	case TRS_64:
		width = height = 64;
		break;
	case TRS_128:
		width = height = 128;
		break;
	case TRS_512:
		width = height = 512;
		break;
	case TRS_1024:
		width = height = 1024;
		break;
	}
}

IRenderTarget* NGRender::CreateTempRenderTarget(TempRenderTargetSize size, TempRenderTargetColor format, const char * pFileName, long iLine, long usageIndex)
{
	RestoreIfNeed();

	LockRes(_FL_, resource_creation_sc, "CreateTempRenderTarget");

	api->Trace("CreateTempRenderTarget from (%s, %d)", pFileName, iLine);

	for (dword n = 0; n < tempRenderTargetsC.Size(); n++)
	{
		if (tempRenderTargetsC[n].size == size && 
			tempRenderTargetsC[n].color == format && 
			tempRenderTargetsC[n].index == usageIndex)
		{
			tempRenderTargetsC[n].rt->AddRef();
			UnlockRes(_FL_, resource_creation_sc, "CreateTempRenderTarget");
			return tempRenderTargetsC[n].rt;
		}
	}


	dword rt_width;
	dword rt_height;
	ConvertTRS2Size(size, rt_width, rt_height);

	RENDERFORMAT Format = FMT_A8R8G8B8;
	switch (format)
	{
	case TRC_FIXED_RGBA_8:
		Format = FMT_A8R8G8B8;
		break;
	case TRC_FIXED_RGBA_16:
		//Format = FMT_A16B16G16R16;
		//Format = FMT_A2R10G10B10;
		Format = FMT_A8R8G8B8;
		break;
	case TRC_FLOAT_R:
		Format = FMT_R32F;
		break;
	}
	UnlockRes(_FL_, resource_creation_sc, "CreateTempRenderTarget");

	IRenderTarget* rt = CreateRenderTarget(rt_width, rt_height, pFileName, iLine, Format);

	LockRes(_FL_, resource_creation_sc, "CreateTempRenderTarget");

	TempRTC t;
	t.size = size;
	t.color = format;
	t.index = usageIndex;
	t.rt = rt;
	tempRenderTargetsC.Add(t);

	UnlockRes(_FL_, resource_creation_sc, "CreateTempRenderTarget");

	return rt;
}

IRenderTargetDepth* NGRender::CreateTempDepthStencil(TempRenderTargetSize size, const char * pFileName, long iLine, long usageIndex)
{
	RestoreIfNeed();

	LockRes(_FL_, resource_creation_sc, "CreateTempDepthStencil");

	api->Trace("CreateTempDepthStencil from (%s, %d)", pFileName, iLine);


	for (dword n = 0; n < tempRenderTargetsD.Size(); n++)
	{
		if (tempRenderTargetsD[n].size == size && 
			tempRenderTargetsD[n].index == usageIndex)
		{
			tempRenderTargetsD[n].rt->AddRef();
			UnlockRes(_FL_, resource_creation_sc, "CreateTempDepthStencil");
			return tempRenderTargetsD[n].rt;
		}
	}

	dword rt_width;
	dword rt_height;
	ConvertTRS2Size(size, rt_width, rt_height);

	UnlockRes(_FL_, resource_creation_sc, "CreateTempDepthStencil");

	IRenderTargetDepth* rt = CreateDepthStencil(rt_width, rt_height, pFileName, iLine, FMT_D24S8);

	LockRes(_FL_, resource_creation_sc, "CreateTempDepthStencil");

	TempRTD t;
	t.size = size;
	t.index = usageIndex;
	t.rt = rt;
	tempRenderTargetsD.Add(t);


	UnlockRes(_FL_, resource_creation_sc, "CreateTempDepthStencil");

	return rt;
}



IRenderTarget* NGRender::CreateRenderTarget(dword Width, dword Height, const char * pFileName, long iLine, RENDERFORMAT Format, RENDERMULTISAMPLE_TYPE MultiSample, long MipLevelsCount)
{
	RestoreIfNeed();

	LockRes(_FL_, resource_creation_sc, "CreateRenderTarget");

	api->Trace(" - CreateRenderTarget: %dx%d from (%s, %d) Format %d\n", Width, Height, pFileName, iLine, Format);

	RenderTarget* pRT = NEW RenderTarget();
	if (pRT)
	{
		pRT->SetFileLine(pFileName, iLine);
		if (pRT->Create(Width, Height, Format, MultiSample, MipLevelsCount))
		{
			aRenderTargets.Add(pRT);
		} else
		{
			RELEASE(pRT);
		}
	}

#ifdef _XBOX
	//т.к. внутри pRT->Create вызовется ReleaseOwnerShip
	NGRender::pRS->D3D()->AcquireThreadOwnership();
#endif

	UnlockRes(_FL_, resource_creation_sc, "CreateRenderTarget");

	return pRT;
}

IRenderTargetDepth* NGRender::CreateDepthStencil(dword Width, dword Height, const char * pFileName, long iLine, RENDERFORMAT Format, RENDERMULTISAMPLE_TYPE MultiSample, bool bCanResolveOnX360)
{
	RestoreIfNeed();

	LockRes(_FL_, resource_creation_sc, "CreateDepthStencil");

	api->Trace(" - CreateDepthStencil: %dx%d from (%s, %d) Format %d\n", Width, Height, pFileName, iLine, Format);

	RenderTargetDepth* pRT = NEW RenderTargetDepth();
	if (pRT)
	{
		pRT->SetFileLine(pFileName, iLine);
		if (pRT->Create(Width, Height, Format, MultiSample, bCanResolveOnX360))
		{
			aRenderTargetsDepth.Add(pRT);
		} else
		{
			RELEASE(pRT);
		}
	}

#ifdef _XBOX
	//т.к. внутри pRT->Create вызовется ReleaseOwnerShip
	NGRender::pRS->D3D()->AcquireThreadOwnership();
#endif


	UnlockRes(_FL_, resource_creation_sc, "CreateDepthStencil");

	return pRT;
}


bool NGRender::SetIndices(IIBuffer * pIBuffer, dword dwBaseVertexIndex)
{
	LockRes(_FL_, resource_creation_sc, "SetIndices");

	if (IsRenderDisabled())
	{
		UnlockRes(_FL_, resource_creation_sc, "SetIndices");
		return false;
	}

	dword dwUsage = 0;
	IDirect3DIndexBuffer9 * pIndexStream = NULL;
	if (pIBuffer)
	{
#ifndef _XBOX
		//Assert(!pIBuffer->IsReseted());
#endif

		pIndexStream = ((CDX8IBuffer*)pIBuffer)->GetDX8IndexBuffer();
		dwUsage = pIBuffer->GetUsage();
	}


	SetIndicesD3D(pIndexStream, dwBaseVertexIndex, dwUsage, pIBuffer);

	UnlockRes(_FL_, resource_creation_sc, "SetIndices");
	return true;
}

bool NGRender::SetTexture(dword dwStage, IBaseTexture * pTexture)
{
	LockRes(_FL_, resource_creation_sc, "SetTexture");


	if (pTexture == NULL)
	{
		pTexture = getWhiteTexture();
	}

	IDirect3DBaseTexture9* pDXTexture = (IDirect3DBaseTexture9*)pTexture->GetBaseTexture();


	HRESULT hr = D3D()->SetTexture(dwStage, pDXTexture);
	Assert(hr == D3D_OK);

	UnlockRes(_FL_, resource_creation_sc, "SetTexture");

	return true;
}

bool NGRender::SetStreamSource(dword StreamNumber, IVBuffer * pVBuffer, dword Stride, dword dwOffsetFromStartInBytes)
{
	LockRes(_FL_, resource_creation_sc, "SetStreamSource");
	if (StreamNumber == 0)
	{
		lastVBUsageFlag = 0;
	}

#ifndef _XBOX
	if (IsRenderDisabled())
	{
		UnlockRes(_FL_, resource_creation_sc, "SetStreamSource");
		return false;
	}
#endif


	IDirect3DVertexBuffer9* pVertexStream = NULL;
	dword dwUsage = 0;
	if (pVBuffer)
	{
#ifndef _XBOX
		//Assert(!pVBuffer->IsReseted());
#endif

		if (Stride == 0)
		{
			Stride = pVBuffer->GetStride();
		}

		if (Stride == 0)
		{
			UnlockRes(_FL_, resource_creation_sc, "SetStreamSource");
			api->Trace("DXError: try to SetStreamSource with stride = 0");
			return false; 
		}

		if (pVBuffer->IsLocked())
		{
			SetErrorSignal();
			api->Trace("DXError: try to SetStreamSource but vertex buffer is locked: file: %s, line: %d", pVBuffer->GetFileName(), pVBuffer->GetFileLine());
		}

		dwUsage = pVBuffer->GetUsage();
		pVertexStream = ((CDX8VBuffer*)pVBuffer)->GetDX8VertexBuffer();
	}



	SetStreamSourceD3D(StreamNumber, pVertexStream, Stride, dwOffsetFromStartInBytes, dwUsage, pVBuffer);


	UnlockRes(_FL_, resource_creation_sc, "SetStreamSource");
	return true;
}


// Render Target Section
IRenderTarget* NGRender::GetRenderTarget(const char* szFile, long line)
{
	RestoreIfNeed();

	AssertCoreThread;


	IRenderTarget* renderTarget = NULL;
	IRenderTargetDepth* renderTargetDepth = NULL;

	pDevice->GetRenderTarget(&renderTarget, &renderTargetDepth);

	if (renderTarget)
	{
		renderTarget->AddRef();
	}

	return renderTarget;
}

IRenderTargetDepth* NGRender::GetDepthStencil(const char* szFile, long line)
{
	RestoreIfNeed();

	AssertCoreThread;


	IRenderTarget* renderTarget = NULL;
	IRenderTargetDepth* renderTargetDepth = NULL;

	pDevice->GetRenderTarget(&renderTarget, &renderTargetDepth);

	if (renderTargetDepth)
	{
		renderTargetDepth->AddRef();
	}

	return renderTargetDepth;
}

bool NGRender::SetRenderTarget(RenderTartgetOptions options, IRenderTarget * pRenderTarget, IRenderTargetDepth * pZStencil)
{
	RestoreIfNeed();

	AssertCoreThread;

	if (IsRenderDisabled())
	{
		return false;
	}



	if (bInsideBeginScene)
	{
#ifndef _XBOX
		int res = MessageBox((HWND)api->Storage().GetLong("system.hwnd"), "Can't call SetRenderTarget inside BeginScene/EndScene pair, call EndScene before\nContinue work ?", "Error", MB_YESNO);
		if (res == IDNO)
		{
			Assert(false);
		}
#else
		Assert(false);
#endif
	}

	if (pRenderTarget && pZStencil)
	{
		RENDERSURFACE_DESC Zdesc;
		pZStencil->GetDesc(&Zdesc);

		RENDERSURFACE_DESC ColorDesc;
		pRenderTarget->GetDesc(&ColorDesc);

		//Проверка на соответствие размеров Z и Color буферов
		Assert(Zdesc.Width == ColorDesc.Width);
		Assert(Zdesc.Height == ColorDesc.Height);

		// Вано: если включен режим disableMSAA, то отключаем проверку на multi sample если включен режим disableMSAA
#ifdef _XBOX
		X360RenderDevice * x360 = (X360RenderDevice*)pDevice;
		if (x360->IsEnabledMSAA())
		{
			Assert(Zdesc.MultiSampleType == ColorDesc.MultiSampleType);
			Assert(Zdesc.MultiSampleQuality == ColorDesc.MultiSampleQuality);
		}
#else
		Assert(Zdesc.MultiSampleType == ColorDesc.MultiSampleType);
		Assert(Zdesc.MultiSampleQuality == ColorDesc.MultiSampleQuality);
#endif
	}


	if (pZStencil == NULL)
	{
		bDepthDisabled = true;
	} else
	{
		bDepthDisabled = false;
	}

	pDevice->SetRenderTarget(options, pRenderTarget, pZStencil);
	return true;
}

void NGRender::SetGPRAllocationCount (DWORD dwVertexShaderCount, DWORD dwPixelShaderCount)
{
#ifdef _XBOX
	Assert (dwVertexShaderCount+dwPixelShaderCount == 128);
	Assert (dwVertexShaderCount >= 16);
	Assert (dwPixelShaderCount >= 16);

	D3D()->SetShaderGPRAllocation(0, dwVertexShaderCount, dwPixelShaderCount);
#endif

}


bool NGRender::PushRenderTarget()
{
	AssertCoreThread;


	IRenderTarget* renderTarget = NULL;
	IRenderTargetDepth* renderTargetDepth = NULL;
	pDevice->GetRenderTarget(&renderTarget, &renderTargetDepth);


	stRenderTarget.Push();
	stRenderTarget.Top().color = renderTarget;
	stRenderTarget.Top().depth = renderTargetDepth;
	stRenderTarget.Top().viewPort = GetViewport();

	return true;
}

bool NGRender::PopRenderTarget(RenderTartgetOptions options)
{
	AssertCoreThread;


	if (!stRenderTarget.Size()) 
	{
		api->Trace("DXError: Try to pop RenderTarget, but RenderTarget stack is empty");
		return false;
	}

	SetRenderTarget(options, stRenderTarget.Top().color, stRenderTarget.Top().depth);


	SetViewport(stRenderTarget.Top().viewPort);

	stRenderTarget.Pop();

	return true;
}


//Utils section
bool NGRender::Clear(dword Count, CONST RENDERRECT * pRects, dword Flags, RENDERCOLOR Color, float Z, dword Stencil)
{
	RestoreIfNeed();

	LockRes(_FL_, resource_creation_sc, "NGRender::Clear");

	//dword dwId = pixBeginEvent(_FL_, "NGRender::Clear");

	DWORD dwFlags = ClearFlagsToDX(Flags);

	//Color = 0xFFFF0000;
	HRESULT hr = pDevice->D3D()->Clear(0, NULL, dwFlags, Color, Z, Stencil);
	Assert(hr == D3D_OK);

#ifdef _XBOX
	D3D()->FlushHiZStencil(D3DFHZS_ASYNCHRONOUS);
#endif

	//pixEndEvent(_FL_, dwId);

	UnlockRes(_FL_, resource_creation_sc, "NGRender::Clear");

	return true;
}

bool NGRender::BeginScene()
{
	RestoreIfNeed();


	//OutputDebugString("NGRender::BeginScene\n");
	//LockRes(_FL_, resource_creation_sc);

	loading_screen_sc.Enter();


	if (IsRenderDisabled())
	{
		loading_screen_sc.Leave();
		return false;
	}


	//Если в режиме загрузки...
	if (loadingScreenThread != NULL)
	{
		if (LoadingScreenThreadId != ::GetCurrentThreadId())
		{
			loading_screen_sc.Leave();
			api->Trace("WARNING: Try to work with render in enabled LoadingScreen");
			return false;
		}
	}
	loading_screen_sc.Leave();



	if (bInsideBeginScene)
	{
#ifndef _XBOX

		int res = MessageBox((HWND)api->Storage().GetLong("system.hwnd"), "Can't call BeginScene twice ! Call EndScene before.\nContinue work ?", "Error", MB_YESNO);
		if (res == IDNO)
		{
			Assert(false);
		}
#else
		Assert(false);
#endif
	}

	pDevice->BeginScene();
	bInsideBeginScene = true;

	//OutputDebugString("NGRender::BeginScene bInsideBeginScene = true\n");

	//UnlockRes(_FL_, resource_creation_sc);

	return true;
}

bool NGRender::EndScene(IBaseTexture* pDestiantionTexture, bool bSkipAnyWork, bool bDontResolveDepthOnX360)
{
	RestoreIfNeed();


#ifdef _DEBUG
	loading_screen_sc.Enter();
	//Если в режиме загрузки...
	if (loadingScreenThread != NULL)
	{
		if (LoadingScreenThreadId != ::GetCurrentThreadId())
		{
			loading_screen_sc.Leave();
			api->Trace("WARNING: Try to work with render in enabled LoadingScreen");
			return false;
		}
	}
	loading_screen_sc.Leave();
#endif

	//OutputDebugString("NGRender::EndScene\n");

	//LockRes(_FL_, resource_creation_sc);


	if (IsRenderDisabled())
	{
		return false;
	}



	if (!bInsideBeginScene)
	{
#ifndef _XBOX
		int res = MessageBox((HWND)api->Storage().GetLong("system.hwnd"), "Can't call EndScene before BeginScene\nContinue work ?", "Error", MB_YESNO);
		if (res == IDNO)
		{
			Assert(false);
		}
#else
		Assert(false);
#endif
		return true;
	}


	pDevice->EndScene(pDestiantionTexture, bSkipAnyWork, bDontResolveDepthOnX360);
	bInsideBeginScene = false;

	//OutputDebugString("NGRender::EndScene bInsideBeginScene = false\n");

	//UnlockRes(_FL_, resource_creation_sc);

	return true;
}

bool NGRender::IsInsideBeginScene() const
{
	AssertCoreThread;


	return bInsideBeginScene;
}

void _cdecl NGRender::SetBackgroundColor(const Color & color)
{
	AssertCoreThread;


	m_backColor = color;
}

void _cdecl NGRender::SetErrorSignal(float fSignalTime)
{
	//TODO:
}

bool NGRender::SetUseMipFillColor(bool bNewUseMipFillColor)
{
	AssertCoreThread;


	bool bOld = bUseMipFillColor;
	bUseMipFillColor = bNewUseMipFillColor;
	return bOld;
}


bool NGRender::SaveTexture2File(IBaseTexture * pTexture, const char * pFileName)
{
	AssertCoreThread;


#ifdef _XBOX
	return false;
#else
	if (!pTexture || !pFileName || !pFileName[0]) return false;
	if (pTexture->GetType() == DX8TYPE_TEXTURE)
	{
		D3DXSaveTextureToFileA(pFileName, D3DXIFF_DDS, (LPDIRECT3DBASETEXTURE9)((CDX8Texture*)pTexture)->GetBaseTexture(), null);
		return true;
	}


	return false;
#endif
}


#ifndef _XBOX
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT num = 0; // number of image encoders
	UINT size = 0; // size of the image encoder array in bytes
	Gdiplus::ImageCodecInfo* pImageCodecInfo;
	Gdiplus::GetImageEncodersSize(&num, &size);
	if(size == 0) return -1; // Failure
	pImageCodecInfo = (Gdiplus::ImageCodecInfo*) LocalAlloc(LPTR, size);
	if(pImageCodecInfo == NULL) return -1; // Failure
	Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);
	for(UINT j = 0; j < num; ++j)
	{
		if(_wcsicmp(pImageCodecInfo[j].MimeType, format) == 0 )
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			LocalFree(pImageCodecInfo);
			return j; // Success
		}
	}
	LocalFree(pImageCodecInfo);
	return -1; // Failure
}

#endif

bool NGRender::MakeScreenshot(const char * sufix)
{
	AssertCoreThread;


#ifndef _XBOX

	wchar_t cFileName[_MAX_PATH];
	for(long i = iScreenshotFrameIndex; i < iScreenshotFrameIndex + 10000; i++)
	{
		swprintf_s(cFileName, _MAX_PATH-2, L"CaptainBlood_%04d.jpg", i);

		if (_waccess(cFileName, 0) == -1) break;
	}


	HWND windowHWND = (HWND)api->Storage().GetLong("system.hwnd");
	HWND desktopHWND  = GetDesktopWindow();

	WINDOWINFO winInfo;
	GetWindowInfo(windowHWND, &winInfo);
	DWORD dwWinWidth = winInfo.rcClient.right - winInfo.rcClient.left;
	DWORD dwWinHeight = winInfo.rcClient.bottom - winInfo.rcClient.top;


	HDC hDesktopDC = GetDC(desktopHWND);
	HDC hCaptureDC = CreateCompatibleDC(hDesktopDC);
	HBITMAP hCaptureBitmap = CreateCompatibleBitmap(hDesktopDC, dwWinWidth, dwWinHeight);

	HBITMAP OldBmp = (HBITMAP)SelectObject(hCaptureDC, hCaptureBitmap); 

	BitBlt(hCaptureDC, 0, 0, dwWinWidth, dwWinHeight, hDesktopDC, winInfo.rcClient.left, winInfo.rcClient.top, SRCCOPY); 
	SelectObject(hCaptureDC, OldBmp); 

	static BOOL gInitialized = FALSE;
	Gdiplus::GdiplusStartupInput input;
	Gdiplus::GdiplusStartupOutput output;
	ULONG dwToken;
	Gdiplus::Status status = Gdiplus::Ok;

	if (!gInitialized)
	{
		status = Gdiplus::GdiplusStartup(&dwToken,&input,&output);
		gInitialized = TRUE;
	}

	if (status == Gdiplus::Ok)
	{
		CLSID jpgClsid;
		GetEncoderClsid(L"image/jpeg", &jpgClsid);
		Gdiplus::Bitmap SrcBitmap(hCaptureBitmap, 0);

		Gdiplus::EncoderParameters encoderParameters;
		ULONG quality;
		encoderParameters.Count = 1;
		encoderParameters.Parameter[0].Guid = Gdiplus::EncoderQuality;
		encoderParameters.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
		encoderParameters.Parameter[0].NumberOfValues = 1;
		quality = 100;
		encoderParameters.Parameter[0].Value = &quality;
		SrcBitmap.Save(cFileName, &jpgClsid, &encoderParameters);
	}

	//release...
	//=======================================================================               
	ReleaseDC(desktopHWND, hDesktopDC);
	DeleteDC(hCaptureDC);
	DeleteObject(hCaptureBitmap);


#endif

	return true;
}



// Viewport Section
bool NGRender::PushViewport()
{
	AssertCoreThread;


	stViewports.Push() = GetViewport();

	Assert(stViewports.Size() < 128);

	return true;
}

bool NGRender::PopViewport()
{
	AssertCoreThread;


	if (!stViewports.Size()) 
	{
		api->Trace("DXError: Try pop viewport, but viewport stack is empty.");
		return false;
	}

	SetViewport(stViewports.Top());

	stViewports.Pop();

	return true;
}

const RENDERVIEWPORT& NGRender::GetViewport() const
{
	return m_CurrentViewPort;
}

bool NGRender::SetViewport(const RENDERVIEWPORT & Viewport)
{
	AssertCoreThread;

	if (IsRenderDisabled()) return false;


	Assert (Viewport.MinZ >= 0.0f && Viewport.MinZ <= 1.0f)
		Assert (Viewport.MaxZ >= 0.0f && Viewport.MaxZ <= 1.0f)

		m_CurrentViewPort = Viewport;
	HRESULT hr = D3D()->SetViewport(&ViewPortToDx(m_CurrentViewPort));
	Assert(hr == D3D_OK);

	PrepareShadowLimiters();

	return true;
}

// Transform and Camera Section
bool NGRender::SetCamera(const Vector & vLookFrom, const Vector & vLookTo, const Vector & vUp)
{
	AssertCoreThread;


	Matrix mtx;
	if (!mtx.BuildView(vLookFrom, vLookTo, vUp)) 
	{
		if (!mtx.BuildView(vLookFrom, vLookTo, vUp))
		{
			return false;
		}
	}

	SetView(mtx);
	bNeedRecalculateFrustum = true;
	return true;
}

void NGRender::SetView(const Matrix & mMatrix)
{
	AssertCoreThread;


	*mtxView = mMatrix;
	bNeedRecalculateFrustum = true;

	vCameraPosition_WorldSpace = mtxView->GetCamPos();
	vCameraDirection_WorldSpace = -Vector(mtxView->m[0][2], mtxView->m[1][2], mtxView->m[2][2]);

	UpdateWVP();
}

void NGRender::SetWorld(const Matrix & mMatrix)
{
	AssertCoreThread;

	*mtxWorld = mMatrix;
	UpdateWVP();
}

void NGRender::SetProjection(const Matrix & mMatrix)
{
	AssertCoreThread;

	*mtxProjection = mMatrix;
	UpdateWVP();
}

bool NGRender::SetPerspective(float fPerspective)
{
	AssertCoreThread;

	Matrix mProjection;

	m_Screen2DInfo.fFOV = fPerspective;
	m_Screen3DInfo.fFOV = fPerspective;

	float fHeight = (float)GetViewport().Height; 
	float fWidth = (float)GetViewport().Width; 

	mProjection.BuildProjection(fPerspective, fWidth, fHeight, 0.1f, 4000.0f);

	SetProjection(mProjection);
	return true;
}

bool NGRender::SetPerspective(float fPerspective, float fWidth, float fHeight, float fZNear, float fZFar)
{
	AssertCoreThread;


	Matrix mProjection;
	mProjection.BuildProjection(fPerspective, fWidth, fHeight, fZNear, fZFar);
	SetProjection(mProjection);
	return true;
}

const Matrix& NGRender::GetView() const
{
	AssertCoreThread;


	return *mtxView;
}

const Matrix& NGRender::GetWorld() const
{
	AssertCoreThread;


	return *mtxWorld;
}

const Matrix& NGRender::GetProjection() const
{
	AssertCoreThread;


	return *mtxProjection;
}

const Plane* NGRender::GetFrustum()
{
	return GetFrustumFast();
}

dword NGRender::GetNumFrustumPlanes() const
{
	return 5;
}

// Clip planes
void NGRender::SetClipPlane(dword dwIndex, const Plane & plane)
{
	AssertCoreThread;


	HRESULT hr = D3D()->SetClipPlane(dwIndex, (float*)&plane);
	Assert(hr == D3D_OK);
}

// Light section
const Vector & NGRender::GetGlobalLightDirection()
{
	return stdLightVariables.dirLight.vGlobalLightDirection;
}

const Color & NGRender::GetGlobalLightColor()
{
	AssertCoreThread;

	return stdLightVariables.dirLight.vGlobalLightColor;
}

const Color & NGRender::GetGlobalLightBackColor()
{
	AssertCoreThread;

	return stdLightVariables.dirLight.vGlobalLightBackColor;
}


void NGRender::SetGlobalLight(const Vector & vDirection, bool bAttachSpecularDirToCam, const Color & cColor, const Color & cBackLightColor)
{
	AssertCoreThread;

	stdLightVariables.dirLight.vGlobalLightColor = cColor;
	stdLightVariables.dirLight.vGlobalLightColorSpecular = cColor;
	stdLightVariables.dirLight.vGlobalLightColorSpecular.v4.v.Normalize();
	stdLightVariables.dirLight.vGlobalLightColorSpecular.v4.v *= SPECULAR_STRENGHT;


	stdLightVariables.dirLight.vGlobalLightDirection = !vDirection;
	stdLightVariables.dirLight.dwAttachSpecDirToCam = bAttachSpecularDirToCam == true ? 1 : 0;
	stdLightVariables.dirLight.vGlobalLightBackColor = cBackLightColor;
}

dword NGRender::SetLights(const Vector & vPos, float fRadius)
{
	AssertCoreThread;

	//TODO:
	return 0;
}

void NGRender::EnumCreatedLights(array<ILight*> & createdLights)
{
	createdLights = aLights;
}


void NGRender::ReleaseLight(ILight * light)
{
	aLights.Del(light);

	lightPool.Free(light);
}

ILight* NGRender::CreateLight(const Vector & vPos, const Color & cColor, float fRadius, float fDirectivityFactor)
{
	RestoreIfNeed();

	LockRes(_FL_, resource_creation_sc, "CreateLight");


	ILight * pLight = lightPool.Allocate();;
	if (!pLight)
	{
		UnlockRes(_FL_, resource_creation_sc, "CreateLight");
		return null;
	}

	aLights.Add(pLight);

	pLight->Init(this);
	pLight->Set(vPos, cColor, fRadius, fDirectivityFactor);

	//pLight->SetFileLine(_FL_);

	UnlockRes(_FL_, resource_creation_sc, "CreateLight");

	return pLight;
}


void NGRender::SetAmbientSpecular (const Color& clr)
{
	AssertCoreThread;
	stdLightVariables.ambientLight.vAmbientLightSpecular = clr;

}

const Color& NGRender::GetAmbientSpecular ()
{
	AssertCoreThread;

	return stdLightVariables.ambientLight.vAmbientLightSpecular;
}

void NGRender::SetAmbient (const Color& clr)
{
	AssertCoreThread;
	stdLightVariables.ambientLight.vAmbientLight = clr;
}

void NGRender::SetAmbientGamma(float fGamma)
{
	stdLightVariables.ambientLight.vAmbientLight.a = fGamma;
}

float NGRender::GetAmbientGamma()
{
	return stdLightVariables.ambientLight.vAmbientLight.a;
}


const Color& NGRender::GetAmbient ()
{
	AssertCoreThread;

	return stdLightVariables.ambientLight.vAmbientLight;
}


bool NGRender::DrawIndexedPrimitive(const ShaderId & id, RENDERPRIMITIVETYPE Type, dword MinVertexIndex, dword NumVertices, dword StartIndex, dword PrimitiveCount)
{
	LockRes(_FL_, resource_creation_sc, "DrawIndexedPrimitive");

	bool bRes = DrawIndexedPrimitiveSingleThread(id, Type, MinVertexIndex, NumVertices, StartIndex, PrimitiveCount);

	UnlockRes(_FL_, resource_creation_sc, "DrawIndexedPrimitive");

	return bRes;
}

// Draw section
bool NGRender::DrawIndexedPrimitiveSingleThread(const ShaderId & id, RENDERPRIMITIVETYPE Type, dword MinVertexIndex, dword NumVertices, dword StartIndex, dword PrimitiveCount)
{
#ifdef RENDER_DEBUG_DRAW_SKIP
	curDrawIndex++;
	if (IsDebugDrawSkipped(curDrawIndex - 1))
		return true;
#endif

#ifndef _XBOX

	if (IsRenderDisabled())
	{
		return false;
	}

	if (((lastVBUsageFlag & USAGE_DYNAMIC) != 0) && ((lastIBUsageFlag & USAGE_DYNAMIC) == 0))
	{
		//Vertex buffer is dynamic, but index buffer not - X360 assert emulation
		//                                      api->Trace("Vertex buffer is dynamic, but index buffer not - X360 assert emulation");
		//                                      Assert(false);
	}

	if (((lastVBUsageFlag & USAGE_DYNAMIC) == 0) && ((lastIBUsageFlag & USAGE_DYNAMIC) != 0))
	{
		//Vertex buffer is not dynamic, but index buffer is dynamic - X360 assert emulation
		//                                      api->Trace("Vertex buffer is not dynamic, but index buffer is dynamic - X360 assert emulation");
		//                                      Assert(false);
	}

	if (pLastIndices == NULL)
	{
		//DrawIndexedPrimitive without indices ????
		api->Trace("DrawIndexedPrimitive without indices ???? - X360 assert emulation");
		return false;
	}

#endif


	if (PrimitiveCount <= 0)
	{
		api->Trace("DXERROR: Can't render zero primitives !!!");
		return false;
	}

	dword dwTime = 0;
	RDTSC_B(dwTime);

	SHADER* shader = (SHADER*)(id);
	if (!shader)
	{
		api->Error("DXERROR: Can't find shader");
		return false;
	}


	//-------------------------------
#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)



	if (!bNeedReplaceDP_to_DPUP)
	{
		shader->PrefetchCommandBuffer();

		stats.dwPolyCount += PrimitiveCount;
		stats.dwBatchCount++;
		IDirect3DIndexBuffer9* pIndexData = NULL;
		D3D()->GetIndices(&pIndexData);

		m_ShaderManager.getVarDatabase()->ApplyStandartVariables(stdVariables);
		m_ShaderManager.getVarDatabase()->ApplyEnvironmentLightingVariables(stdLightVariables, *stdVariables.vCamDirRelativeWorld);
		m_ShaderManager.getVarDatabase()->ApplyFogVariables(stdFogVariables);

		shader->UpdateVariablesFromDB();
		shader->SetStreamSource(0, vertexStreams[0].vertexData, vertexStreams[0].dwOffsetFromStartInBytes);
		shader->SetStreamSource(1, vertexStreams[1].vertexData, vertexStreams[1].dwOffsetFromStartInBytes);
		shader->SetStreamSource(2, vertexStreams[2].vertexData, vertexStreams[2].dwOffsetFromStartInBytes);
		shader->SetDrawIndexedParameters((GPUPRIMTYPE)PtToDX (Type), pIndexData, PrimitiveCount, StartIndex);

		if (pIndexData)
		{
			pIndexData->Release();
		}


		/*
		if (passLights.Size() > 0)
		{
		BYTE * srcCommandBufferData = shader->getPhysicalBytes();
		DWORD dwSizeInDwords = shader->getSizeInDwords();
		DWORD dwOffsetToEndToken = dwSizeInDwords * sizeof(DWORD);

		GPUCOMMAND_BUFFER_END * cmdEnd = (GPUCOMMAND_BUFFER_END*)(srcCommandBufferData + dwOffsetToEndToken);
		cmdEnd->Place();

		shader->debugOutCommandBuffer(srcCommandBufferData, true);

		int a = 0;
		}
		*/

		shader->ApplyStateHacksInplace(D3D(), &hacks[0], bDisableHacks);

		bool bDisableAllHiZ = bDepthDisabled | bDisableHiZ;
		m_ShaderManager.getBatchExecutor()->Run(shader, D3D(), bDepthDisabled, bDisableAllHiZ);

		shader->RestoreStateHacksInplace(D3D(), &hacks[0], bDisableHacks);


		RDTSC_E(dwTime);
		stats.dwDrawTime += dwTime;

		return true;
	} 

	stats.dwPolyCount += PrimitiveCount;
	stats.dwBatchCountUP++;

	m_ShaderManager.getVarDatabase()->ApplyStandartVariables(stdVariables);
	m_ShaderManager.getVarDatabase()->ApplyEnvironmentLightingVariables(stdLightVariables, *stdVariables.vCamDirRelativeWorld);
	m_ShaderManager.getVarDatabase()->ApplyFogVariables(stdFogVariables);


	shader->DynamicDraw_SetupGPU(D3D());
	shader->DynamicDraw_ApplyStateHacks(D3D(), &hacks[0], bDisableHacks);

	byte* ptr = ((CDX8VBuffer*)pMemVB)->GetMemoryVertexBuffer();

	//Memory index buffer must also use DYNAMIC FLAG !!!!
	Assert(pMemIB);
	byte* mIB = ((CDX8IBuffer*)pMemIB)->GetMemoryIndexBuffer();
	D3DFORMAT ibFMT = FormatToDX(((CDX8IBuffer*)pMemIB)->GetFormat());
	//Assert(MinVertexIndex == 0);
	//Assert(StartIndex == 0);

	//Слишком много полигонов для Ring buffer'a
	Assert((NumVertices*dwMemVBStride)+(PrimitiveCount*3*sizeof(word)) < (2 * 1024 * 1024));

	DWORD dwDpUpSize = ((CDX8VBuffer*)pMemVB)->GetSize() + ((PrimitiveCount*3) * sizeof(word));
	if (dwDpUpSize >= stats.dwMaxDpUpSize)
	{
		stats.dwMaxDpUpSize = dwDpUpSize;
	}
	stats.dwDrawUpBytes += dwDpUpSize;


	HRESULT hr = D3D()->DrawIndexedPrimitiveUP(PtToDX (Type), MinVertexIndex, NumVertices, PrimitiveCount, mIB + (sizeof(word) * StartIndex), ibFMT, ptr, dwMemVBStride);

	if (shader->isNeedFlushHiZ() || bDepthDisabled)
	{
		D3D()->FlushHiZStencil(D3DFHZS_ASYNCHRONOUS);
	}

	shader->DynamicDraw_RestoreStateHacks(D3D(), &hacks[0], bDisableHacks);

	RDTSC_E(dwTime);
	stats.dwDrawTimeUP += dwTime;

	return false;

#else
	//-----------------------------


	shader->ApplyStandartVariables(stdVariables);
	shader->ApplyEnvironmentLightingVariables(stdLightVariables, *stdVariables.vCamDirRelativeWorld);
	shader->ApplyFogVariables(stdFogVariables);


	dword passCount = shader->Begin();
	for (dword n = 0; n < passCount; n++)
	{
		shader->BeginPass(n);


		stats.dwPolyCount += PrimitiveCount;

#ifndef _XBOX
		if (!bDisableHacks)
		{
			if (api->DebugKeyState('F') && ::GetCurrentThreadId() == api->GetThreadId())
			{
				D3D()->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
				SetTexture(0, getWhiteTexture());
				SetTexture(1, getWhiteTexture());
				SetTexture(2, getWhiteTexture());
				SetTexture(3, getWhiteTexture());
			}
		}
#endif          
		ApplyStateHacks();

		HRESULT hr;
		if (bNeedReplaceDP_to_DPUP)
		{
			stats.dwBatchCountUP++;
			byte* ptr = ((CDX8VBuffer*)pMemVB)->GetMemoryVertexBuffer();

			//Memory index buffer must also use DYNAMIC FLAG !!!!
			Assert(pMemIB);
			byte* mIB = ((CDX8IBuffer*)pMemIB)->GetMemoryIndexBuffer();
			D3DFORMAT ibFMT = FormatToDX(((CDX8IBuffer*)pMemIB)->GetFormat());
			//Assert(MinVertexIndex == 0);
			//Assert(StartIndex == 0);

			//Слишком много полигонов для Ring buffera
			Assert((NumVertices*dwMemVBStride)+(PrimitiveCount*3*sizeof(word)) < (2 * 1024 * 1024));


			DWORD dwDpUpSize = ((CDX8VBuffer*)pMemVB)->GetSize() + ((PrimitiveCount*3) * sizeof(word));
			if (dwDpUpSize >= stats.dwMaxDpUpSize)
			{
				stats.dwMaxDpUpSize = dwDpUpSize;
			}
			stats.dwDrawUpBytes += dwDpUpSize;



			hr = D3D()->DrawIndexedPrimitiveUP(PtToDX (Type), MinVertexIndex, NumVertices, PrimitiveCount, mIB + (sizeof(word) * StartIndex), ibFMT, ptr, dwMemVBStride);
		} else
		{

			/*
			void * data = NULL;
			vertexStreams[0].vertexData->Lock(0, 0, &data, 0);


			WORD* pInd = (WORD*)pLastIndices->Lock();

			for (int rrr = 0; rrr < PrimitiveCount * 3; rrr++)
			{
			Assert (pInd[StartIndex + rrr] < NumVertices);
			}


			vertexStreams[0].vertexData->Unlock();
			pLastIndices->Unlock();
			*/

			stats.dwBatchCount++;
			hr = D3D()->DrawIndexedPrimitive(PtToDX (Type), dwCurrentBaseVertexIndex, MinVertexIndex, NumVertices, StartIndex, PrimitiveCount);

		}

		shader->EndPass(n);
	} //pases

	shader->End();

	RestoreAfterStateHacks ();

	RDTSC_E(dwTime);

	if (bNeedReplaceDP_to_DPUP == false)
	{
		stats.dwDrawTime += dwTime;
	} else
	{
		stats.dwDrawTimeUP += dwTime;
	}

	return true;
#endif
}

bool NGRender::DrawPrimitive(const ShaderId & id, RENDERPRIMITIVETYPE Type, dword StartVertex, dword PrimitiveCount)
{
	AssertCoreThread;

#ifdef RENDER_DEBUG_DRAW_SKIP
	curDrawIndex++;
	if (IsDebugDrawSkipped(curDrawIndex - 1))
		return true;
#endif

	if (PrimitiveCount == 0)
	{
		api->Trace("ERROR: Can't render technique polycount = 0 !!!!");
		return false;
	}

#ifndef _XBOX
	if (IsRenderDisabled())
	{
		return false;
	}
#endif

	dword dwTime = 0;
	RDTSC_B(dwTime);

	SHADER* shader = (SHADER*)id;
	if (!shader)
	{
		api->Error("DXERROR: Can't find shader");
		return false;
	}


#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)

	if (!bNeedReplaceDP_to_DPUP)
	{
		shader->PrefetchCommandBuffer();

		stats.dwPolyCount += PrimitiveCount;
		stats.dwBatchCount++;

		m_ShaderManager.getVarDatabase()->ApplyStandartVariables(stdVariables);
		m_ShaderManager.getVarDatabase()->ApplyEnvironmentLightingVariables(stdLightVariables, *stdVariables.vCamDirRelativeWorld);
		m_ShaderManager.getVarDatabase()->ApplyFogVariables(stdFogVariables);

		shader->UpdateVariablesFromDB();
		shader->SetStreamSource(0, vertexStreams[0].vertexData, vertexStreams[0].dwOffsetFromStartInBytes);
		shader->SetStreamSource(1, vertexStreams[1].vertexData, vertexStreams[1].dwOffsetFromStartInBytes);
		shader->SetStreamSource(2, vertexStreams[2].vertexData, vertexStreams[2].dwOffsetFromStartInBytes);
		shader->SetDrawParameters((GPUPRIMTYPE)PtToDX (Type), PrimitiveCount, StartVertex);


		/*
		if (PrimitiveCount > 21844)
		{
		BYTE * srcCommandBufferData = shader->getPhysicalBytes();
		DWORD dwSizeInDwords = shader->getSizeInDwords();
		DWORD dwOffsetToEndToken = dwSizeInDwords * sizeof(DWORD);

		GPUCOMMAND_BUFFER_END * cmdEnd = (GPUCOMMAND_BUFFER_END*)(srcCommandBufferData + dwOffsetToEndToken);
		cmdEnd->Place();

		shader->debugOutCommandBuffer(srcCommandBufferData, true);

		int a = 0;
		}
		*/


		shader->ApplyStateHacksInplace(D3D(), &hacks[0], bDisableHacks);

		bool bDisableAllHiZ = bDepthDisabled | bDisableHiZ;
		m_ShaderManager.getBatchExecutor()->Run(shader, D3D(), bDepthDisabled, bDisableAllHiZ);

		shader->RestoreStateHacksInplace(D3D(), &hacks[0], bDisableHacks);



		RDTSC_E(dwTime);
		stats.dwDrawTime += dwTime;

		return true;
	} 

	m_ShaderManager.getVarDatabase()->ApplyStandartVariables(stdVariables);
	m_ShaderManager.getVarDatabase()->ApplyEnvironmentLightingVariables(stdLightVariables, *stdVariables.vCamDirRelativeWorld);
	m_ShaderManager.getVarDatabase()->ApplyFogVariables(stdFogVariables);

	shader->DynamicDraw_SetupGPU(D3D());
	shader->DynamicDraw_ApplyStateHacks(D3D(), &hacks[0], bDisableHacks);

	byte* ptr = ((CDX8VBuffer*)pMemVB)->GetMemoryVertexBuffer();
	HRESULT hr = D3D()->DrawPrimitiveUP(PtToDX(Type), PrimitiveCount, ptr + (StartVertex*dwMemVBStride), dwMemVBStride);

	if (shader->isNeedFlushHiZ() || bDepthDisabled)
	{
		D3D()->FlushHiZStencil(D3DFHZS_ASYNCHRONOUS);
	}

	shader->DynamicDraw_RestoreStateHacks(D3D(), &hacks[0], bDisableHacks);


	RDTSC_E(dwTime);
	stats.dwPolyCount += PrimitiveCount;
	stats.dwBatchCountUP++;
	stats.dwDrawTimeUP += dwTime;


	DWORD dwDpUpSize = ((CDX8VBuffer*)pMemVB)->GetSize();
	if (dwDpUpSize >= stats.dwMaxDpUpSize)
	{
		stats.dwMaxDpUpSize = dwDpUpSize;
	}
	stats.dwDrawUpBytes += dwDpUpSize;


	return false;
#else

	shader->ApplyStandartVariables(stdVariables);
	shader->ApplyEnvironmentLightingVariables(stdLightVariables, *stdVariables.vCamDirRelativeWorld);
	shader->ApplyFogVariables(stdFogVariables);

	dword passCount = shader->Begin();

	for (dword n = 0; n < passCount; n++)
	{
		shader->BeginPass(n);

		stats.dwPolyCount += PrimitiveCount;

#ifndef _XBOX
		if (!bDisableHacks)
		{
			if (api->DebugKeyState('F') && ::GetCurrentThreadId() == api->GetThreadId() && bDisableHacks != true)
			{
				D3D()->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
				SetTexture(0, getWhiteTexture());
				SetTexture(1, getWhiteTexture());
				SetTexture(2, getWhiteTexture());
				SetTexture(3, getWhiteTexture());
			}
		}
#endif

		ApplyStateHacks ();

		HRESULT hr;
		if (bNeedReplaceDP_to_DPUP)
		{
			stats.dwBatchCountUP++;

			DWORD dwDpUpSize = ((CDX8VBuffer*)pMemVB)->GetSize();
			if (dwDpUpSize >= stats.dwMaxDpUpSize)
			{
				stats.dwMaxDpUpSize = dwDpUpSize;
			}
			stats.dwDrawUpBytes += dwDpUpSize;

			byte* ptr = ((CDX8VBuffer*)pMemVB)->GetMemoryVertexBuffer();
			hr = D3D()->DrawPrimitiveUP(PtToDX(Type), PrimitiveCount, ptr + (StartVertex*dwMemVBStride), dwMemVBStride);
		} else
		{
			stats.dwBatchCount++;
			hr = D3D()->DrawPrimitive(PtToDX(Type), StartVertex, PrimitiveCount);
		}

		Assert(hr == D3D_OK);

		shader->EndPass(n);
	} //passes

	shader->End();

	RestoreAfterStateHacks ();

	RDTSC_E(dwTime);

	if (!bNeedReplaceDP_to_DPUP)
	{
		stats.dwDrawTime += dwTime;
	} else
	{
		stats.dwDrawTimeUP += dwTime;
	}



	return true;
#endif
}

bool NGRender::DrawPrimitiveUP(const ShaderId & id, RENDERPRIMITIVETYPE PrimitiveType, dword PrimitiveCount, const void * pVertex, dword Stride)
{
	LockRes(_FL_, resource_creation_sc, "DrawPrimitiveUP");

#ifdef RENDER_DEBUG_DRAW_SKIP
	curDrawIndex++;
	if (IsDebugDrawSkipped(curDrawIndex - 1))
		return true;
#endif

#ifndef _XBOX
	if (IsRenderDisabled())
	{
		UnlockRes(_FL_, resource_creation_sc, "DrawPrimitiveUP");
		return false;
	}
#endif

	dword dwTime = 0;
	RDTSC_B(dwTime);

	SHADER* shader = (SHADER*)id;
	if (!shader)
	{
		UnlockRes(_FL_, resource_creation_sc, "DrawPrimitiveUP");
		api->Error("DXERROR: Can't find shader");
		return false;
	}


#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)

	m_ShaderManager.getVarDatabase()->ApplyStandartVariables(stdVariables);
	m_ShaderManager.getVarDatabase()->ApplyEnvironmentLightingVariables(stdLightVariables, *stdVariables.vCamDirRelativeWorld);
	m_ShaderManager.getVarDatabase()->ApplyFogVariables(stdFogVariables);

	shader->DynamicDraw_SetupGPU(D3D());
	shader->DynamicDraw_ApplyStateHacks(D3D(), &hacks[0], bDisableHacks);

	HRESULT hr = D3D()->DrawPrimitiveUP(PtToDX(PrimitiveType), PrimitiveCount, pVertex, Stride);

	if (shader->isNeedFlushHiZ() || bDepthDisabled)
	{
		D3D()->FlushHiZStencil(D3DFHZS_ASYNCHRONOUS);
	}

	shader->DynamicDraw_RestoreStateHacks(D3D(), &hacks[0], bDisableHacks);


	RDTSC_E(dwTime);
	stats.dwBatchCountUP++;
	stats.dwDrawTimeUP += dwTime;
	stats.dwPolyCount += PrimitiveCount;


	DWORD dwDpUpSize = PrimitiveCount * 3 * Stride;
	if (dwDpUpSize >= stats.dwMaxDpUpSize)
	{
		stats.dwMaxDpUpSize = dwDpUpSize;
	}
	stats.dwDrawUpBytes += dwDpUpSize;



	UnlockRes(_FL_, resource_creation_sc, "DrawPrimitiveUP");
	return true;

#else


	shader->ApplyStandartVariables(stdVariables);
	shader->ApplyEnvironmentLightingVariables(stdLightVariables, *stdVariables.vCamDirRelativeWorld);
	shader->ApplyFogVariables(stdFogVariables);

	dword passCount = shader->Begin();
	for (dword n = 0; n < passCount; n++)
	{
		shader->BeginPass(n);

		stats.dwBatchCountUP++;
		stats.dwPolyCount += PrimitiveCount;

		DWORD dwDpUpSize = PrimitiveCount * 3 * Stride;
		if (dwDpUpSize >= stats.dwMaxDpUpSize)
		{
			stats.dwMaxDpUpSize = dwDpUpSize;
		}
		stats.dwDrawUpBytes += dwDpUpSize;



		ApplyStateHacks ();

		HRESULT hr = D3D()->DrawPrimitiveUP(PtToDX(PrimitiveType), PrimitiveCount, pVertex, Stride);
		Assert(hr == D3D_OK);

		shader->EndPass(n);
	} //passes

	shader->End();
	RestoreAfterStateHacks ();

	RDTSC_E(dwTime);
	stats.dwDrawTimeUP += dwTime;

	UnlockRes(_FL_, resource_creation_sc, "DrawPrimitiveUP");
	return true;
#endif
}


/*
bool NGRender::DrawIndexedPrimitiveLighted(RENDERPRIMITIVETYPE Type, dword MinVertexIndex, dword NumVertices, dword StartIndex, dword PrimitiveCount, const GMXOBB &boundBox, const char * pTechnique, void * pObsoletteMustBeNUL)
{
if (IsRenderDisabled()) return false;

//TODO:
api->Trace("TODO: DrawIndexedPrimitiveLighted");
return false;
}


bool NGRender::DrawLines(RS_LINE * pRSL, dword dwLinesNum, const char * pTechniqueName, void * pObsoletteMustBeNULL, dword dwStride)
{
if (IsRenderDisabled()) return false;

//TODO:
api->Trace("TODO: DrawLines");
return false;
}

bool NGRender::DrawRects(RS_RECT * pRects, dword dwRectsNum, const char * pTechniqueName, void * pObsoletteMustBeNULL, dword dwStride, dword dwSubTexX, dword dwSubTexY)
{
if (IsRenderDisabled()) return false;

//TODO:
api->Trace("TODO: DrawRects");
return false;
}

bool NGRender::DrawSprites(RS_SPRITE * pSprites, dword dwSpritesNum, const char * pTechniqueName, void * pObsoletteMustBeNULL, dword dwStride)
{
if (IsRenderDisabled()) return false;


//TODO:
api->Trace("TODO: DrawSprites");
return false;
}

bool NGRender::DrawPolygon(const Vector * pVectors, dword dwNumPoints, const Color & color, const Matrix& matrix, const char * pTechniqueName, void * pObsoletteMustBeNULL)
{
if (IsRenderDisabled()) return false;

//TODO:
api->Trace("TODO: DrawPolygon");
return false;
}


// Debug Draw section
bool NGRender::DrawSphere(const Matrix & mPos, dword dwColor, const char * pTechniqueName, void * pObsoletteMustBeNULL)
{
if (IsRenderDisabled()) return false;

//TODO:
api->Trace("TODO: DrawSphere");
return false;
}

bool NGRender::DrawSphere(const Vector & vPos, float fRadius, dword dwColor, const char * pTechniqueName, void * pObsoletteMustBeNULL)
{
if (IsRenderDisabled()) return false;

//TODO:
api->Trace("TODO: DrawSphere");
return false;
}

bool NGRender::DrawLine(const Vector & v1, dword dwColor1, const Vector & v2, dword dwColor2, bool bWorldMatrix, const char * pTechniqueName, void * pObsoletteMustBeNULL)
{
if (IsRenderDisabled()) return false;

//TODO:
api->Trace("TODO: DrawLine");
return false;
}

bool NGRender::DrawBufferedLine(const Vector & v1, dword dwColor1, const Vector & v2, dword dwColor2, bool bWorldMatrix)
{
if (IsRenderDisabled()) return false;

//TODO:
api->Trace("TODO: DrawBufferedLine");
return false;
}

bool NGRender::DrawBox(const Vector & vMin, const Vector & vMax, const Matrix & mMatrix, dword dwColor, const char * pTechniqueName, void * pObsoletteMustBeNULL)
{
if (IsRenderDisabled()) return false;

//TODO:
api->Trace("TODO: DrawBox");
return false;
}

bool NGRender::DrawCapsule(float fRadius, float fHeight, dword dwColor, const Matrix &WorldMat, const char * pTechniqueName, void * pObsoletteMustBeNULL)
{
if (IsRenderDisabled()) return false;

//TODO:
api->Trace("TODO: DrawCapsule");
return false;
}

bool NGRender::DrawSolidBox(const Vector & vMin, const Vector & vMax, const Matrix & mMatrix, dword dwColor, const char * pTechniqueName, void * pObsoletteMustBeNULL)
{
if (IsRenderDisabled()) return false;

//TODO:
api->Trace("TODO: DrawSolidBox");
return false;
}

bool NGRender::DrawMatrix(const Matrix & mMatrix, float fScale, const char * pTechniqueName, void * pObsoletteMustBeNULL)
{
if (IsRenderDisabled()) return false;

//TODO:
api->Trace("TODO: DrawMatrix");
return false;
}

bool NGRender::DrawVector(const Vector & v1, const Vector & v2, dword dwColor, const char * pTechniqueName, void * pObsoletteMustBeNULL)
{
if (IsRenderDisabled()) return false;

//TODO:
api->Trace("TODO: DrawVector");
return false;
}

bool NGRender::DrawNormals(IVBuffer * pVBuffer, dword dwStartVertex, dword dwNumVertices, dword dwColor1, dword dwColor2, float fScale, dword dwStride, const char * pTechniqueName, void * pObsoletteMustBeNULL)
{
if (IsRenderDisabled()) return false;

//TODO:
api->Trace("TODO: DrawNormals");
return false;
}

bool NGRender::FlushBufferedLines(bool bIdentityMatrix)
{
if (IsRenderDisabled()) return false;

//TODO:
api->Trace("TODO: FlushBufferedLines");
return false;
}

void NGRender::DrawXZCircle (const Vector& center, float fRadius, dword dwColor, const char* szTechnique)
{
if (IsRenderDisabled()) return;

//TODO:
api->Trace("TODO: DrawXZCircle");
}

void NGRender::DrawSphereGizmo (const Vector& pos, float fRadius, dword dwColor1, dword dwColor2, const char* szTechnique)
{
if (IsRenderDisabled()) return;

//TODO:
api->Trace("TODO: DrawSphereGizmo");
}

*/

// Filter section
bool NGRender::AddRenderFilter(IRenderFilter * pFilter, float fExecuteLevel)
{
	AssertCoreThread;


	pFilter->SetExecuteLevel(fExecuteLevel);
	aRenderFilters.Add(pFilter);
	return true;
}

bool NGRender::RemoveRenderFilter(IRenderFilter * pFilter)
{
	AssertCoreThread;

	for (DWORD i = 0; i < aRenderFilters.Size(); i++)
	{
		if (aRenderFilters[i] == pFilter)
		{
			aRenderFilters.ExtractNoShift(i);
			return true;
		}
	}


	return false;

}


// Create advanced new font
IAdvFont * _cdecl NGRender::CreateAdvancedFont(const char * pFontName, float fHeight, dword dwColor)
{
	RestoreIfNeed();

	LockRes(_FL_, resource_creation_sc, "CreateFont");

	IAdvFont * pFont = pAdvFonts->CreateAdvancedFont(pFontName, fHeight, dwColor);
	if (!pFont) 
	{
		// error
		api->Trace("DXError: Can't create advanced font %s", pFontName);
		UnlockRes(_FL_, resource_creation_sc, "CreateAdvancedFont");
		return null;
	}

#ifdef _XBOX
	D3D()->AcquireThreadOwnership();
#endif


	UnlockRes(_FL_, resource_creation_sc, "CreateFont");

	return pFont;
}


// Font and print section
IFont* _cdecl NGRender::CreateFont(const char * pFontName, float fHeight, dword dwColor, const char * pFontTechnique)
{
	RestoreIfNeed();

	LockRes(_FL_, resource_creation_sc, "CreateFont");

	IFont * pFont = pFonts->CreateFont(pFontName, fHeight, dwColor);
	if (!pFont) 
	{
		// error
		api->Trace("DXError: Can't create font %s", pFontName);
		UnlockRes(_FL_, resource_creation_sc, "CreateFont");
		return null;
	}

#ifdef _XBOX
	D3D()->AcquireThreadOwnership();
#endif

	pFont->SetTechnique(pFontTechnique);

#ifdef _XBOX
	D3D()->AcquireThreadOwnership();
#endif

	UnlockRes(_FL_, resource_creation_sc, "CreateFont");

	return pFont;
}

IFont* NGRender::GetSystemFont() const
{
	AssertCoreThread;


	if (pSystemFont)
	{
		pSystemFont->AddRef();
	}

	return pSystemFont; 
}


bool _cdecl NGRender::PrintShadowed(float x, float y, dword dwColor, const char * pFormat, ...)
{
	if(!pFormat) return true;
	LockRes(_FL_, resource_creation_sc, "Print");


	if (IsRenderDisabled() || !pSystemFont) return false;

	static char cTmpBuffer[4096];

	va_list args;
	va_start(args, pFormat);
	crt_vsnprintf(cTmpBuffer, sizeof(cTmpBuffer), pFormat, args);
	va_end(args);

	bDisableHacks = true;
	D3D()->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	pSystemFont->SetColor(0xFF202020);
	pSystemFont->Print(x+1, y+1, "%s", cTmpBuffer);

	pSystemFont->SetColor(dwColor);
	pSystemFont->Print(x, y, "%s", cTmpBuffer);
	bDisableHacks = false;

	UnlockRes(_FL_, resource_creation_sc, "Print");

	return true;
}

bool _cdecl NGRender::Print(float x, float y, dword dwColor, const char * pFormat, ...)
{
	if(!pFormat) return true;
	LockRes(_FL_, resource_creation_sc, "Print");


	if (IsRenderDisabled() || !pSystemFont) return false;

	static char cTmpBuffer[4096];

	va_list args;
	va_start(args, pFormat);
	crt_vsnprintf(cTmpBuffer, sizeof(cTmpBuffer), pFormat, args);
	va_end(args);

	bDisableHacks = true;
	D3D()->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	pSystemFont->SetColor(dwColor);
	pSystemFont->Print(x, y, "%s", cTmpBuffer);
	bDisableHacks = false;

	UnlockRes(_FL_, resource_creation_sc, "Print");

	return true;
}

bool _cdecl NGRender::Print(const Vector & vPos, float fViewDistance, float fLine, dword dwColor, const char * pFormat, ...)
{
	AssertCoreThread;


	if (IsRenderDisabled() || !pSystemFont) return false;

	static char cTmpBuffer[4096];

	va_list args;
	va_start(args, pFormat);
	crt_vsnprintf(cTmpBuffer, sizeof(cTmpBuffer), pFormat, args);
	va_end(args);

	bDisableHacks = true;
	D3D()->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	pSystemFont->SetColor(dwColor);
	pSystemFont->Print(vPos, fViewDistance, fLine, "%s", cTmpBuffer);
	bDisableHacks = false;

	return true;
}

bool _cdecl NGRender::PrintBuffered(float x, float y, dword dwColor, const char * pFormat, ...)
{
	AssertCoreThread;


	static char cTmpBuffer[4096];

	va_list args;
	va_start(args, pFormat);
	crt_vsnprintf(cTmpBuffer, sizeof(cTmpBuffer), pFormat, args);
	va_end(args);

	PrintBuffer* pPB = &aBufferedPrints[aBufferedPrints.Add()];
	pPB->x = x;
	pPB->y = y;
	pPB->bCentered = false;
	pPB->dwColor = dwColor;
	COPY_STRING(pPB->pBuffer, cTmpBuffer);
	return true;
}

bool _cdecl NGRender::PrintBuffered(const Vector & vPos, float fViewDistance, float fLine, dword dwColor, const char * pFormat, ...)
{
	AssertCoreThread;


	static char cTmpBuffer[4096];

	va_list args;
	va_start(args, pFormat);
	crt_vsnprintf(cTmpBuffer, sizeof(cTmpBuffer), pFormat, args);
	va_end(args);

	PrintBuffer * pPB = &aBufferedPrints[aBufferedPrints.Add()];
	pPB->vPos = vPos;
	pPB->fViewDistance = fViewDistance;
	pPB->bCentered = true;
	pPB->fLine = fLine;
	pPB->dwColor = dwColor;
	COPY_STRING(pPB->pBuffer, cTmpBuffer);
	return true;
}


//update textures
bool NGRender::UpdateTexture(ITexture * pTexSource, ITexture * pTexDestination)
{
	AssertCoreThread;


#ifndef _XBOX
	if (!pTexSource || !pTexDestination)
	{
		return false;
	}

	HRESULT hr = D3D()->UpdateTexture((IDirect3DBaseTexture9 *)((CDX8Texture*)pTexSource)->GetBaseTexture(), (IDirect3DBaseTexture9 *)((CDX8Texture*)pTexDestination)->GetBaseTexture());
	Assert(hr == D3D_OK);

	return true;
#else

	//unsupported !!!!
	Assert(false);

	return true;

#endif
}



void NGRender::DrawFullScreenQuadAs16x9 (float Width, float Height, const ShaderId & id)
{
	LockRes(_FL_, resource_creation_sc, "DrawFullScreenQuadAs16x9");

	float fWidthMultipler = 1.0f;
	float fHeightMultipler = (1.0f / ((16.0f / 9.0f) / width_div_height_k));

	if (fHeightMultipler > 1.0f)
	{
		fWidthMultipler = 1.0f / fHeightMultipler;
		fHeightMultipler = 1.0f;
		
	}

	FLOAT HalfPixelSizeX = 1.0f / Width;
	FLOAT HalfPixelSizeY = 1.0f / Height;

	float left = -1.0f - HalfPixelSizeX;
	float right = 1.0f - HalfPixelSizeX;

	float up = -1.0f + HalfPixelSizeY;
	float down = 1.0f + HalfPixelSizeY;

	up *= fHeightMultipler;
	down *= fHeightMultipler;
	left *= fWidthMultipler;
	right *= fWidthMultipler;


	bDisableHacks = true;

#ifdef _XBOX
	PostProcessQuad[0].vPos = Vector4 (    left, up, 0.0f, 1.0f );
	PostProcessQuad[1].vPos = Vector4 ( right,     up, 0.0f, 1.0f );
	PostProcessQuad[2].vPos = Vector4 ( right,   down, 0.0f, 1.0f );
	PostProcessQuad[3].vPos = Vector4 (    left,  down, 0.0f, 1.0f );

	DrawPrimitiveUP(id, PT_QUADLIST, 1, PostProcessQuad, sizeof(QuadVertex));
#else
	PostProcessQuad[0].vPos = Vector4 (    left,  down, 0.0f, 1.0f );
	PostProcessQuad[1].vPos = Vector4 (    left, up, 0.0f, 1.0f );
	PostProcessQuad[2].vPos = Vector4 ( right,   down, 0.0f, 1.0f );
	PostProcessQuad[3].vPos = Vector4 ( right,     up, 0.0f, 1.0f );

	DrawPrimitiveUP(id, PT_TRIANGLESTRIP, 2, PostProcessQuad, sizeof(QuadVertex));
#endif

	bDisableHacks = false;

	UnlockRes(_FL_, resource_creation_sc, "DrawFullScreenQuad");
}

//post process
void NGRender::DrawFullScreenQuad (float Width, float Height, const ShaderId & id)
{
	LockRes(_FL_, resource_creation_sc, "DrawFullScreenQuad");

	//POST_PROCESS_FVF

	FLOAT HalfPixelSizeX = 1.0f / Width;
	FLOAT HalfPixelSizeY = 1.0f / Height;

	float left = -1.0f - HalfPixelSizeX;
	float right = 1.0f - HalfPixelSizeX;

	float up = -1.0f + HalfPixelSizeY;
	float down = 1.0f + HalfPixelSizeY;



	bDisableHacks = true;

#ifdef _XBOX
	PostProcessQuad[0].vPos = Vector4 (    left, up, 0.0f, 1.0f );
	PostProcessQuad[1].vPos = Vector4 ( right,     up, 0.0f, 1.0f );
	PostProcessQuad[2].vPos = Vector4 ( right,   down, 0.0f, 1.0f );
	PostProcessQuad[3].vPos = Vector4 (    left,  down, 0.0f, 1.0f );

	DrawPrimitiveUP(id, PT_QUADLIST, 1, PostProcessQuad, sizeof(QuadVertex));
#else
	PostProcessQuad[0].vPos = Vector4 (    left,  down, 0.0f, 1.0f );
	PostProcessQuad[1].vPos = Vector4 (    left, up, 0.0f, 1.0f );
	PostProcessQuad[2].vPos = Vector4 ( right,   down, 0.0f, 1.0f );
	PostProcessQuad[3].vPos = Vector4 ( right,     up, 0.0f, 1.0f );

	DrawPrimitiveUP(id, PT_TRIANGLESTRIP, 2, PostProcessQuad, sizeof(QuadVertex));
#endif

	bDisableHacks = false;

	UnlockRes(_FL_, resource_creation_sc, "DrawFullScreenQuad");
}

const RENDERVIEWPORT& NGRender::GetFullScreenViewPort_3D ()
{
	return m_FullScreenViewPort_3D;
}

const RENDERVIEWPORT& NGRender::GetFullScreenViewPort_2D ()
{
	return m_FullScreenViewPort_2D;
}


void NGRender::PostProcess ()
{

	AssertCoreThread;

	if (loadingScreenThread != NULL)
	{
		return;
	}

	if (bInsideBeginScene == false)
	{
		return;
	}


	if (!bPostProcessEnabled)
	{
		//api->Trace("Don't do filter due to bPostProcessEnabled");
		return;
	}

	if (bPostProcessAlreadyDone)
	{
		//api->Trace("Don't do filter due to bPostProcessAlreadyDone");
		return;
	}


	IRenderTargetDepth* depth = GetDepthStencil(_FL_);
	bPostProcessAlreadyDone = true;

	//api->Trace("Do filter");

	EnabledFilters.DelAll();
	DWORD i = 0;
	for (i = 0 ; i < aRenderFilters.Size(); i++)
	{
		if (aRenderFilters[i]->IsEnabled())
		{
			EnabledFilters.Add(aRenderFilters[i]);

			//api->Trace("Active filter found");
		}
	}

	if (EnabledFilters.Size() <= 0
#ifdef SHOW_HISTOGRAM_DELETE_ME_BEFORE_RELEASE
		&& !bShowHistogram
#endif
		)
	{
		return;
	}

	EndScene(NULL);

	dword dwPostprocessID = pixBeginEvent(_FL_, "NGRender::PostProcess");

	IRenderTarget* pScreen = GetRenderTarget(_FL_);

	//экран теперь лежит в pTex
	IBaseTexture* pTex = pScreen->AsTexture();


#ifdef _XBOX
	X360RenderDevice* x360 = (X360RenderDevice*)pDevice;
	x360->DisableMSAA();

	//Ставим не MSAA буффер (он станет не MSAA, потому, что DisableMSAA ему не даст, а ресолвиться будет в туже текстуру, что и MSAA)
	SetRenderTarget(RTO_RESTORE_CONTEXT, pScreen, depth);
#endif



	//Запоминаем RT
	PushRenderTarget();

	//Вполняем все фильтры...
	for (i = 0 ; i < EnabledFilters.Size(); i++)
	{
		dword dwEventId = pixBeginEvent(_FL_, "Post filter ---- [%d]", i);

		EnabledFilters[i]->FilterImage(pTex, pScreen);
		pTex = pScreen->AsTexture();

		pixEndEvent(_FL_, dwEventId);
	}

#ifdef SHOW_HISTOGRAM_DELETE_ME_BEFORE_RELEASE
	if (bShowHistogram)
	{
		PushRenderTarget();

		SetRenderTarget(RTO_DONTOCH_CONTEXT, histogram, depth);
		BeginScene();
		Clear(0, null, CLEAR_TARGET, 0x0, 1.0f, 0x0);

		//тут рисовать посторитель гистограммы
		if (histogram_source && histogram_min && histogram_max && _histogram_query)
		{
			for (int step = 0; step < histogram_update_chunk; step++)
			{
				HRESULT r = D3D_OK;

				float min = (((float)(histogramm_bin)-0.5f) / 254.0f);
				float max = (((float)(histogramm_bin)+0.5f) / 254.0f);

				histogram_min->SetFloat(min);
				histogram_max->SetFloat(max);
				histogram_source->SetTexture(pTex);

				_histogram_query->Begin();

				ShaderId id;
				GetShaderId("BuildHistogram", id);
				DrawFullScreenQuad((float)m_Screen3DInfo.dwWidth, (float)m_Screen3DInfo.dwHeight, id);

				_histogram_query->End();



				DWORD pixelsVisible = _histogram_query->GetResult();;
				if (pixelsVisible = 0xFFFFFFFF)
				{
					pixelsVisible = 0;
				}



				/*
				api->Trace("bin : %d, min %f, max %f, pixels %d", histogramm_bin, min, max, pixelsVisible);

				static char tmpName[512];
				crt_snprintf(tmpName, 500, "c:\\histo%08d.dds", histogramm_bin);

				SaveTexture2File(histogram->AsTexture(), tmpName);
				*/


				float histogramValue = (float)pixelsVisible / histogram_totalPixelsInScreen;

				histogram_data[histogramm_bin] = histogramValue;

				histogramm_bin++;
				if (histogramm_bin >= 255)
				{
					histogramm_bin = 0;
				}
			}
		}

		EndScene();

		PopRenderTarget(RTO_RESTORE_CONTEXT);

		PushViewport();

		SetViewport(m_FullScreenViewPort_3D);


		//Темные зоны показываем
		if (api->DebugKeyState('D'))
		{
			histogram_source->SetTexture(pTex);
			ShaderId id;
			GetShaderId("ShowDarkAreas", id);
			DrawFullScreenQuad((float)m_Screen3DInfo.dwWidth, (float)m_Screen3DInfo.dwHeight, id);
		}

		//Светлые зоны показываем
		if (api->DebugKeyState('L'))
		{
			histogram_source->SetTexture(pTex);
			ShaderId id;
			GetShaderId("ShowLightAreas", id);
			DrawFullScreenQuad((float)m_Screen3DInfo.dwWidth, (float)m_Screen3DInfo.dwHeight, id);
		}

		PopViewport();


	} // show histogramm
#endif

	pScreen->Release();

	if (depth)
	{
		depth->Release();
	}

	pixEndEvent(_FL_, dwPostprocessID);


#ifdef _XBOX

	AssertCoreThread;

	if (!stRenderTarget.Size()) 
	{
		api->Trace("DXError: Try to pop RenderTarget, but RenderTarget stack is empty");
		Assert(false)
	}

	//Переключаем в 720p реальные разрешение - пора рисовать HUD и GUI
	x360->SwitchToHUDRender(stRenderTarget.Top().color, stRenderTarget.Top().depth, stRenderTarget.Top().viewPort);

	stRenderTarget.Pop();

#else

	//Восстанавливаем RT
	PopRenderTarget(RTO_RESTORE_CONTEXT_COLORONLY);

#endif

	BeginScene();


}

ITexture* NGRender::GetPostprocessTexture ()
{
	AssertCoreThread;


	//TODO:
	Assert(false);
	return NULL;
}






bool NGRender::CreateDevice ()
{
	pD3D = Direct3DCreate9( D3D_SDK_VERSION );

	if (!pD3D)
	{
		MakeError(ERR_D3D);
		return false;
	}



	ZERO(d3dpp);

	d3dpp.BackBufferWidth = GetScreenInfo2D().dwWidth;
	d3dpp.BackBufferHeight = GetScreenInfo2D().dwHeight;
	d3dpp.BackBufferFormat = FormatToDX (GetScreenInfo2D().BackBufferFormat);
	d3dpp.BackBufferCount = 1;
	d3dpp.hDeviceWindow = (HWND)api->Storage().GetLong("system.hwnd");
	d3dpp.Windowed = GetScreenInfo2D().bWindowed;
	d3dpp.EnableAutoDepthStencil = true;
	d3dpp.AutoDepthStencilFormat = FormatToDX(GetScreenInfo2D().StencilFormat);

#ifndef _XBOX
	if (GetScreenInfo2D().bWindowed)
	{
		D3DDISPLAYMODE d3ddm;
		if (pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm) != D3D_OK)
		{
			MakeError(ERR_DEVICE);
			return false;
		}

		d3dpp.BackBufferFormat = d3ddm.Format;
		d3dpp.AutoDepthStencilFormat = (d3dpp.BackBufferFormat == D3DFMT_R5G6B5) ? D3DFMT_D16 : D3DFMT_D24S8;
		m_Screen3DInfo.StencilFormat = FormatFromDX (d3dpp.AutoDepthStencilFormat);
		if(d3ddm.Width < m_Screen3DInfo.dwWidth || d3ddm.Height < m_Screen3DInfo.dwHeight) 
		{
			d3dpp.BackBufferWidth = d3ddm.Width;
			d3dpp.BackBufferHeight = d3ddm.Height;
			m_Screen3DInfo.dwWidth = d3ddm.Width;
			m_Screen3DInfo.dwHeight = d3ddm.Height;
		}
	}
#endif

	d3dpp.MultiSampleQuality = 0;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;

	switch (multiSample)
	{
	case 2:
		d3dpp.MultiSampleType = D3DMULTISAMPLE_2_SAMPLES;
		api->Trace("Multisample 2x");
		break;
	case 4:
		d3dpp.MultiSampleType = D3DMULTISAMPLE_4_SAMPLES;
		api->Trace("Multisample 4x");
		break;
#ifndef _XBOX
	case 8:
		d3dpp.MultiSampleType = D3DMULTISAMPLE_8_SAMPLES;
		api->Trace("Multisample 8x");
		break;
#endif
	default:
		api->Trace("Multisample disabled");
	}



	d3dpp.Flags = 0;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.FullScreen_RefreshRateInHz = 0;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	if(!GetScreenInfo2D().bWindowed)
	{
		d3dpp.FullScreen_RefreshRateInHz = 0;
	}


#ifndef _XBOX
	pDevice = NEW DX9RenderDevice(bMaximize2FullScreen, bExpand2FullScreen);
	if (!pDevice->Create(pD3D, d3dpp, deviceIndex, m_Screen2DInfo, m_Screen3DInfo))
	{
		MakeError(ERR_DEVICE);
		DELETE(pDevice);
		RELEASE(pD3D);
		return false;
	}
#else
	pDevice = NEW X360RenderDevice;
	if (!pDevice->Create(pD3D, d3dpp, deviceIndex, m_Screen2DInfo, m_Screen3DInfo))
	{
		MakeError(ERR_DEVICE);
		DELETE(pDevice);
		RELEASE(pD3D);
		return false;
	}

	/* just for test
	d3dpp.BackBufferHeight = 480;
	d3dpp.BackBufferWidth = 640;

	pDevice = NEW DX9RenderDevice;
	if (!pDevice->Create(pD3D, d3dpp))
	{
	DELETE(pDevice);
	RELEASE(pD3D);
	return false;
	}
	*/

#endif


	//api->Trace("ForceWideScreen = %d (Force 16:9 mode, independent from resolution)", bForceWideScreen);

	if (bForceWideScreen < 0)
	{
		api->Trace("Render WIDE SCREEN mode = AutoDetect");
	} else
	{
		/*
		if (bForceWideScreen == ASPECT_ALWAYS_SQUARE_PIXELS)
		{
		api->Trace("Render WIDE SCREEN mode = Always square pixels");
		aspect = ASPECT_ALWAYS_SQUARE_PIXELS;
		}
		*/

		if (bForceWideScreen == ASPECT_4_TO_3)
		{
			api->Trace("Render WIDE SCREEN mode = Force 4:3");
			aspect = ASPECT_4_TO_3;
		}

		if (bForceWideScreen == ASPECT_16_TO_9)
		{
			api->Trace("Render WIDE SCREEN mode = Force 16:9");
			aspect = ASPECT_16_TO_9;
		}

		if (bForceWideScreen == ASPECT_16_TO_10)
		{
			api->Trace("Render WIDE SCREEN mode = Force 16:10");
			aspect = ASPECT_16_TO_10;
		}

		if (bForceWideScreen == ASPECT_5_TO_4)
		{
			api->Trace("Render WIDE SCREEN mode = Force 5:4");
			aspect = ASPECT_5_TO_4;
		}

		if (bForceWideScreen == ASPECT_3_TO_2)
		{
			api->Trace("Render WIDE SCREEN mode = Force 3:2");
			aspect = ASPECT_3_TO_2;
		}

		/*
		if (bForceWideScreen == ASPECT_18_TO_10)
		{
		api->Trace("Render WIDE SCREEN mode = Force 18:10");
		aspect = ASPECT_18_TO_10;
		}

		if (bForceWideScreen == ASPECT_16_TO_5)
		{
		api->Trace("Render WIDE SCREEN mode = Force 16:5");
		aspect = ASPECT_16_TO_5;
		}

		if (bForceWideScreen == ASPECT_25_TO_16)
		{
		api->Trace("Render WIDE SCREEN mode = Force 25:16");
		aspect = ASPECT_25_TO_16;
		}

		if (bForceWideScreen == ASPECT_2_TO_1)
		{
		api->Trace("Render WIDE SCREEN mode = Force 2:1");
		aspect = ASPECT_2_TO_1;
		}

		if (bForceWideScreen == ASPECT_1_TO_1)
		{
		api->Trace("Render WIDE SCREEN mode = Force 1:1");
		aspect = ASPECT_1_TO_1;
		}

		if (bForceWideScreen == ASPECT_29_TO_10)
		{
		api->Trace("Render WIDE SCREEN mode = Force 29:10");
		aspect = ASPECT_29_TO_10;
		}
		*/


	}



	float fAspectScreenW = (float)GetScreenInfo2D().dwWidth;
	float fAspectScreenH = (float)GetScreenInfo2D().dwHeight;


	if (bForceWideScreen < 0)
	{
		//autodetect
		float divider = (fAspectScreenH / fAspectScreenW);

		//Если не определился aspect, то 1:1 включаем...
		aspect = ASPECT_ALWAYS_SQUARE_PIXELS;

		if (divider > ((3.0f/4.0f)-0.01f) && divider < ((3.0f/4.0f)+0.01f))
		{
			api->Trace("AUTODETECTED 4:3");
			aspect = ASPECT_4_TO_3;
		}

		if (divider > ((9.0f/16.0f)-0.01f) && divider < ((9.0f/16.0f)+0.01f))
		{
			api->Trace("AUTODETECTED 16:9");
			aspect = ASPECT_16_TO_9;
		}

		if (divider > ((10.0f/16.0f)-0.01f) && divider < ((10.0f/16.0f)+0.01f))
		{
			api->Trace("AUTODETECTED 16:10");
			aspect = ASPECT_16_TO_10;
		}

		if (divider > ((4.0f/5.0f)-0.01f) && divider < ((4.0f/5.0f)+0.01f))
		{
			api->Trace("AUTODETECTED 5:4");
			aspect = ASPECT_5_TO_4;
		}

		if (divider > ((2.0f/3.0f)-0.01f) && divider < ((2.0f/3.0f)+0.01f))
		{
			api->Trace("AUTODETECTED 3:2");
			aspect = ASPECT_3_TO_2;
		}

		if (aspect == ASPECT_ALWAYS_SQUARE_PIXELS)
		{
			api->Trace("AUTODETECT FAILED, USE 1:1");
		}

		/*
		if (divider > ((10.0f/18.0f)-0.01f) && divider < ((10.0f/18.0f)+0.01f))
		{
		aspect = ASPECT_18_TO_10;
		}

		if (divider > ((5.0f/16.0f)-0.01f) && divider < ((5.0f/16.0f)+0.01f))
		{
		aspect = ASPECT_16_TO_5;
		}

		if (divider > ((16.0f/25.0f)-0.01f) && divider < ((16.0f/25.0f)+0.01f))
		{
		aspect = ASPECT_25_TO_16;
		}

		if (divider > ((1.0f/2.0f)-0.01f) && divider < ((1.0f/2.0f)+0.01f))
		{
		aspect = ASPECT_2_TO_1;
		}

		if (divider > ((1.0f/1.0f)-0.01f) && divider < ((1.0f/1.0f)+0.01f))
		{
		aspect = ASPECT_1_TO_1;
		}

		if (divider > ((10.0f/29.0f)-0.01f) && divider < ((10.0f/29.0f)+0.01f))
		{
		aspect = ASPECT_29_TO_10;
		}
		*/


	}



	fWidthAspectMultipler = (float)pRS->GetScreenInfo2D().dwHeight / (float)pRS->GetScreenInfo2D().dwWidth;

	api->Trace("[%d] Screen info %dx%d", aspect, pRS->GetScreenInfo2D().dwWidth, pRS->GetScreenInfo2D().dwHeight);

	switch (aspect)
	{

	case ASPECT_ALWAYS_SQUARE_PIXELS:
		{
			//autodetect
			if (bForceWideScreen < 0)
			{

				width_div_height_k = (float)pRS->GetScreenInfo2D().dwWidth / (float)pRS->GetScreenInfo2D().dwHeight;
				fWidthAspectMultipler = 1.0f;


			} else
			{
				fWidthAspectMultipler = 1.0f;
				width_div_height_k = 1.0f;
			}

			break;
		}



	case ASPECT_4_TO_3:
		fWidthAspectMultipler = ((3.0f / 4.0f) / fWidthAspectMultipler);
		width_div_height_k = 4.0f / 3.0f;
		break;

	case ASPECT_16_TO_9:
		fWidthAspectMultipler = ((9.0f / 16.0f) / fWidthAspectMultipler);
		width_div_height_k = 16.0f / 9.0f;
		break;

	case ASPECT_16_TO_10:
		fWidthAspectMultipler = ((10.0f / 16.0f) / fWidthAspectMultipler);
		width_div_height_k = 16.0f / 10.0f;
		break;

	case ASPECT_5_TO_4:
		fWidthAspectMultipler = ((4.0f / 5.0f) / fWidthAspectMultipler);
		width_div_height_k = 5.0f / 4.0f;
		break;

	case ASPECT_3_TO_2:
		fWidthAspectMultipler = ((2.0f / 3.0f) / fWidthAspectMultipler);
		width_div_height_k = 3.0f / 2.0f;
		break;

		/*
		case ASPECT_18_TO_10:
		fWidthAspectMultipler = ((10.0f / 18.0f) / fWidthAspectMultipler);
		//calculate me !!!!
		fFOVAspectMultipler = 1.0f;
		width_div_height_k = 18.0f / 10.0f;
		break;

		case ASPECT_16_TO_5:
		fWidthAspectMultipler = ((5.0f / 16.0f) / fWidthAspectMultipler);
		//calculate me !!!!
		fFOVAspectMultipler = 1.0f;
		width_div_height_k = 16.0f / 5.0f;
		break;

		case ASPECT_25_TO_16:
		fWidthAspectMultipler = ((16.0f / 25.0f) / fWidthAspectMultipler);
		//calculate me !!!!
		fFOVAspectMultipler = 1.0f;
		width_div_height_k = 25.0f / 16.0f;
		break;

		case ASPECT_2_TO_1:
		fWidthAspectMultipler = ((1.0f / 2.0f) / fWidthAspectMultipler);
		//calculate me !!!!
		fFOVAspectMultipler = 1.0f;
		width_div_height_k = 2.0f / 1.0f;
		break;

		case ASPECT_1_TO_1:
		fWidthAspectMultipler = ((1.0f / 1.0f) / fWidthAspectMultipler);
		//calculate me !!!!
		fFOVAspectMultipler = 1.0f;
		width_div_height_k = 1.0f / 1.0f;
		break;

		case ASPECT_29_TO_10:
		//2880x990
		fWidthAspectMultipler = ((9.90f / 28.80f) / fWidthAspectMultipler);
		//calculate me !!!!

		//надо еще побольше вверх на 0.2
		fFOVAspectMultipler = 1.56f;
		width_div_height_k = 29.0f / 10.0f;
		break;
		*/

	default:
		//4:3 по умолчанию
		fWidthAspectMultipler = ((3.0f / 4.0f) / fWidthAspectMultipler);
		width_div_height_k = 4.0f / 3.0f;
	}

	api->Trace("Aspect width multipler %f", fWidthAspectMultipler);

	api->Trace("---------------------------------------------------------------------------------\n");


	return true;
}


bool NGRender::ReleaseResource(IResource * pResource)
{
	AssertCoreThread;



	switch (pResource->GetType())
	{
	case DX8TYPE_TEXTURE:
		{
			aTextures.Del((CDX8Texture*)pResource);
		}
		break;
	case DX8TYPE_VERTEXBUFFER:
		{
			aVBuffers.Del((IVBuffer*)pResource);
		}
		break;
	case DX8TYPE_INDEXBUFFER:
		aIBuffers.Del((CDX8IBuffer*)pResource);
		break;
	case DX8TYPE_FONT:
		//pFonts->Release((IFont*)pResource);
		break;
	case DX8TYPE_ADVANCED_FONT:
		//pAdvFonts->Release((IFont*)pResource);
		break;

	}
	return true;

}

dword NGRender::GetTextureDegradation()
{
	AssertCoreThread;


	return dwTextureDegradation;
}

bool NGRender::IsMipMapsFillColor ()
{
	AssertCoreThread;


	return bUseMipFillColor;
}






IVariable* NGRender::GetTechniqueGlobalVariable(const char * pVariableName, const char * pFilename, long iLine)
{
	AssertCoreThread;

	IVariable* var = m_ShaderManager.getShaderVariable (pVariableName, pFilename, iLine);
	return var;
}


void NGRender::CreateSphere()
{
#define CalcKColor(ind) {kColor = light | !Vector(pSphereVerts[t*3 + ind].v.x, pSphereVerts[t*3 + ind].v.y, pSphereVerts[t*3 + ind].v.z); if(kColor < 0.0f) kColor = 0.0f; }
#define CLerp(c, min) (dword(c*(kColor*(1.0f - min) + min)))
#define Color ((CLerp(255.0f, 0.5f) << 24) | (CLerp(255.0f, 0.7f) << 16) | (CLerp(255.0f, 0.7f) << 8) | (CLerp(255.0f, 0.7f) << 0));

	const float myPI = 3.1415926535897932f;
	const long a1 = 12;
	const long a2 = (a1/2);

	dwNumSphereTrgs = a1 * a2 * 2;

	pVSphereBuffer = CreateVertexBuffer(sizeof(SphVertex) * dwNumSphereTrgs * 6 * 2, sizeof(SphVertex), _FL_);
	SphVertex * pSphereVerts = (SphVertex*)pVSphereBuffer->Lock();

	Vector light = !Vector(0.0f, 0.0f, 1.0f);
	float kColor;
	//Заполняем вершины
	for(long i = 0, t = 0; i < a2; i++)
	{
		float r1 = sinf(myPI*i/float(a2));
		float y1 = cosf(myPI*i/float(a2));
		float r2 = sinf(myPI*(i + 1)/float(a2));
		float y2 = cosf(myPI*(i + 1)/float(a2));
		for(long j = 0; j < a1; j++)
		{
			float x1 = sinf(2.0f*myPI*j/float(a1));
			float z1 = cosf(2.0f*myPI*j/float(a1));
			float x2 = sinf(2.0f*myPI*(j + 1)/float(a1));
			float z2 = cosf(2.0f*myPI*(j + 1)/float(a1));
			//0
			pSphereVerts[t*3 + 0].v.x = r1*x1;
			pSphereVerts[t*3 + 0].v.y = y1;
			pSphereVerts[t*3 + 0].v.z = r1*z1;
			CalcKColor(0);
			pSphereVerts[t*3 + 0].c = Color;
			//1
			pSphereVerts[t*3 + 1].v.x = r2*x1;
			pSphereVerts[t*3 + 1].v.y = y2;
			pSphereVerts[t*3 + 1].v.z = r2*z1;
			CalcKColor(1);
			pSphereVerts[t*3 + 1].c = Color;
			//2
			pSphereVerts[t*3 + 2].v.x = r1*x2;
			pSphereVerts[t*3 + 2].v.y = y1;
			pSphereVerts[t*3 + 2].v.z = r1*z2;
			CalcKColor(2);
			pSphereVerts[t*3 + 2].c = Color;
			//3 = 2
			pSphereVerts[t*3 + 3] = pSphereVerts[t*3 + 2];
			//4 = 1
			pSphereVerts[t*3 + 4] = pSphereVerts[t*3 + 1];
			//5
			pSphereVerts[t*3 + 5].v.x = r2*x2;
			pSphereVerts[t*3 + 5].v.y = y2;
			pSphereVerts[t*3 + 5].v.z = r2*z2;
			CalcKColor(5);
			pSphereVerts[t*3 + 5].c = Color;
			//Добавили 2 треугольника
			t += 2;
		}
	}
	pVSphereBuffer->Unlock();

#undef CalcKColor
#undef CLerp
#undef Color

}

StateFilter* NGRender::getRedundantStatesFilter()
{
	if (pDevice->DisableStateManager())
	{
		return NULL;
	}

	return redundantStatesFilter;
}







void NGRender::ReadIniParams()
{
	IFileService * fs = (IFileService *)api->GetService("FileService");
	Assert(fs);
	IIniFile * pEngineIni = fs->SystemIni();
	if (!pEngineIni)
	{
		return;
	}


	api->Trace("\nRender parameters:");
	api->Trace("---------------------------------------------------------------------------------");


	winPosX = pEngineIni->GetLong("", "windowPosX", 10);
	winPosY = pEngineIni->GetLong("", "windowPosY", 10);



	bDontShowReloading = pEngineIni->GetLong("Render", "EnableReloadingScreens", 0) == 0;
	api->Trace("DontShowReloading = %d (Don't show reloading screen)", bDontShowReloading);

	waitGPUFrames = pEngineIni->GetLong("Render", "WaitForGPUFrames", -1);
	api->Trace("WaitForGPUFrames = %d (Sync CPU and GPU every [0..3] frame (smothnes but lower FPS) reccomended value = 1, (-1) disable", waitGPUFrames);


	bDrawStats = pEngineIni->GetLong("Render", "PerfGraphs", 0) != 0;
	api->Trace("PerfGraphs = %d (Show performance graphs)", bDrawStats);

	dwTextureDegradation = pEngineIni->GetLong("Render", "texture_degradation", 0);
	api->Trace("texture_degradation = %d (Use smalest mip level)", dwTextureDegradation);

	bUseMipFillColor = pEngineIni->GetLong("Render", "mipfillcolor", 0) == 1;
	api->Trace("mipfillcolor = %d (Show mip maps as different colors)", bUseMipFillColor);

	m_Screen2DInfo.dwWidth = pEngineIni->GetLong("Render", "screen_width", 640);
	m_Screen2DInfo.dwHeight = pEngineIni->GetLong("Render", "screen_height", 480);
	m_Screen2DInfo.fFOV = pEngineIni->GetFloat("Render", "fov", 1.0f);

	m_Screen3DInfo.dwWidth = m_Screen2DInfo.dwWidth;
	m_Screen3DInfo.dwHeight = m_Screen2DInfo.dwHeight;
	m_Screen3DInfo.fFOV = m_Screen2DInfo.fFOV;

	multiSample = pEngineIni->GetLong("Render", "MultiSample", 0);

	deviceIndex = pEngineIni->GetLong("Render", "AdapterIndex", 0);



	bPostProcessEnabled = pEngineIni->GetLong("Render", "PostProcess", 1) != 0;
	api->Trace("PostProcess = %d (Enable fullscreen postprocess effects)", bPostProcessEnabled);


#ifndef _XBOX
	m_Screen2DInfo.bWindowed = pEngineIni->GetLong("Render", "full_screen", 1) == 0;
#else
	m_Screen2DInfo.bWindowed = false;
#endif
	m_Screen3DInfo.bWindowed = m_Screen2DInfo.bWindowed;

	bExpand2FullScreen = pEngineIni->GetLong("Render", "Expand2FullScreen", 0) != 0;
	bMaximize2FullScreen = pEngineIni->GetLong("Render", "maximize2fullscreen", 0) != 0;


	dword r, g, b;
	const char * backColor = pEngineIni->GetString("Render", "BackColor", "0, 0, 128");
	crt_sscanf(backColor, "%d, %d, %d", &r, &g, &b);

	m_backColor = Color(r / 255.0f, g / 255.0f, b / 255.0f, 0.0f);




#ifndef _XBOX
	bForceWideScreen = pEngineIni->GetLong("Render", "WideScreenMode", -1);
#else
	bForceWideScreen = ASPECT_4_TO_3;

	// Get the user video settings.
	XVIDEO_MODE VideoMode; 
	XMemSet( &VideoMode, 0, sizeof(XVIDEO_MODE) ); 
	XGetVideoMode( &VideoMode );
	if (VideoMode.fIsWideScreen)
	{
		bForceWideScreen = ASPECT_16_TO_9;
		api->Trace("Render MODE X360 16:9");
	} else
	{
		bForceWideScreen = ASPECT_4_TO_3;
		api->Trace("Render MODE X360 4:3");
	}

#endif

	RELEASE(pEngineIni);
}




void NGRender::SetStateHack(StateHack state, long state_value)
{
	AssertCoreThread;


	// Vano: работает на XBox
	/*#ifndef _XBOX*/
	if (state_value >= 0)
	{
		hacks[state].enabled = true;
		hacks[state].s = state;
		hacks[state].v = state_value;
	} else
	{
		hacks[state].enabled = false;
		hacks[state].s = state;
		hacks[state].v = state_value;

		if (state == SH_CULLMODE)
		{
			redundantStatesFilter->DirtyState (D3DRS_CULLMODE);
		}
	}
	/*#endif*/
}

long NGRender::GetStateHack(StateHack state)
{
	AssertCoreThread;


	return hacks[state].v;
}

void NGRender::ApplyStateHacks ()
{
	// Vano: работает на XBox
	/*#ifndef _XBOX*/

	LockRes(_FL_, resource_creation_sc, "ApplyStateHacks");


	if (bDisableHacks)
	{
		UnlockRes(_FL_, resource_creation_sc, "ApplyStateHacks");
		return;
	}


	//cull hack
	if (hacks[SH_CULLMODE].enabled)
	{
		dword cullValue = D3DCULL_NONE;
		switch (hacks[SH_CULLMODE].v)
		{
		case SHCM_NONE:
			cullValue = D3DCULL_NONE;
			break;
		case SHCM_CW:
			cullValue = D3DCULL_CW;
			break;
		case SHCM_CCW:
			cullValue = D3DCULL_CCW;
			break;
		case SHCM_INVERSE:
			if (redundantStatesFilter->GetCullState() == D3DCULL_CW) cullValue = D3DCULL_CCW;
			if (redundantStatesFilter->GetCullState() == D3DCULL_CCW) cullValue = D3DCULL_CW;
			break;
		}
		redundantStatesFilter->SetRenderState(D3DRS_CULLMODE, cullValue);
	} 

	/*
	else
	{
	redundantStatesFilter->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	}
	*/


	if (hacks[SH_CLIPPLANEENABLE].enabled)
	{
		redundantStatesFilter->SetRenderState(D3DRS_CLIPPLANEENABLE, hacks[SH_CLIPPLANEENABLE].v);
	}

	if (hacks[SH_SCISSORRECT].enabled)
	{
		redundantStatesFilter->SetRenderState(D3DRS_SCISSORTESTENABLE, hacks[SH_SCISSORRECT].v);
	}

	if (hacks[SH_COLORWRITE].enabled)
	{
		if (hacks[SH_COLORWRITE].v == SHCW_ENABLE)
		{
			redundantStatesFilter->SetRenderState(D3DRS_COLORWRITEENABLE, 0x0000000F);
		} else
		{
			redundantStatesFilter->SetRenderState(D3DRS_COLORWRITEENABLE, 0x0);
		}
	}

	if (hacks[SH_ZWRITE].enabled)
	{
		if (hacks[SH_ZWRITE].v == SHZW_ENABLE)
		{
			redundantStatesFilter->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		} else
		{
			redundantStatesFilter->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
		}
	}


	UnlockRes(_FL_, resource_creation_sc, "ApplyStateHacks");
	/*#endif*/
}

void NGRender::RestoreAfterStateHacks ()
{
	// Vano: работает на XBox
	/*#ifndef _XBOX*/
	LockRes(_FL_, resource_creation_sc, "RestoreAfterStateHacks");

	if (bDisableHacks)
	{
		UnlockRes(_FL_, resource_creation_sc, "RestoreAfterStateHacks");
		return;
	}

	DWORD defValue;

	if (hacks[SH_CULLMODE].enabled)
	{
		if (DefaultRenderStates::GetDefaultValue(D3DRS_CULLMODE, defValue))
		{
			redundantStatesFilter->DirtyState (D3DRS_CULLMODE);
			redundantStatesFilter->SetRenderState(D3DRS_CULLMODE, defValue);
		}
	}





	if (hacks[SH_SCISSORRECT].enabled)
	{
		if (DefaultRenderStates::GetDefaultValue(D3DRS_SCISSORTESTENABLE, defValue))
		{
			redundantStatesFilter->SetRenderState(D3DRS_SCISSORTESTENABLE, defValue);
		}
	}


	if (hacks[SH_CLIPPLANEENABLE].enabled)
	{
		if (DefaultRenderStates::GetDefaultValue(D3DRS_CLIPPLANEENABLE, defValue))
		{
			redundantStatesFilter->SetRenderState(D3DRS_CLIPPLANEENABLE, defValue);
		}
	}

	if (hacks[SH_COLORWRITE].enabled)
	{
		if (DefaultRenderStates::GetDefaultValue(D3DRS_COLORWRITEENABLE, defValue))
		{
			redundantStatesFilter->SetRenderState(D3DRS_COLORWRITEENABLE, defValue);
		}
	}

	if (hacks[SH_ZWRITE].enabled)
	{
		if (DefaultRenderStates::GetDefaultValue(D3DRS_ZWRITEENABLE, defValue))
		{
			redundantStatesFilter->SetRenderState(D3DRS_ZWRITEENABLE, defValue);
		}
	}

	UnlockRes(_FL_, resource_creation_sc, "RestoreAfterStateHacks");
	/*#endif*/
}

void NGRender::SetScissorRect(CONST RECT *pRect)
{
	AssertCoreThread;


	HRESULT h = D3D()->SetScissorRect(pRect);
	Assert(h == D3D_OK);
}


bool NGRender::DrawLoadingScreen ()
{



	DWORD dwTimeValue = GetTickCount();
	float fCurrentTimeValue = dwTimeValue / 1000.0f;

	float fDeltaTime = fCurrentTimeValue - fLoadingTimeValue;
	fLoadingTimeValue = fCurrentTimeValue;

	if (fDeltaTime > 0.7f) fDeltaTime = 0.7f;

	/*
	char text[16384];
	crt_snprintf(text, 10000, "DrawLoadingScreen =======================================================================\n");
	OutputDebugString(text);
	*/

	loading_screen_sc.Enter();
	LockRes(_FL_, resource_creation_sc, "DrawLoadingScreen");

#ifdef _XBOX
	D3D()->AcquireThreadOwnership();
#endif


#ifndef _XBOX
	switch (pDevice->D3D()->TestCooperativeLevel())
	{
	case D3DERR_DEVICENOTRESET:
		//OnResetDevice();
		//OutputDebugString("Threaded acquire focus\n");
		bNeedResetDevice = true;
		break;

	case D3DERR_DEVICELOST: 
		//OutputDebugString("Threaded lost focus\n");
		bCantRender = true;
		bInsideBeginScene = false;
		break;

	}
#endif

#ifdef _XBOX
	X360RenderDevice* x360 = (X360RenderDevice*)pDevice;
	x360->DisableMSAA();
	x360->SwitchToHUDRenderLoadingScreenHack();
#endif


	//цикл рендера рисующий куйню....
	//=======================================================================
	BeginScene();

	//D3D()->Clear(0, null, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x20202020, 1.0, 0);






	D3D()->Clear(0, null, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0x0, 1.0, 0);

#ifdef _XBOX
	D3D()->FlushHiZStencil(D3DFHZS_ASYNCHRONOUS);
#endif

	if (bShowLegalVideos == FALSE)
	{
		if (varTexture && varColor)
		{




			const RENDERSCREEN & rscr = GetScreenInfo3D();
			ShaderId id;

			if (pReloadingTexture)
			{
				varColor->SetVector4(Vector4(1.0f));
				varTexture->SetTexture(pReloadingTexture);

				GetShaderId("ls_PostProcessFillScreen_Texture", id);
				DrawFullScreenQuadAs16x9((float)rscr.dwWidth, (float)rscr.dwHeight, id);
			}




			//----------- рисуем стрелочку компаса -------------------------

			//размер спрайта стрелки в виртуальных координатах в 1280x720
			const float ArrowSpriteSize = 168.0f;

			//размер спрайта подложки компаса в виртуальных координатах в 1280x720
			const float BackKompassSpriteSize = 170.0f;


			float fWidthMultipler = 1.0f;
			float fHeightMultipler = (1.0f / ((16.0f / 9.0f) / width_div_height_k));
			if (fHeightMultipler > 1.0f)
			{
				fWidthMultipler = 1.0f / fHeightMultipler;
				fHeightMultipler = 1.0f;
			}


			//float fHeightMultipler = (1.0f / ((16.0f / 9.0f) / width_div_height_k));

			float HalfPixelSizeX = 1.0f / (float)rscr.dwWidth;
			float HalfPixelSizeY = 1.0f / (float)rscr.dwHeight;

			//Находим границы экрана для рисования...
			float screen_left = (-1.0f - HalfPixelSizeX) * fWidthMultipler;
			float screen_right = (1.0f - HalfPixelSizeX) * fWidthMultipler;
			float screen_up = (-1.0f + HalfPixelSizeY) * fHeightMultipler;
			float screen_down = (1.0f + HalfPixelSizeY) * fHeightMultipler;


			float fSpriteSize = (ArrowSpriteSize / 720.0f);

			const float shadowOffset = 5.0f;

			float fShadowSpriteCenterX = (shadowOffset + 1135.5f) / 1280.0f;
			float fShadowSpriteCenterY = (shadowOffset + 480.5f) / 720.0f;
			float fShadowCenterX = screen_left + ((screen_right - screen_left) * fShadowSpriteCenterX);
			float fShadowCenterY = screen_up + ((screen_down - screen_up) * (1.0f - fShadowSpriteCenterY));


			float fSpriteCenterX = 1135.5f / 1280.0f;
			float fSpriteCenterY = 480.5f / 720.0f;
			float fCenterX = screen_left + ((screen_right - screen_left) * fSpriteCenterX);
			float fCenterY = screen_up + ((screen_down - screen_up) * (1.0f - fSpriteCenterY));

			float fSpriteNSize = ((screen_down - screen_up) * fSpriteSize);

			QuadVertex spr[4];

			float srcAngle = (fCompassArrowAngle * 0.5f);
			float fRotoAngle = 0.5f * (sinf(fCompassArrowAngle) * cosf(fCompassArrowAngle * 4.0f));

			Matrix mRotateZ;
			mRotateZ.BuildRotateZ(4.398f + fRotoAngle);

			float fSquareK = width_div_height_k;//(float)rscr.dwWidth / (float)rscr.dwHeight;
			float fInvSquareK = 1.0f / fSquareK;

			float left = -(fSpriteNSize*0.5f);
			float right = (fSpriteNSize*0.5f);
			float up = -(fSpriteNSize*0.5f);
			float down = (fSpriteNSize*0.5f);

			spr[0].vPos = Vector4 (    left,  down, 0.0f, 1.0f );
			spr[1].vPos = Vector4 (    left, up, 0.0f, 1.0f );
			spr[2].vPos = Vector4 ( right,   down, 0.0f, 1.0f );
			spr[3].vPos = Vector4 ( right,     up, 0.0f, 1.0f );

			spr[0].vPos.v = spr[0].vPos.v * mRotateZ;
			spr[1].vPos.v = spr[1].vPos.v * mRotateZ;
			spr[2].vPos.v = spr[2].vPos.v * mRotateZ;
			spr[3].vPos.v = spr[3].vPos.v * mRotateZ;

			spr[0].vPos.v *= Vector(fInvSquareK, 1.0f, 0.0f);
			spr[1].vPos.v *= Vector(fInvSquareK, 1.0f, 0.0f);
			spr[2].vPos.v *= Vector(fInvSquareK, 1.0f, 0.0f);
			spr[3].vPos.v *= Vector(fInvSquareK, 1.0f, 0.0f);

			spr[0].vPos.v += Vector(fShadowCenterX, fShadowCenterY, 0.0f);
			spr[1].vPos.v += Vector(fShadowCenterX, fShadowCenterY, 0.0f);
			spr[2].vPos.v += Vector(fShadowCenterX, fShadowCenterY, 0.0f);
			spr[3].vPos.v += Vector(fShadowCenterX, fShadowCenterY, 0.0f);




			float fNearV = 1.0f;
			float fFarV = 0.0f;
			float fNearU = 0.0f;
			float fFarU = 1.0f;

			spr[0].v0 = fFarV;
			spr[0].u0 = fNearU; 

			spr[1].v0 = fNearV;
			spr[1].u0 = fNearU; 

			spr[2].v0 = fFarV;
			spr[2].u0 = fFarU; 

			spr[3].v0 = fNearV;
			spr[3].u0 = fFarU; 



			fCompassArrowAngle += fDeltaTime;
			//------------------------------------------------------------------


			int angleProgress = (int)(Clampf ((loading_percents / loading_percents_total)) * 360.0f);

			//angleProgress = 180;


			const int alphaTailSize = 30;



			/*#ifdef STOP_DEBUG*/
			static QuadVertex radialProgress[360 + 2 + 1 + alphaTailSize + alphaTailSize];
			/*
			#else
			array<QuadVertex> radialProgress(_FL_);
			radialProgress.AddElements(360 + 2 + 1 + alphaTailSize + alphaTailSize);
			#endif
			*/


			float fRadialSpriteSize = (BackKompassSpriteSize / 720.0f);
			float fSpriteRadialNSize = (((screen_down - screen_up) * fRadialSpriteSize) * 0.5f);

			float fCenterAlpha = 0.5f;

			radialProgress[0].vPos = Vector4 (0.0f, 0.0f, 0.0f, fCenterAlpha);
			radialProgress[0].vPos.v *= Vector(1.0f, fSquareK, 0.0f);
			radialProgress[0].vPos.v += Vector(fCenterX, fCenterY, 0.0f);
			radialProgress[0].v0 = 0.5f;
			radialProgress[0].u0 = 0.5f; 

			angleProgress++;


			float fAlpha = 1.0f;

			for (int i = 0-alphaTailSize; i <= (angleProgress+alphaTailSize); i++)
			{
				fAlpha = 1.0f;

				if (i < 0)
				{
					fAlpha = 1.0f - (-(float)i / (float)alphaTailSize);
				}

				if (i > angleProgress)
				{
					fAlpha = 1.0f - ((i-angleProgress) / (float)alphaTailSize);
				}


				int idx = i + 1 + alphaTailSize;

				mRotateZ.BuildRotateZ(Deg2Rad((float)-i));
				radialProgress[idx].vPos = Vector4 (0.0f, fSpriteRadialNSize, 0.0f, fAlpha);
				radialProgress[idx].vPos.v = radialProgress[idx].vPos.v * mRotateZ;

				radialProgress[idx].u0 = /*1.0f -*/ (0.5f + ((radialProgress[idx].vPos.v.x / fSpriteRadialNSize) * 0.5f));
				radialProgress[idx].v0 = 1.0f - (0.5f + ((radialProgress[idx].vPos.v.y / fSpriteRadialNSize) * 0.5f));

				radialProgress[idx].vPos.v *= Vector(fInvSquareK, 1.0f, 0.0f);
				radialProgress[idx].vPos.v += Vector(fCenterX, fCenterY, 0.0f);
			}



			varColor->SetVector4(Vector4(1.0f));
			varTexture->SetTexture(pTextureProgress);
			//varTexture->ResetTexture();


			bDisableHacks = true;

			int polyCount = angleProgress+alphaTailSize+alphaTailSize - 1;

			if (polyCount > 0)
			{
				GetShaderId("ls_PostProcessFillScreen_TextureVAlpha", id);
				DrawPrimitiveUP(id, PT_TRIANGLEFAN, polyCount, &radialProgress[0], sizeof(QuadVertex));
			}
			bDisableHacks = false;


			//------------- рисуем стрелочку физически -------------------------------------------


			varColor->SetVector4(Vector4(0.0f, 0.0f, 0.0f, 0.4f));
			varTexture->SetTexture(pTextureArrow);
			//varTexture->ResetTexture();


			bDisableHacks = true;
			GetShaderId("ls_PostProcessFillScreen_Texture", id);
			DrawPrimitiveUP(id, PT_TRIANGLESTRIP, 2, spr, sizeof(QuadVertex));
			bDisableHacks = false;




			spr[0].vPos.v -= Vector(fShadowCenterX, fShadowCenterY, 0.0f);
			spr[1].vPos.v -= Vector(fShadowCenterX, fShadowCenterY, 0.0f);
			spr[2].vPos.v -= Vector(fShadowCenterX, fShadowCenterY, 0.0f);
			spr[3].vPos.v -= Vector(fShadowCenterX, fShadowCenterY, 0.0f);

			spr[0].vPos.v += Vector(fCenterX, fCenterY, 0.0f);
			spr[1].vPos.v += Vector(fCenterX, fCenterY, 0.0f);
			spr[2].vPos.v += Vector(fCenterX, fCenterY, 0.0f);
			spr[3].vPos.v += Vector(fCenterX, fCenterY, 0.0f);



			varColor->SetVector4(Vector4(1.0f));
			varTexture->SetTexture(pTextureArrow);


			bDisableHacks = true;

			GetShaderId("ls_PostProcessFillScreen_Texture", id);
			DrawPrimitiveUP(id, PT_TRIANGLESTRIP, 2, spr, sizeof(QuadVertex));
			bDisableHacks = false;



			//------------- рисуем хинт -------------------------------------------
			DWORD dwTotalHints = loadingHints.Size();
			if (pAdvLoaderFont && dwTotalHints > 0)
			{
				pAdvLoaderFont->SetKerning(-7.0f);


				//на виртуальном экране 1280x720 координаты - 218.0f, 555.0f, 1062.0f, 639.0f

				//x 17 y 77 width 66 height 11.6

				//Подсказку рисуем

				//float fHeightMultipler = (1.0f / ((16.0f / 9.0f) / width_div_height_k));
				float fWidthMultipler = 1.0f;
				float fHeightMultipler = (1.0f / ((16.0f / 9.0f) / width_div_height_k));
				if (fHeightMultipler > 1.0f)
				{
					fWidthMultipler = 1.0f / fHeightMultipler;
					fHeightMultipler = 1.0f;
				}



				float height_min = -100.0f * fHeightMultipler;
				height_min = (height_min * 0.5f) + 50.0f;

				float height_max = 100.0f * fHeightMultipler;
				height_max = (height_max * 0.5f) + 50.0f;

				float hght = (height_max - height_min) / 100.0f;

//
				float width_l = -100.0f * fWidthMultipler;
				width_l = (width_l * 0.5f) + 50.0f;

				float width_r = 100.0f * fWidthMultipler;
				width_r = (width_r * 0.5f) + 50.0f;

				float wdht = (width_r - width_l) / 100.0f;
//


				float p_top = height_min + (hght * 77.0f);
				float p_height = (hght * 11.6f);

				float p_left = width_l + (wdht * 17.0f);
				float p_right = (wdht * 66.0f);


				pAdvLoaderFont->SetScale(fLoaderFontScale);


				pAdvLoaderFont->Print(false, p_left/*17.0f*/, p_top, p_right/*66.0f*/, p_height, loadingHints[activeHintID].c_str(), IAdvFont::Center, IAdvFont::Center, false, true);


				p_top = height_min + (hght * 56.0f);
				p_height = (hght * 11.0f);

				pAdvLoaderFont->Print(false, 17.0f, p_top, 66.0f, p_height, loadingMessage.c_str(), IAdvFont::Center, IAdvFont::Center, false, true);


				pAdvLoaderFont->SetKerning(0.0f);

				activeHintTime += fDeltaTime;

				if (activeHintTime >= HINT_CHANGE_TIME)
				{
					DWORD dwNewHintID = rand () % dwTotalHints;
					if (dwNewHintID == activeHintID)
					{
						dwNewHintID = (dwNewHintID + 1) % dwTotalHints;
					}

					activeHintID = dwNewHintID;

					activeHintTime = 0.0f;
				}

			}


		}

	} else
	{
		x360_vidArray.Enter();


		bool b1 = x360_played_legal->IsProcessed();
		bool b2 = x360_played_legal->Frame(false);

		if (!b1 || !b2)
		{
			legalVideos[dwLegalVideoIdx]->Stop();


			dwLegalVideoIdx++;
			if (dwLegalVideoIdx >= legalVideos.Size())
			{
				x360_played_legal = NULL;
				SetEvent( evtLegalVideosDone );

				bShowLegalVideos = false;
			} else
			{
				x360_played_legal = legalVideos[dwLegalVideoIdx];
				x360_played_legal->Start();
			}
		}

		x360_vidArray.Leave();

	}





	// Вано: Подкручиваем гамму
	//GammaCorrection();

	EndScene(NULL);
	pDevice->Present();


	CheckAvailMemory();

#ifdef _XBOX
	D3D()->ReleaseThreadOwnership();
#endif

	UnlockRes(_FL_, resource_creation_sc, "DrawLoadingScreen");

#ifdef _XBOX
	MEMORYSTATUS memStatus;
	GlobalMemoryStatus(&memStatus);
	dword s = dword(memStatus.dwTotalPhys - memStatus.dwAvailPhys);


	PIXAddNamedCounter(((float)memStatus.dwTotalPhys) / 1024.0f / 1024.0f, "Memory : total phys MB");
	PIXAddNamedCounter(((float)memStatus.dwAvailPhys) / 1024.0f / 1024.0f, "Memory : avail phys MB");
	PIXAddNamedCounter(((float)s) / 1024.0f / 1024.0f, "Memory : used MB");

#endif





	//=======================================================================
	//цикл рендера рисующий куйню....




	if (IsDeviceLost())
	{
		api->Trace("Device lost - exiting from loading screen\n");
		loading_screen_sc.Leave();
		return true;
	}


	loading_screen_sc.Leave();

	//Если пора выходить или девай потерян - все равно не сможем рисовать ничего, то на выход...
	if (_bNeedCloseThread != atomicI32::False)
	{
		return true;
	}

	if (_finalClean != atomicI32::False)
	{
		return true;
	}

	if (bShowLegalVideos == FALSE)
	{
		Sleep(50);
	} else
	{
		Sleep(30);
	}


	/*
	processMessages - надо....
	//------------------------------------
	MSG msg;
	while (PeekMessage (&msg, 0, 0, 0, PM_REMOVE))
	{
	DefWindowProc(hwnd,iMsg,wParam,lParam);
	//TranslateMessage(&msg);
	//DispatchMessage(&msg);
	}
	//------------------------------------
	*/

	return false;
}


bool NGRender::IsLoadingScreenEnabled()
{
	return (loadingScreenThread != NULL);
}


void NGRender::EnableLoadingScreen (bool bEnable, const char * loadingScreenTexture)
{
	if (bDontShowReloading) return;
	//return;

	if (bInsideBeginScene && bEnable)
	{
		//Если были внутри сцены, завершить надо текущую...
		EndFrame(api->GetDeltaTime());
	}


	if (_finalClean != atomicI32::False)
	{
		return;
	}

	/*
	if (bInsideFrame && bEnable == false)
	{
	EndFrame(0.01f);
	}
	*/


	/*
	if (bEnable)
	{
	OutputDebugString("NGRender::EnableLoadingScreen\n");
	} else
	{
	OutputDebugString("NGRender::DisableLoadingScreen\n");
	}
	*/        
	//api->Trace("NGRender::EnableLoadingScreen(%d)\n", bEnable);

	if (loadingScreenTexture)
	{
		//
		api->Trace("NGRender::EnableLoadingScreen('%s')\n", loadingScreenTexture);
		SetLoadingImage(loadingScreenTexture);
	}

	api->Trace("NGRender::EnableLoadingScreen - %d from thread 0x%08X \n", bEnable, ::GetCurrentThreadId());
	api->Trace("0x%08X core thread\n", api->GetThreadId());





	if (!bEnable)
	{
		WaitForSingleObject(evtLegalVideosDone, INFINITE);

		UnloadLegalVideos();

		//Два раза не пытаемся выключить
		if (loadingScreenThread == NULL) return;

		//Если видео не доигрались надо что то делать...
		//UnloadLegalVideos();

		//на выход сигнал
		_bNeedCloseThread = atomicI32::True;

		WaitForSingleObject(loadingScreenThread, INFINITE);


		CloseHandle(loadingScreenThread);
		loadingScreenThread = NULL;

		//D3D()->Resume();
#ifdef _XBOX
		D3D()->AcquireThreadOwnership();
#endif



		//Начинаем новую сцену...
		if (bNeedBeginScene_AfterExit)
		{
			//OutputDebugString("NGRender::DisableLoadingScreen - executing BeginScene\n");
			BeginScene();
		}


	} else
	{
		//Два раза не пытаемся включить
		if (loadingScreenThread)
		{
#ifdef _XBOX
			OutputDebugString("EnableLoadingScreen...early exit\n");
#endif
			return;
		}

		if (bNeedToStartVideoPlayer)
		{
			x360_played_legal->Start();                  
			bNeedToStartVideoPlayer = FALSE;
		}


		loading_screen_sc.Enter();

		getAdvancedFontsManager()->Refresh(pAdvLoaderFont);

		//fCompassArrowAngle = 0.0f;


		DWORD dwTimeValue = GetTickCount();
		fLoadingTimeValue = dwTimeValue / 1000.0f;

		bNeedBeginScene_AfterExit = bInsideBeginScene;
		//Заканчиваем старую сцену...
		if (bInsideBeginScene)
		{
			//OutputDebugString("NGRender::EnableLoadingScreen - executing EndScene\n");
			EndScene(NULL);
		}


		_bNeedCloseThread = atomicI32::False;
		m_ShaderManager.ResetAllTextureVariables();

		const RENDERSCREEN & rs = NGRender::pRS->GetScreenInfo2D();
		RENDERVIEWPORT vp2D;
		vp2D.X = 0;
		vp2D.Y = 0;
		vp2D.Width = rs.dwWidth;
		vp2D.Height = rs.dwHeight;
		vp2D.MinZ = 0.0f;
		vp2D.MaxZ = 1.0f;
		NGRender::pRS->SetViewport(vp2D);

		BuildHintsList();

		//D3D()->Suspend();


#ifdef _XBOX
		DWORD dwID = D3D()->QueryThreadOwnership();
		D3D()->ReleaseThreadOwnership();
#endif

		//начало потока сигнал
		loadingScreenThread = CreateThread(null, 0, (LPTHREAD_START_ROUTINE)NGRender::ThreadExecute, this, CREATE_SUSPENDED, &LoadingScreenThreadId);
		SetThreadPriority(loadingScreenThread, THREAD_PRIORITY_BELOW_NORMAL);

#ifdef _XBOX
		XSetThreadProcessor( loadingScreenThread, 2 );
#endif

		XSetThreadName(LoadingScreenThreadId, "Render::LoadingScreen");
		ResumeThread(loadingScreenThread);

		loading_screen_sc.Leave();
	}

#ifdef _XBOX
	OutputDebugString("EnableLoadingScreen...done\n");
#endif


}


bool NGRender::GetShaderId (const char* shaderName, ShaderId & id)
{
	LockRes(_FL_, resource_creation_sc, "GetShaderId");

	id = (ShaderId)m_ShaderManager.FindShaderByName(shaderName);

	if (id == NULL)
	{
		api->Trace("Render Error: Can't find shader '%s'", shaderName);
	}

	UnlockRes(_FL_, resource_creation_sc, "GetShaderId");



	return (id != NULL);
}

bool NGRender::GetShaderLightingId (const char* shaderName, ShaderLightingId & id)
{
	LockRes(_FL_, resource_creation_sc, "GetShaderLightingId");

	bool bError = false;

	static char TechniqueRealName[8192];
	const char* lights_postfix_array[] = { "_01", "_02", "_03", "_04", "_05", "_06" };


	id.off = (ShaderId)m_ShaderManager.FindShaderByName(shaderName);

	crt_strcpy(TechniqueRealName, ARRSIZE(TechniqueRealName), "LIGHT_");
	crt_strcat(TechniqueRealName, ARRSIZE(TechniqueRealName), shaderName);
	id.on = (ShaderId)m_ShaderManager.FindShaderByName(TechniqueRealName);

	if (!id.on)
	{
		bError = true;
	}

	UnlockRes(_FL_, resource_creation_sc, "GetShaderLightingId");

	return !bError;
}

/*
void NGRender::ShowStartupScreen(dword dwLocStringID, const char * englishStringID, const char * szAdditionalMessage)
{



if (pAdvLoaderFont == NULL)
{
//Компилируются системные шейдера похоже, не надо еще писать ничего...
return;
}

//      getAdvancedFontsManager()->Refresh(pAdvLoaderFont);

pAdvLoaderFont->SetScale(0.35f * fLoaderFontScale);

//float fHeight = pLoaderFont->GetHeight();

//pLoaderFont->SetHeight(24.0f * fLoaderFontScale);


//float fHeightReal = pLoaderFont->GetHeight();



//---------------------

string message1 = "This version of a game still in development. Please be aware that there is a chance that nsome bugs or hangs may be present";

string display_message = englishStringID;

ILocStrings* pLocStr = (ILocStrings*)api->GetService("LocStrings");

if (pLocStr)
{
const char* val = NULL;

val = pLocStr->GetString(2000000);
if (val != NULL)
{
message1 = val;
} 


val = pLocStr->GetString(dwLocStringID);
if (val != NULL)
{
display_message = val;
}




} else
{
api->Trace("RENDER: Can't find LOCSERVICE !!");
}

static char messageToDisplay[4096];

if (szAdditionalMessage)
{
crt_snprintf(messageToDisplay, 4000, "%s %s", display_message.c_str(), szAdditionalMessage);
} else
{
crt_snprintf(messageToDisplay, 4000, "%s", display_message.c_str());
}


//---------------------

for (long i = 0; i < 4; i++)
{
BeginScene();
SetViewport(m_CurrentViewPort);

D3D()->Clear(0, null, D3DCLEAR_TARGET, 0x40404040, 1.0, 0);

//pAdvLoaderFont->Print(0, 0, 1, 1, "Hello");


const char* message = message1.c_str();
//float size = pLoaderFont->GetLength(message);
//pLoaderFont->Print((m_CurrentViewPort.Width * 0.5f) - (size * 0.5f), (m_CurrentViewPort.Height * 0.5f)-fHeightReal, message);

pAdvLoaderFont->SetScale(0.45f * fLoaderFontScale);
pAdvLoaderFont->Print(false, 0.0f, 0.0f, 100.0f, 100.0f, message, IAdvFont::Center, IAdvFont::Center, false, true);


//............



//pLoaderFont->Print(80.0f, m_CurrentViewPort.Height - fHeightReal-60.0f, messageToDisplay);

pAdvLoaderFont->SetScale(0.25f * fLoaderFontScale);
pAdvLoaderFont->Print(false, 5.0f, 0.0f, 100.0f, 90.0f, messageToDisplay, IAdvFont::Left, IAdvFont::Bottom, false, true);


EndScene();
pDevice->Present();
}

//pLoaderFont->SetHeight(fHeight);

}
*/

IRender::ScreenAspect NGRender::GetWideScreenAspect()
{
	return aspect;
}

float NGRender::GetWideScreenAspectWidthMultipler()
{
	return fWidthAspectMultipler;
}


void NGRender::SetLoadingImage(const char * szTextureName)
{
	if (szTextureName == NULL)
	{
		return;
	}

	if (crt_stricmp (strReloadingTextureName.c_str(), szTextureName) == 0)
	{
		return;
	}

	LockRes(_FL_, resource_creation_sc, "SetLoadingImage");

	static char tmpName[512];

#ifdef _XBOX
	//на боксе из корня берем jpg
	crt_snprintf(tmpName, 511, "%s.jpg", szTextureName);
#else
	//на PC из resource...
	crt_snprintf(tmpName, 511, "resource\\%s.jpg", szTextureName);
#endif

	


	//-----------------------------------------------
	UnlockRes(_FL_, resource_creation_sc, "SetLoadingImage");
	IBaseTexture* base = CreateTextureUseD3DX(_FL_, "%s", tmpName);
	LockRes(_FL_, resource_creation_sc, "SetLoadingImage");
		//m_TXXLoader->Load_TXX(szTextureName, _FL_);
	if (base)
	{
		RELEASE(pReloadingTexture);
		pReloadingTexture = NULL;
		strReloadingTextureName = szTextureName;

		base->SetName("special_loading_screen_texture_for_use");
		pReloadingTexture = base;
	} else
	{
		api->Trace("Loading screen '%s' - not found !!! Using previous loading screen '%s' !!! ", tmpName, strReloadingTextureName.c_str());
	}



	UnlockRes(_FL_, resource_creation_sc, "SetLoadingImage");
}

void NGRender::SetLoadingScreenPercents (float percents, float total_percents, const char* stage_description)
{
	LockRes(_FL_, resource_creation_sc, "SetLoadingScreenPercents");

	loading_percents = percents;
	loading_percents_total = total_percents;

	UnlockRes(_FL_, resource_creation_sc, "SetLoadingScreenPercents");
}


void NGRender::ThreadExecute(LPVOID lpThreadParameter)
{
	while (true)
	{
		bool bNeedCloseThread = pThreadRS->DrawLoadingScreen();
		if (bNeedCloseThread)
		{
			break;
		}
	}

	ExitThread(0);
}


#ifndef _XBOX
IMediaPlayer* NGRender::GetMediaPlayer(const char * pFileName,bool bLoopPlay,bool useStreaming)
{
	return (IMediaPlayer*)NULL;
}
#else
#include "mediaplayer\\xboxmediaplayer.h"
IMediaPlayer* NGRender::GetMediaPlayer(const char* pFileName,bool bLoopPlay,bool useStreaming)
{
	HRESULT hr;
	XBoxMediaPlayer* pMediaPlayer = NEW XBoxMediaPlayer(D3D(), pFileName, hr, bLoopPlay, useStreaming);
	if( pMediaPlayer && FAILED(hr) )
	{
		delete pMediaPlayer;
		pMediaPlayer = null;
	}
	return pMediaPlayer;
}
#endif


#ifndef _XBOX

IVideoCodec* NGRender::CreateVideoCodec(bool useStreaming)
{
	IVideoCodec* pCodec = NEW WMVCodec(this);
	return pCodec;
}
#else

IVideoCodec* NGRender::CreateVideoCodec(bool useStreaming)
{
	IVideoCodec* pCodec = NEW XMVCodec(this,useStreaming);
	return pCodec;
}
#endif


void NGRender::InitShadowLimiters ()
{
	GMX_ShadowLimiters = pRS->GetTechniqueGlobalVariable("GMX_ShadowLimiters", _FL_);
	GMX_ShadowMatrix = pRS->GetTechniqueGlobalVariable("GMX_ShadowMatrix", _FL_);

	Assert(GMX_ShadowLimiters);
	Assert(GMX_ShadowMatrix);

	PrepareShadowLimiters();
}

void NGRender::ReleaseShadowLimiters()
{
	GMX_ShadowLimiters = NULL;
	GMX_ShadowMatrix = NULL;
}


void NGRender::SetupShadowLimiters ()
{
	GMX_ShadowLimiters->SetVector4(mShadowVar);
	GMX_ShadowMatrix->SetMatrix(*gmxFinalShdwMatrix);
}

void NGRender::MakeError (ErrorID id)
{
	ILocStrings* pLocStr = (ILocStrings*)api->GetService("LocStrings");

	ICoreStorageString * errorDesc = api->Storage().GetItemString("system.error", _FL_);

	if (!errorDesc)
	{
		return;
	}


	if (id == ERR_D3D)
	{
		if (pLocStr)
		{
			const char* val = pLocStr->GetString(1000100);
			if (val != NULL)
			{
				errorDesc->Set(val);
				errorDesc->Release();
				return;
			}
		}

		errorDesc->Set("Can't init D3D");
		errorDesc->Release();
		return;

	} else
	{
		if (id == ERR_DEVICE)
		{
			if (pLocStr)
			{
				const char* val = pLocStr->GetString(1000101);
				if (val != NULL)
				{
					errorDesc->Set(val);
					errorDesc->Release();
					return;
				}
			}

			errorDesc->Set("Can't init D3D Device");
			errorDesc->Release();
			return;

		} else
		{
			if (id == ERR_INIT)
			{
				if (pLocStr)
				{
					const char* val = pLocStr->GetString(1000102);
					if (val != NULL)
					{
						errorDesc->Set(val);
						errorDesc->Release();
						return;
					}
				}

				errorDesc->Set("Can't init Render service");
				errorDesc->Release();
				return;
			} else
			{
				if (id == ERR_SHADERS30)
				{
					if (pLocStr)
					{
						const char* val = pLocStr->GetString(1000103);
						if (val != NULL)
						{
							errorDesc->Set(val);
							errorDesc->Release();
							return;
						}
					}

					errorDesc->Set("Device not support shader model 3.0 !");
					errorDesc->Release();
					return;
				} else
				{
					if (id == ERR_D3DX)
					{
						if (pLocStr)
						{
							const char* val = pLocStr->GetString(1000104);
							if (val != NULL)
							{
								errorDesc->Set(val);
								errorDesc->Release();
								return;
							}
						}

						errorDesc->Set("Required D3DX not found !! Please update your DirectX to latest version !");
						errorDesc->Release();
						return;
					}
				}
			}
		}
	}


	errorDesc->Set("Unknown Render service error");
	errorDesc->Release();
	return;

}

// - viewport or resolution changed
void NGRender::PrepareShadowLimiters ()
{
	float fWidth, fHeight;

	IRenderTarget * renderTarget = NULL;
	IRenderTargetDepth * renderTargetDepth = NULL;
	pDevice->GetRenderTarget(&renderTarget, &renderTargetDepth);

	RENDERSURFACE_DESC desc;
	if (renderTarget) 
	{
		renderTarget->GetDesc(&desc);
		fWidth = (float)desc.Width;
		fHeight = (float)desc.Height;
	}
	else
		return;

	const RENDERVIEWPORT &rvp = pRS->GetViewport();

	mShadowVar.x = rvp.X / fWidth;
	mShadowVar.y = rvp.Y / fHeight;
	mShadowVar.z = rvp.Width / fWidth;
	mShadowVar.w = rvp.Height / fHeight;

	//Матрица масштабирования
	Matrix mInvertY;
	mInvertY.BuildScale(Vector(1.0, -1.0, 1.0f));
	//Матрица приводящая к диапазону 0..1
	Matrix mNormalization;
	mNormalization.BuildScale(Vector(0.5f, 0.5f, 1.0f));
	mNormalization = (mInvertY * mNormalization) * Matrix(Vector(0.0f), Vector(0.5f, 0.5f, 1.0f));


	Matrix mFitToViewport;
	mFitToViewport.BuildScale(Vector(mShadowVar.z, mShadowVar.w, 1.0f));
	mFitToViewport = mFitToViewport * Matrix(Vector(0.0f), Vector(mShadowVar.x, mShadowVar.y, 1.0f));

	*mShadow = mNormalization * mFitToViewport;
}


#if HAVE_PIX_BEGINEND_EVENT_SUPPORT
dword _cdecl NGRender::pixBeginEvent (const char *srcFile, long srcLine, const char * pFormat, ...)
{
	if (loadingScreenThread) return 0;

	static char pixTmpBuffer[4096];
	static char pixTmpBuffer2[4096];

	va_list args;
	va_start(args, pFormat);
	crt_vsnprintf(pixTmpBuffer, sizeof(pixTmpBuffer), pFormat, args);
	va_end(args);


	crt_snprintf(pixTmpBuffer2, 4095, "'%s' - %s, %d", pixTmpBuffer, srcFile, srcLine);

	PIXBeginNamedEvent( 0x0, pixTmpBuffer2);

	dword dwRet = dwPixGUID;
	currentEvents.Push(eventID(dwRet, srcFile, srcLine));
	dwPixGUID++;

	return dwRet;
}

void _cdecl NGRender::pixEndEvent (const char *srcFile, long srcLine, dword id)
{
	if (loadingScreenThread) return;


	const eventID & evtId = currentEvents.Top();
	if (id != evtId.id)
	{
		int a = 0;

		api->Trace("ERROR !!!  pixEndEvent - incorrect bracing, maybe pixBeginEvent missed ?");
		api->Trace("events stack:");
		for(dword i = 0; i < currentEvents.Size(); i++)
		{
			api->Trace("%u: %s, %i", i, currentEvents[i].srcFile, currentEvents[i].srcLine);
		}

		Assert(false);
	}

	currentEvents.Pop();



	//static char pixTmpBuffer[4096];
	//static char pixTmpBuffer2[4096];
	//crt_snprintf(pixTmpBuffer2, 4095, "%s, %d", srcFile, srcLine);



	PIXEndNamedEvent();
}
#endif


void NGRender::UpdateWVP()
{
	mtxViewInv.Inverse(*mtxView);
	mtxWorldInv.Inverse(*mtxWorld);

	//                              mtxWorld
	//mtxView
	//mtxProjection
	//mShadow

	Matrix::MultiplySIMD_4x4(mtxWorldView->matrix, mtxWorld->matrix, mtxView->matrix);
	Matrix::MultiplySIMD_4x4(mtxWorldViewProj->matrix, mtxWorldView->matrix, mtxProjection->matrix);
	Matrix::MultiplySIMD_4x4(gmxFinalShdwMatrix->matrix, mtxWorldViewProj->matrix, mShadow->matrix);

	//mtxWorldView.EqMultiplyFast(mtxWorld, mtxView);
	//mtxWorldViewProj.EqMultiplyFast(mtxWorldView, mtxProjection);
	//gmxFinalShdwMatrix = mtxWorldViewProj * mShadow;
}

bool NGRender::IsCreateRenderTarget(RENDERFORMAT format)
{
	D3DFORMAT d3d_fmt = FormatToDX (format);

	HRESULT hr = pD3D->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.BackBufferFormat, D3DUSAGE_RENDERTARGET, D3DRTYPE_SURFACE, d3d_fmt);

	return SUCCEEDED( hr );
}

bool NGRender::IsPostPixelShaderBlendingSupported(RENDERFORMAT format)
{
	D3DFORMAT d3d_fmt = FormatToDX (format);

	HRESULT hr = pD3D->CheckDeviceFormat( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.BackBufferFormat, D3DUSAGE_RENDERTARGET | D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, D3DRTYPE_SURFACE, d3d_fmt);

	return SUCCEEDED( hr );
}

void NGRender::ResolveDepth (IRenderTargetDepth* dest)
{
	pDevice->ResolveDepth(dest);
}

void NGRender::ResolveColor (IBaseTexture* dest)
{
	pDevice->ResolveColor(dest);
}

bool NGRender::IsBlendFactorSupported()
{
	D3DCAPS9 caps;
	D3D()->GetDeviceCaps(&caps);

	if (caps.SrcBlendCaps & D3DPBLENDCAPS_BLENDFACTOR)
	{
		return true;
	}

	return false;
}


void NGRender::LightChangeState (ILight* light_ptr)
{
	dword dwIdx = activeLights.Find(light_ptr);

	//Если включение
	if (light_ptr->isEnabled())
	{
		//Не было в списке
		if (dwIdx == INVALID_ARRAY_INDEX)
		{
			//Добавляем
			activeLights.Add(light_ptr);
		}

		return;
	} else
	{
		//Если выключение и был в списке включенных - удаляем из этого списка
		if (dwIdx != INVALID_ARRAY_INDEX)
		{
			activeLights.Extract(dwIdx);
		}
	}






}



//true на один кадр !!! когда рендер вернулся в нормальное состояне (alt-tab вернули вокус) в full screen
bool NGRender::IsRenderReseted()
{
#ifdef _XBOX
	return false;
#else

	return bDeviceReseted;
#endif

}



#ifdef _XBOX


void NGRender::X360_DeviceSuspend()
{
	D3D()->Suspend();
}

void NGRender::X360_DeviceResume()
{
	D3D()->Resume();
}

#endif



#if defined (_XBOX) && !defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)

void NGRender::DisableHyperZ (bool bDisable)
{
	bDisableHiZ = bDisable;

	if (bDisable)
	{
		D3D()->SetRenderState(D3DRS_HIZENABLE, D3DHIZ_DISABLE);
		D3D()->SetRenderState(D3DRS_HIZWRITEENABLE, D3DHIZ_DISABLE);
	} else
	{
		D3D()->SetRenderState(D3DRS_HIZENABLE, D3DHIZ_AUTOMATIC);
		D3D()->SetRenderState(D3DRS_HIZWRITEENABLE, D3DHIZ_AUTOMATIC);
	}
}

bool NGRender::IsHyperZDisabled()
{
	return bDisableHiZ;
}

#endif

void NGRender::ReleaseQueryNotify(OcclusionQuery * ptr)
{
	queries.Del(ptr);
}

IOcclusionQuery * NGRender::CreateOcclusionQuery(const char *srcFile, long srcLine)
{
	OcclusionQuery * q = NEW OcclusionQuery;
	queries.Add(q);

	return q;
}


void NGRender::setFogParams (float h_density, float h_min, float h_max, float d_density, float d_min, float d_max, const Color & color)
{
	float fDistMinSq = d_min * d_min;
	float fDistMaxSq = d_max * d_max;

	stdFogVariables.fHeight_Density = h_density;
	stdFogVariables.fHeight_Min = h_min;
	stdFogVariables.fHeight_Max = h_max;
	stdFogVariables.fHeight_DistanceInv =  1.0f / (h_max - h_min);

	stdFogVariables.fDistance_Density = d_density;
	stdFogVariables.fDistance_Min = d_min;
	stdFogVariables.fDistance_Max = d_max;
	stdFogVariables.fDistance_DistanceInv = 1.0f / (fDistMaxSq - fDistMinSq);
	stdFogVariables.fDistance_MinSquared = fDistMinSq;

	stdFogVariables.fogColor = color;
}

void NGRender::getFogParams (float & h_density, float & h_min, float & h_max, float & d_density, float & d_min, float & d_max, Color & color)
{
	h_density = stdFogVariables.fHeight_Density;
	h_min = stdFogVariables.fHeight_Min;
	h_max = stdFogVariables.fHeight_Max;

	d_density = stdFogVariables.fDistance_Density;
	d_min = stdFogVariables.fDistance_Min;
	d_max = stdFogVariables.fDistance_Max;

	color = stdFogVariables.fogColor;
}



void NGRender::OnResetDevice()
{
	bInsideBeginScene = false;
	bNeedResetDevice = false;

	api->Trace("NGRender::OnResetDevice()");

	m_ShaderManager.OnLostDevice();
	for (dword i = 0; i < aVBuffers.Size(); i++)
	{
		api->Trace("VB::Reset   File: %s, line: %d", aVBuffers[i]->GetFileName(), aVBuffers[i]->GetFileLine());
		((CDX8VBuffer*)aVBuffers[i])->OnLostDevice();
	}

	for (dword i = 0; i < aIBuffers.Size(); i++)
	{
		api->Trace("IB::Reset   File: %s, line: %d", aIBuffers[i]->GetFileName(), aIBuffers[i]->GetFileLine());

		((CDX8IBuffer*)aIBuffers[i])->OnLostDevice();
	}


	for (dword i = 0; i < aRenderTargets.Size(); i++)
	{
		api->Trace("RT::Reset   File: %s, line: %d", aRenderTargets[i]->GetFileName(), aRenderTargets[i]->GetFileLine());

		((RenderTarget*)aRenderTargets[i])->OnLostDevice();
	}

	for (dword i = 0; i < aRenderTargetsDepth.Size(); i++)
	{
		api->Trace("RTD::Reset   File: %s, line: %d", aRenderTargetsDepth[i]->GetFileName(), aRenderTargetsDepth[i]->GetFileLine());

		((RenderTargetDepth*)aRenderTargetsDepth[i])->OnLostDevice();
	}



	for (dword i = 0; i < aTextures.Size(); i++)
	{
		api->Trace("Tex::Reset   File: %s, line: %d", aTextures[i]->GetFileName(), aTextures[i]->GetFileLine());
		aTextures[i]->OnLostDevice();
	}


	d3d9Sync.Lost();



	for (dword i = 0; i < queries.Size(); i++)
	{
		api->Trace("Query::Reset 0x%08X", queries[i]);
		queries[i]->OnLostDevice();
	}



	pDevice->Lost();


	//              m_Screen3DInfo.dwWidth = d3dpp.BackBufferWidth;
	//m_Screen3DInfo.dwHeight = d3dpp.BackBufferHeight;


	HRESULT hr = D3D()->Reset(&d3dpp);
	Assert(hr == D3D_OK);

	RENDERVIEWPORT vp;
	vp.X      = 0;
	vp.Y      = 0;
	vp.Width  = GetScreenInfo2D().dwWidth;
	vp.Height = GetScreenInfo2D().dwHeight;
	vp.MinZ   = 0.0f;
	vp.MaxZ   = 1.0f;
	SetViewport(vp);
	SetPerspective(GetScreenInfo2D().fFOV);
	//              m_FullScreenViewPort = vp;


	m_ShaderManager.OnResetDevice();
	for (dword i = 0; i < aVBuffers.Size(); i++)
	{
		((CDX8VBuffer*)aVBuffers[i])->OnResetDevice();
	}


	for (dword i = 0; i < aIBuffers.Size(); i++)
	{
		((CDX8IBuffer*)aIBuffers[i])->OnResetDevice();
	}

	for (dword i = 0; i < aTextures.Size(); i++)
	{
		aTextures[i]->OnResetDevice();
	}



	for (dword i = 0; i < aRenderTargets.Size(); i++)
	{
		((RenderTarget*)aRenderTargets[i])->OnResetDevice();
	}

	for (dword i = 0; i < aRenderTargetsDepth.Size(); i++)
	{
		((RenderTargetDepth*)aRenderTargetsDepth[i])->OnResetDevice();
	}

	for (dword i = 0; i < queries.Size(); i++)
	{
		queries[i]->OnResetDevice();
	}


	pDevice->Reset();

	/*
	for (dword i = 0; i < tempRenderTargetsC.Size(); i++)
	{
	((RenderTarget*)tempRenderTargetsC[i].rt)->OnResetDevice();
	}

	for (dword i = 0; i < tempRenderTargetsD.Size(); i++)
	{
	((RenderTargetDepth*)tempRenderTargetsD[i].rt)->OnResetDevice();
	}
	*/      


	d3d9Sync.Reset();




#ifndef _XBOX
	redundantStatesFilter->SetDefaultStates();
#endif


	bCantRender = false;
	bDeviceReseted = true;

}



struct VidMemInfo
{
	const char* type;
	const char* file;
	long line;

	long size;

	const char* txx_fileName;

	VidMemInfo()
	{
		type = NULL;
		file = NULL;
		line = 0;
		size = 0;
		txx_fileName = NULL;
	}

	VidMemInfo(const char* _type, const char* _file, long _line, long _size)
	{
		type = _type;
		file = _file;
		line = _line;
		size = _size;
		txx_fileName = NULL;
	}

	VidMemInfo(const char* _type, const char* _file, long _line, long _size, const char* txxName)
	{
		type = _type;
		file = _file;
		line = _line;
		size = _size;
		txx_fileName = txxName;
	}

};


bool ComapreMemStat(const VidMemInfo & a1, const VidMemInfo & a2)
{
	if (a1.size > a2.size) return true;
	return false;
}




void NGRender::DumpMemStat ()
{

	array<VidMemInfo> info(_FL_);


	dword dwTotalTexSize = 0;
	for (long i=0; i<aTextures; i++)
	{
		IBaseTexture* tx = aTextures[i];
		info.Add(VidMemInfo("Texture", tx->GetFileName(), tx->GetFileLine(), tx->GetSize(), tx->GetName()));
	}

	dword dwTotalVBSize = 0;
	for (long i=0; i<aVBuffers; i++)
	{
		IVBuffer* obj = aVBuffers[i];
		info.Add(VidMemInfo("Vertex Buffer", obj->GetFileName(), obj->GetFileLine(), obj->GetSize()));
	}

	dword dwTotalIBSize = 0;
	for (long i=0; i<aIBuffers; i++)
	{
		IIBuffer* obj = aIBuffers[i];
		info.Add(VidMemInfo("Index Buffer", obj->GetFileName(), obj->GetFileLine(), obj->GetSize()));
	}

	dword dwTotalRTSize = 0;
	for (long i=0; i<aRenderTargets; i++)
	{
		IRenderTarget* obj = aRenderTargets[i];

		bool bShared = false;
		for (dword q = 0; q < tempRenderTargetsC.Size(); q++)
		{
			if (tempRenderTargetsC[q].rt == obj)
			{
				bShared = true;
				break;
			}
		}

		if (bShared)
		{
			info.Add(VidMemInfo("Render Target Color (shared)", obj->GetFileName(), obj->GetFileLine(), obj->GetSize()));
		} else
		{
			info.Add(VidMemInfo("Render Target Color", obj->GetFileName(), obj->GetFileLine(), obj->GetSize()));
		}
	}

	dword dwTotalRTDSize = 0;
	for (long i=0; i<aRenderTargetsDepth; i++)
	{
		IRenderTargetDepth* obj = aRenderTargetsDepth[i];

		bool bShared = false;
		for (dword q = 0; q < tempRenderTargetsD.Size(); q++)
		{
			if (tempRenderTargetsD[q].rt == obj)
			{
				bShared = true;
				break;
			}
		}

		if (bShared)
		{
			info.Add(VidMemInfo("Render Target Depth (shared)", obj->GetFileName(), obj->GetFileLine(), obj->GetSize()));
		} else
		{
			info.Add(VidMemInfo("Render Target Depth", obj->GetFileName(), obj->GetFileLine(), obj->GetSize()));
		}
	}

	info.QSort(ComapreMemStat);

	api->Trace("\n\nVideo memory stats\n================================================================================\n");
	api->Trace("Mb          Type                    Source info");

	dword dwTotalSize = 0;

	for (dword i = 0; i < info.Size(); i++)
	{
		api->Trace("%3.3fKb, '%s', %s, line %d", info[i].size / 1024.0f, info[i].type, info[i].file, info[i].line);
		if (info[i].txx_fileName)
		{
			api->Trace("'%s'", info[i].txx_fileName);
		} else
		{
			api->Trace("");
		}

		dwTotalSize += info[i].size;
	}

	api->Trace("Total size : %3.2fMb", dwTotalSize / 1024.0f / 1024.0f);


}





void NGRender::initPerfGraphs ()
{
	perfGraphs.DelAll();


	struct GraphDesc
	{
		string name;
		float scale;
	};

	array<GraphDesc> descriptions(_FL_, 128);


	IFileService * fs = (IFileService *)api->GetService("FileService");
	Assert(fs);
	//api->GetVarString("ini")
	IIniFile * pIni = fs->OpenIniFile("counters.ini", _FL_);
	if (pIni)
	{
		array<string> values(_FL_, 256);
		pIni->GetStrings("Counters", "Graph", values);

		for (dword n = 0; n < values.Size(); n++)
		{

			static char g_name[1024];
			static char g_name2[1024];

			for (dword q = 0; q < values[n].Size(); q++)
			{
				if (values[n].c_str()[q] == ',')
				{
					crt_strncpy(g_name, 1023, values[n].c_str(), q);
					crt_strncpy(g_name2, 1023, values[n].c_str()+q+1, 1023);
				}
			}


			GraphDesc* d = &descriptions[descriptions.Add()];
			d->name = g_name;
			d->scale = (float)atof(g_name2);


		}
	} else
	{
		GraphDesc* d = &descriptions[descriptions.Add()];
		d->name = "*";
		d->scale = 1.0f;
	}



	perfGraph p;
	dword dwPerCounterCount = api->GetNumberOfPerformanceCounters();
	api->Trace("Registred perfomance counters : %d", dwPerCounterCount);
	for (dword n = 0; n < dwPerCounterCount; n++)
	{
		string counterName = api->GetPerformanceName(n);

		api->Trace("Perf counter [%d] : '%s'", n, counterName.c_str());


		for (dword q = 0; q < descriptions.Size(); q++)
		{
			if (counterName.IsFileMask(descriptions[q].name))
			{
				p.dwColor = Color(Vector().Rand(Vector(0.6f), Vector(1.0f))).GetDword();
				p.dwID = n;
				p.name = counterName;
				p.scale = descriptions[q].scale;
				perfGraphs.Add(p);

			}
		}

	}

}

#ifdef GAME_RUSSIAN
#pragma optimize("", off)
void  __declspec(dllexport) NGRender::InitProtectData(dword dwValue1, dword dwValue2)
{
	dword result, result2;

	dword value1 = (dwValue1 >> 7L) & 0x7;
	dword value2 = (dwValue2 >> 16L) & 0x3;
	switch (value1)
	{
		case 0 : result = dwValue1 * (value1 + 16); break;
		case 1 : result = dwValue1 * (value1 + 15); break;
		case 2 : result = dwValue1 * (value1 + 14); break;
		case 3 : result = dwValue1 * (value1 + 13); break;
		case 4 : result = dwValue1 * (value1 + 12); break;
		case 5 : result = dwValue1 * (value1 + 11); break;
		case 6 : result = dwValue1 * (value1 + 10); break;
		case 7 : result = dwValue1 * (value1 + 9); break;
		default: result = dwValue1 * 16;
	}

	dword value3 = (dwValue2 >> 24L) & 0x7;
	switch (value2)
	{
		case 0 : result2 = dwValue2 + value2 + 122; break;
		case 1 : result2 = dwValue2 + value2 + 121; break;
		case 2 : result2 = dwValue2 + value2 + 120; break;
		default: result2 = dwValue2 + 122;
	}

	switch (value3)
	{
		case 0 : result2 *= (32 - value3); break;
		case 1 : result2 *= (33 - value3); break;
		case 2 : result2 *= value3 * 16; break;
		case 3 : result2 *= (35 - value3); break;
		case 4 : result2 *= 8 * value3; break;
		case 5 : result2 *= (value3 + 27); break;
		case 6 : result2 *= (38 - value3); break;
		case 7 : result2 *= (value3 + 25); break;
		default: result2 *= 32;
	}

	protectValue = result + result2;
}
#pragma optimize("", on)
#endif


