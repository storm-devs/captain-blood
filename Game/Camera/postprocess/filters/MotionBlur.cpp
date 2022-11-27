#include "..\PostProcess.h"

//MotionBlur:: MotionBlur()
//{
//	SourceTex = NULL;
//
//	varAccumulator = NULL;
//	varTexture	   = NULL;
//
////	BlendType	= NULL;
//	BlendFactor = NULL;
//
//	pAccumulator = NULL;
//	pTexture	 = NULL;
//
//	bFirstTime = true;
//
//	GrabTime = 0;
//	fPassedTime = 999999.0;
//
//	fOriginal	= /*0.5f*/0.05f;
//	fAfterImage = /*0.5f*/0.50f;
//
////	BlurOriginalK = NULL;
////	AfterImageK	  = NULL;
//
////	motionBlur = false;
//
////	fTime = 0.0f;
//
//	detailLevel = det_hi;
//}
//
//MotionBlur::~MotionBlur()
//{
//	SourceTex = NULL;
//
///*	if( BlendType )
//	{
//		BlendType->Release();
//		BlendType = NULL;
//	}*/
//
//	BlendFactor = NULL;
//
//	if( pAccumulator )
//	{
//		pAccumulator->Release();
//		pAccumulator = NULL;
//	}
//
//	if( pTexture )
//	{
//		pTexture->Release();
//		pTexture = NULL;
//	}
//
//	varAccumulator = NULL;
//	varTexture = NULL;
//
///*	if( BlurOriginalK )
//	{
//		BlurOriginalK->Release();
//		BlurOriginalK = NULL;
//	}
//
//	if( AfterImageK )
//	{
//		AfterImageK->Release();
//		AfterImageK = NULL;
//	}*/
//}
//
//void MotionBlur::UserInit()
//{
//	dword divider = 1;
//
//	#ifndef _XBOX
//
//	IFileService *storage = (IFileService *)api->GetService("FileService");
//	Assert(storage)
//
//	IIniFile *ini = storage->SystemIni();//storage->OpenIniFile(api->GetVarString("Ini"),_FL_);
//
//	if( ini )
//	{
//		const char *s = ini->GetString("PostEffects","MotionBlur","hi");
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
////	FIX ME!!!
////	SourceTex = Render().GetSourceTextureVariable();
//	SourceTex = Render().GetTechniqueGlobalVariable("PP_ScreenTexture",_FL_);
//
////	BlendType	= Render().GetTechniqueGlobalVariable("MotionBlendType"	 ,_FL_);
//	BlendFactor = Render().GetTechniqueGlobalVariable("MotionBlendFactor",_FL_);
//
////	Render().GetScreenInfo().BackBufferFormat
////	FMT_A32B32G32R32F
//
//	DWORD dwWidth  = Render().GetScreenInfo3D().dwWidth;
//	DWORD dwHeight = Render().GetScreenInfo3D().dwHeight;
//
//	RENDERFORMAT TexFormat = Render().GetScreenInfo3D().BackBufferFormat;
//
//	texViewPort.X = 0;
//	texViewPort.Y = 0;
//
////	texViewPort.Width  = dwWidth;
////	texViewPort.Height = dwHeight;
//	texViewPort.Width  = dwWidth /divider;
//	texViewPort.Height = dwHeight/divider;
//
//	texViewPort.MinZ = 0.0f;
//	texViewPort.MaxZ = 1.0f;
//	
////	pAccumulator = Render().CreateTexture(dwWidth,dwHeight,1,USAGE_RENDERTARGET,TexFormat,_FL_,POOL_DEFAULT);
//
////	pAccumulator = Render().CreateRenderTarget(dwWidth		  ,dwHeight		   ,_FL_,TexFormat);
//	pAccumulator = Render().CreateRenderTarget(dwWidth/divider,dwHeight/divider,_FL_,TexFormat);
//
////	pTexture = Render().CreateRenderTarget(dwWidth,dwHeight,_FL_,TexFormat);
//
//	switch( detailLevel )
//	{
//		case det_low:
//			pTexture = Render().CreateTempRenderTarget(TRS_SCREEN_QUARTER_3D,TRC_FIXED_RGBA_8,_FL_,0);
//			break;
//
//		case det_med:
//			pTexture = Render().CreateTempRenderTarget(TRS_SCREEN_HALF_3D	 ,TRC_FIXED_RGBA_8,_FL_,0);
//			break;
//
//		case det_hi:
//			pTexture = Render().CreateTempRenderTarget(TRS_SCREEN_FULL_3D	 ,TRC_FIXED_RGBA_8,_FL_,0);
//			break;
//	}
//
//	varAccumulator = Render().GetTechniqueGlobalVariable("MotionAccuBuffer",_FL_);
//	varTexture	   = Render().GetTechniqueGlobalVariable("MotionTexture"   ,_FL_);
//
//
//	Render().GetShaderId("MotionBlurShow", MotionBlurShow_id);
//	Render().GetShaderId("MotionBlurCopy", MotionBlurCopy_id);
//
//	
//
////	BlurOriginalK = Render().GetTechniqueGlobalVariable("BlurOriginalK",_FL_);
////	AfterImageK	  = Render().GetTechniqueGlobalVariable("AfterImageK"  ,_FL_);
//}
///*
//void MotionBlur::FilterImage(IBaseTexture *source, IRenderTarget *destination)
//{
//	Render().SetRenderTarget(RTO_DONTOCH_CONTEXT,destination);
//	Render().SetViewport(Render().GetFullScreenViewPort());
//	Render().BeginScene();
//
//	if( SourceTex )
//		SourceTex->SetTexture(source);
//
//	if( varAccumulator )
//		varAccumulator->SetTexture(bFirstTime ? source : pAccumulator->AsTexture());
//
//	if( BlurOriginalK )
//		BlurOriginalK->SetFloat(fOriginal);
//	
//	if( AfterImageK )
//		AfterImageK  ->SetFloat(fAfterImage);
//
//	if( BlendType )
//		BlendType->SetDword(motionBlur ? 0 : 1);
//
//	if( BlendFactor )
//		BlendFactor->SetFloat(0.9f);
//
//	Render().DrawFullScreenQuad("MotionBlurShow",
//		(float)Render().GetScreenInfo().dwWidth,(float)Render().GetScreenInfo().dwHeight);
//
//	Render().EndScene();
//
//	bFirstTime = false;
//
//	if( fTime < 0.01f )
//		return;
//
//	fTime = 0.0f;
//
//	Render().SetRenderTarget(RTO_DONTOCH_CONTEXT,pAccumulator);
//	Render().SetViewport(Render().GetFullScreenViewPort());
//	Render().BeginScene();
//
//	if( varAccumulator )
//		varAccumulator->SetTexture(destination->AsTexture());
//
//	Render().DrawFullScreenQuad("MotionBlurCopy",
//		(float)Render().GetScreenInfo().dwWidth,(float)Render().GetScreenInfo().dwHeight);
//
//	Render().EndScene();
//}
//*/
//void MotionBlur::FilterImage(IBaseTexture *source, IRenderTarget *destination)
//{
////	Assert(detailLevel > det_off)
//	return;
//	if( detailLevel < det_low )
//		return;
//
//	Render().PushRenderTarget();
////	if( fTime > 0.01f )
//	{
//	//	fTime = 0.00f;
//
//		// обновляем изображение
//
//		Render().SetRenderTarget(RTO_DONTOCH_CONTEXT,pTexture);
//	//	Render().SetViewport(Render().GetFullScreenViewPort());
//		Render().SetViewport(texViewPort);
//		Render().BeginScene();
//
//		if( SourceTex )
//			SourceTex->SetTexture(source);
//
//		if( varAccumulator )
//			varAccumulator->SetTexture(bFirstTime ? source : pAccumulator->AsTexture());
//
//		bFirstTime = false;
//
//		if( BlendFactor )
//		//	BlendFactor->SetFloat(0.90f);
//		//	BlendFactor->SetFloat(0.95f);
//			BlendFactor->SetFloat(1.0f - fOriginal);
//
//	//	Render().DrawFullScreenQuad("MotionBlurShow",
//	//		(float)Render().GetScreenInfo().dwWidth,(float)Render().GetScreenInfo().dwHeight);
//		Render().DrawFullScreenQuad(
//			(float)texViewPort.Width,(float)texViewPort.Height,
//			MotionBlurShow_id);
//
//		Render().EndScene();
//
//		// сбрасываем в буфер
//
//		Render().SetRenderTarget(RTO_DONTOCH_CONTEXT,pAccumulator);
//	//	Render().SetViewport(Render().GetFullScreenViewPort());
//		Render().SetViewport(texViewPort);
//		Render().BeginScene();
//
//		if( varTexture )
//			varTexture->SetTexture(pTexture->AsTexture());
//
//	//	Render().DrawFullScreenQuad("MotionBlurCopy",
//	//		(float)Render().GetScreenInfo().dwWidth,(float)Render().GetScreenInfo().dwHeight);
//		Render().DrawFullScreenQuad(
//			(float)texViewPort.Width,(float)texViewPort.Height,
//			MotionBlurCopy_id);
//
//		Render().EndScene();
//	}
//
//	// текущий кадр
//
//	Render().SetRenderTarget(RTO_DONTOCH_CONTEXT,destination);
//	Render().SetViewport(Render().GetFullScreenViewPort_3D());
//	Render().BeginScene();
//
//	if( SourceTex )
//		SourceTex->SetTexture(source);
//
//	if( varAccumulator )
//		varAccumulator->SetTexture(pAccumulator->AsTexture());
//
//	if( BlendFactor )
//	//	BlendFactor->SetFloat(0.5f);
//		BlendFactor->SetFloat(fAfterImage);
//
//	Render().DrawFullScreenQuad(
//		(float)Render().GetScreenInfo3D().dwWidth,(float)Render().GetScreenInfo3D().dwHeight,
//		MotionBlurShow_id);
//
//	Render().EndScene();
//
//	Render().PopRenderTarget(RTO_DONTOCH_CONTEXT);
//
//}
//
//ITexture *MotionBlur::GetInputTexture()
//{
//	return Render().GetPostprocessTexture();
//}
//
//void MotionBlur::SetParams(float  fOriginal, float  fAfterImage)
//{
//	if( detailLevel == det_off )
//	{
//	//	Enable(false);
//		return;
//	}
//
//	this->fOriginal	  = fOriginal;
//	this->fAfterImage = fAfterImage;
//
//	bFirstTime = true;
//}
//
//void MotionBlur::GetParams(float &fOriginal, float &fAfterImage)
//{
//	fOriginal	= this->fOriginal;
//	fAfterImage = this->fAfterImage;
//}
//
////============================================================================================
//
//RealMotionBlurController:: RealMotionBlurController()
//{
//	PostProcessService *pServ = (PostProcessService *)api->GetService("PostProcessService");
//	MotionBlur *pBlur = (MotionBlur *)pServ->GetMotionBlurFilter();
//
//	turnedOff = pBlur->turnedOff();
//
//	bActive = true;
//}
//
//RealMotionBlurController::~RealMotionBlurController()
//{
////	if( EditMode_IsOn())
////	{
//		PostProcessService *pServ = (PostProcessService *)api->GetService("PostProcessService");
//		MotionBlur *pBlur = (MotionBlur *)pServ->GetMotionBlurFilter();
//
//		pBlur->Enable(false);
////	}
//}
//
//void RealMotionBlurController::Restart()
//{
//	PostProcessService *pServ = (PostProcessService *)api->GetService("PostProcessService");
//	MotionBlur *pBlur = (MotionBlur *)pServ->GetMotionBlurFilter();
//
//	pBlur->Enable(false);
//
//	ReCreate();
//}
//
////Инициализировать объект
//bool RealMotionBlurController::Create(MOPReader &reader)
//{
//	if( turnedOff )
//		return true;
//
//	SetUpdate(&RealMotionBlurController::FlushPostProcess,ML_POSTEFFECTS);
//
//	if( !MissionObject::Create(reader))
//		return false;
//
//	fOriginal	= reader.Float();
//	fAfterImage = reader.Float();
//
////	blur = reader.Bool();
//
//	Activate(reader.Bool());
//
//	return true;
//}
//
////Инициализировать объект
//bool RealMotionBlurController::EditMode_Create(MOPReader &reader)
//{
//	Create(reader);
//
//	return true;
//}
//
////Обновить параметры
//bool RealMotionBlurController::EditMode_Update(MOPReader &reader)
//{
//	Create(reader);
//
//	return true;
//}
//
//void RealMotionBlurController::Activate(bool isActive)
//{
//	if( turnedOff )
//		return;
//
//	bActive = isActive;
//
//	PostProcessService* pServ = (PostProcessService*)api->GetService("PostProcessService");
//	MotionBlur *pBlur = (MotionBlur *)pServ->GetMotionBlurFilter();
//
//	if( bActive )
//	{
//		pBlur->Enable(true);
//		pBlur->SetParams(fOriginal,fAfterImage);
//
//	//	pBlur->SetType(blur);
//	}
//	else
//	{
//		pBlur->Enable(false);
//	}
//
//	MissionObject::Activate(isActive);
//}
//
//void _cdecl RealMotionBlurController::FlushPostProcess(float dltTime, long level)
//{
////	PostProcessService *pServ = (PostProcessService *)api->GetService("PostProcessService");
////	MotionBlur *pBlur = (MotionBlur *)pServ->GetAfterImageFilter();
//
////	pBlur->Update(dltTime);
//
//	return;
//	Render().PostProcess();
//}

//============================================================================================
//Параметры инициализации
//============================================================================================

class RealMotionBlurController : public MissionObject
{
public:
	RealMotionBlurController() {}
	virtual ~RealMotionBlurController() {}
};

MOP_BEGINLISTCG(RealMotionBlurController, "MotionBlur", '1.00', 0, "MotionBlur", "Post effects")

/*	MOP_FLOAT("Original"  , 0.5f);
	MOP_FLOAT("AfterImage", 0.5f);

	MOP_BOOL("Motion blur", false);*/

	MOP_FLOATEXC("Apply force", 0.05f, 0.0f, 1.0f, "Скорость обновления буфера");
	MOP_FLOATEXC("Post factor", 0.50f, 0.0f, 1.0f, "Сила действия эффекта");

	MOP_BOOL("Active", true);

MOP_ENDLIST(RealMotionBlurController)
