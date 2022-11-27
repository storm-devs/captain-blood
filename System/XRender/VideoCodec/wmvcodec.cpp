#ifndef _XBOX

#include "wmvcodec.h"

// обращение к выделению памяти через движок
void * _cdecl IWMVMemoryManager::CoreAlloc(size_t size) {
	return api->Reallocate(0,size,_FL_);
}
void _cdecl IWMVMemoryManager::CoreFree(void * ptr) {
	api->Free(ptr,_FL_);
}
// вывод лога через движок
void IWMVMemoryManager::LogTrace(const char* pcText) {
	api->Trace("%s", pcText);
}

// установленная в движке громкость звука
float IWMVMemoryManager::GetVolume()
{
	float fGlobVol = api->Storage().GetFloat("Options.GlobalVolume",1.f);
	float fFXVol = api->Storage().GetFloat("Options.FxVolume",1.f);
	return powf(fGlobVol * fFXVol, 0.4f);
}

// класс обертка для текстуры
// используется как в движковом рендере, так и для работы видеокодека
// свопит для использования две текстуры: пока одна заполняется видеокодеком, другая выводиться в рендер движка
class WMVTexture : public IWMVTexture
{
	CritSection critSect;
	int nUseIdx;
	int nRefCount;
	bool bReady;

	RENDERLOCKED_RECT lr;

public:
	WMVTexture()
	{
		m_pTexture[0] = m_pTexture[1] = 0;
		nUseIdx = 0;
		nRefCount = 1;
		bReady = false;
		lr.pBits = NULL;
	}

	virtual void Release() {
		if(--nRefCount <= 0) delete this;
	}

	void AddRef() {
		nRefCount++;
	}

	// лочится текстура из видеокодека
	virtual bool Lock(void* &pTxtBuffer, int &lTxtPitch)
	{
		// see free texture
		int n = 1 - nUseIdx;
		if( !m_pTexture[n] )
		{
			return false;
		}
		if( !m_pTexture[n]->LockRect(0,&lr,null,LOCK_DISCARD) )
		{
			return false;
		}
		AddRef();
		pTxtBuffer = lr.pBits;
		lTxtPitch = lr.Pitch;
		return true;
	}
	// анлочится текстура в видеокодеке
	virtual void Unlock()
	{
		bReady = true;
		int n = 1 - nUseIdx;
		if(m_pTexture[n]) {
			m_pTexture[n]->UnlockRect(0);
		}
		critSect.Enter();
		nUseIdx = n;
		critSect.Leave();
		Release();
	}

	// включить использование текстуры (используется в рендере видео)
	ITexture* UsageTexture(bool use)
	{
		if( !bReady )
			return null;

		// начать использование (входим в крит секцию)
		if( use )
		{
			critSect.Enter();
			return m_pTexture[nUseIdx];
		}
		// закончить использование (выходим из крит секции)
		else
		{
			critSect.Leave();
			return null;
		}
	}

	static IRender* render;
	static WMVCodec* codec;
	static IFileService* pFileService;
	ITexture* m_pTexture[2];

protected:
	virtual ~WMVTexture()
	{
		RELEASE(m_pTexture[0]);
		RELEASE(m_pTexture[1]);
	}
};
IRender* WMVTexture::render = NULL;
WMVCodec* WMVTexture::codec = NULL;
IFileService* WMVTexture::pFileService = NULL;


IWMVTexture* IWMVTexture::Create(unsigned int& w, unsigned int& h)
{
	if( !WMVTexture::render ) return NULL;
	if( !WMVTexture::codec ) return NULL;
	WMVTexture* pTexture = NEW WMVTexture();
	if( !pTexture ) return NULL;

	pTexture->m_pTexture[0] = WMVTexture::render->CreateTexture( w, h, 1, USAGE_DYNAMIC, FMT_L8, _FL_, POOL_DEFAULT );
	pTexture->m_pTexture[1] = WMVTexture::render->CreateTexture( w, h, 1, USAGE_DYNAMIC, FMT_L8, _FL_, POOL_DEFAULT );

	WMVTexture::codec->SetWMVTexture(pTexture);
	return pTexture;
}




void WMVCodec::SetWMVTexture(WMVTexture* pTex)
{
	RELEASE( pWMVTexture );
	pWMVTexture = pTex;
	if( pTex ) pTex->AddRef();
}

WMVCodec::WMVCodec(IRender* _pRender) : IVideoCodec(_pRender)
{
	shaderID = NULL;
	textureVar = NULL;

	WMVTexture::render = _pRender;
	WMVTexture::pFileService = (IFileService*)api->GetService("FileService");

	m_bLoopPlay = false;
	bMakeUninitializeDD = false;
	m_pGraph = NULL;
	pWMVTexture = NULL;
	Init();
}

WMVCodec::~WMVCodec()
{
	Release();
}

bool WMVCodec::OpenFile(const char* pcVideoFile,bool bLoopPlay)
{
	CloseFile();
	DELETE(m_pGraph);
	m_bLoopPlay = bLoopPlay;

	RELEASE( pWMVTexture );
	WMVTexture::codec = this;
	m_pGraph = IWMVGraph::CreateWMVGraphInstance();
	Assert(m_pGraph);

	if( WMVTexture::pFileService && WMVTexture::pFileService->SystemIni() )
	{
		if( WMVTexture::pFileService->SystemIni()->GetLong("sound","off",0) == 1 )
			m_pGraph->SoundOff(true);
	}

	if( !m_pGraph->OpenFile(pcVideoFile) )
	{
		DELETE(m_pGraph);
		WMVTexture::codec = null;
		return false;
	}

	// ok return
	IVideoCodec::OpenFile(pcVideoFile,bLoopPlay);
	//Play();
	WMVTexture::codec = null;
	return true;
}

bool WMVCodec::Frame(bool bPause)
{
	if( !IVideoCodec::Frame(bPause) ) return false;

	if( IsSystemPause() )
		bPause = true;

	if( bPause != this->bPause )
	{
		this->bPause = bPause;
		if( m_pGraph )
			if(bPause)
				m_pGraph->Stop();
			else
				m_pGraph->Play();
	}

	if( m_pGraph && m_pGraph->IsFinish() && !bPause )
	{
		if( m_bLoopPlay )
			m_pGraph->Restart();
		else
		{
			Stop();
			return IsProcessed();
		}
	}

	if( pWMVTexture )
	{
		ITexture* pTexture = pWMVTexture->UsageTexture(true);
		if( pTexture )
		{
			IRender* pRS = GetRender();

			textureVar->SetTexture(pTexture);
			pRS->DrawPrimitiveUP(shaderID, PT_TRIANGLESTRIP, 2, pVideoVertex, sizeof(VideoVertex));
		}
		pWMVTexture->UsageTexture(false);
	}

	return IsProcessed();
}

void WMVCodec::CloseFile()
{
	if( m_pGraph )
		m_pGraph->CloseFile();
	IVideoCodec::CloseFile();
}

void WMVCodec::SetViewPosition(float fx, float fy, float fw, float fh, bool bModifyAspect)
{
	IRender* pRS = GetRender();

	float fWidthMultipler = 1.f;
	float fHeightMultipler = 1.f;

	float cx = (float)pRS->GetFullScreenViewPort_2D().Width;
	float cy = (float)pRS->GetFullScreenViewPort_2D().Height;
	float scr_aspect = cx / cy;
	const float def_aspect = 16.0f / 9.0f;
	fWidthMultipler = def_aspect / scr_aspect * pRS->GetWideScreenAspectWidthMultipler();

	if( !bModifyAspect )
	{
		if( fWidthMultipler > 1.f )
		{
			fHeightMultipler = 1.f / fWidthMultipler;
			fWidthMultipler = 1.f;
		}
	}

	float HalfPixelSizeX = 1.0f / (float)pRS->GetScreenInfo2D().dwWidth;
	float HalfPixelSizeY = 1.0f / (float)pRS->GetScreenInfo2D().dwHeight;

	//Находим границы для рисования...
	float fl = (fx * 2.f - 1.f - HalfPixelSizeX) * fWidthMultipler;
	float fr = fl + fw * 2.f * fWidthMultipler;
	float ft = (1.f - fy * 2.f - HalfPixelSizeY) * fHeightMultipler;
	float fb = ft - fh * 2.f * fHeightMultipler;

	pVideoVertex[2].vPos = Vector (fl, fb, 1.0f);
	pVideoVertex[0].vPos = Vector (fr, fb, 1.0f);
	pVideoVertex[1].vPos = Vector (fr, ft, 1.0f);
	pVideoVertex[3].vPos = Vector (fl, ft, 1.0f);
}

void WMVCodec::Stop()
{
	if(m_pGraph)
		m_pGraph->Stop();
	IVideoCodec::Stop();
}

void WMVCodec::Pause(bool bPause)
{
	IVideoCodec::Pause(bPause);
}

bool WMVCodec::Init()
{
	IRender* pRS = GetRender();
	pRS->GetShaderId("VideoSprite", shaderID);
	textureVar = pRS->GetTechniqueGlobalVariable("VideoTex", _FL_);

	pVideoVertex[2].vPos = Vector (0.5f, 1.0f, 0.0f);
	pVideoVertex[0].vPos = Vector (1.0f, 1.0f, 0.0f);
	pVideoVertex[1].vPos = Vector (1.0f, 0.5f, 0.0f);
	pVideoVertex[3].vPos = Vector (0.5f, 0.5f, 0.0f);

	pVideoVertex[2].tu = 0.0f;  pVideoVertex[2].tv = 0.0f;
	pVideoVertex[0].tu = 1.0f;  pVideoVertex[0].tv = 0.0f;
	pVideoVertex[1].tu = 1.0f;  pVideoVertex[1].tv = 1.0f;
	pVideoVertex[3].tu = 0.0f;  pVideoVertex[3].tv = 1.0f;

	HRESULT hr = S_OK;
    if (FAILED(hr = CoInitialize(NULL)))
        return false;
	bMakeUninitializeDD = true;
	return true;
}

void WMVCodec::Release()
{
	CloseFile();

	DELETE(m_pGraph);

	RELEASE(pWMVTexture);

	if( bMakeUninitializeDD ) CoUninitialize();
	bMakeUninitializeDD = false;
}

void WMVCodec::Play()
{
	if( m_pGraph )
		m_pGraph->Play();
}

#endif
