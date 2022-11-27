#include "RenderTargetDepth.h"
#include "..\Render.h"

RenderTargetDepth::RenderTargetDepth()
{
	bRestoreMode = false;
	refCount = 1;
	m_surface = NULL;
	mem_size = 0;

	srcFile = NULL;
	srcLine = -1;


#ifdef _XBOX
	m_DepthTexture = NULL;
#endif

}

RenderTargetDepth::~RenderTargetDepth()
{

	NGRender::pRS->NotifyDeleteRTD(this);

	RELEASE_D3D(m_surface, 0);

#ifdef _XBOX
	RELEASE(m_DepthTexture);
#endif

}

void RenderTargetDepth::AddRef()
{
	refCount++;
}

bool RenderTargetDepth::Release()
{
	refCount--;
	if (refCount <= 0)
	{
		delete this;
		return true;
	}

	return false;
}

bool RenderTargetDepth::ForceRelease()
{
	refCount = 1;
	Release();
	return true;
}

DX8RESOURCETYPE RenderTargetDepth::GetType() const
{
	return DX8TYPE_DEPTHSTENCIL;
}

dword RenderTargetDepth::GetSize() const
{
#ifdef _XBOX
	//На боксе это все в текстурах...
	return 0;
#else
	return mem_size;
#endif
}

const char* RenderTargetDepth::GetFileName() const
{
	return srcFile;
}

long RenderTargetDepth::GetFileLine() const
{
	return srcLine;
}

void RenderTargetDepth::SetFileLine(const char * pFileName, long iLine)
{
	srcFile = pFileName;
	srcLine = iLine;
}

#ifdef _XBOX

IBaseTexture*  RenderTargetDepth::AsTexture ()
{
	return m_DepthTexture;
}


const PredicatedTilingInfo& RenderTargetDepth::GetTileInfoDepthOnly ()
{
	return TI_OnlyDepth;
}


void  RenderTargetDepth::CalculateEDRAMOffset ()
{
	X360RenderDevice* x360 = (X360RenderDevice*)NGRender::pRS->Device();

	D3DFORMAT fmt = FormatToDX(initParams.Format);
	D3DMULTISAMPLE_TYPE msFmt = MSTypeToDX(initParams.MultiSample);


	x360->CalculateTiling(initParams.Width, initParams.Height, D3DFMT_UNKNOWN, D3DFMT_D24X8, msFmt, TI_OnlyDepth);
	x360->CreateEDRAMSurfaces(TI_OnlyDepth, EDRAM_ONLY_DEPTH, D3DFMT_UNKNOWN, msFmt);

}


#endif



bool RenderTargetDepth::Create (dword Width, dword Height, RENDERFORMAT Format, RENDERMULTISAMPLE_TYPE MultiSample, bool canResolve)
{
	initParams.Width = Width;
	initParams.Height = Height;
	initParams.Format = Format;
	initParams.MultiSample = MultiSample;

#ifndef _XBOX
	NGRender::pRS->D3D()->CreateDepthStencilSurface(Width, Height, FormatToDX(Format), MSTypeToDX(MultiSample), 0, false, &m_surface, NULL);
	mem_size = Width * Height * 4;

	if (!m_surface)
	{
		return false;
	}

#else
	m_surface = NULL;

	if (!bRestoreMode && canResolve)
	{
		m_DepthTexture = NGRender::pRS->CreateTexture(Width, Height, 1, USAGE_RENDERTARGET, Format, _FL_, POOL_DEFAULT);
	} else
	{
		m_DepthTexture = NULL;
	}

	CalculateEDRAMOffset();

#endif

	return true;
}

void RenderTargetDepth::CreateFromDX (IDirect3DSurface9* depthStencilSurface, bool bRestoreMode)
{
	D3DSURFACE_DESC d;
	depthStencilSurface->GetDesc(&d);

	initParams.Format = FormatFromDX(d.Format);
	initParams.Height = d.Height;
	initParams.Width = d.Width;
	initParams.MultiSample = MSTypeFromDX(d.MultiSampleType);



	m_surface = depthStencilSurface;
	//m_surface->AddRef();
}

DX8RESOURCETYPE	RenderTargetDepth::GetSubType() const
{
	return DX8TYPE_DEPTHSTENCIL;
}

bool RenderTargetDepth::GetDesc (RENDERSURFACE_DESC * pDesc)
{
	memset (pDesc, 0, sizeof(RENDERSURFACE_DESC));

	pDesc->Width = initParams.Width;
	pDesc->Height = initParams.Height;
	pDesc->Format = initParams.Format;
	pDesc->MultiSampleType = initParams.MultiSample;
	return false;
}

IDirect3DSurface9* RenderTargetDepth::GetD3DSurface()
{
	return m_surface;
}

void RenderTargetDepth::OnResetDevice()
{
	bRestoreMode = true;
	Create (initParams.Width,	initParams.Height, initParams.Format,	initParams.MultiSample);
	bRestoreMode = false;
}

void RenderTargetDepth::OnLostDevice()
{
	api->Trace("surface : 0x%08X", m_surface);

	RELEASE(m_surface);

}
