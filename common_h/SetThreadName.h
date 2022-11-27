#ifndef _DEBUGER_THREAD_SET_NAME_
#define _DEBUGER_THREAD_SET_NAME_


#ifndef STOP_DEBUG

typedef struct tagXTHREADNAME_INFO {
	DWORD dwType;     // Must be 0x1000
	LPCSTR szName;    // Pointer to name (in user address space)
	DWORD dwThreadID; // Thread ID (-1 for caller thread)
	DWORD dwFlags;    // Reserved for future use; must be zero
} XTHREADNAME_INFO;


__forceinline void XSetThreadName( DWORD dwThreadID, LPCSTR szThreadName )
{
	XTHREADNAME_INFO info;

	info.dwType = 0x1000;
	info.szName = szThreadName;
	info.dwThreadID = dwThreadID;
	info.dwFlags = 0;

	__try
	{
		RaiseException( 0x406D1388, 0, sizeof(info)/sizeof(DWORD), (DWORD *)&info );
	}
	__except( GetExceptionCode()==0x406D1388 ? EXCEPTION_CONTINUE_EXECUTION : EXCEPTION_EXECUTE_HANDLER )
	{
	}
}

#else

__forceinline void XSetThreadName( DWORD dwThreadID, LPCSTR szThreadName )
{

}

#endif


#endif