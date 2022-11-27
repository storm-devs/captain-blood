

#ifndef _CoreBase_h_
#define _CoreBase_h_

#include "..\..\common_h\core.h"
#include "..\..\common_h\corecmds.h"

void CoreLogOutData(const char * format, const void * data);

__forceinline void _cdecl CoreLogOut(const char * format, ...)
{
	CoreLogOutData(format, &format + 1);
}

#endif

