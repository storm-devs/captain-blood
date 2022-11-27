#ifndef FORMAT_CONVERTOR
#define FORMAT_CONVERTOR

#include "GraphicsApi.h"
#include "..\..\common_h\Render.h"


//-------- Таблицы для преобразований --------------------




static D3DRESOURCETYPE tblResourceTypeToDX[] = {D3DRTYPE_SURFACE, D3DRTYPE_VOLUME, D3DRTYPE_TEXTURE, D3DRTYPE_VOLUMETEXTURE, 
D3DRTYPE_CUBETEXTURE, D3DRTYPE_VERTEXBUFFER, D3DRTYPE_INDEXBUFFER};

static RENDERRESOURCETYPE tblResourceTypeFromDX[] = {RTYPE_SURFACE, RTYPE_SURFACE, RTYPE_VOLUME, RTYPE_TEXTURE, RTYPE_VOLUMETEXTURE, 
RTYPE_CUBETEXTURE, RTYPE_VERTEXBUFFER, RTYPE_INDEXBUFFER};



inline RENDERFORMAT FormatFromDX (D3DFORMAT dxFormat)
{
	switch (dxFormat)
	{
	case D3DFMT_R16F:
		return FMT_R16F;
	case D3DFMT_UNKNOWN:
		return FMT_UNKNOWN;
	case D3DFMT_A16B16G16R16:
		return FMT_A16B16G16R16;
	case D3DFMT_A16B16G16R16F:
		return FMT_A16B16G16R16F;
	case D3DFMT_A32B32G32R32F:
		return FMT_A32B32G32R32F;
	case D3DFMT_A2R10G10B10:
		return FMT_A2R10G10B10;
	//case D3DFMT_R8G8B8:
		//return FMT_R8G8B8;
	case D3DFMT_A8R8G8B8:
		return FMT_A8R8G8B8;
	case D3DFMT_X8R8G8B8:
		return FMT_X8R8G8B8;
	
#ifdef _XBOX
	case D3DFMT_G16R16F:
		return FMT_G16R16F;
	case D3DFMT_LE_X8R8G8B8:
		return FMT_LE_X8R8G8B8;
#endif


	case D3DFMT_R5G6B5:
		return FMT_R5G6B5;
	case D3DFMT_X1R5G5B5:
		return FMT_X1R5G5B5;
	case D3DFMT_A1R5G5B5:
		return FMT_A1R5G5B5;
	case D3DFMT_A4R4G4B4:
		return FMT_A4R4G4B4;
	//case D3DFMT_R3G3B2:
		//return FMT_R3G3B2;
	case D3DFMT_A8:
		return FMT_A8;
	//case D3DFMT_A8R3G3B2:
		//return FMT_A8R3G3B2;
	case D3DFMT_X4R4G4B4:
		return FMT_X4R4G4B4;
	//case D3DFMT_A8P8:
		//return FMT_A8P8;
	//case D3DFMT_P8:
		//return FMT_P8;
	case D3DFMT_L8:
		return FMT_L8;
	case D3DFMT_A8L8:
		return FMT_A8L8;
	//case D3DFMT_A4L4:
		//return FMT_A4L4;
	case D3DFMT_V8U8:
		return FMT_V8U8;
	case D3DFMT_L6V5U5:
		return FMT_L6V5U5;
	case D3DFMT_X8L8V8U8:
		return FMT_X8L8V8U8;
	case D3DFMT_Q8W8V8U8:
		return FMT_Q8W8V8U8;
	case D3DFMT_V16U16:
		return FMT_V16U16;
	case D3DFMT_UYVY:
		return FMT_UYVY;
	case D3DFMT_YUY2:
		return FMT_YUY2;
	case D3DFMT_DXT1:
		return FMT_DXT1;
	//case D3DFMT_DXT2:
		//return FMT_DXT2;
	case D3DFMT_DXT3:
		return FMT_DXT3;
	//case D3DFMT_DXT4:
		//return FMT_DXT4;
	case D3DFMT_DXT5:
		return FMT_DXT5;
	//case D3DFMT_D16_LOCKABLE:
		//return FMT_D16_LOCKABLE;
	case D3DFMT_D32:
		return FMT_D32;
	//case D3DFMT_D15S1:
		//return FMT_D15S1;
	case D3DFMT_D24S8:
		return FMT_D24S8;
	case D3DFMT_D16:
		return FMT_D16;
	case D3DFMT_D24X8:
		return FMT_D24X8;
	//case D3DFMT_D24X4S4:
		//return FMT_D24X4S4;
	case D3DFMT_VERTEXDATA:
		return FMT_VERTEXDATA;
	case D3DFMT_INDEX16:
		return FMT_INDEX16;
	case D3DFMT_INDEX32:
		return FMT_INDEX32;
	}


	return FMT_UNKNOWN;
}

inline D3DFORMAT FormatToDX (RENDERFORMAT renderFormat)
{
	switch (renderFormat)
	{
	case FMT_R16F:
		return D3DFMT_R16F;
	case FMT_UNKNOWN:
		return D3DFMT_UNKNOWN;
	case FMT_A16B16G16R16:
		return D3DFMT_A16B16G16R16;
	case FMT_A16B16G16R16F:
		return D3DFMT_A16B16G16R16F;
	case FMT_A32B32G32R32F:
		return D3DFMT_A32B32G32R32F;
	case FMT_A2R10G10B10:
		return D3DFMT_A2R10G10B10;
	//case FMT_R8G8B8:
		//return D3DFMT_R8G8B8;
	case FMT_A8R8G8B8:
		return D3DFMT_A8R8G8B8;
	case FMT_G16R16F:
		return D3DFMT_G16R16F;

#ifdef _XBOX
	case FMT_LE_X8R8G8B8:
		return D3DFMT_LE_X8R8G8B8;
#endif

	case FMT_X8R8G8B8:
		return D3DFMT_X8R8G8B8;
	case FMT_R5G6B5:
		return D3DFMT_R5G6B5;
	case FMT_X1R5G5B5:
		return D3DFMT_X1R5G5B5;
	case FMT_A1R5G5B5:
		return D3DFMT_A1R5G5B5;
	case FMT_A4R4G4B4:
		return D3DFMT_A4R4G4B4;
	//case FMT_R3G3B2:
		//return D3DFMT_R3G3B2;
	case FMT_A8:
		return D3DFMT_A8;
	//case FMT_A8R3G3B2:
		//return D3DFMT_A8R3G3B2;
	case FMT_X4R4G4B4:
		return D3DFMT_X4R4G4B4;
	//case FMT_A8P8:
		//return D3DFMT_A8P8;
	//case FMT_P8:
		//return D3DFMT_P8;
	case FMT_L8:
		return D3DFMT_L8;
	case FMT_A8L8:
		return D3DFMT_A8L8;
	//case FMT_A4L4:
		//return D3DFMT_A4L4;
	case FMT_V8U8:
		return D3DFMT_V8U8;
	case FMT_L6V5U5:
		return D3DFMT_L6V5U5;
	case FMT_X8L8V8U8:
		return D3DFMT_X8L8V8U8;
	case FMT_Q8W8V8U8:
		return D3DFMT_Q8W8V8U8;
	case FMT_V16U16:
		return D3DFMT_V16U16;
	case FMT_UYVY:
		return D3DFMT_UYVY;
	case FMT_YUY2:
		return D3DFMT_YUY2;
	case FMT_DXT1:
		return D3DFMT_DXT1;
	case FMT_DXT2:
		return D3DFMT_DXT2;
	case FMT_DXT3:
		return D3DFMT_DXT3;
	case FMT_DXT4:
		return D3DFMT_DXT4;
	case FMT_DXT5:
		return D3DFMT_DXT5;
	//case FMT_D16_LOCKABLE:
		//return D3DFMT_D16_LOCKABLE;
	case FMT_D32:
		return D3DFMT_D32;
	//case FMT_D15S1:
		//return D3DFMT_D15S1;
#ifdef _XBOX
	case FMT_D24S8:
		return D3DFMT_D24X8;
#else
	case FMT_D24S8:
		return D3DFMT_D24S8;
#endif

	case FMT_D16:
		return D3DFMT_D16;
	case FMT_D24X8:
		return D3DFMT_D24X8;
	//case FMT_D24X4S4:
		//return D3DFMT_D24X4S4;
	case FMT_R32F:
		return D3DFMT_R32F;
	case FMT_VERTEXDATA:
		return D3DFMT_VERTEXDATA;
	case FMT_INDEX16:
		return D3DFMT_INDEX16;
	case FMT_INDEX32:
		return D3DFMT_INDEX32;
	}

	return D3DFMT_UNKNOWN;
}


#ifdef _XBOX

inline D3DFORMAT FormatToLinearDX (RENDERFORMAT renderFormat)
{
	switch (renderFormat)
	{
	case FMT_R16F:
		return D3DFMT_LIN_R16F;
	case FMT_UNKNOWN:
		return D3DFMT_UNKNOWN;
	case FMT_A16B16G16R16:
		return D3DFMT_LIN_A16B16G16R16;
	case FMT_A16B16G16R16F:
		return D3DFMT_LIN_A16B16G16R16F;
	case FMT_A32B32G32R32F:
		return D3DFMT_LIN_A32B32G32R32F;
	case FMT_A2R10G10B10:
		return D3DFMT_LIN_A2R10G10B10;
		//case FMT_R8G8B8:
		//return D3DFMT_R8G8B8;
	case FMT_A8R8G8B8:
		return D3DFMT_LIN_A8R8G8B8;

	case FMT_X8R8G8B8:
		return D3DFMT_LIN_X8R8G8B8;
	case FMT_R5G6B5:
		return D3DFMT_LIN_R5G6B5;
	case FMT_X1R5G5B5:
		return D3DFMT_LIN_X1R5G5B5;
	case FMT_A1R5G5B5:
		return D3DFMT_LIN_A1R5G5B5;
	case FMT_A4R4G4B4:
		return D3DFMT_LIN_A4R4G4B4;
		//case FMT_R3G3B2:
		//return D3DFMT_R3G3B2;
	case FMT_A8:
		return D3DFMT_LIN_A8;
		//case FMT_A8R3G3B2:
		//return D3DFMT_A8R3G3B2;
	case FMT_X4R4G4B4:
		return D3DFMT_LIN_X4R4G4B4;
		//case FMT_A8P8:
		//return D3DFMT_A8P8;
		//case FMT_P8:
		//return D3DFMT_P8;
	case FMT_L8:
		return D3DFMT_LIN_L8;
	case FMT_A8L8:
		return D3DFMT_LIN_A8L8;
		//case FMT_A4L4:
		//return D3DFMT_A4L4;
	case FMT_V8U8:
		return D3DFMT_LIN_V8U8;
	case FMT_L6V5U5:
		return D3DFMT_LIN_L6V5U5;
	case FMT_X8L8V8U8:
		return D3DFMT_LIN_X8L8V8U8;
	case FMT_Q8W8V8U8:
		return D3DFMT_LIN_Q8W8V8U8;
	case FMT_V16U16:
		return D3DFMT_LIN_V16U16;
	case FMT_UYVY:
		return D3DFMT_LIN_UYVY;
	case FMT_YUY2:
		return D3DFMT_LIN_YUY2;
	case FMT_DXT1:
		return D3DFMT_LIN_DXT1;
	case FMT_DXT2:
		return D3DFMT_LIN_DXT2;
	case FMT_DXT3:
		return D3DFMT_LIN_DXT3;
	case FMT_DXT4:
		return D3DFMT_LIN_DXT4;
	case FMT_DXT5:
		return D3DFMT_LIN_DXT5;
		//case FMT_D16_LOCKABLE:
		//return D3DFMT_D16_LOCKABLE;
	case FMT_D32:
		return D3DFMT_LIN_D32;
		//case FMT_D15S1:
		//return D3DFMT_D15S1;
	case FMT_D24S8:
		return D3DFMT_LIN_D24S8;
	case FMT_D16:
		return D3DFMT_LIN_D16;
	case FMT_D24X8:
		return D3DFMT_LIN_D24X8;
		//case FMT_D24X4S4:
		//return D3DFMT_D24X4S4;
	case FMT_G16R16F:
		return D3DFMT_LIN_G16R16F;
	case FMT_R32F:
		return D3DFMT_LIN_R32F;
	case FMT_VERTEXDATA:
		return D3DFMT_VERTEXDATA;
	case FMT_INDEX16:
		return D3DFMT_INDEX16;
	case FMT_INDEX32:
		return D3DFMT_INDEX32;
	}

	return D3DFMT_UNKNOWN;
}
#endif


inline DWORD ClearFlagsToDX (DWORD clearFlags)
{
	dword dwRetVal = 0;

	if ((clearFlags & CLEAR_TARGET) > 0) dwRetVal = dwRetVal | D3DCLEAR_TARGET;
	if ((clearFlags & CLEAR_ZBUFFER) > 0) dwRetVal = dwRetVal | D3DCLEAR_ZBUFFER;
	if ((clearFlags & CLEAR_STENCIL) > 0) dwRetVal = dwRetVal | D3DCLEAR_STENCIL;

	return dwRetVal;
}

inline DWORD ClearFlagsFromDX (DWORD clearFlags)
{
	dword dwRetVal = 0;

	if ((clearFlags & D3DCLEAR_TARGET) > 0) dwRetVal = dwRetVal | CLEAR_TARGET;
	if ((clearFlags & D3DCLEAR_ZBUFFER) > 0) dwRetVal = dwRetVal | CLEAR_ZBUFFER;
	if ((clearFlags & D3DCLEAR_STENCIL) > 0) dwRetVal = dwRetVal | CLEAR_STENCIL;

	return dwRetVal;
}


inline DWORD UsageToDX (DWORD renderUsage)
{
	dword dwRetVal = 0;

	if ((renderUsage & USAGE_RENDERTARGET) > 0) dwRetVal = dwRetVal | D3DUSAGE_RENDERTARGET;
	if ((renderUsage & USAGE_DEPTHSTENCIL) > 0) dwRetVal = dwRetVal | D3DUSAGE_DEPTHSTENCIL;

#ifndef _XBOX
	if ((renderUsage & USAGE_DYNAMIC) > 0) dwRetVal = dwRetVal | D3DUSAGE_DYNAMIC;
#endif

	if ((renderUsage & USAGE_POINTS) > 0) dwRetVal = dwRetVal | D3DUSAGE_POINTS;
	if ((renderUsage & USAGE_WRITEONLY) > 0) dwRetVal = dwRetVal | D3DUSAGE_WRITEONLY;

	return dwRetVal;
}

inline DWORD UsageFromDX (DWORD dxUsage)
{
	dword dwRetVal = 0;

	if ((dxUsage & D3DUSAGE_RENDERTARGET) > 0) dwRetVal = dwRetVal | USAGE_RENDERTARGET;
	if ((dxUsage & D3DUSAGE_DEPTHSTENCIL) > 0) dwRetVal = dwRetVal | USAGE_DEPTHSTENCIL;
#ifndef _XBOX
	if ((dxUsage & D3DUSAGE_DYNAMIC) > 0) dwRetVal = dwRetVal | USAGE_DYNAMIC;
#endif
	if ((dxUsage & D3DUSAGE_POINTS) > 0) dwRetVal = dwRetVal | USAGE_POINTS;
	if ((dxUsage & D3DUSAGE_WRITEONLY) > 0) dwRetVal = dwRetVal | USAGE_WRITEONLY;

	return dwRetVal;
}




inline DWORD LockFlagsToDX (DWORD lockFlags)
{
	dword dwRetVal = 0;

#ifndef _XBOX
	if ((lockFlags & LOCK_DISCARD) > 0) dwRetVal = dwRetVal | D3DLOCK_DISCARD;
#endif
	if ((lockFlags & LOCK_NOSYSLOCK) > 0) dwRetVal = dwRetVal | D3DLOCK_NOSYSLOCK;

	return dwRetVal;
}

inline DWORD LockFlagsFromDX (DWORD lockFlags)
{
	dword dwRetVal = 0;

#ifndef _XBOX
	if ((lockFlags & D3DLOCK_DISCARD) > 0) dwRetVal = dwRetVal | LOCK_DISCARD;
#endif
	if ((lockFlags & D3DLOCK_NOSYSLOCK) > 0) dwRetVal = dwRetVal | LOCK_NOSYSLOCK;

	return dwRetVal;
}






inline D3DPOOL PoolToDX (RENDERPOOL renderPool)
{
	switch (renderPool)
	{
	case POOL_DEFAULT:
		return D3DPOOL_DEFAULT;
	case POOL_MANAGED:
		return D3DPOOL_MANAGED;
	case POOL_SYSTEMMEM:
		return D3DPOOL_SYSTEMMEM;
	}

	return D3DPOOL_MANAGED;
}

inline RENDERPOOL PoolFromDX (D3DPOOL dxPool)
{
	switch (dxPool)
	{
	case D3DPOOL_DEFAULT:
		return POOL_DEFAULT;
	case D3DPOOL_MANAGED:
		return POOL_MANAGED;
	case D3DPOOL_SYSTEMMEM:
		return POOL_SYSTEMMEM;
	}

	return POOL_MANAGED;
}


inline D3DMULTISAMPLE_TYPE MSTypeToDX(RENDERMULTISAMPLE_TYPE renderMST)
{
	switch(renderMST)
	{
	case MULTISAMPLE_NONE:
		return D3DMULTISAMPLE_NONE;
	case MULTISAMPLE_2_SAMPLES:
		return D3DMULTISAMPLE_2_SAMPLES;
	case MULTISAMPLE_4_SAMPLES:
		return D3DMULTISAMPLE_4_SAMPLES;
#ifndef _XBOX
	case MULTISAMPLE_8_SAMPLES:
		return D3DMULTISAMPLE_8_SAMPLES;
#endif
	}
	
	return D3DMULTISAMPLE_NONE;
}

inline RENDERMULTISAMPLE_TYPE MSTypeFromDX(D3DMULTISAMPLE_TYPE dxMST)
{
	switch(dxMST)
	{
	case D3DMULTISAMPLE_NONE:
		return MULTISAMPLE_NONE;
	case D3DMULTISAMPLE_2_SAMPLES:
		return MULTISAMPLE_2_SAMPLES;
	case D3DMULTISAMPLE_4_SAMPLES:
		return MULTISAMPLE_4_SAMPLES;
#ifndef _XBOX
	case D3DMULTISAMPLE_8_SAMPLES:
		return MULTISAMPLE_8_SAMPLES;
#endif
	}

	return MULTISAMPLE_NONE;
}



inline D3DRESOURCETYPE ResourceTypeToDX(RENDERRESOURCETYPE renderRT)
{
	switch (renderRT)
	{
	case RTYPE_SURFACE:
		return D3DRTYPE_SURFACE;
	case RTYPE_VOLUME:
		return D3DRTYPE_VOLUME;
	case RTYPE_TEXTURE:
		return D3DRTYPE_TEXTURE;
	case RTYPE_VOLUMETEXTURE:
		return D3DRTYPE_VOLUMETEXTURE;
	case RTYPE_CUBETEXTURE:
		return D3DRTYPE_CUBETEXTURE;
	case RTYPE_VERTEXBUFFER:
		return D3DRTYPE_VERTEXBUFFER;
	case RTYPE_INDEXBUFFER:
		return D3DRTYPE_INDEXBUFFER;
	}

	return D3DRTYPE_SURFACE;
}

inline RENDERRESOURCETYPE ResourceTypeFromDX(D3DRESOURCETYPE dxRT)
{
	switch (dxRT)
	{
	case D3DRTYPE_SURFACE:
		return RTYPE_SURFACE;
	case D3DRTYPE_VOLUME:
		return RTYPE_VOLUME;
	case D3DRTYPE_TEXTURE:
		return RTYPE_TEXTURE;
	case D3DRTYPE_VOLUMETEXTURE:
		return RTYPE_VOLUMETEXTURE;
	case D3DRTYPE_CUBETEXTURE:
		return RTYPE_CUBETEXTURE;
	case D3DRTYPE_VERTEXBUFFER:
		return RTYPE_VERTEXBUFFER;
	case D3DRTYPE_INDEXBUFFER:
		return RTYPE_INDEXBUFFER;
	}

	return RTYPE_SURFACE;
}


inline D3DCUBEMAP_FACES FaceToDX (RENDERCUBEMAP_FACES renderFace)
{
	switch (renderFace)
	{
	case CUBEMAP_FACE_POSITIVE_X:
		return D3DCUBEMAP_FACE_POSITIVE_X;
	case CUBEMAP_FACE_NEGATIVE_X:
		return D3DCUBEMAP_FACE_NEGATIVE_X;
	case CUBEMAP_FACE_POSITIVE_Y:
		return D3DCUBEMAP_FACE_POSITIVE_Y;
	case CUBEMAP_FACE_NEGATIVE_Y:
		return D3DCUBEMAP_FACE_NEGATIVE_Y;
	case CUBEMAP_FACE_POSITIVE_Z:
		return D3DCUBEMAP_FACE_POSITIVE_Z;
	case CUBEMAP_FACE_NEGATIVE_Z:
		return D3DCUBEMAP_FACE_NEGATIVE_Z;
	}

	return D3DCUBEMAP_FACE_POSITIVE_X;
}

inline RENDERCUBEMAP_FACES FaceFromDX (D3DCUBEMAP_FACES dxFace)
{
	switch (dxFace)
	{
	case D3DCUBEMAP_FACE_POSITIVE_X:
		return CUBEMAP_FACE_POSITIVE_X;
	case D3DCUBEMAP_FACE_NEGATIVE_X:
		return CUBEMAP_FACE_NEGATIVE_X;
	case D3DCUBEMAP_FACE_POSITIVE_Y:
		return CUBEMAP_FACE_POSITIVE_Y;
	case D3DCUBEMAP_FACE_NEGATIVE_Y:
		return CUBEMAP_FACE_NEGATIVE_Y;
	case D3DCUBEMAP_FACE_POSITIVE_Z:
		return CUBEMAP_FACE_POSITIVE_Z;
	case D3DCUBEMAP_FACE_NEGATIVE_Z:
		return CUBEMAP_FACE_NEGATIVE_Z;
	}

	return CUBEMAP_FACE_POSITIVE_X;
}


static D3DPRIMITIVETYPE tblPtToDX[] = {D3DPT_POINTLIST, D3DPT_LINELIST, 
D3DPT_LINESTRIP, D3DPT_TRIANGLELIST,
D3DPT_TRIANGLESTRIP, D3DPT_TRIANGLEFAN};

static RENDERPRIMITIVETYPE tblPtFromDX[] = {PT_POINTLIST, PT_POINTLIST, PT_LINELIST, 
PT_LINESTRIP, PT_TRIANGLELIST, PT_TRIANGLESTRIP, 
PT_TRIANGLEFAN};


//------------------------------------

inline D3DPRIMITIVETYPE PtToDX (RENDERPRIMITIVETYPE renderPT)
{
	switch (renderPT)
	{
	case PT_POINTLIST:
		return D3DPT_POINTLIST;
	case PT_LINELIST:
		return D3DPT_LINELIST;
	case PT_LINESTRIP:
		return D3DPT_LINESTRIP;
	case PT_TRIANGLELIST:
		return D3DPT_TRIANGLELIST;
	case PT_TRIANGLESTRIP:
		return D3DPT_TRIANGLESTRIP;
	case PT_TRIANGLEFAN:
		return D3DPT_TRIANGLEFAN;

#ifdef _XBOX
	case PT_QUADLIST:
		return D3DPT_QUADLIST;
#endif

	}

	return D3DPT_POINTLIST;
}

inline RENDERPRIMITIVETYPE PtToDX (D3DPRIMITIVETYPE dxPT)
{
	switch (dxPT)
	{
	case D3DPT_POINTLIST:
		return PT_POINTLIST;
	case D3DPT_LINELIST:
		return PT_LINELIST;
	case D3DPT_LINESTRIP:
		return PT_LINESTRIP;
	case D3DPT_TRIANGLELIST:
		return PT_TRIANGLELIST;
	case D3DPT_TRIANGLESTRIP:
		return PT_TRIANGLESTRIP;
	case D3DPT_TRIANGLEFAN:
		return PT_TRIANGLEFAN;
#ifdef _XBOX
	case D3DPT_QUADLIST:
		return PT_QUADLIST;
#endif

	}

	return PT_POINTLIST;
}


inline D3DSURFACE_DESC SurfDescToDX (const RENDERSURFACE_DESC& renderDesc)
{
	D3DSURFACE_DESC d;
	d.Format = FormatToDX(renderDesc.Format);
	d.Height = renderDesc.Height;
	d.MultiSampleQuality = renderDesc.MultiSampleQuality;
	d.MultiSampleType = MSTypeToDX (renderDesc.MultiSampleType);
	d.Pool = PoolToDX(renderDesc.Pool);
	d.Type = ResourceTypeToDX (renderDesc.Type);
	d.Usage = UsageToDX(renderDesc.Usage);
	d.Width = renderDesc.Width;

	return d;
}

inline RENDERSURFACE_DESC SurfDescFromDX (const D3DSURFACE_DESC& dxDesc)
{

	RENDERSURFACE_DESC d;
	d.Format = FormatFromDX(dxDesc.Format);
	d.Height = dxDesc.Height;
	d.MultiSampleQuality = dxDesc.MultiSampleQuality;
	d.MultiSampleType = MSTypeFromDX (dxDesc.MultiSampleType);
	d.Pool = PoolFromDX(dxDesc.Pool);
	d.Type = ResourceTypeFromDX (dxDesc.Type);
	d.Usage = UsageFromDX(dxDesc.Usage);
	d.Width = dxDesc.Width;
	return d;
}


inline D3DVERTEXBUFFER_DESC VertexDescToDX (const RENDERVERTEXBUFFER_DESC& renderDesc)
{
	D3DVERTEXBUFFER_DESC d;
	d.Format = FormatToDX(renderDesc.Format);
	d.FVF = renderDesc.FVF;
	d.Pool = PoolToDX (renderDesc.Pool);
	d.Size = renderDesc.Size;
	d.Type = ResourceTypeToDX(renderDesc.Type);
	d.Usage = UsageToDX (renderDesc.Usage);
	return d;
}

inline RENDERVERTEXBUFFER_DESC VertexDescFromDX (const D3DVERTEXBUFFER_DESC& dxDesc)
{
	RENDERVERTEXBUFFER_DESC d;
	d.Format = FormatFromDX(dxDesc.Format);
	d.FVF = dxDesc.FVF;
	d.Pool = PoolFromDX (dxDesc.Pool);
	d.Size = dxDesc.Size;
	d.Type = ResourceTypeFromDX(dxDesc.Type);
	d.Usage = UsageFromDX (dxDesc.Usage);
	return d;
}

inline D3DINDEXBUFFER_DESC IndexDescToDX (const RENDERINDEXBUFFER_DESC& renderDesc)
{
	D3DINDEXBUFFER_DESC d;
	d.Format = FormatToDX(renderDesc.Format);
	d.Pool = PoolToDX(renderDesc.Pool);
	d.Size = renderDesc.Size;
	d.Type = ResourceTypeToDX(renderDesc.Type);
	d.Usage = UsageToDX(renderDesc.Usage);
	return d;
}


inline RENDERINDEXBUFFER_DESC IndexDescFromDX (const D3DINDEXBUFFER_DESC& dxDesc)
{
	RENDERINDEXBUFFER_DESC d;
	d.Format = FormatFromDX(dxDesc.Format);
	d.Pool = PoolFromDX(dxDesc.Pool);
	d.Size = dxDesc.Size;
	d.Type = ResourceTypeFromDX(dxDesc.Type);
	d.Usage = UsageFromDX(dxDesc.Usage);
	return d;
}

inline RENDERRECT RectFromDX (const D3DRECT &dxRect)
{
	RENDERRECT d;
	d.x1 = dxRect.x1;
	d.x2 = dxRect.x2;
	d.y1 = dxRect.y1;
	d.y2 = dxRect.y2;
	return d;
}



inline D3DRECT RectToDX (const RENDERRECT &renderRect)
{
	D3DRECT d;
	d.x1 = renderRect.x1;
	d.x2 = renderRect.x2;
	d.y1 = renderRect.y1;
	d.y2 = renderRect.y2;
	return d;
}



inline D3DVIEWPORT9 ViewPortToDx(const RENDERVIEWPORT& vp)
{
	D3DVIEWPORT9 ret;
	ret.X = vp.X;
	ret.Y = vp.Y;
	ret.Width = vp.Width;
	ret.Height = vp.Height;
	ret.MinZ = vp.MinZ;
	ret.MaxZ = vp.MaxZ;

	return ret;
}







#endif
