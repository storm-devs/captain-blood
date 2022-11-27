#include "Font.h"
#include "..\Render.h"

char cTmpBuffer[8192];
char cTmpBuffer2[8192];

CDX8Font::CDX8Font(RDX8Font * _pRFont, float _fHeight, dword _dwColor) : Resource(DX8TYPE_FONT, 0)
{
	kerning_value = 0.0f;
	fCurrentZ = 0.0f;
	pRFont = _pRFont;

	fHeight = _fHeight;

	dwColor = _dwColor;

	pRS->GetShaderId("dbgFontAlpha", shaderID);
	

	pRFont->AddRef();


	pFontTexture = pRS->GetTechniqueGlobalVariable("FontTexture", _FL_);
	pFontColor = pRS->GetTechniqueGlobalVariable("FontColor", _FL_);
}

CDX8Font::~CDX8Font()
{
	pFontColor = NULL;
	pFontTexture = NULL;

	pRFont->Release();
}

bool CDX8Font::Release() 
{
	if (Resource.Release()) return ForceRelease();
	return false;
}

bool CDX8Font::ForceRelease() 
{ 
	Resource.ForceRelease(); 
	NGRender::pRS->getFontsManager()->Release(this);
	delete this;
	return true;
}

void CDX8Font::SetColor(dword dwColor)
{
	this->dwColor = dwColor;
}

void CDX8Font::SetKerning (float kerningValue)
{
	kerning_value = kerningValue;
}

float CDX8Font::GetHeight() const
{
	float res =  (fHeight < 0.0f) ? pRFont->GetOriginalHeight() : fHeight;

/*
	float fHeightMultipler = 1.0f;
	if (NGRender::pRS->IsWideScreen())
	{
		fHeightMultipler = 1.3333333f;
	}
*/

	return res /** fHeightMultipler*/;
}

void CDX8Font::SetHeight(float fHeight)
{
	this->fHeight = fHeight;
}

void CDX8Font::SetTechnique(const char * pTechniqueName)
{
	pRS->GetShaderId(pTechniqueName, shaderID);
}

float _cdecl CDX8Font::GetHeight(const char * pFormatString, ...) const
{
	if (!pFormatString) return 0.0f;

	va_list args;
	va_start(args, pFormatString);
	crt_vsnprintf(cTmpBuffer, sizeof(cTmpBuffer), pFormatString, args);
	va_end(args);

	dword dwRowNum = 1;
	dword dwLen = strlen(cTmpBuffer);
	for (dword i=0; i<dwLen; i++)
		if (cTmpBuffer[i] == '\n') dwRowNum++;




	return float(dwRowNum) * GetHeight();
}

float _cdecl CDX8Font::GetLength(const char * pFormatString, ...) const
{


	if (!pFormatString) return 0.0f;

	va_list args;
	va_start(args, pFormatString);
	crt_vsnprintf(cTmpBuffer, sizeof(cTmpBuffer), pFormatString, args);
	va_end(args);

	return pRFont->GetLength(fHeight, kerning_value, cTmpBuffer);
}

float _cdecl CDX8Font::GetLength(dword dwLength, const char * pFormatString, ...) const
{
	if (!pFormatString) return 0.0f;

	va_list args;
	va_start(args, pFormatString);
	crt_vsnprintf(cTmpBuffer, sizeof(cTmpBuffer), pFormatString, args);
	va_end(args);
	if (strlen(cTmpBuffer) > dwLength) cTmpBuffer[dwLength] = 0;

	return pRFont->GetLength(fHeight, kerning_value, cTmpBuffer);
}

void _cdecl CDX8Font::Print(float x, float y, const char * pFormatString, ...)
{
	if (!pFormatString) return;

	cTmpBuffer2[0] = 0;
	va_list args;
	va_start(args, pFormatString);
	crt_vsnprintf(cTmpBuffer, sizeof(cTmpBuffer), pFormatString, args);
	va_end(args);

	if (cTmpBuffer[0] == 0)
	{
		return;
	}

	IVBuffer * pVBuffer = pRFont->PrintBuffer(x, y, fCurrentZ, cTmpBuffer, fHeight, kerning_value);
	IIBuffer* pIBuffer = pRFont->GetIndexBuffer();

	if (pVBuffer == NULL || 
		  pRFont->GetNumPrimitives() <= 0 ||
			pIBuffer == NULL)
	{
		return;
	}

	//pRS->SetTexture(0, pRFont->GetTexture());
	//pParameters->SetDword(0, dwColor);

	if (pFontTexture)
	{
		pFontTexture->SetTexture(pRFont->GetTexture());
	}

	if (pFontColor)
	{
		pFontColor->SetVector4(Color(dwColor).v4);
	}

	pRS->SetStreamSource(0, pVBuffer);
	pRS->SetIndices(pIBuffer);

	pRS->DrawIndexedPrimitive(shaderID, PT_TRIANGLELIST, 0, pRFont->GetNumVertices(), 0, pRFont->GetNumPrimitives());
	fCurrentZ = 0.0f;

	pRS->SetStreamSource(0, NULL);
}

IBaseTexture*	CDX8Font::GetTexture()
{
	return pRFont->GetTexture();
}

void _cdecl	CDX8Font::Print(const Vector & vPos, float fViewDistance, float fLine, const char * pFormatString, ...)
{
	if (!pFormatString) return;

	float fDistance = sqrtf(~(vPos - pRS->GetView().GetCamPos()));
	if (fViewDistance > 0.0f && fDistance > fViewDistance) return;
	if (fViewDistance <= 0.0f)
	{
		fViewDistance = 1.0f;
		fDistance = 0.0f;
	}

	va_list args;
	va_start(args, pFormatString);
	crt_vsnprintf(cTmpBuffer, sizeof(cTmpBuffer), pFormatString, args);
	va_end(args);

	if (!cTmpBuffer[0]) return;

	Matrix mVP(pRS->GetView(), pRS->GetProjection());
	Vector4 v = mVP.Projection(vPos, pRS->GetViewport().Width * 0.5f, pRS->GetViewport().Height * 0.5f);
	if (v.w <= 0.0f) return;

	dword dwTmpColor = dwColor;
	dwColor = Color(dwColor).MulAlpha(Clamp((1.0f - fDistance / fViewDistance) * 3.0f));

	float fXViewportScale = (float)pRS->GetScreenInfo2D().dwWidth / (float)pRS->GetViewport().Width;
	float fYViewportScale = (float)pRS->GetScreenInfo2D().dwHeight / (float)pRS->GetViewport().Height;

	float fWidth = fXViewportScale * GetLength(cTmpBuffer);
	fCurrentZ = v.z;
	Print(v.x - fWidth * 0.5f, v.y - fYViewportScale * GetHeight() * (0.5f - fLine), cTmpBuffer);

	dwColor = dwTmpColor;
}
