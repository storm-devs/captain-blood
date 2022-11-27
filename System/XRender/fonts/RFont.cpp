#include "Font.h"
#include "..\Render.h"

IVBuffer *	RDX8Font::pVBuffer = null;
IIBuffer *	RDX8Font::pIBuffer = null;
dword		RDX8Font::dwVBufferVertices = 0;
dword		RDX8Font::dwVBufferRefCount = 0;

RDX8Font::RDX8Font()
{
	dwVBufferRefCount++;
	pFntTexture = null;
	dwRefCount = 0;
	fDefaultKerning = 0.0f;
}

RDX8Font::~RDX8Font()
{
	dwVBufferRefCount--;
	
	RELEASE(pFntTexture); 
	if (!dwVBufferRefCount)	
	{
		RELEASE(pIBuffer);
		RELEASE(pVBuffer);
	}
	
	pFntTexture = null;
}

dword RDX8Font::AddRef()
{
	return dwRefCount++;
}

dword RDX8Font::Release()
{
	dwRefCount--;

	if (!dwRefCount) 
	{
		RELEASE(pFntTexture); 
		return true;
	}
	return false;
}

void RDX8Font::Create(const char * pFontName)
{
	if (dwRefCount) return;	// already loaded

	char str[_MAX_PATH], str1[_MAX_PATH];
	ZERO(Symbols);

	IFileService * fs = (IFileService *)api->GetService("FileService");
	Assert(fs);
	crt_snprintf(str, _MAX_PATH-1, "resource\\ini\\fonts\\%s.ini", pFontName);
	IIniFile * pI = fs->OpenIniFile(str, _FL_);
	if (!pI) 
	{
		api->Trace("RDX8Font error: can't find resource\\ini\\fonts\\%s.ini file", pFontName);
		return;
	}

	if(!pI->IsSectionCreated((char*)pFontName))
	{
		api->Trace("RDX8Font error: section [%s] not found in resource\\ini\\fonts\\%s.ini file", pFontName, pFontName);
		return;
	}
	
	fTexSizeX = pI->GetFloat((char*)pFontName, "sizex", -1);
	if(fTexSizeX <= 0.0f)
	{
		api->Trace("RDX8Font error: invalidate <sizex> in resource\\ini\\fonts\\%s.ini file", pFontName);
		return;
	}

	fTexSizeY = pI->GetFloat((char*)pFontName, "sizey", -1);
	if(fTexSizeY <= 0.0f)
	{
		api->Trace("RDX8Font error: invalidate <sizey> in resource\\ini\\fonts\\%s.ini file", pFontName);
		return;
	}

	fOriginalHeight = pI->GetFloat((char*)pFontName, "height", -1);
	if(fOriginalHeight <= 0.0f)
	{
		api->Trace("RDX8Font error: invalidate <sizex> in resource\\ini\\fonts\\%s.ini file", pFontName);
		return;
	}

	fDefaultKerning = pI->GetFloat((char*)pFontName, "kerning", 0);

/*
	if (NGRender::pRS->IsWideScreen())
	{
		fOriginalHeight = fOriginalHeight * 1.3333333f;
	}
	*/

	


	fDV = fOriginalHeight / fTexSizeY;


	for (dword i=0; i<256; i++)
	{
		crt_snprintf(str, _MAX_PATH, "key_%d", i);
		str1[0] = 0;
		const char * value = pI->GetString(pFontName, str);

		if (value && value[0]) crt_sscanf(value, "%f, %f, %f", &Symbols[i].fStartU, &Symbols[i].fStartV, &Symbols[i].fDU);
		
		Symbols[i].fStartU /= fTexSizeX;
		Symbols[i].fStartV /= fTexSizeY;
		Symbols[i].fDU /= fTexSizeX;
	}	

	sName = pFontName;
	sName.Lower();

	ReserveBuffer(4 * 4096);

	const char * textureName = pI->GetString(pFontName, "texture");
	bool bOMFC = ((NGRender*)pRS)->SetUseMipFillColor(false);
	pFntTexture = pRS->CreateTextureFullQuality(_FL_, textureName);
	((NGRender*)pRS)->SetUseMipFillColor(bOMFC);

	RELEASE(pI);
}

float RDX8Font::GetOriginalHeight() const
{
	return fOriginalHeight;
}

bool RDX8Font::ReserveBuffer(dword dwVertices)
{
	if (pIBuffer && !pIBuffer->IsReseted())
	{
		if (!dwVertices) return false;
		if (dwVertices <= dwVBufferVertices) return true;

		if (pVBuffer || pIBuffer)
		{
			return false;
		}
	}
	
	dwVBufferVertices = Max(dwVBufferVertices, dwVertices);

	RELEASE(pVBuffer);
	pVBuffer = pRS->CreateVertexBuffer(sizeof(FONT_VERTEX) * dwVBufferVertices, sizeof(FONT_VERTEX), _FL_, USAGE_WRITEONLY | USAGE_DYNAMIC, POOL_DEFAULT);

	RELEASE(pIBuffer);
	pIBuffer = pRS->CreateIndexBuffer(3 * sizeof(word) * ((dwVBufferVertices*2)+1), _FL_, USAGE_WRITEONLY | USAGE_DYNAMIC, FMT_INDEX16, POOL_DEFAULT);

	if (word * pW = (word*)pIBuffer->Lock()) 
	{
		for (dword i=0; i<dwVBufferVertices>>2; i++)
		{
			*pW++ = word(i*4 + 2); *pW++ = word(i*4 + 1); *pW++ = word(i*4 + 0);
			*pW++ = word(i*4 + 3); *pW++ = word(i*4 + 2); *pW++ = word(i*4 + 0);
		}

		pIBuffer->Unlock();
	}

	return !pVBuffer && !pIBuffer; 
}

float RDX8Font::GetLength(float fHeight, float kerning, const char * pString) const
{
	if (!pString) return 0.0f;


	if (kerning >= -0.001f && kerning <= 0.001f)
	{
		kerning = fDefaultKerning;
	}

	dword dwLen = strlen(pString);
	if (!dwLen) return 0.0f;
	fHeight = (fHeight < 0.0f) ? fDV * fTexSizeY : fHeight;
	float fScale = fHeight / (fDV * fTexSizeY);

	float fMaxWidth = 0.0f, fWidth = 0.0f;
	for (dword i=0; i<dwLen; i++)
	{
		if (pString[i] == '\n')
		{
			fMaxWidth = Max(fMaxWidth, fWidth);
			fWidth = 0.0f;
			continue;
		}
		const symbol_t * pS = &Symbols[byte(pString[i])];

		fWidth += fScale * ((pS->fDU * fTexSizeX) + kerning);
	}

	float fWidthMultipler = NGRender::pRS->GetWideScreenAspectWidthMultipler();

	fWidth *= fWidthMultipler;
	fMaxWidth *= fWidthMultipler;
	
	return Max(fWidth, fMaxWidth);
}

dword RDX8Font::GetNumVertices() const
{
	return dwLastLen * 4;
}

IIBuffer * RDX8Font::GetIndexBuffer() const
{ 
	return pIBuffer; 
}

IVBuffer * RDX8Font::PrintBuffer(float x, float y, float z, const char * pString, float fHeight, float kerning)
{
	if (!pString) return null;

	if (kerning >= -0.001f && kerning <= 0.001f)
	{
		kerning = fDefaultKerning;
	}


	fHeight = (fHeight < 0.0f) ? fDV * fTexSizeY : fHeight;
	float fScale = fHeight / (fDV * fTexSizeY);
	dword dwLen = strlen(pString);

	if (!ReserveBuffer(dwLen * 4)) return null;

	
#ifndef _XBOX
	DWORD dwFlags = LOCK_DISCARD;
#else
	DWORD dwFlags = 0;
#endif


	FONT_VERTEX * pVFB = (FONT_VERTEX*)pVBuffer->Lock(0, 0, dwFlags);
	if (!pVFB) return false;

	float fStartX = float(long(x) + 0.5f);

	float fX = fStartX;
	float fY = float(long(y) + 0.5f);

	float fScrX = float(pRS->GetViewport().Width) / 2.0f;
	float fScrY = float(pRS->GetViewport().Height) / 2.0f;


	float fWidthMultipler = NGRender::pRS->GetWideScreenAspectWidthMultipler();

	//api->Trace("mul : %f, wide : %d, wmul : %f", fWidthMultipler, NGRender::pRS->IsWideScreen(), fWidthMultipler);




	dwLastLen = 0;
	for (dword i=0; i<dwLen; i++)
	{
		if (pString[i] == '\n')
		{
			fX = fStartX;
			fY += fHeight;
			continue;
		}

		symbol_t * pS = &Symbols[byte(pString[i])];

		if (fabsf(pS->fDU) < 1e-10f) { continue; }

		FONT_VERTEX * pV = &pVFB[dwLastLen * 4];

		pV[0].vPos.x = fX / fScrX - 1.0f;
		pV[0].vPos.y = -(fY / fScrY - 1.0f);
		pV[0].vPos.z = z;
		pV[0].tu = pS->fStartU;
		pV[0].tv = pS->fStartV;

		pV[1].vPos.x = fX / fScrX - 1.0f;
		pV[1].vPos.y = -((fY + (fHeight)) / fScrY - 1.0f);
		pV[1].vPos.z = z;
		pV[1].tu = pS->fStartU;
		pV[1].tv = pS->fStartV + fDV;

		pV[2].vPos.x = (fX + (fWidthMultipler * (fScale * pS->fDU * fTexSizeX))) / fScrX - 1.0f;
		pV[2].vPos.y = -((fY + (fHeight)) / fScrY - 1.0f);
		pV[2].vPos.z = z;
		pV[2].tu = pS->fStartU + pS->fDU;
		pV[2].tv = pS->fStartV + fDV;

		pV[3].vPos.x = (fX + (fWidthMultipler * (fScale * pS->fDU * fTexSizeX))) / fScrX - 1.0f;
		pV[3].vPos.y = -(fY / fScrY - 1.0f);
		pV[3].vPos.z = z;
		pV[3].tu = pS->fStartU + pS->fDU;
		pV[3].tv = pS->fStartV;

		fX += ((fScale * ((pS->fDU * fTexSizeX)+kerning)) * fWidthMultipler);

		dwLastLen++;
	}
	pVBuffer->Unlock();

	return pVBuffer;
}

dword RDX8Font::GetNumPrimitives() const
{
	return dwLastLen * 2;
}