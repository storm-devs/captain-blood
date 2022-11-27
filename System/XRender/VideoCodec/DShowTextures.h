#ifndef _XBOX

#include <windows.h>
#include <d3d9types.h>
#include "Sdk_BaseClasses\streams.h"

//-----------------------------------------------------------------------------
// Define GUID for Texture Renderer
// {71771540-2017-11cf-AE26-0020AFD79767}
//-----------------------------------------------------------------------------
struct __declspec(uuid("{71771540-2017-11cf-ae26-0020afd79767}")) CLSID_TextureRenderer;

class IWMVTexture;

//-----------------------------------------------------------------------------
// CTextureRenderer Class Declarations
//-----------------------------------------------------------------------------
class CTextureRenderer : public CBaseVideoRenderer
{
public:
    CTextureRenderer(LPUNKNOWN pUnk,HRESULT *phr);
    ~CTextureRenderer();

private:
    HRESULT CheckMediaType(const CMediaType *pmt );     // Format acceptable?
    HRESULT SetMediaType(const CMediaType *pmt );       // Video format notification
    HRESULT DoRenderSample(IMediaSample *pMediaSample); // New video sample

    BOOL m_bUseDynamicTextures;
    LONG m_lVidWidth;   // Video width
    LONG m_lVidHeight;  // Video Height

	IWMVTexture* m_pTexture;
	D3DFORMAT       m_TextureFormat; // Texture format
};

#endif
