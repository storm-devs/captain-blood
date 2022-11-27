#ifndef STOP_DEBUG
#ifndef _XBOX

#ifndef _APP_INTERNAL_DEBUGER_HEADER_
#define _APP_INTERNAL_DEBUGER_HEADER_


#include <windows.h>
#include <dbghelp.h>
#include <stdio.h>



struct debugerParams
{
	HANDLE proc;
	HANDLE hProcess;
	DWORD pId;
	volatile long * exitFlagPtr;
};



class appDebuger
{
public:
	appDebuger(const char * logFilePathForCopy);
	~appDebuger();


public:
	void start(int breakPointKeyCode1, int breakPointKeyCode2 = -1, int breakPointKeyCode3 = -1);
	void stop();


public:
	static void debuggerMakeThreadDump(DWORD th32OwnerProcessID, DWORD th32ThreadID, FILE * resultFile);
	static void debuggerMakeThreadDumpContext(LPCONTEXT pContext, DWORD th32OwnerProcessID, DWORD th32ThreadID, FILE * resultFile);

	static LONG WINAPI debugerUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo);
	static void debugerBreakpointHotkeyThread(LPVOID lpThreadParameter);


	static BOOL GetLogicalAddress(PVOID addr, char * szModule, DWORD len, DWORD& section, DWORD& offset);
	static const char* GetExceptionString( DWORD dwCode );
	static void pauseApplication(DWORD debugerThreadID);

	static void BuildDebugDumpFilePath(char path[_MAX_PATH]);
	static void BuildDeadlockDumpFilePath(char path[_MAX_PATH]);
	static DWORD GetFileLen(const char * path);
	static void LogDump();
	static void CopyDump(const char * src, DWORD from, const char * dst);

private:
	debugerParams params;
	volatile long m_exitFlag;
	HANDLE debugThread;
	static char logFilePath[_MAX_PATH];
	static DWORD debugDumpFileLen;
	static DWORD deadlockDumpFileLen;

public:
	static DWORD mainThreadID;
	static DWORD debugThreadID;
	static DWORD dwFailedThreadID;

	static int g_breakPointKeyCode1;
	static int g_breakPointKeyCode2;
	static int g_breakPointKeyCode3;
	static char currentDirectory[MAX_PATH+2];

public:
	static LPTOP_LEVEL_EXCEPTION_FILTER previousExceptionFilter;
};


#endif

#endif
#endif