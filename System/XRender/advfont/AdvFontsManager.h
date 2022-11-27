#ifndef __ADV_FONTS_MANAGER__IMPL___
#define __ADV_FONTS_MANAGER__IMPL___


#include "..\..\..\common_h\Render.h"
#include "..\..\..\common_h\templates\array.h"
#include "AdvFont.h"


class AdvancedFonts
{
	CritSection critSection;

	string buffer1;
	string buffer2;

public:

	AdvancedFonts();
	virtual ~AdvancedFonts();

	IAdvFont	* CreateAdvancedFont(const char * pFontName, float fHeight, dword dwColor, const char * fntShader = NULL, const char * circularShader = NULL, const char * circularShdwShader = NULL);
	bool	Release(IAdvFont * pFont);

	void Refresh(IAdvFont * pFont);

private:

	void CreateBuffers();

	IVBuffer* pVB;
	IIBuffer* pIB;

	array<RenderAdvFont*> createdFonts;

};


#endif
