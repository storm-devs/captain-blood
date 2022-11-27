#ifndef _XBOX
#ifndef XLIVESHOWERPC_H
#define XLIVESHOWERPC_H

#include "IXLiveShower.h"

class XLiveShowerPC : public IXLiveShower
{
public:
	XLiveShowerPC(IDirect3DDevice9* pD3D, void* pD3DPP);
	virtual ~XLiveShowerPC();

	virtual bool RenderFrame();

protected:
	bool m_bInited;
};

#endif
#endif
