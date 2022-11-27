#ifdef _XBOX


#include "X360_Device.h"
#include "..\Render.h"
#include "..\RenderTargets\RenderTarget.h"
#include "..\RenderTargets\RenderTargetDepth.h"



X360RenderDevice::X360RenderDevice()
{
	bHudRender = false;

	bDisableMSAAForPostprocess = false;

	X360RestoreEDRAM_id = NULL;
	X360RestoreEDRAM_ColorOnly_id = NULL;
	X360Upscale3DTo2D_id  = NULL;

	d3d_device = NULL;

	curBackBufferIndex = 0;
	
	for (dword i = 0; i < DEVICE_BACK_BUFFERS_COUNT; i++)
	{
		screenBuffer[i] = NULL;
	}

	screenDepth = NULL;

	scene3dDepth = NULL;
	scene3d = NULL;
	

	current_RT = NULL;
	current_RT_depth = NULL;

	m_pPrevSurface = NULL;
	m_pPrevDepthSurface = NULL;

	pEDRAM_Color = NULL;
	pEDRAM_Depth = NULL;

	bInsideTiling = false;

	pLastSettedSurface = NULL;
	pLastSettedDepthSurface = NULL;

}

X360RenderDevice::~X360RenderDevice()
{
	RELEASE(d3d_device);
}

// D3D()->SetRenderTarget(0, m_LastSettedSurface);
// D3D()->SetDepthStencilSurface(m_LastSettedDepthSurface);



void X360RenderDevice::EnableMSAA()
{
	Assert (bInsideTiling == false);

	bDisableMSAAForPostprocess = false;

}

void X360RenderDevice::DisableMSAA()
{
	Assert (bInsideTiling == false);

	bDisableMSAAForPostprocess = true;
}



void X360RenderDevice::Cleanup ()
{
	pEDRAM_Color = NULL;
	pEDRAM_Depth = NULL;

	FORCERELEASE(screenDepth);

	for (dword i = 0; i < DEVICE_BACK_BUFFERS_COUNT; i++)
	{
		FORCERELEASE(screenBuffer[i]);
	}


	FORCERELEASE(scene3dDepth);
	FORCERELEASE(scene3d);

	

}

void X360RenderDevice::Init ()
{
	pEDRAM_Color = NGRender::pRS->GetTechniqueGlobalVariable("EDRAM_Color", _FL_);
	pEDRAM_Depth = NGRender::pRS->GetTechniqueGlobalVariable("EDRAM_Depth", _FL_);

	NGRender::pRS->GetShaderId("X360RestoreEDRAM", X360RestoreEDRAM_id);
	NGRender::pRS->GetShaderId("X360RestoreEDRAM_ColorOnly", X360RestoreEDRAM_ColorOnly_id);
	NGRender::pRS->GetShaderId("X360Upscale3DTo2D", X360Upscale3DTo2D_id);

	

	



	Assert (pEDRAM_Color);
	Assert (pEDRAM_Depth);

}

void X360RenderDevice::Reset ()
{

}

#ifndef STOP_DEBUG

void callBackFunctionLock(DWORD Flags, D3DBLOCKTYPE BlockType, float ClockTime, DWORD ThreadTime)
{
	//OutputDebugString("GPU Resource Lock Stall !!!\n");
}

#endif


bool X360RenderDevice::Create (IDirect3D9* d3d9, D3DPRESENT_PARAMETERS &params, long deviceIndex, RENDERSCREEN & m_Screen2DInfo, RENDERSCREEN & m_Screen3DInfo)
{
	params.Windowed = false;
	params.DisableAutoBackBuffer = TRUE;
	params.DisableAutoFrontBuffer = TRUE;
	params.EnableAutoDepthStencil = FALSE;

	//params.PresentationInterval = D3DPRESENT_INTERVAL_ONE;


	
	//1920x1080
	//1280x720

	//params.BackBufferWidth = 1920;
	//params.BackBufferHeight = 1080;




/*
	params.BackBufferWidth = 1280;
	params.BackBufferHeight = 720;
	params.MultiSampleType = D3DMULTISAMPLE_NONE;
*/


/*

// EDRAM tiles are allocated in units of 80x16 pixels at 1X multisampling,
// 80x8 at 2X and 40x8 at 4X:

#define GPU_EDRAM_TILE_WIDTH_1X                     80
#define GPU_EDRAM_TILE_HEIGHT_1X                    16

#define GPU_EDRAM_TILE_WIDTH_2X                     80
#define GPU_EDRAM_TILE_HEIGHT_2X                    8

#define GPU_EDRAM_TILE_WIDTH_4X                     40
#define GPU_EDRAM_TILE_HEIGHT_4X                    8


// An EDRAM tile size, in bytes.  Note that 64-bit surfaces have an
// allocation granularity of twice this (10240 bytes) but can have a 5120
// byte start alignment:

#define GPU_EDRAM_TILE_SIZE                         5120

// Total number of usable EDRAM tiles:

#define GPU_EDRAM_SIZE                              (10*1024*1024)

#define GPU_EDRAM_TILES                             (GPU_EDRAM_SIZE / GPU_EDRAM_TILE_SIZE) = 2048 tiles total


962 лимит


960x680 12x81

960x648


11x84 = 880x672

*/

	
	// Get the user video settings.
	XVIDEO_MODE VideoMode; 
	XMemSet( &VideoMode, 0, sizeof(XVIDEO_MODE) ); 
	XGetVideoMode( &VideoMode );

	DWORD width_3D = 0;
	DWORD height_3D = 0;


/*


1280/720 = 1.7777777777777777
1024 / 768 = 1.33333333333333333

{
	params.BackBufferWidth = 1040;
	params.BackBufferHeight = 585;
} else
{
	params.BackBufferWidth = 907;
	params.BackBufferHeight = 680;
}
*/


	if (VideoMode.fIsWideScreen)
	{
		//Такое разрешение влезает в EDRAM без тайлинга...
		params.BackBufferWidth = 1280;
		params.BackBufferHeight = 720;
		params.MultiSampleType = D3DMULTISAMPLE_NONE;

		m_Screen3DInfo.dwWidth = 1040;
		m_Screen3DInfo.dwHeight = 585;

	} else
	{
		params.BackBufferWidth = 1024;
		params.BackBufferHeight = 768;
		params.MultiSampleType = D3DMULTISAMPLE_NONE;

		m_Screen3DInfo.dwWidth = 907;
		m_Screen3DInfo.dwHeight = 680;

	}




	//params.BackBufferFormat = D3DFMT_A8R8G8B8;
	params.BackBufferFormat = D3DFMT_LE_X8R8G8B8;
	params.FrontBufferFormat = D3DFMT_LE_X8R8G8B8;



	params.RingBufferParameters.Flags = 0;
	params.RingBufferParameters.PrimarySize  = 128 * 1024; // default 32 kb
	params.RingBufferParameters.pPrimary = NULL;
	params.RingBufferParameters.SecondarySize = 4 * 1024 * 1024; // default 2Mb
	params.RingBufferParameters.pSecondary = NULL;
	params.RingBufferParameters.SegmentCount = 32; // default 32


	if (d3d9->CreateDevice(0, D3DDEVTYPE_HAL, NULL, D3DCREATE_BUFFER_2_FRAMES, &params, &d3d_device) != D3D_OK)
	{
		throw("Can't create Direct3D Device !");
	}

/*
	WORD ConvertFloatTo16BitGammasRGB( FLOAT f )        { return ConvertFloatToNBitGamma<WORD, 16, GammaFuncsRGB>( f ); }
	WORD Convert10BitGammaPWLTo16BitGammasRGB( WORD i ) { return ConvertFloatTo16BitGammasRGB( Convert10BitGammaPWLToFloat( i ) ); }


	D3DPWLGAMMA PWLGamma;
	for( UINT j = 0; j < 128; ++j )
	{
		PWLGamma.red[j].Base = PWLGamma.green[j].Base = PWLGamma.blue[j].Base = Convert10BitGammaPWLTo16BitGammasRGB( (WORD) ( j << 3 ) );
	}

	for( UINT j = 0; j < 127; ++j )
	{
		PWLGamma.red[j].Delta    = PWLGamma.red[j+1].Base - PWLGamma.red[j].Base;
		PWLGamma.green[j].Delta  = PWLGamma.green[j+1].Base - PWLGamma.green[j].Base;
		PWLGamma.blue[j].Delta   = PWLGamma.blue[j+1].Base - PWLGamma.blue[j].Base;
	}

	PWLGamma.red[127].Delta    = 0xffff - PWLGamma.red[127].Base;
	PWLGamma.green[127].Delta  = 0xffff - PWLGamma.green[127].Base;
	PWLGamma.blue[127].Delta   = 0xffff - PWLGamma.blue[127].Base;

	d3d_device->SetPWLGamma( 0, &PWLGamma );
*/



#if defined (_DEBUG) || defined(PROFILE)

	//Release - PIX profile

	//8Mb отжираем
	PIXEnableTextureTracking(0, 4 * 1024 * 1024, NULL);
#endif

#ifndef STOP_DEBUG
	d3d_device->SetBlockCallback(0, callBackFunctionLock);
#endif



	CDX8IBuffer::pD3D8 = D3D();
	CDX8VBuffer::pD3D8 = D3D();
	CDX8Texture::pD3D8 = D3D();

#if !(defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER))
	SHADER::pD3D8 = D3D();
#endif


	StateFilter::pD3D8 = D3D();


	m_Screen2DInfo.dwWidth = params.BackBufferWidth;
	m_Screen2DInfo.dwHeight = params.BackBufferHeight;
	m_Screen2DInfo.BackBufferFormat = FormatFromDX(params.BackBufferFormat);

	m_Screen3DInfo.BackBufferFormat = m_Screen2DInfo.BackBufferFormat;
	m_Screen3DInfo.bWindowed = m_Screen2DInfo.bWindowed;
	m_Screen3DInfo.fFOV = m_Screen2DInfo.fFOV;
	m_Screen3DInfo.StencilFormat = m_Screen2DInfo.StencilFormat;


	//Сюда будем рисовать 3D сцену
	scene3d = NGRender::pRS->CreateRenderTarget(m_Screen3DInfo.dwWidth, m_Screen3DInfo.dwHeight, _FL_, FMT_LE_X8R8G8B8, MSTypeFromDX(D3DMULTISAMPLE_2_SAMPLES));
	scene3dDepth = NGRender::pRS->CreateDepthStencil(m_Screen3DInfo.dwWidth, m_Screen3DInfo.dwHeight, _FL_, FMT_D24S8, MSTypeFromDX(D3DMULTISAMPLE_2_SAMPLES));
	

	//Тут реальный бекбуффер 720p
	for (dword i = 0; i < DEVICE_BACK_BUFFERS_COUNT; i++)
	{
		screenBuffer[i] = NGRender::pRS->CreateRenderTarget(params.BackBufferWidth, params.BackBufferHeight, _FL_, FMT_LE_X8R8G8B8, MSTypeFromDX(D3DMULTISAMPLE_NONE));
	}

	
	screenDepth = NGRender::pRS->CreateDepthStencil(params.BackBufferWidth, params.BackBufferHeight, _FL_, FMT_D24S8, MSTypeFromDX(D3DMULTISAMPLE_NONE));



	//Чистим текстуры черным, начальные...
	for (int i = 0; i < DEVICE_BACK_BUFFERS_COUNT; i++)
	{
		IDirect3DTexture9* pScreenPixels = (IDirect3DTexture9*)screenBuffer[i]->AsTexture()->GetBaseTexture();
		D3DLOCKED_RECT lr;
		pScreenPixels->LockRect(0, &lr, NULL, 0);
		memset (lr.pBits, 0, lr.Pitch * params.BackBufferHeight);
		pScreenPixels->UnlockRect(0);
	}

	

	SetRenderTarget(RTO_DONTOCH_CONTEXT, scene3d, scene3dDepth);

	return true;
}

void X360RenderDevice::Present ()
{
	dword dwId = NGRender::pRS->pixBeginEvent(_FL_, "::Present");

/*	
	IBaseTexture* buffTex0 = backBuffer0->AsTexture();
	//IDirect3DBaseTexture9* pDXTexture0 = NGRender::pRS->GetDXBaseTexture(buffTex0);

	IBaseTexture* buffTex1 = backBuffer1->AsTexture();
	IDirect3DBaseTexture9* pDXTexture1 = NGRender::pRS->GetDXBaseTexture(buffTex1);

	IDirect3DSurface9* rtColorOld = NULL;
	IDirect3DSurface9* rtDepthOld = NULL;
	D3D()->GetRenderTarget(0, &rtColorOld);
	D3D()->GetDepthStencilSurface(&rtDepthOld);

	D3D()->SetRenderTarget(0, finalRenderTarget);
	D3D()->SetDepthStencilSurface( NULL );


	pEDRAM_Color->SetTexture(buffTex0);
	pEDRAM_Depth->SetTexture(NULL);
	RENDERSCREEN rs = NGRender::pRS->GetScreenInfo();
	NGRender::pRS->DrawFullScreenQuad(rs.dwWidth, rs.dwHeight, X360RestoreEDRAM_ColorOnly_id);



	// Wait for the vertical blank before we resolve to the front buffer to avoid tearing.
	D3D()->SynchronizeToPresentationInterval();


	D3DVECTOR4 ClearColor;
	ClearColor.x = 0.0f;
	ClearColor.y = 0.0f;
	ClearColor.z = 0.0f;
	ClearColor.w = 0.0f;

	// Resolve the rendered scene back to the front buffer.
	D3D()->Resolve( D3DRESOLVE_RENDERTARGET0 | D3DRESOLVE_ALLFRAGMENTS,
		NULL,
		pDXTexture1,
		NULL,
		0, 0, 
		&ClearColor, 
		1.0f, 0, NULL );
*/

	

	D3D()->SetShaderGPRAllocation(0, 0, 0);

	EnableMSAA();


	IBaseTexture* buffTex1 = screenBuffer[curBackBufferIndex]->AsTexture();
	IDirect3DBaseTexture9* pDXTexture1 = (IDirect3DBaseTexture9*)buffTex1->GetBaseTexture();

	curBackBufferIndex = curBackBufferIndex + 1;
	if (curBackBufferIndex >= DEVICE_BACK_BUFFERS_COUNT)
	{
		curBackBufferIndex = 0;
	}


	//Меняем...
	rt_options = RTO_DONTOCH_CONTEXT;
	current_RT = scene3d;
	current_RT_depth = scene3dDepth;
	bHudRender = false;


	D3D()->SynchronizeToPresentationInterval();
	D3D()->Swap( pDXTexture1, NULL );
	

	//D3D()->Present(NULL, NULL, NULL, NULL);


	rt_options = RTO_DONTOCH_CONTEXT;

/*
	D3D()->SetRenderTarget(0, rtColorOld);
	D3D()->SetDepthStencilSurface(rtDepthOld);
*/

	NGRender::pRS->pixEndEvent(_FL_, dwId);
}

void X360RenderDevice::BeginScene ()
{
	static char tmp[8192];

	RENDERSURFACE_DESC dsc;

	if (current_RT)
	{
		current_RT->GetDesc(&dsc);
	} else
	{
		if (current_RT_depth)
		{
			current_RT_depth->GetDesc(&dsc);
		} else
		{
			dsc.Width = 0;
			dsc.Height = 0;
		}
	}

	dwBeginSceneId = NGRender::pRS->pixBeginEvent(_FL_, "::BeginScene(%dx%d)", dsc.Width, dsc.Height);

	if (rt_options == RTO_RESTORE_CONTEXT || rt_options == RTO_RESTORE_CONTEXT_COLORONLY)
	{
		if (current_RT)
		{
			IBaseTexture* SavedEDRAM = current_RT->AsTexture();

			pEDRAM_Color->SetTexture(SavedEDRAM);

			if (current_RT_depth && rt_options == RTO_RESTORE_CONTEXT)
			{
				RenderTargetDepth* rtd = (RenderTargetDepth*)current_RT_depth;
				pEDRAM_Depth->SetTexture(rtd->AsTexture());
			} else
			{
				pEDRAM_Depth->ResetTexture();
			}
			
			RENDERSCREEN rs;
			if (bHudRender)
			{
				rs = NGRender::pRS->GetScreenInfo2D();
			} else
			{
				rs = NGRender::pRS->GetScreenInfo3D();
			}
			 

			if (current_RT_depth && rt_options == RTO_RESTORE_CONTEXT)
			{
				NGRender::pRS->DrawFullScreenQuad(rs.dwWidth, rs.dwHeight, X360RestoreEDRAM_id);
			} else
			{
				NGRender::pRS->DrawFullScreenQuad(rs.dwWidth, rs.dwHeight, X360RestoreEDRAM_ColorOnly_id);
			}
		}
	}


	NGRender::pRS->pixEndEvent(_FL_, dwBeginSceneId);

	bInsideTiling = true;
	
}

void X360RenderDevice::EndScene (IBaseTexture* pDestiantionTexture, bool bSkipAnyWork, bool bDontResolveDepthOnX360)
{
	D3DVECTOR4 clearColor;
	clearColor.x = 0.0f;
	clearColor.y = 0.0f;
	clearColor.z = 0.0f;
	clearColor.w = 0.0f;

	if (bSkipAnyWork)
	{
		return;
	}


	
	if (current_RT_depth && bDontResolveDepthOnX360 == false)
	{
		IDirect3DTexture9* depth_tex = (IDirect3DTexture9*)((RenderTargetDepth*)current_RT_depth)->AsTexture()->GetBaseTexture();;

		

		D3D()->Resolve( D3DRESOLVE_DEPTHSTENCIL | D3DRESOLVE_FRAGMENT0, NULL, depth_tex, NULL, 0, 0, &clearColor, 1.0f, 0L, NULL);
	}
	




	if (current_RT)
	{

		IDirect3DTexture9* tex = (IDirect3DTexture9*)current_RT->AsTexture()->GetBaseTexture();

		if (pDestiantionTexture)
		{
			IDirect3DTexture9* tex2 = (IDirect3DTexture9*)pDestiantionTexture->GetBaseTexture();
			D3D()->Resolve( D3DRESOLVE_RENDERTARGET0 | D3DRESOLVE_ALLFRAGMENTS, NULL, tex2, NULL, 0, 0, &clearColor, 1.0f, 0L, NULL);
		}

		D3D()->Resolve(D3DRESOLVE_RENDERTARGET0 | D3DRESOLVE_ALLFRAGMENTS, NULL, tex, NULL, 0, 0, &clearColor, 1.0f, 0x0, NULL);



		//------------------------------------
		//надо построить мип цепоцки если это необходимо
		long mipCount = ((RenderTarget*)current_RT)->GetMipCount();


		if (mipCount > 1)
		{
			dword dwMipId = NGRender::pRS->pixBeginEvent(_FL_, "::Build render target mipmaps");

			RENDERSURFACE_DESC desc;
			current_RT->GetDesc(&desc);

			UINT mipWidth = desc.Width;
			UINT mipHeight = desc.Height;

			D3DVIEWPORT9 oldVp;
			D3D()->GetViewport(&oldVp);


			XGTEXTURE_DESC Desc;
			D3DVIEWPORT9 mipVp = oldVp;


			for (long i = 1; i < mipCount; i++)
			{

				//Рисум мип в EDRAM
				IBaseTexture* sourceMip = current_RT->AsTexture();
				IDirect3DTexture9* tex = (IDirect3DTexture9*)sourceMip->GetBaseTexture();

				XGGetTextureDesc( tex, i, &Desc );
				mipVp.Width = Desc.Width;
				mipVp.Height = Desc.Height;
				D3D()->SetViewport( &mipVp );


				D3D()->SetSamplerState( 0, D3DSAMP_MINMIPLEVEL, i - 1 );

				//D3D()->Clear(0, null, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0x30405060, 1.0, 0);


				pEDRAM_Color->SetTexture(sourceMip);
				pEDRAM_Depth->ResetTexture();


				NGRender::pRS->DrawFullScreenQuad(mipWidth, mipHeight, X360RestoreEDRAM_ColorOnly_id);


				//Ресолвим в мип текстуры из EDRAM
				
				D3D()->Resolve(D3DRESOLVE_RENDERTARGET0 | D3DRESOLVE_ALLFRAGMENTS, NULL, tex, NULL, i, 0, &clearColor, 1.0f, 0x0, NULL);
			}

			D3D()->SetViewport( &oldVp );

			D3D()->SetSamplerState( 0, D3DSAMP_MINMIPLEVEL, 13 );




			NGRender::pRS->pixEndEvent(_FL_, dwMipId);
		}
	}





	bInsideTiling = false;


/*
	//Возвращаем старые...
	D3D()->SetRenderTarget(0, m_pPrevSurface);
	D3D()->SetDepthStencilSurface(m_pPrevDepthSurface);
*/
	

}

void X360RenderDevice::SetRenderTarget (RenderTartgetOptions options, IRenderTarget* renderTarget, IRenderTargetDepth* renderTargetDepth)
{
	//

	dword dwId = NGRender::pRS->pixBeginEvent(_FL_, "SetRenderTarget (RenderTartgetOptions = %d)", options);


	rt_options = options;
	current_RT = renderTarget;
	current_RT_depth = renderTargetDepth;



/*
	//Получаем старые с рендера
	D3D()->GetRenderTarget(0, &m_pPrevSurface);
	D3D()->GetDepthStencilSurface(&m_pPrevDepthSurface);
*/


	if (renderTarget && renderTargetDepth)
	{
		info = ((RenderTarget*)current_RT)->GetTileInfoWithDepth();
	} else
	{
		if (renderTarget && renderTargetDepth == NULL)
		{
			info = ((RenderTarget*)current_RT)->GetTileInfo();
		} else
		{
			if (renderTarget == NULL && renderTargetDepth)
			{
				info = ((RenderTargetDepth*)current_RT_depth)->GetTileInfoDepthOnly();
			} else
			{
				//Ничего делать не надо
				NGRender::pRS->pixEndEvent(_FL_, dwId);
				return;
			}
		}

	}

	if (bDisableMSAAForPostprocess == false)
	{
		D3D()->SetRenderTarget(0, info.pSurface);
		D3D()->SetDepthStencilSurface(info.pDepthSurface);
	} else
	{
		D3D()->SetRenderTarget(0, info.pSurface_withoutMSAA);
		D3D()->SetDepthStencilSurface(info.pDepthSurface_withoutMSAA);
	}


	pLastSettedSurface = info.pSurface;
	pLastSettedDepthSurface = info.pDepthSurface;

	NGRender::pRS->pixEndEvent(_FL_, dwId);
}



void X360RenderDevice::GetRenderTarget (IRenderTarget** renderTarget, IRenderTargetDepth** renderTargetDepth)
{
	if (renderTarget)
	{
		*renderTarget = current_RT;
	}

	if (renderTargetDepth)
	{
		*renderTargetDepth = current_RT_depth;
	}
}

IDirect3DDevice9* X360RenderDevice::D3D()
{
	return d3d_device;
}

bool X360RenderDevice::DisableStateManager ()
{
	return false;
}

void X360RenderDevice::CalculateTiling(dword Width, dword Height, D3DFORMAT Format, D3DFORMAT DepthFormat, D3DMULTISAMPLE_TYPE MultiSample, PredicatedTilingInfo& info)
{
	// Check need for predicated tiling
	info.dwTileWidth  = Width;
	info.dwTileHeight = Height;

	DWORD dwNumBackBufferEDRAMTiles = 0;

	if (Format != D3DFMT_UNKNOWN)
	{
		dwNumBackBufferEDRAMTiles = XGSurfaceSize( info.dwTileWidth, info.dwTileHeight, Format, MultiSample );
	}

	DWORD dwNumZBufferEDRAMTiles  = 0;
	if (DepthFormat != D3DFMT_UNKNOWN)
	{
		dwNumZBufferEDRAMTiles = XGSurfaceSize( info.dwTileWidth, info.dwTileHeight, DepthFormat, MultiSample );
	}

	//Такой RT лезет в нижную часть EDRAM, можно не портить основную EDRAM память...
	if (dwNumBackBufferEDRAMTiles + dwNumZBufferEDRAMTiles < 64)
	{
		info.bUseLastPartOfEDRAM = true;
	} else
	{
		info.bUseLastPartOfEDRAM = false;
	}

	//Обычный RT
	if (dwNumBackBufferEDRAMTiles + dwNumZBufferEDRAMTiles < GPU_EDRAM_TILES)
	{
		DWORD HyperZTiles = XGHierarchicalZSize(Width, Height, MultiSample);
		Assert (HyperZTiles <= GPU_HIERARCHICAL_Z_TILES);
		
		if (HyperZTiles <= GPU_HIERARCHICAL_Z_TILES)
		{
			info.hyperZDisable = false;
		} else
		{
			info.hyperZDisable = true;
		}

		return;
	}


	//Predicated tiling is prohibited by Joker due to march 2009 TCR 
	Assert(false);
}


void X360RenderDevice::CreateEDRAMSurfaces (PredicatedTilingInfo& info, EdramRTMode rtmode, D3DFORMAT fmt, D3DMULTISAMPLE_TYPE msaa)
{
	D3DSURFACE_PARAMETERS SurfaceParameters = {0};

	if (info.bUseLastPartOfEDRAM == false)
	{
		SurfaceParameters.Base = 0;
	} else
	{
		SurfaceParameters.Base = GPU_EDRAM_TILES - XGSurfaceSize( info.dwTileWidth, info.dwTileHeight, fmt, msaa );
		if (rtmode == EDRAM_COLOR_AND_DEPTH)
		{
			SurfaceParameters.Base = SurfaceParameters.Base - XGSurfaceSize( info.dwTileWidth, info.dwTileHeight, D3DFMT_D24X8, msaa );
		}
	}

	

	HRESULT hr = D3D_OK;

	if (rtmode == EDRAM_ONLY_COLOR || rtmode == EDRAM_COLOR_AND_DEPTH)
	{
		hr = D3D()->CreateRenderTarget(info.dwTileWidth, info.dwTileHeight, fmt, msaa, 0, FALSE, &info.pSurface, &SurfaceParameters);
		hr = D3D()->CreateRenderTarget(info.dwTileWidth, info.dwTileHeight, fmt, D3DMULTISAMPLE_NONE, 0, FALSE, &info.pSurface_withoutMSAA, &SurfaceParameters);
	} else
	{
		info.pSurface = NULL;
		info.pSurface_withoutMSAA = NULL;
	}


	if (rtmode == EDRAM_ONLY_DEPTH || rtmode == EDRAM_COLOR_AND_DEPTH)
	{
		if (rtmode == EDRAM_COLOR_AND_DEPTH)
		{
			SurfaceParameters.Base = SurfaceParameters.Base + XGSurfaceSize( info.dwTileWidth, info.dwTileHeight, fmt, msaa );

			//Assert (sizeOf_MSAAHACK == SurfaceParameters.Base);
		}


		if (info.hyperZDisable)
		{
			Assert (false);
			SurfaceParameters.HierarchicalZBase = 0;
		} else
		{
			SurfaceParameters.HierarchicalZBase = 0;
		}

		hr = D3D()->CreateDepthStencilSurface(info.dwTileWidth, info.dwTileHeight, D3DFMT_D24X8, msaa, 0, FALSE, &info.pDepthSurface, &SurfaceParameters);
		hr = D3D()->CreateDepthStencilSurface(info.dwTileWidth, info.dwTileHeight, D3DFMT_D24X8, D3DMULTISAMPLE_NONE, 0, FALSE, &info.pDepthSurface_withoutMSAA, &SurfaceParameters);
	} else
	{
		info.pDepthSurface = NULL;
		info.pDepthSurface_withoutMSAA = NULL;
	}
}


void X360RenderDevice::ResolveColor (IBaseTexture * dest)
{
	if (current_RT && dest)
	{
		HRESULT hr;
		RENDERSURFACE_DESC desc1;
		current_RT_depth->GetDesc(&desc1);
		
		dword dwTexWidth = dest->GetWidth();
		dword dwTexHeight = dest->GetHeight();

		Assert(desc1.Width == dwTexWidth);
		Assert(desc1.Height == dwTexHeight);

		IDirect3DTexture9* color_tex = (IDirect3DTexture9*)dest->GetBaseTexture();

		D3D()->Resolve( D3DRESOLVE_RENDERTARGET0 | D3DRESOLVE_ALLFRAGMENTS, NULL, color_tex, NULL, 0, 0, NULL, 1.0f, 0L, NULL);
	}


}

void X360RenderDevice::ResolveDepth (IRenderTargetDepth* dest)
{
	if (current_RT_depth && dest)
	{
		HRESULT hr;
		RENDERSURFACE_DESC desc1;
		RENDERSURFACE_DESC desc2;
		current_RT_depth->GetDesc(&desc1);
		dest->GetDesc(&desc2);

		if (desc1.Width != desc2.Width || desc1.Height != desc2.Height)
		{
			int a = 0;
		}

		Assert(desc1.Width == desc2.Width);
		Assert(desc1.Height == desc2.Height);

		//Assert(desc1.MultiSampleType == desc2.MultiSampleType);
		//Assert(desc1.MultiSampleQuality == desc2.MultiSampleQuality);

		IDirect3DTexture9* depth_tex = (IDirect3DTexture9*)((RenderTargetDepth*)dest)->AsTexture()->GetBaseTexture();

		D3D()->Resolve( D3DRESOLVE_DEPTHSTENCIL | D3DRESOLVE_FRAGMENT0, NULL, depth_tex, NULL, 0, 0, NULL, 1.0f, 0L, NULL);
	}

}


void X360RenderDevice::SwitchToHUDRenderLoadingScreenHack()
{
	if (bHudRender)
	{
		return;
	}


	bHudRender = true;
	SetRenderTarget(RTO_DONTOCH_CONTEXT, screenBuffer[curBackBufferIndex], screenDepth);
}

void X360RenderDevice::SwitchToHUDRender(IRenderTarget* color, IRenderTargetDepth* depth, const RENDERVIEWPORT & viewPort)
{
	if (bHudRender)
	{
		return;
	}


	bHudRender = true;
	//Надо поставить...
	//screenBuffer[curBackBufferIndex]

	NGRender::pRS->SetRenderTarget(RTO_DONTOCH_CONTEXT, screenBuffer[curBackBufferIndex], screenDepth);

	const RENDERSCREEN & rs = NGRender::pRS->GetScreenInfo2D();
	RENDERVIEWPORT vp2D;
	vp2D.X = 0;
	vp2D.Y = 0;
	vp2D.Width = rs.dwWidth;
	vp2D.Height = rs.dwHeight;
	vp2D.MinZ = 0.0f;
	vp2D.MaxZ = 1.0f;
	NGRender::pRS->SetViewport(vp2D);

	if (color)
	{
		IBaseTexture* textureToUpscale = color->AsTexture();
		pEDRAM_Color->SetTexture(textureToUpscale);
		pEDRAM_Depth->ResetTexture();
		
		NGRender::pRS->DrawFullScreenQuad(rs.dwWidth, rs.dwHeight, X360Upscale3DTo2D_id);
	}
}


#endif