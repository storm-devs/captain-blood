#ifndef SHADOWS_SERVICE_INTERFACE
#define SHADOWS_SERVICE_INTERFACE

#include "core.h"
#include "render.h"

class IShadowsService : public Service
{

public:
	IShadowsService () {};
	virtual ~IShadowsService () {};

	virtual bool Init() = 0;

	virtual IS_XBOX(IRenderTargetDepth *, IRenderTarget *) GetShadowMap () = 0;
	virtual IRenderTarget* GetPostProcessTexture () = 0;

#ifndef _XBOX
	virtual IRenderTargetDepth* GetZStencil () = 0;
#endif

//#ifdef _XBOX
	virtual void SetLinearZAsRT() = 0;
	virtual IRenderTarget * GetLinearZRT() = 0;
//#endif

	virtual void SetShadowMapViewPort () = 0;
	virtual void SetPostProcessTexViewPort () = 0;

	virtual void SetPostProcessAsRT (bool bRestoreContext) = 0;
	
	virtual dword GetShadowMapRes () = 0;
	virtual dword GetPostProcessTexWidth () = 0;
	virtual dword GetPostProcessTexHeight () = 0;

	virtual bool  IsPostProcessEnabled() = 0;

	virtual void SetShadowColor(Color _dwShadowColor) = 0;
	virtual Color GetShadowColor() = 0;

	virtual bool  IsActiveShadowMap() = 0;

	virtual void  NotifyActivation() = 0;
	virtual void  NotifyDeactivation() = 0;

	virtual void  Blur(dword dwLevel) = 0;

	virtual void ClearScreen() = null;
};


#endif