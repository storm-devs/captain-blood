#ifndef RENDER_OCCLUSION_QUERY
#define RENDER_OCCLUSION_QUERY 


#include "..\..\..\common_h\Render.h"
#include "..\GraphicsApi.h"

class OcclusionQuery : public IOcclusionQuery
{

	IDirect3DQuery9* query;

	void Create();

public:

	OcclusionQuery();
	virtual ~OcclusionQuery();

	virtual void Release();

	virtual void Begin();
	virtual void End();

	//Если результат не готов и флажок bWaitResult=false, вернет 0xFFFFFFFF 
	virtual DWORD GetResult(bool bWaitResult = true);

	void OnLostDevice();
	void OnResetDevice();

};


#endif

