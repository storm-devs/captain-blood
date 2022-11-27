#include "AdvFontsManager.h"
#include "..\Render.h"


AdvancedFonts::AdvancedFonts() : createdFonts (_FL_)
{
	pVB = NULL;
	pIB = NULL;

	CreateBuffers();

}


AdvancedFonts::~AdvancedFonts()
{
	if (pVB)
	{
		pVB->Release();
		pVB = NULL;
	}

	if (pIB)
	{
		pIB->Release();
		pIB = NULL;
	}
}

void AdvancedFonts::CreateBuffers()
{
	pVB = NGRender::pRS->CreateVertexBuffer( sizeof(RenderAdvFont::Vertex) * icon_max_count * 4, sizeof(RenderAdvFont::Vertex), _FL_, USAGE_WRITEONLY | USAGE_DYNAMIC, POOL_DEFAULT);
	Assert(pVB)

		pIB = NGRender::pRS->CreateIndexBuffer(sizeof(WORD) * icon_max_count * 6, _FL_, USAGE_WRITEONLY | USAGE_DYNAMIC);
	Assert(pIB)

		WORD *p = (WORD *)pIB->Lock();
	Assert(p)

		for( long i = 0 ; i < icon_max_count ; i++ )
		{
			p[i*6 + 0] = WORD(i*4 + 0);
			p[i*6 + 1] = WORD(i*4 + 1);
			p[i*6 + 2] = WORD(i*4 + 2);
			p[i*6 + 3] = WORD(i*4 + 0);
			p[i*6 + 4] = WORD(i*4 + 2);
			p[i*6 + 5] = WORD(i*4 + 3);
		}

		pIB->Unlock();
}

void AdvancedFonts::Refresh(IAdvFont * pFont)
{
	if (pFont)
	{
		RenderAdvFont* fnt = (RenderAdvFont*)pFont;
		fnt->PostInit();
	}

}

IAdvFont * AdvancedFonts::CreateAdvancedFont(const char * pFontName, float fHeight, dword dwColor, const char * fntShader, const char * circularShader, const char * circularShdwShader)
{
//	RenderAdvFont * pFont = null;
/*	for (dword i = 0; i < createdFonts.Size(); i++)
	{
		RenderAdvFont* fnt = createdFonts[i];

		if (fnt->GetName() == pFontName)
		{
			fnt->PostInit();

			fnt->AddRef();
			return fnt;
		}
	}*/

	

	RenderAdvFont* pNewFont = NEW RenderAdvFont(&critSection, &buffer1, &buffer2, pVB, pIB, pFontName, fHeight, dwColor, fntShader, circularShader, circularShdwShader);
	createdFonts.Add(pNewFont);

	return pNewFont;
}

bool AdvancedFonts::Release(IAdvFont * pFont)
{
	if (!pFont)
	{
		return false;
	}

	createdFonts.Del((RenderAdvFont*)pFont);
	return true;
}
