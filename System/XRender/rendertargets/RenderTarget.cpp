#include "RenderTarget.h"
#include "..\Render.h"


RenderTarget::RenderTarget()
{
	bRestoreMode = false;

	bNeedCopy = false;

	refCount = 1;

	m_surface = NULL;
	m_texture = NULL;

	mem_size = 0;

	srcFile = NULL;
	srcLine = -1;


	for (long i = 0; i < MAX_MIPCOUNT; i++)
	{
		m_TextureSurfaces[i] = NULL;
	}

}

RenderTarget::~RenderTarget()
{
	NGRender::pRS->NotifyDeleteRT(this);

	for (long i = 0; i < MAX_MIPCOUNT; i++)
	{
		RELEASE(m_TextureSurfaces[i]);
	}
	
	RELEASE_D3D(m_surface, 0);

	RELEASE(m_texture);
}



void RenderTarget::AddRef()
{
	refCount++;
}

bool RenderTarget::Release()
{
	refCount--;
	if (refCount <= 0)
	{
		delete this;
		return true;
	}

	return false;
}

bool RenderTarget::ForceRelease()
{
	refCount = 1;
	Release();
	return true;
}

DX8RESOURCETYPE RenderTarget::GetType() const
{
	return DX8TYPE_SURFACE;
}

dword RenderTarget::GetSize() const
{
#ifdef _XBOX
	//На боксе это все в текстурах...
	return 0;
#else
	return mem_size;
#endif
	
}

const char* RenderTarget::GetFileName() const
{
	return srcFile;
}

long RenderTarget::GetFileLine() const
{
	return srcLine;
}

void RenderTarget::SetFileLine(const char * pFileName, long iLine)
{
	srcFile = pFileName;
	srcLine = iLine;
}

bool RenderTarget::Create (dword Width, dword Height, RENDERFORMAT Format, RENDERMULTISAMPLE_TYPE MultiSample, long MipLevelsCount)
{
	if (MipLevelsCount <= 0)
	{
		MipLevelsCount = 1;
	}

	if (MipLevelsCount > MAX_MIPCOUNT)
	{
		MipLevelsCount = MAX_MIPCOUNT;
	}

	initParams.Width = Width;
	initParams.Height = Height;
	initParams.Format = Format;
	initParams.MultiSample = MultiSample;
	initParams.MipLevelsCount = MipLevelsCount;


#ifdef _XBOX
	//На XBOX не создаем сурфейс, только текстуру
	bNeedCopy = true;
	m_surface = NULL;

#else
	if (MultiSample != MULTISAMPLE_NONE)
	{
		bNeedCopy = true;
	} else
	{
		bNeedCopy = false;
	}

	if (MipLevelsCount > 1)
	{
		bNeedCopy = true;
	}

	if (bNeedCopy)
	{
		D3DFORMAT fmt = FormatToDX(Format);
		D3DMULTISAMPLE_TYPE msFmt = MSTypeToDX(MultiSample);
		NGRender::pRS->D3D()->CreateRenderTarget(Width, Height, fmt, msFmt, 0, false, &m_surface, NULL);

		mem_size = Width * Height * 4;

		if (!m_surface)
		{
			return false;
		}
	}

#endif



	dsc.Format = Format;
	dsc.Height = Height;
	dsc.Width = Width;
	dsc.MultiSampleQuality = 0;
	dsc.MultiSampleType = MultiSample;
	dsc.Pool = POOL_DEFAULT;
	dsc.Size = 0;
	dsc.Type = RTYPE_SURFACE;
	dsc.Usage = USAGE_RENDERTARGET;

	

	if (!bRestoreMode)
	{
		m_texture = NGRender::pRS->CreateTexture(Width, Height, MipLevelsCount, USAGE_RENDERTARGET, Format, srcFile, srcLine, POOL_DEFAULT);
		mem_size = Width * Height * 4;
	}

	if (!m_texture)
	{
		RELEASE(m_surface);
		return false;
	}


	IDirect3DTexture9* pDXTexture = (IDirect3DTexture9*)m_texture->GetBaseTexture();

	for (long i = 0; i < MipLevelsCount; i++)
	{
		pDXTexture->GetSurfaceLevel(i, &m_TextureSurfaces[i]);
	}

	

	if (bNeedCopy == false)
	{
		m_surface = m_TextureSurfaces[0];
		m_surface->AddRef();
	}
	
#ifdef _XBOX
	CalculateEDRAMOffset();
#endif

	return true;
}


void RenderTarget::CreateFromDX (IDirect3DSurface9* depthStencilSurface, bool bRestoreMode)
{
	D3DSURFACE_DESC d;
	depthStencilSurface->GetDesc(&d);
	
	dsc.Format = FormatFromDX(d.Format);
	dsc.Height = d.Height;
	dsc.Width = d.Width;
	dsc.MultiSampleQuality = 0;
	dsc.MultiSampleType = MSTypeFromDX(d.MultiSampleType);
	dsc.Pool = PoolFromDX(d.Pool);
	dsc.Size = 0;
	dsc.Type = RTYPE_SURFACE;
	dsc.Usage = UsageFromDX(dsc.Usage);

	m_surface = depthStencilSurface;
	//m_surface->AddRef();

	// Вано добавил, потому что используется в EndScene
	initParams.MipLevelsCount = 1;

	if (!bRestoreMode)
	{
		m_texture = NGRender::pRS->CreateTexture(dsc.Width, dsc.Height, 1, USAGE_RENDERTARGET, dsc.Format, _FL_, POOL_DEFAULT);
		mem_size = dsc.Width * dsc.Height * 4;
	}

	IDirect3DTexture9* pDXTexture = (IDirect3DTexture9*)m_texture->GetBaseTexture();
	if (pDXTexture)
	{
		pDXTexture->GetSurfaceLevel(0, &m_TextureSurfaces[0]);
	} else
	{
		m_TextureSurfaces[0] = null;
	}


	m_TextureSurfaces[0]->AddRef();
	ULONG rc = m_TextureSurfaces[0]->Release();


	bNeedCopy = true;
}


DX8RESOURCETYPE	RenderTarget::GetSubType() const
{
	return DX8TYPE_SURFACE;
}

bool RenderTarget::GetDesc(RENDERSURFACE_DESC * pDesc)
{
	*pDesc = dsc;
	return true;
}

IBaseTexture* RenderTarget::AsTexture ()
{
	return m_texture;
}

IDirect3DSurface9* RenderTarget::GetD3DSurface()
{
	return m_surface;
}


long  RenderTarget::GetMipCount()
{
	return initParams.MipLevelsCount;
}


IDirect3DSurface9* RenderTarget::GetTextureSurface(long mipIndex)
{
	return m_TextureSurfaces[mipIndex];
}

bool RenderTarget::NeedCopy()
{
	return bNeedCopy;
}


void RenderTarget::CopyToTexture (IBaseTexture* pDestiantionTexture)
{
#ifndef _XBOX
	IDirect3DSurface9* copyFrom = NULL; 

	if (!bNeedCopy)
	{
		copyFrom = m_surface;
	} else
	{
		copyFrom = m_TextureSurfaces[0];
	}

	if (copyFrom && pDestiantionTexture)
	{
		IDirect3DSurface9* copyTo = NULL;
		IDirect3DTexture9* pDXTexture = (IDirect3DTexture9*)pDestiantionTexture->GetBaseTexture();
		pDXTexture->GetSurfaceLevel(0, &copyTo);
		NGRender::pRS->D3D()->StretchRect(copyFrom, NULL, copyTo, NULL, D3DTEXF_LINEAR);

		RELEASE(copyTo);
	}

#else

	//Not available for X360 !!!!
	Assert(false);

#endif
}



#ifdef _XBOX

const PredicatedTilingInfo& RenderTarget::GetTileInfo ()
{
	return TI_OnlyColor;
}

const PredicatedTilingInfo& RenderTarget::GetTileInfoWithDepth ()
{
	return TI_ColorAndDepth;
}


void RenderTarget::CalculateEDRAMOffset ()
{

	X360RenderDevice* x360 = (X360RenderDevice*)NGRender::pRS->Device();

	D3DFORMAT fmt = FormatToDX(dsc.Format);
	D3DMULTISAMPLE_TYPE msFmt = MSTypeToDX(dsc.MultiSampleType);

	x360->CalculateTiling(dsc.Width, dsc.Height, fmt, D3DFMT_UNKNOWN, msFmt, TI_OnlyColor);
	x360->CalculateTiling(dsc.Width, dsc.Height, fmt, D3DFMT_D24X8, msFmt, TI_ColorAndDepth);

	//создаем теперь нужные нам Surface's они памяти не занимают, т.к это просто адрес в EDRAM
	//====================================================================================================

	x360->CreateEDRAMSurfaces(TI_OnlyColor, EDRAM_ONLY_COLOR, fmt, msFmt);
	x360->CreateEDRAMSurfaces(TI_ColorAndDepth, EDRAM_COLOR_AND_DEPTH, fmt, msFmt);
}


bool RenderTarget::FitEDRAMWithMainRT()
{
	if (TI_ColorAndDepth.bUseLastPartOfEDRAM)
	{
		return true;
	}

	return false;
}


#endif


void RenderTarget::OnResetDevice()
{
	bRestoreMode = true;
	Create (initParams.Width,	initParams.Height, initParams.Format, initParams.MultiSample, initParams.MipLevelsCount);
	bRestoreMode = false;
}

void RenderTarget::OnLostDevice()
{
	
	api->Trace("surface : 0x%08X", m_surface);

	for (long i = 0; i < MAX_MIPCOUNT; i++)
	{
		api->Trace("surface(mip%d) : 0x%08X", i, m_TextureSurfaces[i]);

		RELEASE(m_TextureSurfaces[i]);
	}

	RELEASE(m_surface);

}

