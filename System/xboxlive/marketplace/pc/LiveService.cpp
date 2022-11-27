/*#ifndef _XBOX

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

//#include <winlive.h>
//#include <Xonline.h>
//#include <xtl.h>

#include <stdio.h>

#include "LiveService.h"

//#pragma comment(lib,"xlive.lib")

LiveService::LiveService()
{
	m_pcErrorText[0] = '\0';
}

LiveService::~LiveService()
{
	Release();
}

bool LiveService::Init()
{*/
/*	DWORD dwErrCode;

	XLIVE_INITIALIZE_INFO info;
	ZeroMemory(&info,sizeof(info));
	info.cbSize = sizeof(info);
	info.pD3D = pD3D;
	info.pD3DPP = pD3DPP;
	HRESULT hr = XLiveInitialize(&info);
	if( hr!=S_OK )
	{
		info.dwFlags = XLIVE_INITFLAG_NO_AUTO_LOGON;
		hr = XLiveInitialize(&info);
		if( hr!=S_OK )
		{
			dwErrCode = GetLastError();
			SetError("LiveService::Init() : Error! Failed XLive initialize");
			//return false;
		}
	}

	WCHAR wcServerPath[1024];
	DWORD dwServerPathLength = sizeof(wcServerPath);
	dwErrCode = XStorageBuildServerPath(0, XSTORAGE_FACILITY_PER_TITLE, NULL, 0, L"*.*", wcServerPath, &dwServerPathLength );
	if( dwErrCode != ERROR_SUCCESS )
	{
		SetError("LiveService::Init() : Error! BuildServerPath failed");
	}
*/
/*	return true;
}

bool LiveService::Frame(float dltTime)
{
	return true;
}

void LiveService::Release()
{
}

void LiveService::SetError(const char * frmt,...)
{
	va_list args;
	va_start(args, frmt);
	_vsnprintf_s(m_pcErrorText, sizeof(m_pcErrorText), sizeof(m_pcErrorText-1), frmt, args);
	va_end(args);
}

#endif
*/