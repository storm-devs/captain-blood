#include "Sea.h"
#include "..\..\common_h\tga.h"
#include "SSE.h"
// 
// 
// //Для D3DXPLANE, FIXME !!!
// #include <d3dx9.h>
// #pragma comment(lib, "d3dx9.lib")
// 
// 
// 
// #define NUM_VERTEXS		65500
// #define NUM_INDICES		165000
// 
// #define FRAMES		64
// #define XWIDTH		128
// #define YWIDTH		128
// 
// #define GC_CONSTANT             0 	// Global constants = {0.0, 1.0, 0.5, 0.0000152590218967 = (0.5 / 32767.5)}
// #define GC_CONSTANT2            1 	// Global constants 2 = {2.0, -1.0, 0.00036621652552071 = (12 / 32767.5), fog}
// #define GC_ANIMATION			2 	// Animation frames(0.0-1.0) for 4 stages = {stage0, stage1, stage2, stage3}
// #define GC_SHADOW_CONST1	    3  
// #define GC_SHADOW_CONST2		7 
// #define GC_LIGHT4               11 	//
// #define GC_LIGHT3               13 
// #define GC_LIGHT2               15
// #define GC_LIGHT1               17 
// #define GC_LIGHT0               19 
// #define GC_MATERIAL             21 	// 
// #define GC_CAMERA_POS           23 	// Local Camera position = {x, y, z, 0.0}
// #define GC_MTX_WVP              24	// c[0] = mWorld * mView * mProjection
// 
// #define GC_FREE					28
// 
// 
// #define EXECUTE_LEVEL (ML_ALPHA1-1)
// 
// 
// SEA * SEA::pSea = null;
// bool SEA::bIntel = false;
// bool SEA::bSSE = false;
// bool SEA::bDisableSSE = false;
// 
// 
// __forceinline dword _fastcall SEA::PackNormal (const Vector& n)
// {
// 	
// 	BYTE p_nx = (BYTE)fftol(n.x*127.5f + 127.5f);
// 	BYTE p_ny = (BYTE)fftol(n.y*127.5f + 127.5f);
// 	BYTE p_nz = (BYTE)fftol(n.z*127.5f + 127.5f);
// 
// 	return ((p_nx << 16) | (p_ny << 8) | p_nz);
// }
// 
// __forceinline float _fastcall SEA::UnpackNormalX (DWORD packed_normal)
// {
// 	BYTE bt_x = (BYTE)((packed_normal >> 16) & 0xFF);
// 
// 	return (((float)bt_x / 255.0f) * 2.0f) - 1.0f;
// }
// 
// __forceinline float _fastcall SEA::UnpackNormalY (DWORD packed_normal)
// {
// 	BYTE bt_y = (BYTE)((packed_normal >> 8) & 0xFF);
// 
// 	return (((float)bt_y / 255.0f) * 2.0f) - 1.0f;
// 
// }
// 
// __forceinline float _fastcall SEA::UnpackNormalZ (DWORD packed_normal)
// {
// 	BYTE bt_z = (BYTE)((packed_normal) & 0xFF);
// 
// 	return (((float)bt_z / 255.0f) * 2.0f) - 1.0f;
// }
// 
// 
// 
// SEA::SEA() : 
// 	aBlocks(_FL_, 128),
// 	aBumps(_FL_),
// 	aNormals(_FL_),
// 	aThreads(_FL_),
// 	aEventCalcBlock(_FL_),
// 	aThreadsTest(_FL_)
// {
// 	//FIX ME - ХАК !!!
// 	fSeaHeight = -4.0f;
// 
// 	bShowTexture = false;
// 
// 	fFlatSeaLength = 400.0f;
// 
// 	fs_flatSeaTexture = NULL;
// 	pFoamTexture = NULL;
// 	pBumpTexture = NULL;
// 	vClearColor = Color(0.2f, 0.0f, 0.0f);
// 
// 	fs_vClearColor = NULL;
// 	fs_vColor = NULL;
// 
// 	bFlatSea = false;
// 
// 
// 	fs_foamTexture = NULL;
// 	bRefraction = true;
// 
// 	fs_TextureProjMatrix = NULL;
// 	fs_vUnderWaterColor = NULL;
// 	pReflection = NULL;
// 	pReflectionSurfaceDepth = NULL;
// 	pReflectionSurface = NULL;
// 	pRefraction = NULL;
// 	pRefractionSurface = NULL;
// 	fs_bumpTexture = NULL;
// 	fs_reflTexture = NULL;
// 	fs_refrTexture = NULL;
// 	fs_bumpFrame = NULL;
// 	fs_vSubsurfaceColor = NULL;
// 	fFlatSeaBumpScale = NULL;
// 	fs_SpecularPower = NULL;
// 	fs_vSpecularColor = NULL;
// 	fs_FoamScale = NULL;
// 
// 
// 	fSmallBumpScale = 0.1f;
// 
// 
// 
// 	fFoamScale = 1.0f;
// 
// 
// 
// 
// 
// 	dwMaxDim = 65536 * 2;
// 	dwMinDim = 128;
// 
// 	fMaxSeaHeight = 20.0f;
// 	fGridStep = 0.06f;
// 	fLodScale = 0.4f;
// 
// 	fAmp1 = 1.0f;
// 	fFrame1 = 0.0f;
// 	fAnimSpeed1 = 1.0f;
// 	vMove1 = 0.0f;
// 	fScale1 = 0.4f;
// 
// 	fAmp2 = 1.0f;
// 	fFrame2 = 10.0f;
// 	fAnimSpeed2 = 1.0f;
// 	vMove2 = 0.0f;
// 	fScale2 = 2.0f;
// 
// 	fPosShift = 2.0f;
// 
// 	fFogSeaDensity = 0.0f;
// 	fFogStartDistance = 0.0f;
// 	vFogColor = 0.0f;
// 	bFogEnable = false;
// 
// 	pSeaFrame1 = NEW float[XWIDTH * YWIDTH];
// 	pSeaFrame2 = NEW float[XWIDTH * YWIDTH];
// 	pSeaNormalsFrame1 = NEW float[2 * XWIDTH * YWIDTH];
// 	pSeaNormalsFrame2 = NEW float[2 * XWIDTH * YWIDTH];
// 
// 	pVSeaBuffer = NULL;
// 	pISeaBuffer = NULL;
// 
// 	bTempFullMode = false;
// 
// 	// HyperThreading section
// 	hEventCalcMaps = CreateEvent(null, false, false, null);
// 
// 	InitializeCriticalSection(&cs);
// 	InitializeCriticalSection(&cs1);
// 
// 	pSea = this;
// 	bIntel = intel.IsIntelCPU();
// 	bIntel = false;
// 	bSSE = intel.IsSSE();
// }
// 
// SEA::~SEA()
// {
// 	
// 	if (fs_flatSeaTexture) fs_flatSeaTexture->Release();
// 	if (fs_vSubsurfaceColor) fs_vSubsurfaceColor->Release();
// 	if (fs_TextureProjMatrix) fs_TextureProjMatrix->Release();
// 		
// 	if (pFoamTexture) pFoamTexture->Release();
// 	if (pReflection) pReflection->Release();
// 	if (pReflectionSurfaceDepth) pReflectionSurfaceDepth->Release();
// 	if (pReflectionSurface) pReflectionSurface->Release();
// 	if (pRefraction) pRefraction->Release();
// 	if (pRefractionSurface) pRefractionSurface->Release();
// 	if (fs_bumpTexture) fs_bumpTexture->Release();
// 	if (fs_reflTexture) fs_reflTexture->Release();
// 	if (fs_refrTexture) fs_refrTexture->Release();
// 	if (fs_bumpFrame) fs_bumpFrame->Release();
// 	if (fFlatSeaBumpScale) fFlatSeaBumpScale->Release();
// 	if (fs_vClearColor) fs_vClearColor->Release();
// 	if (fs_vColor) fs_vColor->Release();
// 	if (fs_SpecularPower) fs_SpecularPower->Release();
// 
// 	if (fs_foamTexture) fs_foamTexture->Release();
// 	if (fs_vUnderWaterColor) fs_vUnderWaterColor->Release();
// 	if (fs_vSpecularColor) fs_vSpecularColor->Release();
// 	if (fs_FoamScale) fs_FoamScale->Release();
// 
// 
// 	if (pBumpTexture) pBumpTexture->Release();
// 
// 	fs_bumpTexture = NULL;
// 	fs_reflTexture = NULL;
// 	fs_refrTexture = NULL;
// 	fs_bumpFrame = NULL;
// 
// 	pReflection = NULL;
// 	pReflectionSurface = NULL;
// 	pReflectionSurfaceDepth = NULL;
// 	pRefraction = NULL;
// 	pRefractionSurface = NULL;
// 
// 
// 
// 
// 	DeleteCriticalSection(&cs);
// 	DeleteCriticalSection(&cs1);
// 	
// #ifndef _XBOX 
// 
// 	for (long i=0; i<aThreads; i++)
// 	{
// 		if (aThreads[i]) TerminateThread(aThreads[i], 0);
// 		if (aEventCalcBlock[i]) CloseHandle(aEventCalcBlock[i]);
// 	}
// 
// #endif
// 
// 	if (hEventCalcMaps) CloseHandle(hEventCalcMaps);
// 
// //	Render().Release(pVolumeTexture);
// 
// 
// 	if (pVSeaBuffer) pVSeaBuffer->Release();
// 	if (pISeaBuffer) pISeaBuffer->Release();
// 
// 	pVSeaBuffer = NULL;
// 	pISeaBuffer = NULL;
// 
// 	DELETE(pIndices);
// 	DELETE(pVSea);
// 
// 	for (long i=0; i<aBumps; i++) DELETE(aBumps[i]);
// 	for (long i=0; i<aNormals; i++) DELETE(aNormals[i]);
// 
// 	DELETE(pSeaFrame1);
// 	DELETE(pSeaFrame2);
// 	DELETE(pSeaNormalsFrame1);
// 	DELETE(pSeaNormalsFrame2);
// }
// 
// 
// void SEA::SetFoamTexture (const char* szTextureName)
// {
// 	if (pFoamTexture)
// 	{
// 		pFoamTexture->Release();
// 		pFoamTexture = NULL;
// 	}
// 
// 	pFoamTexture = Render().CreateTexture(_FL_, szTextureName);
// 
// }
// 
// void SEA::SetBumpTexture (const char* szTextureName)
// {
// 	if (pBumpTexture)
// 	{
// 		pBumpTexture->Release();
// 		pBumpTexture = NULL;
// 	}
// 
// 	pBumpTexture = Render().CreateAnimationTexture(_FL_, szTextureName);
// }
// 
// //Обновление параметров
// bool SEA::EditMode_Update(MOPReader & reader)
// {
// 	InitParams();
// 
// 	fMaxSeaHeight = reader.Float();
// 	fSeaHeight = reader.Float();
// 
// 
// 	float fScale = (fMaxSeaHeight >= _fAmp1 + _fAmp2) ? 1.0f : fMaxSeaHeight / (_fAmp1 + _fAmp2);
// 	fAmp1 = _fAmp1 * fScale;
// 	fAmp2 = _fAmp2 * fScale;
// 
// 
// 
// 	SetBumpTexture(reader.String());
// 	float fBumpAnimSpeed = reader.Float();
// 	if (pBumpTexture) pBumpTexture->SetAnimSpeed(fBumpAnimSpeed);
// 
// 	fSmallBumpScale = reader.Float();
// 
// 	SetFoamTexture(reader.String());
// 	fFoamScale = reader.Float();
// 
// 	vClearColor = reader.Colors();
// 	vSeaColor = reader.Colors();
// 	vSeaColor *= reader.Float();
// 	
// 	vSeaSubsurfaceColor = reader.Colors();
// 	vSeaColor *= reader.Float();
// 
// 	fSpecularPower = reader.Float();
// 
// 	vUnderWaterColor = reader.Colors();
// 	vSeaSpecularColor = reader.Colors();
// 
// 
// 	bFlatSea = reader.Bool();
// 	bShowTexture = reader.Bool();
// 	fFlatSeaLength = reader.Float();
// 
// 	Show(reader.Bool());
// 
// /*
// 	vSeaColor = vSeaColor * 4.0f;
// 	vSeaSubsurfaceColor = vSeaSubsurfaceColor * 4.0f;
// */
// 
// 
// 	Realize(0.0f, EXECUTE_LEVEL);
// 
// 
// 	return true;
// 
// }
// 
// 
// 
// bool SEA::Create(MOPReader & reader)
// {
// 
// 
// 
// 	SetUpdate(&SEA::Realize, EXECUTE_LEVEL);
// 
// 
// 	const char* iniName = api->GetVarString("Ini");
// 	IIniFile* pEngineIni =Files().OpenIniFile(iniName, _FL_);
// 
// 	bool bDisableHyperThreading = true;
// 	bDisableSSE = true;
// 	if (pEngineIni)
// 	{
// 		bDisableHyperThreading = (pEngineIni) ? pEngineIni->GetLong(null, "HyperThreading", 1) == 0 : false;
// 		bDisableSSE = (pEngineIni) ? pEngineIni->GetLong(null, "DisableSSE", 0) != 0 : false;
// 		pEngineIni->Release();
// 		pEngineIni=NULL;
// 	}
// 
// 	if (bDisableHyperThreading) bHyperThreading = false;
// 	if (bDisableSSE) bSSE = false;
// 	if (!bIntel) bHyperThreading = false;
// 
// 	if (bHyperThreading)
// 	{
// 		dword dwLogicals, dwCores, dwPhysicals;
// 		intel.CPUCount(&dwLogicals, &dwCores, &dwPhysicals);
// 		api->Trace("Total logical: %d, Total cores: %d, Total physical: %d", dwLogicals, dwCores, dwPhysicals);
// 
// 		dword dwNumThreads = dwLogicals * dwCores - 1;
// 	
// 		for (dword i=0; i<dwNumThreads; i++)
// 		{
// 			HANDLE & hEvent = aEventCalcBlock[aEventCalcBlock.Add()];
// 			hEvent = CreateEvent(null, false, false, null);
// 
// 			HANDLE & hThread = aThreads[aThreads.Add()];
// 			hThread = CreateThread(null, 0, (LPTHREAD_START_ROUTINE)SEA::ThreadExecute, (void*)i, CREATE_SUSPENDED, null);
// 			SetThreadPriority(hThread, THREAD_PRIORITY_NORMAL);
// 			ResumeThread(hThread);
// 
// 			aThreadsTest.Add();
// 		}
// 
// 		bHyperThreading = dwNumThreads > 0;
// 	}
// 
// 	pVSeaBuffer = Render().CreateVertexBuffer(NUM_VERTEXS * sizeof(SeaVertex), sizeof(SeaVertex), _FL_, USAGE_WRITEONLY | USAGE_DYNAMIC, POOL_DEFAULT);
// 	pISeaBuffer = Render().CreateIndexBuffer(NUM_INDICES * 3 * sizeof(word), _FL_, USAGE_WRITEONLY | USAGE_DYNAMIC, FMT_INDEX16, POOL_DEFAULT);
// 
// 	pIndices = NEW dword[NUM_VERTEXS*3];
// 	pVSea = NEW SeaVertex[NUM_VERTEXS];
// 
// 	pVolumeTexture = Render().CreateVolumeTexture(XWIDTH, YWIDTH, FRAMES, 1, 0, FMT_A8R8G8B8, _FL_, POOL_MANAGED);
// 	/*pAnimTexture = Render().CreateAnimationTexture(pVolumeTexture, _FL_);
// 	pAnimTexture->SetAnimSpeed(20.0f);*/
// 	
// 	byte bMin = 0xFF;
// 	byte bMax = 0;
// 
// 	array<byte*> aTmpBumps(_FL_);
// 
// 	dword i;
// 
// 	for (i=0; i<FRAMES; i++)
// 	{
// 		char str[256];
// 		char * pFBuffer = null;
// 		dword dwSize;
// 		crt_snprintf(str, 250, "resource\\sea\\sea%.4d.tga", i);
// 		//sprintf(str, "resource\\sea\\sea0000.tga", i);
// 		//fio->LoadFile(str, &pFBuffer, &dwSize);
// 		pFBuffer = (char*)Files().Load(str, &dwSize);
// 
// 		if (!pFBuffer) 
// 		{
// 			api->Trace("Sea: Can't load %s", str);
// 			return false;
// 		}
// 
// 		char * pFB = pFBuffer + sizeof(TGA_H);
// 
// 		byte * pBuffer = NEW byte[XWIDTH * YWIDTH];
// 		aTmpBumps.Add(pBuffer);
// 		
// 		for (dword y=0; y<YWIDTH; y++)
// 			for (dword x=0; x<XWIDTH; x++)
// 			{
// 				byte bB = (*pFB);
// 				//bB = byte(float(bB - 79.0f) * 255.0f / (139.0f - 79.0f));
// 				if (bB < bMin) bMin = bB;
// 				if (bB > bMax) bMax = bB;
// 				pBuffer[x + y * XWIDTH] = bB & 0xFF;
// 				pFB += sizeof(dword);
// 			}
// 
// 		DELETE(pFBuffer);
// 	}
// 
// 	for (i=0; i<FRAMES; i++)
// 	{
// 		byte * pBuffer = NEW byte[XWIDTH * YWIDTH];
// 		aBumps.Add(pBuffer);
// 
// 		for (dword y=0; y<YWIDTH; y++)
// 			for (dword x=0; x<XWIDTH; x++)
// 			{
// 				dword dwAddress = x + y * YWIDTH;
// 				float b1, b2, b3, b4, b5; // -2 -1 0 1 2
// 
// 				b1 = 0.08f * float(aTmpBumps[(i - 2) & (FRAMES - 1)][dwAddress]);
// 				b2 = 0.17f * float(aTmpBumps[(i - 1) & (FRAMES - 1)][dwAddress]);
// 				b3 = 0.50f * float(aTmpBumps[(i - 0) & (FRAMES - 1)][dwAddress]);
// 				b4 = 0.17f * float(aTmpBumps[(i + 1) & (FRAMES - 1)][dwAddress]);
// 				b5 = 0.08f * float(aTmpBumps[(i + 2) & (FRAMES - 1)][dwAddress]);
// 
// 				//pBuffer[dwAddress] = byte(b1 + b2 + b3 + b4 + b5);
// 				pBuffer[dwAddress] = aTmpBumps[(i - 0) & (FRAMES - 1)][dwAddress];
// 			}
// 
// 		/*char str[256];
// 		sprintf(str, "%.4d", i);
// 		HANDLE hFile = fio->_CreateFile(str, GENERIC_WRITE, FILE_SHARE_READ, CREATE_ALWAYS);
// 		if (INVALID_HANDLE_VALUE != hFile)
// 		{
// 			fio->_WriteFile(hFile, pBuffer, sizeof(byte) * 256 * 256, null);
// 			fio->_CloseHandle(hFile);
// 		}*/
// 	}
// 
// 	for (i=0; i<aTmpBumps(); i++) DELETE(aTmpBumps[i]);
// 
// 	// build normals
// 	for (i=0; i<FRAMES; i++)
// 	{
// 		dword * pBuffer = NEW dword[XWIDTH * YWIDTH];
// 		aNormals.Add(pBuffer);
// 
// 		RENDERLOCKED_BOX	box;
// 		if (!pVolumeTexture->LockBox(0, &box, 0, 0))
// 		{
// 			// error
// 		}
// 
// 		for (dword y=0; y<YWIDTH; y++)
// 			for (dword x=0; x<XWIDTH; x++)
// 			{
// #define GET_MASSIVE(dx,dy)	(float(pMassive[((x+dx)&(XWIDTH-1)) + ((y+dy)&(XWIDTH-1)) * XWIDTH] & 0xFF) / 255.0f)
// 				byte * pMassive = aBumps[i];
// 	
// 				float fCenter	= GET_MASSIVE(0, 0);
// 
// 				float fLeft		= GET_MASSIVE(-1, 0) - fCenter; 
// 				float fRight	= GET_MASSIVE(1, 0) - fCenter; 
// 				float fTop		= GET_MASSIVE(0, -1) - fCenter; 
// 				float fBottom	= GET_MASSIVE(0, 1) - fCenter; 
// 
// 				float f00		= GET_MASSIVE(-1, -1) - fCenter; 
// 				float f10		= GET_MASSIVE(1, -1) - fCenter; 
// 				float f11		= GET_MASSIVE(1, 1) - fCenter; 
// 				float f01		= GET_MASSIVE(-1, 1) - fCenter; 
// 
// 				Vector vRes, d0, d1, d2, d3, d4, d5, d6, d7;
// 
// 				dword dwNums = 0;
// 				if (fLeft < 0.0f) dwNums++;
// 				if (fRight < 0.0f) dwNums++;
// 				if (fTop < 0.0f) dwNums++;
// 				if (fBottom < 0.0f) dwNums++;
// 
// 				float d = 1.0f;
// 				d0 = Vector(-1.f, d * fLeft, 0.f);
// 				d1 = Vector(0.f, d * fTop, -1.f);
// 				d2 = Vector(1.f, d * fRight, 0.f);
// 				d3 = Vector(0.f, d * fBottom, 1.f);
// 
// 				//res = !((d0^d1) + (d2^d3)); 
// 
// 				Vector v1 = (d1^d0);
// 				Vector v2 = (d3^d2);
// 				Vector v3 = (d0^d3);
// 				Vector v4 = (d2^d1);
// 
// 				vRes = !((d1^d0) + (d3^d2) + (d0^d3) + (d2^d1)); 
// 				//vRes = !((d1^d0) + (d3^d2)); 
// 
// 				/*d0 = Vector(0.0f, d * fLeft, -1.0f);
// 				d1 = Vector(1.0f, d * fTop, -1.0f);
// 				d2 = Vector(1.0f, d * fRight, 0.0f);
// 				d3 = Vector(1.0f, d * fBottom, 1.0f);
// 				d4 = Vector(0.0f, d * f00, 1.0f);
// 				d5 = Vector(-1.0f, d * f11, 1.0f);
// 				d6 = Vector(-1.0f, d * f10, 0.0f);
// 				d7 = Vector(-1.0f, d * f01, -1.0f);
// 
// 				res = !((d0^d1) + (d1^d2) + (d2^d3) + (d3^d4) + (d4^d5) + (d5^d6) + (d6^d7) + (d7^d0));*/
// 
// 				dword dwRes = MAKELONG(short(vRes.x * 32767.5f), short(vRes.z * 32767.5f));
// 
// 				aNormals[i][x + y * XWIDTH] = dwRes;
// 
// 				//res = Vector(0.0f, 1.0f, 0.0f);
// 				//res = !Vector(res.x * 2.0f, res.y, res.z * 2.0f);
// 
// 				long red	= long((vRes.x * 0.5f + 0.5f) * 255.0f);		// FIX-ME no ftol
// 				long green	= long((vRes.y * 0.5f + 0.5f) * 255.0f);		// FIX-ME no ftol
// 				long blue	= long((vRes.z * 0.5f + 0.5f) * 255.0f);		// FIX-ME no ftol
// 
// 				*(dword*)&(((char*)box.pBits)[i * box.SlicePitch + y * box.RowPitch + x * 4]) = ARGB(0x80, blue, green, red);
// 			}
// 
// 		TGA_H TgaHead = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 32};
// 		TgaHead.width = XWIDTH;
// 		TgaHead.height = YWIDTH;
// 		pVolumeTexture->UnlockBox(0);
// 	}
// 	
// 	//JOKER - EditMode_Update();
// 
// 	EditMode_Update(reader);
// 
// 
// 	api->Trace("Intel CPU: %s, SSE: %s, HyperThreading: %s", (bIntel) ? "Yes" : "No", (bSSE) ? "On" : "Off", (bHyperThreading) ? "On" : "Off");
// 
// 
// 
// 
// 
// 	fs_flatSeaTexture = Render().GetTechniqueGlobalVariable("fs_flatSeaTexture", _FL_);
// 	fs_bumpFrame = Render().GetTechniqueGlobalVariable("fs_bumpFrame", _FL_);
// 	fs_bumpTexture = Render().GetTechniqueGlobalVariable("fs_bumpTexture", _FL_);
// 	fs_reflTexture = Render().GetTechniqueGlobalVariable("fs_reflTexture", _FL_);
// 	fs_refrTexture = Render().GetTechniqueGlobalVariable("fs_refrTexture", _FL_);
// 	fFlatSeaBumpScale = Render().GetTechniqueGlobalVariable("fFlatSeaBumpScale", _FL_);
// 
// 	fs_TextureProjMatrix = Render().GetTechniqueGlobalVariable("fs_TextureProjMatrix", _FL_);
// 	fs_vClearColor = Render().GetTechniqueGlobalVariable("fs_vClearColor", _FL_);
// 	fs_vColor = Render().GetTechniqueGlobalVariable("fs_vColor", _FL_);
// 	fs_vSubsurfaceColor = Render().GetTechniqueGlobalVariable("fs_vSubsurfaceColor", _FL_);
// 	fs_SpecularPower = Render().GetTechniqueGlobalVariable("fs_SpecularPower", _FL_);
// 	fs_foamTexture= Render().GetTechniqueGlobalVariable("fs_foamTexture", _FL_);
// 	fs_vUnderWaterColor= Render().GetTechniqueGlobalVariable("fs_vUnderWaterColor", _FL_);
// 	fs_vSpecularColor= Render().GetTechniqueGlobalVariable("fs_vSpecularColor", _FL_);
// 	fs_FoamScale= Render().GetTechniqueGlobalVariable("fs_FoamScale", _FL_);
// 
// 
// 
// 
// 
// 
// 	DWORD dwSize = 512;
// 	pReflection = Render().CreateTexture(dwSize, dwSize, 1, USAGE_RENDERTARGET, Render().GetScreenInfo().BackBufferFormat, _FL_ , POOL_DEFAULT);
// 	pRefraction = Render().CreateTexture(dwSize, dwSize, 1, USAGE_RENDERTARGET, Render().GetScreenInfo().BackBufferFormat, _FL_ , POOL_DEFAULT);
// 	pReflectionSurface = pReflection->GetSurfaceLevel(0, _FL_);
// 	pRefractionSurface = pRefraction->GetSurfaceLevel(0, _FL_);
// 	pReflectionSurfaceDepth = Render().CreateDepthStencilSurface(dwSize, dwSize, _FL_);
// 
// 
// 	return true;
// }
// 
// void SEA::InitParams()
// {
// 	//return true;
// 	v4SeaColor = Vector4(10.0f / 255.0f, 55.0f / 255.0f, 100.0f / 255.0f, 1.0f);
// 	v4SkyColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
// 
// 	float fReflection = 0.8f;
// 	float fTransparency = 0.7f;
// 	float fFrenel = 0.75f;
// 	float fAttenuation = 0.9f;
// 	v4SeaParameters = Vector4(fAttenuation, fReflection, fTransparency, 0.0f);
// 
// 	fAmp1 = _fAmp1 = 8.0f;
// 	fAnimSpeed1 = 6.0f;
// 	fScale1 = 0.5f;
// 	vMoveSpeed1 = Vector(5.0f, 0.0f, 0.0f);
// 
// 	fAmp2 = _fAmp2 = 1.0f;
// 	fAnimSpeed2 = 3.0f;
// 	fScale2 = 2.0f;
// 	vMoveSpeed2 = Vector(0.0f, 0.0f, 2.0f);
// 
// 	fBumpScale = 0.1f;
// 	fBumpSpeed = 1.0f;
// 	fGridStep = 0.07f;
// 	fLodScale = 0.5f;
// 
// 	fPosShift = 1.2f;
// 	
// 
// 
// 
// 
// }
// 
// long SEA::VisCode(const Vector & vP)
// {
// 	long vc = 0;
// 
// 	/*
// 	CVECTOR v = vP - vCamPos;
// 
// 	CVECTOR vp1 = CVECTOR(pFrustumPlanes[0].Nx, pFrustumPlanes[0].Ny, pFrustumPlanes[0].Nz);
// 	CVECTOR vp2 = CVECTOR(pFrustumPlanes[1].Nx, pFrustumPlanes[1].Ny, pFrustumPlanes[1].Nz);
// 	CVECTOR vp3 = CVECTOR(pFrustumPlanes[2].Nx, pFrustumPlanes[2].Ny, pFrustumPlanes[2].Nz);
// 	CVECTOR vp4 = CVECTOR(pFrustumPlanes[3].Nx, pFrustumPlanes[3].Ny, pFrustumPlanes[3].Nz);
// 
// 	//if((v | pFrustumPlanes[0].n) < 0 ) vc |= 0x10;
// 	if ((v | vp1) < 0 ) vc |= 0x01;
// 	if ((v | vp2) < 0 ) vc |= 0x02;
// 	if ((v | vp3) < 0 ) vc |= 0x04;
// 	if ((v | vp4) < 0 ) vc |= 0x08;
// 
// 	*/
// 
// 
// 	//Max - так точность значительно выше, блоки не должны пропадать иногда в далеке
// 	Plane * p = &pFrustumPlanes[0];
// 
// 	if(p->n.x*vP.x + p->n.y*vP.y + p->n.z*vP.z < p->D) vc |= 0x01;
// 	p = &pFrustumPlanes[1];
// 	if(p->n.x*vP.x + p->n.y*vP.y + p->n.z*vP.z < p->D) vc |= 0x02;
// 	p = &pFrustumPlanes[2];
// 	if(p->n.x*vP.x + p->n.y*vP.y + p->n.z*vP.z < p->D) vc |= 0x04;
// 	p = &pFrustumPlanes[3];
// 	if(p->n.x*vP.x + p->n.y*vP.y + p->n.z*vP.z < p->D) vc |= 0x08;
// 
// 	return vc;
// }
// 
// bool SEA::isVisibleBBox(const Vector & vCenter, const Vector & _v1, const Vector & _v2)
// {
// 	/*CVECTOR vc = vCenter - vCamPos;
// 	CVECTOR vp1 = v1 - vCamPos;
// 	CVECTOR vp2 = v2 - vCamPos;
// 	float fR2 = sqrtf(Sqr((vp1.x - vp2.x) * 0.5f) + Sqr((vp1.z - vp2.z)) * 0.5f);*/
// 
// 	// if sphere not visible - return
// 	//  for (dword i=0; i<dwNumFrustumPlanes; i++)
// 	//    if ((pFrustumPlanes[i].n | vc) - pFrustumPlanes[i].d < -fR2) return false;
// 
// 
// 	//joker - один хер все глючит, пока будет фейк
// 
// 	Vector v1 = _v1;
// 	Vector v2 = _v2;
// 
// 
// 	float addSize = 50.0f;
// 
// 	if (v1.x < v2.x)
// 	{
// 		v1.x -= addSize;
// 		v2.x += addSize;
// 	} else
// 	{
// 		v1.x += addSize;
// 		v2.x -= addSize;
// 	}
// 
// 	if (v1.y < v2.y)
// 	{
// 		v1.y -= addSize;
// 		v2.y += addSize;
// 	} else
// 	{
// 		v1.y += addSize;
// 		v2.y -= addSize;
// 	}
// 
// 	if (v1.z < v2.z)
// 	{
// 		v1.z -= addSize;
// 		v2.z += addSize;
// 	} else
// 	{
// 		v1.z += addSize;
// 		v2.z -= addSize;
// 	}
// 
// 
// 
// 	// check box visible
// 	long vc = 0xFF;
// 	vc &= VisCode(Vector(v1.x, v1.y, v1.z)); if (vc == 0) return true;
// 	vc &= VisCode(Vector(v1.x, v2.y, v1.z)); 
// 	vc &= VisCode(Vector(v2.x, v1.y, v1.z)); 
// 	vc &= VisCode(Vector(v2.x, v2.y, v1.z)); if (vc == 0) return true;
// 	vc &= VisCode(Vector(v1.x, v1.y, v2.z)); 
// 	vc &= VisCode(Vector(v1.x, v2.y, v2.z)); 
// 	vc &= VisCode(Vector(v2.x, v1.y, v2.z)); 
// 	vc &= VisCode(Vector(v2.x, v2.y, v2.z)); 
// 
// 	return vc == 0;
// }
// 
// inline float SEA::CalcLod(const float & x, const float & y, const float & z)
// {
// 	return Sqr(x - vCamPos.x) + /*Sqr((y - vCamPos.y)) + */Sqr(z - vCamPos.z);
// }
// 
// void SEA::CalculateLOD(const Vector & v1, const Vector & v2, long & iMaxLOD, long & iMinLOD)
// {
// 	float	fCur, fMax, fMin;
// 
// 	fCur = CalcLod(v1.x, vSeaCenterPos.y, v1.z); fMax = fCur;
// 	fCur = CalcLod(v2.x, vSeaCenterPos.y, v1.z); if (fCur > fMax) fMax = fCur;
// 	fCur = CalcLod(v1.x, vSeaCenterPos.y, v2.z); if (fCur > fMax) fMax = fCur;
// 	fCur = CalcLod(v2.x, vSeaCenterPos.y, v2.z); if (fCur > fMax) fMax = fCur;
// 
// 	if (vCamPos.x < v1.x)
// 	{
// 		if (vCamPos.z < v1.z)
// 			fMin = CalcLod(v1.x, vSeaCenterPos.y, v1.z);
// 		else
// 		if (vCamPos.z > v2.z)
// 			fMin = CalcLod(v1.x, vSeaCenterPos.y, v2.z);
// 		else
// 			fMin = CalcLod(v1.x, vSeaCenterPos.y, vCamPos.z);
// 	}
// 	else
// 	if (vCamPos.x > v2.x)
// 	{
// 		if (vCamPos.z < v1.z)
// 			fMin = CalcLod(v2.x, vSeaCenterPos.y, v1.z);
// 		else
// 		if (vCamPos.z > v2.z)
// 			fMin = CalcLod(v2.x, vSeaCenterPos.y, v2.z);
// 		else
// 			fMin = CalcLod(v2.x, vSeaCenterPos.y, vCamPos.z);
// 	}
// 	else
// 	{
// 		if (vCamPos.z < v1.z)
// 			fMin = CalcLod(vCamPos.x, vSeaCenterPos.y, v1.z);
// 		else
// 		if (vCamPos.z > v2.z)
// 			fMin = CalcLod(vCamPos.x, vSeaCenterPos.y, v2.z);
// 		else
// 			fMin = CalcLod(vCamPos.x, vSeaCenterPos.y, vCamPos.z);
// 	}
// 
// 	iMaxLOD = int(0.5f * logf(fLodScale * fMax) / logf(2)); 
// 	if (iMaxLOD < 4) iMaxLOD = 4;
// 	iMinLOD = int(0.5f * logf(fLodScale * fMin) / logf(2)); 
// 	if (iMinLOD < 4) iMinLOD = 4;
// }
// 
// void SEA::AddBlock(long iTX, long iTY, long iSize, long iLOD)
// {
// 	SeaBlock * pB = &aBlocks[aBlocks.Add()];
// 
// 	pB->iTX = iTX;		
// 	pB->iTY = iTY;
// 	pB->iSize = iSize;
// 	pB->iLOD = iLOD;
// 
// 	pB->bInProgress = false;
// 	pB->bDone = false;
// 
// 	pB->iSize0 = iSize >> iLOD;
// }
// 
// void SEA::BuildTree(long iTX, long iTY, long iLev)
// {
// 	long	iMaxLOD, iMinLOD;
// 	long	iSize = long(dwMaxDim >> iLev);
// 	float	fGSize = fGridStep * iSize;
// 
// 	Vector v1 = vSeaCenterPos + Vector(iTX * fGSize, -fMaxSeaHeight / 2.0f, iTY * fGSize);
// 	Vector v2 = v1 + Vector(fGSize, fMaxSeaHeight, fGSize);
// 
// 
// 	
// 
// 	if (!isVisibleBBox(vSeaCenterPos, v1, v2)) return;
// 
// 	CalculateLOD(v1, v2, iMaxLOD, iMinLOD);
// 
// 	if (iSize <= long(dwMinDim) || iMaxLOD - iMinLOD <= 1)
// 	{
// 		AddBlock(iTX, iTY, iSize, iMinLOD);
// 		return;
// 	}
// 
// 	iTX *= 2; 
// 	iTY *= 2; 
// 	iLev++;
// 
// 	BuildTree(iTX    , iTY    , iLev);
// 	BuildTree(iTX + 1, iTY    , iLev);
// 	BuildTree(iTX    , iTY + 1, iLev);
// 	BuildTree(iTX + 1, iTY + 1, iLev);
// }
// 
// inline void PrefetchNTA(dword dwAddress)
// {
// 	/*_asm
// 	{
// 		mov	eax, dwAddress
// 		and esi, ~15d
// 		//add esi, 128d
// 		prefetchnta [esi]
// 	}*/
// }
// 
// #ifndef _XBOX 
// 
// // INTEL COMMENT:
// // This version of the function takes in 4 at a time, to take advantage of SSE.
// // In particular I have converted the normalise and square roots over to SSE.
// void SEA::SSE_WaveXZ(SeaVertex * * pArray)
// {
// 	Vector vNormal[4];
// 	float nY1[4];
// 	float nY2[4];
// 	bool bDistancePass = false;
// 
// 	for( int i=0; i<4; i++ )
// 	{
// 		long iX11, iX12, iX21, iX22, iY11, iY12, iY21, iY22;
// 
// 		float fDistance = Sqr(pArray[i]->vPos.x - vCamPos.x) + Sqr(pArray[i]->vPos.z - vCamPos.z);
// 		if( fDistance > 900.0f * 900.0f )
// 		{
// 			vNormal[i].x = 0.0f;
// 			nY1[i] = 1.0f;
// 			nY2[i] = 1.0f;
// 			vNormal[i].y = 1.0f;
// 			vNormal[i].z = 0.0f;
// 			pArray[i]->vPos.y = 0.0f;
// 			continue;
// 		}
// 
// 		bDistancePass = true;
// 
// 		float x1 = (pArray[i]->vPos.x + vMove1.x) * fScale1;
// 		float z1 = (pArray[i]->vPos.z + vMove1.z) * fScale1;
// 		iX11 = ffloor(x1 + 0.0f), iX12 = iX11 + 1; iY11 = ffloor(z1 + 0.0f), iY12 = iY11 + 1; 
// 		float fX1 = (x1 - iX11);
// 		float fZ1 = (z1 - iY11);
// 		iX11 &= (XWIDTH-1); iX12 &= (XWIDTH-1); iY11 &= (XWIDTH-1); iY12 &= (XWIDTH-1);
// 
// 		float x2 = (pArray[i]->vPos.x + vMove2.x) * fScale2;
// 		float z2 = (pArray[i]->vPos.z + vMove2.z) * fScale2;
// 		iX21 = ffloor(x2 + 0.0f), iX22 = iX21 + 1; iY21 = ffloor(z2 + 0.0f), iY22 = iY21 + 1; 
// 		float fX2 = (x2 - iX21);
// 		float fZ2 = (z2 - iY21);
// 		iX21 &= (XWIDTH-1); iX22 &= (XWIDTH-1); iY21 &= (XWIDTH-1); iY22 &= (XWIDTH-1);
// 
// 		float a1, a2, a3, a4;
// 
// 		a1 = pSeaFrame1[iX11 + iY11 * XWIDTH]; a2 = pSeaFrame1[iX12 + iY11 * XWIDTH];
// 		a3 = pSeaFrame1[iX11 + iY12 * XWIDTH]; a4 = pSeaFrame1[iX12 + iY12 * XWIDTH];
// 		pArray[i]->vPos.y = (fAmp1 * (a1 + fX1 * (a2 - a1) + fZ1 * (a3 - a1) + fX1 * fZ1 * (a4 + a1 - a2 - a3)) + fSeaHeight);
// 
// 		a1 = pSeaFrame2[iX21 + iY21 * XWIDTH]; a2 = pSeaFrame2[iX22 + iY21 * XWIDTH];
// 		a3 = pSeaFrame2[iX21 + iY22 * XWIDTH]; a4 = pSeaFrame2[iX22 + iY22 * XWIDTH];
// 		pArray[i]->vPos.y += (fAmp2 * (a1 + fX2 * (a2 - a1) + fZ2 * (a3 - a1) + fX2 * fZ2 * (a4 + a1 - a2 - a3)) + fSeaHeight);
// 
// 		float nx1, nx2, nx3, nx4, nz1, nz2, nz3, nz4;
// 
// 		nx1 = pSeaNormalsFrame1[2 * (iX11 + iY11 * XWIDTH) + 0]; nz1 = pSeaNormalsFrame1[2 * (iX11 + iY11 * XWIDTH) + 1];
// 		nx2 = pSeaNormalsFrame1[2 * (iX12 + iY11 * XWIDTH) + 0]; nz2 = pSeaNormalsFrame1[2 * (iX12 + iY11 * XWIDTH) + 1];
// 		nx3 = pSeaNormalsFrame1[2 * (iX11 + iY12 * XWIDTH) + 0]; nz3 = pSeaNormalsFrame1[2 * (iX11 + iY12 * XWIDTH) + 1];
// 		nx4 = pSeaNormalsFrame1[2 * (iX12 + iY12 * XWIDTH) + 0]; nz4 = pSeaNormalsFrame1[2 * (iX12 + iY12 * XWIDTH) + 1];
// 
// 		float nX1 = (nx1 + fX1 * (nx2 - nx1) + fZ1 * (nx3 - nx1) + fX1 * fZ1 * (nx4 + nx1 - nx2 - nx3));
// 		float nZ1 = (nz1 + fX1 * (nz2 - nz1) + fZ1 * (nz3 - nz1) + fX1 * fZ1 * (nz4 + nz1 - nz2 - nz3));
// 
// 		nx1 = pSeaNormalsFrame2[2 * (iX21 + iY21 * XWIDTH) + 0]; nz1 = pSeaNormalsFrame2[2 * (iX21 + iY21 * XWIDTH) + 1];
// 		nx2 = pSeaNormalsFrame2[2 * (iX22 + iY21 * XWIDTH) + 0]; nz2 = pSeaNormalsFrame2[2 * (iX22 + iY21 * XWIDTH) + 1];
// 		nx3 = pSeaNormalsFrame2[2 * (iX21 + iY22 * XWIDTH) + 0]; nz3 = pSeaNormalsFrame2[2 * (iX21 + iY22 * XWIDTH) + 1];
// 		nx4 = pSeaNormalsFrame2[2 * (iX22 + iY22 * XWIDTH) + 0]; nz4 = pSeaNormalsFrame2[2 * (iX22 + iY22 * XWIDTH) + 1];
// 
// 		float nX2 = (nx1 + fX2 * (nx2 - nx1) + fZ2 * (nx3 - nx1) + fX2 * fZ2 * (nx4 + nx1 - nx2 - nx3));
// 		float nZ2 = (nz1 + fX2 * (nz2 - nz1) + fZ2 * (nz3 - nz1) + fX2 * fZ2 * (nz4 + nz1 - nz2 - nz3));
// 
// 		nY1[i] = 1.0f - (Sqr(nX1) + Sqr(nZ1));
// 		nY2[i] = 1.0f - (Sqr(nX2) + Sqr(nZ2));
// 
// 		vNormal[i].x = fScale1 * fAmp1 * nX1 + fScale2 * fAmp2 * nX2;
// 		vNormal[i].z = fScale1 * fAmp1 * nZ1 + fScale2 * fAmp2 * nZ2;
// 	}
// 
// 	if( bDistancePass )
// 	{
// 		// INTEL COMMENT:
// 		// xmm variables.
// 		__m128 m128X, m128Y, m128Z;
// 		__m128 m128NX, m128NY, m128NZ;
// 		__m128 m128Y1, m128Y2;
// 		__m128 m128SY1, m128SY2;
// 		Vector n1, n2, n3, n4;
// 
// 
// 		// INTEL COMMENT:
// 		// Gather the Y's.
// 		SSE_GatherFourFloats( &nY1[0], &nY1[1], &nY1[2], &nY1[3], &m128Y1 ); 
// 		SSE_GatherFourFloats( &nY2[0], &nY2[1], &nY2[2], &nY2[3], &m128Y2 ); 
// 		m128SY1 = _mm_sqrt_ps( m128Y1 );
// 		m128SY2 = _mm_sqrt_ps( m128Y2 );
// 		m128Y = _mm_add_ps( m128SY1, m128SY2 );
// 
// 		// INTEL COMMENT:
// 		// Gather the X's and Z's.
// 		SSE_GatherFourFloats( &vNormal[0].x, &vNormal[1].x, &vNormal[2].x, &vNormal[3].x, &m128X ); 
// 		SSE_GatherFourFloats( &vNormal[0].z, &vNormal[1].z, &vNormal[2].z, &vNormal[3].z, &m128Z );
// 
// 		// INTEL COMMENT:
// 		// Normalise.
// 		SSE_Normalise( m128X, m128Y, m128Z, &m128NX, &m128NY, &m128NZ );
// 
// 		// INTEL COMMENT:
// 		// Scatter back out.
// 		SSE_ScatterFourFloats( &n1.x, &n2.x, &n3.x, &n4.x, m128NX ); 
// 		SSE_ScatterFourFloats( &n1.y, &n2.y, &n3.y, &n4.y, m128NY ); 
// 		SSE_ScatterFourFloats( &n1.z, &n2.z, &n3.z, &n4.z, m128NZ ); 
// 
// 		pArray[0]->dwPackedNormal = PackNormal(n1);
// 		pArray[1]->dwPackedNormal = PackNormal(n2);
// 		pArray[2]->dwPackedNormal = PackNormal(n3);
// 		pArray[3]->dwPackedNormal = PackNormal(n4);
// 	}
// 	else
// 	{
// 		pArray[0]->dwPackedNormal = PackNormal(vNormal[0]); 
// 		pArray[1]->dwPackedNormal = PackNormal(vNormal[1]); 
// 		pArray[2]->dwPackedNormal = PackNormal(vNormal[2]); 
// 		pArray[3]->dwPackedNormal = PackNormal(vNormal[3]); 
// 	}
// }
// 
// #endif
// 
// float __fastcall SEA::WaveXZ(float x, float z, Vector * pNormal)
// {
// 	long iX11, iX12, iX21, iX22, iY11, iY12, iY21, iY22;
// 
// 	float fDistance = Sqr(x - vCamPos.x) + Sqr(z - vCamPos.z);
// 	if (fDistance > 900.0f * 900.0f)
// 	{
// 		if (pNormal) *pNormal = Vector(0.0f, 1.0f, 0.0f);
// 		return 0.0f;
// 	}
// 
// 	float x1 = (x + vMove1.x) * fScale1;
// 	float z1 = (z + vMove1.z) * fScale1;
// 	iX11 = ffloor(x1 + 0.0f), iX12 = iX11 + 1; iY11 = ffloor(z1 + 0.0f), iY12 = iY11 + 1; 
// 	float fX1 = (x1 - iX11);
// 	float fZ1 = (z1 - iY11);
// 	iX11 &= (XWIDTH-1); iX12 &= (XWIDTH-1); iY11 &= (XWIDTH-1); iY12 &= (XWIDTH-1);
// 
// 	float x2 = (x + vMove2.x) * fScale2;
// 	float z2 = (z + vMove2.z) * fScale2;
// 	iX21 = ffloor(x2 + 0.0f), iX22 = iX21 + 1; iY21 = ffloor(z2 + 0.0f), iY22 = iY21 + 1; 
// 	float fX2 = (x2 - iX21);
// 	float fZ2 = (z2 - iY21);
// 	iX21 &= (XWIDTH-1); iX22 &= (XWIDTH-1); iY21 &= (XWIDTH-1); iY22 &= (XWIDTH-1);
// 
// 	float a1, a2, a3, a4;
// 
// 	a1 = pSeaFrame1[iX11 + iY11 * XWIDTH]; a2 = pSeaFrame1[iX12 + iY11 * XWIDTH];
// 	a3 = pSeaFrame1[iX11 + iY12 * XWIDTH]; a4 = pSeaFrame1[iX12 + iY12 * XWIDTH];
// 	float fRes = fAmp1 * (a1 + fX1 * (a2 - a1) + fZ1 * (a3 - a1) + fX1 * fZ1 * (a4 + a1 - a2 - a3));
// 
// 	a1 = pSeaFrame2[iX21 + iY21 * XWIDTH]; a2 = pSeaFrame2[iX22 + iY21 * XWIDTH];
// 	a3 = pSeaFrame2[iX21 + iY22 * XWIDTH]; a4 = pSeaFrame2[iX22 + iY22 * XWIDTH];
// 	fRes += fAmp2 * (a1 + fX2 * (a2 - a1) + fZ2 * (a3 - a1) + fX2 * fZ2 * (a4 + a1 - a2 - a3));
// 
// 	float nx1, nx2, nx3, nx4, nz1, nz2, nz3, nz4;
// 
// 	nx1 = pSeaNormalsFrame1[2 * (iX11 + iY11 * XWIDTH) + 0]; nz1 = pSeaNormalsFrame1[2 * (iX11 + iY11 * XWIDTH) + 1];
// 	nx2 = pSeaNormalsFrame1[2 * (iX12 + iY11 * XWIDTH) + 0]; nz2 = pSeaNormalsFrame1[2 * (iX12 + iY11 * XWIDTH) + 1];
// 	nx3 = pSeaNormalsFrame1[2 * (iX11 + iY12 * XWIDTH) + 0]; nz3 = pSeaNormalsFrame1[2 * (iX11 + iY12 * XWIDTH) + 1];
// 	nx4 = pSeaNormalsFrame1[2 * (iX12 + iY12 * XWIDTH) + 0]; nz4 = pSeaNormalsFrame1[2 * (iX12 + iY12 * XWIDTH) + 1];
// 
// 	float nX1 = (nx1 + fX1 * (nx2 - nx1) + fZ1 * (nx3 - nx1) + fX1 * fZ1 * (nx4 + nx1 - nx2 - nx3));
// 	float nZ1 = (nz1 + fX1 * (nz2 - nz1) + fZ1 * (nz3 - nz1) + fX1 * fZ1 * (nz4 + nz1 - nz2 - nz3));
// 
// 	nx1 = pSeaNormalsFrame2[2 * (iX21 + iY21 * XWIDTH) + 0]; nz1 = pSeaNormalsFrame2[2 * (iX21 + iY21 * XWIDTH) + 1];
// 	nx2 = pSeaNormalsFrame2[2 * (iX22 + iY21 * XWIDTH) + 0]; nz2 = pSeaNormalsFrame2[2 * (iX22 + iY21 * XWIDTH) + 1];
// 	nx3 = pSeaNormalsFrame2[2 * (iX21 + iY22 * XWIDTH) + 0]; nz3 = pSeaNormalsFrame2[2 * (iX21 + iY22 * XWIDTH) + 1];
// 	nx4 = pSeaNormalsFrame2[2 * (iX22 + iY22 * XWIDTH) + 0]; nz4 = pSeaNormalsFrame2[2 * (iX22 + iY22 * XWIDTH) + 1];
// 
// 	float nX2 = (nx1 + fX2 * (nx2 - nx1) + fZ2 * (nx3 - nx1) + fX2 * fZ2 * (nx4 + nx1 - nx2 - nx3));
// 	float nZ2 = (nz1 + fX2 * (nz2 - nz1) + fZ2 * (nz3 - nz1) + fX2 * fZ2 * (nz4 + nz1 - nz2 - nz3));
// 	//float nX2 = 0.0f;
// 	//float nZ2 = 0.0f;
// 
// 	/*float fDistance = sqrt_ss(Sqr(x - vCamPos.x) + Sqr(fRes - vCamPos.y) + Sqr(z - vCamPos.z));
// 	{
// 		float distance_mul = 1.0f - fDistance / 1900.0f;
// 		if (distance_mul < 0.0f) distance_mul = 0.0f;
// 
// 		fRes *= distance_mul;
// 
// 		nX1 *= distance_mul;
// 		nZ1 *= distance_mul;
// 
// 		nX2 *= distance_mul;
// 		nZ2 *= distance_mul;
// 	}*/
// 
// 	if (pNormal)
// 	{
// 		/*pNormal->x = 0.0f;
// 		pNormal->y = 1.0f;
// 		pNormal->z = 0.0f;*/
// 		float nY1 = sqrtf(1.0f - (Sqr(nX1) + Sqr(nZ1)));
// 		float nY2 = sqrtf(1.0f - (Sqr(nX2) + Sqr(nZ2)));
// 
// 		Vector vNormal;
// 
// 		vNormal.x = fScale1 * fAmp1 * nX1 + fScale2 * fAmp2 * nX2;
// 		vNormal.z = fScale1 * fAmp1 * nZ1 + fScale2 * fAmp2 * nZ2;
// 		vNormal.y = nY1 + nY2;
// 
// 		//vNormal.x += 2.0f * vNormal.x;
// 		//vNormal.z += 2.0f * vNormal.z;
// 		vNormal = !vNormal;
// 
// 		*pNormal = vNormal;
// 	}
// 
// 	return (fRes+fSeaHeight);
// }
// 
// void SEA::PrepareIndicesForBlock(dword dwBlockIndex)
// {
// 	SeaBlock * pB = &aBlocks[dwBlockIndex];
// 
// 	float	fStep = fGridStep * float(1 << pB->iLOD);
// 	float	fSize = fGridStep * pB->iSize;
// 	long	x, y, size0 = pB->iSize >> pB->iLOD;
// 
// 	pB->iIStart = iIStart;
// 
// 	float x1 = float(pB->iTX * pB->iSize) * fGridStep;
// 	float y1 = float(pB->iTY * pB->iSize) * fGridStep;
// 	float x2 = x1 + float(size0) * fStep;
// 	float y2 = y1 + float(size0) * fStep;
// 
// 	pB->iX1 = fftoi(x1 / fGridStep); 
// 	pB->iX2 = fftoi(x2 / fGridStep); 
// 	pB->iY1 = fftoi(y1 / fGridStep); 
// 	pB->iY2 = fftoi(y2 / fGridStep); 
// 
// 	// analyse 
// 	long i, j;
// 	for (i=0; i<long(dwBlockIndex); i++) //if (i == 6)
// 	{
// 		SeaBlock * pB2 = &aBlocks[i];
// 
// 		// Test Up & Down
// 		bool bTestedUp = pB->iY1 == pB2->iY2;
// 		bool bTestedDown = pB->iY2 == pB2->iY1;
// 
// 		//if (!(GetAsyncKeyState('5')<0))
// 		if (bTestedUp || bTestedDown)
// 		{
// 			long iAddSrc = pB2->iIStart + ((bTestedUp) ? (pB2->iSize0 + 1) * pB2->iSize0 : 0);
// 			long iAddDst = pB->iIStart + ((bTestedUp) ? 0 : (pB->iSize0 + 1) * pB->iSize0);
// 
// 			if ((pB->iX1 >= pB2->iX1 && pB->iX2 <= pB2->iX2) || (pB->iX1 <= pB2->iX1 && pB->iX2 >= pB2->iX2))
// 			{
// 				long iMinX = Max(pB->iX1, pB2->iX1);
// 				long iMaxX = Min(pB->iX2, pB2->iX2);
// 				
// 				long iStartDstX = pB->iSize0 * (iMinX - pB->iX1) / (pB->iX2 - pB->iX1);
// 				long iStartSrcX = pB2->iSize0 * (iMinX - pB2->iX1) / (pB2->iX2 - pB2->iX1);
// 
// 				long iEndDstX = pB->iSize0 * (iMaxX - pB->iX1) / (pB->iX2 - pB->iX1);
// 				long iEndSrcX = pB2->iSize0 * (iMaxX - pB2->iX1) / (pB2->iX2 - pB2->iX1);
// 
// 				if (pB->iLOD == pB2->iLOD)
// 					for (j=iStartDstX; j<=iEndDstX; j++) 
// 						pIndices[iAddDst + j] = pIndices[iAddSrc + iStartSrcX + (j - iStartDstX)];
// 				else
// 					for (j=iStartDstX; j<=iEndDstX; j++) 
// 						pIndices[iAddDst + j] = pIndices[iAddSrc + iStartSrcX + (j - iStartDstX) / 2];
// 
// 				continue;
// 			}
// 		}
// 
// 		// Test Left & Right
// 		bool bTestedLeft = pB->iX1 == pB2->iX2;
// 		bool bTestedRight = pB->iX2 == pB2->iX1;
// 		//if ((GetAsyncKeyState('6')<0))
// 		if (bTestedLeft || bTestedRight)
// 		{
// 			long iAddSrc = pB2->iIStart + ((bTestedLeft) ? (pB2->iSize0) : 0);
// 			long iAddDst = pB->iIStart + ((bTestedLeft) ? 0 : (pB->iSize0));
// 
// 			if ((pB->iY1 >= pB2->iY1 && pB->iY2 <= pB2->iY2) || (pB->iY1 <= pB2->iY1 && pB->iY2 >= pB2->iY2))
// 			{
// 				long iMinY = Max(pB->iY1, pB2->iY1);
// 				long iMaxY = Min(pB->iY2, pB2->iY2);
// 				
// 				long iStartDstY = pB->iSize0 * (iMinY - pB->iY1) / (pB->iY2 - pB->iY1);
// 				long iStartSrcY = pB2->iSize0 * (iMinY - pB2->iY1) / (pB2->iY2 - pB2->iY1);
// 
// 				long iEndDstY = pB->iSize0 * (iMaxY - pB->iY1) / (pB->iY2 - pB->iY1);
// 				long iEndSrcY = pB2->iSize0 * (iMaxY - pB2->iY1) / (pB2->iY2 - pB2->iY1);
// 
// 				if (pB->iLOD == pB2->iLOD)
// 					for (j=iStartDstY; j<=iEndDstY; j++) 
// 						pIndices[iAddDst + j * (pB->iSize0 + 1)] = pIndices[iAddSrc + (iStartSrcY + j - iStartDstY) * (pB2->iSize0 + 1)];
// 				else
// 					for (j=iStartDstY; j<=iEndDstY; j++) 
// 						pIndices[iAddDst + j * (pB->iSize0 + 1)] = pIndices[iAddSrc + (iStartSrcY + (j - iStartDstY) / 2) * (pB2->iSize0 + 1)];
// 
// 				continue;
// 			}
// 		}
// 	}
// 
// 	long iIFirst = 1000000, iILast = -1000000;
// 
// 	for (y=0; y<=size0; y++)
// 		for (x=0; x<=size0; x++)
// 		{
// 			if (pIndices[iIStart] != dword(-1))
// 			{
// 				iIStart++;
// 				continue;
// 			}
// 
// 			if (iVStart < iIFirst) iIFirst = iVStart;
// 			if (iVStart > iILast) iILast = iVStart;
// 
// 			pIndices[iIStart++] = dword(iVStart);
// 			iVStart++;
// 		}
// 
// 	pB->iIFirst = iIFirst;
// 	pB->iILast = iILast;
// 
// 	// setup triangles
// 	long yy, dyy = size0 + 1;
// 	for(y=0, yy = 0; y<size0; y++, yy += dyy)
// 		for(x=0; x<size0; x++) 
// 		{
// 			// first triangle
// 			*pTriangles++ = (word)pIndices[pB->iIStart + dword(x + yy + dyy + 1)];
// 			*pTriangles++ = (word)pIndices[pB->iIStart + dword(x + yy + 1)];
// 			*pTriangles++ = (word)pIndices[pB->iIStart + dword(x + yy)];
// 			// second triangle
// 			*pTriangles++ = (word)pIndices[pB->iIStart + dword(x + yy + dyy)];
// 			*pTriangles++ = (word)pIndices[pB->iIStart + dword(x + yy + dyy + 1)];
// 			*pTriangles++ = (word)pIndices[pB->iIStart + dword(x + yy)];
// 			
// 			iTStart += 2;
// 		}
// }
// 
// #ifndef _XBOX 
// void SEA::SSE_WaveXZBlock(SeaBlock * pB)
// {
// 	//return;
// 	if (!pB) return;
// 
// 	SeaVertex	* vTmp[4];
// 	SeaVertex	vFake;
// 
// 	vFake.vPos.x = vCamPos.x + 1e+5f;
// 	vFake.vPos.z = vCamPos.z + 1e+5f;
// 
// 	float	cx, cz, fStep = fGridStep * float(1 << pB->iLOD);
// 	float	fSize = fGridStep * pB->iSize;
// 	long	x, y, size0 = pB->iSize >> pB->iLOD;
// 	
// 	float x1 = float(pB->iTX * pB->iSize) * fGridStep;
// 	float y1 = float(pB->iTY * pB->iSize) * fGridStep;
// 	float x2 = x1 + float(size0) * fStep;
// 	float y2 = y1 + float(size0) * fStep;
// 
// 	pB->iX1 = fftoi(x1 / fGridStep); 
// 	pB->iX2 = fftoi(x2 / fGridStep); 
// 	pB->iY1 = fftoi(y1 / fGridStep); 
// 	pB->iY2 = fftoi(y2 / fGridStep); 
// 
// 	x1 += vSeaCenterPos.x;	x2 += vSeaCenterPos.x;
// 	y1 += vSeaCenterPos.z;	y2 += vSeaCenterPos.z;
// 
// 	//Vector vNormal, vTmp;
// 	long iCurrentV = 0;
// 
// 	long iIStart1 = pB->iIStart;
// 	long iIFirst = pB->iIFirst;
// 	long iILast = pB->iILast;
// 
// 	// calculate
// 	for (cz = y1, y=0; y<=size0; y++, cz+=fStep)
// 	{
// 		for (cx = x1, x=0; x<=size0; x++, cx+=fStep)
// 		{
// 			dword dwVIndex = pIndices[iIStart1];
// 
// 			if (long(dwVIndex) < iIFirst || long(dwVIndex) > iILast)
// 			{
// 				iIStart1++;
// 				continue;
// 			}
// 
// 			vTmp[iCurrentV] = &pVSea[dwVIndex];
// 			vTmp[iCurrentV]->vPos.x = cx;
// 			vTmp[iCurrentV]->vPos.z = cz;
// 			
// 			iCurrentV++;
// 
// 			if (iCurrentV == 4)
// 			{
// 				SSE_WaveXZ(&vTmp[0]);
// 				iCurrentV = 0;
// 
// 				for (long i=0; i<4; i++)
// 				{
// 					/*
// 					vTmp[i]->tu = (vTmp[i]->vPos.x - vTmp[i]->vNormal.x * 5.3f) * fBumpScale;
// 					vTmp[i]->tv = (vTmp[i]->vPos.z - vTmp[i]->vNormal.z * 5.3f) * fBumpScale;
// 					*/
// 					
// 
// 					vTmp[i]->vPos.x -= UnpackNormalX(vTmp[i]->dwPackedNormal) * fPosShift * 3.0f;
// 					vTmp[i]->vPos.z -= UnpackNormalZ(vTmp[i]->dwPackedNormal) * fPosShift * 3.0f;
// 				}
// 			}
// 			
// 			/*vTmp.y = WaveXZ(cx, cz, &vNormal);
// 
// 			vTmp.x = cx - vNormal.x * fPosShift * 3.0f;
// 			vTmp.z = cz - vNormal.z * fPosShift * 3.0f;
// 
// 			pVSea[dwVIndex].vPos = vTmp;
// 			pVSea[dwVIndex].vNormal = vNormal;
// 			pVSea[dwVIndex].tu = (cx - vNormal.x * 5.3f) * fBumpScale;
// 			pVSea[dwVIndex].tv = (cz - vNormal.z * 5.3f) * fBumpScale;*/
// 
// 			iIStart1++;
// 		}
// 	}
// 
// 	if (iCurrentV != 0) 
// 	{
// 		for (long i=iCurrentV; i<4; i++) vTmp[i] = &vFake;
// 
// 		SSE_WaveXZ(&vTmp[0]);
// 		for (long i=0; i<iCurrentV; i++)
// 		{
// /*
// 			vTmp[i]->tu = (vTmp[i]->vPos.x - vTmp[i]->vNormal.x * 5.3f) * fBumpScale;
// 			vTmp[i]->tv = (vTmp[i]->vPos.z - vTmp[i]->vNormal.z * 5.3f) * fBumpScale;
// */
// 
// 			
// 			vTmp[i]->vPos.x -= UnpackNormalX(vTmp[i]->dwPackedNormal) * fPosShift * 3.0f;
// 			vTmp[i]->vPos.z -= UnpackNormalZ(vTmp[i]->dwPackedNormal) * fPosShift * 3.0f;
// 		}
// 	}
// 
// 	pB->bDone = true;
// 
// 	EnterCriticalSection(&cs1);
// 	iBlocksDoneNum++;
// 	LeaveCriticalSection(&cs1);
// }
// #endif
// 
// void SEA::WaveXZBlock(SeaBlock * pB)
// {
// 	//return;
// 	if (!pB) return;
// 
// 	float	cx, cz, fStep = fGridStep * float(1 << pB->iLOD);
// 	float	fSize = fGridStep * pB->iSize;
// 	long	x, y, size0 = pB->iSize >> pB->iLOD;
// 	
// 	float x1 = float(pB->iTX * pB->iSize) * fGridStep;
// 	float y1 = float(pB->iTY * pB->iSize) * fGridStep;
// 	float x2 = x1 + float(size0) * fStep;
// 	float y2 = y1 + float(size0) * fStep;
// 
// 	pB->iX1 = fftoi(x1 / fGridStep); 
// 	pB->iX2 = fftoi(x2 / fGridStep); 
// 	pB->iY1 = fftoi(y1 / fGridStep); 
// 	pB->iY2 = fftoi(y2 / fGridStep); 
// 
// 	x1 += vSeaCenterPos.x;	x2 += vSeaCenterPos.x;
// 	y1 += vSeaCenterPos.z;	y2 += vSeaCenterPos.z;
// 
// 	Vector vNormal, vTmp;
// 
// 	long iIStart1 = pB->iIStart;
// 	long iIFirst = pB->iIFirst;
// 	long iILast = pB->iILast;
// 
// 	// calculate
// 	for (cz = y1, y=0; y<=size0; y++, cz+=fStep)
// 	{
// 		for (cx = x1, x=0; x<=size0; x++, cx+=fStep)
// 		{
// 			dword dwVIndex = pIndices[iIStart1];
// 
// 			if (long(dwVIndex) < iIFirst || long(dwVIndex) > iILast)
// 			{
// 				iIStart1++;
// 				continue;
// 			}
// 			
// 			vTmp.y = WaveXZ(cx, cz, &vNormal);
// 
// 			vTmp.x = cx - vNormal.x * fPosShift * 3.0f;
// 			vTmp.z = cz - vNormal.z * fPosShift * 3.0f;
// 
// 			pVSea[dwVIndex].vPos = vTmp;
// 			pVSea[dwVIndex].dwPackedNormal = PackNormal (vNormal);
// /*
// 			pVSea[dwVIndex].tu = (cx - vNormal.x * 5.3f) * fBumpScale;
// 			pVSea[dwVIndex].tv = (cz - vNormal.z * 5.3f) * fBumpScale;
// */
// 
// 			iIStart1++;
// 		}
// 	}
// 
// 	pB->bDone = true;
// 
// 	EnterCriticalSection(&cs1);
// 	iBlocksDoneNum++;
// 	LeaveCriticalSection(&cs1);
// }
// 
// 
// SEA::SeaBlock * SEA::GetUndoneBlock()
// {
// 	SeaBlock * pB = null;
// 	for (long i=0; i<aBlocks; i++) if (!aBlocks[i].bInProgress)
// 	{
// 		pB = &aBlocks[i];
// 		pB->bInProgress = true;
// 		return pB;
// 	}
// 	return pB;
// }
// 
// dword SEA::ThreadExecute(long iThreadIndex)
// {
// 	HANDLE hHandles[] = {SEA::pSea->hEventCalcMaps, SEA::pSea->aEventCalcBlock[iThreadIndex]};
// 
// 	while (true)
// 	{
// 		dword dwValue = WaitForMultipleObjects(ARRSIZE(hHandles), hHandles, false, INFINITE);
// 
// 		if (dwValue >= WAIT_OBJECT_0 && dwValue < WAIT_OBJECT_0 + ARRSIZE(hHandles))
// 		{
// 			HANDLE hValue = hHandles[dwValue - WAIT_OBJECT_0];
// 			
// 			if (hValue == SEA::pSea->hEventCalcMaps)
// 			{
// 			}
// 
// 			if (hValue == SEA::pSea->aEventCalcBlock[iThreadIndex])
// 			{
// 				while (true)
// 				{
// 					EnterCriticalSection(&SEA::pSea->cs);
// 					SeaBlock * pB = SEA::pSea->GetUndoneBlock();
// 					LeaveCriticalSection(&SEA::pSea->cs);
// 
// 					if (!pB) break;
// 
// #ifndef _XBOX 
// 					if (SEA::bIntel && SEA::bSSE) SEA::pSea->SSE_WaveXZBlock(pB); else SEA::pSea->WaveXZBlock(pB);
// #endif
// 
// 					SEA::pSea->iB2++;
// 					SEA::pSea->aThreadsTest[iThreadIndex]++;
// 				}
// 			}
// 		}
// 	}
// }
// 
// void SEA::CalculateNormalMap(float fFrame, float fAmplitude, float * pfOut, array<dword*> & aFrames)
// {
// 	long iFrame1 = fftol(fFrame) % aFrames.Len();
// 	long iFrame2 = (iFrame1 + 1) % aFrames.Len();
// 
// 	float fDelta = fFrame - iFrame1;
// 
// 	dword * pB1 = aFrames[iFrame1];
// 	dword * pB2 = aFrames[iFrame2];
// 
// 	for (long y=0; y<YWIDTH; y++)
// 		for (long x=0; x<XWIDTH; x++)
// 		{
// 			dword dw1 = pB1[x + y * XWIDTH];
// 			dword dw2 = pB2[x + y * XWIDTH];
// 			float nx1 = float(short(dw1)) / 32767.5f;
// 			float nx2 = float(short(dw2)) / 32767.5f;
// 			float nz1 = float(short(dw1 >> 0x10)) / 32767.5f;
// 			float nz2 = float(short(dw2 >> 0x10)) / 32767.5f;
// 
// 			pfOut[2 * (x + y * XWIDTH) + 0] = (nx1 + (nx2 - nx1) * fDelta);
// 			pfOut[2 * (x + y * XWIDTH) + 1] = (nz1 + (nz2 - nz1) * fDelta);
// 		}
// }
// 
// void SEA::CalculateHeightMap(float fFrame, float fAmplitude, float * pfOut, array<byte*> & aFrames)
// {
// 	long iFrame1 = fftol(fFrame) % aFrames.Len();
// 	long iFrame2 = (iFrame1 + 1) % aFrames.Len();
// 
// 	float fDelta = fFrame - iFrame1;
// 
// 	byte * pB1 = aFrames[iFrame1];
// 	byte * pB2 = aFrames[iFrame2];
// 
// 	for (long y=0; y<YWIDTH; y++)
// 		for (long x=0; x<XWIDTH; x++)
// 		{
// 			float f1 = pB1[x + y * XWIDTH];
// 			float f2 = pB2[x + y * XWIDTH];
// 
// 			pfOut[x + y * XWIDTH] = fAmplitude * (f1 + (f2 - f1) * fDelta);
// 		}
// }
// 
// void _cdecl SEA::Realize(float fDeltaTime, long level)
// {
// 	if(!IsShow()) return;
// 
// 
// 	if (api->DebugKeyState(VK_SHIFT) && api->DebugKeyState('S'))
// 	{
// 		if (bTempFullMode)
// 		{
// 			bTempFullMode = false;
// 			fGridStep = fTempGridStep;
// 			fLodScale = fTempLodScale;
// 		}
// 		else
// 		{
// 			bTempFullMode = true;
// 			fTempGridStep = fGridStep;
// 			fTempLodScale = fLodScale;
// 
// 			fGridStep = 0.07f;
// 			fLodScale = 0.5f;
// 		}
// 
// 		Sleep(100);
// 	}
// 
// 	vMove1 += fDeltaTime * vMoveSpeed1;
// 	vMove2 += fDeltaTime * vMoveSpeed2;
// 
// 	fFrame1 += fDeltaTime * fAnimSpeed1;
// 	fFrame2 +=  fDeltaTime * fAnimSpeed2;
// 	while (fFrame1 >= FRAMES) fFrame1 -= FRAMES;
// 	while (fFrame2 >= FRAMES) fFrame2 -= FRAMES;
// 
// 	//pSeaParameters->SetTexture(0, pAnimTexture);
// 
// 	dword dwTotalRDTSC = 0;
// 
// #ifndef _XBOX 
// 	RDTSC_B(dwTotalRDTSC);
// #endif
// 	
// 	Matrix mView, mIView;
// 	mView = Render().GetView();
// 	mIView = mView;
// 	mIView.Inverse();
// 
// 	vCamPos = mView.GetCamPos();
// 
// 	BuildEnvironmentTextureMaps();
// 
// /*
// 	if (GetAsyncKeyState('M') < 0)
// 	{
// 		Render().SetTexture(0, pReflection);
// 		RS_SPRITE spr[4];
// 		spr[0].vPos = Vector (0.0f, 1.0f, 0.0f);
// 		spr[1].vPos = Vector (1.0f, 1.0f, 0.0f);
// 		spr[2].vPos = Vector (1.0f, 0.0f, 0.0f);
// 		spr[3].vPos = Vector (0.0f, 0.0f, 0.0f);
// 
// 		spr[0].tu = 0.0f;  spr[0].tv = 0.0f;
// 		spr[1].tu = 1.0f;  spr[1].tv = 0.0f;
// 		spr[2].tu = 1.0f;  spr[2].tv = 1.0f;
// 		spr[3].tu = 0.0f;  spr[3].tv = 1.0f;
// 
// 		spr[0].dwColor = 0xFFFFFFFF;
// 		spr[1].dwColor = 0xFFFFFFFF;
// 		spr[2].dwColor = 0xFFFFFFFF;
// 		spr[3].dwColor = 0xFFFFFFFF;
// 		Render().DrawSprites(spr, 1);
// 	}
// 	*/
// 
// /*
// 	EnvMap_Render();
// 	SunRoad_Render();
// */
// 
// 	ISurface* pSurf = NULL;
// 
// 	if (bFlatSea)
// 	{
// 		//
// 		ITexture* pTex = Render().GetPostprocessTexture();
// 		pSurf = pTex->GetSurfaceLevel(0, _FL_);
// 
// 		Render().PushRenderTarget();
// 		Render().SetRenderTarget(0, pSurf, NULL);
// 		Render().Clear(0, null, CLEAR_TARGET, 0x0, 1.0f, 0x0);
// 	}
// 
// 
// 	pFrustumPlanes = Render().GetFrustum();
// 	dwNumFrustumPlanes = Render().GetNumFrustumPlanes();
// 
// 
// 	float fBlockSize = 256.0f * fGridStep;
// 	long iNumBlocks = (long)dwMaxDim / (256 * 2);
// 	vSeaCenterPos = Vector(fBlockSize * (long(vCamPos.x / fBlockSize) - iNumBlocks), fMaxSeaHeight * 0.5f, fBlockSize * (long(vCamPos.z / fBlockSize) - iNumBlocks) );
// 
// 	iB1 = 0;
// 	iB2 = 0;
// 
// 	iVStart = 0;
// 	iTStart = 0;
// 	iIStart = 0;
// 
// 	memset(pIndices, 0xFF, NUM_VERTEXS * 3 * sizeof(pIndices[0]));
// 
// 	dword dwX = 0;
// 
// #ifndef _XBOX 
// 	RDTSC_B(dwX);
// #endif
// 
// 	//SetEvent(hEventCalcMaps);
// 	CalculateHeightMap(fFrame1, 1.0f / 255.0f, pSeaFrame1, aBumps);
// 	CalculateNormalMap(fFrame1, 1.0f / 255.0f, pSeaNormalsFrame1, aNormals);
// 
// 	CalculateHeightMap(fFrame2, 1.0f / 255.0f, pSeaFrame2, aBumps);
// 	CalculateNormalMap(fFrame2, 1.0f / 255.0f, pSeaNormalsFrame2, aNormals);
// 
// #ifndef _XBOX 
// 	RDTSC_E(dwX);
// #endif
// 	//api->Trace("dwX = %d", dwX);
// 
// 	aBlocks.Empty();
// 	BuildTree(0, 0, 0);
// 	aBlocks.QSort(SeaBlock::QSort);
// 
// 	dword	i;
// 	long	iNumVPoints = 0;
// 	for (i=0; i<aBlocks(); i++) 
// 	{
// 		iNumVPoints += aBlocks[i].iSize0 * aBlocks[i].iSize0;
// 		if (iNumVPoints >= NUM_VERTEXS)
// 		{
// 			aBlocks.DelRange(i, aBlocks.Last());
// 			break;
// 		}
// 	}
// 
// 	
// 	SeaVertex * pVSea2 = (SeaVertex*)pVSeaBuffer->Lock(0, 0, LOCK_DISCARD | LOCK_NOSYSLOCK);
// 	pTriangles = (word*)pISeaBuffer->Lock(0, 0, LOCK_DISCARD | LOCK_NOSYSLOCK);
// 
// 	for (i=0; i<aBlocks(); i++) PrepareIndicesForBlock(i);
// 	//for (i=0; i<aBlocks(); i++) SetBlock(i);
// 
// 	iBlocksDoneNum = 0;
// 
// 	bool bHT = false;
// 
// 	dword dwBlockRDTSC = 0;
// 
// #ifndef _XBOX 
// 	RDTSC_B(dwBlockRDTSC);
// #endif
// 
// 	if (bHyperThreading)			// P4 / PRESCOTT Version
// 	{
// 		bHT = true;
// 
// 		for (long i=0; i<aEventCalcBlock; i++)
// 		{
// 			aThreadsTest[i] = 0;
// 			SetEvent(aEventCalcBlock[i]);
// 		}
// 
// 		while (true)
// 		{
// 			EnterCriticalSection(&cs);
// 			SeaBlock * pB = GetUndoneBlock();
// 			LeaveCriticalSection(&cs);
// 			if (!pB) break;
// 
// #ifndef _XBOX 
// 			if (bIntel && bSSE) SSE_WaveXZBlock(pB); else WaveXZBlock(pB);
// #endif
// 
// 
// 			iB1++;
// 		}
// 
// 		while (iBlocksDoneNum < aBlocks)
// 		{
// #ifndef _XBOX 
// 
// 			//FIXME: JOKER: возможно надо Sleep поставить вместо этого !!!!
// 
// 			_mm_pause();
// #endif
// 		}
// 	}
// 	else
// 	{
// 		while (true)
// 		{
// 			SeaBlock * pB = GetUndoneBlock();
// 			if (!pB) break;
// 
// #ifndef _XBOX 
// 			if (bIntel && bSSE) SSE_WaveXZBlock(pB); else WaveXZBlock(pB);
// #endif
// 
// 			iB1++;
// 		}
// 	}
// 
// #ifndef _XBOX 
// 	RDTSC_E(dwBlockRDTSC);
// #endif
// 
// 	if (iVStart && iTStart)
// 		memcpy(pVSea2, pVSea, iVStart * sizeof(SeaVertex));
// 
// 	pVSeaBuffer->Unlock();
// 	pISeaBuffer->Unlock();
// 
// 	if (iVStart && iTStart)
// 	{
// 		Matrix mWorldView, mWorldViewProj;
// 
// 		Matrix mView = Render().GetView();
// 		Matrix mWorld; mWorld.SetIdentity();// = Render().GetWorld();
// 		Matrix mProjection = Render().GetProjection();
// 
// 		mWorldView.EqMultiply(mWorld, mView);
// 		mWorldViewProj.EqMultiply(mWorldView, mProjection);
// 
// 		mWorldViewProj.Transposition();
// 
// 		static float fTmp = 0.0f;
// 		fTmp += fDeltaTime * fBumpSpeed;
// 		while (fTmp >= 1.0f) fTmp -= 1.0f;
// 
// /* JOKER FIX ME
// 		Render().SetVertexShaderConstant(GC_CONSTANT, &Vector4(0.0f, 1.0f, 0.5f, 0.0000152590218967f), 1);
// 		Render().SetVertexShaderConstant(GC_CONSTANT2, &Vector4(2.0f, -1.0f, 0.00036621652552071f, (bFogEnable) ? fFogSeaDensity : 0.0f), 1);
// 		Render().SetVertexShaderConstant(GC_ANIMATION, &Vector4(fTmp, fTmp, fTmp, fTmp), 1);
// 		Render().SetVertexShaderConstant(GC_CAMERA_POS, &Vector4(vCamPos.x, vCamPos.y, vCamPos.z, 1.0f), 1);
// 		Render().SetVertexShaderConstant(GC_MTX_WVP, &mWorldViewProj, 4);
// 
// 		Render().SetVertexShaderConstant(GC_FREE, &v4SeaParameters, 1);
// 		Render().SetVertexShaderConstant(GC_FREE + 1, &v4SeaColor, 1);
// 		Render().SetVertexShaderConstant(GC_FREE + 2, &v4SkyColor, 1);
// 
// 		Render().SetVertexShaderConstant(GC_FREE + 5, &Vector4(1.0f, 0.0f, 0.0f, 1.0f), 1);
// 		Render().SetVertexShaderConstant(GC_FREE + 6, &Vector4(fFrenel, 1.0f, 0.5f, 1.0f), 1);		// Frenel K, Frenel Max
// 		Render().SetVertexShaderConstant(GC_FREE + 7, &Vector4(1.0f, 0.0f, 0.0f, 1.0f), 1);
// 		Render().SetVertexShaderConstant(GC_FREE + 8, &Matrix(0.0f, 0.0f, PId2), 4);			// Matrix!!
// 
// 		Vector vTmp = !Vector(0.0f, 1.0f, 0.0f);
// 		Render().SetVertexShaderConstant(GC_FREE + 30, &Vector4(vTmp.x, vTmp.y, vTmp.z, 1.0f), 1);
// 
// 		Render().SetTexture(0, pVolumeTexture);
// 		Render().SetTexture(3, pEnvMap);
// 
// 		Render().DrawIndexedPrimitiveNoVShader(D3DPT_TRIANGLELIST, iVSeaBuffer, sizeof(SeaVertex), iISeaBuffer, 0, iVStart, 0, iTStart, "Sea2");
// 
// 		Render().SetTexture(3, pSunRoadMap);
// 		Render().DrawIndexedPrimitiveNoVShader(D3DPT_TRIANGLELIST, iVSeaBuffer, sizeof(SeaVertex), iISeaBuffer, 0, iVStart, 0, iTStart, "Sea2_SunRoad");
// */
// 		Render().SetWorld(Matrix());
// 
// 		if (fs_FoamScale)
// 		{
// 			fs_FoamScale->SetFloat(fFoamScale);
// 		}
// 
// 		if (fs_vUnderWaterColor)
// 		{
// 			fs_vUnderWaterColor->SetVector4(vUnderWaterColor.v4);
// 		}
// 
// 		if (fs_reflTexture)
// 		{
// 			fs_reflTexture->SetTexture(pReflection);
// 		}
// 
// 		if (fs_refrTexture)
// 		{
// 			fs_refrTexture->SetTexture(pRefraction);
// 		}
// 
// 		if (fs_bumpFrame && pBumpTexture)
// 		{
// 			float f = pBumpTexture->GetAnimPos();
// 			fs_bumpFrame->SetFloat(f);
// 		}
// 
// 		if (fFlatSeaBumpScale)
// 		{
// 			fFlatSeaBumpScale->SetVector4(Vector4(fSmallBumpScale));
// 		}
// 
// 
// 		if (fs_vClearColor)
// 		{
// 			fs_vClearColor->SetVector4(vClearColor.v4);
// 		}
// 
// 		if (fs_vColor)
// 		{
// 			fs_vColor->SetVector4(vSeaColor.v4);
// 		}
// 
// 		if (fs_vSubsurfaceColor)
// 		{
// 			fs_vSubsurfaceColor->SetVector4(vSeaSubsurfaceColor.v4);
// 		}
// 
// 
// 
// 
// 		if (fs_bumpTexture)
// 		{
// 			fs_bumpTexture->SetTexture(pBumpTexture);
// 		}
// 
// 		if (fs_SpecularPower)
// 		{
// 			fs_SpecularPower->SetFloat(fSpecularPower);
// 		}
// 
// 		if (fs_foamTexture)
// 		{
// 			fs_foamTexture->SetTexture(pFoamTexture);
// 		}
// 
// 
// 		if (fs_vSpecularColor)
// 		{
// 			fs_vSpecularColor->SetVector4(vSeaSpecularColor.v4);
// 		}
// 
// 		Render().SetStreamSource(0, pVSeaBuffer);
// 		Render().SetIndices(pISeaBuffer);
// 		Render().DrawIndexedPrimitive(PT_TRIANGLELIST, 0, iVStart, 0, iTStart, "Sea3d");
// 	}
// 
// #ifndef _XBOX 
// 	RDTSC_E(dwTotalRDTSC);
// #endif
// 
// 
// 	if (bFlatSea)
// 	{
// 		//
// 		Render().PopRenderTarget();
// 
// 
// 		pSurf->Release();
// 
// 
// 		if (bShowTexture)
// 		{
// 			Render().SetTexture(0, Render().GetPostprocessTexture());
// 			RS_SPRITE spr[4];
// 			spr[0].vPos = Vector (0.0f, 1.0f, 0.0f);
// 			spr[1].vPos = Vector (1.0f, 1.0f, 0.0f);
// 			spr[2].vPos = Vector (1.0f, 0.0f, 0.0f);
// 			spr[3].vPos = Vector (0.0f, 0.0f, 0.0f);
// 
// 			spr[0].tu = 0.0f;  spr[0].tv = 0.0f;
// 			spr[1].tu = 1.0f;  spr[1].tv = 0.0f;
// 			spr[2].tu = 1.0f;  spr[2].tv = 1.0f;
// 			spr[3].tu = 0.0f;  spr[3].tv = 1.0f;
// 
// 			spr[0].dwColor = 0xFFFFFFFF;
// 			spr[1].dwColor = 0xFFFFFFFF;
// 			spr[2].dwColor = 0xFFFFFFFF;
// 			spr[3].dwColor = 0xFFFFFFFF;
// 			Render().DrawSprites(spr, 1);
// 		}
// 
// 
// 		Vector vOffset = Render().GetView().GetCamPos();
// 		vOffset.y = 0.0f;
// 
// 		Vector quad[4];
// 
// 		float fSize = fFlatSeaLength;
// 
// 		quad[0] = Vector (-fSize, fSeaHeight, fSize) + vOffset;
// 		quad[1] = Vector (-fSize, fSeaHeight, -fSize) + vOffset;
// 		quad[2] = Vector (fSize, fSeaHeight, fSize) + vOffset;
// 		quad[3] = Vector (fSize, fSeaHeight, -fSize) + vOffset;
// 
// 
// 		//Матрица масштабирования
// 		Matrix mInvertY;
// 		mInvertY.BuildScale(Vector(1.0, -1.0, 1.0f));
// 		//Матрица приводящая к диапазону 0..1
// 		Matrix mNormalization;
// 		mNormalization.BuildScale(Vector(0.5f, 0.5f, 1.0f));
// 		mNormalization = (mInvertY * mNormalization) * Matrix(Vector(0.0f), Vector(0.5f, 0.5f, 1.0f));
// 
// 
// 		if (fs_TextureProjMatrix)
// 		{
// 			fs_TextureProjMatrix->SetMatrix(mNormalization);
// 		}
// 		
// 
// 
// 
// 
// 
// 		if (fs_flatSeaTexture)
// 		{
// 			fs_flatSeaTexture->SetTexture(Render().GetPostprocessTexture());
// 		}
// 
// 		//******************************************************************************************************************
// 
// 		//******************************************************************************************************************
// 
// 
// 		Render().DrawPrimitiveUP(PT_TRIANGLESTRIP, 2, quad, sizeof(Vector), "ShowFlatSea");
// 	}
// 
// 
// 
// 
// 	//Render().Print(50, 300, "Total ticks with rendering %d", /*iVStart, iTStart, */dwTotalRDTSC);
// 	/*Render().Print(50, 320, "calc blk%s: %d", (bHT) ? " (HT)" : "", dwBlockRDTSC);
// 	Render().Print(50, 340, "Blocks in 1st thread: %d", iB1);
// 	for (long i=0; i<aThreadsTest; i++)
// 		Render().Print(50, 360 + 20 * i, "Blocks in thread %d: %d", i + 1, aThreadsTest[i]);
// 	*/
// 	//Render().Print(30, 140, "rdtsc = %d", dwBlockRDTSC);
// 	//Render().Print(30, 160, "Intel CPU: %s, SSE: %s, HyperThreading: %s", (bIntel) ? "Yes" : "No", (bSSE) ? "On" : "Off", (bHyperThreading) ? "On" : "Off");
// }
// 
// float SEA::Trace(const Vector & vSrc, const Vector & vDst)
// {
// 	long iNumTests = 5;
// 	float fRes = 2.0f;
// 	float fDV = 1.0f / float(iNumTests - 1);
// 
// 	if (vSrc.y > fMaxSeaHeight && vDst.y > fMaxSeaHeight) return 2.0f;
// 
// 	for (long i=0; i<iNumTests; i++)
// 	{
// 		Vector vTemp = vSrc + float(i) * fDV * (vDst - vSrc);
// 		float fWaveY = WaveXZ(vTemp.x, vTemp.z, null);
// 
// 		if (fWaveY > vTemp.y) return float(i) * fDV;
// 	}
// 
// 	return 2.0f;
// }
// 
// 
// void SEA::BuildEnvironmentTextureMaps ()
// {
// /*
// 	Vector v1 = vSeaCenterPos + Vector(iTX * fGSize, -fMaxSeaHeight / 2.0f, iTY * fGSize);
// 	Vector v2 = v1 + Vector(fGSize, fMaxSeaHeight, fGSize);
// */
// 	//FIX ME - ХАК !!!
// 	float PlaneHeight = 0.5f;
// 
// 
// 	D3DXPLANE plane;
// 	D3DXPlaneFromPointNormal( &plane, &D3DXVECTOR3(0, PlaneHeight, 0), &D3DXVECTOR3(0, 1, 0) );
// 
// 	D3DXMATRIX matReflect;
// 	D3DXMatrixReflect( &matReflect, &plane );
// 
// 
// 	Matrix mView = Render().GetView();
// 
// 	Matrix mViewNew = mView;
// 
// 	Matrix Invertor;
// 	memcpy (Invertor.m, matReflect.m, sizeof(float)* 16);
// 	mViewNew = Invertor * mViewNew;
// 
// 	Render().SetView(mViewNew);
// 
// 	Render().PushRenderTarget();
// 
// 	//--------- reflection
// 
// 	Render().SetRenderTarget(0, pReflectionSurface, pReflectionSurfaceDepth);
// 	Render().Clear(0, null, CLEAR_TARGET | CLEAR_ZBUFFER | CLEAR_STENCIL, vClearColor.GetDword(), 1.0f, 0x0);
// 
// 	Matrix _mWorld = Matrix();
// 	Matrix _mView = Render().GetView();
// 	Matrix _mProj = Render().GetProjection();
// 	Matrix _mWorldView = _mWorld * _mView;
// 	Matrix _mWorldViewProj = _mWorldView * _mProj;
// 
// 
// 
// 
// 	D3DXMATRIX mInv;
// 	memcpy (mInv.m, _mWorldViewProj.m, sizeof(float)* 16);
// 	D3DXMatrixInverse(&mInv, NULL, &mInv);
// 	D3DXMatrixTranspose(&mInv, &mInv);
// 	D3DXPlaneTransform(&plane, &plane, &mInv);
// 	Plane pplane;
// 	pplane.n.x = plane.a;
// 	pplane.n.y = plane.b;
// 	pplane.n.z = plane.c;
// 	pplane.d = plane.d;
// 	Render().SetClipPlane (0, pplane);
// 	Render().SetEffect("FlatSeaReverseCull");
// 	Event("SeaReflection");
// 
// 
// 	//--------- refraction
// 
// 
// 	if (bRefraction)
// 	{
// 		Render().SetRenderTarget(0, pRefractionSurface, pReflectionSurfaceDepth);
// 		Render().Clear(0, null, CLEAR_TARGET | CLEAR_ZBUFFER | CLEAR_STENCIL, vUnderWaterColor.GetDword(), 1.0f, 0x0);
// 
// 
// 		Render().SetView(mView);
// 		_mView = Render().GetView();
// 		_mProj = Render().GetProjection();
// 		_mWorldView = _mWorld * _mView;
// 		_mWorldViewProj = _mWorldView * _mProj;
// 
// 		D3DXPlaneFromPointNormal( &plane, &D3DXVECTOR3(0, 0, 0), &D3DXVECTOR3(0, -1, 0) );
// 		memcpy (mInv.m, _mWorldViewProj.m, sizeof(float)* 16);
// 		D3DXMatrixInverse(&mInv, NULL, &mInv);
// 		D3DXMatrixTranspose(&mInv, &mInv);
// 		D3DXPlaneTransform(&plane, &plane, &mInv);
// 		pplane.n.x = plane.a;
// 		pplane.n.y = plane.b;
// 		pplane.n.z = plane.c;
// 		pplane.d = plane.d;
// 		Render().SetClipPlane (0, pplane);
// 		Render().SetEffect("FlatSeaForwardCull");
// 		Event("SeaReflection");
// 	}
// 
// 
// 
// 	Render().SetWorld(Matrix());
// 
// 	Render().SetEffect();
// 
// 	Render().PopRenderTarget();
// 
// 	Render().SetView(mView);
// }
// 
// 
// 
// /*
// dword SEA::AttributeChanged(ATTRIBUTES * pAttribute)
// {
// 	ATTRIBUTES * pParent = pAttribute->GetParent();
// 	ATTRIBUTES * pParent2 = (pParent) ? pParent->GetParent() : null;
// 
// 	char * sName = pAttribute->GetThisName();
// 	char * sValue = pAttribute->GetThisAttr();
// 
// 	if (*pParent == "isDone")
// 	{
// 		Realize(0);
// 	}
// 
// 	if (*pParent == "Sea2")
// 	{
// 		if (*pAttribute == "WaterColor")	{ v4SeaColor = COLOR2VECTOR4(pAttribute->GetAttributeAsDword()); return 0; }
// 		if (*pAttribute == "SkyColor")		{ v4SkyColor = COLOR2VECTOR4(pAttribute->GetAttributeAsDword()); return 0; }
// 		
// 		if (*pAttribute == "Amp1")			{ _fAmp1 = pAttribute->GetAttributeAsFloat(); return 0; }
// 		if (*pAttribute == "AnimSpeed1")	{ fAnimSpeed1 = pAttribute->GetAttributeAsFloat(); return 0; }
//  		if (*pAttribute == "Scale1")		{ fScale1 = pAttribute->GetAttributeAsFloat(); return 0; }
// 		if (*pAttribute == "MoveSpeed1")	{ sscanf(pAttribute->GetThisAttr(), "%f, %f, %f", &vMoveSpeed1.x, &vMoveSpeed1.y, &vMoveSpeed1.z); return 0; }
// 
// 		if (*pAttribute == "Amp2")			{ _fAmp2 = pAttribute->GetAttributeAsFloat(); return 0; }
// 		if (*pAttribute == "AnimSpeed2")	{ fAnimSpeed2 = pAttribute->GetAttributeAsFloat(); return 0; }
// 		if (*pAttribute == "Scale2")		{ fScale2 = pAttribute->GetAttributeAsFloat(); return 0; }
// 		if (*pAttribute == "MoveSpeed2")	{ sscanf(pAttribute->GetThisAttr(), "%f, %f, %f", &vMoveSpeed2.x, &vMoveSpeed2.y, &vMoveSpeed2.z); return 0; }
// 
// 		if (*pAttribute == "PosShift")		{ fPosShift = pAttribute->GetAttributeAsFloat(); return 0; }
// 
// 		if (*pAttribute == "BumpScale")		{ fBumpScale = pAttribute->GetAttributeAsFloat(); return 0; }
// 		if (*pAttribute == "BumpSpeed")		{ fBumpSpeed = pAttribute->GetAttributeAsFloat(); return 0; }
// 		if (*pAttribute == "LodScale")		{ fLodScale = pAttribute->GetAttributeAsFloat(); return 0; }
// 		if (*pAttribute == "GridStep")		{ fGridStep = pAttribute->GetAttributeAsFloat(); return 0; }
// 
// 		if (*pAttribute == "Reflection")	{ v4SeaParameters.y = pAttribute->GetAttributeAsFloat(); return 0; }
// 		if (*pAttribute == "Transparency")	{ v4SeaParameters.z = pAttribute->GetAttributeAsFloat(); return 0; }
// 		if (*pAttribute == "Attenuation")	{ v4SeaParameters.x = pAttribute->GetAttributeAsFloat(); return 0; }
// 
// 		if (*pAttribute == "Frenel")		{ fFrenel = pAttribute->GetAttributeAsFloat(); return 0; }
// 		return 0;
// 	}
// 
// 	if (*pParent == "fog")
// 	{
// 		if (*pAttribute == "Enable")	{ bFogEnable = pAttribute->GetAttributeAsDword()!=0; return 0; }
// 		if (*pAttribute == "Start")		{ fFogStartDistance = pAttribute->GetAttributeAsFloat(); return 0; }
// 		if (*pAttribute == "Color")		{ vFogColor = (1.0f / 255.0f) * COLOR2VECTOR(pAttribute->GetAttributeAsDword()); return 0; }
// 		if (*pAttribute == "SeaDensity"){ fFogSeaDensity = pAttribute->GetAttributeAsFloat(); return 0; }
// 		return 0;
// 	}
// 
// 	if (*pAttribute == "MaxSeaHeight")
// 	{
// 		fMaxSeaHeight = AttributesPointer->GetAttributeAsFloat("MaxSeaHeight", 50.0f);
// 
// 		float fScale = (fMaxSeaHeight >= _fAmp1 + _fAmp2) ? 1.0f : fMaxSeaHeight / (_fAmp1 + _fAmp2);
// 
// 		fAmp1 = _fAmp1 * fScale;
// 		fAmp2 = _fAmp2 * fScale;
// 	}
// 
// 	return 0;
// }
// */
// 

/*
MOP_BEGINLISTG(SEA, "Sea", '1.00', 100, "Arcade Sea");

  MOP_FLOAT("Sea height", 20.0f);
	MOP_FLOAT("Sea elevation", -4.0f);

	MOP_STRING("Bump Texture Name", "OceanBump");
	MOP_FLOAT ("Bump animation speed", 3.78f)
	MOP_FLOAT ("Bump Texture scale", 0.0441f);

	MOP_STRING("Foam Texture Name", "foam");
	MOP_FLOAT("Foam Texture scale", 1.115f);

	
	MOP_COLOR ("Reflection Clear color", Color(1.0f, 1.0f, 1.0f));
	MOP_COLOR ("Sea color", Color(0xFF10465BL))
	MOP_FLOAT ("Sea color multipler", 1.0f);
	MOP_COLOR ("Sea subsurface color", Color(0xFF3F88C8L))
	MOP_FLOAT ("Sea subsurface color multipler", 1.0f);
	MOP_FLOAT ("Specular power", 36.222f);
	MOP_COLOR ("Under water color", Color(0xFF16192FL))
	MOP_COLOR ("Specular color", Color(0xFF838398L))

	MOP_BOOL ("Sea is flat", true);
	MOP_BOOL ("Show flat sea texture", false);
	MOP_FLOAT ("Flat Sea Length", 400.0f);

	MOP_BOOLC("Show", true, "Show or hide sea")
	
MOP_ENDLIST(SEA)
*/