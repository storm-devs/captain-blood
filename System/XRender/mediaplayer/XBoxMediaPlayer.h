#ifdef _XBOX
#ifndef XBOX_MEDIA_PLAYER
#define XBOX_MEDIA_PLAYER

#include "..\\render.h"
#include <Xmedia2.h>

class XBoxMediaPlayer : public IMediaPlayer
{
public:
	XBoxMediaPlayer(IDirect3DDevice9* pD3D, const char* pcFileName, HRESULT& hr, bool bLoopPlay, bool useStreaming);
	virtual ~XBoxMediaPlayer();

	virtual bool Release();
	virtual MPResultCode RenderNextFrame();
	virtual void SetRectangle(RECT& rect);
	virtual void SetUV(float fl,float ft,float fr,float fb);
	virtual void Pause(bool isPause);

protected:
	IXMedia2XmvPlayer * xmvPlayer;
	IDirect3DDevice9* m_pD3DDevice;
	bool m_bLoopPlay;
	void * m_pvMemBuffer;
	dword m_dwMemSize;

	void ClosePlayer();
};

#endif
#endif