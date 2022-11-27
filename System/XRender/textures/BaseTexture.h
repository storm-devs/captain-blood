
#ifndef DX8BASETEXTURE_HPP
#define DX8BASETEXTURE_HPP

#include "..\GraphicsApi.h"

#include "..\..\..\common_h\Render.h"
#include "..\Resource.h"
#include "..\D3DConvertor.h"

/*
class IRBaseTexture
{
public:
	virtual IDirect3DBaseTexture9	* GetBaseTexture() = 0;
};
*/

class CDX8BaseTexture : public IBaseTexture
{
public:
	CDX8BaseTexture();
	virtual ~CDX8BaseTexture();

	virtual bool		Release() { return Resource.Release(); }
	virtual bool		ForceRelease() { return Resource.ForceRelease(); }

	virtual const char*		GetName() const { return (const char *)sName.GetBuffer(); };
	virtual dword		GetNameHash() const { return dwNameHash; };

	
	virtual void			SetName(const char * pName)
	{ 
		sName = pName; 
		dwNameHash = string::HashNoCase(sName.c_str());
	}
	virtual const string &	GetTexName() const { return sName; };
	virtual dword			GetLevelCount() const { return dwLevels; };
	virtual dword			GetLOD() const; 
	virtual dword			SetLOD(dword dwLOD);

	virtual RENDERFORMAT	GetFormat()	const { return FormatFromDX (Format); };
	virtual RENDERPOOL		GetPool()	const { return PoolFromDX (Pool); };
	virtual dword		GetUsage()	const { return dwUsage; };

	virtual dword		GetWidth() const { return dwWidth; };
	virtual dword		GetHeight() const { return dwHeight; };
	virtual dword		GetDepth() const { return dwDepth; };

	virtual bool		IsRenderTarget() const { return (dwUsage & D3DUSAGE_RENDERTARGET); };

	string				sName;

	dword dwNameHash;

	dword				dwWidth, dwHeight, dwDepth; 
	dword				dwLevels, dwUsage;
	D3DFORMAT			Format;
	D3DPOOL				Pool;

protected:

	// Resource implementation
	DX8_RESOURCE_IMPLEMENT
};

// ================================================================================================
#define DX8_BASETEXTURE_IMPLEMENT															\
public:																						\
	virtual const char*		GetName() const 			{ return BaseTexture.GetName(); };		\
	virtual dword		GetNameHash() const 			{ return BaseTexture.GetNameHash(); };		\
	virtual const string &	GetTexName() const			{ return BaseTexture.GetTexName(); };	\
	virtual void		SetName(const char * pName)	{ BaseTexture.SetName(pName); };			\
	virtual dword		GetLevelCount()	const		{ return BaseTexture.GetLevelCount(); };	\
	virtual dword		GetLOD() const 				{ return BaseTexture.GetLOD(); };			\
	virtual dword		SetLOD(dword dwLOD)			{ return BaseTexture.SetLOD(dwLOD); };		\
	virtual RENDERFORMAT	GetFormat() const			{ return BaseTexture.GetFormat(); };		\
	virtual RENDERPOOL		GetPool() const 			{ return BaseTexture.GetPool(); };			\
	virtual dword		GetUsage() const			{ return BaseTexture.GetUsage(); };			\
	virtual dword		GetWidth() const			{ return BaseTexture.GetWidth(); };			\
	virtual dword		GetHeight() const			{ return BaseTexture.GetHeight(); };		\
	virtual dword		GetDepth() const			{ return BaseTexture.GetDepth(); };			\
	virtual bool		IsRenderTarget() const		{ return BaseTexture.IsRenderTarget(); };	\
	virtual void OnResetDevice(); \
	virtual void OnLostDevice(); \
private:																						\
	CDX8BaseTexture		BaseTexture;															

#endif