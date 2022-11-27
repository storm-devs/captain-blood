#include "BaseTexture.h"
#include "..\Render.h"

CDX8BaseTexture::CDX8BaseTexture() : Resource(DX8TYPE_UNKNOWN, 0)
{
	dwWidth = 0;
	dwHeight = 0;
	dwDepth = 0;
	dwLevels = 0;
}

CDX8BaseTexture::~CDX8BaseTexture()
{
	for (dword n =0 ; n < 16; n++)
	{
		NGRender::pRS->D3D()->SetTexture(n, NULL);
	}
}

dword CDX8BaseTexture::GetLOD() const
{
	return 0;
}

dword CDX8BaseTexture::SetLOD(dword dwLOD)
{
	return 0;
}
