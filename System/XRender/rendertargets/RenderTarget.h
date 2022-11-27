#ifndef COLOR_RENDER_TARGET
#define COLOR_RENDER_TARGET

#include "..\GraphicsApi.h"
#include "..\..\..\common_h\Render.h"

#ifdef _XBOX
#include "..\device\X360_Device.h"
#endif


#define MAX_MIPCOUNT 32

class RenderTarget : public IRenderTarget
{
protected:

	bool bRestoreMode;

	virtual ~RenderTarget();

	RENDERSURFACE_DESC dsc;

	dword mem_size;

	const char* srcFile;
	long srcLine;


	struct createParams
	{
		dword Width;
		dword Height;
		RENDERFORMAT Format;
		RENDERMULTISAMPLE_TYPE MultiSample;
		long MipLevelsCount;
	};

	createParams initParams;



public:

	RenderTarget();

	bool Create (dword Width, dword Height, RENDERFORMAT Format, RENDERMULTISAMPLE_TYPE MultiSample, long MipLevelsCount);
	void CreateFromDX (IDirect3DSurface9* depthStencilSurface, bool bRestoreMode);

	virtual DX8RESOURCETYPE	GetSubType() const;
	virtual bool GetDesc(RENDERSURFACE_DESC * pDesc);

	virtual IBaseTexture* AsTexture ();


	virtual void CopyToTexture (IBaseTexture* pDestiantionTexture);


//===============================================================
	
	
	IDirect3DSurface9* GetD3DSurface();

	void OnResetDevice();
	void OnLostDevice();




//===============================================================
private:

	long refCount;

public:

	// release inner resource object
	virtual bool Reset() { return false; };
	// recreate inner resource object
	virtual bool Recreate() { return false; };
	// return true if resource is loaded
	virtual bool IsLoaded() { return false; };
	// return true if resource can't load or not created
	virtual bool IsError() { return false; };

	virtual void	AddRef();
	virtual bool	Release();
	virtual bool	ForceRelease();

	virtual long GetRef()
	{
		return refCount;
	}

	virtual DX8RESOURCETYPE GetType() const;
	virtual dword GetSize() const;

	virtual const char* GetFileName() const;
	virtual long GetFileLine() const;

	virtual void SetFileLine(const char * pFileName, long iLine);


	long GetMipCount();
	IDirect3DSurface9* GetTextureSurface(long mipIndex);

	bool NeedCopy();


#ifdef _XBOX

	PredicatedTilingInfo TI_OnlyColor;
	PredicatedTilingInfo TI_ColorAndDepth;
	
		

	void CalculateEDRAMOffset ();


	const PredicatedTilingInfo& GetTileInfo ();
	const PredicatedTilingInfo& GetTileInfoWithDepth ();

	virtual bool FitEDRAMWithMainRT();

	//const PredicatedTilingInfo& GetTileInfoDepthOnly ();

	
#endif

private:


	bool bNeedCopy;
	
	//Куда рисовать - может быть 0 мипом текстуры или рендертаргетом
	IDirect3DSurface9* m_surface;


	IDirect3DSurface9* m_TextureSurfaces[MAX_MIPCOUNT];


	IBaseTexture* m_texture;




};

#endif
