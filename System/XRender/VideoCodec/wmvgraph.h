#ifndef _XBOX

#ifndef COOL_THINGS_VIDEO_PLAYER_WMVGRAPH
#define COOL_THINGS_VIDEO_PLAYER_WMVGRAPH

#include <windows.h>
#include <d3d9types.h>
#include "Sdk_BaseClasses\streams.h"

#include "wmvinterfaces.h"

class WMVGraph : public IWMVGraph
{
public:
	WMVGraph();
	~WMVGraph();

	virtual bool OpenFile(const char* pcVideoFile);
	virtual void CloseFile();

	virtual void Play();
	virtual void Stop();
	virtual void Restart();
	virtual void Pause(bool bPause);
	virtual void SoundOff(bool bSoundOff) { m_bSoundOff = bSoundOff; }
	virtual bool IsFinish();

private:
	HRESULT AddFilterByCLSID( IGraphBuilder *pGB,     // Pointer to the Filter Graph Manager.
							 const GUID& clsid,      // CLSID of the filter to create.
							 LPCWSTR wszName,        // A name for the filter.
							 IBaseFilter **ppFilter); // Receives a pointer to the filter.
	void CleanupDShow();
	void Msg(const char *szFormat, ...);

	IGraphBuilder*		m_pGB;          // GraphBuilder
	IMediaControl*		m_pMC;          // Media Control
	IMediaPosition*		m_pMP;          // Media Position
	IMediaEvent*		m_pME;          // Media Event
	IFileSourceFilter*	m_pFileSource;
	IBaseFilter*		m_pRenderer;    // our custom renderer

	bool				m_bSoundOff;

	REFTIME	m_maxTime;
	REFTIME	m_curTime;
};

#endif
#endif
