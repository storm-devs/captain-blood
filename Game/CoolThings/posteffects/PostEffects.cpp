#include "PostEffects.h"

CREATE_SERVICE_NAMED("PostEffects", PostEffects, 130)

PostEffects::PostEffects()
{
	m_uberFilter = null;
	m_storageBrightness = null;
}

PostEffects::~PostEffects()
{
	RELEASE(m_storageBrightness);
	DELETE(m_uberFilter);

#ifdef GAME_RUSSIAN
	RELEASE(gameTicks);
#endif
}

bool PostEffects::Init()
{
	m_render = (IRender*)api->GetService("DX9Render");
	m_files = (IFileService*)api->GetService("FileService");

	m_uberFilter = NEW UberFilter(m_render, m_files);

	m_render->AddRenderFilter(m_uberFilter, 2.0f);

	api->SetStartFrameLevel(this, Core_DefaultExecuteLevel);
	api->SetEndFrameLevel(this, Core_DefaultExecuteLevel + 0x1000 - 1);

	/*m_render->GetShaderId("PE_GammaCorrection", m_gammaShader);

	m_gammaCoeff = m_render->GetTechniqueGlobalVariable("peGammaCoeff", _FL_);
	m_rawImageTexture = m_render->GetTechniqueGlobalVariable("peOriginalScreenTexture", _FL_);
	m_storageBrightness = api->Storage().GetItemFloat("Profile.Custom.Brightness", _FL_);
	if (!m_storageBrightness->IsValidate())
		m_storageBrightness->Set(0.5f);*/

	renderScreen = m_render->GetScreenInfo3D();

	IVariable * peSS = m_render->GetTechniqueGlobalVariable("peScreenSize", _FL_);
	Vector4 ss;
	ss.x = float(renderScreen.dwWidth);
	ss.y = float(renderScreen.dwHeight);
	ss.z = 1.0f / renderScreen.dwWidth;
	ss.w = 1.0f / renderScreen.dwHeight;
	peSS->SetVector4(ss);
	
#ifdef GAME_RUSSIAN
	randStartTime = RRnd(0.0033f, 0.011f);
	totalTime = 1.0f / renderScreen.dwWidth;
	isLiveServiceDone = false;
	gameTicks = api->Storage().GetItemFloat("Profile.Global.Ticks", _FL_);
#endif

	return true;
}

//Исполнение в начале кадра
void PostEffects::StartFrame(float dltTime)
{
	m_uberFilter->StartFrame(dltTime);

#ifdef GAME_RUSSIAN
	totalTime += dltTime;
	dword rr = dword(m_render) * 16;
	dword ff = (dword(m_files) + 122) * 32;
	dword value = Min(dword(1), dword(totalTime * randStartTime) * dword(m_render->ProtectValue() - (rr + ff)));
	//Assert(dword(m_render->ProtectValue() - (rr + ff)) == 0);
	m_uberFilter->SetProtValue(float(value));
	m_uberFilter->SetProtValue2(float(value));
#endif
}

//Исполнение в конце кадра
void PostEffects::EndFrame(float dltTime)
{
	//if (api->DebugKeyState('G'))
	//	return;

	/*IRenderTarget * curRT = m_render->GetRenderTarget(_FL_);
	IRenderTargetDepth * curRTDepth = m_render->GetDepthStencil(_FL_);
	m_render->EndScene(null, false, true);

	m_render->SetRenderTarget(RTO_DONTOCH_CONTEXT, curRT, curRTDepth);
	m_render->BeginScene();

	float gamma = m_storageBrightness->Get();
	float small_delta = 0.05f;
	float delta = fabs(gamma - 0.5f);
	if (delta <= small_delta)
		gamma = 1.0f;
	else
	{
		if (gamma < 0.5f)
			gamma = 1.0f + (0.5f - small_delta - gamma) * 10.0f;
		else
			gamma = 1.0f - (gamma - (0.5f + small_delta)) * 1.75f;
	}

	m_gammaCoeff->SetFloat(gamma);
	m_rawImageTexture->SetTexture(curRT->AsTexture());

	m_render->PushViewport();

	RENDERVIEWPORT	vp;
	vp.X = 0;		vp.Width = renderScreen.dwWidth;
	vp.Y = 0;		vp.Height = renderScreen.dwHeight;
	vp.MinZ = 0.0f; vp.MaxZ = 1.0f;
	m_render->SetViewport(vp);

	//m_render->DrawFullScreenQuad(float(vp.Width), float(vp.Height), m_gammaShader);

	m_render->PopViewport();*/

#ifndef STOP_DEBUG
	m_uberFilter->DebugDraw();
#endif
}
