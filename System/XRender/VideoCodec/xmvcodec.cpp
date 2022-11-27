#ifdef _XBOX

#include "xmvcodec.h"
#include "..\Render.h"
#include "..\mediaplayer\XBoxMediaPlayer.h"

XMVCodec::XMVCodec(IRender* _pRender,bool _useStreaming) : IVideoCodec(_pRender)
{
	pPlayer = 0;
	SetViewPosition(0.f,0.f,1.f,1.f,false);
	useStreaming = _useStreaming;
	bPausePlay = false;
}

XMVCodec::~XMVCodec()
{
	CloseFile();
}

bool XMVCodec::OpenFile(const char* pcVideoFile,bool bLoopPlay)
{
	CloseFile();
	pPlayer = GetRender()->GetMediaPlayer(pcVideoFile,bLoopPlay,useStreaming);
	if( pPlayer==NULL )
		return false;
	UpdateViewPosition();
	return true;
}




bool XMVCodec::Frame(bool bPause)
{
	if( pPlayer )
	{
		if( useStreaming && bPausePlay != (IsPause() || bPause)  )
		{
			bPausePlay = !bPausePlay;
			pPlayer->Pause(bPausePlay);
		}

		UpdateViewPosition();

		//NGRender::pRS->D3D()->Clear(0, NULL, D3DCLEAR_TARGET, rand(), 0, 0x0);

		DWORD dwSavedColorWrite;
		DWORD dwSavedFillMode;
		DWORD dwSavedAlphaBlend;
		DWORD dwSavedAlphaTest;
		NGRender::pRS->D3D()->GetRenderState(D3DRS_COLORWRITEENABLE, &dwSavedColorWrite);
		NGRender::pRS->D3D()->GetRenderState(D3DRS_FILLMODE, &dwSavedFillMode);
		NGRender::pRS->D3D()->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwSavedAlphaBlend);
		NGRender::pRS->D3D()->GetRenderState(D3DRS_ALPHATESTENABLE, &dwSavedAlphaTest);


		NGRender::pRS->D3D()->SetRenderState(D3DRS_COLORWRITEENABLE, 0x0000000F);
		NGRender::pRS->D3D()->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

		NGRender::pRS->D3D()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		NGRender::pRS->D3D()->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		
		IMediaPlayer::MPResultCode rc = pPlayer->RenderNextFrame();

		// wait till async audio stream reading
		//while( rc == IMediaPlayer::MP_NO_DATA )
		//	rc = pPlayer->RenderNextFrame();

		if( rc == IMediaPlayer::MP_EOF )
		{
			CloseFile();
		}

		NGRender::pRS->D3D()->SetRenderState(D3DRS_COLORWRITEENABLE, dwSavedColorWrite);
		NGRender::pRS->D3D()->SetRenderState(D3DRS_FILLMODE, dwSavedFillMode);
		NGRender::pRS->D3D()->SetRenderState(D3DRS_ALPHABLENDENABLE, dwSavedAlphaBlend);
		NGRender::pRS->D3D()->SetRenderState(D3DRS_ALPHATESTENABLE, dwSavedAlphaTest);


	}
	return pPlayer!=0;
}

void XMVCodec::CloseFile()
{
	Stop();
	if( pPlayer )
	{
		pPlayer->Release();
		pPlayer = 0;
	}
	bPausePlay = false;
}

void XMVCodec::SetViewPosition(float fx, float fy, float fw, float fh, bool bModifyAspect)
{
	IRender* pRS = GetRender();

	float fAspect = 1.f;
	if( bModifyAspect )
	{
		float cx = (float)pRS->GetFullScreenViewPort_2D().Width;
		float cy = (float)pRS->GetFullScreenViewPort_2D().Height;
		float scr_aspect = cx / cy;
		const float def_aspect = 16.0f / 9.0f;
		fAspect = def_aspect / scr_aspect * pRS->GetWideScreenAspectWidthMultipler();
	}

	float fWidthMultipler = fAspect;
	float fHeightMultipler = 1.f;

	viewPos.left = (fx - 0.5f) * fWidthMultipler + 0.5f;
	viewPos.top = (fy - 0.5f) * fHeightMultipler + 0.5f;
	viewPos.right = (fx + fw - 0.5f) * fWidthMultipler + 0.5f;
	viewPos.bottom = (fy + fh - 0.5f) * fHeightMultipler + 0.5f;

	//SetUV();
	float uvl=0.f;
	float uvr=1.f;
	float uvt=0.f;
	float uvb=1.f;

	float fvpw = viewPos.right - viewPos.left;
	float fvph = viewPos.bottom - viewPos.top;

	// правим UV по левой стороне
	if( viewPos.left < 0.f ) {
		uvl = -viewPos.left / fvpw;
		viewPos.left = 0.f;
	}
	if( viewPos.left > 1.f ) {
		uvl = (viewPos.left - 1.f) / fvpw;
		viewPos.left = 1.f;
	}
	// правим UV по верху
	if( viewPos.top < 0.f ) {
		uvt = -viewPos.top / fvph;
		viewPos.top = 0.f;
	}
	if( viewPos.top > 1.f ) {
		uvt = 1.f - (viewPos.top - 1.f) / fvph;
		viewPos.top = 1.f;
	}
	// правим UV по правой стороне
	if( viewPos.right > 1.f ) {
		uvr = 1.f - (viewPos.right - 1.f) / fvpw;
		viewPos.right = 1.f;
	}
	if( viewPos.right < 0.f ) {
		uvr = -viewPos.right / fvpw;
		viewPos.right = 0.f;
	}
	// правим UV по низу
	if( viewPos.bottom > 1.f ) {
		uvb = 1.f - (viewPos.bottom - 1.f) / fvph;
		viewPos.bottom = 1.f;
	}
	if( viewPos.bottom < 0.f ) {
		uvb = -viewPos.bottom / fvph;
		viewPos.bottom = 0.f;
	}

	if( pPlayer )
		((XBoxMediaPlayer*)pPlayer)->SetUV(uvl,uvt,uvr,uvb);

	UpdateViewPosition();
}

void XMVCodec::UpdateViewPosition()
{
	if( !pPlayer ) return;

	//RENDERSCREEN & scr = GetHostObj()->Render().GetScreenInfo();
	const RENDERVIEWPORT & vprt = GetRender()->GetViewport();

	RECT r;
	r.left = vprt.X + (long)(viewPos.left * vprt.Width);
	r.top = vprt.Y + (long)(viewPos.top * vprt.Height);
	r.right = vprt.X + (long)(viewPos.right * vprt.Width);
	r.bottom = vprt.Y + (long)(viewPos.bottom * vprt.Height);

	pPlayer->SetRectangle(r);
}

void XMVCodec::Stop()
{
	//CloseFile();
	//if( pPlayer )
	//	pPlayer->Stop();
	IVideoCodec::Stop();
}

#endif
