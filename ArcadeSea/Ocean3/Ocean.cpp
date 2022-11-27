#include "Ocean.h"
#include "..\..\common_h\tga.h"

#define MAX_VERTICES		65400
#define MAX_EDGE_INDICES	10000

#define OCEAN3_DYNAMIC		0//USAGE_DYNAMIC

CREATE_SERVICE(Ocean3Service, 10000)

bool Ocean2::m_bDebugDraw = false;
int Ocean2::m_iDebugDraw = 0;
int Ocean2::m_iNumReflections = 0;

float Ocean2::fGridStep = 0.0745f;
int Ocean2::dwMaxDim = 32768 + 32768 + 16384;
int Ocean2::dwMinDim = 256;

inline float Frenel(float ci, float n)
{
	float ct = float(sqrt(1.0f + SQR(n) * (SQR(ci) - 1.0f)));
	return 0.5f * (SQR((ci - n * ct) / (ci + n * ct)) + SQR((n * ci - ct) / (n * ci + ct)));
}

Ocean3Service::Ocean3Service()
{
	m_fftWavesSize = 7;
	fftWaves.Init(1 << m_fftWavesSize);
	fftBump.Init(128);

	m_indices = null;

	pFrenelTexture = null;

	pVarOceanRT = null;
	pVarBumpMulMove = null;
	pVarBumpDistance = null;
	pVarDMapTexture = null;
	pVarShifts = null;
	pVarSkyColor = null;
	pVarWaterColor = null;
	pVarTranslucenceColor = null;
	pVarFrenelTex = null;
	pVarReflTex = null;
	pVarRefrTex = null;
	pVarBumpTex = null;
	pVarBumpPower = null;
	pVarSphereMapTexture = null;
	pVarFoamTexture = null;
	pVarDistFade = null;
	pVarReflTrans = null;
	pVarRefrParams = null;
	pVarRefrParams2 = null;
	pVarFlatMapping = null;
	//pVarRefrMapping = null;
	pVarSunRoadParams = null;
	pVarSunLight = null;
	pVarSunLightParams = null;
	pVarSunRoadColor = null;
	pVarSunBumpScale = null;
	pVarTranslucenceAngle = null;
	pVarTranslucenceParams = null;

	pVarBooleanParams = null;

	pVarFoamParams1 = null;
	pVarFoamParams2 = null;
	pVarFoamColor = null;
	pVarFoamRotate = null;
	pVarFogParams = null;
	pVarFogColor = null;
	pVarViewport = null;
	pVarCamPos = null;
	pVarFakeHeight = null;

	pReflectionTexture = null;
	pReflectionTexture2 = null;
	pOceanRT = null;
	sphereRT = null;
	pReflectionSurfaceDepth = null;
	pOceanRTDepth = null;

	pRefractRT = null;
	pRefractRTDepth = null;
	
	m_vSeaBuffer = null;
 	m_iSeaBuffer = null;
 	m_vSeaBufferTemp[0] = null;
 	m_vSeaBufferTemp[1] = null;
	m_iSeaBufferTemp[0] = null;
	m_iSeaBufferTemp[1] = null;

	m_verts2D = null;
	m_indexes2D = null;

	pFoamTexture = null;

	faceRT = null;

	m_sphereMapMission = null;

	m_isOceansHaveFakeHeight = false;

	m_curBuffer = 0;
	m_curTexture = 0;

	m_regenerateSphereMap = true;

	m_normTex2Temp[0] = null;
	m_normTex2Temp[1] = null;
	m_normTex2Temp[2] = null;

	m_halfScreenCopyVB = null;
}

Ocean3Service::~Ocean3Service()
{
	DELETE(m_indices);

	RELEASE(m_normTex2Temp[0]);
	RELEASE(m_normTex2Temp[1]);
	RELEASE(m_normTex2Temp[2]);

	// релизим общие параметры
	RELEASE(pFrenelTexture);

	RELEASE(faceRT);

	RELEASE(pReflectionTexture);
	RELEASE(pReflectionTexture2);
	RELEASE(pReflectionSurfaceDepth);

	RELEASE(sphereRT);

	RELEASE(pOceanRT);
	RELEASE(pOceanRTDepth);

	RELEASE(pRefractRT);
	RELEASE(pRefractRTDepth);

 	RELEASE(m_vSeaBufferTemp[0]);
 	RELEASE(m_vSeaBufferTemp[1]);
 	//RELEASE(m_iSeaBuffer);
 	RELEASE(m_iSeaBufferTemp[0]);
 	RELEASE(m_iSeaBufferTemp[1]);

	RELEASE(m_verts2D);
	RELEASE(m_indexes2D);

	RELEASE(pFoamTexture);

	RELEASE(m_halfScreenCopyVB);
}

bool Ocean3Service::Init()
{
	render = (IRender *)api->GetService("DX9Render");
	files = (IFileService *)api->GetService("FileService");

	pShdServ = (IShadowsService *)api->GetService("ShadowsService");

	if (float(render->GetScreenInfo3D().dwWidth) / float(render->GetScreenInfo3D().dwHeight) > 2.6f)
		Ocean2::fGridStep *= 1.1f;

	pFoamTexture = render->CreateTexture(_FL_, "weather\\foam\\foam");

	pFrenelTexture = (ITexture*)render->CreateTexture(_FL_, "weather\\ocean_frenel");
	/*pFrenelTexture = (ITexture*)render->CreateTexture(256, 1, 1, 0, FMT_A8R8G8B8, _FL_);
	//Заполняем текстуру с рассчитанным френелем
	RENDERLOCKED_RECT lr;
	pFrenelTexture->LockRect(0, &lr, null, 0);
	dword * pDst = (dword*)lr.pBits;
	for (long i=0; i<128; i++)
	{	
		float ci = float(i) * float(1.0f / 128.0f);
		float k = 255.0f * MinMax(0.01f, 1.0f, Frenel(ci, 0.75f));
		pDst[i] = ARGB(0, dword(k), 0, 0);
		pDst[255 - i] = ARGB(0, dword(k), 0, 0);
	}
	pFrenelTexture->UnlockRect(0);*/
	//render->SaveTexture2File(pFrenelTexture, "ocean_frenel.tga");

	pVarFrenelTex = render->GetTechniqueGlobalVariable("Ocean2FrenelTexture", _FL_);
	pVarFrenelTex->SetTexture(pFrenelTexture);

	pVarRefrParams = render->GetTechniqueGlobalVariable("Ocean2RefrParams", _FL_);
	pVarRefrParams2 = render->GetTechniqueGlobalVariable("Ocean2RefrParams2", _FL_);

	pVarDepthTexture = render->GetTechniqueGlobalVariable("Ocean3DepthTexture", _FL_);

	pVarOceanRT = render->GetTechniqueGlobalVariable("Ocean2RTTexture", _FL_);
	pVarBumpDistance = render->GetTechniqueGlobalVariable("Ocean2BumpDistance", _FL_);
	pVarBumpMulMove = render->GetTechniqueGlobalVariable("Ocean2BumpScaleMove", _FL_);

	pVarDMapTexture = render->GetTechniqueGlobalVariable("Ocean2DMapTexture", _FL_);

	//pVarRefrMapping = render->GetTechniqueGlobalVariable("Ocean2RefrMapping", _FL_);

	pVarReflTex = render->GetTechniqueGlobalVariable("Ocean2ReflTexture", _FL_);
	pVarRefrTex = render->GetTechniqueGlobalVariable("Ocean2RefrTexture", _FL_);
	pVarBumpTex = render->GetTechniqueGlobalVariable("Ocean2BumpTexture", _FL_);
	pVarBumpPower = render->GetTechniqueGlobalVariable("Ocean2BumpPower", _FL_);

	pVarDistFade = render->GetTechniqueGlobalVariable("Ocean2DistFade", _FL_);
	pVarReflTrans = render->GetTechniqueGlobalVariable("Ocean2ReflTrans", _FL_);

	pVarShifts = render->GetTechniqueGlobalVariable("Ocean2Shifts", _FL_);
	
	pVarSkyColor = render->GetTechniqueGlobalVariable("Ocean2SkyColor", _FL_);
	pVarWaterColor = render->GetTechniqueGlobalVariable("Ocean2WaterColor", _FL_);
	pVarTranslucenceColor = render->GetTechniqueGlobalVariable("Ocean2TranslucenceColor", _FL_);

	pVarCamPos = render->GetTechniqueGlobalVariable("Ocean2CamPos", _FL_);

	pVarFakeHeight = render->GetTechniqueGlobalVariable("Ocean2FakeHeight", _FL_);

	pVarSunRoadParams = render->GetTechniqueGlobalVariable("Ocean2SunRoadParams", _FL_);
	pVarSunLight = render->GetTechniqueGlobalVariable("Ocean2SunLight", _FL_);
	pVarSunLightParams = render->GetTechniqueGlobalVariable("Ocean2SunLightParams", _FL_);
	pVarSunRoadColor = render->GetTechniqueGlobalVariable("Ocean2SunRoadColor", _FL_);
	pVarSunBumpScale = render->GetTechniqueGlobalVariable("Ocean2SunBumpScale", _FL_);

	pVarTranslucenceAngle = render->GetTechniqueGlobalVariable("Ocean2TranslucenceAngle", _FL_);
	pVarTranslucenceParams = render->GetTechniqueGlobalVariable("Ocean2TranslucenceParams", _FL_);
	
	pVarBooleanParams = render->GetTechniqueGlobalVariable("Ocean2BoolParams", _FL_);

	pVarFoamParams1 = render->GetTechniqueGlobalVariable("Ocean2FoamParams1", _FL_);
	pVarFoamParams2 = render->GetTechniqueGlobalVariable("Ocean2FoamParams2", _FL_);
	pVarFoamColor = render->GetTechniqueGlobalVariable("Ocean2FoamColor", _FL_);
	pVarFoamRotate = render->GetTechniqueGlobalVariable("Ocean2FoamRotate", _FL_);

	pVarFogParams = render->GetTechniqueGlobalVariable("Ocean2FogParams", _FL_);
	pVarFogColor = render->GetTechniqueGlobalVariable("Ocean2FogColor", _FL_);
		
	pVarViewport = render->GetTechniqueGlobalVariable("Ocean2Viewport", _FL_);

	pVarSphereMapTexture = render->GetTechniqueGlobalVariable("Ocean2SphereMapTexture", _FL_);
	pVarFoamTexture = render->GetTechniqueGlobalVariable("Ocean2FoamTexture", _FL_);

	pVarFlatMapping = render->GetTechniqueGlobalVariable("Ocean2FlatMapping", _FL_);

	pVarUV05 = render->GetTechniqueGlobalVariable("ocean3uv05", _FL_);

	const RENDERSCREEN & si = render->GetScreenInfo3D();

	float fsize = float(Max(si.dwWidth, si.dwHeight)) / 1.8f;
	rr_size = Min(int(fsize), 512);

	if (rr_size == 512)
	{
		pReflectionTexture = render->CreateRenderTarget(512, 512, _FL_, FMT_A8R8G8B8);
		pReflectionTexture2 = render->CreateTempRenderTarget(TRS_512, TRC_FIXED_RGBA_8, _FL_);//render->CreateRenderTarget(512, 512, _FL_, FMT_A8R8G8B8);
		pReflectionSurfaceDepth = render->CreateTempDepthStencil(TRS_512, _FL_);

		pRefractRT = render->CreateRenderTarget(512, 512, _FL_, FMT_A8R8G8B8);
		pRefractRTDepth = render->CreateTempDepthStencil(TRS_512, _FL_);
	}
	else
	{
		pReflectionTexture = render->CreateRenderTarget(rr_size, rr_size, _FL_, FMT_A8R8G8B8);
		pReflectionTexture2 = render->CreateRenderTarget(rr_size, rr_size, _FL_, FMT_A8R8G8B8);
		pReflectionSurfaceDepth = render->CreateDepthStencil(rr_size, rr_size, _FL_);

		pRefractRT = render->CreateRenderTarget(rr_size, rr_size, _FL_, FMT_A8R8G8B8);
		pRefractRTDepth = render->CreateDepthStencil(rr_size, rr_size, _FL_);
	}

	// skydome section 
		sphereRT = render->CreateRenderTarget(512, 512, _FL_, FMT_A8R8G8B8);
		//p512RTDepth = render->CreateTempDepthStencil(TRS_512, _FL_);
		render->GetShaderId("Create_SkyDome2", SkyDome_id);
		varSkyDomeTexture = render->GetTechniqueGlobalVariable("SkyDomeTexture", _FL_);
		varSkyDomeBools = render->GetTechniqueGlobalVariable("SkyDomeBools", _FL_);

	m_oceanRTWidth = si.dwWidth;
	m_oceanRTHeight = si.dwHeight;
#ifndef _XBOX
	IRenderTarget * screen = render->GetRenderTarget(_FL_);
	RENDERSURFACE_DESC desc;
	screen->GetDesc(&desc);
	RENDERMULTISAMPLE_TYPE multiSample = desc.MultiSampleType;
	RELEASE(screen);

	// pOceanRT нельзя делать TempRenderTarget - потому что его содержимое используется в FakeFill
	pOceanRT = render->CreateRenderTarget(m_oceanRTWidth, m_oceanRTHeight, _FL_, FMT_A8R8G8B8, multiSample);
	pOceanRTDepth = render->CreateDepthStencil(m_oceanRTWidth, m_oceanRTHeight, _FL_, FMT_D24S8, multiSample);
#else
	// pOceanRT нельзя делать TempRenderTarget - потому что его содержимое используется в FakeFill
	pOceanRT = render->CreateRenderTarget(m_oceanRTWidth, m_oceanRTHeight, _FL_, FMT_A8R8G8B8);
	pOceanRTDepth = null;
#endif

	render->GetShaderId("Ocean", m_oceanID);
	render->GetShaderId("Ocean3RefractAlpha", m_alphaRefractID);
	render->GetShaderId("Ocean3ClearAlpha", m_shaderClearAlpha);

	render->GetShaderId("Ocean3ZPrePass", m_zPrePassID);
	render->GetShaderId("Ocean3ZPrePassSetAlpha", m_zPrePassSetAlphaID);
	
	render->GetShaderId("NewOcean3", m_oceanShaderID);
	render->GetShaderId("NewWater3", m_waterShaderID);
	render->GetShaderId("MirrorOcean3", m_mirrorShaderID);
	render->GetShaderId("BadWater", m_badFlatWaterShaderID);
	render->GetShaderId("Ocean3ClearRT_Depth", m_clearRTDepthID);
	
	render->GetShaderId("Ocean2_flatNoZ", m_flatNoZID);

#ifndef _XBOX
	const char * errorNotEnoughVideoMemory = "OceanService: Can't create render targets, maybe not enough video memory?";

	if (!pReflectionTexture || !pReflectionTexture2 || !pReflectionSurfaceDepth ||
		!pOceanRT || !pRefractRT || !pRefractRTDepth || !sphereRT || !pOceanRTDepth)
	{
		Error(1000600, errorNotEnoughVideoMemory);
		return false;
	}
#endif

	m_indices = NEW dword[MAX_EDGE_INDICES]; Assert(m_indices);

#ifndef _XBOX
	m_vSeaBufferTemp[0] = render->CreateVertexBuffer(MAX_VERTICES * sizeof(SeaVertex), sizeof(SeaVertex), _FL_, USAGE_WRITEONLY | USAGE_DYNAMIC, POOL_DEFAULT);
	m_iSeaBufferTemp[0] = render->CreateIndexBuffer(MAX_VERTICES * 2 * 3 * sizeof(word), _FL_, USAGE_WRITEONLY | USAGE_DYNAMIC, FMT_INDEX16, POOL_DEFAULT);
#else
 	m_vSeaBufferTemp[0] = render->CreateVertexBuffer(MAX_VERTICES * sizeof(SeaVertex), sizeof(SeaVertex), _FL_, OCEAN3_DYNAMIC, POOL_DEFAULT);
 	m_vSeaBufferTemp[1] = render->CreateVertexBuffer(MAX_VERTICES * sizeof(SeaVertex), sizeof(SeaVertex), _FL_, OCEAN3_DYNAMIC, POOL_DEFAULT);
 	m_iSeaBufferTemp[0] = render->CreateIndexBuffer(MAX_VERTICES * 2 * 3 * sizeof(word), _FL_, OCEAN3_DYNAMIC, FMT_INDEX16, POOL_MANAGED);
 	m_iSeaBufferTemp[1] = render->CreateIndexBuffer(MAX_VERTICES * 2 * 3 * sizeof(word), _FL_, OCEAN3_DYNAMIC, FMT_INDEX16, POOL_MANAGED);
 	//m_iSeaBuffer = render->CreateIndexBuffer(MAX_VERTICES * 2 * 3 * sizeof(word), _FL_, OCEAN3_DYNAMIC, FMT_INDEX16, POOL_MANAGED);
#endif

	dword dwCells = 10;
	float fCellSize = (float(Ocean2::dwMaxDim) * Ocean2::fGridStep) / float(dwCells);

	m_verts2D = render->CreateVertexBuffer(dwCells * dwCells * sizeof(Vector), sizeof(Vector), _FL_);
	if (m_verts2D)
	{
		Vector * pV = (Vector *)m_verts2D->Lock();
		for (dword z=0; z<dwCells; z++)
			for (dword x=0; x<dwCells; x++)
			{
				pV[x + z * dwCells] = Vector((float(x) - float(dwCells) / 2.0f) * fCellSize, 0.0f, (float(z) - float(dwCells) / 2.0f) * fCellSize);
			}
		m_verts2D->Unlock();
	}

	m_indexes2D = render->CreateIndexBuffer(dwCells * dwCells * 2 * 3 * sizeof(word), _FL_);
	if (m_indexes2D)
	{
		word * pI = (word *)m_indexes2D->Lock();

		for (dword z=0; z<dwCells - 1; z++)
			for (dword x=0; x<dwCells - 1; x++)
			{
				*pI++ = word((z + 0) * dwCells + x + 1);
				*pI++ = word((z + 1) * dwCells + x);
				*pI++ = word((z + 0) * dwCells + x);

				*pI++ = word((z + 0) * dwCells + x + 1);
				*pI++ = word((z + 1) * dwCells + x + 1);
				*pI++ = word((z + 1) * dwCells + x);
			}
		m_indexes2D->Unlock();
	}

	m_normTex2Temp[0] = render->CreateTexture(128, 128, 8, 0, FMT_A8R8G8B8, _FL_);
	m_normTex2Temp[1] = render->CreateTexture(128, 128, 8, 0, FMT_A8R8G8B8, _FL_);
	m_normTex2Temp[2] = render->CreateTexture(128, 128, 8, 0, FMT_A8R8G8B8, _FL_);

	struct v4uv
	{
		Vector4 pos;
		float	u, v;
	};

	m_halfScreenCopyVB = render->CreateVertexBuffer(4 * sizeof(v4uv), sizeof(v4uv), _FL_);
	v4uv * buf = (v4uv *)m_halfScreenCopyVB->Lock();
	float HalfPixelSize = 1.0f / float(rr_size);
	float left = -1.0f - HalfPixelSize;
	float right = 1.0f - HalfPixelSize;
	float up = -1.0f + HalfPixelSize;
	float down = 1.0f + HalfPixelSize;
	buf[0].pos = Vector4( left,	 up,	0.0f, 1.0f );
	buf[1].pos = Vector4( right, up,	0.0f, 1.0f );
	buf[2].pos = Vector4( right, down,	0.0f, 1.0f );
	buf[3].pos = Vector4( left,	 down,	0.0f, 1.0f );
	m_halfScreenCopyVB->Unlock();

#ifndef _XBOX
	if (!m_indexes2D || !m_verts2D || !m_vSeaBufferTemp[0] || !m_iSeaBufferTemp[0] ||
		!m_normTex2Temp[0] || !m_normTex2Temp[1] || !m_normTex2Temp[2])
	{
		Error(1000600, errorNotEnoughVideoMemory);
		return false;
	}
#endif

	api->SetStartFrameLevel(this, Core_DefaultExecuteLevel);

	m_bumpAlreadyGenerated = false;

	InitProtectData((dword)this);

	return true;
}

#pragma optimize("", off)
void  __declspec(dllexport) Ocean3Service::InitProtectData(dword dwValue)
{
	dword result;

	dword value1 = (dwValue >> 7L) & 0x7;
	dword value2 = (dwValue >> 16L) & 0x3;
	switch (value1)
	{
		case 0 : result = dwValue + value1 + 128; break;
		case 1 : result = dwValue + value1 + 127; break;
		case 2 : result = dwValue + value1 + 126; break;
		case 3 : result = dwValue + value1 + 125; break;
		case 4 : result = dwValue + value1 + 124; break;
		case 5 : result = dwValue + value1 + 123; break;
		case 6 : result = dwValue + value1 + 122; break;
		case 7 : result = dwValue + value1 + 121; break;
		default: result = dwValue + 128;
	}

	dword value3 = (dwValue >> 24L) & 0x7;
	switch (value2)
	{
		case 0 : result -= value2 + 1000; break;
		case 1 : result -= value2 + 999; break;
		case 2 : result -= value2 + 998; break;
		default: result -= 1000;
	}

	vRender = (IRender*)api->GetService("DX9Render");

	switch (value3)
	{
		case 0 : result *= (128 - value3); break;
		case 1 : result *= (129 - value3); break;
		case 2 : result *= value3 * 64; break;
		case 3 : result *= (131 - value3); break;
		case 4 : result *= 32 * value3; break;
		case 5 : result *= (133 - value3); break;
		case 6 : result *= (134 - value3); break;
		case 7 : result *= (135 - value3); break;
		default: result *= 128;
	}

	protectValue = result;
}
#pragma optimize("", on)

void Ocean3Service::Error(long id, const char * errorEnglish)
{
	ILocStrings * locStrings = (ILocStrings*)api->GetService("LocStrings");
	ICoreStorageString * storage = api->Storage().GetItemString("system.error", _FL_);

	if (!storage)
		return;

	const char * errorString = locStrings->GetString(id);
	storage->Set((errorString) ? errorString : errorEnglish);
	storage->Release();
}

void Ocean3Service::StartFrame(float deltaTime)
{
#ifdef _XBOX
	m_curBuffer ^= 1;
	m_curTexture++;
	if (m_curTexture > 2)
		m_curTexture = 0;
#endif

	m_vSeaBuffer = m_vSeaBufferTemp[m_curBuffer];
	m_iSeaBuffer = m_iSeaBufferTemp[m_curBuffer];

	m_normTex2 = m_normTex2Temp[m_curTexture];

	m_bumpAlreadyGenerated = false;
}

IBaseTexture * Ocean3Service::GetBumpTexture()
{
	//return normTex->AsTexture();
	return m_normTex2;
}

#define VPERM(x, y, z, w)	((unsigned int)(x << 6L) | (y << 4L) | (z << 2L) | w)

void Ocean3Service::GenerateBumpMap()
{
	if (m_bumpAlreadyGenerated)
		return;

#ifndef _XBOX
	__declspec(align(128)) complex mip[64 * 64];
	memset(mip, 0, sizeof(mip));

	int size = fftBump.GetSize();
	fft::dcomplex * frame = fftBump.GetCurFrame();

	RENDERLOCKED_RECT lr;
	bool result = m_normTex2->LockRect(0, &lr, null, 0);
	if (result && lr.pBits)
	{
		char * bits = (char*)lr.pBits;
		for (int y=0; y<size; y++)
			for (int x=0; x<size; x++)
			{
				float fx = frame[x + y * size].c2.Re;
				float fz = frame[x + y * size].c2.Im;

				mip[x / 2 + y / 2 * (size / 2)].Re += fx;
				mip[x / 2 + y / 2 * (size / 2)].Im += fz;

				fx = fx * 127.5f + 127.5f;
				fz = fz * 127.5f + 127.5f;

				*(dword*)&bits[y*lr.Pitch + x*4] = ARGB(fz, 0, 0, fx);
			}
		m_normTex2->UnlockRect(0);
	}

	for (long i=1; i<8; i++)
	{
		result = m_normTex2->LockRect(i, &lr, null, 0);
		if (!result || !lr.pBits)
			continue;

		size >>= 1;
		char * bits = (char*)lr.pBits;
		for (int y=0; y<size; y++)
			for (int x=0; x<size; x++)
			{
				float fx = mip[x + y * size].Re * 0.25f;
				float fz = mip[x + y * size].Im * 0.25f;

				mip[x + y * size].Re = 0.0f;
				mip[x + y * size].Im = 0.0f;

				mip[x / 2 + y / 2 * (size / 2)].Re += fx;
				mip[x / 2 + y / 2 * (size / 2)].Im += fz;

				fx = fx * 127.5f + 127.5f;
				fz = fz * 127.5f + 127.5f;

				*(dword*)&bits[y*lr.Pitch + x*4] = ARGB(fz, 0, 0, fx);
			}

		m_normTex2->UnlockRect(i);
	}
#else
	// XBOX Version of genereting mips
	__declspec(align(128)) complex mip2[64 * 64];

	__vector4 _v1275 = XMVectorReplicate(127.5f);
	__vector4 _v025 = XMVectorReplicate(0.25f);
	const float k = 127.5f * 1.0f / float(1 << 22);
	__vector4 _vPackScale = XMVectorReplicate(k);
	__vector4 _vThree = XMVectorReplicate(3.0f + k);

	__declspec(align(16)) byte byteControlK34[16] = {8,9,10,11,  12,13,14,15,  24,25,26,27,  28,29,30,31};
	__vector4 permControlK34 = __lvlx(&byteControlK34, 0);

	__declspec(align(16)) byte byteControl0145[16] = {0,1,2,3, 4,5,6,7, 16,17,18,19, 20,21,22,23};
	__vector4 permControl0145 = __lvlx(&byteControl0145, 0);

	int size = fftBump.GetSize();
	fft::dcomplex * frame = fftBump.GetCurFrame();

	RENDERLOCKED_RECT lr;
	m_normTex2->LockRect(0, &lr, null, 0);
	char * bits = (char*)lr.pBits;
	//dword dw1, dw2;
	//RDTSC_B(dw1);
	for (int y=0; y<size/2; y++)
	{
		fft::dcomplex * __restrict v1 = frame + y*2 * size;
		fft::dcomplex * __restrict v2 = frame + (y*2 + 1) * size;
		dword * __restrict out_bits[2];
		out_bits[0] = (dword*)&bits[y*2 * lr.Pitch];
		out_bits[1] = (dword*)&bits[(y*2+1) * lr.Pitch];
		__vector4 * out_mip = (__vector4 *)&mip2[y * size/2].Re;
		for (int x=0; x<size/8; x++)
		{
			// 0, 1		2,  3		4,  5		6,  7
			// 8, 9		10, 11		12, 13		14, 15
			__vector4 p[16];
			for (int i=0; i<8; i++)
			{
				p[i] = __lvlx(v1, i * 16);	
				p[8+i] = __lvlx(v2, i * 16);	
			}

			__vector4 pm[4];
			pm[0] = __vmulfp(p[0] + p[1] + p[8] + p[9], _v025);
			pm[1] = __vmulfp(p[2] + p[3] + p[10] + p[11], _v025);
			pm[2] = __vmulfp(p[4] + p[5] + p[12] + p[13], _v025);
			pm[3] = __vmulfp(p[6] + p[7] + p[14] + p[15], _v025);

			__stvx(__vperm(pm[0], pm[1], permControlK34), out_mip, 0);
			__stvx(__vperm(pm[2], pm[3], permControlK34), out_mip, 16);

			v1 += 8;
			v2 += 8;
			out_mip += 2;

			for (int k=0; k<2; k++)
			{
				for (int t=0; t<2; t++)
				{
					__vector4 n1 = __vmaddfp(p[k*8+t*4+0], _vPackScale, _vThree);
					//n1 = __vmaddfp(n1, _vPackScale, _vThree);

					__vector4 n2 = __vmaddfp(p[k*8+t*4+1], _vPackScale, _vThree);
					//n2 = __vmaddfp(n2, _vPackScale, _vThree);

					__vector4 n3 = __vmaddfp(p[k*8+t*4+2], _vPackScale, _vThree);
					//n3 = __vmaddfp(n3, _vPackScale, _vThree);
					
					__vector4 n4 = __vmaddfp(p[k*8+t*4+3], _vPackScale, _vThree);
					//n4 = __vmaddfp(n4, _vPackScale, _vThree);
						
					__vector4 res = __vzero();
					res = __vpkd3d(res, n1, VPACK_D3DCOLOR, VPACK_32, 3); 
					res = __vpkd3d(res, n2, VPACK_D3DCOLOR, VPACK_32, 2); 
					res = __vpkd3d(res, n3, VPACK_D3DCOLOR, VPACK_32, 1); 
					res = __vpkd3d(res, n4, VPACK_D3DCOLOR, VPACK_32, 0); 

					__stvx(res, out_bits[k], 0);

					out_bits[k] += 4;
				}
			}
		}
	}
	//RDTSC_E(dw1);
	//RDTSC_B(dw2);
	m_normTex2->UnlockRect(0);

	for (long i=1; i<5; i++)
	{
		m_normTex2->LockRect(i, &lr, null, 0);
		bits = (char*)lr.pBits;

		size >>= 1;

		for (int y=0; y<size/2; y++)
		{
			__vector4 * __restrict v1 = (__vector4*)&mip2[(y*2) * size].Re;
			__vector4 * __restrict v2 = (__vector4*)&mip2[((y*2) + 1) * size].Re;

			dword * __restrict out_bits[2];
			out_bits[0] = (dword*)&bits[y*2 * lr.Pitch];
			out_bits[1] = (dword*)&bits[(y*2+1) * lr.Pitch];

			__vector4 * out_mip = (__vector4 *)&mip2[y * size / 2].Re;

			for (int x=0; x<size/8; x++)
			{
				__vector4 p[8];
				for (int i=0; i<4; i++)
				{
					p[i] = __lvlx(v1, i * 16);	
					p[4+i] = __lvlx(v2, i * 16);	
				}

				//float fx = mip2[x + y * size].Re;
				//float fz = mip2[x + y * size].Im;

				__vector4 pm[4];
				pm[0] = p[0] + p[4]; 	pm[0] += __vpermwi(pm[0], VPERM(2, 3, 0, 2));
				pm[1] = p[1] + p[5]; 	pm[1] += __vpermwi(pm[1], VPERM(2, 3, 0, 2));
				pm[2] = p[2] + p[6]; 	pm[2] += __vpermwi(pm[2], VPERM(2, 3, 0, 2));
				pm[3] = p[3] + p[7]; 	pm[3] += __vpermwi(pm[3], VPERM(2, 3, 0, 2));

				__stvx(__vmulfp(__vperm(pm[0], pm[1], permControl0145), _v025), out_mip, 0);
				__stvx(__vmulfp(__vperm(pm[2], pm[3], permControl0145), _v025), out_mip, 16);

				for (int k=0; k<2; k++)
				{
					for (int t=0; t<2; t++)
					{
						__vector4 n1 = __vpermwi(p[k*4 + t*2 + 0], VPERM(2, 3, 0, 1));
						n1 = __vmaddfp(n1, _vPackScale, _vThree);
						//n1 = __vmaddfp(n1, _vPackScale, _vThree);

						__vector4 n2 = __vpermwi(n1, VPERM(2, 3, 0, 1));
						//__vector4 n2 = __vpermwi(p[k*4 + t*2 + 0], VPERM(2, 3, 2, 3));
						//n2 = __vmaddfp(n2, _v1275, _v1275);
						//n2 = __vmaddfp(n2, _vPackScale, _vThree);

						__vector4 n3 = __vpermwi(p[k*4 + t*2 + 1], VPERM(2, 3, 0, 1));
						n3 = __vmaddfp(n3, _vPackScale, _vThree);
						//n3 = __vmaddfp(n3, _vPackScale, _vThree);

						__vector4 n4 = __vpermwi(n3, VPERM(2, 3, 0, 1));
						//__vector4 n4 = __vpermwi(p[k*4 + t*2 + 1], VPERM(2, 3, 2, 3));
						//n4 = __vmaddfp(n4, _v1275, _v1275);
						//n4 = __vmaddfp(n4, _vPackScale, _vThree);
							
						__vector4 res = __vzero();
						res = __vpkd3d(res, n1, VPACK_D3DCOLOR, VPACK_32, 3); 
						res = __vpkd3d(res, n2, VPACK_D3DCOLOR, VPACK_32, 2); 
						res = __vpkd3d(res, n3, VPACK_D3DCOLOR, VPACK_32, 1); 
						res = __vpkd3d(res, n4, VPACK_D3DCOLOR, VPACK_32, 0); 

						__stvx(res, out_bits[k], 0);

						out_bits[k] += 4;
					}
				}

				v1 += 4;
				v2 += 4;
				out_mip += 2;
			}
		}

		m_normTex2->UnlockRect(i);
	}
	for (long i=5; i<8; i++)
	{
		m_normTex2->LockRect(i, &lr, null, 0);
		bits = (char*)lr.pBits;

		size >>= 1;

		for (int y=0; y<size; y++)
			for (int x=0; x<size; x++)
			{
				float fx = mip2[x + y * size].Re;
				float fz = mip2[x + y * size].Im;

				mip2[x + y * size].Re = 0.0f;
				mip2[x + y * size].Im = 0.0f;

				mip2[x / 2 + y / 2 * (size / 2)].Re += fx * 0.25f;
				mip2[x / 2 + y / 2 * (size / 2)].Im += fz * 0.25f;

				fx = fx * 127.5f + 127.5f;
				fz = fz * 127.5f + 127.5f;

				*(dword*)&bits[y*lr.Pitch + x*4] = ARGB(fz, 0, 0, fx);
			}

		m_normTex2->UnlockRect(i);
	}
	//RDTSC_E(dw2);
	//api->Trace("dw1-2 = %d, %d", dw1, dw2);
#endif

	return;
}

Ocean2::Ocean2() :
	m_waterParts(_FL_, 2),
	m_blocks(_FL_, 112),
	m_flatBlocks(_FL_, 12)
{
	pReflIterator = null;

	m_seaHeight = 0.0f;

	m_bumpPosU = 0.0f;
	m_bumpPosV = 0.0f;
	m_bumpAngle = 0.0f;

	m_normalDisplacement = 1.4f;

	m_foamU = 0.0f;
	m_foamV = 0.0f;

	m_offBigOcean = false;
	m_referenceExecRefr = false;
	
	m_executor.Reset();
	m_refractor.Reset();

	m_wavesMove = 0.0f;
	m_wavesMoveSpeed = 0.0f;

	wave_len = -100.0f;
	wave_amp = -100.0f;
	wind_pwr = -100.0f;
	wave_drx = -100.0f;
	wave_drz = -100.0f;
	wave_frm = -100.0f;
	wave_spd = -100.0f;

	wave_len2 = -100.0f;
	wave_amp2 = -100.0f;
	wind_pwr2 = -100.0f;
	wave_drx2 = -100.0f;
	wave_drz2 = -100.0f;
	wave_frm2 = -100.0f;
	wave_spd2 = -100.0f;
}

Ocean2::~Ocean2()
{
	// дожидаемся окончания работы экзекутора, если он был запущен
	m_executor.Ptr()->StopWorking();

	RELEASE(pReflIterator);

	if (m_referenceExecRefr)
	{
		m_referenceExecRefr = false;
		
		m_executor.Ptr()->DecRef(this);
		m_refractor.Ptr()->DecRef();
	}
}

//Создание объекта
bool Ocean2::Create(MOPReader & reader)
{
	Registry(OCEAN_GROUP3);

	static const ConstString id_Ocean3Executor("Ocean3Executor");	
	Mission().CreateObject(m_executor.GetSPObject(), "Ocean3Executor", id_Ocean3Executor, true);

	static const ConstString id_Ocean3Refractor("Ocean3Refractor");
	Mission().CreateObject(m_refractor.GetSPObject(), "Ocean3Refractor", id_Ocean3Refractor, true);

	m_oceans = (Ocean3Service *)api->GetService("Ocean3Service");

	// Инициализируем все общие переменные из FX, текстуры
	m_bumpFrame = 0.0f;

	m_oceans->m_regenerateSphereMap = true;

	pReflIterator = &Mission().GroupIterator(MG_SEAREFLECTION, _FL_);

	bool res = EditMode_Update(reader);

	return res;
}

//Обновление параметров
bool Ocean2::EditMode_Update(MOPReader & reader)
{
	float wave_len1, wave_amp1, wind_pwr1, wave_drx1, wave_drz1, wave_frm1;
	float wave_len3, wave_amp3, wind_pwr3, wave_drx3, wave_drz3, wave_frm3;

	m_oceanEnable = reader.Bool();
	m_oceanAnimate = (reader.Bool()) ? 0.0f : 1.0f;
	m_oceanEnableRealWaves = reader.Bool();
	
	connectToName = reader.String();

	m_seaHeight = reader.Float();
	m_mainReflectionHeight = reader.Bool();
	m_mainFFTSource = reader.Bool();
	m_hasOwnReflection = reader.Bool();
	m_seaFakeHeight = reader.Float();

	m_skyColor = reader.Colors();
	m_waterColor = reader.Colors();

	isNoSwing = reader.Bool();

	// FFT for waves
		wave_len1 = reader.Float();
		wave_amp1 = reader.Float();
		wind_pwr1 = reader.Float();
		Vector wave_dir1 = reader.Angles();
		wave_drx1 = cosf(wave_dir1.y);
		wave_drz1 = sinf(wave_dir1.y);
		wave_frm1 = reader.Float();
		wave_spd = reader.Float();

	m_wavesAmplitude = reader.Float();
	m_wavesScale = reader.Float();
	m_maxWavesDistance = Min(2800.0f, reader.Float());
	m_wavesMoveSpeed = reader.Position();
	m_posShift = reader.Float() / 5.0f * sqrtf(256.0f / m_oceans->fftWaves.GetSize());

	m_sunRoadEnable = reader.Bool();
	Vector sunRoadAngle = reader.Angles();
	sunRoadAngle.x -= PI;
	m_sunRoadParams.v = (Vector(0.0f, 0.0f, 1.0f) * Matrix().BuildRotateX(sunRoadAngle.x)) * Matrix().BuildRotateY(sunRoadAngle.y);
	m_sunRoadColor = reader.Colors();
	m_sunBumpScale = reader.Float() * 1.0f;
	m_sunRoadColor *= reader.Float() * 0.01f;
	m_sunRoadParams.w = reader.Float();

	Vector sunLight = reader.Angles();
	m_sunLight.v = (Vector(0.0f, 0.0f, 1.0f) * Matrix().BuildRotateX(sunLight.x)) * Matrix().BuildRotateY(sunLight.y);
	m_sunLight.w = 0.0f;
	m_sunLightParams.x = reader.Float();	// mul
	m_sunLightParams.y = reader.Float();	// add
	m_sunLightParams.z = m_sunLightParams.w = 0.0f;

	// транслюценция
	m_translucenceEnable = reader.Bool();
	m_translucenceLight = 0.0f;
	m_translucenceLight.x = reader.Float();
	m_translucenceParams = 0.0f;
	m_translucenceParams.x = reader.Float();
	m_translucenceParams.y = reader.Float();
	m_translucenceColor = reader.Colors();

	m_foamEnable = reader.Bool();
	m_foamColor = reader.Colors();
	m_foamColor *= reader.Float();
	m_foamAngles = reader.Angles();
	m_foamStartY = reader.Float();
	m_foamHeightY = reader.Float();
	m_foamPowerY = reader.Float();
	m_foamUVScale = reader.Float();
	m_foamDisplacement = reader.Float();
	m_foamMoveAngles = reader.Angles();
	m_foamMoveSpeed = reader.Float();

	m_fogColor = reader.Colors();
	m_fogMultiply = reader.Float();
	m_fogStart = reader.Float();
	m_fogDistance = reader.Float();

	m_reflectParticles = reader.Bool();
	m_ReflectionMinimum = reader.Float();
	m_ReflectionMultiply = reader.Float();
	m_ReflectionPower = reader.Float();
	m_ReflectionEnvPower = reader.Float();
	m_ReflectionBlendSkyObj = reader.Float();
	m_ReflectionMin = reader.Float();
	m_ReflectionMax = reader.Float();
	m_RefractionPower = reader.Float();
	m_TransparencyMin = reader.Float();
	m_TransparencyMax = reader.Float();

	m_refractionParams.x = m_RefractionPower * 0.001f;
	m_refractionParams.y = m_ReflectionEnvPower * 0.001f;
	m_refractionParams.z = m_ReflectionMultiply;
	m_refractionParams.w = m_ReflectionPower;

	m_refractionParams2.x = m_ReflectionMinimum;
	m_refractionParams2.y = m_ReflectionBlendSkyObj;
	m_refractionParams2.z = 0.0f;
	m_refractionParams2.w = 0.0f;

	// FFT for bump texture
		wave_len3 = reader.Float();
		wave_amp3 = reader.Float();
		wind_pwr3 = reader.Float();
		Vector wave_dir3 = reader.Angles();
		wave_drx3 = cosf(wave_dir3.y);
		wave_drz3 = sinf(wave_dir3.y);
		wave_frm3 = reader.Float();
		wave_spd2 = reader.Float();

	m_bumpAngle = Deg2Rad(reader.Float());
	m_initialBumpScaleU = reader.Float();
	m_initialBumpScaleV = reader.Float();
	m_bumpScaleU = m_initialBumpScaleU * 0.004f;
	m_bumpScaleV = m_initialBumpScaleV * 0.004f;
	m_bumpPower = reader.Float();
	m_bumpMoveSpeed = reader.Float() * 0.005f;

	// зачитываем кусочки воды
	m_waterParts.DelAll();
	long numWaters = reader.Array();
	for (long i=0; i<numWaters; i++)
	{
		bool isMirror = reader.Bool();

		float wpDetail = 1.0f / reader.Float();

		float m_bumpFade = reader.Float();
		float leftDist = 1.0f / Max(0.0001f, reader.Float());
		float leftPower = reader.Float() * 255.0f;
		float rightDist = 1.0f / Max(0.0001f, reader.Float());
		float rightPower = reader.Float() * 255.0f;
		float upDist = 1.0f / Max(0.0001f, reader.Float());
		float upPower = reader.Float() * 255.0f;
		float bottomDist = 1.0f / Max(0.0001f, reader.Float());
		float bottomPower = reader.Float() * 255.0f;
		bool enableWPT = reader.Bool();

		long numParts = reader.Array();
		// зачитываем все части воды - и если отличаются от предыдущих - то 

		array<Vector> tmpV(_FL_, 64);
		tmpV.Empty();

		Vector l1 = reader.Position();
		Vector r1 = reader.Position();
		tmpV.Add(l1);
		tmpV.Add(r1);
		float len = (r1 - l1).GetLength();
		long numUPoints = Max(long(5), long((r1 - l1).GetLength() * wpDetail)); 

		// если кол-во частей воды = 1, то скипаем ее
		if (numParts == 1) continue;

		WaterPart & part = m_waterParts[m_waterParts.Add()];
		part.isMirror = isMirror;
		part.numParts = numParts;
		part.bumpFade = m_bumpFade;

		part.vBufferFlat = Render().CreateVertexBuffer(sizeof(Vector) * (numParts - 1) * 6, sizeof(Vector), _FL_);
		Vector * flatv = (Vector *)part.vBufferFlat->Lock();

		part.boxMin.Min(l1, r1);
		part.boxMax.Max(l1, r1);

		float totalLeftDistance = 0.0f;
		float totalRightDistance = 0.0f;

		// считаем кол-во вертексов
		long numv = 0;
		for (long j=0; j<numParts-1; j++)
		{
			Vector l2 = reader.Position();
			Vector r2 = reader.Position();
			tmpV.Add(l2);
			tmpV.Add(r2);

			part.boxMin.Min(l2); part.boxMin.Min(r2);
			part.boxMax.Max(l2); part.boxMax.Max(r2);

			*flatv++ = l1;	*flatv++ = r1;	*flatv++ = l2;
			*flatv++ = r1;	*flatv++ = r2;	*flatv++ = l2;

			float len1 = (l2 - l1).GetLength();
			float len2 = (r2 - r1).GetLength();
			long num = Max(long(2), Max(long(len1 * wpDetail), long(len2 * wpDetail)));
			numv += (j == 0) ? num : num - 1;

			totalLeftDistance += len1;
			totalRightDistance += len1;

			l1 = l2; 
			r1 = r2;
		}

		part.vBufferFlat->Unlock();

		part.numUPoints = numUPoints;

		part.curBuffer = 0;

		part.numVerts = numv * numUPoints;
		part.numTrgs = (numv - 1) * (numUPoints - 1) * 2;
		if (part.numTrgs * 3 > 65000)
		{
			part.isBad = true;
			continue;
		}
		part.isBad = false;
#ifdef _XBOX
		part.vBuffer[0] = Render().CreateVertexBuffer(part.numVerts * sizeof(Ocean3Service::SeaVertex), sizeof(Ocean3Service::SeaVertex), _FL_, OCEAN3_DYNAMIC, POOL_DEFAULT);
		part.vBuffer[1] = Render().CreateVertexBuffer(part.numVerts * sizeof(Ocean3Service::SeaVertex), sizeof(Ocean3Service::SeaVertex), _FL_, OCEAN3_DYNAMIC, POOL_DEFAULT);
#else
		part.vBuffer[0] = Render().CreateVertexBuffer(part.numVerts * sizeof(Ocean3Service::SeaVertex), sizeof(Ocean3Service::SeaVertex), _FL_, USAGE_WRITEONLY | USAGE_DYNAMIC, POOL_DEFAULT);
		part.vBuffer[1] = null;
#endif
		part.verts = NEW WaterVector[numv * numUPoints];
		WaterVector * verts = part.verts;

		part.iBuffer = Render().CreateIndexBuffer(part.numTrgs * 3 * sizeof(word), _FL_);
		word * pI = (word *)part.iBuffer->Lock();
		for (long z=0; z<numv - 1; z++)
			for (long x=0; x<numUPoints - 1; x++)
			{
				*pI++ = word((z + 0) * numUPoints + x + 1);
				*pI++ = word((z + 1) * numUPoints + x);
				*pI++ = word((z + 0) * numUPoints + x);

				*pI++ = word((z + 0) * numUPoints + x + 1);
				*pI++ = word((z + 1) * numUPoints + x + 1);
				*pI++ = word((z + 1) * numUPoints + x);
			}
		part.iBuffer->Unlock();

		l1 = tmpV[0];
		r1 = tmpV[1];

		// 
		float ldist = 0.0f;
		float rdist = 0.0f;

		for (long j=0; j<numParts-1; j++)
		{
			Vector l2 = tmpV[(j + 1) * 2 + 0];
			Vector r2 = tmpV[(j + 1) * 2 + 1];
			float len1 = (l2 - l1).GetLength();
			float len2 = (r2 - r1).GetLength();
			long numVPoints = Max(long(2), Max(long(len1 * wpDetail), long(len2 * wpDetail)));

			for (long y=(j==0) ? 0 : 1; y<numVPoints; y++)
			{
				float delta = y / (numVPoints - 1.0f);
				Vector v1 = l1 + delta * (l2 - l1);
				Vector v2 = r1 + delta * (r2 - r1);
				float dist = (v2 - v1).GetLength();
				float ld = ldist + delta * len1;
				float rd = rdist + delta * len2;
				for (long x=0; x<numUPoints; x++)
				{
					float k = float(x) / float(numUPoints - 1.0f);
					Vector v = v1 + (v2 - v1) * k;
					
					verts->v = v;
					if (enableWPT)
					{
						float fromUp = Lerp(ld, rd, k);
						float fromBottom = Lerp(totalLeftDistance, totalRightDistance, k) - fromUp;

						float k1 = (1.0f - Clamp((dist * k) * leftDist)) * leftPower;
						float k2 = (1.0f - Clamp((dist * (1.0f - k)) * rightDist)) * rightPower;
						float k3 = (1.0f - Clamp((fromUp) * upDist)) * upPower;
						float k4 = (1.0f - Clamp((fromBottom) * bottomDist)) * bottomPower;
						dword c = dword(Min(255.0f, k1 + k2 + k3 + k4) * 0.25f);
#ifndef _XBOX
						verts->color = ((c & 7L) << 11L) | ((c & 0x38) << (16L + 11L - 3L));
#else
						verts->color = ((c & 0x38) << (11L - 3L)) | ((c & 0x7L) << (16L + 11L));
#endif
					}
					else
						verts->color = 0;
					verts++;
				}
			}

			ldist += len1;
			rdist += len2;

			l1 = l2;
			r1 = r2;
		}
	}

	if (fabsf(wave_len1 - wave_len) > 1e-5f ||
		fabsf(wave_amp1 - wave_amp) > 1e-5f ||
		fabsf(wind_pwr1 - wind_pwr) > 1e-5f ||
		fabsf(wave_drx1 - wave_drx) > 1e-5f ||
		fabsf(wave_drz1 - wave_drz) > 1e-5f ||
		fabsf(wave_frm1 - wave_frm) > 1e-5f )
	{
		wave_len = wave_len1;
		wave_amp = wave_amp1;
		wind_pwr = wind_pwr1;
		wave_drx = wave_drx1;
		wave_drz = wave_drz1;
		wave_frm = wave_frm1;

		if (EditMode_IsOn())
			m_oceans->fftWaves.Setup(wave_len, wave_amp * 1.0e-10f, wind_pwr, wave_drx, wave_drz, wave_frm);
		//m_oceans->fftBump.Setup(wave_len, wave_amp * 1.0e-10f, wind_pwr, wave_drx, wave_drz);
	}

	if (fabsf(wave_len3 - wave_len2) > 1e-5f ||
		fabsf(wave_amp3 - wave_amp2) > 1e-5f ||
		fabsf(wind_pwr3 - wind_pwr2) > 1e-5f ||
		fabsf(wave_drx3 - wave_drx2) > 1e-5f ||
		fabsf(wave_drz3 - wave_drz2) > 1e-5f ||
		fabsf(wave_frm3 - wave_frm2) > 1e-5f )
	{
		wave_len2 = wave_len3;
		wave_amp2 = wave_amp3;
		wind_pwr2 = wind_pwr3;
		wave_drx2 = wave_drx3;
		wave_drz2 = wave_drz3;
		wave_frm2 = wave_frm3;

		if (EditMode_IsOn())
			m_oceans->fftBump.Setup(wave_len2, wave_amp2 * 1.0e-10f, wind_pwr2, wave_drx2, wave_drz2, wave_frm2);
		//m_oceans->fftBump.Setup(wave_len, wave_amp * 1.0e-10f, wind_pwr, wave_drx, wave_drz);
	}
		
	m_wavesScale *= float(m_oceans->fftWaves.GetSize()) / wave_len;

	bool bVisible = reader.Bool();

	Show(bVisible);

	return true;
}

void Ocean2::SetupFFT()
{
	m_oceans->fftWaves.Setup(wave_len, wave_amp * 1.0e-10f, wind_pwr, wave_drx, wave_drz, wave_frm);
	m_oceans->fftBump.Setup(wave_len2, wave_amp2 * 1.0e-10f, wind_pwr2, wave_drx2, wave_drz2, wave_frm2);
}

void Ocean2::Show(bool isShow)
{
	/*if (IsShow() == isShow)
		return;*/

	MissionObject::Show(isShow);

	DelUpdate();

	if (IsShow())
	{
		SetUpdate(&Ocean2::First, ML_FIRST + 5);
		SetUpdate(&Ocean2::FakeFill, ML_ALPHA3 - 3);
		SetUpdate(&Ocean2::ClearScreen, ML_FILL - 1);
		SetUpdate(&Ocean2::Execute, ML_EXECUTE1 - 2);
		SetUpdate(&Ocean2::Realize, ML_ALPHA3 - 2);
#ifndef STOP_DEBUG
		SetUpdate((MOF_UPDATE)&Ocean2::DebugRealize, ML_LAST);
#endif

		if (!m_referenceExecRefr)
		{
			m_referenceExecRefr = true;
			m_executor.Ptr()->AddRef();
			m_refractor.Ptr()->AddRef();
		}

		m_offBigOcean = false;

		// ищем активные моря и отключаем у них m_oceanEnable
		if (m_oceanEnable)
		{
			MGIterator * iter = &Mission().GroupIterator(OCEAN_GROUP3, _FL_);
			for (;!iter->IsDone();iter->Next())
			{
				Ocean2 * o2 = (Ocean2 *)iter->Get();
				if (o2->m_oceanEnable && o2 != this && o2->IsShow())
				{
					o2->m_offBigOcean = true;
					LogicDebugError("Ocean2: Ocean surface '%s' was disabled, potential double/tripple/quad oceans surfaces active at the same time.", iter->Get()->GetObjectID().c_str());
				}
			}
			iter->Release();		
		}
	}
	else
	{
		DelUpdate(&Ocean2::First);
		DelUpdate(&Ocean2::FakeFill);
		DelUpdate(&Ocean2::ClearScreen);
		DelUpdate(&Ocean2::Execute);
		DelUpdate(&Ocean2::Realize);
#ifndef STOP_DEBUG
		DelUpdate(&Ocean2::DebugRealize);
#endif
		if (m_referenceExecRefr)
		{
			m_referenceExecRefr = false;
			m_executor.Ptr()->DecRef(this);
			m_refractor.Ptr()->DecRef();
		}
	}

	if (!EditMode_IsOn())
	{
		Console().Trace(COL_ALL, "Ocean '%s' %s", GetObjectID().c_str(), (IsShow()) ? "enabled" : "disabled");
		LogicDebug("Ocean '%s' %s", GetObjectID().c_str(), (IsShow()) ? "enabled" : "disabled");
	}
}

// Вызываеться, когда все объекты созданны но ещё не началось исполнение миссии
void Ocean2::PostCreate()
{
	FindObject(connectToName, connectToPtr);
}

//Активирование/деактивирование объекта
void Ocean2::Activate(bool isActive)
{
}

bool Ocean2::BoxIsVisible(const Plane * frustum, const Vector & vMin, const Vector & vMax)
{
	for(long i=0; i<5; i++)
	{
		const Plane & p = frustum[i];

		if (p.Dist(Vector(vMin.x, vMin.y, vMin.z)) > 0.0f) continue;
		if (p.Dist(Vector(vMax.x, vMin.y, vMin.z)) > 0.0f) continue;
		if (p.Dist(Vector(vMin.x, vMin.y, vMax.z)) > 0.0f) continue;
		if (p.Dist(Vector(vMax.x, vMin.y, vMax.z)) > 0.0f) continue;
		if (p.Dist(Vector(vMin.x, vMax.y, vMin.z)) > 0.0f) continue;
		if (p.Dist(Vector(vMax.x, vMax.y, vMin.z)) > 0.0f) continue;
		if (p.Dist(Vector(vMin.x, vMax.y, vMax.z)) > 0.0f) continue;
		if (p.Dist(Vector(vMax.x, vMax.y, vMax.z)) > 0.0f) continue;

		return false;
	}

	return true;
}

void _cdecl Ocean2::Execute(float fDeltaTime, long level)
{ 
	if (!IsWorking()) return;

	//api->Trace("============================");

	m_totalRDTSC = 0;
	m_totalWaveXZRDTSC = 0;
	m_totalWaveXZVerts = 0;

	if(connectToPtr.Validate())
		connectToPtr.Ptr()->GetMatrix(m_connectMtx);
	else
		m_connectMtx.SetIdentity();

	// FIX-ME: Тут злобный хак, надо подумать как пофиксить
	// Без этих строчек падает, тока когда океан единственный объект в сцене
	Render().SetStreamSource(0, null, 0);
	Render().SetStreamSource(1, null, 0);

	ProfileTimer t2;
	Matrix savedView = Render().GetView();
	m_View = m_connectMtx * savedView;
	// FIX-ME - это только для того чтобы получить правильные frustum planes
	Render().SetView(m_View);

	const Plane * frustumPlanes = Render().GetFrustum();
	m_numFrustumPlanes = Min(Render().GetNumFrustumPlanes(), dword(6));
	for (dword i=0; i<m_numFrustumPlanes; i++)
		m_frustumPlanes[i] = frustumPlanes[i];

	m_deltaTime = fDeltaTime;
	m_camPos = m_View.GetCamPos();
	m_camDir = Matrix(m_View).Inverse().vz;

	m_wavesMove += m_wavesMoveSpeed * fDeltaTime * m_oceanAnimate;

	m_bumpPosU += sinf(m_bumpAngle) * m_bumpMoveSpeed * fDeltaTime * m_oceanAnimate;
	m_bumpPosV += cosf(m_bumpAngle) * m_bumpMoveSpeed * fDeltaTime * m_oceanAnimate;

	m_visibleAnyWater = false;

	m_pVSea = null;
	if (m_oceanEnable && !m_offBigOcean)
	{
	// на xbox'e лочим вертекс буффер прямо в WorkThread(), потому что он на самом деле и так является куском памяти
#ifdef _XBOX
		m_pVSea = null;
#endif

		m_visibleAnyWater = true;

		if (fabsf(m_seaFakeHeight) > 0.5f)
			m_oceans->m_isOceansHaveFakeHeight = true;
	}

	if (m_oceanEnable && !m_offBigOcean)
	{
		if (m_oceans->m_iSeaBuffer->IsLocked())
			m_oceans->m_iSeaBuffer->Unlock();
		if (m_oceans->m_vSeaBuffer->IsLocked())
			m_oceans->m_vSeaBuffer->Unlock();

		pTriangles = (dword *)m_oceans->m_iSeaBuffer->Lock(0, 0, LOCK_DISCARD);
		m_pVSea = (Ocean3Service::SeaVertex *)m_oceans->m_vSeaBuffer->Lock(0, 0, LOCK_DISCARD);
	}

	t2.Stop();

	//api->Trace("numVis = %d", numVis);

	ProfileTimer t3;
	// лочим все нужные буффера
	for (long i=0; i<m_waterParts.Len(); i++)
	{
		WaterPart & wp = m_waterParts[i];
		
		// проверяем видимость бокса
		wp.isVisible = BoxIsVisible(m_frustumPlanes, wp.boxMin, wp.boxMax);

		if (!wp.isVisible)
			continue;

		m_visibleAnyWater = true;

		if (wp.isBad)
		{
#ifdef STOP_DEBUG
			wp.isVisible = false;
#endif
			continue;
		}

		wp.vsea = (Ocean3Service::SeaVertex *)m_waterParts[i].vBuffer[m_waterParts[i].curBuffer]->Lock(0, 0, LOCK_DISCARD);
	}
	t3.Stop();

	ProfileTimer t4;
	// 1 раз в кадр генерим шум и нормали из него 
	//if (m_oceanEnable && !m_offBigOcean)
		//GenerateBumpTexture(false);
	t4.Stop();
	
	m_executor.Ptr()->AddToExecute(this);
	// запускаем обсчет океана в отдельном потоке

	// восстанавливаем view матрицу
	Render().SetView(savedView);

	//if (api->DebugKeyState('4'))
	{
		//api->Trace("t2 = %d, t3 = %d, t4 = %d", t2.GetTime32(), t3.GetTime32(), t4.GetTime32());
	}

	//api->Trace("ticks = %d, %d", ticks, nums);
}

void Ocean2::GenerateBumpTexture()
{
	m_oceans->GenerateBumpMap();
}

void _cdecl Ocean2::FakeFill(float fDeltaTime, long level)
{
	//return;
#ifdef _XBOX
	if (!IsWorking()) 
		return;

	DrawFlatOceanAndParts(true);
#endif
}

void _cdecl Ocean2::First(float fDeltaTime, long level)
{
	m_iDebugDraw = 0;
	m_iNumReflections = 0;

	//m_oceans->m_isFirstRealize = true;

	//GenerateSphereMap();

	m_oceans->m_isOceansHaveFakeHeight = false;
}

void _cdecl Ocean2::ClearScreen(float fDeltaTime, long level)
{
	if (!EditMode_IsOn())
		m_oceans->pShdServ->ClearScreen();
}

bool Ocean2::IsWorking(bool visibleCheck) const
{
	if (!IsShow()) return false;
	if (EditMode_IsOn() && !EditMode_IsVisible()) return false;
	if (api->DebugKeyState('O')) return false;
	
	if (visibleCheck)
		return IsOceanOrWatersVisible();

	return true;
}

#define VPERM(x, y, z, w)	((unsigned int)(x << 6L) | (y << 4L) | (z << 2L) | w)

void Ocean2::WorkThreadPreFFT()
{
	if (!m_visibleAnyWater)
		return;

	m_fftCurFrame = m_oceans->fftWaves.GetCurFrame();

	if (m_oceanEnable && !m_offBigOcean && m_pVSea && pTriangles)
	{
		float fBlockSize = 256.0f * fGridStep;
		long iNumBlocks = (long)dwMaxDim / (256 * 2);

		// was 0.11f
		fLodScale = 0.112f;

		vSeaCenterPos.x = fBlockSize * (long(m_camPos.x / fBlockSize) - iNumBlocks);
		vSeaCenterPos.y = m_seaHeight + m_seaFakeHeight;
		vSeaCenterPos.z = fBlockSize * (long(m_camPos.z / fBlockSize) - iNumBlocks);

		m_VStart = 0;
		m_TStart = 0;
		m_IStart = 0;

		m_blocks.Empty();
		BuildTree(0, 0, 0);
	
		//Убраем блоки которые дальше чем m_maxWavesDistance в другой массив
		float wavesDistance = m_maxWavesDistance;//Max(1000.0f, m_maxWavesDistance);
		m_flatBlocks.Empty();
		int numZero = 0, zverts = 0;
		Vector camPos2D = Vector(m_camPos.x, 0.0f, m_camPos.z);
		for (int i=0; i<m_blocks.Len(); i++)
		{
			const SeaBlock & block = m_blocks[i];

			float	fStep = fGridStep * float(1 << block.iLOD);
			float	fSize = fGridStep * block.iSize;
			int	size0 = block.iSize >> block.iLOD;
			
			float x1 = float(block.iTX * block.iSize) * fGridStep;
			float y1 = float(block.iTY * block.iSize) * fGridStep;
			float x2 = x1 + float(size0) * fStep;
			float y2 = y1 + float(size0) * fStep;

			x1 += vSeaCenterPos.x;	x2 += vSeaCenterPos.x;
			y1 += vSeaCenterPos.z;	y2 += vSeaCenterPos.z;

			if ((Vector(x1, 0.0f, y1) - camPos2D).GetLengthXZ2() > Sqr(wavesDistance) &&
				(Vector(x2, 0.0f, y1) - camPos2D).GetLengthXZ2() > Sqr(wavesDistance) &&
				(Vector(x1, 0.0f, y2) - camPos2D).GetLengthXZ2() > Sqr(wavesDistance) &&
				(Vector(x2, 0.0f, y2) - camPos2D).GetLengthXZ2() > Sqr(wavesDistance))
			{
				numZero++;
				zverts += Sqr(block.iSize0 + 1);
				
				m_flatBlocks.Add(Vector4(x1, y1, x2, y2));

				m_blocks.ExtractNoShift(i);
				i--;
			}
		}

		//api->Trace("total = %d, numZeroes = %d, zv = %d", m_blocks.Len(), numZero, zverts);

		m_blocks.QSort(SeaBlock::QSort);

		if (!m_blocks.IsEmpty())
		{
			dword dw1;
			RDTSC_B(dw1);
			//m_blocks.Empty();
			int numVerts = 0;
			int numEdgeIndices = 0;
			for (int i=m_blocks.Last(); i>=0; i--) 
			{
				numVerts += Sqr(m_blocks[i].iSize0 + 1);
				numEdgeIndices += 4 * (m_blocks[i].iSize0 + 1);
				
				if (numVerts >= MAX_VERTICES - 32 || numEdgeIndices >= MAX_EDGE_INDICES - 32)
				{
					api->Trace("Sea blocks skipped, num = %d, verts = %d, indices = %d", i+1, numVerts, numEdgeIndices);
					m_blocks.DelRange(0, i);
					break;
				}
			}

			//api->Trace("verts = %d, indices = %d", numVerts, numEdgeIndices);

			memset(m_oceans->m_indices, 0xFF, MAX_EDGE_INDICES * sizeof(m_oceans->m_indices[0]));

			for (int i=0; i<m_blocks.Len(); i++)
				PrepareIndicesForBlock(i);

			RDTSC_E(dw1);
			m_totalRDTSC += dw1;
		}
	}
}

void Ocean2::WorkThread()
{
	if (!m_visibleAnyWater)
		return;

	m_fftCurFrame = m_oceans->fftWaves.GetCurFrame();

#ifdef _XBOX
	//__VMXSetReg(vamp, XMVectorReplicate(m_wavesAmplitude));
	__VMXSetReg(vscale, XMVectorReplicate(m_wavesScale));
	__VMXSetReg(vmove, XMVectorSet(m_wavesMove.x, m_wavesMove.z, m_wavesMove.x, m_wavesMove.z));
	__VMXSetReg(v0011, XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f));
	
	int fftSize = m_oceans->fftWaves.GetSize();
	__VMXSetReg(vxwidth, XMVectorSetInt(fftSize-1, fftSize-1, fftSize-1, fftSize-1));
	__VMXSetReg(vfftsize, XMVectorSetInt(fftSize, fftSize, fftSize, fftSize));

	__VMXSetReg(vxzero, XMVectorSetInt(0, 0, 0, 0));
	__VMXSetReg(vmask_y, XMVectorSetInt(0, 0xFFFFFFFF, 0, 0));

	__declspec(align(16)) byte byteControl0127[16] = {0,1,2,3, 4,5,6,7, 8,9,10,11, 28,29,30,31};
	__VMXSetReg(vpermControl0127, __lvlx(&byteControl0127, 0));
	__declspec(align(16)) byte byteControl_VXVZ_XZ[16] = {0,1,2,3, 4,5,6,7, 16,17,18,19, 24,25,26,27};
	__VMXSetReg(vpermControl_VXVZ_XZ, __lvlx(&byteControl_VXVZ_XZ, 0));
	__VMXSetReg(v_05_4096, XMVectorSet(0.0f, 0.0f, 0.5f * 2048.0f, 0.5f * 2048.0f));
	__VMXSetReg(v_shift_normals, XMVectorSetInt(0, 0, 16, 0));
	__VMXSetReg(vmask1000, XMVectorSetInt(0xFFFFFFFF, 0, 0, 0));
	__VMXSetReg(vmask1010, XMVectorSetInt(0xFFFFFFFF, 0, 0xFFFFFFFF, 0));
	__VMXSetReg(vx_shift_y_fftSize, XMVectorSetInt(4, 4 + m_oceans->m_fftWavesSize, 4, 4 + m_oceans->m_fftWavesSize));

#endif

	// считаем океан, если есть и работает и видимый
	if (m_oceanEnable && !m_offBigOcean && m_pVSea && pTriangles && (!m_blocks.IsEmpty() || !m_flatBlocks.IsEmpty()))
	{
		dword dw1;
		RDTSC_B(dw1);

		m_valpha = 0;
#ifndef _XBOX
		for (int i=0; i<m_blocks.Len(); i++)
			WaveXZBlock_PC(m_blocks[i]); 
#else
		bool m_original = api->DebugKeyState(ICore::xb_dpad_left);

		__VMXSetReg(valpha, XMVectorSetInt(0, 0, 0, 0));

		if (m_original)
			for (int i=0; i<m_blocks.Len(); i++)
				WaveXZBlock_PC(m_blocks[i]); 
		else
			for (int i=0; i<m_blocks.Len(); i++)
				WaveXZBlock_x360(m_blocks[i]); 

#endif		
		// дорисовываем плоские блоки
		Ocean3Service::SeaVertex * __restrict verts = m_pVSea;
		int VStart = m_VStart;
		for (int i=0; i<m_flatBlocks.Len(); i++)
		{
			const float & x1 = m_flatBlocks[i].x;
			const float & z1 = m_flatBlocks[i].y;

			const float & x2 = m_flatBlocks[i].z;
			const float & z2 = m_flatBlocks[i].w;
			
			verts->pos = Vector(x1, 0.0f, z1);			verts->nrm = 0;			verts++;
			verts->pos = Vector(x2, 0.0f, z1);			verts->nrm = 0;			verts++;
			verts->pos = Vector(x2, 0.0f, z2);			verts->nrm = 0;			verts++;
			verts->pos = Vector(x1, 0.0f, z2);			verts->nrm = 0;			verts++;

			*pTriangles++ = DW_F_2W(VStart + 0, VStart + 1);
			*pTriangles++ = DW_F_2W(VStart + 2, VStart + 0);
			*pTriangles++ = DW_F_2W(VStart + 2, VStart + 3);

			VStart += 4;
		}

		m_VStart += m_flatBlocks.Len() * 4;
		m_TStart += m_flatBlocks.Len() * 2;

		RDTSC_E(dw1);
		m_totalRDTSC += dw1;
	}

#ifdef _XBOX
	//m_pVSea = (Ocean3Service::SeaVertex *)m_oceans->m_vSeaBuffer->Lock();
#endif

	// обсчитываем куски воды
	// TO-DO доделать куски воды на основе FFT
	for (long i=0; i<m_waterParts; i++)
	{
		WaterPart & wp = m_waterParts[i];

		if (!wp.isVisible || wp.isBad || !wp.vsea) 
			continue;

		__declspec(align(16)) Vector4 vtmp = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
		for (long j=0; j<wp.numVerts; j++)
		{
			vtmp.v = wp.verts[j].v;
			m_valpha = wp.verts[j].color;
#ifdef _XBOX
			__VMXSetReg(valpha, XMVectorSetInt(0, 0, 0, wp.verts[j].color));
#endif
			WaveXZ(vtmp, wp.vsea);
			wp.vsea++;
		}
	}

#ifdef _XBOX
	//m_oceans->m_vSeaBuffer->Unlock();
#endif
}

#ifndef STOP_DEBUG
void _cdecl Ocean2::DebugRealize(float fDeltaTime, long level)
{
	if (api->DebugKeyState(VK_CONTROL, VK_SHIFT, 'S'))
	{
		m_bDebugDraw ^= 1;
		Sleep(100);
	}
	
	if (!m_bDebugDraw) 
		return;

	if (m_iDebugDraw == 0)
		Render().Print(10.0f, 20.0f, 0xFFFFFFFF, "Current oceans: (reflections: %d)", m_iNumReflections);

	m_iDebugDraw++;
	Render().Print(20.0f, m_iDebugDraw * 19.0f + 20.0f, 0xFFFFFF00, "%s :", GetObjectID().c_str());

	bool isFFTSource = IsMainFFTSource() | (m_executor.Ptr()->GetBumpOcean() == this);

	if (m_oceanEnable && !m_offBigOcean)
	{
		m_iDebugDraw++;
		Render().Print(30.0f, m_iDebugDraw * 19.0f + 20.0f, 0xFFBFBFBF, "ocean: verts(%d) %s", m_numVerts, isFFTSource ? " <==> FFT SOURCE" : "");
	}

	for (long i=0; i<m_waterParts; i++)
	{
		m_iDebugDraw++;

		WaterPart & wp = m_waterParts[i];
		const char * status = (wp.isBad) ? "BAD" : 
			((IsShow()) ? ((wp.isVisible) ? "visible" : "culled") : "off");

		dword color = 0xFFDFDFDF;
		if (wp.isBad)
			color = 0xFFFF0000;
		if (_stricmp(status, "visible") == 0)
			color = 0xFFAFAFFF;
		if (_stricmp(status, "culled") == 0)
			color = 0xFF4F4FFF;

		Render().Print(30.0f, m_iDebugDraw * 19.0f + 20.0f, color, "water: %s, verts(%d) %s", status, wp.numVerts, isFFTSource ? " <==> FFT SOURCE" : "");
	}
}
#endif

void Ocean2::UnlockBuffers()
{
	for (long i=0; i<m_waterParts.Len(); i++)
	{
		WaterPart & wp = m_waterParts[i];
		if (!wp.isBad && wp.vBuffer[wp.curBuffer])
		{
			if (wp.vBuffer[wp.curBuffer]->IsLocked())
				wp.vBuffer[wp.curBuffer]->Unlock();
		}
	}
}

bool Ocean2::IsAllMirrors() const
{
	if (IsBigOceanActive()) 
		return false;

	for (int i=0; i<m_waterParts.Len(); i++)
	{
		if (!m_waterParts[i].isMirror)
			return false;
	}

	return true;
}

void _cdecl Ocean2::Realize(float fDeltaTime, long level)
{
	//m_oceans->m_regenerateSphereMap = true;
	static bool deviceReseted = false;
	if (!m_oceans->sphereRT || deviceReseted || m_oceans->m_regenerateSphereMap || 
		m_oceans->m_sphereMapMission != &Mission())
	{
		deviceReseted = false;
		m_oceans->m_regenerateSphereMap = false;
		m_oceans->m_sphereMapMission = &Mission();
		// FIX-ME, 1 кадр после этого будет сжатое море, убрать комментарий чуть ниже
		GenerateSphereMap();
	}
	// FIX-ME, 1 кадр после этого будет сжатое море
	//GenerateSphereMap();

	if (Render().IsRenderReseted())
		deviceReseted = true;

	//if (IsWorking() && !IsAllMirrors()) 
	//	GenerateBumpTexture(true);

	if (api->DebugKeyState('G'))
	{
		RS_SPRITE spr[4];
		spr[0].vPos = Vector (0.0f, 1.0f, 0.1f);
		spr[1].vPos = Vector (1.0f, 1.0f, 0.1f);
		spr[2].vPos = Vector (1.0f, -0.25f, 0.1f);
		spr[3].vPos = Vector (0.0f, -0.25f, 0.1f);

		spr[0].tu = 0.0f;  spr[0].tv = 0.0f;
		spr[1].tu = 1.0f;  spr[1].tv = 0.0f;
		spr[2].tu = 1.0f;  spr[2].tv = 1.0f;
		spr[3].tu = 0.0f;  spr[3].tv = 1.0f;

		spr[0].dwColor = 0xFFFFFFFF;
		spr[1].dwColor = 0xFFFFFFFF;
		spr[2].dwColor = 0xFFFFFFFF;
		spr[3].dwColor = 0xFFFFFFFF;

		//Render().DrawSprites(m_oceans->pRefractRT->AsTexture(), spr, 1, "dbgSpritesNoblend");
		//Render().DrawSprites(m_depthTex->AsTexture(), spr, 1, "dbgSpritesNoblend");
		//Render().DrawSprites(m_oceans->pOceanRT->AsTexture(), spr, 1, "dbgSpritesNoblend");
		//Render().DrawSprites(m_oceans->pRefractRT->AsTexture(), spr, 1, "dbgSpritesNoblend");
		//Render().DrawSprites(m_oceans->fbmTex->AsTexture(), spr, 1, "dbgSpritesNoblend");
		Render().DrawSprites(m_oceans->sphereRT->AsTexture(), spr, 1, "dbgSpritesNoblend");
		//Render().DrawSprites(m_oceans->GetBumpTexture(), spr, 1, "dbgSpritesNoblend");
	}
}

bool Ocean2::IsOceanOrWatersVisible() const
{
	bool visible = false;
	visible |= (m_oceanEnable && !m_offBigOcean);// && !m_blocks.IsEmpty());
	for (int i=0; i<m_waterParts.Len() && !visible; i++)
		visible |= m_waterParts[i].isVisible;

	return visible;
}

void Ocean2::DrawRealOceanAndParts(float deltaTime, const RENDERVIEWPORT & oldVP)
{
	dword dw1;
	RDTSC_B(dw1);
	if (m_oceans->m_iSeaBuffer->IsLocked())
		m_oceans->m_iSeaBuffer->Unlock();
	if (m_oceans->m_vSeaBuffer->IsLocked())
		m_oceans->m_vSeaBuffer->Unlock();
	RDTSC_E(dw1);
	m_totalRDTSC += dw1;

	if (!m_oceanEnable && m_waterParts.IsEmpty())
		return;

	// проверяем, видим ли мы хоть одну океан/воду в данном месте
	bool visible = IsOceanOrWatersVisible();

	if (m_hasOwnReflection)
	{
		if (visible)
		{
			m_iNumReflections++;
			m_refractor.Ptr()->MakeReflection(m_oceans->pReflectionTexture2, m_oceans->pReflectionSurfaceDepth, m_seaHeight, m_reflectParticles, true);
		}

		m_oceans->pVarReflTex->SetTexture(m_oceans->pReflectionTexture2->AsTexture());
	}
	else
		m_oceans->pVarReflTex->SetTexture(m_oceans->pReflectionTexture->AsTexture());

	m_oceans->pVarBumpTex->SetTexture(m_oceans->GetBumpTexture());
	m_oceans->pVarBumpPower->SetVector4(Vector4(m_bumpPower, 0.0f, m_normalDisplacement, 0.0f));
	m_oceans->pVarBumpMulMove->SetVector4(Vector4(m_bumpScaleU, m_bumpScaleV, m_bumpPosU, m_bumpPosV));

	float scale = 90.0f / Max(0.1f, Max(m_initialBumpScaleU, m_initialBumpScaleV));
	float half = powf(2.5f, scale - 1.0f);
	m_oceans->pVarBumpDistance->SetVector4(Vector4(1.0f / (35.0f * half), 1.0f / (85.0f * half), 1.0f / (210.0f * half), 0.07f));

	float foamsin = m_foamUVScale * sinf(m_foamAngles.y);
	float foamcos = m_foamUVScale * cosf(m_foamAngles.y);
	m_oceans->pVarFoamRotate->SetVector4(Vector4(foamcos, foamsin, -foamsin, foamcos));

	m_oceans->pVarSkyColor->SetVector4(m_skyColor.v4);
	m_oceans->pVarWaterColor->SetVector4(m_waterColor.v4);
	m_oceans->pVarTranslucenceColor->SetVector4(m_translucenceColor.v4);

	m_oceans->pVarShifts->SetVector4(Vector4(m_posShift * 3.0f, m_posShift, 0.0f, 0.0f));

	m_oceans->pVarDistFade->SetFloat(1.0f / m_maxWavesDistance);
	m_oceans->pVarReflTrans->SetVector4(Vector4(m_ReflectionMin, m_ReflectionMax, m_TransparencyMin, m_TransparencyMax));

	m_oceans->pVarSphereMapTexture->SetTexture(m_oceans->sphereRT->AsTexture());
	m_oceans->pVarFoamTexture->SetTexture(m_oceans->pFoamTexture);//m_oceans->fbmTex->AsTexture());
	
	m_oceans->pVarSunBumpScale->SetVector4(Vector4(m_sunBumpScale, 1.0f, m_sunBumpScale, 1.0f));
	m_oceans->pVarSunRoadColor->SetVector4(m_sunRoadColor.v4);
	m_oceans->pVarSunRoadParams->SetVector4(m_sunRoadParams);
	m_oceans->pVarSunLight->SetVector4(m_sunLight);
	m_oceans->pVarSunLightParams->SetVector4(m_sunLightParams);
	
	m_oceans->pVarTranslucenceAngle->SetVector4(m_translucenceLight);
	m_oceans->pVarTranslucenceParams->SetVector4(m_translucenceParams);

	m_oceans->pVarRefrParams->SetVector4(m_refractionParams);
	m_oceans->pVarRefrParams2->SetVector4(m_refractionParams2);

	m_oceans->pVarFogParams->SetVector4(Vector4(m_fogMultiply, 1.0f / m_fogDistance, m_fogStart, 0.0f));
	m_oceans->pVarFogColor->SetVector4(m_fogColor.v4);

	m_oceans->pVarFrenelTex->SetTexture(m_oceans->pFrenelTexture);

	m_oceans->pVarRefrTex->SetTexture(m_oceans->pRefractRT->AsTexture());

	Vector vCamPos = m_View.GetCamPos();

	const RENDERSCREEN & si = Render().GetScreenInfo3D();

	Matrix mSavedView = Render().GetView();
	if (!isNoSwing)
		Render().SetView((Mission().GetInverseSwingMatrix() * Matrix(m_View).Inverse()).Inverse());

	boolVector32 boolParams;
	boolParams.set(0, m_sunRoadEnable);
	boolParams.set(1, m_translucenceEnable);
	boolParams.set(2, m_foamEnable);
	m_oceans->pVarBooleanParams->SetBool32(boolParams);
	
	m_foamU += deltaTime * m_foamMoveSpeed * cosf(m_foamAngles.y + m_foamMoveAngles.y) * m_oceanAnimate;
	m_foamV += deltaTime * m_foamMoveSpeed * sinf(m_foamAngles.y + m_foamMoveAngles.y) * m_oceanAnimate;

	m_oceans->pVarFoamParams1->SetVector4(Vector4(m_foamStartY, 1.0f / m_foamHeightY, m_foamPowerY, 0.0f));
	m_oceans->pVarFoamParams2->SetVector4(Vector4(m_foamUVScale, m_foamDisplacement, m_foamU, m_foamV));
	m_oceans->pVarFoamColor->SetVector4(m_foamColor.v4);

	IRenderTarget * screenRT = null;

	{
		m_oceans->pVarFakeHeight->SetVector4(Vector4(m_seaFakeHeight, m_seaHeight + m_seaFakeHeight, 0.0f));

		// рисуем полный океан
		if (m_oceanEnable && !m_offBigOcean && (!m_blocks.IsEmpty() || !m_flatBlocks.IsEmpty()))
		{
			Render().SetIndices(m_oceans->m_iSeaBuffer);
			Render().SetWorld(Matrix());
			Render().SetStreamSource(0, m_oceans->m_vSeaBuffer);
#ifndef _XBOX
			Render().DrawIndexedPrimitive(m_oceans->m_oceanShaderID, PT_TRIANGLELIST, 0, m_VStart, 0, m_TStart);
#else
			int numDraw = m_TStart;
			int curIndex = 0;
			const int maxDraw = 21665;
			while (numDraw)
			{
				long curDraw = Min(maxDraw, numDraw);
				Render().DrawIndexedPrimitive(m_oceans->m_oceanShaderID, PT_TRIANGLELIST, 0, m_VStart, curIndex, curDraw);
				curIndex += curDraw * 3;
				numDraw -= curDraw;
			}
#endif

			//api->Trace("total = %d", m_totalRDTSC);
			//api->Trace("nums = %d, tick per = %.1f", m_totalWaveXZVerts, float(m_totalWaveXZRDTSC) / float(m_totalWaveXZVerts));
			//Render().Print(10.0f, 40.0f, 0xFFFFFFFF, "m_VStart = %d", m_VStart);
		}

		m_oceans->pVarDistFade->SetFloat(0.0f);
		m_oceans->pVarFakeHeight->SetVector4(Vector4(m_seaFakeHeight, 0.0f, 0.0f));

		// рисуем воды
		for (long i=0; i<m_waterParts; i++)
		{
			WaterPart & wp = m_waterParts[i];

			if (wp.isBad || (wp.isMirror && !m_oceanEnableRealWaves))
			{
#ifndef STOP_DEBUG
				Render().SetWorld(Matrix().SetIdentity());
				Render().SetIndices(null);
				Render().SetStreamSource(0, wp.vBufferFlat);
				Render().DrawPrimitive(m_oceans->m_badFlatWaterShaderID, PT_TRIANGLELIST, 0, (wp.numParts - 1) * 2);
#endif
				continue;
			}

			if (!wp.isVisible) 
				continue;

			m_oceans->pVarBumpPower->SetVector4(Vector4(m_bumpPower, wp.bumpFade, m_normalDisplacement, 0.0f));

			Render().SetWorld(Matrix().SetIdentity());
			Render().SetIndices(wp.iBuffer);
			Render().SetStreamSource(0, wp.vBuffer[wp.curBuffer]);
			Render().DrawIndexedPrimitive((wp.isMirror) ? m_oceans->m_mirrorShaderID : m_oceans->m_waterShaderID, 
				PT_TRIANGLELIST, 0, wp.numVerts, 0, wp.numTrgs);

#ifdef _XBOX
			wp.curBuffer ^= 1;
#endif
		}
	}

	Render().SetView(mSavedView);
}

void Ocean2::DrawFlatOceanAndParts(bool bNoZ, bool alphaRefractPass, bool zPrePass, bool zPrePassSetAlpha)
{
	RENDERVIEWPORT oldVP = Render().GetViewport();
	Matrix mView = Render().GetView();
	Matrix mProj = Render().GetProjection();
	Vector vCamPos = mView.GetCamPos();

	if (!isNoSwing)
		Render().SetView((Mission().GetInverseSwingMatrix() * Matrix(m_View).Inverse()).Inverse());

	ShaderId * technique = null;
	
	if (alphaRefractPass)
		technique = &m_oceans->m_alphaRefractID;
	else if (zPrePass)
	{
		if (zPrePassSetAlpha)
			technique = &m_oceans->m_zPrePassSetAlphaID;
		else
			technique = &m_oceans->m_zPrePassID;
	}
	else
		technique = (bNoZ) ? &m_oceans->m_flatNoZID : &m_oceans->m_oceanID;

	// рисуем плоскость океана
	m_oceans->pVarUV05->SetVector4(Vector4(0.5f + 0.5f / float(oldVP.Width), 0.5f + 0.5f / float(oldVP.Height), 0.0f, 0.0f));
	m_oceans->pVarFlatMapping->SetVector4(Vector4(float(oldVP.X), float(oldVP.Y), 1.0f / float(oldVP.Width), 1.0f / float(oldVP.Height)));
	m_oceans->pVarOceanRT->SetTexture(m_oceans->pOceanRT->AsTexture());

	if (m_oceanEnable && !m_offBigOcean)
	{
		Vector connectedPos = m_connectMtx.GetCamPos();
		m_oceans->pVarCamPos->SetVector4(Vector4(vCamPos.x + connectedPos.x, vCamPos.y, vCamPos.z + connectedPos.z, 0.65f));

		Matrix mWorld = Matrix().BuildPosition(0.0f, m_seaHeight + ((bNoZ) ? m_seaFakeHeight : 0.0f), 0.0f);

		Render().SetWorld(mWorld);
		Render().SetIndices(m_oceans->m_indexes2D);
		Render().SetStreamSource(0, m_oceans->m_verts2D);
		Render().DrawIndexedPrimitive(*technique, PT_TRIANGLELIST, 0, Sqr(10), 0, Sqr(10 - 1) * 2);
	}

	if (!zPrePass)
	{
		m_oceans->pVarFakeHeight->SetVector4(0.0f);
		m_oceans->pVarCamPos->SetVector4(Vector4(0.0f, 0.0f, 0.0f, 1.0f));
		// TO-DO: соединить все части воедино
		for (long i=0; i<m_waterParts; i++)
		{
			WaterPart & wp = m_waterParts[i];

			if (wp.isBad || (wp.isMirror && !m_oceanEnableRealWaves))
			{
	#ifndef STOP_DEBUG
				Render().SetWorld(Matrix().SetIdentity());
				Render().SetIndices(null);
				Render().SetStreamSource(0, wp.vBufferFlat);
				Render().DrawPrimitive(m_oceans->m_badFlatWaterShaderID, PT_TRIANGLELIST, 0, (wp.numParts - 1) * 2);
	#endif
				continue;
			}

			if (!wp.isVisible || wp.isMirror || m_oceanEnableRealWaves) 
				continue;

			Render().SetWorld(Matrix().SetIdentity());
			Render().SetIndices(null);
			Render().SetStreamSource(0, wp.vBufferFlat);
			Render().DrawPrimitive(*technique, PT_TRIANGLELIST, 0, (wp.numParts - 1) * 2);
		}
	}

	Render().SetView(mView);
}

void Ocean2::Command(const char * id, dword numParams, const char * * params)
{
	if (string::IsEqual(id, "postdraw"))
	{
		DrawFlatOceanAndParts();
	} else if (string::IsEqual(id, "regensky"))
	{
		m_oceans->m_regenerateSphereMap = true;
	}
}

inline float Ocean2::CalcLod(const float & x, const float & y, const float & z)
{
	return Sqr(x - m_camPos.x) + /*Sqr((y - m_camPos.y)) + */Sqr(z - m_camPos.z);
}

void Ocean2::CalculateLOD(const Vector & v1, const Vector & v2, int & iMaxLOD, int & iMinLOD)
{
	float fCur, fMax, fMin;

	fCur = CalcLod(v1.x, vSeaCenterPos.y, v1.z); fMax = fCur;
	fCur = CalcLod(v2.x, vSeaCenterPos.y, v1.z); if (fCur > fMax) fMax = fCur;
	fCur = CalcLod(v1.x, vSeaCenterPos.y, v2.z); if (fCur > fMax) fMax = fCur;
	fCur = CalcLod(v2.x, vSeaCenterPos.y, v2.z); if (fCur > fMax) fMax = fCur;

	if (m_camPos.x < v1.x)
	{
		if (m_camPos.z < v1.z)
			fMin = CalcLod(v1.x, vSeaCenterPos.y, v1.z);
		else
		if (m_camPos.z > v2.z)
			fMin = CalcLod(v1.x, vSeaCenterPos.y, v2.z);
		else
			fMin = CalcLod(v1.x, vSeaCenterPos.y, m_camPos.z);
	}
	else
	if (m_camPos.x > v2.x)
	{
		if (m_camPos.z < v1.z)
			fMin = CalcLod(v2.x, vSeaCenterPos.y, v1.z);
		else
		if (m_camPos.z > v2.z)
			fMin = CalcLod(v2.x, vSeaCenterPos.y, v2.z);
		else
			fMin = CalcLod(v2.x, vSeaCenterPos.y, m_camPos.z);
	}
	else
	{
		if (m_camPos.z < v1.z)
			fMin = CalcLod(m_camPos.x, vSeaCenterPos.y, v1.z);
		else
		if (m_camPos.z > v2.z)
			fMin = CalcLod(m_camPos.x, vSeaCenterPos.y, v2.z);
		else
			fMin = CalcLod(m_camPos.x, vSeaCenterPos.y, m_camPos.z);
	}

	iMaxLOD = int(0.5f * logf(fLodScale * fMax) / logf(2)); 
	if (iMaxLOD < 1) iMaxLOD = 1;
	iMinLOD = int(0.5f * logf(fLodScale * fMin) / logf(2)); 
	if (iMinLOD < 1) iMinLOD = 1;
}

void Ocean2::AddBlock(int iTX, int iTY, int iSize, int iLOD)
{
	SeaBlock & block = m_blocks[m_blocks.Add()];

	block.iTX = iTX;		
	block.iTY = iTY;
	block.iSize = iSize;
	block.iLOD = Max(4, iLOD);

	block.bInProgress = false;
	block.bDone = false;

	block.iSize0 = iSize >> block.iLOD;
}

bool Ocean2::isVisibleBBox(const Vector & vSeaCenterPos, const Vector & v1, const Vector & v2)
{
	Vector center = (v1 + v2) * 0.5f;
	float radius = (center - v2).GetLength2();

	//const Plane * frustum = Render().GetFrustum();
	//dword numPlanes =  Render().GetNumFrustumPlanes();

	for (dword p=0; p<m_numFrustumPlanes; p++)
	{
		float dist = m_frustumPlanes[p].Dist(center);
		if (dist * fabsf(dist) < -radius) return false;
	}

	return true;
}

void Ocean2::BuildTree(int iTX, int iTY, int iLev)
{
	int	iMaxLOD, iMinLOD;
	int	iSize = int(dwMaxDim >> iLev);
	float fGSize = fGridStep * iSize;

	Vector v1 = vSeaCenterPos + Vector(iTX * fGSize, 0.0f, iTY * fGSize);
	Vector v2 = v1 + Vector(fGSize, 35.0f, fGSize);

	if (!isVisibleBBox(vSeaCenterPos, v1, v2)) return;

	CalculateLOD(v1, v2, iMaxLOD, iMinLOD);

	if (iSize <= dwMinDim || iMaxLOD - iMinLOD <= 1)
	{
		AddBlock(iTX, iTY, iSize, iMinLOD);
		return;
	}

	iTX *= 2; 
	iTY *= 2; 
	iLev++;

	BuildTree(iTX    , iTY    , iLev);
	BuildTree(iTX + 1, iTY    , iLev);
	BuildTree(iTX    , iTY + 1, iLev);
	BuildTree(iTX + 1, iTY + 1, iLev);
}

MOP_BEGINLISTG(Ocean2, "Ocean2", '1.00', 101, "Weather")
	MOP_GROUPBEG("Ocean params")
		MOP_BOOL("Enable ocean", false)
		MOP_BOOL("Stop animation", false)
		MOP_BOOL("Enable real waves", false)
		MOP_STRINGC("Connect to", "", "Connect Ocean3 to object")
		MOP_FLOATEX("Height", 0.0f, -100.0f, 100.0f)
		MOP_BOOLC("Main reflection Height", false, "Если true, то данный height будет использоваться как главный для общего рефлекшена")
		MOP_BOOLC("Main FFT source", false, "Если true, то данные FFT параметры будут главными, пока этот объект активен")
		MOP_BOOLC("Own reflection", false, "Если true, то для этого объекта будет свой отдельный рефлекшн - ЭТО ОЧЕНЬ ДОРОГО!!!")
		MOP_FLOATEXC("Fake Offset", 0.0f, -100.0f, 100.0f, "Задает смещение реального моря относительно Height, для больших волн")
		MOP_COLOR("Sky color", Color(1.0f, 1.0f, 1.0f))
		MOP_COLOR("Water color", Color(0.1f, 0.2f, 0.2f)) 
		MOP_BOOLC("No swing", true, "No swing ocean in swing machine")
		MOP_GROUPBEG("Wave params")
			MOP_GROUPBEG("FFT Wave params")
				MOP_FLOATC("wave_len", 256.0f, "Длина волн")
				MOP_FLOATC("wave_amp", 300.0f, "Высота волны, магический параметр: зависит от wave_len")
				MOP_FLOATC("wind_pwr", 20.0f, "Скорость ветра, разная форма и амплитуда волн")
				MOP_ANGLESC("wave_dir", 0.0f, "Угол в котором распространяются волны, зависит от wave_form")
				MOP_FLOATC("wave_form", 0.1f, "Форма волны, чем больше число, тем вытянутее волны")
				MOP_FLOATC("wave_speed", 1.25f, "Скорость анимации воды")
			MOP_GROUPEND()
			MOP_FLOATEXC("Waves Amplitude", 0.0f, 0.0f, 10000.0f, "Скалирование волн, на океан не действует! Только для кусков воды.")
			MOP_FLOATEXC("Waves Scale", 1.0f, 0.0f, 10000.0f, "Расстягивание волн, для океана лучше оставлять 1.0")
			MOP_FLOATEX("Waves Distance", 2200.0f, 1.0f, 3000.0f)
			MOP_POSITIONC("Wave Move", Vector(3.0f, 0.0f, 0.0f), "Направление и скорость движения волн")
			MOP_FLOATC("Choppy waves", 12.0f, "Делает волны более острыми")
		MOP_GROUPEND()
		MOP_GROUPBEG("Sun road params")
			MOP_BOOL("Enable sunroad", false)
			MOP_ANGLESC("Sunroad angle", Vector(6.0f, 4.0f, 0.0f), "Угол где Солнце, Y - азимут и X - высота")
			MOP_COLOR("Sun road color", Color(1.0f, 1.0f, 1.0f)) 
			MOP_FLOATEXC("Scale", 1.0f, 0.0f, 1000.0f, "Скалирование бампа, больше - рандомнее")
			MOP_FLOATEXC("Color power", 100.0f, 0.001f, 30000.0f, "Усиление цвета")
			MOP_FLOATEXC("Sunroad power", 130.0f, 0.0001f, 10000.0f, "Ширина солнечной дорожки")
		MOP_GROUPEND()
		MOP_GROUPBEG("Lighting params")
			MOP_ANGLESC("Sun angle", Vector(4.7f, 3.1415f, 0.0f), "Угол где Солнце, Y - азимут и X - высота")
			MOP_FLOATEXC("Multiply", 1.1f, 0.0f, 10.0f, "Коэффициент освещения")
			MOP_FLOATEXC("Ambient", 0.25f, 0.0f, 10.0f, "Постоянная часть освещения")
		MOP_GROUPEND()
		MOP_GROUPBEG("Translucence params")
			MOP_BOOL("Enable translucence", true)
			MOP_FLOATEXC("Translucence Angle", 0.6f, 0.0f, 20.0f, "Угол просвечивания")
			MOP_FLOATEXC("Translucence Power", 6.0f, 0.0f, 1000.0f, "Изменение величины области просвечивания")
			MOP_FLOATEXC("Translucence Multiply", 10.0f, 0.0f, 1000.0f, "Умножитель просвечивания")
			MOP_COLOR("Translucence color", Color(0.0f, 0.1f, 0.0f)) 
		MOP_GROUPEND()
		MOP_GROUPBEG("Foam params")
			MOP_BOOL("Enable foam", false)
			MOP_COLORC("Foam Color", Color(1.0f, 1.0f, 1.0f), "Цвет пены")
			MOP_FLOATEXC("Foam color power", 1.0f, -1000.0f, 1000.0f, "Усиление цвета пены")
			MOP_ANGLESC("Foam angle", Vector(0.0f), "Угол поворота пены, используется только Y")
			MOP_FLOATEXC("Foam start Y", 0.0f, -1000.0f, 1000.0f, "Откуда начинается пена на волнах")
			MOP_FLOATEXC("Foam height Y", 5.0f, 0.001f, 1000.0f, "Высота (относительно Foam start Y) на которой пена становится максимальной")
			MOP_FLOATEXC("Foam power Y", 1.0f, 0.0f, 1000.0f, "Как быстро изменяется сила пены от высоты")
			MOP_FLOATEXC("Foam UV scale", 0.02f, 0.0f, 1000.0f, "Скалирование текстуры пены")
			MOP_FLOATEXC("Texture displacement", 0.01f, 0.0f, 1000.0f, "Сила смещения текстуры пены по бампу")
			MOP_ANGLESC("Foam move angle", Vector(0.0f), "Направлние движения пены, относительно Foam angle, используется только Y")
			MOP_FLOATC("Foam move speed", 0.0f, "Скорость движения пены")
		MOP_GROUPEND()
		MOP_GROUPBEG("Fog params")
			MOP_COLORC("Fog Color", Color(1.0f, 1.0f, 1.0f), "Цвет тумана")
			MOP_FLOATEXC("Fog multiply", 0.0f, 0.0f, 1000.0f, "Сила тумана")
			MOP_FLOATEXC("Fog start", 0.0f, 0.0f, 10000.0f, "Расстояние с которого начинается туман")
			MOP_FLOATEXC("Fog distance", 1.0f, 1.0f, 10000.0f, "Расстояние до тумана")
		MOP_GROUPEND()
		MOP_GROUPBEG("Refl/Refr params")
			MOP_BOOLC("Reflect Particles", true, "Рисовать партиклы в отражении или нет")
			MOP_FLOATEXC("Reflection Minimum", 0.65f, 0.0f, 1.0f, "Минимальная яркость с которой рефлекшн становится мощнее");
			MOP_FLOATEXC("Reflection Add", 2.00f, 0.0f, 1000.0f, "Делает светлым объектам мощнее рефлекшн");
			MOP_FLOATEXC("Reflection Power", 1.00f, -1000.0f, 1000.0f, "Умножает цвет рефлекшена на это значение");
			MOP_FLOATEXC("Reflection Env power", 100.00f, 0.001f, 1000.0f, "Сила искажения отражения от объектов(небо не входит)");
			MOP_FLOATEXC("Reflection Env/Sky coeff", 0.65f, 0.0f, 1.0f, "Коэффициент блендинга между рефлекшном объектов и неба")
			MOP_FLOATC("Reflection min", 0.1f, "Сила отражения под ногами");
			MOP_FLOATC("Reflection max", 0.75f, "Сила отражения вдали");
			MOP_FLOATEX("Refraction power", 100.00f, 0.001f, 1000.0f)
			MOP_FLOATC("Transparency min", 0.3f, "Прозрачность под ногами");
			MOP_FLOATC("Transparency max", 0.0f, "Прозрачность вдали");
		MOP_GROUPEND()
		MOP_GROUPBEG("Bump params")
			MOP_GROUPBEG("FFT Bump params")
				MOP_FLOATC("wave_len2", 256.0f, "Длина волны для бампа")
				MOP_FLOATC("wave_amp2", 4000.4f, "Амплитуда бампа, магический параметр: зависит от wave_len")
				MOP_FLOATC("wind_pwr2", 27.0f, "Скорость ветра, разная форма и амплитуда бампа")
				MOP_ANGLESC("wave_dir2", 0.0f, "Угол в котором распространяются волны, зависит от wave_form2")
				MOP_FLOATC("wave_form2", 0.0f, "Форма бампа, чем больше число, тем вытянутее бамп")
				MOP_FLOATC("wave_speed2", 2.0f, "Скорость анимации бампа")
			MOP_GROUPEND()
			MOP_FLOATEX("Bump angle", 60.00f, 0.001f, 360.0f)
			MOP_FLOAT("Bump scale U", 12.0f)
			MOP_FLOAT("Bump scale V", 12.0f)
			MOP_FLOATEX("Bump power | v3 |", 0.45f, 0.0f, 100.0f)
			MOP_FLOAT("Bump move speed", 100.0f)
		MOP_GROUPEND()
	MOP_GROUPEND()
	MOP_ARRAYBEG("Waters", 0, 1000)
		MOP_BOOL("Mirror Enable", false)
		MOP_FLOATEX("Detail", 1.0f, 0.5f, 100.0f)
		MOP_FLOATEX("Bump Fade | v3 |", 1.0f, -100.0f, 100.0f)
		MOP_FLOATEX("Left Distance", 4.0f, 0.0f, 10000.0f)
		MOP_FLOATEX("Left Power", 1.0f, 0.0f, 1.0f)
		MOP_FLOATEX("Right Distance", 4.0f, 0.0f, 10000.0f)
		MOP_FLOATEX("Right Power", 1.0f, 0.0f, 1.0f)
		MOP_FLOATEX("Up Distance", 4.0f, 0.0f, 10000.0f)
		MOP_FLOATEX("Up Power", 0.0f, 0.0f, 1.0f)
		MOP_FLOATEX("Bottom Distance", 4.0f, 0.0f, 10000.0f)
		MOP_FLOATEX("Bottom Power", 0.0f, 0.0f, 1.0f)
		MOP_BOOL("Transparency Enable", false)
		MOP_ARRAYBEG("Water Parts", 1, 1000)
			MOP_POSITION("left", Vector(0.0f, 0.0f, 0.0f))
			MOP_POSITION("right", Vector(2.0f, 0.0f, 0.0f))
		MOP_ARRAYEND
	MOP_ARRAYEND
	MOP_BOOL("Visible", true)
MOP_ENDLIST(Ocean2)