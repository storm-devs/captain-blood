#include "MotionEffect.h"
#include "..\PostProcess.h"

//MotionEffect:: MotionEffect()
//{
//	started = true;
//
//	pChainTex[0] = null;
//	pChainTex[1] = null;
//
//	depth_target = null;
//
//	pChain = null;
//
//	MotionFactor = null;
//	MotionMatrix = null;
//
//	FirstMotion = null;
//	MotionScale = null;
//
//	pScreen = null;
//
//	SourceTex = null;
//
//	Screen = null;
//
//	GlowBlurPass = null;
//
//	BlurFactor = null;
//
//	OneDivScreenWidth  = null;
//	OneDivScreenHeight = null;
//
//	blurPasses = 10;
//	blurFactor = 1.0f;
//
//	motionFactor = 10.0f;
//	motionScale	 =  5.0f;
//
//	cameraFactor = 10.0f;
//
//	divider = 1;
//
////	detailLevel = det_hi;
//	detailLevel = det_off;
//
//	pServ = null;
//}
//
//MotionEffect::~MotionEffect()
//{
//	if( pChainTex[0] )
//		pChainTex[0]->Release();
//	if( pChainTex[1] )
//		pChainTex[1]->Release();
//
//	if( pChain )
//		pChain->Release();
//
//	MotionFactor = NULL;
//	MotionMatrix = NULL;
//	MotionScale = NULL;
//	FirstMotion = NULL;
//
//
//	if( pScreen )
//		pScreen->Release();
//
//	if( depth_target )
//	{
//		depth_target->Release();
//		depth_target = null;
//	}
//
//	Screen = NULL;
//	GlowBlurPass = NULL;
//	SourceTex = NULL;
//	BlurFactor = NULL;
//	OneDivScreenWidth = NULL;
//	OneDivScreenHeight = NULL;
//}
//
//void MotionEffect::UserInit()
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
//		const char *s = ini->GetString("PostEffects","MotionEffect","med");
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
//	texViewPort.X = 0;
//	texViewPort.Y = 0;
//
//	texViewPort.Width  = Render().GetScreenInfo3D().dwWidth /divider;
//	texViewPort.Height = Render().GetScreenInfo3D().dwHeight/divider;
//
//	texViewPort.MinZ = 0.0f;
//	texViewPort.MaxZ = 1.0f;
//
////	#ifdef _XBOX
//
//	depth_target = Render().CreateTempDepthStencil(TRS_SCREEN_FULL_3D,_FL_);
//
////	#endif
//
//	pScreen	= Render().CreateTempRenderTarget(TRS_SCREEN_FULL_3D,TRC_FIXED_RGBA_8,_FL_,0);
//
//	switch( detailLevel )
//	{
//		case det_low:
//			pChainTex[0] = Render().CreateTempRenderTarget(TRS_SCREEN_QUARTER_3D,TRC_FIXED_RGBA_8,_FL_,0);
//			pChainTex[1] = Render().CreateTempRenderTarget(TRS_SCREEN_QUARTER_3D,TRC_FIXED_RGBA_8,_FL_,1);
//			pChain		 = Render().CreateTempRenderTarget(TRS_SCREEN_QUARTER_3D,TRC_FIXED_RGBA_8,_FL_,2);
//			break;
//
//		case det_med:
//			pChainTex[0] = Render().CreateTempRenderTarget(TRS_SCREEN_HALF_3D	 ,TRC_FIXED_RGBA_8,_FL_,0);
//			pChainTex[1] = Render().CreateTempRenderTarget(TRS_SCREEN_HALF_3D	 ,TRC_FIXED_RGBA_8,_FL_,1);
//			pChain		 = Render().CreateTempRenderTarget(TRS_SCREEN_HALF_3D	 ,TRC_FIXED_RGBA_8,_FL_,2);
//			break;
//
//		case det_hi:
//			pChainTex[0] = Render().CreateTempRenderTarget(TRS_SCREEN_FULL_3D	 ,TRC_FIXED_RGBA_8,_FL_,1);
//			pChainTex[1] = Render().CreateTempRenderTarget(TRS_SCREEN_FULL_3D	 ,TRC_FIXED_RGBA_8,_FL_,2);
//			pChain		 = Render().CreateTempRenderTarget(TRS_SCREEN_FULL_3D	 ,TRC_FIXED_RGBA_8,_FL_,3);
//			break;
//	}
//
//	MotionMatrix = Render().GetTechniqueGlobalVariable("MotionMatrix",_FL_);
//	MotionFactor = Render().GetTechniqueGlobalVariable("MotionFactor",_FL_);
//
//	FirstMotion = Render().GetTechniqueGlobalVariable("FirstMotion",_FL_);
//	MotionScale = Render().GetTechniqueGlobalVariable("MotionScale",_FL_);
//
//	SourceTex = Render().GetTechniqueGlobalVariable("PP_CommonTexture",_FL_);
//
//	Screen = Render().GetTechniqueGlobalVariable("MotionTexture",_FL_);
//
//	GlowBlurPass = Render().GetTechniqueGlobalVariable("GlowBlurPass",_FL_);
//
//	BlurFactor = Render().GetTechniqueGlobalVariable("BlurFactor",_FL_);
//
//	OneDivScreenWidth  = Render().GetTechniqueGlobalVariable("TexelSizeX",_FL_);
//	OneDivScreenHeight = Render().GetTechniqueGlobalVariable("TexelSizeY",_FL_);
//
//	blurPasses = 10;
//	blurFactor = 1.0f;
//
//	passCount = blurPasses;
//
//	Render().GetShaderId("MotionBlurCopy_Z_Dir", MotionBlurCopy_Z_Dir_id);
//	Render().GetShaderId("MotionEffectPaste", MotionEffectPaste_id);
//	Render().GetShaderId("CalculateDir", CalculateDir_id);
//	Render().GetShaderId("IterativeBlurDir", IterativeBlurDir_id);
//	Render().GetShaderId("MotionBlurCopy", MotionBlurCopy_id);
//
//	pServ = (PostProcessService *)api->GetService("PostProcessService");
//}
//
//void MotionEffect::FilterImage(IBaseTexture *source, IRenderTarget *destination)
//{
////	Assert(detailLevel > det_off)
//	return;
//	if( detailLevel < det_low )
//		return;
//
//	if( !pServ->Enabled())
//		return;
//
//	Render().PushRenderTarget();
//
//	int From = 0;
//	int	To	 = 1;
//
//	#ifndef _XBOX
//
//	IRenderTargetDepth *depth = Render().GetDepthStencil(_FL_);
//
//	#endif
//
//	{
//		RENDERVIEWPORT wp = texViewPort;
//
//		wp.Width  *= divider;
//		wp.Height *= divider;
//
//		#ifdef _XBOX
//
//		Render().BeginScene();
//
//		Render().ResolveDepth(depth_target);
//
//		Render().EndScene(null,true,true);
//
//		Render().SetRenderTarget(RTO_RESTORE_CONTEXT,pScreen,depth_target);
//
//		#else
//
//		Render().BeginScene();
//		Render().EndScene(null,true,true);
//		Render().ResolveDepth(depth_target);
//
//		Render().SetRenderTarget(RTO_RESTORE_CONTEXT,pScreen,depth_target);
//
//		#endif
//
//		Render().SetViewport(wp);
//		Render().BeginScene();
//
//		if( Screen )
//			Screen->SetTexture(source);
//
//		Render().DrawFullScreenQuad((float)wp.Width,(float)wp.Height, MotionBlurCopy_id);
//
//	//	Render().EndScene();
//	}
//
///*	for( dword n = 0 ; n < passCount ; n++ )
//	{
//		From = n&1;
//		To	 = 0;
//
//		if( From == 0 )
//			To	  = 1;
//
//		if( GlowBlurPass )
//		{
//			float k = (float)n + 1.0f;
//
//			k *= 0.5f;
//
//			GlowBlurPass->SetFloat(k);
//		}
//
//		Render().SetRenderTarget(RTO_DONTOCH_CONTEXT,pChainTex[To]);
//		Render().SetViewport(texViewPort);
//		Render().BeginScene();
//
//		if( n == 0 )
//		{
//			if( SourceTex )
//				SourceTex->SetTexture(source);
//		}
//		else
//		{
//			if( SourceTex )
//				SourceTex->SetTexture(pChainTex[From]->AsTexture());
//		}
//
//		Render().DrawFullScreenQuad("IterativeBlur",
//			(float)texViewPort.Width,(float)texViewPort.Height);
//
//		Render().EndScene();
//	}
//
//	Render().PopRenderTarget(RTO_RESTORE_CONTEXT);
//	Render().BeginScene();*/
//
//	//// дальний блюр ////
//
////	float focus = distance*0.50f;
//
////	float focus = distance*0.45f;
////	float focus = distance*0.35f;
//
////	int pn = 6;
//	int pn = 0;
//
//	const float r = 1.0f;
//
////	float d = distance + focus*(pn);
//
////	float d = distance + focus*(pn + 1);
//
////	float d = 10.000f;
//	float d =  0.001f;
////	float d =  5.000f;
//
////	float a[] = {0.4f,0.35f,0.35f,0.4f,0.6f,1.0f};
//
////	float a[] = {0.0f,0.3f,0.25f,0.35f,0.4f,0.6f,1.0f};
//	float a[] = {0.15f,0.3f,0.45f,0.6f,0.75f,0.9f,1.0f};
//
////	pn--;
//	pn++;
//
///*	float buf[] = {
//		-r,-r, d, 1.0f,  0.0f,1.0f,  a[pn],
//		-r, r, d, 1.0f,  0.0f,0.0f,  a[pn],
//		 r,-r, d, 1.0f,  1.0f,1.0f,  a[pn],
//		 r, r, d, 1.0f,  1.0f,0.0f,  a[pn]};*/
//
//	const RENDERVIEWPORT &wp = Render().GetViewport();
//
//	float kx = 1.0f/(texViewPort.Width *divider);
//	float ky = 1.0f/(texViewPort.Height*divider);
//
//	float tl = wp.X*kx; float tr = (wp.X + wp.Width )*kx;
//	float tt = wp.Y*ky; float tb = (wp.Y + wp.Height)*ky;
//
///*	float buf[] = {
//		-r,-r, d, 1.0f,  tl,tb,  a[pn],
//		-r, r, d, 1.0f,  tl,tt,  a[pn],
//		 r,-r, d, 1.0f,  tr,tb,  a[pn],
//		 r, r, d, 1.0f,  tr,tt,  a[pn]};*/
//
//	float dx = kx;
//	float dy = ky;
//
//	float buf[] = {
//		-r - dx,-r + dy, d, 1.0f,  tl,tb,  d,
//		-r - dx, r + dy, d, 1.0f,  tl,tt,  d,
//		 r - dx,-r + dy, d, 1.0f,  tr,tb,  d,
//		 r - dx, r + dy, d, 1.0f,  tr,tt,  d};
//
////	if( Screen )
////		Screen->SetTexture(pChainTex[To]->AsTexture());
//
//	if( SourceTex )
//	//	SourceTex->SetTexture(pScreen->AsTexture());
//		SourceTex->SetTexture(source);
//
///*	float bbb[] = {
//		-r,-r, 0.0f, 1.0f,  tl,tb,  0.0f,
//		-r, r, 0.0f, 1.0f,  tl,tt,  0.0f,
//		 r,-r, 0.0f, 1.0f,  tr,tb,  0.0f,
//		 r, r, 0.0f, 1.0f,  tr,tt,  0.0f};
//
//	Render().DrawPrimitiveUP(PT_TRIANGLESTRIP,2,bbb,sizeof(Vertex),"MotionBlurCopy_Z");*/
//
////	if( back > 0.0f )
//	{
//		
//		Render().DrawPrimitiveUP(MotionBlurCopy_Z_Dir_id, PT_TRIANGLESTRIP,2,buf,sizeof(Vertex));
//
//	//	while( pn-- )
//
//	//	while( pn++ <  6 )
//	//	while( pn++ < 20 )
//		while( pn++ < 15 )
//		{
//		//	focus *= 1.1f;
//		//	focus *= 1.2f;
//
//		//	d = distance + focus*pn;
//
//		//	d += focus;
//			d += 5.0f;
//
//			buf[ 2] = d;
//			buf[ 9] = d;
//			buf[16] = d;
//			buf[23] = d;
//
//		//	float al = a[pn]*back;
//			float al = d;
//
//			buf[ 6] = al;
//			buf[13] = al;
//			buf[20] = al;
//			buf[27] = al;
//
//			Render().DrawPrimitiveUP(MotionBlurCopy_Z_Dir_id, PT_TRIANGLESTRIP,2,buf,sizeof(Vertex));
//		}
//	}
//
//	//// ближний блюр ////
//
///*	if( fore > 0.0f )
//	{
//		pn = 4;
//
//	//	focus = distance*0.75f;
//		focus = distance*0.80f;
//
//	//	float dz = distance*0.25f;
//		float dz = distance*0.20f;
//
//	//	float b[] = {1.0f,0.60f,0.4f,0.3f};
//
//	//	float b[] = {1.0f,0.6f,0.4f,0.3f};
//		float b[] = {1.0f,0.75f,0.5f,0.25f};
//
//		while( pn-- )
//		{
//			d = focus;
//
//			buf[ 2] = d;
//			buf[ 9] = d;
//			buf[16] = d;
//			buf[23] = d;
//
//		//	float al = b[pn]*fore;
//			float al = d;
//
//			buf[ 6] = al;
//			buf[13] = al;
//			buf[20] = al;
//			buf[27] = al;
//
//			Render().DrawPrimitiveUP(PT_TRIANGLESTRIP,2,buf,sizeof(Vertex),"MotionBlurCopy_Z_Inv");
//
//			focus -= dz;
//
//		//	dz *= 0.7f;
//		}
//	}*/
//
////	;
//	Render().EndScene();
//
//	Matrix md = Render().GetView();
//
//	if( started )
//	{
//		prevMat = md;
//		started = false;
//	}
//
//	Matrix mi = md;
//		   mi.Inverse();
//
////	Matrix pi = Render().GetProjection();
////		   pi.Inverse();
//
////	Matrix mm = pi*mi*prevMat*Render().GetProjection();
//	Matrix mm =    mi*prevMat;
//
//	{
//		Render().SetRenderTarget(RTO_DONTOCH_CONTEXT,pChain);
//		Render().SetViewport(texViewPort);
//		Render().BeginScene();
//
//		if( SourceTex )
//			SourceTex->SetTexture(pScreen->AsTexture());
//
//		MotionMatrix->SetMatrix(mm);
//
//		if( MotionFactor )
//			MotionFactor->SetFloat(motionFactor);
//
//		Render().DrawFullScreenQuad((float)texViewPort.Width,(float)texViewPort.Height, CalculateDir_id);
//
//		Render().EndScene();
//	}
//
//	if( BlurFactor )
//		BlurFactor->SetFloat(blurFactor);
//
//	if( MotionScale )
//		MotionScale->SetFloat(motionScale);
//
//	if( OneDivScreenWidth )
//		OneDivScreenWidth ->SetFloat((1.0f/texViewPort.Width )*0.5f);
//
//	if( OneDivScreenHeight )
//		OneDivScreenHeight->SetFloat((1.0f/texViewPort.Height)*0.5f);
//
//	if( Screen )
//		Screen->SetTexture(pChain->AsTexture());
//
//	for( dword n = 0 ; n < passCount ; n++ )
//	{
//		From = n&1;
//		To	 = 0;
//
//		if( From == 0 )
//			To	  = 1;
//
//		if( GlowBlurPass )
//		{
//			float k = (float)n + 1.0f;
//
//			k *= 0.5f;
//
//			GlowBlurPass->SetFloat(k);
//		}
//
//		Render().SetRenderTarget(RTO_DONTOCH_CONTEXT,pChainTex[To]);
//		Render().SetViewport(texViewPort);
//		Render().BeginScene();
//
//		if( n == 0 )
//		{
//			if( SourceTex )
//				SourceTex->SetTexture(pScreen->AsTexture());
//
//			if( FirstMotion )
//				FirstMotion->SetFloat(1.0f);
//		}
//		else
//		{
//			if( SourceTex )
//				SourceTex->SetTexture(pChainTex[From]->AsTexture());
//
//			if( FirstMotion )
//				FirstMotion->SetFloat(0.0f);
//		}
//
//		Render().DrawFullScreenQuad((float)texViewPort.Width,(float)texViewPort.Height, IterativeBlurDir_id);
//
//		Render().EndScene();
//	}
//
///*	if( back > 0.0f || fore > 0.0f )
//	{
//		RENDERVIEWPORT wp = texViewPort;
//
//		wp.Width  *= divider;
//		wp.Height *= divider;
//
//		Render().SetRenderTarget(RTO_RESTORE_CONTEXT,pScreen);
//		Render().SetViewport(wp);
//		Render().BeginScene();
//
//		if( Screen )
//			Screen->SetTexture(pChainTex[To]->AsTexture());
//
//		Render().DrawFullScreenQuad("MotionBlurPaste",
//			(float)wp.Width,(float)wp.Height);
//
//		Render().EndScene();
//	}*/
//
//	Render().PopRenderTarget(RTO_RESTORE_CONTEXT);
//	Render().BeginScene();
//
////	if( back > 0.0f || fore > 0.0f )
//	{
//		if( Screen )
//			Screen->SetTexture(pScreen->AsTexture());
//
//		if( SourceTex )
//			SourceTex->SetTexture(pChainTex[To]->AsTexture());
//
//	//	Render().DrawFullScreenQuad("MotionBlurCopy",
//	//		(float)wp.Width,(float)wp.Height);
//
//		const RENDERVIEWPORT &wp = Render().GetViewport();
//
//		float kx = 1.0f/Render().GetFullScreenViewPort_3D().Width;
//		float ky = 1.0f/Render().GetFullScreenViewPort_3D().Height;
//
//		float tl = wp.X*kx; float tr = (wp.X + wp.Width )*kx;
//		float tt = wp.Y*ky; float tb = (wp.Y + wp.Height)*ky;
//
//		const float r = 1.0f;
//		const float d = 0.0f;
//
//		float dx = kx;
//		float dy = ky;
//
//		float buf[] = {
//			-r - dx,-r + dy, d, 1.0f,  tl,tb,
//			-r - dx, r + dy, d, 1.0f,  tl,tt,
//			 r - dx,-r + dy, d, 1.0f,  tr,tb,
//			 r - dx, r + dy, d, 1.0f,  tr,tt};
//
//		MotionMatrix->SetMatrix(mm);
//
//		if( MotionFactor )
//			MotionFactor->SetFloat(motionFactor);
//
//		Render().DrawPrimitiveUP(MotionEffectPaste_id, PT_TRIANGLESTRIP,2,buf,sizeof(float)*6);
//	}
//
//	Render().EndScene();
//
////	prevMat = md;
//
////	float k = 0.25f;
////	float k = 0.15f;
////	float k = 0.50f;
//
//	float k = api->GetDeltaTime()*cameraFactor;
//
//	if( k > 1.0f )
//		k = 1.0f;
//
//	if((md.pos - prevMat.pos).GetLength() > 1.0 )
//		k = 1.0f;
//
////	prevMat.vx.Lerp(prevMat.vx,md.vx,k); prevMat.wx = Lerp(prevMat.wx,md.wx,k);
//	prevMat.vy.Lerp(prevMat.vy,md.vy,k); prevMat.wy = Lerp(prevMat.wy,md.wy,k);
//	prevMat.vz.Lerp(prevMat.vz,md.vz,k); prevMat.wz = Lerp(prevMat.wz,md.wz,k);
//
//	prevMat.vx = prevMat.vy^prevMat.vz;
//	prevMat.vy = prevMat.vz^prevMat.vx;
//
//	prevMat.pos.Lerp(prevMat.pos,md.pos,k); prevMat.w = Lerp(prevMat.w,md.w,k);
//
//	#ifndef _XBOX
//
//	if( depth )
//		depth->Release();
//
//	#endif
//}
//
//ITexture* MotionEffect::GetInputTexture()
//{
//	return Render().GetPostprocessTexture();
//}
//
//void MotionEffect::SetParams(dword blurPasses, float blurFactor, float motionFactor, float motionScale,
//	float cameraFactor)
//{
//	if( detailLevel == det_off )
//	{
//	//	Enable(false);
//		return;
//	}
//
//	this->blurPasses = blurPasses;
//	this->blurFactor = blurFactor;
//
//	switch( detailLevel )
//	{
//		case det_low:
//			passCount = blurPasses/4 + 1;
//			break;
//
//		case det_med:
//			passCount = blurPasses/2 + 1;
//			break;
//
//		case det_hi:
//			passCount = blurPasses;
//			break;
//	}
//
//	this->motionFactor = motionFactor;
//	this->motionScale  = motionScale;
//
//	this->cameraFactor = cameraFactor;
//
////	started = true;
//}
//
//MotionEffectController:: MotionEffectController()
//{
//	bActive	= true; bEnabled = false;
//
//	pServ = (PostProcessService *)api->GetService("PostProcessService");
//
//	pEffect = pServ->GetMotionEffectFilter();
//	turnedOff = pEffect->turnedOff();
//
//	m_alpha = 1.0f;
//}
//
//MotionEffectController::~MotionEffectController()
//{
////	if( EditMode_IsOn())
////	{
//	//	PostProcessService *pServ = (PostProcessService *)api->GetService("PostProcessService");
//	//	MotionEffect *pEffect = pServ->GetMotionEffectFilter();
//
//		pEffect->Enable(bEnabled = false);
////	}
//}
//
//void MotionEffectController::Restart()
//{
//	if( bEnabled )
//		pEffect->Enable(bEnabled = false);
//
//	MissionObject::Activate(false);
//
//	ReCreate();
//}
//
//bool MotionEffectController::Create(MOPReader &reader)
//{
//	if( turnedOff )
//		return true;
//
//	SetUpdate(&MotionEffectController::Draw,ML_POSTEFFECTS);
//
//	InitParams(reader);
//
//	return true;
//}
//
//bool MotionEffectController::EditMode_Update(MOPReader &reader)
//{
//	if( turnedOff )
//		return true;
//
//	InitParams(reader);
//
//	return true;
//}
//
//void MotionEffectController::Activate(bool isActive)
//{
//	if( turnedOff )
//		return;
//
//	bActive = isActive;
//
////	PostProcessService *pServ = (PostProcessService *)api->GetService("PostProcessService");
////	MotionEffect *pEffect = pServ->GetMotionEffectFilter();
//
//	if( bActive && IsVisible())
//	{
//		pEffect->Enable(bEnabled = true);
//		pEffect->SetParams(blurPasses,m_alpha*blurFactor,motionFactor,motionScale,cameraFactor);
//
//		pEffect->Reset();
//	}
//	else
//	{
//		pEffect->Enable(bEnabled = false);
//	}
//
//	MissionObject::Activate(isActive);
//}
//
//void MotionEffectController::Command(const char *id, dword numParams, const char **params)
//{
//	if( string::IsEmpty(id))
//		return;
//
//	if( string::IsEqual(id,"setfxa"))
//	{
//		if( numParams < 1 )
//		{
//			LogicDebugError("Command MotionEffectController::[setfxa] error. No alpha specified.");
//			return;
//		}
//
//		float alpha = (float)atof(params[0]);
//
//		UpdateAlpha(alpha);
//	}
//}
//
//void _cdecl MotionEffectController::Draw(float dltTime, long level)
//{
//	return;
//	Render().PostProcess();
//
////	PostProcessService *pServ = (PostProcessService *)api->GetService("PostProcessService");
////	MotionEffect *pEffect = pServ->GetMotionEffectFilter();
//
//	if( EditMode_IsOn() && IsActive())
//	{
//	//	PostProcessService *pServ = (PostProcessService *)api->GetService("PostProcessService");
//	//	MotionEffect *pEffect = pServ->GetMotionEffectFilter();
//
//		if( pEffect->IsEnabled() != bEnabled )
//		{
//			pEffect->Enable(bEnabled);
//			pEffect->SetParams(blurPasses,m_alpha*blurFactor,motionFactor,motionScale,cameraFactor);
//			pEffect->Reset();
//		}
//	}
//
//	if( bEnabled )
//		pEffect->SetParams(blurPasses,m_alpha*blurFactor,motionFactor,motionScale,cameraFactor);
//
//	if( pEffect->IsEnabled() && IsActive())
//		pServ->FilterNotify("MotionEffect",GetObjectID().c_str(),53.0f,m_alpha*blurFactor);
//}
//
//void MotionEffectController::InitParams(MOPReader &reader)
//{
//	blurPasses = reader.Long();
//	blurFactor = reader.Float();
//
//	motionFactor = reader.Float();
//	motionScale	 = reader.Float();
//
//	if( motionFactor > 50.0f )
//		motionFactor = 50.0f;
//
//	cameraFactor = reader.Float();
//
//	if( cameraFactor < 10.0f )
//		cameraFactor = 10.0f;
//
//	m_alpha = 1.0f;
//
//	Activate(reader.Bool());
//}
//
//void MotionEffectController::UpdateAlpha(float alpha)
//{
//	m_alpha = alpha;
//
//	if( IsVisible())
//	{
//		if( IsActive())
//		{
//		//	if( !bEnabled )
//				pEffect->Enable(bEnabled = true);
//				pEffect->SetParams(blurPasses,m_alpha*blurFactor,motionFactor,motionScale,cameraFactor);
//
//				pEffect->Reset();
//		}
//	}
//	else
//	{
//	//	if( bEnabled )
//			pEffect->Enable(bEnabled = false);
//	}
//}

class MotionEffectController : public MissionObject
{
public:
	MotionEffectController() {}
	virtual ~MotionEffectController() {}
};

MOP_BEGINLISTCG(MotionEffectController, "MotionEffect", '1.00', 0, "MotionEffect", "Post effects")

	MOP_LONGEX("Blur passes", 10, 1, 20);

	MOP_FLOATEX("Blur factor", 1.0f, 0.0f, 1.0f);

	MOP_FLOATC("Motion factor", 10.0f, "Коэффициент скорости точек");
	MOP_FLOATC("Motion scale" ,  5.0f, "Коэффициент разброса точек");

	MOP_FLOATC("Camera factor", 10.0f, "Скорость обновления");

	MOP_BOOL("Active", true);

MOP_ENDLIST(MotionEffectController)
