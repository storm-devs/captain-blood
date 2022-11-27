#include "Texture.h"
#include "..\Render.h"

CDX8Texture::CDX8Texture() : Resource(DX8TYPE_TEXTURE, 0)
{
#ifdef _XBOX
	bDirectGPUMemory = false;
	pTextureData = NULL;
	dwAllocAttributes = 0;
#endif

	iNumLocks = 0;
	internalSystemData_IDirect3DBaseTexture9 = null;
	bUpdateTexture = false;
	pSysMemTexture = null;
}

CDX8Texture::~CDX8Texture()
{
	
#ifdef _XBOX

	if (internalSystemData_IDirect3DBaseTexture9)
	{
		if (pTextureData)
		{
			Assert(bDirectGPUMemory == true);

			PIXReportDeletedTexture((IDirect3DBaseTexture9 *)internalSystemData_IDirect3DBaseTexture9, TRUE, TRUE);

			free(internalSystemData_IDirect3DBaseTexture9);
			internalSystemData_IDirect3DBaseTexture9 = NULL;

			XMemFree(pTextureData, dwAllocAttributes);
			pTextureData = NULL;

		} else
		{
			Assert(bDirectGPUMemory == false);

			IDirect3DBaseTexture9 * tempTexPtr = (IDirect3DBaseTexture9*)internalSystemData_IDirect3DBaseTexture9;
			RELEASE(tempTexPtr);
			RELEASE(pSysMemTexture);
		}
	}


#else

	IDirect3DBaseTexture9 * tempTexPtr = (IDirect3DBaseTexture9*)internalSystemData_IDirect3DBaseTexture9;
	RELEASE_D3D(tempTexPtr, 0);
	RELEASE_D3D(pSysMemTexture, 0);

#endif
}

bool CDX8Texture::Release() 
{
	if (Resource.Release())
	{
		return ForceRelease();
	}
	return false;
}

bool CDX8Texture::ForceRelease() 
{ 
	Resource.ForceRelease(); 
	NGRender::pRS->ReleaseResource(this);
	delete this;
	return true;
}


bool CDX8Texture::CreateLinear(dword Width, dword Height, dword Levels, dword Usage, RENDERFORMAT Format, RENDERPOOL Pool)
{
	IDirect3DBaseTexture9 * tempTexPtr = (IDirect3DBaseTexture9*)internalSystemData_IDirect3DBaseTexture9;
	RELEASE(tempTexPtr);

#ifdef _XBOX
	bDirectGPUMemory = false;
#endif
	

	initParams.bRenderTarget = false;
	initParams.bLinear = true;

	initParams.Width = Width;
	initParams.Height = Height;
	initParams.Levels = Levels;
	initParams.Usage = Usage;
	initParams.Format = Format;
	initParams.Pool = Pool;
	
	DWORD baseDataSize = Width * Height * 4;
	if (Levels > 1)
	{
		baseDataSize = (DWORD)((float)baseDataSize * 1.33333f);
	}


#ifndef _XBOX
	HRESULT hr = pD3D8->CreateTexture(Width, Height, Levels, UsageToDX (Usage), FormatToDX(Format), PoolToDX (Pool), (IDirect3DTexture9**)&internalSystemData_IDirect3DBaseTexture9, NULL);
	Resource.SetSize((baseDataSize + 4095 & ~4095));
#else

	HRESULT hr;
	if (((Usage & USAGE_RENDERTARGET) > 0) || ((Usage & USAGE_DEPTHSTENCIL) > 0))
	{

		if (Levels <= 1)
		{
			hr = pD3D8->CreateTexture(Width, Height, Levels, UsageToDX (Usage), FormatToDX(Format), PoolToDX (Pool), (IDirect3DTexture9**)&internalSystemData_IDirect3DBaseTexture9, NULL);
			IDirect3DTexture9 * tempTexPtr2 = (IDirect3DTexture9*)internalSystemData_IDirect3DBaseTexture9;
			BOOL bHavePackedMipTail = XGIsPackedTexture(tempTexPtr2);
			Assert(bHavePackedMipTail == FALSE);
			DWORD dataSize = Width*Height*4;
			Resource.SetSize((dataSize + 4095 & ~4095));
		} else
		{

			DWORD dataSize = 0;
			//Для Resolve все mipы должны быть выравнены по 4Kb
			//делаем текстуру руками не используя packed mip tails...
			internalSystemData_IDirect3DBaseTexture9 = malloc(sizeof(IDirect3DTexture9));
			DWORD dwTextureSize = XGSetTextureHeaderEx( Width, Height, Levels, UsageToDX (Usage), FormatToDX(Format), 0, XGHEADEREX_NONPACKED,
				                                          0, XGHEADER_CONTIGUOUS_MIP_OFFSET, 0, (IDirect3DTexture9*)internalSystemData_IDirect3DBaseTexture9, NULL, NULL );

			dwAllocAttributes = MAKE_XALLOC_ATTRIBUTES( 0, FALSE, FALSE, FALSE, 0, XALLOC_PHYSICAL_ALIGNMENT_4K, XALLOC_MEMPROTECT_WRITECOMBINE, FALSE, XALLOC_MEMTYPE_PHYSICAL );
			pTextureData = ( BYTE* )XMemAlloc( dwTextureSize, dwAllocAttributes );
			dataSize += dwTextureSize;

			Assert( pTextureData != NULL );

			XGOffsetResourceAddress( (IDirect3DTexture9*)internalSystemData_IDirect3DBaseTexture9, pTextureData );

			bDirectGPUMemory = true;


			D3DSURFACE_DESC dsc;
			((IDirect3DTexture9*)internalSystemData_IDirect3DBaseTexture9)->GetLevelDesc(0, &dsc);

			//api->Trace("%dx%d", dsc.Width, dsc.Height);

			hr = D3D_OK;
			

			Resource.SetSize((dataSize + 4095 & ~4095) + sizeof(D3DTexture));
		}

	} else
	{
		hr = pD3D8->CreateTexture(Width, Height, Levels, UsageToDX (Usage), FormatToLinearDX(Format), PoolToDX (Pool), (IDirect3DTexture9**)&internalSystemData_IDirect3DBaseTexture9, NULL);
		Resource.SetSize((baseDataSize + 4095 & ~4095));
	}

#endif

	if (Pool == POOL_DEFAULT || ((Usage & USAGE_RENDERTARGET) > 0))
	{
		initParams.bRenderTarget = true;
	}


	if(hr != D3D_OK)
	{
		return false;
	}

	BaseTexture.dwWidth = Width; 
	BaseTexture.dwHeight = Height; 
	BaseTexture.dwDepth = 1;
	BaseTexture.dwLevels = Levels; 
	BaseTexture.dwUsage = UsageToDX(Usage); 
	BaseTexture.Format = FormatToDX (Format); 
	BaseTexture.Pool = PoolToDX (Pool);

	return true;
}

bool CDX8Texture::Create(dword Width, dword Height, dword Levels, dword Usage, RENDERFORMAT Format, RENDERPOOL Pool)
{
	IDirect3DBaseTexture9 * tempTexPtr = (IDirect3DBaseTexture9*)internalSystemData_IDirect3DBaseTexture9;
	RELEASE(tempTexPtr);

#ifdef _XBOX
	bDirectGPUMemory = false;
#endif

	initParams.bRenderTarget = false;
	initParams.bLinear = false;

	initParams.Width = Width;
	initParams.Height = Height;
	initParams.Levels = Levels;
	initParams.Usage = Usage;
	initParams.Format = Format;
	initParams.Pool = Pool;


	if (Pool == POOL_DEFAULT || ((Usage & USAGE_RENDERTARGET) > 0))
	{
		initParams.bRenderTarget = true;
	}



	HRESULT hr = pD3D8->CreateTexture(Width, Height, Levels, UsageToDX (Usage), FormatToDX(Format), PoolToDX (Pool), (IDirect3DTexture9**)&internalSystemData_IDirect3DBaseTexture9, NULL);
	if(hr != D3D_OK)
	{
		return false;
	}

	BaseTexture.dwWidth = Width; 
	BaseTexture.dwHeight = Height; 
	BaseTexture.dwDepth = 1;
	BaseTexture.dwLevels = Levels; 
	BaseTexture.dwUsage = UsageToDX(Usage); 
	BaseTexture.Format = FormatToDX (Format); 
	BaseTexture.Pool = PoolToDX (Pool);

	DWORD baseDataSize = Width * Height * 4;
	if (Levels > 1)
	{
		baseDataSize = (DWORD)((float)baseDataSize * 1.33333f);
	}
	Resource.SetSize((baseDataSize + 4095 & ~4095));

	return true;
}

bool CDX8Texture::Reset()
{
	IDirect3DBaseTexture9 * tempTexPtr = (IDirect3DBaseTexture9*)internalSystemData_IDirect3DBaseTexture9;
	RELEASE(tempTexPtr);
	return true;
}

bool CDX8Texture::Recreate()
{
	return Create(GetWidth(), GetHeight(), GetLevelCount(), GetUsage(), GetFormat(), GetPool());
}

bool CDX8Texture::GetLevelDesc(dword Level, RENDERSURFACE_DESC * pDesc)
{
	D3DSURFACE_DESC tempDesc;
	HRESULT hr = ((IDirect3DTexture9*)internalSystemData_IDirect3DBaseTexture9)->GetLevelDesc(Level, &tempDesc);
	Assert(hr == D3D_OK);

	*pDesc = SurfDescFromDX (tempDesc);
	return true;
}


bool CDX8Texture::LockRect(dword Level, RENDERLOCKED_RECT * pLockedRect, CONST RECT * pRect, dword Flags)
{
	if (internalSystemData_IDirect3DBaseTexture9 == NULL)
	{
		return false;
	}

	if (NGRender::pRS->IsDeviceLost())
	{
		return false;
	}




	D3DLOCKED_RECT tempLR;
	HRESULT hr = ((IDirect3DTexture9*)internalSystemData_IDirect3DBaseTexture9)->LockRect(Level, &tempLR, pRect, LockFlagsToDX(Flags));
	Assert(hr == D3D_OK);

	pLockedRect->pBits = tempLR.pBits;
	pLockedRect->Pitch = tempLR.Pitch;
	
	iNumLocks++;
	return true;
}

bool CDX8Texture::UnlockRect(dword Level)
{
	HRESULT hr = ((IDirect3DTexture9*)internalSystemData_IDirect3DBaseTexture9)->UnlockRect(Level);
	Assert(hr == D3D_OK);

	iNumLocks--;
	return true;
}

/*
IDirect3DBaseTexture9 * CDX8Texture::GetBaseTexture() 
{ 
	//return (iNumLocks) ? null : pTexture;
	return pTexture;
}
*/

void CDX8Texture::SetSysMemTexture(CDX8Texture * pSysMemTexture)
{
	this->pSysMemTexture = pSysMemTexture;
}

CDX8Texture * CDX8Texture::GetSysMemTexture()
{
	return pSysMemTexture;
}

void CDX8Texture::UpdateTexture()
{
	if (!pSysMemTexture) return;

	NGRender::pRS->UpdateTexture(pSysMemTexture, this);
	RELEASE(pSysMemTexture);
}

bool CDX8Texture::CreateUseD3DX (const char* fileName)
{
	IDirect3DBaseTexture9 * tempTexPtr = (IDirect3DBaseTexture9*)internalSystemData_IDirect3DBaseTexture9;
	RELEASE(tempTexPtr);

#ifdef _XBOX
	Assert(bDirectGPUMemory == false);
#endif
/*
#ifdef _XBOX
	bDirectGPUMemory = false;
	Assert(false);

#else 


*/
	IFileService * pFS = (IFileService *)api->GetService("FileService");
	string res;
	pFS->BuildPath(fileName, res);



	HRESULT hr = D3DXCreateTextureFromFileA(NGRender::pRS->D3D(), res.c_str(), (IDirect3DTexture9**)&internalSystemData_IDirect3DBaseTexture9);

	if (hr != D3D_OK) return false;


	RENDERSURFACE_DESC dsc;
	GetLevelDesc(0, &dsc);

	BaseTexture.dwWidth = dsc.Width; 
	BaseTexture.dwHeight = dsc.Height; 
	BaseTexture.dwDepth = 1;
	BaseTexture.dwLevels = 1; 
	BaseTexture.dwUsage = UsageToDX(dsc.Usage); 
	BaseTexture.Format = FormatToDX (dsc.Format); 
	BaseTexture.Pool = PoolToDX (dsc.Pool);



	



	initParams.bRenderTarget = false;
	initParams.bLinear = false;
	initParams.Width = dsc.Width;
	initParams.Height = dsc.Height;
	initParams.Levels = 1;
	initParams.Usage = dsc.Usage;
	initParams.Format = dsc.Format;
	initParams.Pool = dsc.Pool;

	if (dsc.Pool == POOL_DEFAULT || ((dsc.Usage & USAGE_RENDERTARGET) > 0))
	{
		initParams.bRenderTarget = true;
	}



//#endif


	return true;

}



/*
void CDX8Texture::ConvertToX360 ()
{
	D3DTexture* pTiledTexture = NULL;
	HRESULT hr = pD3D8->CreateTexture(BaseTexture.dwWidth, BaseTexture.dwHeight, BaseTexture.dwLevels, BaseTexture.dwUsage, BaseTexture.Format, BaseTexture.Pool, &pTiledTexture, NULL);

	D3DTexture* pUntiledTexture = pTexture;

	XGTEXTURE_DESC destDesc;
	XGGetTextureDesc( pTiledTexture, 0, &destDesc );

	XGTEXTURE_DESC srcDesc;
	XGGetTextureDesc( pUntiledTexture, 0, &srcDesc );


	DWORD numberMipLevels = pUntiledTexture->GetLevelCount();
	int firstPackedMip = XGGetMipTailBaseLevel(destDesc.Width, destDesc.Height, FALSE);

	api->Trace("first packed mip : %d of %d\n", firstPackedMip, numberMipLevels);

	for( int level = 0; level < numberMipLevels; level++ )
	{
		XGTEXTURE_DESC srcLevelDesc;
		XGGetTextureDesc( pUntiledTexture, level, &srcLevelDesc );

		XGTEXTURE_DESC dstLevelDesc;
		XGGetTextureDesc( pTiledTexture, level, &dstLevelDesc );

		D3DLOCKED_RECT srcRect, destRect;
		pUntiledTexture->LockRect( level, &srcRect, NULL, 0 );
		pTiledTexture->LockRect( level, &destRect, NULL, 0 );


		//if (level <= firstPackedMip)
		{
			XGEndianSwapSurface(srcRect.pBits, 
				srcRect.Pitch,
				srcRect.pBits,
				srcRect.Pitch,
				srcLevelDesc.Width,
				srcLevelDesc.Height,
				srcLevelDesc.Format);
		}


		XGTileTextureLevel(  
			destDesc.Width, 
			destDesc.Height,
			level,
			XGGetGpuFormat( destDesc.Format ),
			0,
			destRect.pBits,
			NULL,
			srcRect.pBits, 
			srcRect.Pitch,
			NULL );

		

		pUntiledTexture->UnlockRect( level );
		pTiledTexture->UnlockRect( level );
	}

	pTexture = pTiledTexture;
	pUntiledTexture->Release();
}
*/

#ifdef _XBOX

void CDX8Texture::CreateFromGPUTexture (D3DTexture * pBaseTexture, BYTE* pTextureData, DWORD dwAllocAttributes, DWORD dataSize, const char * pixGUID)
{
	IDirect3DBaseTexture9 * tempTexPtr = (IDirect3DBaseTexture9*)internalSystemData_IDirect3DBaseTexture9;
	RELEASE(tempTexPtr);

	if (pBaseTexture == NULL || pTextureData == NULL)
	{
		return;
	}

	internalSystemData_IDirect3DBaseTexture9 = (void*)pBaseTexture;

	this->pTextureData = pTextureData;
	this->dwAllocAttributes = dwAllocAttributes;

	bDirectGPUMemory = true;

/*
	D3DSURFACE_DESC desc;
	pTexture->GetLevelDesc(0, &desc);
	api->Trace("%dx%d - from '%s', line %d", desc.Width, desc.Height, GetFileName(), GetFileLine());
	NGRender::pRS->D3D()->SetTexture(0, pTexture);


*/
	D3DSURFACE_DESC desc;
	((IDirect3DTexture9*)internalSystemData_IDirect3DBaseTexture9)->GetLevelDesc(0, &desc);

	BaseTexture.dwWidth = desc.Width;
	BaseTexture.dwHeight = desc.Height;
	BaseTexture.dwDepth = 1;
	BaseTexture.dwLevels = ((IDirect3DTexture9*)internalSystemData_IDirect3DBaseTexture9)->GetLevelCount();
	BaseTexture.dwUsage = desc.Usage;
	BaseTexture.Format = desc.Format;
	BaseTexture.Pool = desc.Pool;

	Resource.SetSize((dataSize + 4095 & ~4095) + sizeof(D3DTexture));

	PIXReportNewTexture(pBaseTexture);

	PIXSetTextureName(pBaseTexture, pixGUID);
}

#endif


void CDX8Texture::OnResetDevice()
{
	if (!initParams.bRenderTarget) return;

	if (initParams.bLinear)
	{
		CreateLinear(initParams.Width, initParams.Height, initParams.Levels, initParams.Usage, initParams.Format, initParams.Pool);
	} else
	  {
		  Create(initParams.Width, initParams.Height, initParams.Levels, initParams.Usage, initParams.Format, initParams.Pool);
		}

}

void CDX8Texture::OnLostDevice()
{
	if (initParams.bRenderTarget)
	{
		IDirect3DBaseTexture9 * tempTexPtr = (IDirect3DBaseTexture9*)internalSystemData_IDirect3DBaseTexture9;
		RELEASE(tempTexPtr);
		internalSystemData_IDirect3DBaseTexture9 = NULL;
	}

}
