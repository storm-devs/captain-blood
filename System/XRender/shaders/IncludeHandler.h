

#ifndef FX_SHADERS_INCLUDE_HANDLER
#define FX_SHADERS_INCLUDE_HANDLER



#ifndef MANAGER_STANDALONE_COMPILE_ONLY
#include "..\graphicsApi.h"
#endif

#include "..\CompilerMode.h"


class RenderIncludeHandler : public ID3DXInclude
{
	bool m_bFake;

public:

#ifdef _XBOX
	STDMETHOD(Open)(THIS_ D3DXINCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes, LPSTR pFullPath, DWORD cbFullPath);
#else
	STDMETHOD(Open)(THIS_ D3DXINCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes);
#endif

	STDMETHOD(Close)(THIS_ LPCVOID pData);


	RenderIncludeHandler();
	~RenderIncludeHandler();


	void FakeMode (bool bFake);

};


#endif

