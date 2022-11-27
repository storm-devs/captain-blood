#include "..\PostProcess.h"

//Blur::Blur() : counters(_FL_)
//{
//	SourceTex = null;
////	FilterTex = null;
//
////	Filter = null;
//
////	ActiveMissionObject = null;
////	pGeometry = null;
//
//	pChainTex[0] = null;
//	pChainTex[1] = null;	
//
//	OneDivScreenWidth  = null;
//	OneDivScreenHeight = null;
//
//	PixelSize = null;
//
//	GlowBlurPass = null;
//
//	DOFOriginal	  = null;
//	DOFBlured	  = null;
//	DOFBlurFactor = null;
//
////	BlurA = null;
////	BlurB = null;
//	BlurC = null;
//	BlurD = null;
//
////	BlurPasses = 6;
//
//	GlowThreshold = null;
//	GlowPower	  = null;
//
////	BlurFactor = null;
//
//	InnerFactor = null;
//	OuterFactor = null;
//
////	pSourceTexture = null;
////	pZTexture	   = null;
//
//	DOF_Params = null;
//
////	blurFactor = 1.0f;
//
////	radial = false;
//
////	passCount = BlurPasses;
//	divider	  = 1;
//
////	detailLevel = det_hi;
//	detailLevel = det_off;
//
//	mis_index = -1;
//
//	innerFactor = 0.0f;
//	outerFactor = 0.0f;
//
//	count_k = 0.0f;
//
//	pServ = null;
//}
//
//Blur::~Blur()
//{	
//	DOFBlurFactor = null;
///*
//	if( pSourceTexture )
//	{
//		pSourceTexture->Release();
//		pSourceTexture = null;
//	}
//
//	if( pZTexture )
//	{
//		pZTexture->Release();
//		pZTexture = null;
//	}
//*/
//	GlowThreshold = null;
//	GlowPower = null;
//	GlowBlurPass = null;
//
///*	if( BlurFactor )
//	{
//		BlurFactor->Release();
//		BlurFactor = null;
//	}*/
//
//	InnerFactor = null;
//	OuterFactor = null;
//	DOFBlured = null;
//	DOFOriginal = null;
//
///*	if( BlurA )
//	{
//		BlurA->Release();
//		BlurA = null;
//	}
//	if( BlurB )
//	{
//		BlurB->Release();
//		BlurB = null;
//	}*/
//
//	BlurC = null;
//	BlurD = null;
//
//	if( pChainTex[0] )
//	{
//		pChainTex[0]->Release();
//		pChainTex[0] = null;
//	}
//
//	if( pChainTex[1] )
//	{
//		pChainTex[1]->Release();
//		pChainTex[1] = null;
//	}
//
//	OneDivScreenWidth = null;
//	OneDivScreenHeight = null;
//	DOF_Params = null;
//	SourceTex = null;
///*	if( FilterTex )
//	{
//		FilterTex->Release();
//		FilterTex = null;
//	}
//
//	if( Filter )
//	{
//		Filter->Release();
//		Filter = null;
//	}*/
//
//	PixelSize = null;
//}
//
//void Blur::UserInit()
//{
//	#ifndef _XBOX
//
//	IFileService *storage = (IFileService *)api->GetService("FileService");
//	Assert(storage)
//
//	IIniFile *ini = storage->SystemIni();//storage->OpenIniFile(api->GetVarString("Ini"),_FL_);
//
//	if( ini )
//	{
//		const char *s = ini->GetString("PostEffects","Blur","hi");
//
//		if( string::IsEqual(s,"off"))
//		{
//			detailLevel = det_off;
//		}
//		else
//		if( string::IsEqual(s,"low"))
//		{
//			detailLevel = det_low; divider = 4;
//		}
//		else
//		if( string::IsEqual(s,"med"))
//		{
//			detailLevel = det_med; divider = 2;
//		}
//		else
//		{
//			detailLevel = det_hi ; divider = 1;
//		}
//
//		//ini->Release();
//	}
//
//	#else
//
//	detailLevel = det_low; divider = 4;
//
//	#endif
//
//	if( detailLevel == det_off )
//		return;
//
//	SourceTex = Render().GetTechniqueGlobalVariable("PP_CommonTexture",_FL_);
////	FilterTex = Render().GetTechniqueGlobalVariable("PP_ScreenTexture",_FL_);
//
//	OneDivScreenWidth  = Render().GetTechniqueGlobalVariable("TexelSizeX",_FL_);
//	OneDivScreenHeight = Render().GetTechniqueGlobalVariable("TexelSizeY",_FL_);
//
//	GlowBlurPass = Render().GetTechniqueGlobalVariable("GlowBlurPass",_FL_);
//
//	GlowThreshold = Render().GetTechniqueGlobalVariable("GlowThreshold",_FL_);
//	GlowPower	  = Render().GetTechniqueGlobalVariable("GlowPower"	   ,_FL_);
//
////	BlurFactor = Render().GetTechniqueGlobalVariable("BlurFactor",_FL_);
//
//	InnerFactor = Render().GetTechniqueGlobalVariable("InnerFactor",_FL_);
//	OuterFactor = Render().GetTechniqueGlobalVariable("OuterFactor",_FL_);
//
////	pGeometry = (IGMXService*)api->GetService("GMXService");
////	assert(pGeometry);
//
//	DOFOriginal	  = Render().GetTechniqueGlobalVariable("PP_ScreenTexture",_FL_);
//	//DOFBlured	  = Render().GetTechniqueGlobalVariable("DOFBlured"		  ,_FL_);
//	//DOFBlurFactor = Render().GetTechniqueGlobalVariable("DOFBlurFactor"	  ,_FL_);
////	BlurA = Render().GetTechniqueGlobalVariable("BlurA",_FL_);
////	BlurB = Render().GetTechniqueGlobalVariable("BlurB",_FL_);
//	BlurC = Render().GetTechniqueGlobalVariable("BlurC",_FL_);
//	BlurD = Render().GetTechniqueGlobalVariable("BlurD",_FL_);
//
//	//DOF_Params = Render().GetTechniqueGlobalVariable("DOF_Params",_FL_);
//
//#ifdef _XBOX
//	PixelSize = Render().GetTechniqueGlobalVariable("DOFPixelSize",_FL_);
//#endif
//
//	DWORD dwWidth  = Render().GetScreenInfo3D().dwWidth;
//	DWORD dwHeight = Render().GetScreenInfo3D().dwHeight;
////	DWORD dwWidth  = 128;
////	DWORD dwHeight = 128;
//
//	RENDERFORMAT TexFormat = Render().GetScreenInfo3D().BackBufferFormat;
//
//	texViewPort.X = 0;
//	texViewPort.Y = 0;
//
//	texViewPort.Width  = dwWidth /divider;
//	texViewPort.Height = dwHeight/divider;
//
//	texViewPort.MinZ = 0.0f;
//	texViewPort.MaxZ = 1.0f;
//
////	pChainTex[0] = Render().CreateRenderTarget(dwWidth, dwHeight,_FL_,TexFormat);
////	pChainTex[1] = Render().CreateRenderTarget(dwWidth, dwHeight,_FL_,TexFormat);
//
////	pSourceTexture = Render().CreateRenderTarget(Render().GetScreenInfo().dwWidth,Render().GetScreenInfo().dwHeight,_FL_,TexFormat);
////	pZTexture	   = Render().CreateRenderTarget(Render().GetScreenInfo().dwWidth,Render().GetScreenInfo().dwHeight,_FL_,TexFormat);
//
//	switch( detailLevel )
//	{
//		case det_low:
//			pChainTex[0] = Render().CreateTempRenderTarget(TRS_SCREEN_QUARTER_3D,TRC_FIXED_RGBA_8,_FL_,0);
//			pChainTex[1] = Render().CreateTempRenderTarget(TRS_SCREEN_QUARTER_3D,TRC_FIXED_RGBA_8,_FL_,1);
//			break;
//
//		case det_med:
//			pChainTex[0] = Render().CreateTempRenderTarget(TRS_SCREEN_HALF_3D	 ,TRC_FIXED_RGBA_8,_FL_,0);
//			pChainTex[1] = Render().CreateTempRenderTarget(TRS_SCREEN_HALF_3D	 ,TRC_FIXED_RGBA_8,_FL_,1);
//			break;
//
//		case det_hi:
//			pChainTex[0] = Render().CreateTempRenderTarget(TRS_SCREEN_FULL_3D	 ,TRC_FIXED_RGBA_8,_FL_,0);
//			pChainTex[1] = Render().CreateTempRenderTarget(TRS_SCREEN_FULL_3D	 ,TRC_FIXED_RGBA_8,_FL_,1);
//			break;
//	}
//
////	vp = Render().GetFullScreenViewPort();
//
////	Render().GetShaderId("IterativeRadialBlur",IterativeRadialBlur_id);
//	Render().GetShaderId("ShowGlow",ShowGlow_id);
//	Render().GetShaderId("BlurPaste_Low_Rad",BlurPaste_Low_Rad_id);
//
////	Render().GetShaderId("IterativeRadialBlurH_First_low",IterativeRadialBlurH_First_Low_id);
//
//	Render().GetShaderId("ResizeRectSimple",ResizeRectSimple_id);
//
//	Render().GetShaderId("IterativeRadialBlurH",IterativeRadialBlurH_id);
//	Render().GetShaderId("IterativeRadialBlurV",IterativeRadialBlurV_id);
//
///*	const int n = 16;
//
////	Filter = Render().CreateTexture(n,1,1,USAGE_WRITEONLY|USAGE_DYNAMIC,FMT_R32F,_FL_,POOL_DEFAULT);
//	Filter = Render().CreateTexture(n,1,1,0,FMT_R32F,_FL_);
//	Assert(Filter);
//
//	RENDERLOCKED_RECT r;
//	Assert(Filter->LockRect(0,&r,null,0))
//
//	float *p = (float *)r.pBits;
//
//	for( int j = 0 ; j < n ; j++ )
//	{
//		float x = j - n/2 + 0.5f;
//
//	//	float t = 0.5f;
//	//	float t = 1.0f;
//		float t = 2.0f;
//
//		float y = 1.0f/(sqrtf(2.0f*PI)*t)*expf(-x*x/(2*t*t));
//
//		p[j] = y;
//	}
//
//	Filter->UnlockRect(0);*/
//
//	pServ = (PostProcessService *)api->GetService("PostProcessService");
//}
//
//void Blur::FilterImage(IBaseTexture *source, IRenderTarget *destination)
//{	
//	return;
////	Assert(detailLevel > det_off)
//	if( detailLevel < det_low )
//		return;
//
//	if( !pServ->Enabled())
//	{
//	//	blurFactor = 0.0f;
//
//		innerFactor = 0.0f;
//		outerFactor = 0.0f;
//
//		count_k = 0.0f;
//
//		return;
//	}
//
//	if( mis_index >= 0 && count_k > 0.0f )
//	{
//		int n = counters[mis_index].counter;
//
//		Assert(n > 0)
//
//	//	float k = 1.0f/n;
//		float k = 1.0f/count_k;
//
//	//	Assert(count_k > 0.0f)
//
//		if( n > 1 )
//		{
//		//	blurFactor *= k;
//		}
//	}
//	else
//	{
//	//	blurFactor = 0.0f;
//
//		innerFactor = 0.0f;
//		outerFactor = 0.0f;
//	}
//
////	Assert(blurFactor >= 0.0f)
////	Assert(blurFactor <= 1.0f)
///*	if( blurFactor < 0.0f )
//		blurFactor = 0.0f;
//	if( blurFactor > 1.0f )
//		blurFactor = 1.0f;*/
//
//	Render().PushRenderTarget();
//
//	if( OneDivScreenWidth )
//		OneDivScreenWidth ->SetFloat((1.0f/(texViewPort.Width *divider))*0.5f);
//
//	if( OneDivScreenHeight )
//		OneDivScreenHeight->SetFloat((1.0f/(texViewPort.Height*divider))*0.5f);
//
//	if( GlowBlurPass )
//		GlowBlurPass ->SetFloat(1.0f);
//
//	if( GlowThreshold )
//		GlowThreshold->SetFloat(0.0f);
//
//	if( GlowPower )
//		GlowPower->SetFloat(0.0f);
//
////	if( BlurFactor )
////		BlurFactor->SetFloat(blurFactor);
//
//	if( InnerFactor )
//		InnerFactor->SetFloat(innerFactor);
//	if( OuterFactor )
//		OuterFactor->SetFloat(outerFactor);
//
//	#ifdef _XBOX
//
//	if( detailLevel == det_low )
//	{
//		if( PixelSize )
//			PixelSize->SetVector(Vector(
//				1.0f/(texViewPort.Width *divider),
//				1.0f/(texViewPort.Height*divider),0.0f));
//
//		if( SourceTex )
//			SourceTex->SetTexture(source);
//
//		Render().SetRenderTarget(RTO_DONTOCH_CONTEXT,pChainTex[0]);
//		Render().SetViewport(texViewPort);
//		Render().BeginScene();
//
//		Render().DrawFullScreenQuad(
//			(float)texViewPort.Width,
//			(float)texViewPort.Height,
//			ResizeRectSimple_id);
//
//		Render().EndScene();
//	}
//
//	#endif
//
//	// В маленькой текстуре на входе у нас маленькое изображение...
//
//	int From = 0;
//	int To	 = 1;
//
///*	if( BlurPasses < 1 )
//		BlurPasses = 1;
//
//	if( BlurPasses > 128 )
//		BlurPasses = 128;*/
//
////	if( FilterTex )
////		FilterTex->SetTexture(Filter);
//
////	Vector4 va;
////	Vector4 vb;
//	Vector4 vc;
//	Vector4 vd;
//
////	const float t = 4.5f;
////	const float t = 5.5f;
//	const float t = 2.5f;
//
//	const float a = 0.39894228f/t;
//	const float b = 1.0f/(2.0f*t*t);
//
///*	va.x = a*exp(-15.5*15.5*b);
//	va.y = a*exp(-14.5*14.5*b);
//	va.z = a*exp(-13.5*13.5*b);
//	va.w = a*exp(-12.5*12.5*b);
//
//	vb.x = a*exp(-11.5*11.5*b);
//	vb.y = a*exp(-10.5*10.5*b);
//	vb.z = a*exp(- 9.5* 9.5*b);
//	vb.w = a*exp(- 8.5* 8.5*b);*/
//
//	vc.x = a*exp(- 7.5f*7.5f*b);
//	vc.y = a*exp(- 6.5f*6.5f*b);
//	vc.z = a*exp(- 5.5f*5.5f*b);
//	vc.w = a*exp(- 4.5f*4.5f*b);
//
//	vd.x = a*exp(- 3.5f*3.5f*b);
//	vd.y = a*exp(- 2.5f*2.5f*b);
//	vd.z = a*exp(- 1.5f*1.5f*b);
//	vd.w = a*exp(- 0.5f*0.5f*b);
//
///*	if( BlurA )
//		BlurA->SetVector4(va);
//	if( BlurB )
//		BlurB->SetVector4(vb);*/
//	if( BlurC )
//		BlurC->SetVector4(vc);
//	if( BlurD )
//		BlurD->SetVector4(vd);
//
////	for( dword n = 0 ; n < (dword)passCount ; n++ )
//	for( dword n = 0 ; n < 4 ; n++ )
//	{
//		From = n&1;
//		To	 = 0;
//
//		if( From == 0 )
//			To	  = 1;
//
//		if( GlowBlurPass )
//		{
//		/*	float k = (float)n + 1.0f;
//
//			k *= 0.5f;
//
//			if( n&1 )
//				k -= 0.5f;
//
//			GlowBlurPass->SetFloat(k);*/
//			GlowBlurPass->SetFloat(n < 2 ? 0.8f : 1.6f);
//		}
//
//		Render().SetRenderTarget(RTO_DONTOCH_CONTEXT,pChainTex[To]);
//		Render().SetViewport(texViewPort);
//		Render().BeginScene();
//
//		if( n == 0 )
//		{
//			#ifdef _XBOX
//			
//			if( SourceTex )
//				SourceTex->SetTexture(pChainTex[From]->AsTexture());
//			
//			#else
//			
//				if( SourceTex )
//					SourceTex->SetTexture(source);
//
//			#endif
//		}
//		else
//		{
//			if( SourceTex )
//				SourceTex->SetTexture(pChainTex[From]->AsTexture());
//		}
//
//	//	Render().DrawFullScreenQuad(radial ? "IterativeRadialBlur" : "IterativeBlur",
//	//		(float)texViewPort.Width,(float)texViewPort.Height);
//
//	//	#ifndef _XBOX
//
//	//	if( detailLevel == det_low && n == 0 )
//	//	{
//	//		Render().DrawFullScreenQuad(
//	//			(float)texViewPort.Width,
//	//			(float)texViewPort.Height,
//	//			IterativeRadialBlurH_First_Low_id);
//	//	}
//	//	else
//	//	{
//	//		Render().DrawFullScreenQuad(
//	//			(float)texViewPort.Width,
//	//			(float)texViewPort.Height,
//	//			/*IterativeRadialBlur_id*/ n&1 ? IterativeRadialBlurV_id : IterativeRadialBlurH_id);
//	//	}
//
//	//	#else
//
//		Render().DrawFullScreenQuad(
//			(float)texViewPort.Width,
//			(float)texViewPort.Height,
//			/*IterativeRadialBlur_id*/ n&1 ? IterativeRadialBlurV_id : IterativeRadialBlurH_id);
//
//	//	#endif
//
//		Render().EndScene();
//	}
//
//	//------- теперь рисуем DOF  -------------------
////	Render().SetRenderTarget(RTO_DONTOCH_CONTEXT,destination);
//	Render().PopRenderTarget(RTO_DONTOCH_CONTEXT);
//
//	RENDERVIEWPORT vp = Render().GetFullScreenViewPort_3D();
//
//	Render().SetViewport(vp);
//	Render().BeginScene();
//
//	if( SourceTex )
//		SourceTex->SetTexture(source);
//
//	if( DOFOriginal )
//		DOFOriginal->SetTexture(pChainTex[To]->AsTexture());
///*
////	Render().DrawFullScreenQuad("ShowGlow",
//	Render().DrawFullScreenQuad(detailLevel == det_hi ? "ShowGlow" : (radial ? "BlurPaste_Low_Rad" : "BlurPaste_Low"),
//		(float)Render().GetScreenInfo().dwWidth,(float)Render().GetScreenInfo().dwHeight);*/
//
//	Render().DrawFullScreenQuad(
//		(float)Render().GetScreenInfo3D().dwWidth,(float)Render().GetScreenInfo3D().dwHeight,
//		detailLevel == det_hi ? ShowGlow_id : BlurPaste_Low_Rad_id);
//
//	Render().EndScene();
//
//	if( mis_index >= 0 )
//	{
//	//	blurFactor = 0.0f;
//
//		innerFactor = 0.0f;
//		outerFactor = 0.0f;
//
//		count_k = 0.0f;
//	}
//}
//
//void Blur::SetParams(long BlurPasses, float blurFactor, bool radial)
//{
//	Assert(0)
//
//	if( detailLevel == det_off )
//	{
//	//	Enable(false);
//		return;
//	}
//
///*	this->BlurPasses = BlurPasses;
////	this->blurFactor = blurFactor;
//
////	this->radial = radial;
//
//	switch( detailLevel )
//	{
//		case det_low:
//			passCount = BlurPasses/4 + 1;
//			break;
//
//		case det_med:
//			passCount = BlurPasses/2 + 1;
//			break;
//
//		case det_hi:
//			passCount = BlurPasses;
//			break;
//	}*/
//}
//
//void Blur::AddParams(long BlurPasses, float blurFactor, bool radial, float k)
//{
//	if( detailLevel == det_off )
//	{
//	//	Enable(false);
//		return;
//	}
//
//	this->count_k += k;
//
////	this->BlurPasses  = BlurPasses;
///*
////	this->blurFactor += blurFactor;
//	if( this->blurFactor < blurFactor )
//		this->blurFactor = blurFactor;*/
//
//	if( radial )
//	{
//		if( outerFactor < blurFactor )
//			outerFactor = blurFactor;
//	}
//	else
//	{
//		if( innerFactor < blurFactor )
//			innerFactor = blurFactor;
//	}
//
////	this->radial = radial;
//
///*	switch( detailLevel )
//	{
//		case det_low:
//			passCount = BlurPasses/4 + 1;
//			break;
//
//		case det_med:
//			passCount = BlurPasses/2 + 1;
//			break;
//
//		case det_hi:
//			passCount = BlurPasses;
//			break;
//	}*/
////	passCount = BlurPasses;
//}
//
//ITexture* Blur::GetInputTexture()
//{
//	return (ITexture *)pChainTex[0]->AsTexture();
//}
///*
//void Blur::SetViewPort(RENDERVIEWPORT viewPort, Matrix mProj, Matrix mView)
//{
//	vp = viewPort;
//
//	m_Proj = mProj;
//	m_View = mView;
//}
//
//void Blur::SetActiveMissionObject(BlurController *ActiveMissionObject)
//{
//	this->ActiveMissionObject = ActiveMissionObject;
//}
//*/
//void Blur::enable(bool en, IMission &mis)
//{
//	IMission *m = &mis;
//
//	for( int i = 0 ; i < counters ; i++ )
//	{
//		if( counters[i].m == m )
//			break;
//	}
//
//	if( i >= counters )
//	{
//		Counter &c = counters[counters.Add()];
//
//		c.m		  = m;
//		c.counter = 0;
//	}
//
//	int &counter = counters[i].counter;
//
//	if( en )
//	{
//		if( !counter++ )
//			Enable(true);
//	}
//	else
//	{
//		Assert(counter > 0)
//	//	if( counter <= 0 )
//	//		counter  = 1;
//
//		if( !--counter )
//			Enable(false);
//	}
//}
//
//void Blur::update(IMission &mis)
//{
//	IMission *m = &mis;
//
//	for( int i = 0 ; i < counters ; i++ )
//	{
//		Counter &c = counters[i];
//
//		if( c.m == m )
//		{
//			Enable(c.counter > 0); mis_index = i;
//			return;
//		}
//	}
//
//	mis_index = -1;
//
////	Assert(0)
//}
//
////============================================================================================
//
//BlurController::BlurController()
//{
//	pServ = (PostProcessService*)api->GetService("PostProcessService");
//
//	pBlur = pServ->GetBlurFilter();
//	turnedOff = pBlur->turnedOff();
//
//	bFirst  = true;
//	bActive = true;
//
//	fadingIn  = false;
//	fadingOut = false;
//
//	bEnabled = false;
//
//	activated = false;
//
//	m_alpha = 1.0f;
//}
//
//BlurController::~BlurController()
//{
////	if( EditMode_IsOn())
////	{
//	//	PostProcessService* pServ = (PostProcessService*)api->GetService("PostProcessService");
//	//	IBlur *pBlur = pServ->GetBlurFilter();
//
//		if( bEnabled )
//			pBlur->enable(bEnabled = false,Mission());
//
//	//	pBlur->SetActiveMissionObject(NULL);
////	}
//}
//
//void BlurController::Restart()
//{
//	if( bEnabled )
//		pBlur->enable(bEnabled = false,Mission());
//
//	bFirst  = true;
//	bActive = true;
//
//	fadingIn  = false;
//	fadingOut = false;
//
//	activated = false;
//
//	MissionObject::Activate(false);
//
//	ReCreate();
//}
//
////Инициализировать объект
//bool BlurController::Create(MOPReader &reader)
//{
//	if( turnedOff )
//		return true;
//
//	SetUpdate(&BlurController::FlushPostProcess,ML_POSTEFFECTS);
//
//	viewPort = Render().GetFullScreenViewPort_3D();
//
////	SetUpdate(&BlurController::UpdateViewPort  ,ML_GEOMETRY1);
//
//	if( !MissionObject::Create(reader))
//		return false;
//
////	BlurPasses = reader.Long();
//	BlurPasses = 4;
//
//	int pn = reader.Long();
//
//	blurFactor = reader.Float();
//
////	if( pn )
////		blurFactor *= pn/20.0f;
//	if( pn )
//	//	blurFactor *= 0.1f + 0.9f*pn/20.0f;
//		blurFactor *= pn/20.0f;
//
//	fadeIn  = reader.Float();
//	fadeOut = reader.Float();
//
//	radial = reader.Bool();
//
//	m_alpha = 1.0f;
//
//	Activate(reader.Bool());
//
//	return true;
//}
//
////Инициализировать объект
//bool BlurController::EditMode_Create(MOPReader &reader)
//{
//	Create(reader);
//
//	return true;
//}
//
////Обновить параметры
//bool BlurController::EditMode_Update(MOPReader &reader)
//{
//	Create(reader);
//
//	return true;
//}
//
//void BlurController::Command(const char *id, dword numParams, const char **params)
//{
//	if( string::IsEmpty(id))
//		return;
//
//	if( string::IsEqual(id,"setfxa"))
//	{
//		if( numParams < 1 )
//		{
//			LogicDebugError("Command BlurController::[setfxa] error. No alpha specified.");
//			return;
//		}
//
//		float alpha = (float)atof(params[0]);
//
//		UpdateAlpha(alpha);
//	}
//}
//
//void BlurController::Activate(bool isActive)
//{
//	if( turnedOff )
//		return;
//
////	PostProcessService* pServ = (PostProcessService*)api->GetService("PostProcessService");
////	IBlur *pBlur = pServ->GetBlurFilter();
//
//	pBlur->update(Mission());
//
//	if( bFirst )
//		bFirst = false;
//	else
//	{
//	/*	if( isActive == bActive )
//		{
//			pBlur->SetParams(BlurPasses,blurFactor,radial);
//			return;
//		}*/
//	}
//
//	bActive = isActive;
//
//	if( bActive && IsVisible())
//	{
//		if( !bEnabled || fadingOut )
//		{
//			if( !bEnabled )
//				pBlur->enable(bEnabled = true,Mission());
//
//			if( !fadingIn )
//			{
//				if( fadeIn > 0.0f )
//				{
//					if( fadingOut )
//						time = (1.0f - time/fadeOut)*fadeIn;
//					else
//						time = 0.0f;
//
//					fadingIn  = true;
//					fadingOut = false;
//
//					float k = time/fadeIn;
//
//					pBlur->AddParams(BlurPasses,m_alpha*blurFactor*k,radial,k);
//					activated = true;
//				}
//				else
//				{
//					pBlur->AddParams(BlurPasses,m_alpha*blurFactor,radial,1.0);
//					activated = true;
//
//					fadingIn  = false;
//					fadingOut = false;
//				}
//			}
//		}
//	}
//	else
//	{
//		if( bEnabled )
//		{
//			if( !fadingOut )
//			{
//				if( fadeOut > 0.0f )
//				{
//					if( fadingIn )
//						time = (1.0f - time/fadeIn)*fadeOut;
//					else
//						time = 0.0f;
//
//					fadingOut = true;
//					fadingIn  = false;
//				}
//				else
//				{
//					pBlur->enable(bEnabled = false,Mission());
//
//					fadingIn  = false;
//					fadingOut = false;
//				}
//			}
//		}
//	}
//
//	MissionObject::Activate(isActive);
//}
///*
//void _cdecl BlurController::UpdateViewPort(float dltTime, long level)
//{
//	viewPort = Render().GetViewport();
//
//	Matrix mProj = Render().GetProjection();
//	Matrix mView = Render().GetView();
//
//	PostProcessService* pServ = (PostProcessService*)api->GetService("PostProcessService");
//	IBlur *pBlur = pServ->GetBlurFilter();
//
//	if( pBlur )
//	{
//		pBlur->SetViewPort(viewPort, mProj, mView);
//		pBlur->SetActiveMissionObject(this);
//	}
//};
//*/
//void _cdecl BlurController::FlushPostProcess(float dltTime, long level)
//{
////	PostProcessService* pServ = (PostProcessService*)api->GetService("PostProcessService");
////	IBlur *pBlur = pServ->GetBlurFilter();
//
//	pBlur->update(Mission());
//
//	time += dltTime;
//
//	Assert(!fadingIn || !fadingOut)
//
//	bool isVisible = IsVisible();
//
//	if( fadingIn )
//	{
//		if( time > fadeIn )
//		{
//			time = fadeIn; fadingIn = false;
//		}
//
//		float k = time/fadeIn;
//
//		if( !activated )
//		{
//			pBlur->AddParams(BlurPasses,m_alpha*blurFactor*k,radial,k);
//		}
//	}
//	else
//	if( fadingOut )
//	{
//		if( time > fadeOut )
//		{
//			time = fadeOut; fadingOut = false;
//
//			pBlur->enable(bEnabled = false,Mission());
//		}
//
//		float k = 1.0f - time/fadeOut;
//
//		pBlur->AddParams(BlurPasses,m_alpha*blurFactor*k,radial,k);
//	}
//	else
//	if( IsActive() && IsVisible())
//	{
//		if( !activated )
//		{
//			pBlur->AddParams(BlurPasses,m_alpha*blurFactor,radial,1.0f);
//		}
//	}
//
//	activated = false;
//
////	Render().PostProcess();
//
///*	if( EditMode_IsOn())
//	{
//		if( pBlur->IsEnabled() != bEnabled )
//			pBlur->Enable(bEnabled);
//	}*/
//
//	if( pBlur->IsEnabled() && IsActive())
//		pServ->FilterNotify("Blur",GetObjectID().c_str(),/*11.0f*/13.0f,m_alpha*blurFactor);
//}
//
//void BlurController::UpdateAlpha(float alpha)
//{
//	m_alpha = alpha;
//
//	if( IsVisible())
//	{
//		if( IsActive())
//		{
//			if( !bEnabled )
//				pBlur->enable(bEnabled = true,Mission());
//		}
//	}
//	else
//	{
//		if( bEnabled )
//			pBlur->enable(bEnabled = false,Mission());
//	}
//}

//============================================================================================
//Параметры инициализации
//============================================================================================

class BlurController : public MissionObject
{
};

MOP_BEGINLISTCG(BlurController, "Blur controller", '1.00', 0, "Blur controller", "Post effects")

//	MOP_LONGEX("Blur passes", 6, 1, 20);
	MOP_LONGEX("Blur passes", 0, 0, 20);

	MOP_FLOATEX("Blur factor", 1.0f, 0.0f, 1.0f);

	MOP_FLOATEX("Fade in  time", 0.0f, 0.0f, 100.0f);
	MOP_FLOATEX("Fade out time", 0.0f, 0.0f, 100.0f);

	MOP_BOOL("Radial", false);

	MOP_BOOL("Active", true);

MOP_ENDLIST(BlurController)
