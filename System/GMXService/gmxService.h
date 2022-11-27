/********************************************************************
created:	2009 dec
author:		Sergey Makeev
purpose:	mesh and geometry engine service
*********************************************************************/
#ifndef GMX_SERVICE_H
#define GMX_SERVICE_H

#include "..\..\common_h\core.h"
#include "..\..\common_h\render.h"
#include "..\..\common_h\templates.h"
#include "..\..\Common_h\FileService.h"
#include "..\..\common_h\gmxservice\igmx_service.h"
#include "..\..\common_h\ishadows.h"
#include "..\..\common_h\iconsole.h"
#include "mesh_data.h"

class MeshContainer;
class MeshInstance;
class IPhysics;

enum GMX_ShowMode
{
	GMXSM_ALL = 0,
	GMXSM_NODIFFUSE = 1,
	GMXSM_NODIFFUSE_NONORMAL = 2,
	GMXSM_NONORMAL = 3,

	GMXSM_MAX = 4,

	GMXSM_FORCE_DWORD = 0x7fffffff
};

#define MAX_ANIMATION_BONES (255)

#ifdef _XBOX
#define VERTEXSTREAMS_WITH_CONSTANTS_POOL 512
#endif

class GMXService : public IGMXService
{
	bool bGlobalFrustumCull;
	bool rmode_enabled;
	bool UserColorFromSceneDenied;

	RenderModeShaderId rmode;
	RenderModeShaderId rmode_noalpha;

	dword dwCacheFrameNum;
	
	HackMode mode;

#ifndef STOP_DEBUG

	ITexture * pEmptyNormalMapTexture;

	GMX_ShowMode show_mode;

	IGMXService::RenderModeShaderId* debugShaders;//IGMXService::RenderModeShaderId debugShaders[GMXService::HM_MAX];

public:
	__forceinline const IGMXService::RenderModeShaderId& getDebugShaderByIndex(dword dwDebugIndex)
	{
		Assert(dwDebugIndex < GMXService::HM_MAX);
		return debugShaders[dwDebugIndex];

	}

	__forceinline GMX_ShowMode GetShowMode ()
	{
		return show_mode;
	}


protected:
#endif



	IRender* pRS;
	IShadowsService* pShadowService;
	IFileService * pFS;


	Color serviceUserColor;
	IVariable* pUserColor;
	IVariable* gmxAlphaRef;
	IVariable* GMX_ShadowQuad;
	IVariable* GMX_ShadowColor;


	ITexture* pNoShadowTexture;




#ifndef _XBOX
	IVariable* skinBones;
#else
	dword dwCurrentPoolIndex;
	IVBuffer* bonesArrayPool[VERTEXSTREAMS_WITH_CONSTANTS_POOL];
#endif

	char texturesPath[MAX_PATH];

	htable<MeshContainer *> meshesFiles;

	string strBuffer1, strBuffer2;


	void dumpMeshLeaksAndCleanup();

public:

	
#ifndef STOP_DEBUG
	IBaseTexture* getEmptyNormalMap ()
	{
		return pEmptyNormalMapTexture;
	}
#endif



	MeshInstance * allocMeshInstance(MeshContainer * _container, const char * debugMeshName, const char* pSourceFile, int Line);
	void freeMeshInstance(MeshInstance * meshInst);
	

public:

	GMXService();
	virtual ~GMXService();

	virtual void SetHackMode (IGMXService::HackMode _mode);
	virtual IGMXService::HackMode GetHackMode ();


	virtual IGMXScene* CreateScene (const char* szFileName, IAnimationScene * animationScene, IParticleManager* pManager, ISoundScene * pSound, const char* pSourceFile, int Line);

	virtual IRender* Render();

	virtual void SetTexturePath(const char* szPath);
	virtual const char* GetTexturePath();


	virtual void SetRenderMode (IGMXService::RenderModeShaderId * new_shaders, IGMXService::RenderModeShaderId * new_shaders_without_alpha = NULL);
	virtual IGMXService::RenderModeShaderId* GetRenderMode ();
	virtual IGMXService::RenderModeShaderId* GetRenderModeNoAlpha ();


	__forceinline IGMXService::RenderModeShaderId* GetRenderMode_Fast ()
	{
		if (!rmode_enabled)
		{
			return NULL;
		}

		return &rmode;
	}

	__forceinline IGMXService::RenderModeShaderId* GetRenderModeNoAlpha_Fast ()
	{
		if (!rmode_enabled)
		{
			return NULL;
		}

		return &rmode_noalpha;
	}



	virtual void SetUserColor (const Color& clr);
	virtual const Color& GetUserColor ();

	virtual void SuppressSceneSetUserColor (bool bDenied);
	virtual bool IsSuppressSceneSetUserColor ();


	virtual bool GetGlobalFrustumCullState ();

	__forceinline bool GetGlobalFrustumCullStateFast ()
	{
		return bGlobalFrustumCull;
	}

	virtual void SetGlobalFrustumCullState (bool bEnable);

	virtual void GetRenderModeShaderId (const char * szShaderName, IGMXService::RenderModeShaderId & shaders_id);


	virtual bool Init();

	void SetCustomAlphaRef(float alphaRef);


	MESH_INLINE IShadowsService * GetShadowSrv()
	{
		return pShadowService;
	}

	MESH_INLINE IVariable* GMXService::GetShadowTex_Variable()
	{
		return GMX_ShadowQuad;
	}

	MESH_INLINE IVariable* GMXService::GetShadowColor_Variable()
	{
		return GMX_ShadowColor;
	}

	MESH_INLINE IBaseTexture* GMXService::GetNoShadowTexture ()
	{
		return pNoShadowTexture;
	}



	virtual void EndFrame(float dltTime);



#ifdef _XBOX
	IVBuffer* GetSkinBones_Stream();
#else
	IVariable* GetSkinBones_Variable();
#endif


	static GMXService * pGS;
	static IPhysics * pPhysic;

#ifndef STOP_DEBUG
	static IConsole* pConsole;
	static dword bBlendShapeDebug;
	static dword bGeometryDebug;
#endif


	//Голова списка созданных инстансов
	static MeshInstance * pHead;


	void reportMeshContainerRelease(MeshContainer * meshFile);

	__forceinline dword getCacheFrame()
	{
		return dwCacheFrameNum;
	}

#ifndef STOP_DEBUG
	void _cdecl BlendShapesDebugSwitch(const ConsoleStack& stack);
	void _cdecl GeometryDebugModeSwitch(const ConsoleStack& stack);

	
#endif



};

#endif