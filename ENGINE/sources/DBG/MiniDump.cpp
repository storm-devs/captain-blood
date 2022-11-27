#ifndef STOP_DEBUG
#ifndef _XBOX

#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <dbghelp.h>
#include <tchar.h>
#include "MiniDump.h"


// based on dbghelp.h
typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
										 CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
										 CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
										 CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam
										 );




miniDumpWriter::miniDumpWriter()
{

}

miniDumpWriter::~miniDumpWriter()
{

}


miniDumpWriter::minudumpRes miniDumpWriter::writeMiniDump(struct _EXCEPTION_POINTERS *pExceptionInfo, const char * minidumpFileName)
{
	// firstly see if dbghelp.dll is around and has the function we need
	// look next to the EXE first, as the one in System32 might be old 
	// (e.g. Windows 2000)
	HMODULE hDll = NULL;
	char szDbgHelpPath[_MAX_PATH];

	if (GetModuleFileNameA( NULL, szDbgHelpPath, _MAX_PATH ))
	{
		char *pSlash = strrchr( szDbgHelpPath, '\\');
		if (pSlash)
		{
			strcpy(pSlash+1, "DBGHELP.DLL");
			hDll = ::LoadLibraryA( szDbgHelpPath );
		}
	}

	if (hDll==NULL)
	{
		// load any version we can
		hDll = ::LoadLibraryA( "DBGHELP.DLL" );
	}

	LPCTSTR szResult = NULL;

	if (hDll)
	{
		MINIDUMPWRITEDUMP pDump = (MINIDUMPWRITEDUMP)::GetProcAddress( hDll, "MiniDumpWriteDump" );
		if (pDump)
		{
			//remove ro/system attributes...
			SetFileAttributesA(minidumpFileName, FILE_ATTRIBUTE_NORMAL);

			// create the file
			HANDLE hFile = ::CreateFileA(minidumpFileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );


			if (hFile != INVALID_HANDLE_VALUE)
			{
				_MINIDUMP_EXCEPTION_INFORMATION ExInfo;

				ExInfo.ThreadId = ::GetCurrentThreadId();
				ExInfo.ExceptionPointers = pExceptionInfo;
				ExInfo.ClientPointers = NULL;

				// write the dump
				BOOL bOK = pDump( GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, NULL, NULL );

				::CloseHandle(hFile);
			} else
			{
				return miniDumpWriter::MDR_CANT_OPEN_FILE;
			}

		}
		else
		{
			return miniDumpWriter::MDR_DBGHELP_TOO_OLD;
		}
	}
	else
	{
		return miniDumpWriter::MDR_DBGHELP_NOT_FOUND;
	}

	return miniDumpWriter::MDR_OK;
}

#undef _CRT_SECURE_NO_WARNINGS

#endif
#endif