#ifndef _XBOX

// закомментить этот дефайн если надо отрубить поддержку Games for Windows - Live
// ибо при отсутствии требуемых xlive.dll, xlive.dll.cat и xlivefnt.dll
// рендер не загрузиЦа... а с дллелями пока непонятки :( - надо разбираться с секьюрити
//#define _ENABLE_XLIVE

#define WIN32_LEAN_AND_MEAN
//#define NOMINMAX
#include <windows.h>

#include <d3d9.h>
#include <d3dx9.h>

//#include "..\..\common_h\core.h"
//#include "..\GraphicsApi.h"
#include "XLiveShowerPC.h"

#ifdef _ENABLE_XLIVE
#include <winlive.h>
#pragma comment(lib,"xlive.lib")
#endif

XLiveShowerPC::~XLiveShowerPC()
{
	if( m_bInited )
	{
#ifdef _ENABLE_XLIVE
		XLiveUnInitialize();
#endif
		m_bInited = false;
	}
}

XLiveShowerPC::XLiveShowerPC(IDirect3DDevice9* pD3D, void* pD3DPP)
{
	m_bInited = false;

#ifdef _ENABLE_XLIVE
	DWORD dwErrCode;

	XLIVE_INITIALIZE_INFO xii = {0};
	//xii.dwFlags = XLIVE_INITFLAG_NO_AUTO_LOGON;
	xii.cbSize = sizeof( xii );
	xii.pD3D = pD3D;
	xii.pD3DPP = pD3DPP;
	xii.langID = GetUserDefaultLangID();

	HRESULT hr = XLiveInitialize(&xii);
	/*if( hr == ERROR_GRAPHICS_INVALID_DISPLAY_ADAPTER )
	{
		hr = ERROR_GRAPHICS_INVALID_DISPLAY_ADAPTER;
	}
	else*/
	if( hr!=S_OK )
	{
		xii.dwFlags = XLIVE_INITFLAG_NO_AUTO_LOGON;
		hr = XLiveInitialize(&xii);
		if( hr!=S_OK )
		{
			dwErrCode = GetLastError();

			dwErrCode = XShowSigninUI(1,XSSUI_FLAGS_LOCALSIGNINONLY);
			if( dwErrCode != ERROR_SUCCESS )
				dwErrCode = 0;
			return;
		}
	}

	m_bInited = true;
#endif
}

bool XLiveShowerPC::RenderFrame()
{
#ifdef _ENABLE_XLIVE
	if( m_bInited )
	{
		HRESULT hr = XLiveRender();
		return hr == S_OK;
	}
#endif
	return false;
}

#endif
