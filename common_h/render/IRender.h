#ifndef DX8RENDER_HPP
#define DX8RENDER_HPP




#include "IResource.h"
#include "IFilter.h"
#include "IFont.h"
#include "IAdvancedFont.h"

#include "..\gmxservice\gmx_OBB.h"



/*
JOKER: не во всех DLL есть макрос PROFILE из за этого интерфейсы расходяться
 пока убрал

//Если нужна поддержка pixBeginEvent, pixEndEvent
#if defined (_DEBUG) || defined(PROFILE)
#define HAVE_PIX_BEGINEND_EVENT_SUPPORT (1)
#endif
*/

//Если хотим использовать рендер через патчинг комманд буферов...
#define _PRECOMPILED_COMMAND_BUFFER_BAKER (1)

#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)
#pragma message("Warning: !!! Activated comand buffer render PATH !!!!")
#endif



#define COMPILE_PROPS_INCLUDED (1)


#include "ShaderID.h"

#define USAGE_RENDERTARGET  0x01
#define USAGE_DEPTHSTENCIL  0x02
#define USAGE_DYNAMIC       0x04
#define USAGE_POINTS        0x08
#define USAGE_WRITEONLY     0x10


#define CLEAR_TARGET            0x00000001  /* Clear target surface */
#define CLEAR_ZBUFFER           0x00000002  /* Clear target z buffer */
#define CLEAR_STENCIL           0x00000004  /* Clear stencil planes */

#define LOCK_DISCARD						0x01
#define LOCK_NOSYSLOCK          0x02


#define RENDERCOLOR_ARGB(a,r,g,b)   ((RENDERCOLOR)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))


//-------------------------------------------------------------------------
enum TempRenderTargetSize
{
	//Размер экрана
	TRS_SCREEN_FULL_3D = 0,

	//Половина экрана
	TRS_SCREEN_HALF_3D = 1,

	//Четверть экрана
	TRS_SCREEN_QUARTER_3D = 2,

	//64x64
	TRS_64 = 3,

	//128x128
	TRS_128 = 4,

	//512x512
	TRS_512 = 5,

	//1024x1024
	TRS_1024 = 6,

	//Размер 2D экрана
	TRS_SCREEN_FULL_2D = 7,

	//Половина 2D экрана
	TRS_SCREEN_HALF_2D = 8,

	//Четверть 2D экрана
	TRS_SCREEN_QUARTER_2D = 9,


	TRS_FORCE_DWORD = 0x7fffffff
};

//-------------------------------------------------------------------------
enum TempRenderTargetColor
{
	//R8G8B8A8
	TRC_FIXED_RGBA_8 = 0,

	//R16G16B16A16
	TRC_FIXED_RGBA_16 = 1,

	//R32 или R16
	TRC_FLOAT_R = 2,

	TRC_FORCE_DWORD = 0x7fffffff
};






//-------------------------------------------------------------------------
typedef DWORD RENDERCOLOR;


//-------------------------------------------------------------------------
typedef struct _RENDERRECT {
	LONG x1;
	LONG y1;
	LONG x2;
	LONG y2;
} RENDERRECT;



//-------------------------------------------------------------------------
typedef enum _RENDERRESOURCETYPE {
	RTYPE_SURFACE                =  0,
	RTYPE_VOLUME,
	RTYPE_TEXTURE,
	RTYPE_VOLUMETEXTURE,
	RTYPE_CUBETEXTURE,
	RTYPE_VERTEXBUFFER,
	RTYPE_INDEXBUFFER,

	RTYPE_FORCE_DWORD            = 0x7fffffff
} RENDERRESOURCETYPE;

//-------------------------------------------------------------------------
typedef enum _RENDERPOOL {
	POOL_DEFAULT                 = 0,
	POOL_MANAGED,
	POOL_SYSTEMMEM,
	POOL_SCRATCH,

	POOL_FORCE_DWORD             = 0x7fffffff
} RENDERPOOL;

//-------------------------------------------------------------------------
typedef enum _RENDERPRIMITIVETYPE {
	PT_POINTLIST             = 0,
	PT_LINELIST,
	PT_LINESTRIP,
	PT_TRIANGLELIST,
	PT_TRIANGLESTRIP,
	PT_TRIANGLEFAN,
#ifdef _XBOX
	PT_QUADLIST,
#endif
	PT_FORCE_DWORD           = 0x7fffffff, /* force 32-bit size enum */
} RENDERPRIMITIVETYPE;


//-------------------------------------------------------------------------
enum StretchRectFilter
{
	SRF_NONE,
	SRF_POINT,
	SRF_LINEAR,
	SRF_ANISOTROPIC,
	SRF_PYRAMIDALQUAD,
	SRF_GAUSSIANQUAD,

	SRF_FORCE_DWORD           = 0x7fffffff
};


//-------------------------------------------------------------------------
typedef enum _RENDERFORMAT
{
	FMT_UNKNOWN              =  0,

	FMT_R8G8B8,
	FMT_A8R8G8B8,
	FMT_X8R8G8B8,
	FMT_LE_X8R8G8B8,
	FMT_R5G6B5,
	FMT_X1R5G5B5,
	FMT_A1R5G5B5,
	FMT_A4R4G4B4,
	FMT_R3G3B2,
	FMT_A8,
	FMT_A8R3G3B2,
	FMT_X4R4G4B4,
	FMT_A2B10G10R10,
	FMT_A8B8G8R8,
	FMT_X8B8G8R8,
	FMT_G16R16,
	FMT_A2R10G10B10,
	FMT_A16B16G16R16,

	FMT_A8P8,
	FMT_P8,

	FMT_L8,
	FMT_A8L8,
	FMT_A4L4,

	FMT_V8U8,
	FMT_L6V5U5,
	FMT_X8L8V8U8,
	FMT_Q8W8V8U8,
	FMT_V16U16,
	FMT_A2W10V10U10,

	FMT_UYVY,
	FMT_R8G8_B8G8,
	FMT_YUY2,
	FMT_G8R8_G8B8,
	FMT_DXT1,
	FMT_DXT2,
	FMT_DXT3,
	FMT_DXT4,
	FMT_DXT5,

	FMT_D16_LOCKABLE,
	FMT_D32,
	FMT_D15S1,
	FMT_D24S8,
	FMT_D24X8,
	FMT_D24X4S4,
	FMT_D16,

	FMT_D32F_LOCKABLE,
	FMT_D24FS8,


	FMT_L16,

	FMT_VERTEXDATA,
	FMT_INDEX16,
	FMT_INDEX32,

	FMT_Q16W16V16U16,

	FMT_MULTI2_ARGB8,

	// Floating point surface formats

	// s10e5 formats (16-bits per channel)
	FMT_R16F,
	FMT_G16R16F,
	FMT_A16B16G16R16F,

	// IEEE s23e8 formats (32-bits per channel)
	FMT_R32F,
	FMT_G32R32F,
	FMT_A32B32G32R32F,

	FMT_CxV8U8,

	FMT_FORCE_DWORD          =0x7fffffff
} RENDERFORMAT;


/* Vertex Buffer Description */
//-------------------------------------------------------------------------
typedef struct _RENDERVERTEXBUFFER_DESC
{
	RENDERFORMAT           Format;
	RENDERRESOURCETYPE     Type;
	DWORD               Usage;
	RENDERPOOL             Pool;
	UINT                Size;

	DWORD               FVF;

} RENDERVERTEXBUFFER_DESC;


//-------------------------------------------------------------------------
typedef enum _RENDERCUBEMAP_FACES
{
	CUBEMAP_FACE_POSITIVE_X     = 0,
	CUBEMAP_FACE_NEGATIVE_X,
	CUBEMAP_FACE_POSITIVE_Y,
	CUBEMAP_FACE_NEGATIVE_Y,
	CUBEMAP_FACE_POSITIVE_Z,
	CUBEMAP_FACE_NEGATIVE_Z,

	CUBEMAP_FACE_FORCE_DWORD    = 0x7fffffff
} RENDERCUBEMAP_FACES;


/* Index Buffer Description */
//-------------------------------------------------------------------------
typedef struct _RENDERINDEXBUFFER_DESC
{
	RENDERFORMAT           Format;
	RENDERRESOURCETYPE     Type;
	DWORD               Usage;
	RENDERPOOL             Pool;
	UINT                Size;
} RENDERINDEXBUFFER_DESC;


//-------------------------------------------------------------------------
typedef struct _RENDERVOLUME_DESC
{
	RENDERFORMAT           Format;
	RENDERRESOURCETYPE     Type;
	DWORD               Usage;
	RENDERPOOL             Pool;

	UINT                Width;
	UINT                Height;
	UINT                Depth;
} RENDERVOLUME_DESC;


/* Multi-Sample buffer types */
//-------------------------------------------------------------------------
typedef enum _RENDERMULTISAMPLE_TYPE
{
	MULTISAMPLE_NONE            =  0,
	MULTISAMPLE_NONMASKABLE,
	MULTISAMPLE_2_SAMPLES,
	MULTISAMPLE_3_SAMPLES,
	MULTISAMPLE_4_SAMPLES,
	MULTISAMPLE_5_SAMPLES,
	MULTISAMPLE_6_SAMPLES,
	MULTISAMPLE_7_SAMPLES,
	MULTISAMPLE_8_SAMPLES,
	MULTISAMPLE_9_SAMPLES,
	MULTISAMPLE_10_SAMPLES,
	MULTISAMPLE_11_SAMPLES,
	MULTISAMPLE_12_SAMPLES,
	MULTISAMPLE_13_SAMPLES,
	MULTISAMPLE_14_SAMPLES,
	MULTISAMPLE_15_SAMPLES,
	MULTISAMPLE_16_SAMPLES,

	MULTISAMPLE_FORCE_DWORD     = 0x7fffffff
} RENDERMULTISAMPLE_TYPE;


/* Surface Description */
//-------------------------------------------------------------------------
typedef struct _RENDERSURFACE_DESC
{
	RENDERFORMAT           Format;
	RENDERRESOURCETYPE     Type;
	DWORD               Usage;
	RENDERPOOL             Pool;

	RENDERMULTISAMPLE_TYPE MultiSampleType;
	DWORD               MultiSampleQuality;
	UINT                Width;
	UINT                Height;

	//from dx8
	DWORD Size;
} RENDERSURFACE_DESC;


/* Structure for LockRect */
//-------------------------------------------------------------------------
typedef struct _RENDERLOCKED_RECT
{
	INT                 Pitch;
	void*               pBits;
} RENDERLOCKED_RECT;

//-------------------------------------------------------------------------
typedef struct _RENDERBOX
{
	UINT                Left;
	UINT                Top;
	UINT                Right;
	UINT                Bottom;
	UINT                Front;
	UINT                Back;
} RENDERBOX;


//-------------------------------------------------------------------------
typedef struct _RENDERLOCKED_BOX
{
	INT                 RowPitch;
	INT                 SlicePitch;
	void*               pBits;
} RENDERLOCKED_BOX;


//-------------------------------------------------------------------------
struct RS_LINE
{
	RS_LINE() {};
	RS_LINE(const Vector & vPos, dword dwColor)
	{
		this->vPos = vPos;
		this->dwColor = dwColor;
	}

	Vector		vPos;
	dword		dwColor;
};

//-------------------------------------------------------------------------
struct RS_SPRITE
{
	Vector	vPos;
	dword	dwColor;
	float	tu, tv;
};

//-------------------------------------------------------------------------
struct RS_RECT
{
	RS_RECT() {};
	RS_RECT(const Vector & vPos, float fSizeX, float fSizeY, float fAngle, dword dwColor, dword dwSubTexture = 0)
	{
		this->vPos = vPos;
		this->fSizeX = fSizeX;
		this->fSizeY = fSizeY;
		this->fAngle = fAngle;
		this->dwColor = dwColor;
		this->dwSubTexture = dwSubTexture;
	}

	Vector		vPos;
	float		fSizeX, fSizeY;
	float		fAngle;
	dword		dwColor;
	dword		dwSubTexture;
};

//-------------------------------------------------------------------------
struct RENDERSCREEN
{
	dword		dwWidth, dwHeight;
	float		fFOV;
	bool		bWindowed;
	RENDERFORMAT	BackBufferFormat, StencilFormat;
};

//-------------------------------------------------------------------------
class ILight;



//-------------------------------------------------------------------------
class IVBuffer : public IResource
{
protected:
	IVBuffer() {};
	virtual ~IVBuffer() {};

public:

	virtual bool	Create(dword Length, dword Usage, RENDERPOOL Pool, dword Stride) = 0; 

	virtual bool	GetDesc(RENDERVERTEXBUFFER_DESC * pDesc) = 0;
	virtual void *	Lock(dword dwOffsetToLock = 0, dword dwSizeToLock = 0, dword Flags = 0) = 0;
	virtual bool	Unlock() = 0;
	virtual bool	IsLocked() = 0;

	virtual bool	IsReseted() = 0;

	virtual bool	Copy(const void * pSrc, dword dwSize, dword dwOffset = 0, dword Flags = 0) = 0;

	virtual dword	GetLength() const = 0;
	virtual RENDERPOOL	GetPool() const = 0;
	virtual dword	GetStride() const = 0;
	virtual dword	GetUsage() const = 0;
};

class IIBuffer : public IResource
{
protected:
	IIBuffer() {};
	virtual ~IIBuffer() {};

public:

	virtual bool	Create(dword Length, dword Usage, RENDERFORMAT Format, RENDERPOOL Pool) = 0;

	virtual bool	GetDesc(RENDERINDEXBUFFER_DESC * pDesc) = 0;
	virtual void *	Lock(dword dwOffsetToLock = 0, dword dwSizeToLock = 0, dword Flags = 0) = 0;
	virtual bool	Unlock() = 0;
	virtual bool	IsLocked() = 0;

	virtual bool	IsReseted() = 0;

	virtual bool 	Copy(const void * pSrc, dword dwSize, dword dwOffset = 0, dword Flags = 0) = 0;

	virtual RENDERPOOL		GetPool() const = 0;
	virtual dword		GetUsage() const = 0;
	virtual RENDERFORMAT	GetFormat() const = 0;
	virtual dword		GetLength() const = 0;
};


struct boolVector32
{
	DWORD value;

	boolVector32()
	{
		value = 0;
	}

	//Установить бит
	__forceinline void set(DWORD dwIndex, BOOL bValue)
	{
		DWORD dwVal = (DWORD)bValue;
		DWORD dwOr = (dwVal << dwIndex);
		DWORD dwMaskInv = ~(1 << dwIndex);

		value = (value & dwMaskInv) | dwOr;
	}

	//Получить бит
	__forceinline BOOL get(DWORD dwIndex)
	{
		return (((value >> dwIndex) & 0x1) == 1);
	}

};



//-------------------------------------------------------------------------
class IBaseTexture : public IResource
{
protected:
	void * internalSystemData_IDirect3DBaseTexture9;

public:

	IBaseTexture()
	{
		internalSystemData_IDirect3DBaseTexture9 = NULL;
	}

	virtual ~IBaseTexture()
	{
		internalSystemData_IDirect3DBaseTexture9 = NULL;
	}


	virtual const char *	GetName() const = 0;
	virtual const string & 	GetTexName() const = 0;
	virtual dword		GetNameHash() const = 0;
	virtual void			SetName(const char * pName) = 0;

	virtual RENDERFORMAT	GetFormat() const = 0;
	virtual RENDERPOOL		GetPool() const = 0;
	virtual dword		GetUsage() const = 0;
	virtual dword		GetLevelCount() const = 0;

	virtual dword		GetLOD() const = 0;
	virtual dword		SetLOD(dword dwLOD) = 0;

	virtual dword		GetWidth() const = 0;
	virtual dword		GetHeight() const = 0;
	virtual dword		GetDepth() const = 0;

	virtual bool		IsRenderTarget() const = 0;

	__forceinline void * GetBaseTexture ()
	{
		return internalSystemData_IDirect3DBaseTexture9;
	}


};




#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)

enum VariableType
{
	VT_UNKNOWN = 0,

	VT_VECTOR4 = 1,
	VT_VECTOR3 = 2,
	VT_FLOAT = 3,
	VT_MATRIX4x4 = 4,

	VT_BOOL = 5,

	VT_TEXTURE = 6,

	VT_FORCE_DWORD = 0x7fffffff
};



#define VARDB_PSEVDO_VIRTUAL __forceinline

class IVariable
{
	VariableType type;

	DWORD dwElementsCount;
	DWORD dwSlotsInPool;

	IBaseTexture * activeTexture;
	void * ptrInPool;

public:

	IVariable()
	{
		activeTexture = NULL;
		type = VT_UNKNOWN;
		dwElementsCount = 0;
		dwSlotsInPool = 0;
		ptrInPool = NULL;
	}

	~IVariable()
	{

	}

	VARDB_PSEVDO_VIRTUAL void Init (VariableType _type, DWORD _dwElementsCount, DWORD _dwSlotsInPool, void * _ptrInPool)
	{
		type = _type;
		dwElementsCount = _dwElementsCount;
		dwSlotsInPool = _dwSlotsInPool;
		ptrInPool = _ptrInPool;
	}

	VARDB_PSEVDO_VIRTUAL void ResetTexture()
	{
		if (activeTexture)
		{
			activeTexture->Release();
			activeTexture = NULL;
		}

		DWORD* texAddres = ((DWORD*)ptrInPool);
		*texAddres = NULL;
	}

	VARDB_PSEVDO_VIRTUAL void SetFloat(float val)
	{
		Assert (type == VT_FLOAT && dwElementsCount == 1);
		((Vector4*)ptrInPool)->x = val;
	}

	VARDB_PSEVDO_VIRTUAL void SetBool32(boolVector32 val)
	{
		*((boolVector32*)ptrInPool) = val;
	}

	VARDB_PSEVDO_VIRTUAL void SetTexture(IBaseTexture* val)
	{
		Assert(val);

		if (activeTexture)
		{
			activeTexture->Release();
			activeTexture = NULL;
		}

		activeTexture = val;
		activeTexture->AddRef();


		DWORD* texAddres = ((DWORD*)ptrInPool);
		DWORD pDXTexture = (DWORD)val->GetBaseTexture();
		*texAddres = pDXTexture;
	}

	VARDB_PSEVDO_VIRTUAL void SetMatrix(const Matrix& val)
	{
		Vector4* vecPtr = (Vector4*)ptrInPool;

		vecPtr->x = val.m[0][0];
		vecPtr->y = val.m[1][0];
		vecPtr->z = val.m[2][0];
		vecPtr->w = val.m[3][0];

		vecPtr++;

		vecPtr->x = val.m[0][1];
		vecPtr->y = val.m[1][1];
		vecPtr->z = val.m[2][1];
		vecPtr->w = val.m[3][1];

		vecPtr++;

		vecPtr->x = val.m[0][2];
		vecPtr->y = val.m[1][2];
		vecPtr->z = val.m[2][2];
		vecPtr->w = val.m[3][2];

		vecPtr++;

		vecPtr->x = val.m[0][3];
		vecPtr->y = val.m[1][3];
		vecPtr->z = val.m[2][3];
		vecPtr->w = val.m[3][3];
	}

	VARDB_PSEVDO_VIRTUAL void SetVector(const Vector & val)
	{
		((Vector4*)ptrInPool)->v = val;
	}

	VARDB_PSEVDO_VIRTUAL void SetVectorAsVector4(const Vector & val)
	{
		((Vector4*)ptrInPool)->v = val;
		((Vector4*)ptrInPool)->w = 1.0f;
	}

	VARDB_PSEVDO_VIRTUAL void SetVector4(const Vector4 & val)
	{
		*((Vector4*)ptrInPool) = val;
	}

	VARDB_PSEVDO_VIRTUAL void SetVector4Array(const Vector4* val, dword dwCount)
	{
		XMemCpy(ptrInPool, val, sizeof(Vector4) * dwCount);
	}

	VARDB_PSEVDO_VIRTUAL void * getVariableInplacePointer ()
	{
		return ptrInPool;
	}

	VARDB_PSEVDO_VIRTUAL void convertOffsetToPointer(Vector4 * v4Pool, boolVector32 * bPool, IDirect3DBaseTexture9** tPool)
	{
		Assert(v4Pool != NULL);
		Assert(bPool != NULL);
		Assert(tPool != NULL);

		DWORD dwOffset = (DWORD)ptrInPool;

		if (type == VT_BOOL)
		{
			ptrInPool = (bPool + dwOffset);
		} else
		{
			if (type == VT_TEXTURE)
			{
				ptrInPool = (tPool + dwOffset);
			} else
			{
				ptrInPool = (v4Pool + dwOffset);
			}
		}

		Assert(ptrInPool != NULL);
	}

};



#else

class IVariable
{
protected:

	virtual ~IVariable() {};

public:
	IVariable()
	{
	};

	virtual void	SetFloat(float val) = 0;
	virtual void	SetBool32(boolVector32 val) = 0;
	virtual void	SetTexture(IBaseTexture* val) = 0;
	virtual void	SetMatrix(const Matrix& val) = 0;
	virtual void	SetVector(const Vector & val) = 0;
	virtual void	SetVector4(const Vector4 & val) = 0;

	virtual void	ResetTexture() = 0;

	virtual void	SetVector4Array(const Vector4* val, dword dwCount) = 0;
};

#endif





//-------------------------------------------------------------------------
class ITexture : public IBaseTexture
{
protected:
	ITexture() {};
	virtual ~ITexture() {};

public:

	virtual bool		Create(dword Width, dword Height, dword Levels, dword Usage, RENDERFORMAT Format, RENDERPOOL Pool) = 0;
	virtual bool		GetLevelDesc(dword Level, RENDERSURFACE_DESC * pDesc) = 0;
	virtual bool		LockRect(dword Level, RENDERLOCKED_RECT * pLockedRect, CONST RECT * pRect, dword Flags) = 0;
	virtual bool		UnlockRect(dword Level) = 0;
};






//-------------------------------------------------------------------------
class IRenderTarget : public IResource
{
protected:
	IRenderTarget() {};
	virtual ~IRenderTarget() {};

public:

	virtual DX8RESOURCETYPE	GetSubType() const = 0;
	virtual bool GetDesc(RENDERSURFACE_DESC * pDesc) = 0;

	virtual IBaseTexture* AsTexture () = 0;


#ifdef _XBOX
	virtual bool FitEDRAMWithMainRT() = 0;
#endif
};


//-------------------------------------------------------------------------
class IRenderTargetDepth : public IResource
{
protected:
	IRenderTargetDepth() {};
	virtual ~IRenderTargetDepth() {};

public:

	virtual DX8RESOURCETYPE	GetSubType() const = 0;
	virtual bool GetDesc(RENDERSURFACE_DESC * pDesc) = 0;

#ifdef _XBOX
	virtual IBaseTexture* AsTexture () = 0;
#endif


};


//-------------------------------------------------------------------------

class IOcclusionQuery
{
public:

	IOcclusionQuery() {};
	virtual ~IOcclusionQuery() {};

	virtual void Release() = 0;

	virtual void Begin() = 0;
	virtual void End() = 0;

	//Если результат не готов и флажок bWaitResult=false, вернет 0xFFFFFFFF 
	virtual DWORD GetResult(bool bWaitResult = true) = 0;

};



//-------------------------------------------------------------------------
typedef struct _RENDERVIEWPORT {
	DWORD       X;
	DWORD       Y;
	DWORD       Width;
	DWORD       Height;
	float       MinZ;
	float       MaxZ;
} RENDERVIEWPORT;




//-------------------------------------------------------------------------
enum RenderTartgetOptions
{
	//восстановить содержимое реднертарета (color и depth)
	RTO_RESTORE_CONTEXT,

	//не трогать содержимое реднертарета (на X360 все поломается :))) на PC Clrear в DEBUG режиме)
	RTO_DONTOCH_CONTEXT,

	//восстановить содержимое реднертарета (color)
	RTO_RESTORE_CONTEXT_COLORONLY,


	RTO_FORCE_DWORD = 0x7fffffff
};




//-------------------------------------------------------------------------
enum StateHack
{
	SH_CULLMODE = 0,

	SH_CLIPPLANEENABLE = 1,

	SH_COLORWRITE = 2,

	SH_ZWRITE = 3,

	SH_SCISSORRECT = 4,

	SH_MAX,

	SH_FORCE_DWORD = 0x7fffffff
};

#define SHCM_NONE 0
#define SHCM_CCW 1
#define SHCM_CW 2
#define SHCM_INVERSE 3


#define SHCW_ENABLE 0
#define SHCW_DISABLE 1

#define SHZW_ENABLE 0
#define SHZW_DISABLE 1


//-------------------------------------------------------------------------
class IMediaPlayer
{
protected:
	IMediaPlayer() {};
	virtual ~IMediaPlayer() {};

public:
	enum MPResultCode
	{
		MP_OK = 0,
		MP_EOF = 1,
		MP_NO_DATA = 2,
		MP_FORCE_DWORD = 0x7fffffff
	};

	virtual bool Release() = 0;
	virtual MPResultCode RenderNextFrame() = 0;
	virtual void SetRectangle(RECT& rect) = 0;
	virtual void Pause(bool isPause) = 0;
};

class IVideoCodec;


//-------------------------------------------------------------------------
class IRender : public Service
{

protected:
#ifdef _XBOX
	bool bDisableHiZ;
#endif


public:

	//Возможные аспекты для экранов, отсортированы начиная от самого узкого, до самого широкого
	enum ScreenAspect
	{
		//Всегда считать пиксели квадратными
		//ничего не делать с матрицей проекции и с GUI
		ASPECT_ALWAYS_SQUARE_PIXELS = 0,

		//box screens
		//--------------------------------------

		//1.000 квадратный экран
		//720x720 (240x240)
//		ASPECT_1_TO_1 = 1,

		//1.250 как обычное, но чуть обрезанное с боков
		//900x720 (300x240)
		ASPECT_5_TO_4 = 2,

		//1.333 обычное разрешение
		//960x720 (320x240)
		ASPECT_4_TO_3 = 3,


		//wide screens
		//--------------------------------------

		//1.500 широкоформатное стандартное разрешение, но прилично обрезанное с боков
		//1080x720 (360x240)
		ASPECT_3_TO_2 = 4,

		//1.5625 широкоформатное стандартное разрешение, но чуть побольше обрезанное с боков
		//1125x720 (375x240)
//		ASPECT_25_TO_16 = 5,

		//1.600 широкоформатное стандартное разрешение, но чуть-чуть обрезанное с боков
		//1152x720 (384x240)
		ASPECT_16_TO_10 = 6,

		//1.777 широкоформатное стандартное разрешение
		//1280x720 (426.6666x240)
		ASPECT_16_TO_9 = 7,

		//1.800 широкоформатное стандартное разрешение, но чуть шире с боков
		//1296x720 (432x240)
//		ASPECT_18_TO_10 = 8,

		//2.000 широкоформатное разрешение ширина в два раза больше высоты
		//1440x720 (480x240)
//		ASPECT_2_TO_1 = 9,


		//2.910 очень широкий экран, в два раза почти шире чем 16:9
		//сделали его AlienWare http://www.inewidea.com/2008/01/08/4268.html
		//2095x720 (698x240)
//		ASPECT_29_TO_10 = 10,

		//3.200 очень широкий экран, в два раза почти шире чем 16:9
		//2304x720 (768x240)
//		ASPECT_16_TO_5 = 11,


		//enum force
		//--------------------------------------

		ASPECT_FORCE_DWORD = 0x7fffffff
	};


	IRender()
	{
#ifdef _XBOX
		bDisableHiZ = false;
#endif
	};

	virtual ~IRender() {};

#ifdef _XBOX

#ifdef _PRECOMPILED_COMMAND_BUFFER_BAKER
	__forceinline void DisableHyperZ (bool bDisable)
	{
		bDisableHiZ = bDisable;
	}

	__forceinline bool IsHyperZDisabled()
	{
		return bDisableHiZ;
	}
#else
	virtual void DisableHyperZ (bool bDisable) = 0;
	virtual bool IsHyperZDisabled() = 0;
#endif

	virtual void FlushHiZStencil (bool bAsynchronous) = 0;

#endif

	virtual bool					Release(IResource * pResource) = 0;

	//Устарело
	//virtual dword		GetMaxVertexShaderConst() const = 0;

	virtual const RENDERSCREEN& GetScreenInfo2D() const = 0;
	virtual const RENDERSCREEN& GetScreenInfo3D() const = 0;


	// Vertex/Index buffers section
	virtual IVBuffer *				CreateVertexBuffer(dword Length, dword Stride, const char * pFileName, long iLine, dword Usage = USAGE_WRITEONLY, RENDERPOOL Pool = POOL_MANAGED) = 0;
	virtual IIBuffer *				CreateIndexBuffer(dword Length, const char * pFileName, long iLine, dword Usage = USAGE_WRITEONLY, RENDERFORMAT Format = FMT_INDEX16, RENDERPOOL Pool = POOL_MANAGED) = 0;

	// Textures section
	virtual IBaseTexture * _cdecl	CreateTexture(const char * pFileName, long iLine, const char * pFormat, ...) = 0;
	virtual IBaseTexture * _cdecl	CreateTextureFullQuality(const char * pFileName, long iLine, const char * pFormat, ...) = 0;
	virtual IBaseTexture * _cdecl	CreateTextureUseD3DX(const char * pFileName, long iLine, const char * pFormat, ...) = 0;
	virtual ITexture *				CreateTexture(dword Width, dword Height, dword Levels, dword Usage, RENDERFORMAT Format, const char * pFileName, long iLine, RENDERPOOL Pool = POOL_MANAGED) = 0;

	virtual IRenderTarget*				CreateRenderTarget(dword Width, dword Height, const char * pFileName, long iLine, RENDERFORMAT Format, RENDERMULTISAMPLE_TYPE MultiSample = MULTISAMPLE_NONE, long MipLevelsCount = 1) = 0;
	virtual IRenderTargetDepth*				CreateDepthStencil(dword Width, dword Height, const char * pFileName, long iLine, RENDERFORMAT Format = FMT_D24S8, RENDERMULTISAMPLE_TYPE MultiSample = MULTISAMPLE_NONE, bool bCanResolveOnX360 = true) = 0;

	//Устарело
	//virtual	IParameters *			CreateParameters(const char * pFileName, long iLine, const char * pFormatString) = 0;

	virtual bool					SetIndices(IIBuffer * pIBuffer, dword dwBaseVertexIndex = 0) = 0;

	//Устарело
	//virtual bool					SetTexture(dword dwStage, IBaseTexture * pTexture) = 0;
	virtual bool					SetStreamSource(dword StreamNumber, IVBuffer * pVBuffer, dword Stride = 0, dword dwOffsetFromStartInBytes = 0) = 0;

	// Render Target Section
	virtual IRenderTarget *				GetRenderTarget(const char* szFile, long line) = 0;
	virtual IRenderTargetDepth *				GetDepthStencil(const char* szFile, long line) = 0;


	virtual bool					SetRenderTarget(RenderTartgetOptions options, IRenderTarget * pRenderTarget, IRenderTargetDepth * pZStencil = null) = 0;

	
	//устарело
	/*
	virtual bool					SetRenderTarget(DWORD dwRenderTargetIndex, ITexture * pRenderTarget, dword dwLevel, ISurface * pZStencil = null) = 0;
	virtual bool					SetRenderTarget(DWORD dwRenderTargetIndex, ICubeTexture * pRenderTarget, dword FaceType, dword Level = 0, ISurface * pZStencil = null) = 0;
*/

	virtual bool					PushRenderTarget() = 0;
	virtual bool					PopRenderTarget(RenderTartgetOptions options) = 0;

	// Vertex/Pixel Shader section
/*
	virtual dword					GetFirstFreeVertexShaderConstant() = 0;

	virtual bool					SetVertexShaderConstant(dword dwRegister, const Vector4 & vVector4) = 0;
	virtual bool					SetVertexShaderConstant(dword dwRegister, const Vector & vVector) = 0;
	virtual bool					SetVertexShaderConstant(dword dwRegister, const Matrix & mMatrix) = 0;
	virtual bool					SetVertexShaderConstant(dword dwRegister, const void * pData, dword dwConstantCount) = 0;
	virtual Vector4 *				LockVertexShaderConstant(dword dwRegister, dword dwConstantCount) = 0;
	
	virtual bool					SetPixelShaderConstant(dword dwRegister, const Vector4 & pVector) = 0;
	virtual bool					SetPixelShaderConstant(dword dwRegister, const Vector & vVector) = 0;
	virtual bool					SetPixelShaderConstant(dword dwRegister, const Matrix & mMatrix) = 0;
	virtual bool					SetPixelShaderConstant(dword dwRegister, const void * pData, dword dwConstantCount) = 0;
*/
	/*
	virtual bool					SetMaterial(const Material & Material) = 0;
	virtual const Material & 		GetMaterial() const = 0;
	*/

	virtual bool					Clear(dword Count, CONST RENDERRECT * pRects, dword Flags, RENDERCOLOR Color, float Z, dword Stencil) = 0;
	virtual bool					BeginScene() = 0;

	//Если pDestiantionTexture != null туда будет скопирован содержимое экрана
	virtual bool					EndScene(IBaseTexture* pDestiantionTexture = null, bool bSkipAnyWork = false, bool bDontResolveDepthOnX360 = false) = 0;
	virtual bool					IsInsideBeginScene() const = 0;

	// Viewport Section
	virtual bool					PushViewport() = 0;
	virtual bool					PopViewport() = 0;
	virtual const RENDERVIEWPORT &	GetViewport() const = 0;
	virtual bool					SetViewport(const RENDERVIEWPORT & Viewport) = 0;

	// Transform and Camera Section
	virtual bool					SetCamera(const Vector & vLookFrom, const Vector & vLookTo, const Vector & vUp) = 0;
	virtual void					SetView(const Matrix & mMatrix) = 0;
	virtual void					SetWorld(const Matrix & mMatrix) = 0;
	virtual void					SetProjection(const Matrix & mMatrix) = 0;
	virtual bool					SetPerspective(float fPerspective) = 0;
	virtual bool					SetPerspective(float fPerspective, float fWidth, float fHeight, float fZNear = 0.1f, float fZFar = 4000.0f) = 0;

	virtual const Matrix &			GetView() const = 0;
	virtual const Matrix &			GetWorld() const = 0;
	virtual const Matrix &			GetProjection() const = 0;
	virtual const Plane *					GetFrustum() = 0;
	virtual dword					GetNumFrustumPlanes() const = 0;

// =======================================================
// Добавление виртуальных функций для защиты разных версий
#ifdef GAME_RUSSIAN
	virtual void SetStateHack_Dword() { }
	#ifdef GAME_DEMO
		virtual IBaseTexture * CreateVolumeTexture() { return null; }
	#endif
#endif
// Добавление виртуальных функций для защиты разных версий
// =======================================================

	// Clip planes
	virtual void					SetClipPlane(dword dwIndex, const Plane & plane) = 0;

	// Light section
	virtual void					SetGlobalLight(const Vector & vDirection, bool bAttachSpecularDirToCam, const Color & cColor, const Color & cBackLightColor) = 0;
	virtual const Vector &			GetGlobalLightDirection() = 0;
	virtual const Color &			GetGlobalLightColor() = 0;
	virtual const Color &			GetGlobalLightBackColor() = 0;

	virtual dword					SetLights(const Vector & vPos, float fRadius = -1.0f) = 0;
	virtual ILight *				CreateLight(const Vector & vPos, const Color & cColor, float fRadius, float fDirectivityFactor = 1.0f) = 0;
	virtual void ReleaseLight(ILight * light) = 0;

	// Draw section

	virtual bool					DrawIndexedPrimitiveLighted(const ShaderLightingId & id, RENDERPRIMITIVETYPE Type, dword MinVertexIndex, dword NumVertices, dword StartIndex, dword PrimitiveCount, const GMXOBB &boundBox) = 0;
	virtual bool					DrawIndexedPrimitive(const ShaderId & id, RENDERPRIMITIVETYPE Type, dword MinVertexIndex, dword NumVertices, dword StartIndex, dword PrimitiveCount) = 0;
	virtual bool					DrawPrimitiveLighted(const ShaderLightingId & id, RENDERPRIMITIVETYPE Type, dword StartVertex, dword PrimitiveCount, const GMXOBB & boundBox) = 0;
	virtual bool					DrawPrimitive(const ShaderId & id, RENDERPRIMITIVETYPE Type, dword StartVertex, dword PrimitiveCount) = 0;
	virtual bool					DrawPrimitiveUP(const ShaderId & id, RENDERPRIMITIVETYPE PrimitiveType, dword PrimitiveCount, const void * pVertex, dword Stride) = 0;
	
#ifndef STOP_DEBUG
	// Debug Draw section
	virtual bool					DrawLines(RS_LINE * pRSL, dword dwLinesNum, const char * pTechniqueName = null, void * pObsoletteMustBeNULL = null, dword dwStride = 0) = 0;
	virtual bool					DrawRects(IBaseTexture* pTexture, RS_RECT * pRects, dword dwRectsNum, const char * pTechniqueName = "dbgRects", void * pObsoletteMustBeNULL = null, dword dwStride = sizeof(RS_RECT), dword dwSubTexX = 1, dword dwSubTexY = 1) = 0;
	virtual bool					DrawSprites(IBaseTexture* pTexture, RS_SPRITE * pSprites, dword dwSpritesNum, const char * pTechniqueName = "dbgSprites", void * pObsoletteMustBeNULL = null, dword dwStride = sizeof(RS_SPRITE)) = 0;
	virtual bool					DrawPolygon(const Vector * pVectors, dword dwNumPoints, const Color & color = Color(dword(0xFFFFFFFF)), const Matrix& matrix = Matrix(), const char * pTechniqueName = "stdPolygon", void * pObsoletteMustBeNULL = 0) = 0;

	virtual bool					DrawSphere(const Matrix & mPos, dword dwColor = 0xFFFFFFFF, const char * pTechniqueName = "dbgSphere", void * pObsoletteMustBeNULL = null) = 0;
	virtual bool					DrawSphere(const Vector & vPos, float fRadius, dword dwColor = 0xFFFFFFFF, const char * pTechniqueName = "dbgSphere", void * pObsoletteMustBeNULL = null) = 0;

	virtual bool					DrawLine(const Vector & v1, dword dwColor1, const Vector & v2, dword dwColor2, bool bWorldMatrix = false, const char * pTechniqueName = "dbgLine", void * pObsoletteMustBeNULL = null) = 0;
	virtual bool					DrawBufferedLine(const Vector & v1, dword dwColor1, const Vector & v2, dword dwColor2, bool bWorldMatrix = false, const char* szTechnique = "dbgLine") = 0;
	virtual bool					DrawBox(const Vector & vMin, const Vector & vMax, const Matrix & mMatrix = Matrix(), dword dwColor = 0xFFFFFFFF, const char * pTechniqueName = "dbgLine", void * pObsoletteMustBeNULL = null) = 0;

	virtual bool					DrawCapsule(float fRadius, float fHeight, dword dwColor, const Matrix &WorldMat, const char * pTechniqueName = "stdPolygon", void * pObsoletteMustBeNULL = null) = 0;

	virtual bool					DrawSolidBox(const Vector & vMin, const Vector & vMax, const Matrix & mMatrix = Matrix(), dword dwColor = 0xFFFFFFFF, const char * pTechniqueName = "dbgSolidBox", void * pObsoletteMustBeNULL = null) = 0;
	virtual bool					FlushBufferedLines(bool bIdentityMatrix = true, const char* szTechnqiue = "dbgLine") = 0;
	virtual bool					DrawMatrix(const Matrix & mMatrix, float fScale = 1.0f, const char * pTechniqueName = "dbgLine", void * pObsoletteMustBeNULL = 0) = 0;
	virtual bool					DrawVector(const Vector & v1, const Vector & v2, dword dwColor, const char * pTechniqueName = "dbgLine", void * pObsoletteMustBeNULL = null) = 0;
	virtual bool					DrawNormals(IVBuffer * pVBuffer, dword dwStartVertex, dword dwNumVertices, dword dwColor1, dword dwColor2, float fScale = 1.0f, dword dwStride = 0, const char * pTechniqueName = "dbgLine", void * pObsoletteMustBeNULL = null) = 0;
#else
	// Debug Draw section
	__forceinline  bool					DrawLines(RS_LINE * pRSL, dword dwLinesNum, const char * pTechniqueName = null, void * pObsoletteMustBeNULL = null, dword dwStride = 0) {return false;};
	__forceinline  bool					DrawRects(IBaseTexture* pTexture, RS_RECT * pRects, dword dwRectsNum, const char * pTechniqueName = "dbgRects", void * pObsoletteMustBeNULL = null, dword dwStride = sizeof(RS_RECT), dword dwSubTexX = 1, dword dwSubTexY = 1) {return false;};
	__forceinline  bool					DrawSprites(IBaseTexture* pTexture, RS_SPRITE * pSprites, dword dwSpritesNum, const char * pTechniqueName = "dbgSprites", void * pObsoletteMustBeNULL = null, dword dwStride = sizeof(RS_SPRITE)) {return false;};
	__forceinline  bool					DrawPolygon(const Vector * pVectors, dword dwNumPoints, const Color & color = Color(dword(0xFFFFFFFF)), const Matrix& matrix = Matrix(), const char * pTechniqueName = "stdPolygon", void * pObsoletteMustBeNULL = 0) {return false;};

	__forceinline  bool					DrawSphere(const Matrix & mPos, dword dwColor = 0xFFFFFFFF, const char * pTechniqueName = "dbgSphere", void * pObsoletteMustBeNULL = null) {return false;};
	__forceinline  bool					DrawSphere(const Vector & vPos, float fRadius, dword dwColor = 0xFFFFFFFF, const char * pTechniqueName = "dbgSphere", void * pObsoletteMustBeNULL = null) {return false;};

	__forceinline  bool					DrawLine(const Vector & v1, dword dwColor1, const Vector & v2, dword dwColor2, bool bWorldMatrix = false, const char * pTechniqueName = "dbgLine", void * pObsoletteMustBeNULL = null) {return false;};
	__forceinline  bool					DrawBufferedLine(const Vector & v1, dword dwColor1, const Vector & v2, dword dwColor2, bool bWorldMatrix = false, const char* szTechnique = "dbgLine") {return false;};
	__forceinline  bool					DrawBox(const Vector & vMin, const Vector & vMax, const Matrix & mMatrix = Matrix(), dword dwColor = 0xFFFFFFFF, const char * pTechniqueName = "dbgLine", void * pObsoletteMustBeNULL = null) {return false;};

	__forceinline  bool					DrawCapsule(float fRadius, float fHeight, dword dwColor, const Matrix &WorldMat, const char * pTechniqueName = "stdPolygon", void * pObsoletteMustBeNULL = null) {return false;};

	__forceinline  bool					DrawSolidBox(const Vector & vMin, const Vector & vMax, const Matrix & mMatrix = Matrix(), dword dwColor = 0xFFFFFFFF, const char * pTechniqueName = "dbgSolidBox", void * pObsoletteMustBeNULL = null) {return false;};
	__forceinline  bool					FlushBufferedLines(bool bIdentityMatrix = true, const char* szTechnqiue = "dbgLine") {return false;};
	__forceinline  bool					DrawMatrix(const Matrix & mMatrix, float fScale = 1.0f, const char * pTechniqueName = "dbgLine", void * pObsoletteMustBeNULL = 0) {return false;};
	__forceinline  bool					DrawVector(const Vector & v1, const Vector & v2, dword dwColor, const char * pTechniqueName = "dbgLine", void * pObsoletteMustBeNULL = null) {return false;};
	__forceinline bool					DrawNormals(IVBuffer * pVBuffer, dword dwStartVertex, dword dwNumVertices, dword dwColor1, dword dwColor2, float fScale = 1.0f, dword dwStride = 0, const char * pTechniqueName = "dbgLine", void * pObsoletteMustBeNULL = null) {return false;};

#endif	

	// Technique section
	//Устарело
	//virtual dword					GetTechniqueGUID(const char * pTechniqueName) const = 0;
	//virtual bool					SetEffect(const char * pEffectName = null, void * pObsoletteMustBeNULL = null) = 0;
	//virtual bool					SetTechniquePrefix(const char * pPrefixName = null) = 0;
	virtual IVariable *				GetTechniqueGlobalVariable(const char * pVariableName, const char * pFilename, long iLine) = 0;


	// Filter section
	virtual bool					AddRenderFilter(IRenderFilter * pFilter, float fExecuteLevel) = 0;
	virtual bool					RemoveRenderFilter(IRenderFilter * pFilter) = 0;

	// Create new font
	virtual IFont * _cdecl			CreateFont(const char * pFontName, float fHeight = -1.0f, dword dwColor = 0xFFFFFFFF, const char * pFontTechnique = "dbgFont") = 0;

	// Create advanced new font
	virtual IAdvFont * _cdecl			CreateAdvancedFont(const char * pFontName, float fHeight = -1.0f, dword dwColor = 0xFFFFFFFF) = 0;


	// debug output
	virtual bool _cdecl				Print(float x, float y, dword dwColor, const char * pFormat, ...) = 0;
	virtual bool _cdecl				Print(const Vector & vPos, float fViewDistance, float fLine, dword dwColor, const char * pFormat, ...) = 0;
	virtual bool _cdecl				PrintBuffered(float x, float y, dword dwColor, const char * pFormat, ...) = 0;
	virtual bool _cdecl				PrintBuffered(const Vector & vPos, float fViewDistance, float fLine, dword dwColor, const char * pFormat, ...) = 0;

	//устарело
	//virtual bool _cdecl				PrintMatrix(const Matrix & mMatrix, float x, float y, dword dwColor = 0xFFFFFFFF) = 0;
	//virtual bool _cdecl				PrintVector(const Vector & vVector, float x, float y, dword dwColor = 0xFFFFFFFF) = 0;
	//virtual bool _cdecl				PrintVector4(const Vector4 & vVector4, float x, float y, dword dwColor = 0xFFFFFFFF) = 0;

	virtual void _cdecl				SetBackgroundColor(const Color & color) = 0;
	virtual void _cdecl				SetErrorSignal(float fSignalTime = 2.0f) = 0;

	virtual IFont *					GetSystemFont() const = 0;

	virtual bool					SetUseMipFillColor(bool bNewUseMipFillColor) = 0;

	virtual bool					SaveTexture2File(IBaseTexture * pTexture, const char * pFileName) = 0;

	virtual bool					UpdateTexture(ITexture * pTexSource, ITexture * pTexDestination) = 0;

//Устарело
/*
	virtual IVariable* vl_GetLightNear() = 0;
	virtual IVariable* vl_GetLightFar() = 0;
	virtual IVariable* vl_GetLightPos() = 0;
	virtual IVariable* vl_GetLightColor() = 0;
	virtual IVariable* vl_GetLightAmbient() = 0;
*/

	virtual void SetAmbient (const Color& clr) = 0;
	virtual const Color& GetAmbient () = 0;

	virtual void SetAmbientSpecular (const Color& clr) = 0;
	virtual const Color& GetAmbientSpecular () = 0;


	virtual void DrawFullScreenQuad (float Width, float Height, const ShaderId & id) = 0;
	virtual const RENDERVIEWPORT& GetFullScreenViewPort_3D () = 0;
	virtual const RENDERVIEWPORT& GetFullScreenViewPort_2D () = 0;

	

//Устарело
//	virtual IVariable* GetSourceTextureVariable() = 0;

	virtual void PostProcess () = 0;

	virtual ITexture* GetPostprocessTexture () = 0;


	//Устарело
	//virtual bool StretchRect(ISurface* source, ISurface* destination, StretchRectFilter filter) = 0;



	virtual void DrawXZCircle (const Vector& center, float fRadius, dword dwColor, const char* szTechnique = "dbgLine") = 0;
	virtual void DrawSphereGizmo (const Vector& pos, float fRadius, dword dwColor1, dword dwColor2, const char* szTechnique = "dbgLine") = 0;

	virtual bool MakeScreenshot(const char * sufix = 0) = 0;


	virtual void SetScissorRect(CONST RECT *pRect) = 0;
	virtual void EnableLoadingScreen (bool bEnable, const char * loadingScreenTexture = NULL) = 0;
	virtual void SetLoadingScreenPercents (float percents, float total_percents, const char* stage_description) = 0;
	virtual bool IsLoadingScreenEnabled() = 0;

	virtual void SetStateHack(StateHack state, long state_value) = 0;
	virtual long GetStateHack(StateHack state) = 0;


	virtual void EnumCreatedLights(array<ILight*> & createdLights) = 0;




	virtual IRenderTarget* CreateTempRenderTarget(TempRenderTargetSize size, TempRenderTargetColor format, const char * pFileName, long iLine, long usageIndex = 0) = 0;
	virtual IRenderTargetDepth* CreateTempDepthStencil(TempRenderTargetSize size, const char * pFileName, long iLine, long usageIndex = 0) = 0;

	virtual ScreenAspect GetWideScreenAspect() = 0;
	virtual float GetWideScreenAspectWidthMultipler() = 0;


	//virtual void SetLoadingImage(IBaseTexture * pTexture) = 0;
	
	
	virtual IMediaPlayer* GetMediaPlayer(const char* pFileName,bool bLoopPlay,bool useStreaming) = 0;

	virtual IVideoCodec* CreateVideoCodec(bool useStreaming) = 0;

	//Установить переменные для теней
	virtual void SetupShadowLimiters () = 0;




	struct PerfomanceInfo
	{
		DWORD dwBatchCount;
		DWORD dwPolyCount;
	};


	virtual void GetPerfomanceInfo (IRender::PerfomanceInfo& info) = 0;


	virtual bool IsCreateRenderTarget(RENDERFORMAT format) = 0;
	virtual bool IsPostPixelShaderBlendingSupported(RENDERFORMAT format) = 0;
	virtual bool IsBlendFactorSupported() = 0;



	//true на один кадр !!! когда рендер вернулся в нормальное состояне (alt-tab вернули вокус) в full screen
	virtual bool IsRenderReseted() = 0;

	virtual void SetAmbientGamma(float fGamma) = 0;
	virtual float GetAmbientGamma() = 0;


	virtual bool GetShaderId (const char* shaderName, ShaderId & id) = 0;
	virtual bool GetShaderLightingId (const char* shaderName, ShaderLightingId & id) = 0;

	virtual IOcclusionQuery * CreateOcclusionQuery(const char *srcFile, long srcLine) = 0;

#ifdef GAME_RUSSIAN
public:
	dword protectValue;
	dword __inline ProtectValue() { return protectValue; }
#endif

// =======================================================
// Добавление виртуальных функций для защиты разных версий
#ifdef GAME_RUSSIAN
	virtual float PGetLightingLambda() { return 3.1415f; }
	#ifdef GAME_DEMO
		virtual bool GetLightChangeState() { return false; }
	#endif
#endif
// Добавление виртуальных функций для защиты разных версий
// =======================================================

#if HAVE_PIX_BEGINEND_EVENT_SUPPORT
	virtual dword _cdecl pixBeginEvent (const char *srcFile, long srcLine, const char * pFormat, ...) = 0;
	virtual void _cdecl pixEndEvent (const char *srcFile, long srcLine, dword id) = 0;
#else
	__forceinline dword _cdecl pixBeginEvent (const char *srcFile, long srcLine, const char * pFormat, ...) { return 0; };
	__forceinline void _cdecl pixEndEvent (const char *srcFile, long srcLine, dword id) {};
#endif


	virtual void setFogParams (float h_density, float h_min, float h_max, float d_density, float d_min, float d_max, const Color & color) = 0;
	virtual void getFogParams (float & h_density, float & h_min, float & h_max, float & d_density, float & d_min, float & d_max, Color & color) = 0;

	virtual void DebugIntersectLights (const GMXOBB &boundBox, array<ILight*> & affectedLights) = 0;

	virtual ITexture* getWhiteTexture() = 0;


	virtual void ResolveDepth (IRenderTargetDepth* dest) = 0;
	virtual void ResolveColor (IBaseTexture* dest) = 0;

	virtual void SetGPRAllocationCount (DWORD dwVertexShaderCount, DWORD dwPixelShaderCount) = 0;

	virtual void LightChangeState (ILight* light_ptr) = 0;

#ifdef _XBOX

	virtual void  X360_DeviceSuspend() = 0;
	virtual void  X360_DeviceResume() = 0;



#endif


};






class ILight
{
protected:

	float      affectedByShadow;
	Vector		vPos;
	Color			cRenderColor;
	Color			cColor;
	Color			cBackLightColor;

	float			fRadius;
	float			fDirectivityFactor;


	float fMinDistSquared_LOD;
	float fMaxDistSquared_LOD;
	float kLen_LOD;

	const char* debugName;
	dword debugValue;
	bool bDebugFlag_IsEnabled;

	IRender* m_pRS;



public:

	void Init(IRender * pRS)
	{
		m_pRS = pRS;

		bDebugFlag_IsEnabled = false;

		debugName = NULL;
		debugValue = 0;

		affectedByShadow = 1.0f;

		vPos = 0.0f;
		cRenderColor = Color(1.0f,1.0f,1.0f);
		cColor = Color(1.0f,1.0f,1.0f);
		cBackLightColor = Color(1.0f,1.0f,1.0f);

		fRadius = 1.0f;
		fDirectivityFactor = 0.0f;


		fMinDistSquared_LOD = (fRadius * 10.0f) * (fRadius * 10.0f);
		fMaxDistSquared_LOD = (fRadius * 12.0f) * (fRadius * 12.0f);
		kLen_LOD = 1.0f / (fMaxDistSquared_LOD - fMinDistSquared_LOD);
	}


	ILight()
	{
		Init(NULL);
	}


	~ILight()
	{
	};

	//0 - полностью в тени, 1 - не в тени
	__forceinline float getAffectedByShadowK ()
	{
		return affectedByShadow;
	}

	__forceinline void setAffectedByShadow (bool bVal)
	{
		if (bVal)
		{
			affectedByShadow = 0.0f;
		} else
		{
			affectedByShadow = 1.0f;
		}
	}


	__forceinline void debugSetName (const char * szName)
	{
		debugName = szName;
	}

	__forceinline const char * debugGetName ()
	{
		return debugName;
	}

	__forceinline void debugSetValue (dword dwVal)
	{
		debugValue = dwVal;
	}

	__forceinline dword debugGetValue ()
	{
		return debugValue;
	}


	__forceinline float getLOD_minDist_Squared()
	{
		return fMinDistSquared_LOD;
	}

	__forceinline float getLOD_maxDist_Squared()
	{
		return fMaxDistSquared_LOD;
	}

	__forceinline float getLOD_len_Squared()
	{
		return kLen_LOD;
	}

	__forceinline void Enable(bool _bEnable)
	{
		bDebugFlag_IsEnabled = _bEnable;
		m_pRS->LightChangeState (this);
	}

	__forceinline bool isEnabled() const
	{
		return bDebugFlag_IsEnabled;
	}

	__forceinline void	SetDirection(const Vector & _vDirection)
	{
		vPos = !_vDirection;
	}

	__forceinline void	SetPos(const Vector & _vPos)
	{
		Assert( !(_isnan(_vPos.x) || _isnan(_vPos.x) || _isnan(_vPos.x)));
		vPos = _vPos;
	}
	__forceinline void	SetColor(const Color & _cColor)
	{
		cColor = _cColor;
		cRenderColor = _cColor;
	}

	__forceinline void	SetRadius(float _fRadius)
	{
		fRadius = _fRadius;

		fMinDistSquared_LOD = (fRadius * 10.0f) * (fRadius * 10.0f);
		fMaxDistSquared_LOD = (fRadius * 12.0f) * (fRadius * 12.0f);
		kLen_LOD = 1.0f / (fMaxDistSquared_LOD - fMinDistSquared_LOD);
	}

	__forceinline void	SetDirectivityFactor(float _fDirectivityFactor)
	{
		fDirectivityFactor = _fDirectivityFactor;
	}

	__forceinline void	Set(const Vector & _vPos, const Color & _cColor, float _fRadius, const Color & _cBackLightColor)
	{
		Assert( !(_isnan(_vPos.x) || _isnan(_vPos.x) || _isnan(_vPos.x)));
		Assert(fRadius > 0.00001f);

		vPos = _vPos;
		cColor = _cColor;
		cRenderColor = _cColor;
		fRadius = _fRadius;
		cBackLightColor = _cBackLightColor;

		fMinDistSquared_LOD = (fRadius * 10.0f) * (fRadius * 10.0f);
		fMaxDistSquared_LOD = (fRadius * 12.0f) * (fRadius * 12.0f);
		kLen_LOD = 1.0f / (fMaxDistSquared_LOD - fMinDistSquared_LOD);
	}

	__forceinline const Vector &	GetPos() const
	{
		return vPos;
	}

	__forceinline const Vector &	GetDirection() const
	{
		return vPos;
	}

	__forceinline const Color &	GetColor() const
	{
		return cColor;
	}

	__forceinline const Color &	GetBackColor() const
	{
		return cBackLightColor;
	}

	__forceinline float GetRadius() const
	{
		return fRadius;
	}

	__forceinline float GetDirectivityFactor() const
	{
		return fDirectivityFactor;
	}

	//получить установить реальный цвет, которым будет идти рендер
	__forceinline void	SetRenderColor(const Color & _cColor)
	{
		cRenderColor = _cColor;
	}

	__forceinline const Color &	GetRenderColor() const
	{
		return cRenderColor;
	}

};


#endif