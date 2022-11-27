#ifndef _XBOX

#ifndef _DEBUG
#pragma comment(lib, "xrender\\videocodec\\strmbase.lib")
#else
#pragma comment(lib, "xrender\\videocodec\\strmbased.lib")
#endif
#pragma comment(lib, "winmm.lib")

#include "wmvinterfaces.h"

__forceinline void * _cdecl operator new(size_t size)
{
	return IWMVMemoryManager::CoreAlloc(size);
}
__forceinline void _cdecl operator delete(void * ptr)
{
	IWMVMemoryManager::CoreFree(ptr);
}

#include "wmvgraph.h"
#include "DShowTextures.h"

IWMVGraph* IWMVGraph::CreateWMVGraphInstance()
{
	return new WMVGraph();
}

WMVGraph::WMVGraph()
{
	m_pGB = NULL;
	m_pMC = NULL;
	m_pMP = NULL;
	m_pME = NULL;
	m_pFileSource = NULL;
	m_pRenderer = NULL;
	m_maxTime = 0;
	m_bSoundOff = false;
}

WMVGraph::~WMVGraph()
{
	CloseFile();
}

bool WMVGraph::OpenFile(const char* pcVideoFile)
{
    HRESULT hr = S_OK;

	// Create the filter graph
    hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&m_pGB);
    if (FAILED(hr))
	{
        Msg("Could not create GraphBuilder!  hr=0x%x", hr);
        return false;
	}

    // Create the Texture Renderer object
    m_pRenderer = new CTextureRenderer(NULL, &hr);
    if (FAILED(hr) || !m_pRenderer)
    {
		if( m_pRenderer )
		{
			delete m_pRenderer;
			m_pRenderer = NULL;
		}
        Msg("Could not create texture renderer object!  hr=0x%x", hr);
        return false;
    }

    // Get a pointer to the IBaseFilter on the TextureRenderer, add it to graph
    if (FAILED(hr = m_pGB->AddFilter(m_pRenderer, L"TEXTURERENDERER")))
    {
        Msg("Could not add renderer filter to graph!  hr=0x%x", hr);
        return false;
    }

	// Source Filter
    WCHAR wFileName[MAX_PATH];
	UINT strSize = strlen(pcVideoFile);
	for( UINT d=0,s=0; d<MAX_PATH-1 && s<strSize; s++,d++ )
		wFileName[d] = pcVideoFile[s];
	wFileName[d] = 0;
    IBaseFilter* pFSrc = NULL;
	hr = AddFilterByCLSID(m_pGB, CLSID_WMAsfReader, wFileName, &pFSrc);
	if (FAILED(hr)) {
		Msg("Unable to add [WM ASF Reader]!  hr=0x%x, %s", hr, pcVideoFile);
		return false;
	}

	bool bSuccess = true;

	// Get the Inteface to IFileSource so we can load the file
	m_pFileSource = NULL;
	hr = pFSrc->QueryInterface(IID_IFileSourceFilter, (void **)&m_pFileSource);
	if (!FAILED(hr) && (m_pFileSource != NULL))
	{
		hr = m_pFileSource->Load(wFileName, NULL);
		if (FAILED(hr)) {
			Msg("Unable to load file %s!  hr=0x%x", pcVideoFile, hr);
			bSuccess = false;
		}
	}

	// Video source Filter Output Pin
    IPin* pFSrcPinOut = NULL;
	// Audio source Filter Output Pin
	IPin* pFSrcPinOut0 = NULL;

	// Find the source's Raw Video #1 output pin and the video renderer's input pin
	if (bSuccess && FAILED(hr = pFSrc->FindPin(L"Raw Video 0", &pFSrcPinOut))) {
		if (FAILED(hr = pFSrc->FindPin(L"Raw Video 1", &pFSrcPinOut))) {
			Msg("ConnectWMVFile() unable to find Raw Video Pin #1!  hr=0x%x", hr);
			bSuccess = false;
		}
	}

	// Find the source's Raw Audio #0 output pin and the audio renderer's input pin
	if( !m_bSoundOff )
	{
		if (bSuccess && FAILED(hr = pFSrc->FindPin(L"Raw Audio 0", &pFSrcPinOut0))) {
			if (FAILED(hr = pFSrc->FindPin(L"Raw Audio 1", &pFSrcPinOut0))) {
				pFSrcPinOut0 = NULL;
				hr = S_OK;
			}
		}
	}

	if (bSuccess && pFSrcPinOut0 && FAILED(hr = m_pGB->Render(pFSrcPinOut0)))
	{
		Msg("Could not render source output pin (audio)!  hr=0x%x", hr);
	}

	if (bSuccess && FAILED(hr = m_pGB->Render(pFSrcPinOut)))
    {
        Msg("Could not render source output pin (video)!  hr=0x%x", hr);
        bSuccess = false;
    }

    // Get the graph's media control, event & position interfaces
	if( bSuccess )
	{
		m_pGB->QueryInterface(&m_pMC);
		m_pGB->QueryInterface(&m_pMP);
		m_pGB->QueryInterface(&m_pME);

		//IID_IBasicAudio
		IBasicAudio* pBasicAudio = NULL;
		m_pGB->QueryInterface(&pBasicAudio);
		if( pBasicAudio )
		{
			long nVol = 0;// min = -10000; max = 0
			nVol = (long)(IWMVMemoryManager::GetVolume() * 10000) - 10000;
			if( nVol>0 ) nVol = 0;
			else if( nVol<-10000 ) nVol = -10000;
			pBasicAudio->put_Volume(nVol);
			pBasicAudio->Release();
		}

		if( m_pMP )
			m_pMP->get_Duration(&m_maxTime);
		m_curTime = 0;
	}

	if( pFSrcPinOut0 )
		pFSrcPinOut0->Release();
	if( pFSrcPinOut )
		pFSrcPinOut->Release();
	if( pFSrc )
		pFSrc->Release();

	return bSuccess;
}

HRESULT WMVGraph::AddFilterByCLSID( IGraphBuilder *pGB,     // Pointer to the Filter Graph Manager.
						 const GUID& clsid,      // CLSID of the filter to create.
						 LPCWSTR wszName,        // A name for the filter.
						 IBaseFilter **ppFilter) // Receives a pointer to the filter.
{
	if (!pGB || ! ppFilter) return E_POINTER;
	*ppFilter = 0;
	IBaseFilter *pFilter = 0;
	HRESULT hr = CoCreateInstance(clsid, 0, CLSCTX_INPROC_SERVER,
		IID_IBaseFilter, reinterpret_cast<void**>(&pFilter));
	if (SUCCEEDED(hr))
	{
		hr = pGB->AddFilter(pFilter, wszName);
		if (SUCCEEDED(hr))
			*ppFilter = pFilter;
		else
			pFilter->Release();
	}
	return hr;
}

void WMVGraph::CloseFile()
{
	Stop();
	CleanupDShow();
}

void WMVGraph::Play()
{
    // Start the graph running;
	if( m_pMC )
		m_pMC->Run();
}

void WMVGraph::Stop()
{
	if( m_pMC )
	    m_pMC->Stop();
}

void WMVGraph::Restart()
{
	m_curTime = 0;
	m_pMP->put_CurrentPosition(m_curTime);
}

void WMVGraph::Pause(bool bPause)
{
	if( m_pMC )
		if( bPause )
			m_pMC->Pause();
		else
			m_pMC->Run();
}

bool WMVGraph::IsFinish()
{
	if( !m_pMP ) return true;
	REFTIME curTime;
	m_pMP->get_CurrentPosition(&curTime);
	if( curTime < m_maxTime )
	{
		if( curTime > (m_maxTime - 0.02) &&
			curTime == m_curTime )
			return true;

		m_curTime = curTime;
		return false;
	}
	return true;
}

//-----------------------------------------------------------------------------
// CleanupDShow
//-----------------------------------------------------------------------------
void WMVGraph::CleanupDShow()
{
    // Shut down the graph
    if (m_pMC!=NULL)
	{
		m_pMC->Stop();
		m_pMC->Release();
		m_pMC = NULL;
	}
	if (m_pME!=NULL)
	{
		m_pME->Release();
		m_pME = NULL;
	}
    if (m_pMP!=NULL)
	{
		m_pMP->Release();
		m_pMP = NULL;
	}
	if (m_pFileSource!=NULL)
	{
		m_pFileSource->Release();
		m_pFileSource = NULL;
	}
    if (m_pGB!=NULL)
	{
		m_pGB->Release();
		m_pGB = NULL;
	}
    /*if (m_pRenderer!=NULL)
	{
		m_pRenderer->Release();
		m_pRenderer = NULL;
	}*/
}

void WMVGraph::Msg(const char *szFormat, ...)
{
	char cTmpBuffer[2048];
	va_list args;
	va_start(args, szFormat);
	_vsnprintf_s(cTmpBuffer, sizeof(cTmpBuffer), szFormat, args);
	va_end(args);

	IWMVMemoryManager::LogTrace(cTmpBuffer);
}

#endif
