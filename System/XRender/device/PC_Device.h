#ifndef _XBOX

#ifndef RENDER_DEVICE_DX9_IMPLEMENTATION
#define RENDER_DEVICE_DX9_IMPLEMENTATION

#include "IDevice.h"


class RenderTarget;
class RenderTargetDepth;


class DX9RenderDevice : public IRenderDevice
{


	bool m_bMaximimize;
	bool m_bExpand;

public:


	DX9RenderDevice(bool bMaximimize, bool bExpand);
	virtual ~DX9RenderDevice();

	virtual void Lost ();
	virtual void Reset ();

	virtual bool Create (IDirect3D9* d3d9, D3DPRESENT_PARAMETERS &params, long deviceIndex, RENDERSCREEN & m_Screen2DInfo, RENDERSCREEN & m_Screen3DInfo);

	virtual void Present ();

	virtual void BeginScene ();

	virtual void EndScene (IBaseTexture* pDestiantionTexture, bool bSkipAnyWork, bool bDontResolveDepthOnX360);

	virtual void SetRenderTarget (RenderTartgetOptions options, IRenderTarget* renderTarget, IRenderTargetDepth* renderTargetDepth);

	virtual void GetRenderTarget (IRenderTarget** renderTarget, IRenderTargetDepth** renderTargetDepth);

	virtual IDirect3DDevice9* D3D();

	virtual bool DisableStateManager ();

	virtual void Cleanup ();

	virtual void Init ();

	virtual void ResolveDepth (IRenderTargetDepth* dest);
	virtual void ResolveColor (IBaseTexture * dest);


	void AdjustWindow(long x, long y, bool bExpand, bool bMaximize, D3DPRESENT_PARAMETERS &params);

private:

	bool bPerfHudDevice;

	IDirect3DDevice9* d3d_device;

	IRenderTarget* current_RT;
	IRenderTargetDepth* current_RT_depth;


	RenderTarget* default_RT;
	RenderTargetDepth* default_RT_depth;

};






#endif

#endif