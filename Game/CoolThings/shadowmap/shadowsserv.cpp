#include "shadowsserv.h"
#include "..\..\..\Common_h\FileService.h"
#include "..\..\..\Common_h\LocStrings.h"

CREATE_SERVICE(ShadowsService, 55)

ShadowsService::ShadowsService()
{
	pRS=NULL;

	dwPSSMShadowMapRes = 1024;

	dwPostProcessTexWidth = 512;
	dwPostProcessTexHeight = 512;
	
	bPostProcessEnabled=true;
	bClearedPrevFrame = true;
	bAlreadyCleared = false;

	bOff = false;

	bIsAnyActiveShadowMap = false;
	pptClearStatus = ppt_need2clear;

	pZTexture = null;
	pDepthSurface = null;
	pPostProcessRT = null;
	pDepthSurfaceRT = null;
	pPostProcessRT2 = null;
	pPostProcessTex = null;
	pIBlurTexture = null;
	pIBlurTexSize = null;
	m_linearZRT = null;
}

ShadowsService::~ShadowsService()
{
	if (pZTexture) pZTexture->Release();
	if (m_linearZRT) m_linearZRT->Release();
	if (pDepthSurface) pDepthSurface->Release();
	if (pPostProcessRT) pPostProcessRT->Release();	
	if (pDepthSurfaceRT) pDepthSurfaceRT->Release();
	if (pPostProcessRT2) pPostProcessRT2->Release();
	if (pPostProcessTex) pPostProcessTex->Release();
	pIBlurTexture = NULL;
	pIBlurTexSize = NULL;
}

bool ShadowsService::Init()
{
	pDepthSurface = NULL;
	pPostProcessTex = NULL;
	pPostProcessRT = NULL;

	IFileService * pFS = (IFileService*)api->GetService("FileService");

	IIniFile * pEngineIni = pFS->SystemIni();
		
	int iQuality = 1;

	int iPostTexSize = 512;

	if (pEngineIni)
	{	
		bOff = pEngineIni->GetLong("Shadows", "Off", 0) != 0;
		iPostTexSize = pEngineIni->GetLong("Shadows", "PostTexSz", 512);
		iQuality = pEngineIni->GetLong("Shadows", "Quality", 1);
		//bPostProcessEnabled = pEngineIni->GetLong("Shadows", "Blur", 1) > 0;
		bPostProcessEnabled = true;
	}	

#ifndef STOP_DEBUG
	api->Trace("\nShadow service parameters:");
	api->Trace("---------------------------------------------------------------------------------");
	api->Trace("Off = %d (1 - disable shadow maps)", (bOff) ? 1 : 0);
	api->Trace("PostTexSz = %d (shadow receive texture size)", iPostTexSize);
	api->Trace("Quality = %d (shadow quality, 0 = 2048, 1 = 1536, 2 = 1280, 3 = 1024, 4 = 896, 5 = 768, 6 = 512)", iQuality);
	//api->Trace("Blur = %d (blur receive texture, 0 = disable, 1 = enable)", bPostProcessEnabled);
	api->Trace("---------------------------------------------------------------------------------\n");
#endif

	pRS = (IRender*)api->GetService("DX9Render");

#ifdef _XBOX
	dwPSSMShadowMapRes = 1024 + 128;
	iPostTexSize = 512;
#else
	if (pRS->GetScreenInfo3D().dwWidth > 2048)
		iQuality = Min(iQuality, -1);

	switch (iQuality)
	{
		case -3:
			dwPSSMShadowMapRes = 8192;
			iPostTexSize = Max(iPostTexSize, 2048);
		break;
		case -2:
			dwPSSMShadowMapRes = 4096;
			iPostTexSize = Max(iPostTexSize, 1024 + 512);
		break;
		case -1:
			dwPSSMShadowMapRes = 2048 + 1024;
			iPostTexSize = Max(iPostTexSize, 1024);
		break;
		case 0:
			dwPSSMShadowMapRes = 2048;
		break;
		case 1:
			dwPSSMShadowMapRes = 1024 + 512;
		break;
		case 2:
			dwPSSMShadowMapRes = 1024;
			iPostTexSize = Min(iPostTexSize, 512);
		break;
	}
#endif

	if (iPostTexSize>=128)
	{	
		dwPostProcessTexWidth = iPostTexSize;
		dwPostProcessTexHeight = iPostTexSize;
	}

	if (!bOff)
	{
#ifdef _XBOX
		m_linearZRT = pRS->CreateRenderTarget(dwPostProcessTexWidth, dwPostProcessTexHeight, _FL_, FMT_G16R16F);
#else
		if (pRS->IsCreateRenderTarget(FMT_G16R16F))
			m_linearZRT = pRS->CreateRenderTarget(dwPostProcessTexWidth, dwPostProcessTexHeight, _FL_, FMT_G16R16F);
		else if (pRS->IsCreateRenderTarget(FMT_A16B16G16R16F))
			m_linearZRT = pRS->CreateRenderTarget(dwPostProcessTexWidth, dwPostProcessTexHeight, _FL_, FMT_A16B16G16R16F);
		else if (pRS->IsCreateRenderTarget(FMT_A32B32G32R32F))
			m_linearZRT = pRS->CreateRenderTarget(dwPostProcessTexWidth, dwPostProcessTexHeight, _FL_, FMT_A32B32G32R32F);
		else if (pRS->IsCreateRenderTarget(FMT_R32F))
			m_linearZRT = pRS->CreateRenderTarget(dwPostProcessTexWidth, dwPostProcessTexHeight, _FL_, FMT_R32F);

		if (!m_linearZRT)
		{
			api->Trace("Error: Shadowservice can't create linear Z render target");
			bOff = true;
		}
#endif
		if (!bOff)
		{
			pPostProcessRT = pRS->CreateRenderTarget(dwPostProcessTexWidth, dwPostProcessTexHeight, _FL_, FMT_X8R8G8B8);
			if (iPostTexSize == 512)
				pDepthSurfaceRT = pRS->CreateTempDepthStencil(TRS_512, _FL_);
			else
				pDepthSurfaceRT = pRS->CreateDepthStencil(dwPostProcessTexWidth, dwPostProcessTexWidth, _FL_);	
// На XBox - не нужен второй пост процессный RT
#ifndef _XBOX
			if (bPostProcessEnabled)
				pPostProcessRT2 = pRS->CreateRenderTarget(dwPostProcessTexWidth, dwPostProcessTexHeight, _FL_, FMT_X8R8G8B8);
#endif
			ClearPostProcessTexture();

			pIBlurTexture = pRS->GetTechniqueGlobalVariable("shdsprite_Texture", _FL_);
			pIBlurTexSize = pRS->GetTechniqueGlobalVariable("shdsprite_TexSize", _FL_);

			bool result = CreateRenderTargets(dwPSSMShadowMapRes);
			if (!result && dwPSSMShadowMapRes > 2048)
				result = CreateRenderTargets(2048);

			if (!result || !pPostProcessRT || !pDepthSurfaceRT)
			{
				Error(1000500, "PSSM: Can't create shadow maps or depth buffer, maybe not enough video memory?");
				return false;
			}

			pRS->GetShaderId("shdsrv_Blur", m_shaderBlur);
		}
	}

	api->SetStartFrameLevel(this, Core_DefaultExecuteLevel + 1);
	api->SetEndFrameLevel(this, Core_DefaultExecuteLevel + 0x1000 - 1);

	return true;
}

bool ShadowsService::IsActiveShadowMap()
{ 
	return bIsAnyActiveShadowMap && !bOff;
}

bool ShadowsService::CreateRenderTargets(dword resolution)
{
	if (bOff) return true;

	dwPSSMShadowMapRes = resolution;

#ifdef _XBOX
	pZTexture = pRS->CreateDepthStencil(dwPSSMShadowMapRes, dwPSSMShadowMapRes, _FL_, FMT_D24X8);

	api->Trace("PSSM: Selected format - FMT_D24S8");
#else
	RENDERFORMAT fmt = FMT_R32F;
	api->Trace("PSSM: Selected format - FMT_R32F");

	pZTexture = pRS->CreateRenderTarget(dwPSSMShadowMapRes, dwPSSMShadowMapRes, _FL_, fmt);

	pDepthSurface = pRS->CreateDepthStencil(dwPSSMShadowMapRes, dwPSSMShadowMapRes, _FL_);	

	return pZTexture && pDepthSurface;
#endif
	return true;
}

void ShadowsService::Error(long id, const char * errorEnglish)
{
	ILocStrings * locStrings = (ILocStrings*)api->GetService("LocStrings");
	ICoreStorageString * storage = api->Storage().GetItemString("system.error", _FL_);

	const char * errorString = (locStrings) ? locStrings->GetString(id) : null;
	storage->Set((errorString) ? errorString : errorEnglish);
	storage->Release();
}

IS_XBOX(IRenderTargetDepth *, IRenderTarget *)	ShadowsService::GetShadowMap()
{
	return pZTexture;
}

IRenderTarget* ShadowsService::GetPostProcessTexture()
{
	return pPostProcessRT;
}

#ifndef _XBOX
IRenderTargetDepth* ShadowsService::GetZStencil()
{
	return pDepthSurface;
}
#endif

void ShadowsService::SetShadowMapViewPort()
{
	RENDERVIEWPORT D3DViewPort;
	D3DViewPort.X = 0;
	D3DViewPort.Y = 0;
	D3DViewPort.Width = dwPSSMShadowMapRes;
	D3DViewPort.Height = dwPSSMShadowMapRes;
	D3DViewPort.MinZ = 0.0f;
	D3DViewPort.MaxZ = 1.0f;
	pRS->SetViewport(D3DViewPort);
}

void ShadowsService::SetPostProcessTexViewPort()
{
	RENDERVIEWPORT D3DViewPort;
	D3DViewPort.X = 0;
	D3DViewPort.Y = 0;

	D3DViewPort.Width = dwPostProcessTexWidth;
	D3DViewPort.Height = dwPostProcessTexHeight;
	D3DViewPort.MinZ = 0.0f;
	D3DViewPort.MaxZ = 1.0f;
	pRS->SetViewport(D3DViewPort);
}

void ShadowsService::SetPostProcessAsRT(bool bRestoreContext)
{
	pRS->SetRenderTarget((bRestoreContext) ? RTO_RESTORE_CONTEXT : RTO_DONTOCH_CONTEXT, pPostProcessRT, pDepthSurfaceRT);
	SetPostProcessTexViewPort();
}

//#ifdef _XBOX
void ShadowsService::SetLinearZAsRT()
{
	pRS->SetRenderTarget(RTO_DONTOCH_CONTEXT, m_linearZRT, pDepthSurfaceRT);
	SetPostProcessTexViewPort();
}
//#endif

dword ShadowsService::GetShadowMapRes()
{
	return dwPSSMShadowMapRes;
}

dword ShadowsService::GetPostProcessTexWidth()
{
	return dwPostProcessTexWidth;
}

dword ShadowsService::GetPostProcessTexHeight()
{
	return dwPostProcessTexHeight;
}

void ShadowsService::Blur(dword dwLevel)
{
	//if (api->DebugKeyState('8')) return;
//#ifndef _XBOX
#ifndef _XBOX
	if (dwLevel && pPostProcessRT2 && bPostProcessEnabled)
#else
	if (dwLevel && bPostProcessEnabled)
#endif
	{
		RS_SPRITE spr[4];

		float fPosX = -1.0f;
		float fPosY = -1.0f;

		spr[0].vPos = Vector (fPosX,		fPosY + 2.0f,	0.0f);
		spr[1].vPos = Vector (fPosX + 2.0f,	fPosY + 2.0f,	0.0f);
		spr[2].vPos = Vector (fPosX + 2.0f,	fPosY,			0.0f);
		spr[3].vPos = Vector (fPosX,		fPosY,			0.0f);

		spr[0].tv = 0.0f;	spr[0].tu = 0.0f; 
		spr[1].tv = 0.0f;	spr[1].tu = 1.0f; 
		spr[2].tv = 1.0f;	spr[2].tu = 1.0f;  
		spr[3].tv = 1.0f;	spr[3].tu = 0.0f;  	

		pIBlurTexture->SetTexture(pPostProcessRT->AsTexture());
		pIBlurTexSize->SetFloat(1.0f / float(dwPostProcessTexWidth));

#ifndef _XBOX
		pRS->SetRenderTarget(RTO_DONTOCH_CONTEXT, pPostProcessRT2, null);
#else
		pRS->SetRenderTarget(RTO_DONTOCH_CONTEXT, pPostProcessRT, null);
#endif
		SetPostProcessTexViewPort();
		pRS->BeginScene();

		//char * pTechStr[3] = {"shdsrv_blur1", "shdsrv_blur2", "shdsrv_blur3"};

		pRS->DrawFullScreenQuad((float)dwPostProcessTexWidth, (float)dwPostProcessTexHeight, m_shaderBlur);

		//pRS->DrawSprites(pPostProcessRT->AsTexture(), spr, 1, "shdsrv_Blur");//pTechStr[MinMax(dword(0), dword(2), dwLevel - 1)]);

		pRS->EndScene();

#ifndef _XBOX
		// подмениваем RT на RT2
		Swap(pPostProcessRT, pPostProcessRT2);
#endif
	}
//#endif
}

void ShadowsService::ClearPostProcessTexture()
{
	//Joker: На XBOX приводит к падению, хз почему - но так как код этот выкинется разбиратся не стал
}

void ShadowsService::StartFrame(float dltTime)
{
	if (pRS->IsLoadingScreenEnabled())
		return;
	// потом надо сделать метод clear тут для Clear'a основного рендертаргета

	if (pRS->IsRenderReseted())
		pptClearStatus = ppt_need2clear;

	if (pptClearStatus == ppt_need2clear && !bOff)
	{
		pptClearStatus = ppt_cleared;

		pRS->PushRenderTarget();
		pRS->EndScene();

		SetPostProcessAsRT(false);
		pRS->BeginScene();
		pRS->Clear(0, null, CLEAR_TARGET | CLEAR_ZBUFFER, 0xFFFFFFFF, 1.0f, 0x0);
		pRS->EndScene();

		pRS->PopRenderTarget(RTO_RESTORE_CONTEXT);
		pRS->BeginScene();
	}

	if (!bClearedPrevFrame)
	{
		pRS->Clear(0, null, CLEAR_TARGET | CLEAR_ZBUFFER | CLEAR_STENCIL, 0x0, 1.0, 0);
	}

	if (pRS->IsRenderReseted())
		pptClearStatus = ppt_dirty;

	bClearedPrevFrame = false;
	bIsAnyActiveShadowMap = false;
}

void ShadowsService::ClearScreen()
{
	if (bAlreadyCleared) return;

	bAlreadyCleared = true;
	bClearedPrevFrame = true;
	pRS->Clear(0, null, CLEAR_TARGET | CLEAR_ZBUFFER | CLEAR_STENCIL, 0x0, 1.0, 0);
}

void ShadowsService::EndFrame(float dltTime)
{
	// если не было ни одной активной шадоумапы за кадр, то очистим шадоу текстуру
	if (!bIsAnyActiveShadowMap)
	{
		if (pptClearStatus == ppt_dirty)
			pptClearStatus = ppt_need2clear;
	}
	else
		pptClearStatus = ppt_dirty;

	bAlreadyCleared = false;
}
