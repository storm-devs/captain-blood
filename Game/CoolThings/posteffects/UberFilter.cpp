#include "UberFilter.h"

UberFilter::UberFilter(IRender * render, IFileService * files)
{
	m_render = render;
	m_files = files;

	m_copyRT[0] = null;
	m_copyRT[1] = null;

	m_copyScreen = null;
	m_copyDepth = null;

	m_quarterRT[0] = null;
	m_quarterRT[1] = null;

	Init();
}

UberFilter::~UberFilter()
{
	RELEASE(m_copyRT[0]);
	RELEASE(m_copyRT[1]);

	RELEASE(m_copyScreen);
	RELEASE(m_copyDepth);

	RELEASE(m_quarterRT[0]);
	RELEASE(m_quarterRT[1]);

#ifdef GAME_RUSSIAN
	RELEASE(gameTicks);
	RELEASE(timeInSecFromStart);
#endif
}

void UberFilter::Init()
{
#ifdef GAME_RUSSIAN
	srand(GetTickCount());
	gameTicks = api->Storage().GetItemFloat("Profile.Global.Ticks", _FL_);
#endif

	m_geometry = (IGMXService *)api->GetService("GMXService");

	m_geometry->GetRenderModeShaderId("PEGO", m_RMSGlowObjectID);

#ifdef GAME_RUSSIAN
	timeInSecFromStart = null;

	_glowPower = 3.0f;
	_glowStart = 0.0f;
	_sharpPower = 4.5f;
	_dofPower = 32.0f;
	_dofFocus = 15.0f;

	protValue = 2.5f;

	_dofBlurNear = false;

	ChangePCoeffs();
#endif

	IRenderFilter::Init();

	m_render->GetShaderId("PE_Resize4x4Glow", m_shaderResize4x4Glow);
	m_render->GetShaderId("PE_Resize4x4GlowHQ", m_shaderResize4x4GlowHQ);
	
	m_render->GetShaderId("PE_GaussBlur25H", m_shaderBlurGauss5H);
	m_render->GetShaderId("PE_GaussBlur25V", m_shaderBlurGauss5V);

	m_render->GetShaderId("PE_DOFScale", m_shaderDOFScale);
	m_render->GetShaderId("PE_GaussBlur7H", m_shaderBlurGauss5HDOF);
	m_render->GetShaderId("PE_GaussBlur7V", m_shaderBlurGauss5VDOF);

	m_render->GetShaderId("PE_Sharpness", m_shaderSharpness);
	
	m_render->GetShaderId("PE_Glow", m_shaderGlow);
	m_render->GetShaderId("PE_RadialBlur", m_shaderRadialBlur);
	m_render->GetShaderId("PE_DOF", m_shaderDOF);
	m_render->GetShaderId("PE_Final", m_shaderFinal);

	m_render->GetShaderId("PE_SharpnessWithZ", m_shaderSharpnessWithZ);
	m_render->GetShaderId("PE_FinalWithZ", m_shaderFinalWithZ);

	multiSample = MULTISAMPLE_NONE;

#ifdef _XBOX
	varRestoreDepthTexture = m_render->GetTechniqueGlobalVariable("EDRAM_Depth", _FL_);
	m_render->GetShaderId("X360RestoreEDRAM_DepthOnly", x360RestoreDepthId);

	varDOFParams2 = m_render->GetTechniqueGlobalVariable("peDOFParams2", _FL_);
#else
	IRenderTarget * screen = m_render->GetRenderTarget(_FL_);
	RENDERSURFACE_DESC desc;
	screen->GetDesc(&desc);
	multiSample = desc.MultiSampleType;
	RELEASE(screen);
#endif

	varDepthTexture = m_render->GetTechniqueGlobalVariable("peDepthTexture", _FL_);
	m_render->GetShaderId("PE_DOFCalculateAlpha", shaderDOFCalculateAlpha);
	m_varDOFParams = m_render->GetTechniqueGlobalVariable("peDOFParams", _FL_);
	m_render->GetShaderId("PE_DOFZNear", m_shaderDOFZNear);
	m_render->GetShaderId("PE_DOFZFar", m_shaderDOFZFar);

	/*char shaderName[] = "PE_0000";
	for (int i=0; i<16; i++)
	{
		shaderName[3] = '0' + ((i>>3) & 1);
		shaderName[4] = '0' + ((i>>2) & 1);
		shaderName[5] = '0' + ((i>>1) & 1);
		shaderName[6] = '0' + ((i>>0) & 1);
		m_render->GetShaderId(shaderName, m_combinerShaders[i]);
	}*/

	m_rawImageTexture = m_render->GetTechniqueGlobalVariable("peSourceTexture", _FL_);

	const RENDERSCREEN & si = m_render->GetScreenInfo3D();

#ifndef _XBOX
	m_copyRT[0] = m_render->CreateRenderTarget(si.dwWidth, si.dwHeight, _FL_, FMT_A8R8G8B8, multiSample);
	m_copyRT[1] = m_render->CreateRenderTarget(si.dwWidth, si.dwHeight, _FL_, FMT_A8R8G8B8, multiSample);

	m_copyDepth = m_render->CreateDepthStencil(si.dwWidth, si.dwHeight, _FL_, FMT_D24S8, multiSample);

	#ifndef NO_TOOLS
		m_render->GetShaderId("PE_ToolsWindow", shaderToolsWindow);
		varToolsParams = m_render->GetTechniqueGlobalVariable("peToolsParams", _FL_);
		m_copyScreen = m_render->CreateTempRenderTarget(TRS_SCREEN_FULL_3D, TRC_FIXED_RGBA_8, _FL_, 2);
	#endif
#else
	m_copyRT[0] = m_render->CreateTempRenderTarget(TRS_SCREEN_FULL_3D, TRC_FIXED_RGBA_8, _FL_, 0);
	m_copyRT[1] = m_render->CreateTempRenderTarget(TRS_SCREEN_FULL_3D, TRC_FIXED_RGBA_8, _FL_, 1);
#endif

	m_quarterRT[0] = m_render->CreateTempRenderTarget(TRS_SCREEN_QUARTER_3D, TRC_FIXED_RGBA_8, _FL_);
	m_quarterRT[1] = m_render->CreateTempRenderTarget(TRS_SCREEN_QUARTER_3D, TRC_FIXED_RGBA_8, _FL_, 1);

	m_varRadialBlurParams = m_render->GetTechniqueGlobalVariable("peRadialBlurParams", _FL_);
	m_varGlowObjParams = m_render->GetTechniqueGlobalVariable("peGlowObjParams", _FL_);
	m_varGlowParams = m_render->GetTechniqueGlobalVariable("peGlowParams", _FL_);
	m_varGlowParams2 = m_render->GetTechniqueGlobalVariable("peGlowParams2", _FL_);
	m_varSharpnessWeights = m_render->GetTechniqueGlobalVariable("peSharpness", _FL_);
	m_varBlurWeights = m_render->GetTechniqueGlobalVariable("peBlurWeights", _FL_);
	m_varBlurSource = m_render->GetTechniqueGlobalVariable("peSourceTexture", _FL_);
	m_varOriginalScreen = m_render->GetTechniqueGlobalVariable("peOriginalScreenTexture", _FL_);
	varScreenSize2 = m_render->GetTechniqueGlobalVariable("peScreenSize2", _FL_);
	
	double weights_blur[BLUR_SIZE], weights_glow[BLUR_SIZE];

	int half = BLUR_SIZE >> 1;

	weights_blur[half] = 1.0;
	weights_glow[half] = 1.0;
	for (int i=1; i<half+1; i++)
	{
		double x = 3.0f * double(i) / double(half);
		//   Corresponding symmetric weights
		weights_blur[half - i] = weights_blur[half + i] = exp(-x * x / (2.0 * Sqr(2.0)));
		weights_glow[half - i] = weights_glow[half + i] = exp(-x * x / (2.0 * Sqr(2.0)));
	}
	double sum_blur = 0.0f, sum_glow = 0.0f;
	for (int i=0; i<BLUR_SIZE; i++)
	{
		sum_blur += weights_blur[i];
		sum_glow += weights_glow[i];
	}
	
	for (int i=0; i<BLUR_SIZE; i++)
	{
		weights_blur[i] /= sum_blur;
		weights_glow[i] /= sum_glow;

		m_vectorsGBlur[i].v = float(weights_blur[i]);
		m_vectorsGBlur[i].w = float(weights_glow[i]);
	}

	SetSharpness(1.0f);

	const RENDERSCREEN & rs = m_render->GetScreenInfo3D();

	// устанавливаем координаты для DOF
	Vector4 dofUV[24];
	for (int i=0, k=0, ix=-2, iy=-2; i<25; i++)
	{
		if (ix!=0 || iy!=0) 
			dofUV[k++] = Vector4(float(ix) / rs.dwWidth, float(iy) / rs.dwHeight, 0.0f, 0.0f);
		
		ix++;
		if (ix == 3) 
		{ 
			ix = -2; 
			iy++; 
		}
	}

	varDOFUV = m_render->GetTechniqueGlobalVariable("peDOFUV", _FL_);
	varDOFUV->SetVector4Array(dofUV, 24);

	isEnableDOF = true;
	isEnableGlow = true;
	isEnableSharpness = true;
	
#ifndef _XBOX
	IFileService * fs = (IFileService *)api->GetService("FileService");	Assert(fs);
	IIniFile * ini = fs->SystemIni();
	if (ini)
	{
		//isEnableDOF = !string::IsEqual(ini->GetString("PostEffects", "DepthOfField"), "off");
		//isEnableGlow = !string::IsEqual(ini->GetString("PostEffects", "Glow"), "off");
		//isEnableSharpness = !string::IsEqual(ini->GetString("PostEffects", "Black"), "off");
	}
#endif
}

#ifdef GAME_RUSSIAN
void UberFilter::ChangePCoeffs()
{
	protect1 = 1.2f + (protValue - 2.5f) * ((rand()%4 == 1) ? RRnd(0.05f, 2.2f) : 0.0f);
	protect2 = 2.3f + (protValue - 2.5f) * ((rand()%4 == 2) ? RRnd(0.05f, 2.2f) : 0.0f);
	protect3 = 5.5f + (protValue - 2.5f) * ((rand()%4 == 3) ? RRnd(0.05f, 2.2f) : 0.0f);
	protect4 = 1.3f + (protValue - 2.5f) * ((rand()%4 == 2) ? RRnd(0.05f, 2.2f) : 0.0f);
	protect5 = 3.7f + (protValue - 2.5f) * ((rand()%4 == 1) ? RRnd(0.05f, 2.2f) : 0.0f);

	if (rand()%5 == 2 && fabsf(protValue - 2.5f) > 1e-5f)
	{
		isEnableDOF = false;
		isEnableGlow = false;
		isEnableSharpness = true;
		
		FiltersParams::SharpnessParams & sharp = *m_params.GetSharpnessParams();
		sharp.isActive = true;
		sharp.mo = null;
		protect3 = 5.5f + RRnd(10.0f, 25.0f);
	}
	else
	{
		isEnableDOF = true;
		isEnableGlow = true;
	}
}
#endif

void UberFilter::RecalcGlow()
{
	double weights_glow[BLUR_SIZE];
	int half = BLUR_SIZE >> 1;

	weights_glow[half] = 1.0f;
	for (int i=1; i<half+1; i++)
	{
		double x = 3.0f * double(i) / double(half);
		//   Corresponding symmetric weights
		weights_glow[half - i] = weights_glow[half + i] = exp(-x * x / (2.0 * Sqr(m_params.GetGlowParams()->blurriness)));
	}
	double sum_glow = 0.0f;
	for (int i=0; i<BLUR_SIZE; i++)
		sum_glow += weights_glow[i];
	
	for (int i=0; i<BLUR_SIZE; i++)
	{
		weights_glow[i] /= sum_glow;

		m_vectorsGBlur[i].v = float(weights_glow[i]);
		m_vectorsGBlur[i].w = float(weights_glow[i]);
	}
}

void UberFilter::SetSharpness(float sharpness)
{
	float sides = -(sharpness - 1.0f) / 4.0f;
	m_sharpnessWeight = Vector4(sharpness, sides, 0.0f, 0.0f);
}

#ifdef _XBOX
void UberFilter::X360_RestoreDepth(float width, float height, IRenderTargetDepth * depth)
{
	varRestoreDepthTexture->SetTexture(depth->AsTexture());
	m_render->DrawFullScreenQuad(width, height, x360RestoreDepthId);
}
#endif

void UberFilter::FilterImage(IBaseTexture * source, IRenderTarget * destination)
{
	FiltersParams::GlowParams & glow = *m_params.GetGlowParams();
	FiltersParams::RadialBlurParams & rblur = *m_params.GetRadialBlurParams();
	FiltersParams::SharpnessParams & sharp = *m_params.GetSharpnessParams();
	FiltersParams::DOFParams & dof = *m_params.GetDOFParams();

	// если нету активных фильтров
	if (!sharp.isActive && !glow.isActive && !dof.isActive && !rblur.isActive)
		return;

	if (api->DebugKeyState('P') && !api->DebugKeyState(VK_CONTROL))
		return;

#ifdef GAME_RUSSIAN
	if (rand()%600 == 301)
		ChangePCoeffs();
#endif


	RENDERVIEWPORT oldVP = m_render->GetViewport();
	debugVP = oldVP;

	RENDERSCREEN renderScreen = m_render->GetScreenInfo3D();

	m_render->PushViewport();
	m_render->PushRenderTarget();

	SetSharpness(m_params.GetSharpnessParams()->power);
	RecalcGlow();

#ifdef GAME_RUSSIAN 
	float deltaTime = api->GetDeltaTime();
	_glowPower += deltaTime * 0.047f * (protect1 - 1.2f);
	_glowStart += deltaTime * 0.05f * (protect2 - 2.3f);
	_sharpPower += deltaTime * 0.03f *(protect3 - 5.5f);
	_dofPower += deltaTime * 0.09f * (protect4 - 1.3f);
	_dofFocus += deltaTime * 0.076f * (protect5 - 3.7f);

	m_varBlurWeights->SetVector4Array(m_vectorsGBlur, BLUR_SIZE);
	m_sharpnessWeight.x *= (_sharpPower - 3.5f);
	m_varSharpnessWeights->SetVector4(m_sharpnessWeight);
	m_varGlowParams->SetVector4(Vector4(Min(4.9f, glow.power * (_glowPower - 2.0f)), Max(0.0f, glow.threshold - _glowStart), glow.foreFactor, glow.backFactor));
	m_varGlowParams2->SetVector4(Vector4(glow.realColors, 0.0f, 0.0f, 0.0f));
	m_varRadialBlurParams->SetVector4(Vector4(rblur.focus, rblur.power, rblur.blurCenter.x, rblur.blurCenter.y));

	if (rand()%1000 == 501 && fabsf(protValue - 2.5f) > 1e-5f)
	{
		_dofBlurNear = true;
	}

#else
	m_varBlurWeights->SetVector4Array(m_vectorsGBlur, BLUR_SIZE);
	m_varSharpnessWeights->SetVector4(m_sharpnessWeight);
	m_varGlowParams->SetVector4(Vector4(glow.power, glow.threshold, glow.foreFactor, glow.backFactor));
	m_varGlowParams2->SetVector4(Vector4(glow.realColors, 0.0f, 0.0f, 0.0f));
	m_varRadialBlurParams->SetVector4(Vector4(rblur.focus, rblur.power, rblur.blurCenter.x, rblur.blurCenter.y));
#endif

	float width = float(renderScreen.dwWidth);
	float height = float(renderScreen.dwHeight);

	RENDERVIEWPORT fullVP, quartVP;
	fullVP.X = 0;					fullVP.Y = 0;
	fullVP.Width = dword(width);	fullVP.Height = dword(height);
	fullVP.MinZ = 0.0f;				fullVP.MaxZ = 1.0f;

	quartVP.X = 0;					quartVP.Y = 0;
	quartVP.Width = dword(width)/4;	quartVP.Height = dword(height)/4;
	quartVP.MinZ = 0.0f;			quartVP.MaxZ = 1.0f;

	varScreenSize2->SetVector4(Vector4(sqrtf(Sqr(width) + Sqr(height)), 0.0f, 0.0f, 0.0f));

	dword pixID = m_render->pixBeginEvent(_FL_, "Vano posteffects");

	IRenderTarget * curRT = m_render->GetRenderTarget(_FL_);
	IRenderTargetDepth * curRTDepth = m_render->GetDepthStencil(_FL_);

#ifndef _XBOX
	m_render->ResolveDepth(m_copyDepth);

	#ifndef NO_TOOLS
		bool toolsMode = oldVP.X || oldVP.Y || oldVP.Width != renderScreen.dwWidth || oldVP.Height != renderScreen.dwHeight;
	
		if (toolsMode)
			m_render->ResolveColor(m_copyScreen->AsTexture());
	#endif
#endif

	m_varOriginalScreen->SetTexture(curRT->AsTexture());

	int currentRT = 0;
	//bool depthRestored = false;

	dof.isActive &= isEnableDOF;
	glow.isActive &= isEnableGlow;
	sharp.isActive &= isEnableSharpness;

	bool sharpIsLast = !glow.isActive && !dof.isActive && !rblur.isActive;
	bool glowIsLast = !dof.isActive && !rblur.isActive;
	bool rblurIsLast = !dof.isActive;

#ifdef _XBOX
	varDepthTexture->SetTexture(curRTDepth->AsTexture());

	// На XBox'e можно совместить SHARP и начало GLOW
	if (sharp.isActive && !glow.isActive)
#else
	if (sharp.isActive)
#endif
	{
		dword pixID1 = m_render->pixBeginEvent(_FL_, "Sharp");

		m_render->SetViewport(fullVP);

		if (sharpIsLast)
			m_render->SetRenderTarget(RTO_DONTOCH_CONTEXT, curRT, curRTDepth);
		else
			m_render->SetRenderTarget(RTO_DONTOCH_CONTEXT, m_copyRT[currentRT], m_copyDepth);

		m_render->BeginScene();

#ifndef _XBOX
		// копируем экран и очищаем альфу
		m_render->DrawFullScreenQuad(width, height, m_shaderFinal);
#endif

		// наводим резкость
		m_render->DrawFullScreenQuad(width, height, m_shaderSharpness);

		m_render->EndScene(null, false, true);

		m_varOriginalScreen->SetTexture(m_copyRT[currentRT]->AsTexture());

		currentRT ^= 1;

		m_render->pixEndEvent(_FL_, pixID1);
	}

	if (glow.isActive)
	{
		dword pixID1 = m_render->pixBeginEvent(_FL_, (sharp.isActive) ? "Glow+Sharp" : "Glow");
		// чистим альфу и рисуем объекты в альфу
		{
			//m_render->SetRenderTarget(RTO_DONTOCH_CONTEXT, m_copyRT[currentRT], m_copyDepth);
			//m_render->SetRenderTarget(RTO_DONTOCH_CONTEXT, m_copyRT[currentRT], m_copyDepth);

#ifdef _XBOX
			m_render->SetRenderTarget(RTO_DONTOCH_CONTEXT, m_copyRT[currentRT], curRTDepth);
			m_render->BeginScene();
			// восстанавливаем Z-Buffer если он не был восстановлен в предыдущем фильтре
			//if (!depthRestored)
				//X360_RestoreDepth(float(width), float(height), curRTDepth);
			//depthRestored = true;

			// Накладываем резкость или копируем экран и очищаем альфу
			if (sharp.isActive)
				m_render->DrawFullScreenQuad(width, height, m_shaderSharpnessWithZ);
			else
				m_render->DrawFullScreenQuad(width, height, m_shaderFinalWithZ);
#else
			m_render->SetRenderTarget(RTO_DONTOCH_CONTEXT, m_copyRT[currentRT], m_copyDepth);
			m_render->BeginScene();
			// копируем экран и очищаем альфу
			m_render->DrawFullScreenQuad(width, height, m_shaderFinal);
#endif

			// рендерим в альфу объекты
			if (glow.objects && !glow.objects->IsEmpty())
			{
				// рисуем объекты в альфу на которых glow действует по другому
				m_geometry->SetRenderMode(&m_RMSGlowObjectID);
				
				m_render->SetViewport(oldVP);

				for (int i=0; i<glow.objects->Len(); i++)
				{
					FiltersParams::GlowObject & gobj = (*glow.objects)[i];

					// ставим параметры для рендера
					m_varGlowObjParams->SetVector4(Vector4(gobj.power, gobj.threshold, 0.0f, 0.0f));

					if (gobj.object.Validate())
						gobj.object.Ptr()->Command("postdraw", 0, null);
				}

				m_geometry->SetRenderMode(null);
				
				m_render->SetViewport(fullVP);
			}

			m_render->EndScene(null, false, true);

			m_varOriginalScreen->SetTexture(m_copyRT[currentRT]->AsTexture());
			currentRT ^= 1;
		}

		// уменьшаем картинку в 4 раза
			m_render->SetViewport(quartVP);

		// ресайзим картинку до 1/4 размера
			m_render->SetRenderTarget(RTO_DONTOCH_CONTEXT, m_quarterRT[0], null);
			m_render->BeginScene();
			m_render->DrawFullScreenQuad(width / 4.0f, height / 4.0f, (glow.isHighQuality) ? m_shaderResize4x4GlowHQ : m_shaderResize4x4Glow);
			m_render->EndScene();

		// блюрим картинку по горизонтали
			m_render->SetRenderTarget(RTO_DONTOCH_CONTEXT, m_quarterRT[1]);

			m_render->BeginScene();
			m_varBlurSource->SetTexture(m_quarterRT[0]->AsTexture());
			m_render->DrawFullScreenQuad(width / 4.0f, height / 4.0f, m_shaderBlurGauss5H);
			m_render->EndScene();

		// блюрим картинку по вертикали
			m_render->SetRenderTarget(RTO_DONTOCH_CONTEXT, m_quarterRT[0], null);

			m_render->BeginScene();
			m_varBlurSource->SetTexture(m_quarterRT[1]->AsTexture());
			m_render->DrawFullScreenQuad(width / 4.0f, height / 4.0f, m_shaderBlurGauss5V);
			m_render->EndScene();

		// добавляем glow к основной картинке
			m_render->SetViewport(fullVP);

			if (glowIsLast)
				m_render->SetRenderTarget(RTO_DONTOCH_CONTEXT, curRT, curRTDepth);
			else
				m_render->SetRenderTarget(RTO_DONTOCH_CONTEXT, m_copyRT[currentRT], null);
			m_render->BeginScene();

			m_rawImageTexture->SetTexture(m_quarterRT[0]->AsTexture());
			m_render->DrawFullScreenQuad(width, height, m_shaderGlow);

			m_varOriginalScreen->SetTexture(m_copyRT[currentRT]->AsTexture());
			currentRT ^= 1;

			m_render->EndScene();

		m_render->pixEndEvent(_FL_, pixID1);
	}

	if (rblur.isActive)
	{
		dword pixID1 = m_render->pixBeginEvent(_FL_, "RadialBlur");

		if (rblurIsLast)
			m_render->SetRenderTarget(RTO_DONTOCH_CONTEXT, curRT, curRTDepth);
		else
			m_render->SetRenderTarget(RTO_DONTOCH_CONTEXT, m_copyRT[currentRT], null);
		m_render->BeginScene();

		m_render->DrawFullScreenQuad(width, height, m_shaderRadialBlur);

		m_varOriginalScreen->SetTexture(m_copyRT[currentRT]->AsTexture());
		currentRT ^= 1;

		m_render->EndScene();
	
		m_render->pixEndEvent(_FL_, pixID1);
	}

	if (dof.isActive && (dof.blurFar || dof.blurNear) && dof.blurFactor > 1e-4f)
	{
		dword pixID1 = m_render->pixBeginEvent(_FL_, "DOF");

		m_render->SetRenderTarget(RTO_DONTOCH_CONTEXT, m_copyRT[currentRT], m_copyDepth);
		m_render->BeginScene();

		const Matrix & proj = m_render->GetProjection();

		float kf[6];
		for (int i=0; i<6; i++)
			kf[i] = powf(float(i + 1.0f) / 6.0f, 2.5f);

		//float t = 0.0432901f;
		//float i = 1.0f - t;
		//float v = i*i*i * P0 + 3.0f*i*i*t * P1 + 3.0f*i*t*t * P2 + t*t*t * P3;

		//float kResolution = 1.0f;//(width * height) / (1280.0f * 720.0f);
		//if (api->DebugKeyState('9'))
		// Волшебные параметры для увеличения/уменьшения силы DOF'a от размеров экрана, 1280x720 эталон
		float kResolution = (width * height) / (1280.0f * 720.0f);
		kResolution = powf(kResolution, (kResolution < 1.0f) ? 0.15f : 0.3f);

#ifdef GAME_RUSSIAN
		float dofFocusPoint = Max(0.0f, dof.focusPoint - (_dofFocus - 15.0f));
		float dofBlurFactor = dof.blurFactor * (_dofPower - 31.0f);

		//dof.blurNear |= _dofBlurNear;
#else
		float dofFocusPoint = dof.focusPoint;
		float dofBlurFactor = dof.blurFactor;
#endif

#ifdef _XBOX
		// X360 - версия, в альфу рассчитываем коэффициент блюра 
		//float blurPowK = Bring2Range(0.37f, 0.95f, 1.0f, 15.0f, dof.focusPoint * 0.5f);
		Vector4 dofParams;
		dofParams.x = dofFocusPoint;
		dofParams.y = (dof.blurFar) ? 1.0f : 0.0f;
		dofParams.z = (dof.blurNear) ? 1.0f : 0.0f;
		dofParams.w = 2.0f * dofBlurFactor * kResolution;//powf(dof.blurFactor, blurPowK);
		varDOFParams2->SetVector4(dofParams);
		m_render->DrawFullScreenQuad(width, height, shaderDOFCalculateAlpha);
#else
		// очищаем альфу
		m_render->DrawFullScreenQuad(width, height, m_shaderFinal);

		// PC - версия, в альфу рисуется несколько плашек с коэффициентом блюра для каждого Z
		Vector vz;
		Vector4 vproj;

		float bd[6] = {1.1765f,1.4286f,1.8182f,2.5f,4.0f,7.0f};
		float fd[5] = {0.6667f,0.5000f,0.4000f,0.3333f,0.2857f};

		if (dof.blurNear) 
			for (int i=0; i<5; i++)
			{
				vz = Vector(0.0f, 0.0f, dofFocusPoint * fd[i]);
				vproj = proj.Projection(vz);
				m_varDOFParams->SetVector4(Vector4(vproj.z, kResolution * 2.0f * dofBlurFactor * kf[1 + i], 0.0f, 0.0f));
				m_render->DrawFullScreenQuad(width, height, m_shaderDOFZNear);
			}

		if (dof.blurFar)
			for (int i=0; i<6; i++)
			{
				vz = Vector(0.0f, 0.0f, dofFocusPoint * bd[i]);
				vproj = proj.Projection(vz);
				m_varDOFParams->SetVector4(Vector4(vproj.z, kResolution * 2.0f * dofBlurFactor * kf[i], 0.0f, 0.0f));
				m_render->DrawFullScreenQuad(width, height, m_shaderDOFZFar);
			}
#endif
		m_render->EndScene(null, false, true);

		m_varOriginalScreen->SetTexture(m_copyRT[currentRT]->AsTexture());
		currentRT ^= 1;

		// достаем картинку из основной с блюром по дальности
			m_render->SetViewport(quartVP);

			m_render->SetRenderTarget(RTO_DONTOCH_CONTEXT, m_quarterRT[1]);
			m_render->BeginScene();
			m_render->DrawFullScreenQuad(width / 4.0f, height / 4.0f, m_shaderDOFScale);
			m_render->EndScene();

		// легкий блюр по горизонтали
			m_render->SetRenderTarget(RTO_DONTOCH_CONTEXT, m_quarterRT[0]);
			m_render->BeginScene();
			m_varBlurSource->SetTexture(m_quarterRT[1]->AsTexture());
			m_render->DrawFullScreenQuad(width / 4.0f, height / 4.0f, m_shaderBlurGauss5HDOF);
			m_render->EndScene();

		// легкий блюр по вертикали
			m_render->SetRenderTarget(RTO_DONTOCH_CONTEXT, m_quarterRT[1]);
			m_render->BeginScene();
			m_varBlurSource->SetTexture(m_quarterRT[0]->AsTexture());
			m_render->DrawFullScreenQuad(width / 4.0f, height / 4.0f, m_shaderBlurGauss5VDOF);
			m_render->EndScene();

		m_render->SetRenderTarget(RTO_DONTOCH_CONTEXT, curRT, curRTDepth);
		m_render->BeginScene();

		m_varBlurSource->SetTexture(m_quarterRT[1]->AsTexture());

		m_varDOFParams->SetVector4(Vector4(0.0f, dofBlurFactor, 0.0f, 0.0f));

		m_render->DrawFullScreenQuad(width, height, m_shaderDOF);

		//m_varOriginalScreen->SetTexture(m_copyRT[currentRT]->AsTexture());
		//currentRT ^= 1;

		m_render->EndScene(null, false, true);

		m_render->pixEndEvent(_FL_, pixID1);
	}

	m_render->PopRenderTarget(RTO_DONTOCH_CONTEXT);

// для PC в режиме редактора восстанавливаем само окошко редактора
#if !defined(_XBOX) && !defined(NO_TOOLS)
	// если вьюпорт не совпадает с экраном, то рисуем старый экран c вырезанным окошком
	if (toolsMode)
	{
		m_render->BeginScene();
	
		m_render->SetViewport(fullVP);

		// рисуем первичный экран
		float nearV = float(oldVP.Y) / renderScreen.dwHeight;
		float farV = float(oldVP.Y + oldVP.Height) / renderScreen.dwHeight;
		float nearU = float(oldVP.X) / renderScreen.dwWidth;
		float farU = float(oldVP.X + oldVP.Width) / renderScreen.dwWidth;
		varToolsParams->SetVector4(Vector4(nearU, farU, nearV, farV));
		m_varOriginalScreen->SetTexture(m_copyScreen->AsTexture());
		m_render->DrawFullScreenQuad(width, height, shaderToolsWindow);

		m_render->EndScene();
	}
#endif

	m_render->PopViewport();

	m_render->pixEndEvent(_FL_, pixID);

	/*if (api->DebugKeyState('9'))
	{
		m_render->BeginScene();

		RS_SPRITE spr[4];
		spr[0].vPos = Vector (-1.0f, 1.0f, 0.1f);
		spr[1].vPos = Vector (1.0f, 1.0f, 0.1f);
		spr[2].vPos = Vector (1.0f, -1.0f, 0.1f);
		spr[3].vPos = Vector (-1.0f, -1.0f, 0.1f);

		spr[0].tu = 0.0f;  spr[0].tv = 0.0f;
		spr[1].tu = 1.0f;  spr[1].tv = 0.0f;
		spr[2].tu = 1.0f;  spr[2].tv = 1.0f;
		spr[3].tu = 0.0f;  spr[3].tv = 1.0f;

		spr[0].dwColor = 0xFFFFFFFF;
		spr[1].dwColor = 0xFFFFFFFF;
		spr[2].dwColor = 0xFFFFFFFF;
		spr[3].dwColor = 0xFFFFFFFF;

		m_render->DrawSprites(m_quarterRT[0]->AsTexture(), spr, 1, "dbgSpritesNoBlend");

		m_render->EndScene();
	}*/
}

void UberFilter::StartFrame(float dltTime)
{
#ifdef GAME_RUSSIAN
	if (!timeInSecFromStart || rand()%1000==501)
	{
		RELEASE(timeInSecFromStart);
		const char* difficulty = api->Storage().GetString("profile.global.difficulty", NULL);
		if (difficulty)
		{
			char tmpBuff[512];
			crt_snprintf(tmpBuff, sizeof(tmpBuff), "Profile.%s.time.totalSec", difficulty);
			timeInSecFromStart = api->Storage().GetItemFloat(tmpBuff, _FL_);
		}
	}
#endif

	m_params.GetRadialBlurParams()->isActive = false;
	m_params.GetGlowParams()->isActive = false;
	m_params.GetSharpnessParams()->isActive = false;
	m_params.GetDOFParams()->isActive = false;
}

void UberFilter::DebugDraw()
{
#ifndef STOP_DEBUG
	static bool showDebug = false;

	if (api->DebugKeyState('P',VK_CONTROL))
	{
		showDebug ^= 1;
		Sleep(200);
	}

	if (!showDebug)
		return;

	const RENDERSCREEN & rs = m_render->GetScreenInfo3D();

	// рисуем темный прямоугольник
		float x1 = 2.0f * float(debugVP.X) / float(rs.dwWidth) - 1.0f;
		float x2 = 2.0f * float(debugVP.X + debugVP.Width) / float(rs.dwWidth) - 1.0f;
		
		float y2 = 1.0f - 2.0f * float(debugVP.Y + debugVP.Height) / float(rs.dwHeight);
		float y1 = y2 + 2.0f * (80.0f / rs.dwHeight);

		RS_SPRITE spr[4];
		spr[0].vPos = Vector (x2, y1, 0.0f);
		spr[1].vPos = Vector (x1, y1, 0.0f);
		spr[2].vPos = Vector (x1, y2, 0.0f);
		spr[3].vPos = Vector (x2, y2, 0.0f);

		spr[0].tu = 0.0f;  spr[0].tv = 0.0f;
		spr[1].tu = 0.0f;  spr[1].tv = 0.0f;
		spr[2].tu = 0.0f;  spr[2].tv = 0.0f;
		spr[3].tu = 0.0f;  spr[3].tv = 0.0f;

		spr[0].dwColor = 0x9F000000;
		spr[1].dwColor = 0x9F000000;
		spr[2].dwColor = 0x9F000000;
		spr[3].dwColor = 0x9F000000;

		m_render->DrawSprites(null, spr, 1, "dbgSpritesColorOnly");

	FiltersParams::GlowParams & glow = *m_params.GetGlowParams();
	FiltersParams::RadialBlurParams & rblur = *m_params.GetRadialBlurParams();
	FiltersParams::SharpnessParams & sharp = *m_params.GetSharpnessParams();
	FiltersParams::DOFParams & dof = *m_params.GetDOFParams();

	float y = debugVP.Y + float(debugVP.Height) - 20.0f;
	if (dof.isActive)
	{
		m_render->Print(debugVP.X + 5.0f, y, 0xFFFFFFFF, "DOF : %.2f \"%s\"", powf(dof.blurFactor, 1.0f / 1.2f), (dof.mo) ? dof.mo->GetObjectID().c_str() : "");
		y -= 20.0f;
	}
	if (rblur.isActive)
	{
		m_render->Print(debugVP.X + 5.0f, y, 0xFFFFFFFF, "RadialBlur : %.2f \"%s\"", rblur.power, (rblur.mo) ? rblur.mo->GetObjectID().c_str() : "");
		y -= 20.0f;
	}
	if (glow.isActive)
	{
		m_render->Print(debugVP.X + 5.0f, y, 0xFFFFFFFF, "Glow : %.2f \"%s\"", glow.power, (glow.mo) ? glow.mo->GetObjectID().c_str() : "");
		y -= 20.0f;
	}
	if (sharp.isActive)
	{
		m_render->Print(debugVP.X + 5.0f, y, 0xFFFFFFFF, "Sharpness : %.2f \"%s\"", sharp.power, (sharp.mo) ? sharp.mo->GetObjectID().c_str() : "");
		y -= 20.0f;
	}
#endif
}