#ifndef GMX_MATERIAL_INTERFACE
#define GMX_MATERIAL_INTERFACE


#include "..\render.h"

class IBaseTexture;

class IGMXMaterial
{

protected:

	virtual ~IGMXMaterial() {};

public:

	IGMXMaterial() {};

	//virtual void Setup () = 0;
	//virtual const ShaderLightingId & GetTechnique () = 0;
	//virtual void SetTechnique (const char* szTechnique) = 0;

	//virtual void AddRef () = 0;
	//virtual bool Release () = 0;

	//virtual dword GetTextureCount() = 0;
	//virtual IBaseTexture* GetTexture(dword dwIndex) = 0;
	//virtual void SetTexture (dword dwIndex, IBaseTexture* pTex) = 0;

	//virtual void SetTextureCount (dword _dwTexCount) = 0;


	//virtual const char* GetID () = 0;

};


#endif

