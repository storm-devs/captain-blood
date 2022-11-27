#ifndef DX8TEXTURE_HPP
#define DX8TEXTURE_HPP

#include "..\..\..\common_h\Render.h"
#include "BaseTexture.h"


class CDX8Texture : public ITexture
{


	struct createParams
	{
		bool bRenderTarget;
		bool bLinear;

		dword Width;
		dword Height;
		dword Levels;
		dword Usage;
		RENDERFORMAT Format;
		RENDERPOOL Pool;
	};

	createParams initParams;


public:
	bool		bUpdateTexture;

#ifdef _XBOX
	bool bDirectGPUMemory;
	BYTE* pTextureData;
	DWORD dwAllocAttributes;
#endif

	CDX8Texture();
	~CDX8Texture();

	// create inner resource object
	bool Create(dword Width, dword Height, dword Levels, dword Usage, RENDERFORMAT Format, RENDERPOOL Pool);
	bool CreateLinear(dword Width, dword Height, dword Levels, dword Usage, RENDERFORMAT Format, RENDERPOOL Pool);
	
	// release inner resource object
	virtual bool Reset();
	// recreate inner resource object
	virtual bool Recreate();

	bool		GetLevelDesc(dword Level, RENDERSURFACE_DESC * pDesc);
	bool		LockRect(dword Level, RENDERLOCKED_RECT * pLockedRect, CONST RECT * pRect, dword Flags);
	bool		UnlockRect(dword Level);
	void		SetSysMemTexture(CDX8Texture * pSysMemTexture);
	CDX8Texture * GetSysMemTexture();
	void		UpdateTexture();

	static	IDirect3DDevice9		* pD3D8;

	//virtual IDirect3DBaseTexture9 * GetBaseTexture();


	bool CreateUseD3DX (const char* fileName);

/*
#ifdef _XBOX
	void ConvertToX360 ();
#endif
*/

#ifdef _XBOX
	void CreateFromGPUTexture (D3DTexture *pBaseTexture, BYTE* pTextureData, DWORD dwAllocAttributes, DWORD dataSize, const char * pixGUID);
#endif

private:

	CDX8Texture			* pSysMemTexture;

	long				iNumLocks;
	//IDirect3DTexture9	* pTexture;								// d3d texture

	// Resource implementation
	DX8_RESOURCE_IMPLEMENT

	virtual bool	Release();
	virtual bool	ForceRelease();


	// Base Texture implementation
	DX8_BASETEXTURE_IMPLEMENT
};

#endif