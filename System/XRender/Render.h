#ifndef STORM3_NEXTGEN_RENDER_HEADER
#define STORM3_NEXTGEN_RENDER_HEADER



#include "..\..\common_h\Render.h"
#include "..\..\common_h\IConsole.h"
#include "GraphicsApi.h"
#include "device\IDevice.h"


#include "..\..\Common_h\FileService.h"
#include "..\..\common_h\defines.h"
#include "..\..\common_h\templates\array.h"
#include "..\..\common_h\templates\stack.h"
#include "..\..\common_h\templates\string.h"
#include "..\..\common_h\templates\htable.h"
#include "..\..\common_h\templates\map.h"
#include "..\..\common_h\templates\memoryPool.h"

//#include "Lights\Light.h"

#include "Shaders\ShaderManager.h"
#include "Shaders\Shader.h"
#include "Shaders\StateFilter\StateFilter.h"


#define RENDER_MAX_STREAMS_COUNT 3


#include "Textures/Texture.h"
#include "AdvFont/AdvFontsManager.h"

#include "VertexBuffers/VBuffer.h"
#include "IndexBuffers/IBuffer.h"

#include "XLiveShower\IXLiveShower.h"

#include "D3D9SyncCode.h"

#include "AtomicI32.h"


extern void LockRes (const char* szSrcFile, long SrcLine, const CritSection& sec, const char* the_text);
extern void UnlockRes (const char* szSrcFile, long SrcLine, const CritSection& sec, const char* the_text);


#if defined (_XBOX) && defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)
	#define SHADER precompiledBatch
	#define SHADERVAR IVariable
#else
	#define SHADER Shader
	#define SHADERVAR ShaderVariable
#endif


#define RELEASE_D3D(x, mustBeRefCount) { if (x) {ULONG refCount = x->Release(); /*Assert(refCount == mustBeRefCount);*/ x = 0;} }

#define SPECULAR_STRENGHT 4.6f

#define MAX_RECTS_NUM 250

//#define RENDER_DEBUG_DRAW_SKIP

class TXXLoader;
class IRenderDevice;
class CDX8Fonts;
class IFont;
class CDX8Texture;
class TCapsule;
class OcclusionQuery;
class IVideoCodec;

struct TStateHack
{
	StateHack s;
	dword v;
	bool enabled;

	//dword dwInplaceSave;
};


class NGRender : public IRender
{
protected:

#ifdef RENDER_DEBUG_DRAW_SKIP
	int curDrawIndex;
	int curSkip, curSkip100;

	bool IsDebugDrawSkipped(int num)
	{
		bool isSkipped = false;

		if (curSkip >= 0)
			isSkipped = num == curSkip100 + curSkip;
		else
			isSkipped = num >= curSkip100 && num <= curSkip100 + 99;

		if (isSkipped && GetAsyncKeyState('9') < 0)	_asm int 3

		return isSkipped;
	}
#endif

	bool bConsoleRegistred;

	byte matrixArray[(sizeof(Matrix)*7) + 16];

	Matrix *mtxWorldView;
	Matrix *mtxWorldViewProj;
	Matrix *gmxFinalShdwMatrix;
	Matrix *mtxView;
	Matrix *mtxWorld;
	Matrix *mtxProjection;
	Matrix *mShadow;

	ICoreStorageLong * largeshot;

	Matrix mIdentity;


	Matrix mtxViewInv;
	Matrix mtxWorldInv;

	Vector4 mShadowVar;

	
	byte frustumRaw[(sizeof(Plane)*6) + 16];
	Plane * frustumAligned;


	ShaderFogVariables stdFogVariables;
	ShaderStandartVariables stdVariables;
	ShaderLightingVariables stdLightVariables;

	struct ProgressSpriteInfo
	{
		//Позиция и размер в пикселях на виртуальном экране 1280x720
		float centerX;
		float centerY;
		float size;
	};

	IPackFile * pak;
	IMirrorPath * mirror;


public:

	//FIXME: Удалите из финального мастер-релиза !!!!
	struct RenderStat
	{
		dword dwMaxDpUpSize;
		//char topDpUpName[32];

		dword dwUpdatesCount;
		dword dwConstCount;
		dword dwWCBytesTraffic;
		dword dwConstBytes;

		dword dwDrawUpBytes;

		dword dwBatchCountUP;
		dword dwBatchCount;
		dword dwPolyCount;
		dword dwDrawTime;
		dword dwDrawTimeUP;

		dword LightCountVisible;
		dword LightsPasses;
		dword LightDrawPasses;


		//Полигоны которы нарисовались, только из за света
		dword dwLightsPolyCount;

		RenderStat()
		{
			Reset();
		}

		void Reset()
		{
			dwMaxDpUpSize = 0;

			dwWCBytesTraffic = 0;
			dwConstBytes = 0;
			dwConstCount = 0;
			dwUpdatesCount = 0;

			dwDrawUpBytes = 0;
			dwDrawTime = 0;
			dwDrawTimeUP = 0;
			dwBatchCountUP = 0;
			dwBatchCount = 0;
			dwPolyCount = 0;
			
			LightCountVisible = 0;
			LightsPasses = 0;
			LightDrawPasses = 0;

			dwLightsPolyCount = 0;

		}
	};

private:

	RenderStat stats;
	RenderStat stats_saved;

	DWORD dwInitTime;
	DWORD dwInitTimeShaders;

	float fLoaderFontScale;

	float fLoadingTimeValue;

public:

	__forceinline RenderStat & renderStats()
	{
		return stats;
	}

	static NGRender* pRS;
	static NGRender* pThreadRS;

	NGRender();
	virtual ~NGRender();

	//service functions
	virtual bool Init();
	virtual void StartFrame(float dltTime);
	virtual void EndFrame(float dltTime);


#if defined (_XBOX) && !defined(_PRECOMPILED_COMMAND_BUFFER_BAKER)

	virtual void DisableHyperZ (bool bDisable);
	virtual bool IsHyperZDisabled();

#endif


	virtual bool Release(IResource* pResource);

	virtual const RENDERSCREEN& GetScreenInfo2D() const;
	virtual const RENDERSCREEN& GetScreenInfo3D() const;

	// Vertex/Index buffers section
	virtual IVBuffer* CreateVertexBuffer(dword Length, dword Stride, const char * pFileName, long iLine, dword Usage = USAGE_WRITEONLY, RENDERPOOL Pool = POOL_MANAGED);
	virtual IIBuffer* CreateIndexBuffer(dword Length, const char * pFileName, long iLine, dword Usage = USAGE_WRITEONLY, RENDERFORMAT Format = FMT_INDEX16, RENDERPOOL Pool = POOL_MANAGED);

	// Textures section
	virtual IBaseTexture* _cdecl CreateTexture(const char * pFileName, long iLine, const char * pFormat, ...);
	virtual IBaseTexture * _cdecl	CreateTextureFullQuality(const char * pFileName, long iLine, const char * pFormat, ...);
	virtual IBaseTexture* _cdecl CreateTextureUseD3DX(const char * pFileName, long iLine, const char * pFormat, ...);
	virtual ITexture* CreateTexture(dword Width, dword Height, dword Levels, dword Usage, RENDERFORMAT Format, const char * pFileName, long iLine, RENDERPOOL Pool = POOL_MANAGED);


	virtual IRenderTarget*				CreateRenderTarget(dword Width, dword Height, const char * pFileName, long iLine, RENDERFORMAT Format, RENDERMULTISAMPLE_TYPE MultiSample = MULTISAMPLE_NONE, long MipLevelsCount = 1);
	virtual IRenderTargetDepth* CreateDepthStencil(dword Width, dword Height, const char * pFileName, long iLine, RENDERFORMAT Format = FMT_D24S8, RENDERMULTISAMPLE_TYPE MultiSample = MULTISAMPLE_NONE, bool bCanResolveOnX360 = true);

	virtual bool SetIndices(IIBuffer * pIBuffer, dword dwBaseVertexIndex = 0);
	virtual bool SetTexture(dword dwStage, IBaseTexture * pTexture);
	virtual bool SetStreamSource(dword StreamNumber, IVBuffer * pVBuffer, dword Stride = 0, dword dwOffsetFromStartInBytes = 0);

	// Render Target Section
	virtual IRenderTarget* GetRenderTarget(const char* szFile, long line);
	virtual IRenderTargetDepth* GetDepthStencil(const char* szFile, long line);
	virtual bool SetRenderTarget(RenderTartgetOptions options, IRenderTarget * pRenderTarget, IRenderTargetDepth * pZStencil = null);
	//virtual bool SetRenderTarget(DWORD dwRenderTargetIndex, ITexture * pRenderTarget, dword dwLevel, ISurface * pZStencil = null);
	//virtual bool SetRenderTarget(DWORD dwRenderTargetIndex, ICubeTexture * pRenderTarget, dword FaceType, dword Level = 0, ISurface * pZStencil = null);

	virtual bool PushRenderTarget();
	virtual bool PopRenderTarget(RenderTartgetOptions options);


	//Utils section
	virtual bool Clear(dword Count, CONST RENDERRECT * pRects, dword Flags, RENDERCOLOR Color, float Z, dword Stencil);
	virtual bool BeginScene();

	//Если pDestiantionTexture != null туда будет скопирован содержимое экрана
	virtual bool EndScene(IBaseTexture* pDestiantionTexture = null, bool bSkipAnyWork = false, bool bDontResolveDepthOnX360 = false);
	virtual bool IsInsideBeginScene() const;
	virtual void _cdecl SetBackgroundColor(const Color & color);
	virtual void _cdecl SetErrorSignal(float fSignalTime = 2.0f);
	virtual bool SetUseMipFillColor(bool bNewUseMipFillColor);
	virtual bool SaveTexture2File(IBaseTexture * pTexture, const char * pFileName);
	virtual bool MakeScreenshot(const char * sufix = 0);



	// Viewport Section
	virtual bool PushViewport();
	virtual bool PopViewport();
	virtual const RENDERVIEWPORT& GetViewport() const;
	virtual bool SetViewport(const RENDERVIEWPORT & Viewport);

	// Transform and Camera Section
	virtual bool SetCamera(const Vector & vLookFrom, const Vector & vLookTo, const Vector & vUp);
	virtual void SetView(const Matrix & mMatrix);
	virtual void SetWorld(const Matrix & mMatrix);
	virtual void SetProjection(const Matrix & mMatrix);
	virtual bool SetPerspective(float fPerspective);
	virtual bool SetPerspective(float fPerspective, float fWidth, float fHeight, float fZNear = 0.1f, float fZFar = 4000.0f);

	virtual const Matrix& GetView() const;
	virtual const Matrix& GetWorld() const;
	virtual const Matrix& GetProjection() const;
	virtual const Plane* GetFrustum();


	__forceinline const Plane* GetFrustumFast()
	{
		static Vector v[4];

		if (!bNeedRecalculateFrustum) return frustumAligned;

		const Matrix & mP = *mtxProjection;

		v[0] = !Vector(mP.m[0][0],      0.0f,                   1.0f);
		v[1] = !Vector(-mP.m[0][0],     0.0f,                   1.0f);
		v[2] = !Vector(0.0f,            -mP.m[1][1],    1.0f);
		v[3] = !Vector(0.0f,            mP.m[1][1],             1.0f);

		if(largeshot->Get(0))
		{
			v[0] = Vector(0.0f, 0.0f, 1.0f);
			v[1] = Vector(0.0f, 0.0f, 1.0f);
			v[2] = Vector(0.0f, 0.0f, 1.0f);
			v[3] = Vector(0.0f, 0.0f, 1.0f);
		}

		const Matrix & mIV = mtxViewInv;

		frustumAligned[0].n = mIV.vz;
		frustumAligned[0].d = mIV.vz | mIV.pos;

		for (dword i=0; i<4; i++) 
		{
			frustumAligned[1 + i].n = mIV.MulNormal(v[i]);
			frustumAligned[1 + i].d = frustumAligned[1 + i].n | mIV.pos;
		}

		frustumAligned[5].n = -mIV.vz;
		frustumAligned[5].d = (mIV.vz * 4000.0f + mIV.pos) | (-mIV.vz);

		bNeedRecalculateFrustum = false;

		return frustumAligned;
	}

	virtual dword GetNumFrustumPlanes() const;

	// Clip planes
	virtual void SetClipPlane(dword dwIndex, const Plane & plane);

	// Light section
	virtual void SetGlobalLight(const Vector & vDirection, bool bAttachSpecularDirToCam, const Color & cColor, const Color & cBackLightColor);
	virtual const Vector & GetGlobalLightDirection();
	virtual const Color & GetGlobalLightColor();
	virtual const Color & GetGlobalLightBackColor();


	virtual dword SetLights(const Vector & vPos, float fRadius = -1.0f);
	virtual ILight* CreateLight(const Vector & vPos, const Color & cColor, float fRadius, float fDirectivityFactor = 1.0f);
	virtual void ReleaseLight(ILight * light);

	virtual void SetAmbientSpecular (const Color& clr);
	virtual const Color& GetAmbientSpecular ();

	virtual void SetAmbient (const Color& clr);
	virtual const Color& GetAmbient ();


	virtual void SetScissorRect(CONST RECT *pRect);
	virtual void EnableLoadingScreen (bool bEnable, const char * loadingScreenTexture = NULL);
	virtual void SetLoadingScreenPercents (float percents, float total_percents, const char* stage_description);
	virtual bool IsLoadingScreenEnabled();


	// Draw section
	virtual bool DrawIndexedPrimitiveLighted(const ShaderLightingId & id, RENDERPRIMITIVETYPE Type, dword MinVertexIndex, dword NumVertices, dword StartIndex, dword PrimitiveCount, const GMXOBB &boundBox);
	virtual bool DrawIndexedPrimitive(const ShaderId & id, RENDERPRIMITIVETYPE Type, dword MinVertexIndex, dword NumVertices, dword StartIndex, dword PrimitiveCount);
	virtual bool DrawPrimitiveLighted(const ShaderLightingId & id, RENDERPRIMITIVETYPE Type, dword StartVertex, dword PrimitiveCount, const GMXOBB & boundBox);
	virtual bool DrawPrimitive(const ShaderId & id, RENDERPRIMITIVETYPE Type, dword StartVertex, dword PrimitiveCount);
	virtual bool DrawPrimitiveUP(const ShaderId & id, RENDERPRIMITIVETYPE PrimitiveType, dword PrimitiveCount, const void * pVertex, dword Stride);


	virtual bool DrawLines(RS_LINE * pRSL, dword dwLinesNum, const char * pTechniqueName = null, void * pObsoletteMustBeNULL = null, dword dwStride = 0);
	virtual bool DrawRects(IBaseTexture* pTexture, RS_RECT * pRects, dword dwRectsNum, const char * pTechniqueName = "dbgRects", void * pObsoletteMustBeNULL = null, dword dwStride = sizeof(RS_RECT), dword dwSubTexX = 1, dword dwSubTexY = 1);
	virtual bool DrawSprites(IBaseTexture* pTexture, RS_SPRITE * pSprites, dword dwSpritesNum, const char * pTechniqueName = "dbgSprites", void * pObsoletteMustBeNULL = null, dword dwStride = sizeof(RS_SPRITE));
	virtual bool DrawPolygon(const Vector * pVectors, dword dwNumPoints, const Color & color = Color(dword(0xFFFFFFFF)), const Matrix& matrix = Matrix(), const char * pTechniqueName = "stdPolygon", void * pObsoletteMustBeNULL = 0);

	// Debug Draw section
	virtual bool DrawSphere(const Matrix & mPos, dword dwColor = 0xFFFFFFFF, const char * pTechniqueName = "dbgSphere", void * pObsoletteMustBeNULL = null);
	virtual bool DrawSphere(const Vector & vPos, float fRadius, dword dwColor = 0xFFFFFFFF, const char * pTechniqueName = "dbgSphere", void * pObsoletteMustBeNULL = null);
	virtual bool DrawLine(const Vector & v1, dword dwColor1, const Vector & v2, dword dwColor2, bool bWorldMatrix = false, const char * pTechniqueName = "dbgLine", void * pObsoletteMustBeNULL = null);
	virtual bool DrawBufferedLine(const Vector & v1, dword dwColor1, const Vector & v2, dword dwColor2, bool bWorldMatrix = false, const char* szTechnique = "dbgLine");
	virtual bool DrawBox(const Vector & vMin, const Vector & vMax, const Matrix & mMatrix = Matrix(), dword dwColor = 0xFFFFFFFF, const char * pTechniqueName = "dbgLine", void * pObsoletteMustBeNULL = null);
	virtual bool DrawCapsule(float fRadius, float fHeight, dword dwColor, const Matrix &WorldMat, const char * pTechniqueName = "stdPolygon", void * pObsoletteMustBeNULL = null);

	virtual bool DrawSolidBox(const Vector & vMin, const Vector & vMax, const Matrix & mMatrix = Matrix(), dword dwColor = 0xFFFFFFFF, const char * pTechniqueName = "dbgSolidBox", void * pObsoletteMustBeNULL = null);
	virtual bool DrawMatrix(const Matrix & mMatrix, float fScale = 1.0f, const char * pTechniqueName = "dbgLine", void * pObsoletteMustBeNULL = null);
	virtual bool DrawVector(const Vector & v1, const Vector & v2, dword dwColor, const char * pTechniqueName = "dbgLine", void * pObsoletteMustBeNULL = null);
	virtual bool DrawNormals(IVBuffer * pVBuffer, dword dwStartVertex, dword dwNumVertices, dword dwColor1, dword dwColor2, float fScale = 1.0f, dword dwStride = 0, const char * pTechniqueName = "dbgLine", void * pObsoletteMustBeNULL = null);

	virtual bool FlushBufferedLines(bool bIdentityMatrix = true, const char* szTechnqiue = "dbgLine");

	virtual void DrawXZCircle (const Vector& center, float fRadius, dword dwColor, const char* szTechnique = "dbgLine");
	virtual void DrawSphereGizmo (const Vector& pos, float fRadius, dword dwColor1, dword dwColor2, const char* szTechnique = "dbgLine");



	// Filter section
	virtual bool AddRenderFilter(IRenderFilter * pFilter, float fExecuteLevel);
	virtual bool RemoveRenderFilter(IRenderFilter * pFilter);

	// Font and print section
	virtual IFont* _cdecl CreateFont(const char * pFontName, float fHeight = -1.0f, dword dwColor = 0xFFFFFFFF, const char * pFontTechnique = "dbgFont");

	// Create advanced new font
	virtual IAdvFont * _cdecl			CreateAdvancedFont(const char * pFontName, float fHeight = -1.0f, dword dwColor = 0xFFFFFFFF);


	virtual IFont* GetSystemFont() const;
	virtual bool _cdecl Print(float x, float y, dword dwColor, const char * pFormat, ...);
	virtual bool _cdecl PrintShadowed(float x, float y, dword dwColor, const char * pFormat, ...);
	virtual bool _cdecl Print(const Vector & vPos, float fViewDistance, float fLine, dword dwColor, const char * pFormat, ...);
	virtual bool _cdecl PrintBuffered(float x, float y, dword dwColor, const char * pFormat, ...);
	virtual bool _cdecl PrintBuffered(const Vector & vPos, float fViewDistance, float fLine, dword dwColor, const char * pFormat, ...);


	//update textures
	virtual bool UpdateTexture(ITexture * pTexSource, ITexture * pTexDestination);
	//virtual bool StretchRect(ISurface* source, ISurface* destination, StretchRectFilter filter);



	//post process
	virtual void DrawFullScreenQuad (float Width, float Height, const ShaderId & id);
	virtual void DrawFullScreenQuadAs16x9 (float Width, float Height, const ShaderId & id);

	virtual const RENDERVIEWPORT& GetFullScreenViewPort_3D ();
	virtual const RENDERVIEWPORT& GetFullScreenViewPort_2D ();
	virtual void PostProcess ();
	virtual ITexture* GetPostprocessTexture ();


	virtual void ResolveDepth (IRenderTargetDepth* dest);
	virtual void ResolveColor (IBaseTexture* dest);


	//obsolette
	virtual IVariable* GetTechniqueGlobalVariable(const char * pVariableName, const char * pFilename, long iLine);



	virtual IRenderTarget* CreateTempRenderTarget(TempRenderTargetSize size, TempRenderTargetColor format, const char * pFileName, long iLine, long usageIndex = 0);
	virtual IRenderTargetDepth* CreateTempDepthStencil(TempRenderTargetSize size, const char * pFileName, long iLine, long usageIndex = 0);


	virtual void SetLoadingImage(const char * szTextureName);

	virtual IMediaPlayer* GetMediaPlayer(const char* pFileName,bool bLoopPlay,bool useStreaming);
	virtual IVideoCodec* CreateVideoCodec(bool useStreaming);

private:

	float fCompassArrowAngle;

	IBaseTexture* pTextureProgress;
	IBaseTexture* pTextureArrow;

	string strReloadingTextureName;
	IBaseTexture* pReloadingTexture;

	BOOL bNeedToStartVideoPlayer;
	BOOL bShowLegalVideos;
	DWORD dwLegalVideoIdx;
	array<IVideoCodec*> legalVideos;

	HANDLE evtLegalVideosDone;


	RENDERSCREEN m_Screen2DInfo;
	RENDERSCREEN m_Screen3DInfo;

	struct RTarget
	{
		RENDERVIEWPORT viewPort;
		IRenderTarget* color;
		IRenderTargetDepth* depth;
	};

	stack<RTarget> stRenderTarget;
	stack<RENDERVIEWPORT> stViewports;
	RENDERVIEWPORT m_CurrentViewPort;
	
	RENDERVIEWPORT m_FullScreenViewPort_2D;
	RENDERVIEWPORT m_FullScreenViewPort_3D;

	//Color m_AmbinetColor;
	Color m_backColor;

	bool bInsideBeginScene;
	bool bNeedRecalculateFrustum;






	Vector4 vCameraPosition_WorldSpace;
	Vector4 vCameraDirection_WorldSpace;


	void UpdateWVP();



// Вано: для гамма коррекции
private:
	ShaderId gammaShader;
	IVariable * gammaValue;
	IVariable * preGammaTexture;
	ICoreStorageFloat * storageBrightness;

	void GammaCorrection();


protected:

	volatile bool bDeviceReseted;
	volatile bool bCantRender;
	volatile bool bNeedResetDevice;


	IDirect3D9* pD3D;
	IRenderDevice* pDevice;

	bool CreateDevice ();

public:


	__forceinline bool IsDeviceLost()
	{
#ifdef _XBOX
		return false;
#else
		if (bCantRender)
		{
			return true;
		}

		return false;
		
#endif
	}

	//true когда рендер не может ничего нарисовать - например по alt-tab переключились из full screen
	__forceinline bool IsRenderDisabled()
	{
#ifdef _XBOX
		return false;
#else

		if (bCantRender)
		{
			return true;
		}

		if (bDeviceReseted)
		{
			return true;
		}

		return false;
#endif
	}




protected:

	struct RECT_VERTEX
	{
		Vector	vPos;
		dword	dwColor;
		float	tu, tv;
	};

	struct SphVertex
	{
		Vector	v;
		dword	c;
	};

	struct SolidBoxVertex
	{
		Vector vPos;
		Vector vNormal;
	};

	struct POLYGON_VERTEX
	{
		Vector		vPos;
		dword		dwColor;
	};


	static SolidBoxVertex DX8SBVertex[];
	static short DX8SBIndices[];



	IVBuffer* pSpritesVBuffer;
	IVBuffer* pVSolidBox;
	IIBuffer* pISolidBox;
	IVBuffer* pRectsVBuffer;
	IIBuffer* pRectsIBuffer;
	IVBuffer * pVSphereBuffer;

	dword dwNumSphereTrgs;
	void CreateSphere();
	void MakeDrawVector(RS_LINE * pLines, dword dwNumSubLines, const Matrix & mMatrix, Vector vUp, Vector v1, Vector v2, float fScale, dword dwColor);


	array<CDX8Texture*> aTextures;			// all textures types
	
	array<IRenderTarget*> aRenderTargets;			// all surfaces
	array<IRenderTargetDepth*> aRenderTargetsDepth;			// all surfaces

	array<IVBuffer*> aVBuffers;			// all vertex buffers
	array<IIBuffer*> aIBuffers;			// all index buffers
	array<ILight*> aLights;

	array<ILight*> activeLights;

	array<ILight*> passLights;
	array<IFont*> aFonts;
	array<IRenderFilter*> aRenderFilters;
	array<POLYGON_VERTEX> aPolygons;




	array<RS_LINE> aBufferedLines;

	TCapsule* capsuleRender;
	CDX8Fonts* pFonts;
	AdvancedFonts* pAdvFonts;
	IFont* pSystemFont;
	//IFont* pLoaderFont;
	IAdvFont* pAdvLoaderFont;
	
	ShadersManager m_ShaderManager;

	D3DPRESENT_PARAMETERS d3dpp;


	string m_TextureDirectory;


	long iScreenshotFrameIndex;

	TXXLoader* m_TXXLoader;

	bool bUseMipFillColor;

	dword dwCurrentBaseVertexIndex;

	long winPosX;
	long winPosY;
	bool bExpand2FullScreen;
	bool bMaximize2FullScreen;


	struct PrintBuffer
	{
		bool	bCentered;
		float	x, y;
		Vector	vPos;
		float	fLine, fViewDistance;
		dword	dwColor;
		char	* pBuffer;
	};
	array<PrintBuffer> aBufferedPrints;



	struct QuadVertex
	{
		Vector4 vPos;

		float u0;
		float v0;

	};
	QuadVertex PostProcessQuad[4];


	ITexture* m_WhiteEmptyTexture;
	ITexture* pSpecularTexture;

	IXLiveShower* m_pXLiveShower;


public:

	__forceinline IDirect3DDevice9* D3D()
	{
		if (pDevice)
		{
			return pDevice->D3D();
		}

		return NULL;
	}


	bool ReleaseResource(IResource * pResource);

	dword GetTextureDegradation();
	bool IsMipMapsFillColor ();

/*
	__forceinline IDirect3DBaseTexture9* __GetDXBaseTexture__(IBaseTexture * pTexture)
	{
		if (pTexture)
		{
			//return pTexture->GetBaseTexture();

			DX8RESOURCETYPE type = pTexture->GetType();

			if (type == DX8TYPE_TEXTURE)
			{
				return ((CDX8Texture*)pTexture)->GetBaseTexture();
			} else
			{
				if (type == DX8TYPE_ANIMTEXTURE)
				{
					((CDX8AnimTexture*)pTexture)->GetBaseTexture();
				} else
				{
					if (type == DX8TYPE_VOLUMETEXTURE)
					{
						return ((CDX8VolumeTexture*)pTexture)->GetBaseTexture();
					} else
					{
						if (type == DX8TYPE_CUBETEXTURE)
						{
							return ((CDX8CubeTexture*)pTexture)->GetBaseTexture();
						} 
					}
				}
			}
		}

		return null;
	}
	*/


	void SetLightVariables (ShaderId shaderId, ILight** lightsArray, dword dwArraySize);
	


	
	ITexture* getSpecularTexture();



private:

	bool bShowStatistic;

	StateFilter* redundantStatesFilter;

	bool bDrawStats;

	array<IRenderFilter*> EnabledFilters;


#ifdef SHOW_HISTOGRAM_DELETE_ME_BEFORE_RELEASE
	long histogramm_bin;
	float histogram_totalPixelsInScreen;
	float histogram_data[255];
	IOcclusionQuery* _histogram_query;
	IVariable* histogram_source;
	IVariable* histogram_min;
	IVariable* histogram_max;
	IRenderTarget* histogram;
	bool bShowHistogram;
	long histogram_update_chunk;
#endif

	bool bPostProcessAlreadyDone;
	bool bPostProcessEnabled;
	dword dwTextureDegradation;
	void ReadIniParams();




protected:

	struct Line2DVertex
	{
		Vector p;
		dword color;
	};


	void ScreenToD3D (float sX, float sY, float &d3dX, float &d3dY);
	void Draw2DLinePerfomanace (float pX, float pY, float tX, float tY, dword color);

/*
	struct PerfGraph
	{
		float allocs;
		float batches;
		float polycount;
		float deltatime;
	};

	array<PerfGraph> perfGraph;
*/	
	struct perfGraph
	{
		dword dwColor;
		dword dwID;
		string name;
		array<float> graphData;
		float scale;

		float min;
		float max;
		bool avgInited;
		float avg;

		perfGraph() : graphData (_FL_, 2048)
		{
			avgInited = false;
			min = 99999999999999999.0f;
			max = -99999999999999999.0f;
			avg = 0.0f;

			scale = 1.0f;
			dwID = 0xFFFFFFFF;
			name = "unnamed";
		}
	};

 array<perfGraph> perfGraphs;

public:

	StateFilter* getRedundantStatesFilter();

	CDX8Fonts* getFontsManager();
	AdvancedFonts* getAdvancedFontsManager();


	void NotifyDeleteRT(IRenderTarget * rt);
	void NotifyDeleteRTD(IRenderTargetDepth * rt);


	static void ThreadExecute(LPVOID lpThreadParameter);

/*
	void ShowStartupScreen(dword dwLocStringID, const char * englishStringID, const char * szAdditionalMessage = NULL);
*/



protected:


	TStateHack hacks[SH_MAX];


private:


	string strTitle;
	string strPath;
	string strFullPath;
	string strTexName;


	void* pLastIndices;

	bool bNeedReplaceDP_to_DPUP;
	IVBuffer* pMemVB;
	IIBuffer* pMemIB;
	DWORD dwMemVBStride;


	void initPerfGraphs ();

	long waitGPUFrames;
	bool bDontShowReloading;



	atomicI32 _finalClean;
	atomicI32 _bNeedCloseThread;
	
	//volatile bool finalClean;
	//volatile bool bNeedCloseThread;
	CritSection loading_screen_sc;
	CritSection resource_creation_sc;


	
	

	bool DrawLoadingScreen ();

	void ConvertTRS2Size (TempRenderTargetSize size, dword & width, dword & height);

	struct TempRTC
	{
		TempRenderTargetSize size;
		TempRenderTargetColor color;
		long index;
		IRenderTarget* rt;
	};

	struct TempRTD
	{
		TempRenderTargetSize size;
		long index;
		IRenderTargetDepth* rt;
	};

	array<TempRTC> tempRenderTargetsC;
	array<TempRTD> tempRenderTargetsD;


	bool bDisableHacks;

	ScreenAspect aspect;
	//bool bIsWideScreen;


	float loading_percents;
	float loading_percents_total;


	float activeHintTime;
	DWORD activeHintID;
	array<string> loadingHints;
	string loadingMessage;



	bool bNeedBeginScene_AfterExit;


	IVariable* varColor;
	IVariable* varTexture;


	DWORD LoadingScreenThreadId;

public:

	__forceinline void LockResourceCreation(const char* txt)
	{
		LockRes(_FL_, resource_creation_sc, txt);
	}

	__forceinline void UnLockResourceCreation(const char* txt)
	{
		UnlockRes(_FL_, resource_creation_sc, txt);
	}

	HANDLE loadingScreenThread;


	virtual void SetStateHack(StateHack state, long state_value);
	virtual long GetStateHack(StateHack state);



	virtual bool IsCreateRenderTarget(RENDERFORMAT format);
	virtual bool IsPostPixelShaderBlendingSupported(RENDERFORMAT format);
	virtual bool IsBlendFactorSupported();

	virtual void setFogParams (float h_density, float h_min, float h_max, float d_density, float d_min, float d_max, const Color & color);
	virtual void getFogParams (float & h_density, float & h_min, float & h_max, float & d_density, float & d_min, float & d_max, Color & color);


	virtual bool GetShaderId (const char* shaderName, ShaderId & id);
	virtual bool GetShaderLightingId (const char* shaderName, ShaderLightingId & id);

	


	void ApplyStateHacks ();
	void RestoreAfterStateHacks ();

	IRenderDevice* Device();

	//virtual ScreenAspect IsWideScreen();

	virtual ScreenAspect GetWideScreenAspect();
	virtual float GetWideScreenAspectWidthMultipler();


	virtual ITexture* AddTexture(const char * pFileName, long iLine);

	virtual void SetupShadowLimiters ();

	virtual void EnumCreatedLights(array<ILight*> & createdLights);

	virtual ITexture* getWhiteTexture();


protected:

	float fWidthAspectMultipler;

	IVariable* shaderTime;
	float fTimeToShader;

	IVariable* GMX_ShadowLimiters;
	IVariable* GMX_ShadowMatrix;



	void InitShadowLimiters();
	void ReleaseShadowLimiters();
	void PrepareShadowLimiters (); // - viewport or resolution changed


#ifndef _XBOX
	HWND globalHWND;
#endif




public:


	void DumpAllMemoryUsageToLog();


	void OnResetDevice();


	//Нужно вызывать когда окошко получает фокус
	virtual void AcquireFocus ();


	virtual void GetPerfomanceInfo (IRender::PerfomanceInfo& info);



	//true на один кадр !!! когда рендер вернулся в нормальное состояне (alt-tab вернули вокус) в full screen
	virtual bool IsRenderReseted();


	bool IsFinalCleanup();

	virtual void SetAmbientGamma(float fGamma);
	virtual float GetAmbientGamma();


#if HAVE_PIX_BEGINEND_EVENT_SUPPORT
	virtual dword _cdecl pixBeginEvent (const char *srcFile, long srcLine, const char * pFormat, ...);
	virtual void _cdecl pixEndEvent (const char *srcFile, long srcLine, dword id);
#else
	__forceinline dword _cdecl pixBeginEvent (const char *srcFile, long srcLine, const char * pFormat, ...) { return 0; };
	__forceinline void _cdecl pixEndEvent (const char *srcFile, long srcLine, dword id) {};
#endif

	virtual void DebugIntersectLights (const GMXOBB &boundBox, array<ILight*> & affectedLights);

	virtual void SetGPRAllocationCount (DWORD dwVertexShaderCount, DWORD dwPixelShaderCount);

	virtual void LightChangeState (ILight* light_ptr);

	virtual IOcclusionQuery * CreateOcclusionQuery(const char *srcFile, long srcLine);

	void ReleaseQueryNotify(OcclusionQuery * ptr);
	


#ifdef _XBOX

	virtual void  X360_DeviceSuspend();
	virtual void  X360_DeviceResume();


#endif



protected:


	array<OcclusionQuery*> queries;
	

	struct eventID
	{
		dword id;
		const char * srcFile;
		long srcLine;

		eventID()
		{

		}

		eventID(dword _id, const char * _srcFile, long _srcLine)
		{
			id = _id;
			srcFile = _srcFile;
			srcLine = _srcLine;
		}
	};


	stack<eventID> currentEvents;
	dword dwPixGUID;

	D3D9Sync d3d9Sync;

	objectsPool<ILight, 128> lightPool;


	struct vStream
	{
		IDirect3DVertexBuffer9* vertexData;
		dword dwStride;
		dword dwOffsetFromStartInBytes;
	};

	vStream vertexStreams[RENDER_MAX_STREAMS_COUNT];

	bool bInsideFrame;

	bool bDepthDisabled;

#ifdef GAME_RUSSIAN
	void  __declspec(dllexport) InitProtectData(dword dwValue, dword dwValue2);
#endif

	long bForceWideScreen;
	
	dword lastIBUsageFlag;
	dword lastVBUsageFlag;

	dword dwLastTimeWhenTooltipChange;

	float width_div_height_k;
	long multiSample;
	long deviceIndex;

	void RestoreIfNeed ();


	void DumpMemStat ();


	enum ErrorID
	{
		ERR_D3D = 0,
		ERR_DEVICE = 1,
		ERR_INIT = 2,
		ERR_SHADERS30 = 4,
		ERR_D3DX = 5,

		ERR_FORCE_DWORD = 0x7fffffff
	};

	void MakeError (ErrorID id);


	void CheckAvailMemory();



	void _cdecl DebugMemory(const ConsoleStack & params);

#ifdef _XBOX	

#ifndef STOP_DEBUG
	void PrintMemoryStatistics();
#endif

	virtual void FlushHiZStencil (bool bAsynchronous);
#endif


public:	
	__forceinline bool SetStreamSourceD3D(dword StreamNumber, IDirect3DVertexBuffer9 * pVertexStream, dword Stride, dword dwOffsetFromStartInBytes, dword dwUsage, IVBuffer* pMemVBForDynamicOnly)
	{
		//не использовать напрямую на PC !!!!
		//т.к. не проверяет lost device - специальная функция для X360
		//для PC есть NGRender::SetStreamSource

		//AssertCoreThread;

		Assert (StreamNumber < RENDER_MAX_STREAMS_COUNT);

		//Кратность 4-ке проверяем
		Assert((dwOffsetFromStartInBytes & 0x3) == 0);

		vertexStreams[StreamNumber].vertexData = pVertexStream;
		vertexStreams[StreamNumber].dwStride = Stride;
		vertexStreams[StreamNumber].dwOffsetFromStartInBytes = dwOffsetFromStartInBytes;

#if defined(_XBOX) && defined(USED_X360_DYNAMIC_BUFFER_HACK)
		if (pVertexStream)
		{
			Assert ((dwUsage & USAGE_DYNAMIC) == 0)
			HRESULT hr = D3D()->SetStreamSource(StreamNumber, pVertexStream, dwOffsetFromStartInBytes, Stride);
			Assert(hr == D3D_OK);
			bNeedReplaceDP_to_DPUP = false;
			pMemVB = NULL;
			dwMemVBStride = 0;
		} else
		{
			if ((dwUsage & USAGE_DYNAMIC) != 0)
			{
				//Надо запомнить что все вызовы DrawPrimitive & DrawIndexedPrim надо делать из памяти...
				HRESULT hr = D3D()->SetStreamSource(StreamNumber, null, 0, 0);
				bNeedReplaceDP_to_DPUP = true;
				Assert(pMemVBForDynamicOnly);
				pMemVB = pMemVBForDynamicOnly;
				dwMemVBStride = Stride;
			} else
			{
				HRESULT hr = D3D()->SetStreamSource(StreamNumber, null, 0, 0);
				bNeedReplaceDP_to_DPUP = false;
				pMemVB = NULL;
				dwMemVBStride = 0;
			}

		}
#else

		HRESULT hr = D3D()->SetStreamSource(StreamNumber, pVertexStream, dwOffsetFromStartInBytes, Stride);
		Assert(hr == D3D_OK);

#endif

		if (StreamNumber == 0 && pVertexStream)
		{
			lastVBUsageFlag = dwUsage;
		}

		return true;
	}


	bool SetIndicesD3D(IDirect3DIndexBuffer9 * pIndexStream, dword dwBaseVertexIndex, dword dwUsage, IIBuffer* pMemIBForDynamicOnly)
	{
		//не использовать напрямую на PC !!!!
		//т.к. не проверяет lost device - специальная функция для X360
		//для PC есть NGRender::SetStreamSource

		//AssertCoreThread;

		pLastIndices = NULL;
		lastIBUsageFlag = 0;

		dwCurrentBaseVertexIndex = dwBaseVertexIndex;

#if defined(_XBOX) && defined(USED_X360_DYNAMIC_BUFFER_HACK)
		if (pIndexStream)
		{
			Assert ((dwUsage & USAGE_DYNAMIC) == 0)
			HRESULT hr = D3D()->SetIndices(pIndexStream);
			Assert(hr == D3D_OK);
			pMemIB = NULL;
		} else
		{
			if ((dwUsage & USAGE_DYNAMIC) != 0)
			{
				//Надо запомнить что все вызовы DrawPrimitive & DrawIndexedPrim надо делать из памяти...
				HRESULT hr = D3D()->SetIndices(NULL);
				pMemIB = pMemIBForDynamicOnly;
			} else
			{
				HRESULT hr = D3D()->SetIndices(NULL);
				pMemIB = NULL;
			}

		}
#else

		HRESULT hr = D3D()->SetIndices(pIndexStream);
		Assert(hr == D3D_OK);

#endif

		if (pIndexStream)
		{
			lastIBUsageFlag = dwUsage;
		}


		pLastIndices = pIndexStream;

		return true;
	}

protected:
	
/*
	struct SamplerTexturePair
	{
		dword lineNumber;
		string SamplerName;
		string TextureName;
	};

	bool FindTextureTokenBeforeCloseBracket (const array<string> &tokens, dword &index, dword &lineNum);

	bool SkipToNextComma (const array<string> &tokens, dword &index, dword &lineNum);
	bool SkipToNextNotSpace (const array<string> &tokens, dword &index, dword &lineNum);
	bool SkipToPrevNotSpace (const array<string> &tokens, dword &index, dword &lineNum);
	void ConvertFX2FXL(const char* fxName, const char* fxlName);

*/

	virtual bool DrawIndexedPrimitiveSingleThread(const ShaderId & id, RENDERPRIMITIVETYPE Type, dword MinVertexIndex, dword NumVertices, dword StartIndex, dword PrimitiveCount);

	CritSection x360_vidArray;
	IVideoCodec* x360_played_legal;

	void BuildHintsList();
	void UnloadLegalVideos();
	void LoadLegalVideos(bool bFirstTime);

};




#endif