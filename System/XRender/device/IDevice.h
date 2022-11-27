#ifndef RENDER_DEVICE_INTERFACE_H
#define RENDER_DEVICE_INTERFACE_H

#include "..\GraphicsApi.h"
#include "..\..\..\common_h\defines.h"
#include "..\..\..\common_h\core.h"
#include "..\..\..\common_h\render.h"



class IRenderTarget;
class IRenderTargetDepth;


class IRenderDevice
{


public:

	IRenderDevice() {};
	virtual ~IRenderDevice() {};

	virtual void Lost () {};
	virtual void Reset () {};

	virtual bool Create (IDirect3D9* d3d9, D3DPRESENT_PARAMETERS &params, long deviceIndex, RENDERSCREEN & m_Screen2DInfo, RENDERSCREEN & m_Screen3DInfo) = 0;

	virtual void Present () = 0;

	virtual void BeginScene () = 0;

	virtual void EndScene (IBaseTexture* pDestiantionTexture, bool bSkipAnyWork, bool bDontResolveDepthOnX360) = 0;

	virtual void SetRenderTarget (RenderTartgetOptions options, IRenderTarget* renderTarget, IRenderTargetDepth* renderTargetDepth) = 0;

	virtual void GetRenderTarget (IRenderTarget** renderTarget, IRenderTargetDepth** renderTargetDepth) = 0;

	virtual IDirect3DDevice9* D3D() = 0;

	virtual bool DisableStateManager () = 0;

	virtual void Cleanup () = 0;
	virtual void Init () = 0;

	virtual void ResolveDepth (IRenderTargetDepth* dest) = 0;
	virtual void ResolveColor (IBaseTexture * dest) = 0;



};






#endif
