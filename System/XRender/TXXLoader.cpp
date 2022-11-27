#include "TXXLoader.h"


#ifndef _XBOX


//------------- загрузчик для PC

#include "..\..\Common_h\data_swizzle.h"

TXXLoader::SD_TEXFORMAT TXXLoader::TexFormats[] =
{
	{txxf_dxt1,		D3DFMT_DXT1,		true,	4, "D3DFMT_DXT1"},
	{txxf_dxt3,		D3DFMT_DXT3,		true,	4, "D3DFMT_DXT3"},
	{txxf_dxt5,		D3DFMT_DXT5,		true,	4, "D3DFMT_DXT5"},
	{txxf_a8r8g8b8,	D3DFMT_A8R8G8B8,	false,	1, "D3DFMT_A8R8G8B8"},
	{txxf_r5g6b5,	D3DFMT_R5G6B5,		false,	1, "D3DFMT_R5G6B5"},
	{txxf_a4r4g4b4,	D3DFMT_A4R4G4B4,	false,	1, "D3DFMT_A4R4G4B4"},
	{txxf_a1r5g5b5,	D3DFMT_A1R5G5B5,	false,	1, "D3DFMT_A1R5G5B5"},
	{txxf_q8w8v8u8, D3DFMT_Q8W8V8U8,	false,	1, "D3DFMT_Q8W8V8U8"},
	{txxf_v8u8,		D3DFMT_V8U8,		false,	1, "D3DFMT_V8U8"},
	{txxf_a8,		D3DFMT_A8,			false,	1, "D3DFMT_A8"},
	{txxf_l8,		D3DFMT_L8,			false,	1, "D3DFMT_L8"},
	{txxf_unknown,	D3DFMT_X8R8G8B8,	false,	1, "D3DFMT_X8R8G8B8"},
	{txxf_unknown,	D3DFMT_D24S8,		false,  1, "D3DFMT_D24S8"},
	{txxf_unknown,	D3DFMT_D16,			false,	1, "D3DFMT_D16"},
	{txxf_unknown,	D3DFMT_D24X8,		false,	1, "D3DFMT_D24X8"},
/* X360 unsupported
	{txxf_unknown,	D3DFMT_D24X4S4,		false,	1, "D3DFMT_D24X4S4"},
	{txxf_unknown,	D3DFMT_D15S1,		false,	1, "D3DFMT_D15S1"},
*/
	{txxf_unknown,	D3DFMT_D32,			false,	1, "D3DFMT_D32"}
};





TXXLoader::TXXLoader(NGRender* r_srv)
{
	bSkipDegradation = false;

	pRS = r_srv;

	pFS = (IFileService *)api->GetService("FileService");
	Assert(pFS);
}


TXXLoader::~TXXLoader()
{

}


TXXLoader::SD_TEXFORMAT* TXXLoader::GetFormat(TXXFormat txFormat)
{
	for (dword i=0; i < sizeof(TexFormats) / sizeof(SD_TEXFORMAT); i++)
	{
		if (TexFormats[i].txFormat == txFormat)
		{
			return &TexFormats[i];
		}
	}
	return null;
}



void TXXLoader::IgnoreDegradation (bool bIgnore)
{
	bSkipDegradation = bIgnore;
}


IBaseTexture* TXXLoader::Load_TXX (const char* fileName, const char* sourceFile, long sourceLine)
{
	strFullPath = fileName;
	strFullPath.AddExtention(".txx");

	IDataFile * file = pFS->OpenDataFile(strFullPath.c_str(), file_open_default, _FL_);
	if (!file)
	{
		api->Trace("DX9Error: Can't find texture : '%s' (%s, line: %d)", strFullPath.c_str(), sourceFile, sourceLine);
		return NULL;
	}

	TxxFileHeader head;
	file->Read(&head, sizeof(TxxFileHeader));
	
	XSwizzleULong(head.id);
	XSwizzleULong(head.ver);
	XSwizzleULong(head.type);
	XSwizzleULong(head.format);
	XSwizzleULong(head.width);
	XSwizzleULong(head.height);
	XSwizzleULong(head.depth);
	XSwizzleULong(head.num_mips);
	XSwizzleULong(head.line_size);
	XSwizzleULong(head.bpp);
	XSwizzleFloat(head.fps);

/*
#ifdef _XBOX
	head.num_mips = 1;
#endif
*/	

	SD_TEXFORMAT * pTexFormat = GetFormat((TXXFormat)head.format);
	if (!pTexFormat)
	{
		api->Trace("DX9Error: unknown texture format: %d, texture: %s (%s, line: %d)", head.format, fileName, sourceFile, sourceLine);
		return NULL;
	}


	// apply texture degradation
	dword dwSeekPos = 0;
	dword dwDegradation = pRS->GetTextureDegradation();

	if (bSkipDegradation)
	{
		dwDegradation = 0;
	}

	//dwDegradation  = 2;
	for (dword i=0; i< dwDegradation; i++)
	{
		if (head.num_mips <= 1 || head.width <= 32 || head.height <= 32) break;	// degradation limit
		dwSeekPos += head.line_size * head.height;
		head.num_mips--;
		head.line_size /= 2;
		head.width /= 2; 
		head.height /= 2; 
	}
	if (dwSeekPos)
	{
		//file->SetPos(dwSeekPos, FILE_CURRENT);
		file->Read(null, dwSeekPos);
	}



	if (head.type == txxt_volume)			// if texture - volume texture
	{
		file->Release();
		return NULL;
	}
		
		
	if (head.type == txxt_cube)		// if texture - cubemap texture
	{
		file->Release();
		return NULL;
	} 


	if (head.type == txxt_2d || head.type == txxt_rendertarget) // if simple texture
	{
		CDX8Texture* pTexture = NULL;
		
		if (pRS->IsMipMapsFillColor() && head.num_mips != 1)
		{
			pTexture = CreateMipMapColoredTexture(head, *pTexFormat);
		} else 
			{
				pTexture = LoadTexture(file, head, *pTexFormat);
			}

		if (pTexture) 
		{
			pTexture->SetFileLine(sourceFile, sourceLine);
			pTexture->SetRef(1);

			file->Release();

/*
#ifdef _XBOX
			pTexture->ConvertToX360();
#endif
*/

			return pTexture;
		}
	}



	file->Release();

	return NULL;
}




CDX8Texture* TXXLoader::LoadTexture(IDataFile* file, const TxxFileHeader& _head, SD_TEXFORMAT texFormat)
{
	TxxFileHeader head = _head;

	CDX8Texture * pSysMemTexture = null;
	CDX8Texture * pDX8Texture = null;
	CDX8Texture * pLockTexture = null;

	bool bRenderTarget = (head.type == txxt_rendertarget);

	int savedWidth = head.width;
	int savedHeight = head.height;

	pDX8Texture = (CDX8Texture *)pRS->CreateTexture(head.width, head.height, head.num_mips, (bRenderTarget) ? USAGE_RENDERTARGET : 0, FormatFromDX (texFormat.d3dFormat), null, -1, (bRenderTarget) ? POOL_DEFAULT : POOL_MANAGED);
	if (!pDX8Texture) return null;
	pLockTexture = pDX8Texture;

	if (bRenderTarget)
	{
		pSysMemTexture = (CDX8Texture *)pRS->CreateTexture(head.width, head.height, head.num_mips, 0, FormatFromDX (texFormat.d3dFormat), null, -1, POOL_SYSTEMMEM);
		if (!pSysMemTexture) 
		{
			pDX8Texture->Release();
			return null;
		}

		pDX8Texture->SetSysMemTexture(pSysMemTexture);
		pLockTexture = pSysMemTexture;
	}

	for (dword i=0; i<dword(head.num_mips); i++)
	{
		RENDERSURFACE_DESC desc;
		pLockTexture->GetLevelDesc(i, &desc);

		RENDERLOCKED_RECT lr;
		if (pLockTexture->LockRect(i, &lr, null, 0))
		{
			pDX8Texture->AddLoadRef();
			LoadSurface(file, pDX8Texture, 0, i, &lr, head, texFormat);
			//pDX8Texture->UnlockRect(i);
			pDX8Texture->SetSize(pDX8Texture->GetSize() + head.line_size * head.height);
		}
		head.line_size /= 2;
		head.width /= 2;
		head.height /= 2;
	}

	if (bRenderTarget)
	{
		if (pDX8Texture->IsLoaded()) 
		{
			pDX8Texture->UpdateTexture();
		} else
		{
			if (!pDX8Texture->IsError()) 
			{
				pDX8Texture->bUpdateTexture = true;
			}
		}
	}

	return pDX8Texture;
}




bool TXXLoader::LoadBox(IDataFile * pRF, IBaseTexture * pTexture, RENDERLOCKED_BOX * pLB, const TxxFileHeader & TXHead, const SD_TEXFORMAT & TexFormat)
{
	dword	dwFullSize = TXHead.line_size * TXHead.height * TXHead.depth;

	dword readedBytes = pRF->Read(pLB->pBits, dwFullSize);
	if (readedBytes != dwFullSize)
	{ 
		return false; 
	}

	return true;
}



bool TXXLoader::LoadSurface(IDataFile * pRF, IBaseTexture * pTexture, dword dwFace, dword dwLevel, RENDERLOCKED_RECT * pLR, const TxxFileHeader & TXHead, const SD_TEXFORMAT & TexFormat)
{

#ifndef _XBOX
	//На ПеСи целиком загружаем
	dword dwFullSize = TXHead.line_size * TXHead.height;
	dword readedBytes = pRF->Read(pLR->pBits, dwFullSize);
	if (readedBytes != dwFullSize)
	{ 
		return false; 
	}


#else

	//На коробке грузим по линиям текстуру
	dword dwMultipler = 1;
	switch (TXHead.format)
	{
	case txxf_dxt1:
		dwMultipler = 2;
		break;
	case txxf_dxt3:
		dwMultipler = 4;
		break;
	case txxf_dxt5:
		dwMultipler = 4;
		break;
	}

	byte* destPtr = (byte*)pLR->pBits;
	for (dword line = 0; line < (TXHead.height / dwMultipler); line++)
	{
		dword readedBytes = pRF->Read(destPtr, (TXHead.line_size*dwMultipler));
		if (readedBytes != (TXHead.line_size*dwMultipler))
		{ 
			return false; 
		}

		destPtr += pLR->Pitch;
	}


#endif


	//=====================================================================================================


	CDX8Texture *p2DTex = (CDX8Texture*)pTexture;
	if (p2DTex->IsRenderTarget())
	{
		p2DTex->GetSysMemTexture()->UnlockRect(dwLevel);
		p2DTex->DecLoadRef();
		if (p2DTex->IsLoaded() && p2DTex->bUpdateTexture)
		{
			p2DTex->UpdateTexture();
		}
	}
	else
	{
		p2DTex->UnlockRect(dwLevel);
		p2DTex->DecLoadRef();
	}

	return true;
}



CDX8Texture* TXXLoader::CreateMipMapColoredTexture(TxxFileHeader TXHead, const SD_TEXFORMAT & TexFormat)
{
	CDX8Texture * pDX8Texture = (CDX8Texture *)pRS->CreateTexture(TXHead.width, TXHead.height, TXHead.num_mips, 0, FMT_DXT1, null, -1, POOL_MANAGED);
	if (!pDX8Texture) return null;

	/*
	dword dwColors[] = {0xff00ff00, 0xff0000ff, 0xffffaf00, 0xff00ffff, 
		0xffff00ff, 0xffff0000, 0xffdf0000, 0xffbf0000, 
		0xff9f0000, 0xff7f0000, 0xff5f0000, 0xff4f0000, 
		0xff3f0000, 0xff2f0000, 0xff1f0000, 0xff0f0000 };
*/


	dword dwColors[] = {0xff00ff00, 0xff0000ff,
		0xffffff00, 0xffffe000, 0xffffc000, 0xffffa000, 
		0xffff8000, 0xffff6000, 0xffff4000, 0xffff2000, 0xffff0000 };

	word wColors[sizeof(dwColors) / sizeof(dwColors[0])];

	for (long i = 0; i<sizeof(dwColors) / sizeof(dwColors[0]); i++)
	{
		wColors[i] = Color::Make565(dwColors[i]);
	}

	float colorsCount = ((float)sizeof(dwColors) / (float)sizeof(dwColors[0]));
	float step = colorsCount / float(TXHead.num_mips - 1);
	float idx = 0.0f;

	for (long i=0; i < (long)TXHead.num_mips; i++, idx+=step)
	{
		RENDERLOCKED_RECT lr;
		
		word wColor = wColors[(int)idx];

		if (pDX8Texture->LockRect(i, &lr, null, 0))
		{
			word * pW = (word*)lr.pBits;
			for (long j=0; j<long(TXHead.height * TXHead.width / 16); j++)
			{
				*pW++ = wColor;
				*pW++ = 0;
				*pW++ = 0;
				*pW++ = 0;
			}

			pDX8Texture->UnlockRect(i);
			pDX8Texture->SetSize(pDX8Texture->GetSize() + TXHead.line_size * TXHead.height);
		}

		TXHead.line_size /= 2;
		TXHead.width /= 2;
		TXHead.height /= 2;
	}

	return pDX8Texture;
}

#else

//------------- загрузчик для X360

TXXLoader::TXXLoader(NGRender* r_srv)
{
	pRS = r_srv;

	pFS = (IFileService *)api->GetService("FileService");
	Assert(pFS);
}


TXXLoader::~TXXLoader()
{

}

IBaseTexture* TXXLoader::Load_TXX (const char* fileName, const char* sourceFile, long sourceLine)
{
	//return NULL;

	//fileName = "resource\\textures\\inplace_texture.txx360";

	strFullPath = fileName;
	strFullPath.AddExtention(".txx360");


	IDataFile * file = pFS->OpenDataFile(strFullPath.c_str(), file_open_default, _FL_);
	if (!file)
	{
		api->Trace("DX9Error: Can't find texture : '%s' (%s, line: %d)", strFullPath.c_str(), sourceFile, sourceLine);
		return NULL;
	}
/*
	DWORD dwSize = file->GetSize();

	BYTE* trashBuffer = NEW BYTE[dwSize*2];
	file->Read(trashBuffer, dwSize);


	file->Release();

	delete [] trashBuffer;

	return NULL;
*/

	CDX8Texture * pDX8Texture = null;
	pDX8Texture = (CDX8Texture *)pRS->AddTexture(sourceFile, sourceLine);
	if (!pDX8Texture) return null;


	DWORD dwSize = file->Size();
	dword dwHeaderSize = sizeof(D3DTexture);

	DWORD dwTextureDataSize = dwSize - dwHeaderSize;

	
	D3DTexture* texture = (D3DTexture*)malloc(dwHeaderSize);
	file->Read(texture, dwHeaderSize);

	// Now allocate space for the texture data. This needs to be allocated on 4K boundary in physical memory. 
	DWORD dwAllocAttributes = MAKE_XALLOC_ATTRIBUTES( 0, FALSE, FALSE, FALSE, 0, XALLOC_PHYSICAL_ALIGNMENT_4K, XALLOC_MEMPROTECT_WRITECOMBINE, FALSE, XALLOC_MEMTYPE_PHYSICAL );
	BYTE* pTextureData = ( BYTE* )XMemAlloc( dwTextureDataSize, dwAllocAttributes );

	file->Read(pTextureData, dwTextureDataSize);

	file->Release();


	//texture->Fence = 0;

	// Now fix up the texture header to point to the base and mip addresses inside the texture data.
	// This is the final step - once this is done, the texture is ready to be used!
	D3DBaseTexture *pBaseTexture = ( D3DBaseTexture* )texture;
	XGOffsetBaseTextureAddress( pBaseTexture, pTextureData, pTextureData );

	//api->Trace("Texture name : '%s'", strFullPath.c_str());
	pDX8Texture->CreateFromGPUTexture(texture, pTextureData, dwAllocAttributes, dwTextureDataSize, fileName);

	return pDX8Texture;

}




#endif


