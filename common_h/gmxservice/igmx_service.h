#ifndef GMX_SERVICE_INTERFACE
#define GMX_SERVICE_INTERFACE

#include "..\render.h"


class IRender;
class IGMXScene;
class IParticleManager;
class ISoundScene;
class IAnimationScene;


#define CreateGMX(filename, animation, particles, sound) CreateScene(filename, animation, particles, sound, __FILE__, __LINE__)



class IGMXService : public Service
{

public:

	enum HackMode
	{
		HM_OFF = 0,
		HM_SHOW_BATCHES = 1,
		HM_SHOW_OVERDRAW = 2,
		HM_SHOW_DIFFUSE = 3,
		HM_SHOW_NORMAL_MAP = 4,
		HM_SHOW_PARALLAX = 5,
		HM_SHOW_SHININES = 6,
		HM_SHOW_GLOSS = 7,
		HM_SHOW_OCCLUSION = 8,
		HM_SHOW_NORMALS = 9,

		HM_SHOW_BINORMALS = 10,
		HM_SHOW_TANGENTS = 11,

		HM_SHOWLIGHTCOMPLEX = 12,


		HM_MAX = 13,

		HM_FORCE_DWORD = 0x7fffffff
	};

	struct RenderModeShaderId
	{
		ShaderLightingId bones[4];
	};



	IGMXService() {};
	virtual ~IGMXService() {};

	virtual void SetHackMode (IGMXService::HackMode _mode) = 0;
	virtual IGMXService::HackMode GetHackMode () = 0;


	virtual IGMXScene* CreateScene (const char* szFileName, IAnimationScene * animationScene, IParticleManager* pManager, ISoundScene * pSound, const char* pSourceFile, int Line) = 0;

	virtual IRender* Render() = 0;

	virtual void SetTexturePath(const char* szPath) = 0;
	virtual const char* GetTexturePath() = 0;


	virtual void SetRenderMode (IGMXService::RenderModeShaderId * new_shaders, IGMXService::RenderModeShaderId * new_shaders_without_alpha = NULL) = 0;
	virtual IGMXService::RenderModeShaderId* GetRenderMode () = 0;
	virtual IGMXService::RenderModeShaderId* GetRenderModeNoAlpha () = 0;


	virtual void SetUserColor (const Color& clr) =0;
	virtual const Color& GetUserColor () = 0;

	virtual void SuppressSceneSetUserColor (bool bDenied) =0;
	virtual bool IsSuppressSceneSetUserColor () =0;
	

	virtual bool GetGlobalFrustumCullState () = 0;
	virtual void SetGlobalFrustumCullState (bool bEnable) = 0;

	virtual void GetRenderModeShaderId (const char * szShaderName, IGMXService::RenderModeShaderId & shaders_id) = 0;



};


#endif

