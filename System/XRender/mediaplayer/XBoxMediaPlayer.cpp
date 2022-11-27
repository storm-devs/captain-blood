#ifdef _XBOX

#include "XBoxMediaPlayer.h"
#include "..\..\..\common_h\Sound.h"


XBoxMediaPlayer::~XBoxMediaPlayer()
{
	ClosePlayer();
}

XBoxMediaPlayer::XBoxMediaPlayer(IDirect3DDevice9* pD3D, const char* pcFileName, HRESULT& hr, bool bLoopPlay, bool useStreaming)
{
	xmvPlayer = NULL;
	m_pD3DDevice = pD3D;
	m_bLoopPlay = bLoopPlay;
	m_pvMemBuffer = NULL;
	m_dwMemSize = 0;

	XMEDIA_XMV_CREATE_PARAMETERS crdat;
	ZeroMemory(&crdat, sizeof (crdat));
	crdat.dwFlags = XMEDIA_CREATE_BACKGROUND_MUSIC | XMEDIA_CREATE_CPU_AFFINITY;
	if( bLoopPlay )
		crdat.dwFlags |= XMEDIA_CREATE_FOR_LOOP;
	// use #5 cpu for decoder and render
	crdat.dwAudioDecoderCpu = 5;
	crdat.dwVideoDecoderCpu = 2;
	crdat.dwAudioRendererCpu = 5;
	crdat.dwVideoRendererCpu = 4;
	crdat.dwAudioStreamId = XMEDIA_STREAM_ID_USE_DEFAULT; //XMEDIA_STREAM_ID_DONT_USE
	crdat.dwVideoStreamId = XMEDIA_STREAM_ID_USE_DEFAULT;

	if( useStreaming )
	{
		crdat.createType = XMEDIA_CREATE_FROM_FILE;
		crdat.createFromFile.szFileName = pcFileName;
		crdat.createFromFile.dwIoBlockSize = 0x4000;
		crdat.createFromFile.dwIoBlockCount = 100;
		crdat.createFromFile.dwIoBlockJitter = 3;
	}
	else
	{
		// читаем из файла в память
		IFileService *pFS = (IFileService *)api->GetService("FileService");
		Assert(pFS);
		IDataFile* pFileData = pFS->OpenDataFile(pcFileName, file_open_fromdisk, _FL_);
		if( pFileData )
		{
			m_dwMemSize = pFileData->Size();
			if( m_dwMemSize > 0 )
			{
				m_pvMemBuffer = NEW BYTE[m_dwMemSize];
				Assert(m_pvMemBuffer);
				if( pFileData->Read(m_pvMemBuffer,m_dwMemSize) != m_dwMemSize )
				{
					DELETE(m_pvMemBuffer);
					m_dwMemSize = 0;
				}
			}
			pFileData->Release();
		}

		// данных нет - значит видео нет - ничего не создаем
		if( !m_pvMemBuffer )
		{
			hr = S_FALSE;
			return;
		}

		crdat.createType = XMEDIA_CREATE_FROM_MEMORY;
		crdat.createFromMemory.pvBuffer = m_pvMemBuffer;
		crdat.createFromMemory.dwBufferSize = m_dwMemSize;
	}

	IXAudio2* pAudioInstance = NULL;
	ISoundService* pSndService = (ISoundService*)api->GetService("SoundService");
	if( pSndService )
		pAudioInstance = (IXAudio2*)pSndService->GetXAudio2();

	if( !pAudioInstance )
	{
		crdat.dwAudioStreamId = XMEDIA_STREAM_ID_DONT_USE;
		hr = S_FALSE;
		return;
	}

	// Create the movie player specifying a file path to a movie.
	hr = XMedia2CreateXmvPlayer ( pD3D, pAudioInstance, &crdat, &xmvPlayer );

	// set volume
	if( hr==S_OK && xmvPlayer!=NULL )
	{
		IXAudio2SourceVoice* pSrcVoice = NULL;
		if( xmvPlayer->GetSourceVoice(&pSrcVoice) == S_OK )
		{
			float fGlobVol = api->Storage().GetFloat("Options.GlobalVolume",1.f);
			float fFXVol = api->Storage().GetFloat("Options.FxVolume",1.f);
			pSrcVoice->SetVolume( fGlobVol * fFXVol );
		}
	}
}

bool XBoxMediaPlayer::Release()
{
	delete this;
	return true;
}

void XBoxMediaPlayer::Pause(bool isPause)
{
	if( xmvPlayer )
	{
		if( isPause )
			xmvPlayer->Pause();
		else
			xmvPlayer->Resume();
	}
}

IMediaPlayer::MPResultCode XBoxMediaPlayer::RenderNextFrame()
{
	D3DVIEWPORT9 vp;
	if( m_pD3DDevice )
	{
		m_pD3DDevice->GetViewport(&vp);
	}

	HRESULT hr = XMEDIA_W_EOF;
	//if( xmvPlayer ) hr = xmvPlayer->RenderNextFrame( m_bLoopPlay ? (XMEDIA_PLAY_LOOP | XMEDIA_PLAY_DISABLE_AV_SYNC) : XMEDIA_PLAY_DISABLE_AV_SYNC, NULL );
	if( xmvPlayer )
	{
		long nTimeToPresent = 0;
		hr = xmvPlayer->RenderNextFrame( m_bLoopPlay ? (XMEDIA_PLAY_LOOP | XMEDIA_PLAY_DISABLE_AV_SYNC) : XMEDIA_PLAY_DISABLE_AV_SYNC, &nTimeToPresent );
		if( hr == XMEDIA_W_NO_DATA )
			hr = xmvPlayer->RenderNextFrame( XMEDIA_PLAY_REPEAT_FRAME, NULL );
		else if( hr != XMEDIA_W_EOF )
		{
			if( nTimeToPresent < 0 )
				hr = xmvPlayer->RenderNextFrame( m_bLoopPlay ? XMEDIA_PLAY_LOOP : 0, NULL );
		}
	}

	if( m_pD3DDevice )
	{
		m_pD3DDevice->SetViewport(&vp);
		m_pD3DDevice->SetRenderState(D3DRS_VIEWPORTENABLE, TRUE);
	}


	if(hr==XMEDIA_W_EOF)
		return IMediaPlayer::MP_EOF;

	if(hr==XMEDIA_W_NO_DATA)
		return IMediaPlayer::MP_NO_DATA;

	return IMediaPlayer::MP_OK;
}

void XBoxMediaPlayer::SetRectangle(RECT& rect)
{
	if( !xmvPlayer )
	{
		return;
	}
	
	xmvPlayer->SetRectangle(&rect);
}

void XBoxMediaPlayer::SetUV(float fl,float ft,float fr,float fb)
{
	if( !xmvPlayer ) return;

	XMEDIA_VIDEO_SCREEN vscr;
	if( S_OK != xmvPlayer->GetVideoScreen(&vscr) ) return;

	vscr.upperLeft.fTu = fl; vscr.upperLeft.fTv = ft;
	vscr.upperRight.fTu = fr; vscr.upperRight.fTv = ft;
	vscr.lowerLeft.fTu = fl; vscr.lowerLeft.fTv = fb;
	vscr.lowerRight.fTu = fr; vscr.lowerRight.fTv = fb;

	xmvPlayer->SetVideoScreen(&vscr);
}

void XBoxMediaPlayer::ClosePlayer()
{
	if( xmvPlayer ) xmvPlayer->Release();
	xmvPlayer = 0;
	if( m_pD3DDevice )
		m_pD3DDevice->SetRenderState(D3DRS_VIEWPORTENABLE, TRUE);

	DELETE( m_pvMemBuffer );
	m_dwMemSize = 0;
}

#endif
