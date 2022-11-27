
#ifndef SHADOWS_SERVICE
#define SHADOWS_SERVICE

#include "..\..\..\Common_h\ishadows.h"


class ShadowsService : public IShadowsService
{
public:

	IRender * pRS;

	IRenderTargetDepth * pDepthSurface, * pDepthSurfaceRT;
		
	IRenderTarget * pPostProcessTex;
	IRenderTarget * pPostProcessRT, * pPostProcessRT2;		

	IVariable	* pIBlurTexture, * pIBlurTexSize;

	dword dwPSSMShadowMapRes;
	
	dword dwPostProcessTexWidth;
	dword dwPostProcessTexHeight;

	Color dwShadowColor;

	bool bPostProcessEnabled;
	bool bFirstTime;
	bool bUsePCF;
	bool bIsAnyActiveShadowMap;

public:

	ShadowsService();
	virtual ~ShadowsService();	

	virtual bool Init();

	IS_XBOX(IRenderTargetDepth *, IRenderTarget *) GetShadowMap();

#ifndef _XBOX
	virtual IRenderTargetDepth* GetZStencil();
#endif

	virtual void SetShadowMapViewPort();
	virtual void SetPostProcessTexViewPort();

	virtual void SetPostProcessAsRT (bool bRestoreContext);
//#ifdef _XBOX
	virtual void SetLinearZAsRT();
	virtual IRenderTarget * GetLinearZRT() { return m_linearZRT; }
//#endif

	virtual dword GetShadowMapRes();	

	virtual bool  IsPostProcessEnabled() { return bPostProcessEnabled; };
	virtual dword GetPostProcessTexWidth ();	
	virtual dword GetPostProcessTexHeight ();	

	virtual void SetShadowColor(Color _dwShadowColor) { dwShadowColor = _dwShadowColor; };
	virtual Color GetShadowColor() { return dwShadowColor; };

	virtual void StartFrame(float dltTime);
	virtual void EndFrame(float dltTime);

	virtual bool  IsActiveShadowMap();

	virtual void  NotifyActivation() { bIsAnyActiveShadowMap = true; };
	virtual void  NotifyDeactivation() { /*bIsAnyActiveShadowMap = false;*/ };	

	virtual void  ClearPostProcessTexture();
	virtual IRenderTarget * GetPostProcessTexture();
	virtual void  Blur(dword dwLevel);

	virtual bool CreateRenderTargets(dword resolution);

	virtual void ClearScreen();
private:
	enum PPTClearStatus
	{
		ppt_dirty = 0,
		ppt_cleared = 1,
		ppt_need2clear = 2
	};

	IS_XBOX(IRenderTargetDepth *, IRenderTarget *) pZTexture;
	IRenderTarget		* m_linearZRT;
	bool				bOff;
	bool				bClearedPrevFrame;
	bool				bAlreadyCleared;
	ShaderId			m_shaderBlur;
	PPTClearStatus		pptClearStatus;

	void Error(long id, const char * errorEnglish);
};


#endif