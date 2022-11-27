#ifndef STOP_DEBUG
#ifndef _XBOX

#define _CRT_SECURE_NO_WARNINGS
#include "Debugger.h"
#include "Minidump.h"
#include <tlhelp32.h>
#include <assert.h>
#include <time.h>

#include "symbols.h"


#include "..\..\..\common_h\SetThreadName.h"





static char arrayPDBSearchPath[MAX_PATH + 200];
DebugSymbols _debugerSymbolEngine;

char appDebuger::currentDirectory[];
LPTOP_LEVEL_EXCEPTION_FILTER appDebuger::previousExceptionFilter = NULL;
int appDebuger::g_breakPointKeyCode1 = VK_F5;
int appDebuger::g_breakPointKeyCode2 = -1;
int appDebuger::g_breakPointKeyCode3 = -1;
DWORD appDebuger::debugThreadID = 0;
DWORD appDebuger::mainThreadID = 0;
DWORD appDebuger::dwFailedThreadID = 0;
char appDebuger::logFilePath[_MAX_PATH];
DWORD appDebuger::debugDumpFileLen;
DWORD appDebuger::deadlockDumpFileLen;


#pragma comment(linker, "/defaultlib:dbghelp.lib")



//---- конструктор ------------------------------------------------
appDebuger::appDebuger(const char * logFilePathForCopy)
{	

	InterlockedExchange((long *)&m_exitFlag, 0x0);
	
	debugThread = NULL; 

	assert (previousExceptionFilter == NULL && "appDebuger must be declared once !");
	previousExceptionFilter = SetUnhandledExceptionFilter(debugerUnhandledExceptionFilter);

	GetCurrentDirectoryA(MAX_PATH, appDebuger::currentDirectory);


	strcpy(arrayPDBSearchPath, appDebuger::currentDirectory);
	strcat_s(arrayPDBSearchPath, "\\modules\\debuginfo\\");

	strcpy(logFilePath, appDebuger::currentDirectory);
	strcat(logFilePath, "\\");
	strcat(logFilePath, logFilePathForCopy);

	char path[_MAX_PATH];
	BuildDebugDumpFilePath(path);
	debugDumpFileLen = GetFileLen(path);
	BuildDeadlockDumpFilePath(path);
	deadlockDumpFileLen = GetFileLen(path);

}

//---- деструктор ------------------------------------------------
appDebuger::~appDebuger()
{
	SetUnhandledExceptionFilter( previousExceptionFilter );

	stop();
}


void appDebuger::start(int breakPointKeyCode1, int breakPointKeyCode2, int breakPointKeyCode3)
{
	mainThreadID = GetCurrentThreadId();

	g_breakPointKeyCode1 = breakPointKeyCode1;
	g_breakPointKeyCode2 = breakPointKeyCode2;
	g_breakPointKeyCode3 = breakPointKeyCode3;

	InterlockedExchange((long *)&m_exitFlag, 0x0);

	params.hProcess = GetCurrentProcess();
	params.pId = GetCurrentProcessId();
	params.proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, params.pId);
	params.exitFlagPtr = (long *)&m_exitFlag;

	debugThreadID = 0;
	
	debugThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)debugerBreakpointHotkeyThread, &params, CREATE_SUSPENDED, &debugThreadID);
	XSetThreadName(debugThreadID, "Debugger - thread");
	ResumeThread(debugThread);
}

void appDebuger::stop()
{
	if (m_exitFlag != 0)
	{
		return;
	}

	InterlockedExchange((long *)&m_exitFlag, 0x1);


	WaitForSingleObject(debugThread, INFINITE);

	//OutputDebugStringA("done\n");
}

void appDebuger::BuildDebugDumpFilePath(char path[_MAX_PATH])
{
	strcpy(path, appDebuger::currentDirectory);
	strcat(path, "//storm3_crush_dump.txt");
}

void appDebuger::BuildDeadlockDumpFilePath(char path[_MAX_PATH])
{
	strcpy(path, appDebuger::currentDirectory);
	strcat(path, "//storm3_deadlock_dump.txt");
}

DWORD appDebuger::GetFileLen(const char * path)
{
	FILE * file = fopen(path, "rb");
	if(file)
	{
		fseek(file, 0, SEEK_END);
		DWORD size = ftell(file);
		fclose(file);
		return size;
	}
	return 0;
}

void appDebuger::LogDump()
{
	if(logFilePath)
	{
		char path[_MAX_PATH];
		BuildDebugDumpFilePath(path);
		CopyDump(path, debugDumpFileLen, logFilePath);
		BuildDeadlockDumpFilePath(path);
		CopyDump(path, debugDumpFileLen, logFilePath);
	}
}

void appDebuger::CopyDump(const char * src, DWORD from, const char * dst)
{
	FILE * file = fopen(src, "rb");
	if(file)
	{
		fseek(file, 0, SEEK_END);
		DWORD size = ftell(file);
		fseek(file, from, SEEK_SET);
		if(size > from)
		{
			FILE * to = fopen(dst, "a+b");
			if(to)
			{
				char buf[128];
				while(from < size)
				{					
					DWORD s = size - from;
					if(s > sizeof(buf)) s = sizeof(buf);					
					memset(buf, ' ', sizeof(buf));
					fread(buf, s, 1, file);
					fwrite(buf, s, 1, file);
					from += s;
				}
				fclose(to);
			}
		}
		fclose(file);
	}
}

void appDebuger::debuggerMakeThreadDumpContext(LPCONTEXT pContext, DWORD th32OwnerProcessID, DWORD th32ThreadID, FILE * resultFile)
{
	_debugerSymbolEngine.Init(arrayPDBSearchPath);

	HANDLE sThread = OpenThread(THREAD_ALL_ACCESS, false, th32ThreadID);
	HANDLE sProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, th32OwnerProcessID);

	if (resultFile)
	{
		
		fprintf(resultFile, "----------------------------------------------------------------------------------------\n\n");

		if (mainThreadID == th32ThreadID)
		{
			fprintf(resultFile, "*** Main thread *** \n");
		}

		if (dwFailedThreadID != 0 && dwFailedThreadID == th32ThreadID)
		{
			fprintf(resultFile, "*** Failed thread *** \n");
		}

		
		fprintf(resultFile, "Thread [0x%08X] callstack\n----------------------------------------------------------------------------------------\n", th32ThreadID);
	}



	if (mainThreadID == th32ThreadID)
	{
		OutputDebugStringA("*** Main thread *** \n");
	}


	if (dwFailedThreadID != 0 && dwFailedThreadID == th32ThreadID)
	{
		OutputDebugStringA("*** Failed thread *** \n");
	}


	OutputDebugStringA("----------------------------------------------------------------------------------------\n");


	STACKFRAME64 frame;
	memset( &frame, 0, sizeof(frame) );

	// Initialize the STACKFRAME structure for the first call.  This is only
	// necessary for Intel CPUs, and isn't mentioned in the documentation.
	frame.AddrPC.Offset       = pContext->Eip;
	frame.AddrPC.Mode         = AddrModeFlat;
	frame.AddrStack.Offset    = pContext->Esp;
	frame.AddrStack.Mode      = AddrModeFlat;
	frame.AddrFrame.Offset    = pContext->Ebp;
	frame.AddrFrame.Mode      = AddrModeFlat;

	int loop = 128;
	while ( loop >= 0)
	{
		if (!StackWalk64(IMAGE_FILE_MACHINE_I386, sProcess, sThread, &frame, pContext, NULL, NULL, NULL, NULL))
		{
			fprintf(resultFile, "-------------------------------------------------------------\n");
			break;
		}

		if (frame.AddrPC.Offset == frame.AddrReturn.Offset)
		{
			fprintf(resultFile, "-------------------------------------------------------------\n");
			break;
		}

		if (frame.AddrFrame.Offset == 0)
		{
			fprintf(resultFile, "+++++++++++++++++++++++++++++++++++++++++++++++++++\n");
			break;
		}

		const char * _sym = _debugerSymbolEngine.GetSymbolInfo((DWORD64)frame.AddrPC.Offset, (DWORD_PTR)frame.AddrFrame.Offset);
		OutputDebugStringA(_sym);
		OutputDebugStringA("\n");
		if (resultFile)
		{
			fprintf(resultFile, "%s\n", _sym);
		} 

		//std::string sym = GetSymbolInfo((DWORD64)frame.AddrPC.Offset);
		//printf ("%s\n", sym.c_str());

		loop--;
	}

	_debugerSymbolEngine.Close();

	CloseHandle(sProcess);
	CloseHandle(sThread);
}

void appDebuger::debuggerMakeThreadDump(DWORD th32OwnerProcessID, DWORD th32ThreadID, FILE * resultFile)
{
	HANDLE sThread = OpenThread(THREAD_ALL_ACCESS, false, th32ThreadID);
	CONTEXT ct;
	memset (&ct, 0, sizeof(&ct));
	ct.ContextFlags = CONTEXT_FULL;
	BOOL ctxRes = GetThreadContext(sThread, &ct);
	LPCONTEXT pContext = &ct;
	CloseHandle(sThread);

	debuggerMakeThreadDumpContext(pContext, th32OwnerProcessID, th32ThreadID, resultFile);
	
//
	
	

}

//Поток для мониторинга нажатия на кнопку
void appDebuger::debugerBreakpointHotkeyThread(LPVOID lpThreadParameter)
{
	DWORD dwDebugerThreadID = GetCurrentThreadId();

	debugerParams * params = (debugerParams *)lpThreadParameter;
	volatile long * exitFlag = params->exitFlagPtr;
	while (*exitFlag == false)
	{
		bool bNeedBreak = false;

		if (g_breakPointKeyCode1 >= 0 && g_breakPointKeyCode2 >= 0 && g_breakPointKeyCode3 >= 0)
		{
			if (GetKeyState(g_breakPointKeyCode1) < 0 && GetKeyState(g_breakPointKeyCode2) < 0 && GetKeyState(g_breakPointKeyCode3) < 0)
			{
				bNeedBreak = true;
			}
		} else
		{
			if (g_breakPointKeyCode1 >= 0 && g_breakPointKeyCode2 >= 0)
			{
				if (GetKeyState(g_breakPointKeyCode1) < 0 && GetKeyState(g_breakPointKeyCode2) < 0)
				{
					bNeedBreak = true;
				}
			} else
			{
				if (g_breakPointKeyCode1 >= 0)
				{
					if (GetKeyState(g_breakPointKeyCode1) < 0)
					{
						bNeedBreak = true;
					}
				}
			}
		}




		if (bNeedBreak)
		{
			//printf ("DebugBreak\n");
			pauseApplication(dwDebugerThreadID);
		}
		Sleep(80);
	}

	OutputDebugStringA ("debugger thread closed\n");
}

void appDebuger::pauseApplication(DWORD debugerThreadID)
{
	char szReportPath[_MAX_PATH];
	BuildDeadlockDumpFilePath(szReportPath);

	FILE * dumpFile = fopen(szReportPath, "a+");

	char tmpbuf[256];
	__time64_t ltime;
	_time64(&ltime);
	struct tm* today = _localtime64(&ltime);
	strftime(tmpbuf, sizeof(tmpbuf), "\n\n\n============================================================================================\n[%d %B %Y, %H:%M:%S]\n============================================================================================\n", today);

	OutputDebugStringA(tmpbuf);

	if (dumpFile)
	{
		_debugerSymbolEngine.PrintPDBSearchPath(dumpFile);
		fprintf(dumpFile, "%s", tmpbuf);
	}
	


	DWORD threadsCount = 0;
	HANDLE threads[2048];

	DWORD dwID = GetCurrentProcessId();
	HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (h != INVALID_HANDLE_VALUE)
	{
		THREADENTRY32 te;
		te.dwSize = sizeof(te);
		if (Thread32First(h, &te))
		{
			do
			{
				if (te.th32OwnerProcessID != dwID)
				{
					continue;
				}

				if (te.dwSize >= FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) + sizeof(te.th32OwnerProcessID))
				{
					if (te.th32ThreadID != debugerThreadID)
					{
						HANDLE applicationThread = OpenThread(THREAD_ALL_ACCESS, false, te.th32ThreadID);
						SuspendThread(applicationThread);

						Sleep(100);

						threads[threadsCount] = applicationThread;
						threadsCount++;

						assert(threadsCount < 2048);

						
						debuggerMakeThreadDump(te.th32OwnerProcessID, te.th32ThreadID, dumpFile);
					}
				}
				te.dwSize = sizeof(te);
			} while (Thread32Next(h, &te));
		}
		CloseHandle(h);
	}

	if (dumpFile)
	{
		fclose(dumpFile);
	}

	LogDump();

	int res = MessageBoxA(NULL, "Application now paused\nExecution stack of all threads dumped to file 'storm3_deadlock_dump.txt'\n\nDo you want to continue execution ?\n\n(Yes) - Continue execution\n(No) - Exit application", "Debugger pause", MB_ICONQUESTION | MB_YESNO);
	if (res != IDYES)
	{
		exit(0);
	}

	for (DWORD i = 0; i < threadsCount; i++)
	{
		HANDLE h = threads[i];
		ResumeThread(h);
		CloseHandle(h);
	}

	threadsCount = 0;

}

const char* appDebuger::GetExceptionString( DWORD dwCode )
{
#define EXCEPTION( x ) case EXCEPTION_##x: return #x;

	switch ( dwCode )
	{
		EXCEPTION( ACCESS_VIOLATION )
		EXCEPTION( DATATYPE_MISALIGNMENT )
		EXCEPTION( BREAKPOINT )
		EXCEPTION( SINGLE_STEP )
		EXCEPTION( ARRAY_BOUNDS_EXCEEDED )
		EXCEPTION( FLT_DENORMAL_OPERAND )
		EXCEPTION( FLT_DIVIDE_BY_ZERO )
		EXCEPTION( FLT_INEXACT_RESULT )
		EXCEPTION( FLT_INVALID_OPERATION )
		EXCEPTION( FLT_OVERFLOW )
		EXCEPTION( FLT_STACK_CHECK )
		EXCEPTION( FLT_UNDERFLOW )
		EXCEPTION( INT_DIVIDE_BY_ZERO )
		EXCEPTION( INT_OVERFLOW )
		EXCEPTION( PRIV_INSTRUCTION )
		EXCEPTION( IN_PAGE_ERROR )
		EXCEPTION( ILLEGAL_INSTRUCTION )
		EXCEPTION( NONCONTINUABLE_EXCEPTION )
		EXCEPTION( STACK_OVERFLOW )
		EXCEPTION( INVALID_DISPOSITION )
		EXCEPTION( GUARD_PAGE )
		EXCEPTION( INVALID_HANDLE )
	}

	// If not one of the "known" exceptions, try to get the string
	// from NTDLL.DLL's message table.

	static char szBuffer[512] = { 0 };

	FormatMessageA( FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE, GetModuleHandleA( "NTDLL.DLL"), dwCode, 0, szBuffer, sizeof( szBuffer ), 0);

	return szBuffer;
}


BOOL appDebuger::GetLogicalAddress(PVOID addr, char * szModule, DWORD len, DWORD& section, DWORD& offset)
{
	MEMORY_BASIC_INFORMATION mbi;

	if (!VirtualQuery( addr, &mbi, sizeof(mbi)))
	{
		return FALSE;
	}

	DWORD hMod = (DWORD)mbi.AllocationBase;

	if (!GetModuleFileNameA( (HMODULE)hMod, szModule, len))
	{
		return FALSE;
	}

	// Point to the DOS header in memory
	PIMAGE_DOS_HEADER pDosHdr = (PIMAGE_DOS_HEADER)hMod;

	// From the DOS header, find the NT (PE) header
	PIMAGE_NT_HEADERS pNtHdr = (PIMAGE_NT_HEADERS)(hMod + pDosHdr->e_lfanew);

	PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION( pNtHdr );

	DWORD rva = (DWORD)addr - hMod; // RVA is offset from module load address

	// Iterate through the section table, looking for the one that encompasses
	// the linear address.
	for (unsigned i = 0; i < pNtHdr->FileHeader.NumberOfSections; i++, pSection++ )
	{
		DWORD sectionStart = pSection->VirtualAddress;
		DWORD sectionEnd = sectionStart + max(pSection->SizeOfRawData, pSection->Misc.VirtualSize);

		// Is the address in this section???
		if ( (rva >= sectionStart) && (rva <= sectionEnd) )
		{
			// Yes, address is in the section.  Calculate section and offset,
			// and store in the "section" & "offset" params, which were
			// passed by reference.
			section = i+1;
			offset = rva - sectionStart;
			return TRUE;
		}
	}

	return FALSE;   // Should never get here!
}


//Обработчик исключений
LONG WINAPI appDebuger::debugerUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo)
{
	miniDumpWriter::writeMiniDump(pExceptionInfo, "storm3_crush_dump.dmp");


	_debugerSymbolEngine.Init(arrayPDBSearchPath);

	char szCrashReportPath[_MAX_PATH];
	BuildDebugDumpFilePath(szCrashReportPath);

	FILE * dumpFile = fopen(szCrashReportPath, "a+");

	dwFailedThreadID = GetCurrentThreadId();

	char tmpbuf[256];
	__time64_t ltime;
	_time64(&ltime);
	struct tm* today = _localtime64(&ltime);
	strftime(tmpbuf, sizeof(tmpbuf), "\n\n\n============================================================================================\n[%d %B %Y, %H:%M:%S]\n============================================================================================\n", today);

	PEXCEPTION_RECORD pExceptionRecord = pExceptionInfo->ExceptionRecord;
	OutputDebugStringA(tmpbuf);

	if (dumpFile)
	{
		_debugerSymbolEngine.PrintPDBSearchPath(dumpFile);

		const char * exceptionStr = GetExceptionString(pExceptionRecord->ExceptionCode);

		fprintf(dumpFile, "%s", tmpbuf);
		fprintf(dumpFile, "Exception info:\n");
		fprintf(dumpFile, "   thread : 0x%08X\n", dwFailedThreadID);
		fprintf(dumpFile, "   type : '%s'\n", exceptionStr);
		fprintf(dumpFile, "   code : 0x%08X\n", pExceptionRecord->ExceptionCode);


		// Now print information about where the fault occured
		static char szFaultingModule[MAX_PATH];
		DWORD section, offset;
		GetLogicalAddress(  pExceptionRecord->ExceptionAddress, szFaultingModule, sizeof( szFaultingModule ), section, offset );

		fprintf(dumpFile, "   fault address : %p %02X:%08X\n                   %s\n\n", pExceptionRecord->ExceptionAddress, section, offset, szFaultingModule);

	}


//	PEXCEPTION_RECORD pExceptionRecord = pExceptionInfo->ExceptionRecord;

	//pExceptionRecord

/*
	// First print information about the type of fault
	_tprintf(   _T("Exception code: %08X %s\r\n"),
		pExceptionRecord->ExceptionCode,
		GetExceptionString(pExceptionRecord->ExceptionCode) );

	// Now print information about where the fault occured
	TCHAR szFaultingModule[MAX_PATH];
	DWORD section, offset;
	GetLogicalAddress(  pExceptionRecord->ExceptionAddress,
		szFaultingModule,
		sizeof( szFaultingModule ),
		section, offset );
*/

	//PCONTEXT pCtx = pExceptionInfo->ContextRecord;

	//pExceptionInfo->ContextRecord->


	//OutputDebugStringA(tmpbuf);


	DWORD dwID = GetCurrentProcessId();
	HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (h != INVALID_HANDLE_VALUE)
	{
		THREADENTRY32 te;
		te.dwSize = sizeof(te);
		if (Thread32First(h, &te))
		{
			do
			{
				if (te.th32OwnerProcessID != dwID)
				{
					continue;
				}

				if (te.dwSize >= FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) + sizeof(te.th32OwnerProcessID))
				{
					if (dwFailedThreadID == te.th32ThreadID)
					{
						debuggerMakeThreadDumpContext(pExceptionInfo->ContextRecord, te.th32OwnerProcessID, dwFailedThreadID, dumpFile);
					} else
					{
						//Все потоки приложения, усыпляем...
						HANDLE applicationThread = OpenThread(THREAD_ALL_ACCESS, false, te.th32ThreadID);
						SuspendThread(applicationThread);
						debuggerMakeThreadDump(te.th32OwnerProcessID, te.th32ThreadID, dumpFile);
						CloseHandle(applicationThread);
					}

					
				}
				te.dwSize = sizeof(te);
			} while (Thread32Next(h, &te));
		}
		CloseHandle(h);
	}


	if (dumpFile)
	{
		fclose(dumpFile);
	}

	LogDump();


	_debugerSymbolEngine.Close();

	if ( previousExceptionFilter )
	{
		return previousExceptionFilter(pExceptionInfo);
	} else
	{
		return EXCEPTION_CONTINUE_SEARCH;
	}
}


#undef _CRT_SECURE_NO_WARNINGS

#endif
#endif