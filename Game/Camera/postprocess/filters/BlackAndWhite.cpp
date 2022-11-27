//#include "BlackAndWhite.h"
//#include "..\PostProcess.h"

//BlackAndWhite:: BlackAndWhite()
//{
//	SourceTex = null;
//
//	Scale = null;
//
//	Users = 0;
//	scale = 0.0f;
//
////	detailLevel = det_hi;
//	detailLevel = det_off;
//
//	pServ = null;
//}
//
//BlackAndWhite::~BlackAndWhite()
//{
//	SourceTex = null;
//	Scale = null;
//}
//
//void BlackAndWhite::UserInit()
//{
///*	#ifndef _XBOX
//
//	IFileService *storage = (IFileService *)api->GetService("FileService");
//	Assert(storage)
//
//	IIniFile *ini = storage->SystemIni();//storage->OpenIniFile(api->GetVarString("Ini"),_FL_);
//
//	if( ini )
//	{
//		const char *s = ini->GetString("PostEffects","Black","hi");
//
//		if( string::IsEqual(s,"off"))
//		{
//			detailLevel = det_off;
//		}
//		else
//		{
//			detailLevel = det_hi;
//		}
//
//		//ini->Release();
//	}
//
//	#else
//
//	detailLevel = det_hi;
//
//	#endif*/
//	detailLevel = det_off;
//
//	if( detailLevel == det_off )
//		return;
//
////	SourceTex = Render().GetSourceTextureVariable();
//	SourceTex = Render().GetTechniqueGlobalVariable("PP_CommonTexture",_FL_);
//
//	Scale = Render().GetTechniqueGlobalVariable("BlackScale",_FL_);
//
//	Render().GetShaderId("PostProcessShowBW", PostProcessShowBW_id);
//
//	pServ = (PostProcessService *)api->GetService("PostProcessService");
//}
//
//void BlackAndWhite::FilterImage(IBaseTexture *source, IRenderTarget *destination)
//{
//	return;
////	Assert(detailLevel > det_off)
//	if( detailLevel < det_low )
//		return;
//
//	if( !pServ->Enabled())
//	{
//		this->scale = 0.0f;
//
//		Users = 0;
//
//		return;
//	}
//
//	if( Users < 1 || this->scale < 0.01f )
//	{
//		Enable(false);
//		return;
//	}
//
////	Render().SetRenderTarget(RTO_DONTOCH_CONTEXT,destination);
////	Render().SetViewport(Render().GetFullScreenViewPort());
//
//	Render().BeginScene();
//
//	if( SourceTex )
//		SourceTex->SetTexture(source);
//
//	////
//
//	if( scale > 1.0f )
//		scale = 1.0f;
//
//	////
//
//	if( Scale )
//		Scale->SetFloat(scale);
//
////	Render().DrawFullScreenQuad("PostProcessShowBW",
////		(float)Render().GetScreenInfo().dwWidth,(float)Render().GetScreenInfo().dwHeight);
//
//	const RENDERVIEWPORT &wp = Render().GetViewport();
//
//	float kx = 1.0f/Render().GetFullScreenViewPort_3D().Width;
//	float ky = 1.0f/Render().GetFullScreenViewPort_3D().Height;
//
//	float tl = wp.X*kx; float tr = (wp.X + wp.Width )*kx;
//	float tt = wp.Y*ky; float tb = (wp.Y + wp.Height)*ky;
//
//	const float r = 1.0f;
//	const float d = 0.0f;
//
//	float dx = kx;
//	float dy = ky;
//
//	float buf[] = {
//		-r - dx,-r + dy, d, 1.0f,  tl,tb,
//		-r - dx, r + dy, d, 1.0f,  tl,tt,
//		 r - dx,-r + dy, d, 1.0f,  tr,tb,
//		 r - dx, r + dy, d, 1.0f,  tr,tt};
//
//	Render().DrawPrimitiveUP(PostProcessShowBW_id, PT_TRIANGLESTRIP,2,buf,sizeof(float)*6);
//
//	Render().EndScene();
//
//	////
//
//	this->scale = 0.0f;
//
//	Users = 0;
//
//	////
//}
//
//ITexture* BlackAndWhite::GetInputTexture()
//{
//	return Render().GetPostprocessTexture();
//}
//
//void BlackAndWhite::AddParams(float scale)
//{
//	if( detailLevel == det_off )
//	{
//	//	Enable(false);
//		return;
//	}
//
//	Enable(true);
//
//	this->scale += scale;
//
//	Users++;
//}
//
////============================================================================================
//
//BlackController:: BlackController()
//{
//	pServ = (PostProcessService*)api->GetService("PostProcessService");
//
//	pBlack = pServ->GetBlackFilter();
//	turnedOff = pBlack->turnedOff();
//
//	bActive = true;
//
//	m_alpha = 1.0f;
//}
//
//BlackController::~BlackController()
//{
////	if( EditMode_IsOn())
////	{
//	//	PostProcessService* pServ = (PostProcessService*)api->GetService("PostProcessService");
//	//	BlackAndWhite* pBlack = pServ->GetBlackFilter();
//
//	/*	pBlack->Enable(bEnabled = false);*/
////	}
//}
//
////Инициализировать объект
//bool BlackController::Create(MOPReader &reader)
//{
//	if( turnedOff )
//		return true;
//
//	SetUpdate(&BlackController::FlushPostProcess,ML_POSTEFFECTS);
//
//	if( !MissionObject::Create(reader))
//		return false;
//
////	PostProcessService* pServ = (PostProcessService*)api->GetService("PostProcessService");
////	BlackAndWhite* pBlack = pServ->GetBlackFilter();
//
//	blackFactor = reader.Float();
//
//	m_alpha = 1.0f;
//
///*	if( pBlack )
//		pBlack->SetParams(m_alpha*blackFactor);*/
//
//	Activate(reader.Bool());
//
//	return true;
//}
//
////Инициализировать объект
//bool BlackController::EditMode_Create(MOPReader &reader)
//{
//	Create(reader);
//
//	return true;
//}
//
////Обновить параметры
//bool BlackController::EditMode_Update(MOPReader &reader)
//{
//	Create(reader);
//
//	return true;
//}
//
//void BlackController::Command(const char *id, dword numParams, const char **params)
//{
//	if( string::IsEmpty(id))
//		return;
//
//	if( string::IsEqual(id,"setfxa"))
//	{
//		if( numParams < 1 )
//		{
//			LogicDebugError("Command BlackController::[setfxa] error. No alpha specified.");
//			return;
//		}
//
//		float alpha = (float)atof(params[0]);
//
//		UpdateAlpha(alpha);
//	}
//}
//
//void BlackController::Activate(bool isActive)
//{
//	if( turnedOff )
//		return;
//
//	bActive = isActive;
//
////	PostProcessService* pServ = (PostProcessService*)api->GetService("PostProcessService");
////	BlackAndWhite* pBlack = pServ->GetBlackFilter();
//
///*	if( bActive && IsVisible())
//	{
//		pBlack->Enable(bEnabled = true);
//	}
//	else
//	{
//		pBlack->Enable(bEnabled = false);
//	}*/
//
//	MissionObject::Activate(isActive);
//}
//
//void _cdecl BlackController::FlushPostProcess(float dltTime, long level)
//{
//	return;
//	Render().PostProcess();
//
////	PostProcessService* pServ = (PostProcessService*)api->GetService("PostProcessService");
////	BlackAndWhite* pBlack = pServ->GetBlackFilter();
//
///*	if( EditMode_IsOn())
//	{
//	//	PostProcessService* pServ = (PostProcessService*)api->GetService("PostProcessService");
//	//	BlackAndWhite* pBlack = pServ->GetBlackFilter();
//
//		if( pBlack->IsEnabled() != bEnabled )
//			pBlack->Enable(bEnabled);
//	}
//
//	if( bEnabled )
//		pBlack->SetParams(m_alpha*blackFactor);
//
//	if( pBlack->IsEnabled() && IsActive())
//		pServ->FilterNotify("Black&White",GetObjectID().c_str());*/
//
//	if( IsVisible() && IsActive())
//	{
//		float k = m_alpha*blackFactor;
//
//		pBlack->AddParams(k);
//
//		if( pBlack->IsEnabled())
//			pServ->FilterNotify("Black&White",GetObjectID().c_str(),0.5f,k);
//	}
//}
//
//void BlackController::UpdateAlpha(float alpha)
//{
//	m_alpha = alpha;
//
///*	if( IsVisible())
//	{
//		if( IsActive())
//		{
//			if( !bEnabled )
//				pBlack->Enable(bEnabled = true);
//		}
//	}
//	else
//	{
//		if( bEnabled )
//			pBlack->Enable(bEnabled = false);
//	}*/
//}

//============================================================================================
//Параметры инициализации
//============================================================================================

/*MOP_BEGINLISTCG(BlackController, "Black controller", '1.00', 0, "Black controller", "Post effects")

	MOP_FLOATEX("Scale", 1.0f, 0.0f, 1.0f);

	MOP_BOOL("Active", true);

MOP_ENDLIST(BlackController)*/
