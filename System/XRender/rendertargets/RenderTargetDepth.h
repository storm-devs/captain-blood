#ifndef DEPTH_RENDER_TARGET
#define DEPTH_RENDER_TARGET

#include "..\GraphicsApi.h"
#include "..\..\..\common_h\Render.h"

#ifdef _XBOX
#include "..\device\X360_Device.h"
#endif



class RenderTargetDepth : public IRenderTargetDepth
{
protected:

	bool bRestoreMode;


	struct createParams
	{
		dword Width;
		dword Height;
		RENDERFORMAT Format;
		RENDERMULTISAMPLE_TYPE MultiSample;
	};

	createParams initParams;


	virtual ~RenderTargetDepth();

	dword mem_size;

	const char* srcFile;
	long srcLine;


public:


	RenderTargetDepth();

	

	//Если canResolve = false, то невозможно будет делать данному RT resolve, но и памяти он занимать не будет
	virtual bool Create (dword Width, dword Height, RENDERFORMAT Format, RENDERMULTISAMPLE_TYPE MultiSample, bool canResolve = true);
	virtual void CreateFromDX (IDirect3DSurface9* depthStencilSurface, bool bRestoreMode);

	virtual DX8RESOURCETYPE	GetSubType() const;
	virtual bool GetDesc(RENDERSURFACE_DESC * pDesc);

	//===============================================================


	IDirect3DSurface9* GetD3DSurface();

	void OnResetDevice();
	void OnLostDevice();





	//===============================================================
private:

	long refCount;

public:

	virtual long GetRef()
	{
		return refCount;
	}


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

	virtual DX8RESOURCETYPE GetType() const;
	virtual dword GetSize() const;

	virtual const char* GetFileName() const;
	virtual long GetFileLine() const;

	virtual void SetFileLine(const char * pFileName, long iLine);

#ifdef _XBOX

	PredicatedTilingInfo TI_OnlyDepth;

	virtual IBaseTexture* AsTexture ();

	const PredicatedTilingInfo& GetTileInfoDepthOnly ();

	void CalculateEDRAMOffset ();
#endif


	


private:

#ifdef _XBOX

	IBaseTexture* m_DepthTexture;

#endif

	IDirect3DSurface9* m_surface;

};

#endif




