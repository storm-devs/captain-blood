#ifndef DEFINES_HPP
#define DEFINES_HPP

// includes


// Constants

#define PI	3.14159265358979323846f
#define PIm2	(PI * 2.0f)
#define PId2	(PI / 2.0f)
#define PId4	(PI / 4.0f)


#ifndef _XBOX

#define RDTSC_B(x)	{ _asm rdtsc _asm mov x,eax }
#define RDTSC_E(x)	{ _asm rdtsc _asm sub eax,x _asm mov x,eax }

#else

#define RDTSC_B(x)  {  x = (unsigned long)__mftb();  }
#define RDTSC_E(x) {  unsigned long y = (unsigned long)__mftb();  x = (y - x); }

#endif





// Defines

#define ARRSIZE(ar)		(sizeof(ar)/sizeof(ar[0]))

#ifdef RGB
	#undef RGB
#endif

#ifdef DELETE
	#undef DELETE
#endif

#ifdef SQR
	#undef SQR
#endif

#ifdef _XBOX
#define IS_XBOX(a,b)		a
#else
#define IS_XBOX(a,b)		b
#endif

#define FTOL(l,f)		{ __asm fld dword ptr [f] __asm fistp dword ptr l }
#define GET_DATA(x,p)		{ memcpy(&(x),p,sizeof(x));p+=sizeof(x); }
#define FREE(x)			{ if (x) free(x); x=0; }
#define DELETE(x)		{ if ((x)) delete (x); (x)=0; }
#define DELETE_ARRAY(x)		{ if (x) delete []x; x=0; }
#define DELETE_ENTITY(x)	{ if (_CORE_API->ValidateEntity(&x)) _CORE_API->DeleteEntity(x); }
#define RELEASE(x)		{ if (x) x->Release(); x = 0; }
#define FORCERELEASE(x)		{ if (x) x->ForceRelease(); x = 0; }
#define SQR(a)			( (a)*(a) )
#define FRAND(x)		( (x) * (float)rand()/(float)RAND_MAX  )
#define SIGN(x)			( (x)<0 ? -1.0f : 1.0f )
#define SIGNZ(x)		( (x)==0.0f ? 0.0f : ((x)<0.0f ? -1.0f : 1.0f) )
#define MAX(x,y)		( ((x) > (y)) ? (x) : (y) )
#define MIN(x,y)		( ((x) < (y)) ? (x) : (y) )
#define MAX3(x,y,z)		( ((x) > (y)) ? ((x) > (z) ? (x) : (z)) : ((y) > (z) ? (y) : (z)) )
#define MIN3(x,y,z)		( ((x) < (y)) ? ((x) < (z) ? (x) : (z)) : ((y) < (z) ? (y) : (z)) )
#define CLAMP(x)		( ((x) > 1.0f) ? 1.0f : (((x) < 0.0f) ? 0.0f : (x)) )
#define RGB(r,g,b)		( DWORD(b)|(DWORD(g)<<8L)|(DWORD(r)<<16L) )
#define RGB565(r,g,b)		WORD( ((DWORD(b) >> 3L)) | ((DWORD(g) >> 2L) << 5L) | ((DWORD(r) >> 3L) << 11L) )
#define RGB1555(r,g,b)		WORD( ((DWORD(b) >> 3L)) | ((DWORD(g) >> 3L) << 5L) | ((DWORD(r) >> 3L) << 10L) )
#define ARGB1555(a,r,g,b)	WORD( (DWORD(a&1L) << 15L) | ((DWORD(b) >> 3L)) | ((DWORD(g) >> 3L) << 5L) | ((DWORD(r) >> 3L) << 10L) )
#define ARGB(a,r,g,b)		( DWORD(b)|(DWORD(g)<<8L)|(DWORD(r)<<16L)|(DWORD(a)<<24L) )
#define RGBA(r,g,b,a)		( ARGB(a,r,g,b) )
#define ZERO(x)			{ ZeroMemory(&x,sizeof(x)); }
#define ZERO2(x,y)		{ ZERO(x) ZERO(y) }
#define ZERO3(x,y,z)		{ ZERO2(x,y) ZERO(z) }
#define ZERO4(x,y,z,a)		{ ZERO2(x,y) ZERO2(z,a) }
#define PZERO(x,size)		{ ZeroMemory(x,size); }
#define COPY_STRING(a,b)	{ long slen = strlen(b)+1; a = NEW char[slen]; if (a) crt_strcpy(a,slen, b); }
#define COLOR2VECTOR(a)		CVECTOR(float((a&0xFF0000)>>0x10),float((a&0xFF00)>>0x8),float(a&0xFF));
#define COPY_STRINGLEN(a,b,l)	{ a = NEW char[l+1]; if (a) crt_strcpy(a,l+1, b); }

// game defines
#define METERS2KNOTS(x)		((x) * 3600.0f / 1853.0f)
#define KNOTS2METERS(x)		((x) * 1853.0f / 3600.0f)

#define SMALL_DELTA(a,b)	(fabsf((a) - (b)) < 1e-20f)
#define IS_FZERO(a)		(fabsf(a) < 1e-20f)

template<class T> __forceinline void Swap(T & t1, T & t2) { T tmp; tmp = t1; t1 = t2; t2 = tmp; }
template<class T> __forceinline T Clamp(T t) { if (t < (T)0) return (T)0; if (t > (T)1) return (T)1; return t; }
template<class T> __forceinline T Bring2Range(T Min1, T Max1, T Min2, T Max2, T Value) { if (Value < Min2) Value = Min2; if (Value > Max2) Value = Max2; float Delta = float(Value - Min2) / float(Max2 - Min2); return Min1 + Delta * (Max1 - Min1); }
template<class T> __forceinline T Bring2RangeNoCheck(T Min1, T Max1, T Min2, T Max2, T Value) { float Delta = float(Value - Min2) / float(Max2 - Min2); return Min1 + Delta * (Max1 - Min1); }
template<class T> __forceinline T Min(T t1, T t2) { return ((t1 < t2) ? t1 : t2); }
template<class T> __forceinline T Max(T t1, T t2) { return ((t1 > t2) ? t1 : t2); }
template<class T> __forceinline T Sqr(T t1) { return (t1 * t1); }


template<class T> __forceinline T * AlignPtr(T * ptr, long mask = 0xf)
{
	byte * tmp = (((byte *)ptr - (byte *)0 + mask) & ~mask) + (byte *)0;
	return (T *)tmp;
}

#define AssertAling16(ptr)  Assert((((byte *)ptr - (byte *)0) & 0xf) == 0);

#ifdef _XBOX
	template<> __forceinline float Min(float t1, float t2) { return __fself(t1 - t2, t2, t1); }
	template<> __forceinline float Max(float t1, float t2) { return __fself(t1 - t2, t1, t2); }
	template<> __forceinline float Clamp(float t) { return Min(1.0f, Max(0.0f, t));  }
#endif

template<class T> __forceinline void GetMinMax(T value, T & min, T & max)
{
	if (value < min)
		min = value;
	if (value > max)
		max = value;
}

#ifdef _XBOX
	template<> __forceinline void GetMinMax(float value, float & min, float & max)
	{
		min = Min(value, min);
		max = Max(value, max);
	}
#endif

template<class T> __forceinline T MinMax(T min, T max, T value) { return Max(Min(max, value), min); }

#include "Math3D.h"
//#include "inlines.h"

#endif