#ifndef _XBOX

#include "PC_Device.h"
#include "..\Render.h"
#include "..\RenderTargets\RenderTarget.h"
#include "..\RenderTargets\RenderTargetDepth.h"


DX9RenderDevice::DX9RenderDevice(bool bMaximimize, bool bExpand)
{
	m_bMaximimize = bMaximimize;
	m_bExpand = bExpand;

	bPerfHudDevice = false;
	d3d_device = NULL;

	current_RT = NULL;
	current_RT_depth = NULL;
	default_RT = NULL;
	default_RT_depth = NULL;
}

DX9RenderDevice::~DX9RenderDevice()
{
	RELEASE_D3D(d3d_device, 0);
}

void DX9RenderDevice::Cleanup ()
{
	FORCERELEASE (default_RT);
	FORCERELEASE (default_RT_depth);

}

void DX9RenderDevice::Init ()
{

}

void DX9RenderDevice::Lost ()
{
	if (default_RT)
	{
		default_RT->OnLostDevice();
	}


	if (default_RT_depth)
	{
		default_RT_depth->OnLostDevice();
	}
}

void DX9RenderDevice::Reset ()
{
	IDirect3DSurface9* m_pDefaultSurface = NULL;
	IDirect3DSurface9* m_pDefaultDepthSurface = NULL;

	HRESULT hr = D3D()->GetRenderTarget(0, &m_pDefaultSurface);
	Assert(hr == D3D_OK);

	hr = D3D()->GetDepthStencilSurface(&m_pDefaultDepthSurface);
	Assert(hr == D3D_OK);


	current_RT = NULL;
	current_RT_depth = NULL;


	default_RT->CreateFromDX(m_pDefaultSurface, true);

	default_RT_depth->CreateFromDX(m_pDefaultDepthSurface, true);

	current_RT = default_RT;
	current_RT_depth = default_RT_depth;



	if (m_pDefaultDepthSurface != NULL)
	{
		hr = D3D()->Clear( 0, NULL, D3DCLEAR_ZBUFFER | D3DCLEAR_TARGET, 0x0, 1.0, 0x0);
		Assert(hr == D3D_OK);
	} else
	{
		hr = D3D()->Clear( 0, NULL, D3DCLEAR_TARGET, 0x0, 1.0, 0x0);
		Assert(hr == D3D_OK);
	}



}

bool DX9RenderDevice::DisableStateManager ()
{
	return false;
	//return true;
}

bool DX9RenderDevice::Create (IDirect3D9* d3d9, D3DPRESENT_PARAMETERS &params, long deviceIndex, RENDERSCREEN & m_Screen2DInfo, RENDERSCREEN & m_Screen3DInfo)
{
	d3d_device = NULL;

	UINT AdapterToUse = deviceIndex;
	D3DDEVTYPE DeviceType = D3DDEVTYPE_HAL;

#ifndef STOP_DEBUG
#ifndef _XBOX
	// Look for 'NVIDIA NVPerfHUD' adapter
	// If it is present, override default settings
	for (UINT Adapter = 0; Adapter < d3d9->GetAdapterCount(); Adapter++)
	{
		D3DADAPTER_IDENTIFIER9 Identifier;
		HRESULT Res;
		Res = d3d9->GetAdapterIdentifier(Adapter, 0, &Identifier);
		api->Trace("Adapter : '%s'", Identifier.Description);

		
		if (strcmp(Identifier.Description, "NVIDIA NVPerfHUD") == 0 || strcmp(Identifier.Description, "NVIDIA PerfHUD") == 0)
		{
			api->Trace("Use : NVIDIA NVPerfHUD device");
			bPerfHudDevice = true;
			AdapterToUse = Adapter;
			DeviceType = D3DDEVTYPE_REF;
			break;
		}
	}
#endif
#endif


#ifndef _XBOX


	if (params.Windowed)
	{
		HWND winHandle = (HWND)api->Storage().GetLong("system.hwnd");


		WINDOWINFO savedInfo;
		GetWindowInfo(winHandle, &savedInfo);


		if (m_bMaximimize)
		{
			MONITORINFO miMonInfo = { 0 };  
			miMonInfo.cbSize = sizeof( miMonInfo );   
			HMONITOR hMonitor = MonitorFromWindow( winHandle, MONITOR_DEFAULTTONEAREST );   
			GetMonitorInfo( hMonitor, &miMonInfo ); 

			SetWindowPos(winHandle, HWND_NOTOPMOST, miMonInfo.rcMonitor.left, miMonInfo.rcMonitor.top, miMonInfo.rcMonitor.right-miMonInfo.rcMonitor.left, miMonInfo.rcMonitor.bottom-miMonInfo.rcMonitor.top, SWP_NOOWNERZORDER);
		} else
		{
			RECT r;
			r.left = 0;
			r.right = r.left + params.BackBufferWidth;
			r.top = 0;
			r.bottom = r.top + params.BackBufferHeight;



			DWORD dwFlags = DS_MODALFRAME | DS_FIXEDSYS | WS_CAPTION | WS_BORDER | WS_POPUP | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
			dwFlags = dwFlags & ~WS_THICKFRAME;
			DWORD dwExFlags = WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE;

			AdjustWindowRect(&r, dwFlags, 0);
		
			SetWindowLong(winHandle, GWL_STYLE, dwFlags);
			SetWindowLong(winHandle, GWL_EXSTYLE, dwExFlags);
			SetWindowPos(winHandle, HWND_NOTOPMOST, r.left, r.top, r.right-r.left, r.bottom-r.top, SWP_NOOWNERZORDER);

			if (m_bExpand)
			{
				SetWindowLong(winHandle, GWL_STYLE, dwFlags | WS_MAXIMIZEBOX);
				ShowWindow(winHandle, SW_MAXIMIZE);
			}
		}


		WINDOWINFO info;
		info.cbSize = sizeof(WINDOWINFO);
		GetWindowInfo(winHandle, &info);

		DWORD dwW = info.rcClient.right - info.rcClient.left;
		DWORD dwH = info.rcClient.bottom - info.rcClient.top;

		params.BackBufferWidth = dwW;
		params.BackBufferHeight = dwH;


		SetWindowLong(winHandle, GWL_STYLE, savedInfo.dwStyle);
		SetWindowLong(winHandle, GWL_EXSTYLE, savedInfo.dwExStyle);
		SetWindowPos(winHandle, HWND_NOTOPMOST, savedInfo.rcWindow.left, savedInfo.rcWindow.top, savedInfo.rcWindow.right-savedInfo.rcWindow.left, savedInfo.rcWindow.bottom-savedInfo.rcWindow.top, SWP_NOOWNERZORDER);

		
		
	}


#endif



	if (d3d9->CreateDevice(AdapterToUse, DeviceType, (HWND)api->Storage().GetLong("system.hwnd"),  D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE | D3DCREATE_MULTITHREADED, &params, &d3d_device) != D3D_OK)
	{
		if (d3d9->CreateDevice(AdapterToUse, DeviceType, (HWND)api->Storage().GetLong("system.hwnd"), D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED, &params, &d3d_device) != D3D_OK)
		{
			api->Trace("Can't create Direct3D Device !");
			return false;
		}
	}

	CDX8IBuffer::pD3D8 = D3D();
	CDX8VBuffer::pD3D8 = D3D();
	CDX8Texture::pD3D8 = D3D();
	Shader::pD3D8 = D3D();
	StateFilter::pD3D8 = D3D();


	IDirect3DSurface9* m_pDefaultSurface = NULL;
	IDirect3DSurface9* m_pDefaultDepthSurface = NULL;

	HRESULT hr = D3D()->GetRenderTarget(0, &m_pDefaultSurface);
	Assert(hr == D3D_OK);
	
	hr = D3D()->GetDepthStencilSurface(&m_pDefaultDepthSurface);
	Assert(hr == D3D_OK);


	current_RT = NULL;
	current_RT_depth = NULL;


	default_RT = NEW RenderTarget;
	default_RT->CreateFromDX(m_pDefaultSurface, false);

	default_RT_depth = NEW RenderTargetDepth;
	default_RT_depth->CreateFromDX(m_pDefaultDepthSurface, false);


	current_RT = default_RT;
	current_RT_depth = default_RT_depth;


	if (m_pDefaultDepthSurface != NULL)
	{
		hr = D3D()->Clear( 0, NULL, D3DCLEAR_ZBUFFER | D3DCLEAR_TARGET, 0x0, 1.0, 0x0);
		Assert(hr == D3D_OK);
	} else
	{
		hr = D3D()->Clear( 0, NULL, D3DCLEAR_TARGET, 0x0, 1.0, 0x0);
		Assert(hr == D3D_OK);
	}

	m_Screen2DInfo.dwWidth = params.BackBufferWidth;
	m_Screen2DInfo.dwHeight = params.BackBufferHeight;
	m_Screen2DInfo.BackBufferFormat = FormatFromDX(params.BackBufferFormat);

	m_Screen3DInfo = m_Screen2DInfo;

	return true;
}



void DX9RenderDevice::AdjustWindow(long x, long y, bool bExpand, bool bMaximize, D3DPRESENT_PARAMETERS &params)
{
	if (params.Windowed == false) return;

	HWND winHandle = (HWND)api->Storage().GetLong("system.hwnd");

	if (bMaximize)
	{
		//SetWindowLong(winHandle, GWL_STYLE, WS_POPUP | WS_VISIBLE);

		MONITORINFO miMonInfo = { 0 };  
		miMonInfo.cbSize = sizeof( miMonInfo );   
		HMONITOR hMonitor = MonitorFromWindow( winHandle, MONITOR_DEFAULTTONEAREST );   
		GetMonitorInfo( hMonitor, &miMonInfo ); 

		//miMonInfo.

		SetWindowPos(winHandle, HWND_NOTOPMOST, miMonInfo.rcMonitor.left, miMonInfo.rcMonitor.top, miMonInfo.rcMonitor.right-miMonInfo.rcMonitor.left, miMonInfo.rcMonitor.bottom-miMonInfo.rcMonitor.top, SWP_NOOWNERZORDER);

	} else
	{
		RECT r;
		r.left = x;
		r.right = r.left + params.BackBufferWidth;
		r.top = y;
		r.bottom = r.top + params.BackBufferHeight;

		DWORD dwFlags = DS_MODALFRAME | DS_FIXEDSYS | WS_CAPTION | WS_BORDER | WS_POPUP | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
		dwFlags = dwFlags & ~WS_THICKFRAME;
		DWORD dwExFlags = WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE;

		AdjustWindowRect(&r, dwFlags, 0);

		SetWindowLong(winHandle, GWL_STYLE, dwFlags);
		SetWindowLong(winHandle, GWL_EXSTYLE, dwExFlags);
		SetWindowPos(winHandle, HWND_NOTOPMOST, r.left, r.top, r.right-r.left, r.bottom-r.top, SWP_NOOWNERZORDER);

		if (bExpand)
		{
			SetWindowLong(winHandle, GWL_STYLE, dwFlags | WS_MAXIMIZEBOX);
			ShowWindow(winHandle, SW_MAXIMIZE);
		}
	}
	


	WINDOWINFO info;
	info.cbSize = sizeof(WINDOWINFO);
	GetWindowInfo(winHandle, &info);

	DWORD dwW = info.rcClient.right - info.rcClient.left;
	DWORD dwH = info.rcClient.bottom - info.rcClient.top;
	api->Trace("client size %d x %d", dwW, dwH);
}

void DX9RenderDevice::Present ()
{
	if (NGRender::pRS->IsRenderDisabled()) return;

	HRESULT hr = D3D()->Present(NULL, NULL, NULL, NULL );
	//Assert(hr == D3D_OK);

}

void DX9RenderDevice::BeginScene ()
{
	HRESULT hr = D3D()->BeginScene();
	Assert(hr == D3D_OK);

}

void DX9RenderDevice::EndScene (IBaseTexture* pDestiantionTexture, bool bSkipAnyWork, bool bDontResolveDepthOnX360)
{
	HRESULT hr = D3D()->EndScene();
	Assert(hr == D3D_OK);


	if (bSkipAnyWork)
	{
		return;
	}

	if (current_RT)
	{
		RenderTarget* curRT = (RenderTarget*)current_RT;
		if (pDestiantionTexture)
		{
			curRT->CopyToTexture(pDestiantionTexture);
		}

		if (curRT->NeedCopy ())
		{
#ifndef _XBOX


			long mipCount = ((RenderTarget*)current_RT)->GetMipCount();

			for (long i = 0; i < mipCount; i++)
			{
				IDirect3DSurface9* dest = ((RenderTarget*)current_RT)->GetTextureSurface(i);
				IDirect3DSurface9* src = ((RenderTarget*)current_RT)->GetD3DSurface();
				if (src && dest)
				{
					hr = D3D()->StretchRect(src, NULL, dest, NULL, D3DTEXF_LINEAR);
				}
			}

#else
//!!!			throw;
#endif
		}

	}

}

void DX9RenderDevice::SetRenderTarget (RenderTartgetOptions options, IRenderTarget* renderTarget, IRenderTargetDepth* renderTargetDepth)
{
	current_RT = renderTarget;
	current_RT_depth = renderTargetDepth;


	IDirect3DSurface9* d3d_colorSurface = NULL;
	IDirect3DSurface9* d3d_depthSurface = NULL;

	if (renderTarget)
	{
		d3d_colorSurface = ((RenderTarget*)renderTarget)->GetD3DSurface ();
	}

	if (renderTargetDepth)
	{
		d3d_depthSurface = ((RenderTargetDepth*)renderTargetDepth)->GetD3DSurface ();
	}

	for (dword n =0 ; n < 16; n++)
	{
		NGRender::pRS->D3D()->SetTexture(n, NULL);
	}



	HRESULT hr = D3D()->SetRenderTarget(0, d3d_colorSurface);
	Assert(hr == D3D_OK);
	
	hr = D3D()->SetDepthStencilSurface(d3d_depthSurface);
	Assert(hr == D3D_OK);

	/*if (options == RTO_DONTOCH_CONTEXT)
	{
		//эмулируем разрушение в EDRAM

		dword dwFlags = D3DCLEAR_TARGET;
		if (d3d_depthSurface)
		{
			dwFlags = dwFlags | D3DCLEAR_ZBUFFER;
		}
		hr = D3D()->Clear( 0, NULL, dwFlags, rand(), 1.0, rand());
		Assert(hr == D3D_OK);
	}*/
}

void DX9RenderDevice::GetRenderTarget (IRenderTarget** renderTarget, IRenderTargetDepth** renderTargetDepth)
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


IDirect3DDevice9* DX9RenderDevice::D3D()
{
	return d3d_device;
}

void DX9RenderDevice::ResolveColor (IBaseTexture * dest)
{
	if (current_RT && dest)
	{
		RENDERSURFACE_DESC desc1;
		current_RT_depth->GetDesc(&desc1);
		
		dword dwTexWidth = dest->GetWidth();
		dword dwTexHeight = dest->GetHeight();

		Assert(desc1.Width == dwTexWidth);
		Assert(desc1.Height == dwTexHeight);


		//Assert(desc1.MultiSampleType == desc2.MultiSampleType);
		//Assert(desc1.MultiSampleQuality == desc2.MultiSampleQuality);

		IDirect3DTexture9* color_tex = (IDirect3DTexture9*)dest->GetBaseTexture();

		IDirect3DSurface9* srcDepthSurface = ((RenderTarget*)current_RT)->GetD3DSurface();
		IDirect3DSurface9* destDepthSurface = NULL;
		color_tex->GetSurfaceLevel(0, &destDepthSurface);

		HRESULT hr = D3D()->StretchRect(srcDepthSurface, NULL, destDepthSurface, NULL, D3DTEXF_NONE);

		destDepthSurface->Release();
	}
}

void DX9RenderDevice::ResolveDepth (IRenderTargetDepth* dest)
{
	if (current_RT_depth && dest)
	{
		RENDERSURFACE_DESC desc1;
		RENDERSURFACE_DESC desc2;
		current_RT_depth->GetDesc(&desc1);
		dest->GetDesc(&desc2);

		Assert(desc1.Width == desc2.Width);
		Assert(desc1.Height == desc2.Height);

		//Assert(desc1.MultiSampleType == desc2.MultiSampleType);
		//Assert(desc1.MultiSampleQuality == desc2.MultiSampleQuality);

		IDirect3DSurface9* srcDepthSurface = ((RenderTargetDepth*)current_RT_depth)->GetD3DSurface();
		IDirect3DSurface9* destDepthSurface = ((RenderTargetDepth*)dest)->GetD3DSurface();

		HRESULT hr = D3D()->StretchRect(srcDepthSurface, NULL, destDepthSurface, NULL, D3DTEXF_NONE);
	}
}

#endif