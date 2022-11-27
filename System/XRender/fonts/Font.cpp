#include "Font.h"

IRender * CDX8Font::pRS = null;
IRender * RDX8Font::pRS = null;

CDX8Fonts::CDX8Fonts(IRender * _pRS) :
	aCFonts(_FL_),
	aRFonts(_FL_)
{
	pRS = _pRS;
	CDX8Font::pRS = _pRS;
	RDX8Font::pRS = _pRS;
}

CDX8Fonts::~CDX8Fonts()
{
	dword i;

	for (i=0; i<aCFonts(); i++) DELETE(aCFonts[i]);
	aCFonts.DelAll();
	for (i=0; i<aRFonts(); i++) DELETE(aRFonts[i]);
	aRFonts.DelAll();
}

bool CDX8Fonts::Release(IFont * pFont)
{
	if (!pFont) return false;
	aCFonts.Del((CDX8Font*)pFont);
	return true;
}


IBaseTexture * RDX8Font::GetTexture() const
{
	return pFntTexture;
}

IFont * CDX8Fonts::CreateFont(const char * pFontName, float fHeight, dword dwColor)
{
	CDX8Font * pCFont = null;
	for (dword i=0; i<aRFonts(); i++) if (aRFonts[i]->GetName() == pFontName)
	{
		aRFonts[i]->Create(pFontName);
		pCFont = NEW CDX8Font(aRFonts[i], fHeight, dwColor);
		aCFonts.Add(pCFont);
		return pCFont;
	}

	RDX8Font * pRFont = NEW RDX8Font();
	pRFont->Create(pFontName);
	aRFonts.Add(pRFont);

	pCFont = NEW CDX8Font(pRFont, fHeight, dwColor);

	aCFonts.Add(pCFont);
	return pCFont;
}
