#ifndef STOP_DEBUG
#ifndef _XBOX

#ifndef _APP_MINIDUMP_WRITER_HEADER_
#define _APP_MINIDUMP_WRITER_HEADER_




class miniDumpWriter
{

public:

	enum minudumpRes
	{
		MDR_OK = 0,

		MDR_CANT_OPEN_FILE = 1,
		MDR_DBGHELP_TOO_OLD = 2,
		MDR_DBGHELP_NOT_FOUND = 3,

		MDR_FORCE_DWORD = 0x7fffffff
	};

	miniDumpWriter();
	~miniDumpWriter();


	static minudumpRes writeMiniDump(struct _EXCEPTION_POINTERS *pExceptionInfo, const char * minidumpFileName);

};


#endif

#endif
#endif