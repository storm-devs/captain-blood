#ifndef _XBOX

#include "X360TexConvertor.h"
#include "TXX.h"




//c++->general->additional include directories
//C:\Program Files (x86)\Microsoft Xbox 360 SDK\include\win32\vs2005;C:\Program Files\Microsoft Xbox 360 SDK\include\win32\vs2005

//linker->genreal->additional livrary directories
//C:\Program Files (x86)\Microsoft Xbox 360 SDK\lib\win32\vs2005;C:\Program Files\Microsoft Xbox 360 SDK\lib\win32\vs2005


#include <d3d9.h>
#include <xgraphics.h>



#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "xgraphics.lib")





struct STORMTEXFORMAT
{
	TXXFormat	txFormat;
	D3DFORMAT	d3dFormat_tiled;
	D3DFORMAT	d3dFormat_lin;
	bool		isSwizzled;
	dword		pitchDivider;
	const char	* pFormatName;
};


STORMTEXFORMAT TexFormats[] =
{
	{txxf_dxt1,		D3DFMT_DXT1,		D3DFMT_LIN_DXT1,		true,	4, "D3DFMT_DXT1"},
	{txxf_dxt3,		D3DFMT_DXT3,		D3DFMT_LIN_DXT3,		true,	4, "D3DFMT_DXT3"},
	{txxf_dxt5,		D3DFMT_DXT5,		D3DFMT_LIN_DXT5,		true,	4, "D3DFMT_DXT5"},
	{txxf_a8r8g8b8,	D3DFMT_A8R8G8B8,	D3DFMT_LIN_A8R8G8B8,	false,	1, "D3DFMT_A8R8G8B8"},
	{txxf_r5g6b5,	D3DFMT_R5G6B5,		D3DFMT_LIN_R5G6B5,		false,	1, "D3DFMT_R5G6B5"},
	{txxf_a4r4g4b4,	D3DFMT_A4R4G4B4,	D3DFMT_LIN_A4R4G4B4,	false,	1, "D3DFMT_A4R4G4B4"},
	{txxf_a1r5g5b5,	D3DFMT_A1R5G5B5,	D3DFMT_LIN_A1R5G5B5,	false,	1, "D3DFMT_A1R5G5B5"},
	{txxf_q8w8v8u8, D3DFMT_Q8W8V8U8,	D3DFMT_LIN_Q8W8V8U8,	false,	1, "D3DFMT_Q8W8V8U8"},
	{txxf_v8u8,		D3DFMT_V8U8,		D3DFMT_LIN_V8U8,		false,	1, "D3DFMT_V8U8"},
	{txxf_a8,		D3DFMT_A8,			D3DFMT_LIN_A8,			false,	1, "D3DFMT_A8"},
	{txxf_l8,		D3DFMT_L8,			D3DFMT_LIN_L8,			false,	1, "D3DFMT_L8"},
	{txxf_unknown,	D3DFMT_X8R8G8B8,	D3DFMT_LIN_X8R8G8B8,	false,	1, "D3DFMT_X8R8G8B8"},
	{txxf_unknown,	D3DFMT_D24S8,		D3DFMT_LIN_D24S8,		false,  1, "D3DFMT_D24S8"},
	{txxf_unknown,	D3DFMT_D16,			D3DFMT_LIN_D16,			false,	1, "D3DFMT_D16"},
	{txxf_unknown,	D3DFMT_D24X8,		D3DFMT_LIN_D24X8,		false,	1, "D3DFMT_D24X8"},
	/* X360 unsupported
	{txxf_unknown,	D3DFMT_D24X4S4,		false,	1, "D3DFMT_D24X4S4"},
	{txxf_unknown,	D3DFMT_D15S1,		false,	1, "D3DFMT_D15S1"},
	*/
	{txxf_unknown,	D3DFMT_D32,			D3DFMT_LIN_D32,			false,	1, "D3DFMT_D32"}
};


STORMTEXFORMAT* GetFormat(TXXFormat txFormat)
{
	for (dword i=0; i < sizeof(TexFormats) / sizeof(STORMTEXFORMAT); i++)
	{
		if (TexFormats[i].txFormat == txFormat)
		{
			return &TexFormats[i];
		}
	}
	return null;
}



void XboxConvert_txx(const char * from, const char * to)
{
	IFileService* pFS = (IFileService *)api->GetService("FileService");

	ILoadBuffer * txxFile = pFS->LoadData(from, _FL_);

	if(!txxFile)
	{
		api->Trace("TexConvertor: Can't find texture : '%s'", from);
		return;
	}

	const byte * txxData = txxFile->Buffer();
	const byte * txxEOF = txxData + txxFile->Size();

	if(txxFile->Size() < sizeof(TxxFileHeader))
	{
		api->Trace("TexConvertor: Texture dont have enught size: '%s'", from);
		return;
	}

	TxxFileHeader & head = *(TxxFileHeader *)txxData;
	txxData += sizeof(TxxFileHeader);

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

	if(head.id != TXX_ID || head.ver != TXX_VER)
	{
		api->Trace("TexConvertor: Texture is damage: '%s'", from);
		return;		
	}

	STORMTEXFORMAT * pTexFormat = null;
	for (dword i=0; i < sizeof(TexFormats) / sizeof(STORMTEXFORMAT); i++)
	{
		if (TexFormats[i].txFormat == head.format)
		{
			pTexFormat = &TexFormats[i];
			break;
		}
	}
	if (!pTexFormat)
	{
		api->Trace("TexConvertor: unknown texture format: %d, texture: %s", head.format, from);
		txxFile->Release();
		return;
	}

	if (head.type == txxt_2d && head.type != txxt_rendertarget) // if simple texture
	{
		UINT m_dwBaseSize_linear = 0;
		UINT m_dwMipSize_linear = 0;

		UINT m_dwBaseSize_tiled = 0;
		UINT m_dwMipSize_tiled = 0;

		D3DBaseTexture linearTexture;
		D3DBaseTexture tiledTexture;
		
		// Now set the texture header with the information we've obtained so far.
		XGSetTextureHeaderEx( head.width, head.height, head.num_mips, 0, pTexFormat->d3dFormat_lin, 0, 0, 0, 
			XGHEADER_CONTIGUOUS_MIP_OFFSET, 0,  ( D3DTexture* )&linearTexture,  &m_dwBaseSize_linear,  &m_dwMipSize_linear );

		// Now set the texture header with the information we've obtained so far.
		XGSetTextureHeaderEx( head.width, head.height, head.num_mips, 0, pTexFormat->d3dFormat_tiled, 0, 0, 0, 
			XGHEADER_CONTIGUOUS_MIP_OFFSET, 0,  ( D3DTexture* )&tiledTexture,  &m_dwBaseSize_tiled,  &m_dwMipSize_tiled );


		// Create a buffer to hold the texture data.
		byte* m_pBuffer_linear = new byte[ m_dwBaseSize_linear + m_dwMipSize_linear ];

		byte* m_pBuffer_tiled = new byte[ m_dwBaseSize_tiled + m_dwMipSize_tiled ];

		XGTEXTURE_DESC BaseDesc;
		XGGetTextureDesc( &tiledTexture, 0, &BaseDesc );

		for (dword dwLevel = 0; dwLevel < head.num_mips; dwLevel++)
		{
			// Get the description of the current mip level.
			XGTEXTURE_DESC mipDesc_lin;
			XGGetTextureDesc( &linearTexture, dwLevel, &mipDesc_lin );

			XGTEXTURE_DESC mipDesc_tiled;
			XGGetTextureDesc( &tiledTexture, dwLevel, &mipDesc_tiled );


			// Get the mip level offset within our buffer.
			DWORD dwMipLevelOffset_linear = XGGetMipLevelOffset( &linearTexture, 0, dwLevel );
			if( ( dwLevel > 0 ) && ( m_dwMipSize_linear > 0 ) )
			{
				dwMipLevelOffset_linear += m_dwBaseSize_linear;
			}
			BYTE* mipData_linear = m_pBuffer_linear + dwMipLevelOffset_linear;


			// Get the mip level offset within our buffer.
			DWORD dwMipLevelOffset_tiled = XGGetMipLevelOffset( &tiledTexture, 0, dwLevel );
			if( ( dwLevel > 0 ) && ( m_dwMipSize_tiled > 0 ) )
			{
				dwMipLevelOffset_tiled += m_dwBaseSize_tiled;
			}
			BYTE* mipData_tiled = m_pBuffer_tiled + dwMipLevelOffset_tiled;



			//load surface....
			dword txxLineForRead = head.line_size;
			dword txxLines = head.height;
			switch (head.format)
			{
			case txxf_dxt1:
				txxLineForRead = (head.width/4)*64/8;
				txxLines /= 4;
				Assert(txxLines == mipDesc_lin.HeightInBlocks);
				break;
			case txxf_dxt3:
				txxLineForRead = (head.width/4)*64/8*2;
				txxLines /= 4;
				Assert(txxLines == mipDesc_lin.HeightInBlocks);
				break;
			case txxf_dxt5:
				txxLineForRead = (head.width/4)*64/8*2;
				txxLines /= 4;
				Assert(txxLines == mipDesc_lin.HeightInBlocks);
				break;
			}

			
			Assert(mipDesc_lin.RowPitch >= txxLineForRead);
			byte* destPtr = mipData_linear;
			for (dword line = 0; line < txxLines; line++)
			{
				Assert(txxData + txxLineForRead <= txxEOF);
				memcpy(destPtr, txxData, txxLineForRead);				

				txxData += txxLineForRead;
				destPtr += mipDesc_lin.RowPitch;
			}

			XGEndianSwapSurface(mipData_linear, mipDesc_lin.RowPitch, mipData_linear,	mipDesc_lin.RowPitch, mipDesc_lin.Width, mipDesc_lin.Height, mipDesc_lin.Format);

			

			XGTileTextureLevel(  
				BaseDesc.Width, 
				BaseDesc.Height,
				dwLevel,
				XGGetGpuFormat( mipDesc_tiled.Format ),
				0,
				mipData_tiled,
				NULL,
				mipData_linear, 
				mipDesc_lin.RowPitch,
				NULL );


			head.line_size /= 2;
			head.width /= 2;
			head.height /= 2;
		}
		//----- тут получили линейную и тайленую текстуры для X360, можно любую писать на диск--------



		XGEndianSwapTextureHeader(( D3DTexture* )&tiledTexture);


		DWORD dwWritten = 0;
		HANDLE hFile = CreateFile(to, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, null, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, null);

		// Write the D3D texture header.
		WriteFile( hFile, &tiledTexture, sizeof( tiledTexture ), &dwWritten, NULL );
		//Write texture data.
		WriteFile( hFile, m_pBuffer_tiled, m_dwBaseSize_tiled + m_dwMipSize_tiled, &dwWritten, NULL);

		CloseHandle( hFile );

		delete [] m_pBuffer_linear;
		delete [] m_pBuffer_tiled;

		m_pBuffer_linear = NULL;
		m_pBuffer_tiled = NULL;


	} else
	{
		api->Trace("TexConvertor: %s is not simple 2d texture or render target texture !!!", from);
		txxFile->Release();
		return;
	}


	txxFile->Release();
}




/*


код загрузки для X360
============================================================

D3DTexture g_Texture;

HANDLE hFile = CreateFile( "game:\\stonewall.tex", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

DWORD dwSize = GetFileSize( hFile, NULL );
DWORD dwTextureDataSize = dwSize - sizeof( D3DTexture );

// First load the D3DTexture structure contents so we have a valid texture header. 
ReadFile( hFile, &g_Texture, sizeof( g_Texture ), &dwRead, NULL );


// Now allocate space for the texture data. This needs to be allocated on 4K boundary in physical memory. 
DWORD dwAllocAttributes = MAKE_XALLOC_ATTRIBUTES( 0, FALSE, FALSE, FALSE, 0, XALLOC_PHYSICAL_ALIGNMENT_4K, XALLOC_MEMPROTECT_WRITECOMBINE, FALSE, XALLOC_MEMTYPE_PHYSICAL );
BYTE *g_pTextureData = ( BYTE* )XMemAlloc( dwTextureDataSize, dwAllocAttributes );

ReadFile( hFile, g_pTextureData, dwTextureDataSize, &dwRead, NULL );

CloseHandle( hFile );


// Now fix up the texture header to point to the base and mip addresses inside the texture data.
// This is the final step - once this is done, the texture is ready to be used!
D3DBaseTexture *pBaseTexture = ( D3DBaseTexture* )&g_Texture;
XGOffsetBaseTextureAddress( pBaseTexture, g_pTextureData, g_pTextureData );


============================================================
готово!!!

*/



#endif