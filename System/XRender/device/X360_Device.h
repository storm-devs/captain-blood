#ifdef _XBOX

#ifndef RENDER_DEVICE_XBOX360_IMPLEMENTATION
#define RENDER_DEVICE_XBOX360_IMPLEMENTATION

#include "IDevice.h"


class RenderTarget;
class RenderTargetDepth;


#define DEVICE_BACK_BUFFERS_COUNT 2

enum EdramRTMode
{
	EDRAM_ONLY_COLOR = 0,
	EDRAM_COLOR_AND_DEPTH = 1,
	EDRAM_ONLY_DEPTH = 2,

	EDRAM_FORCE_DWORD = 0x7fffffff
};


struct PredicatedTilingInfo
{
	bool hyperZDisable;

	bool bUseLastPartOfEDRAM;

	//обычный рендертаргет в EDRAM
	DWORD dwTileWidth;
	DWORD dwTileHeight;
	IDirect3DSurface9* pSurface;
	IDirect3DSurface9* pDepthSurface;


	//рендертаргет в EDRAM без MSAA
	IDirect3DSurface9* pSurface_withoutMSAA;
	IDirect3DSurface9* pDepthSurface_withoutMSAA;

};





class X360RenderDevice : public IRenderDevice
{
	bool bHudRender;

	bool bDisableMSAAForPostprocess;
	bool bInsideTiling;

	IDirect3DDevice9* d3d_device;



	long curBackBufferIndex;


	IRenderTarget* scene3d;
	IRenderTargetDepth* scene3dDepth;

	IRenderTarget* screenBuffer[DEVICE_BACK_BUFFERS_COUNT];
	IRenderTargetDepth* screenDepth;

	


	ShaderId X360RestoreEDRAM_id;
	ShaderId X360RestoreEDRAM_ColorOnly_id;
	ShaderId X360Upscale3DTo2D_id;


	IRenderTarget* current_RT;
	IRenderTargetDepth* current_RT_depth;


	RenderTartgetOptions rt_options;

	IDirect3DSurface9* m_pPrevSurface;
	IDirect3DSurface9* m_pPrevDepthSurface;

	IVariable* pEDRAM_Color;
	IVariable* pEDRAM_Depth;

	dword dwBeginSceneId;

	IDirect3DSurface9* pLastSettedSurface;
	IDirect3DSurface9* pLastSettedDepthSurface;


public:

	X360RenderDevice();
	virtual ~X360RenderDevice();

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



private:

	PredicatedTilingInfo info;

public:

	void CalculateTiling(dword Width, dword Height, D3DFORMAT Format, D3DFORMAT DepthFormat, D3DMULTISAMPLE_TYPE MultiSample, PredicatedTilingInfo& info);

	void CreateEDRAMSurfaces (PredicatedTilingInfo& info, EdramRTMode rtmode, D3DFORMAT fmt, D3DMULTISAMPLE_TYPE msaa);


	void EnableMSAA();
	void DisableMSAA();

	//На вход приходит "старый" 3d рендертаргет
	void SwitchToHUDRender(IRenderTarget* color, IRenderTargetDepth* depth, const RENDERVIEWPORT & viewPort);
	void SwitchToHUDRenderLoadingScreenHack();

	bool IsEnabledMSAA() { return !bDisableMSAAForPostprocess; }



};






#endif

#endif