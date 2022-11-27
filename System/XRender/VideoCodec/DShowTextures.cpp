#ifndef _XBOX

#include "DShowTextures.h"
#include <d3d9caps.h>
#include "wmvinterfaces.h"

//-----------------------------------------------------------------------------
// CTextureRenderer constructor
//-----------------------------------------------------------------------------
CTextureRenderer::CTextureRenderer( LPUNKNOWN pUnk, HRESULT *phr )
                                  : CBaseVideoRenderer(__uuidof(CLSID_TextureRenderer),
                                    NAME("Texture Renderer"), pUnk, phr),
                                    m_bUseDynamicTextures(FALSE)
{
	m_pTexture = NULL;
    // Store and AddRef the texture for our use.
    ASSERT(phr);
    if (phr)
        *phr = S_OK;
}


//-----------------------------------------------------------------------------
// CTextureRenderer destructor
//-----------------------------------------------------------------------------
CTextureRenderer::~CTextureRenderer()
{
	if( m_pTexture )
		m_pTexture->Release();
	m_pTexture = NULL;
}


//-----------------------------------------------------------------------------
// CheckMediaType: This method forces the graph to give us an R8G8B8 video
// type, making our copy to texture memory trivial.
//-----------------------------------------------------------------------------
HRESULT CTextureRenderer::CheckMediaType(const CMediaType *pmt)
{
    HRESULT   hr = E_FAIL;
    VIDEOINFO *pvi=0;

    CheckPointer(pmt,E_POINTER);

    // Reject the connection if this is not a video type
    if( *pmt->FormatType() != FORMAT_VideoInfo ) {
        return E_INVALIDARG;
    }

    // Only accept RGB24 video
    pvi = (VIDEOINFO *)pmt->Format();

    if(IsEqualGUID( *pmt->Type(),    MEDIATYPE_Video)  &&
	   IsEqualGUID( *pmt->Subtype(), MEDIASUBTYPE_YV12))
    {
        hr = S_OK;
    }

    return hr;
}

//-----------------------------------------------------------------------------
// SetMediaType: Graph connection has been made.
//-----------------------------------------------------------------------------
HRESULT CTextureRenderer::SetMediaType(const CMediaType *pmt)
{
    UINT uintWidth = 2;
    UINT uintHeight = 2;

    // Retrive the size of this media type
    VIDEOINFO *pviBmp;                      // Bitmap info header
    pviBmp = (VIDEOINFO *)pmt->Format();

    m_lVidWidth  = pviBmp->bmiHeader.biWidth;
    m_lVidHeight = abs(pviBmp->bmiHeader.biHeight);

	uintWidth = m_lVidWidth;
	uintHeight = ((m_lVidHeight*3 + 1) >> 1); // summary height = 1.5 x height

	m_pTexture = IWMVTexture::Create(uintWidth, uintHeight);
	if( !m_pTexture )
		return E_FAIL;

    return S_OK;
}


//-----------------------------------------------------------------------------
// DoRenderSample: A sample has been delivered. Copy it to the texture.
//-----------------------------------------------------------------------------
HRESULT CTextureRenderer::DoRenderSample( IMediaSample * pSample )
{
    BYTE  * pbS = NULL;
    DWORD * pdwS = NULL;
    DWORD * pdwD = NULL;
    UINT row, q;// col, dwordWidth;

    CheckPointer(pSample,E_POINTER);
    CheckPointer(m_pTexture,E_UNEXPECTED);

    // Get the video bitmap buffer
    BYTE  *pBmpBuffer; // Bitmap buffer
    pSample->GetPointer( &pBmpBuffer );

	void* pBuff;
	int nPitch;
	if( m_pTexture->Lock(pBuff,nPitch) )
	{
		BYTE* pTxtBuffer = (BYTE*)pBuff;
		LONG lTxtPitch = nPitch;

		// fill Y texture
		LONG lVidPitch = m_lVidWidth;
		q = m_lVidHeight;
		for( row = 0; row < q; row++ )
        {
			memcpy(pTxtBuffer,pBmpBuffer,m_lVidWidth);
            pBmpBuffer += lVidPitch;
            pTxtBuffer += lTxtPitch;
        }

		// fill U,V texture
		lVidPitch = (m_lVidWidth >> 1);
		q = (m_lVidHeight >> 1);
		LONG lBmpOffsetV = lVidPitch * q;
		for( row = 0; row < q; row++)
        {
			// U
			memcpy(pTxtBuffer,pBmpBuffer,lVidPitch);
			// V
			memcpy(pTxtBuffer + lVidPitch, pBmpBuffer+lBmpOffsetV, lVidPitch);
			// next texture row
			pBmpBuffer += lVidPitch;
            pTxtBuffer += lTxtPitch;
        }
		m_pTexture->Unlock();
	}

	pSample->Release();

    return S_OK;
}

#endif
