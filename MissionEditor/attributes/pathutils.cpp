#include <windows.h>
#include <string.h>
#include "..\..\common_h\Storm3Crt.h"

static char BigBuffer[MAX_PATH];
static char Path1[MAX_PATH];
static char Path2[MAX_PATH];


const char* MakeRelativePath (const char* fullpath, const char* relativeto)
{
	crt_strncpy (Path1, MAX_PATH, fullpath, MAX_PATH-1);
	crt_strncpy (Path2, MAX_PATH, relativeto, MAX_PATH-1);
	crt_strlwr (Path1);
	crt_strlwr (Path2);

	
  int Len = strlen(Path2);
  if (Len)
		if (Path2[Len-1] != '\\') crt_strcat (Path2, MAX_PATH, "\\");



	char* result = strstr(Path1, Path2);
	if (result != Path1)
	{
		BigBuffer[0] = 0;
	} else
		{
			crt_strncpy (BigBuffer, MAX_PATH, Path1+strlen(Path2), MAX_PATH-1);
		}

	return BigBuffer;
}