#ifndef STORM_CRT_LIB
#define STORM_CRT_LIB


#if (_MSC_VER < 1400)
//Если не visual studio 8 то использовать небезопасные crt функции
#define USE_UNSECURED_CRT
#endif


#include "d_types.h"


#ifdef USE_UNSECURED_CRT


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdarg.h>


#define crt_sscanf sscanf

inline void crt_splitpath(const char * path,
                   char * drive, dword driveSizeInCharacters,
                   char * dir,   dword dirSizeInCharacters,
                   char * fname, dword nameSizeInCharacters,
                   char * ext,   dword extSizeInBytes)
{
 _splitpath(path, drive, dir, fname, ext);
}


inline dword crt_strlen (const char* str1)
{
  return (dword)strlen(str1);
}


inline long crt_stricmp(const char* str1, const char* str2)
{
  return stricmp(str1, str2);
}

inline long crt_strnicmp(const char* str1, const char* str2, dword size)
{
  return strnicmp(str1, str2, size);
}



inline void crt_strlwr(char* str1)
{
  strlwr(str1);
}

inline void crt_strupr(char* str1)
{
  strupr(str1);
}

inline long crt_flushall()
{
  return flushall();
}

inline long crt_vsnprintf(char *buffer, dword count, const char *format, va_list argptr)
{
  return _vsnprintf (buffer, count, format, argptr);
}

inline long crt_snprintf(char* buffer, dword count, const char *format, ...)
{
  va_list args;
  va_start(args, format);
  long strlen = crt_vsnprintf(buffer, count, format, args);
  va_end(args);

  return strlen;
}


inline void crt_localtime64(struct tm* _tm, const __time64_t *timer)
{
  struct tm* curTime = _localtime64(timer);
  memcpy (_tm, curTime, sizeof(struct tm));
}

inline void crt_strcpy (char* dest, dword destSize, const char* source)
{
  strcpy(dest, source);
}

inline void crt_strncpy (char* dest, dword destSize, const char* source, dword count)
{
  strncpy(dest, source, count);
}


inline void crt_strcat (char* dest, dword destSize, const char* source)
{
  strcat(dest, source);
}


inline const char* crt_strstr (const char* s1, const char* s2)
{
 return strstr(s1, s2);
}


inline FILE* crt_fopen (const char* filename, const char* mode)
{
 return fopen(filename, mode);;
}


#else


#define _CRT_SECURE_CPP_OVERLOAD_SECURE_NAMES 1


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdarg.h>

#define crt_sscanf sscanf_s


inline void crt_splitpath(const char * path,
                   char * drive, dword driveSizeInCharacters,
                   char * dir,   dword dirSizeInCharacters,
                   char * fname, dword nameSizeInCharacters,
                   char * ext,   dword extSizeInBytes)
{
 _splitpath_s(path, drive, driveSizeInCharacters, dir, dirSizeInCharacters, fname, nameSizeInCharacters, ext, extSizeInBytes);
}                   




inline dword crt_strlen (const char* str1)
{
  return (dword)strlen(str1);
}


inline long crt_stricmp(const char* str1, const char* str2)
{
  return _stricmp(str1, str2);
}

inline long crt_strnicmp(const char* str1, const char* str2, dword size)
{
  return _strnicmp(str1, str2, size);
}


inline void crt_strlwr(char* str1)
{
  _strlwr_s(str1, crt_strlen(str1) + 1);
}

inline void crt_strupr(char* str1)
{
  _strupr_s(str1, crt_strlen(str1) + 1);
}

inline long crt_flushall()
{
  return _flushall();
}

inline long crt_vsnprintf(char *buffer, dword count, const char *format, va_list argptr)
{
  return _vsnprintf_s (buffer, count, count-1, format, argptr);
}

inline long crt_snprintf(char* buffer, dword count, const char *format, ...)
{
  va_list args;
  va_start(args, format);
  long strlen = crt_vsnprintf(buffer, count, format, args);
  va_end(args);

  return strlen;
}

inline void crt_localtime64(struct tm* _tm, const __time64_t *timer)
{
  _localtime64_s(_tm, timer); 

}

inline void crt_strcpy (char* dest, dword destSize, const char* source)
{
  strcpy_s(dest, destSize, source);
}

inline void crt_strncpy (char* dest, dword destSize, const char* source, dword count)
{
  strncpy_s(dest, destSize, source, count);
}


inline void crt_strcat (char* dest, dword destSize, const char* source)
{
  strcat_s(dest, destSize, source);
}


inline const char* crt_strstr (const char* s1, const char* s2)
{
 return strstr(s1, s2);
}

inline FILE* crt_fopen (const char* filename, const char* mode)
{
 FILE* ptr = 0;
 fopen_s(&ptr, filename, mode);
 return ptr;
}




#endif




#endif
