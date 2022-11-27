//============================================================================================
// Spirenkov Maxim, 2003
//============================================================================================
// Math3D
//============================================================================================

#ifndef _Math3D_h_
#define _Math3D_h_

#include "d_types.h"

#ifndef _XBOX
#ifdef SIMD_ENABLE
#include <xmmintrin.h>
#endif
#endif

#define mathinline __forceinline
#define mathcall __fastcall
#define mathalign __declspec(align(16)) 

#include "Math3D\Vector.h"
#include "Math3D\Vector4.h"
#include "Math3D\Matrix.h"
#include "Math3D\Quaternion.h"
#include "Math3D\Color.h"
#include "Math3D\Plane.h"
#include "Math3D\Box.h"
#include "Math3D\Line.h"
#include "Math3D\Sphere.h"
#include "Math3D\Triangle.h"
#include "Math3D\Capsule.h"


///Быстрое приведение числа с плавающей точкой к целому с отбрасыванием дробной части
mathinline long mathcall fftol(float f)
{
#ifndef _XBOX
	long l;
	static const float cnt[2] = {-0.4999999f, 0.4999999f};
	_asm
	{
		mov		eax, f
		fld		f
		shr		eax, 31
		fadd	dword ptr [cnt + eax*4]
		fistp	l
	}
	return l;
#else
	return long(f + ((f >= 0) ? 0.4999999f : -0.4999999f));
#endif
}

///Быстрое приведение числа с плавающей точкой к целому с округлением к ближайшему
mathinline long mathcall fftoi(float f)
{
#ifndef _XBOX
	long l;
	_asm
	{
		fld	f
		fistp	l
	}
	return l;
#else
	return long(f);
#endif
}

//Преобразование float в инт с округлением к ближайшем уцелому. Целое число не должно выходить за пределы 23-х бит
mathinline int mathcall float2int23(float x)
{
	const int magic = (150 << 23) | (1 << 22);
	x += *(float*)&magic;
	return *(int*)&x-magic;
}

///Fast floor
mathinline long mathcall ffloor(float f)
{
#ifndef _XBOX
	long l;
	static const float c = -0.5f;
	_asm
	{
		fld	f
		fadd	c
		fistp	l
	}
	return l;
#else
	return floorf(f);
#endif
}

///Fast ceil
mathinline long mathcall fceil(float f)
{
#ifndef _XBOX
	long l;
	static const float c = 0.5f;
	_asm
	{
		fld	f
		fadd	c
		fistp	l
	}
	return l;
#else
	return fceil(f);
#endif
}

///Fast fasb in memory
mathinline float & mathcall ffabs(float & f)
{
	*(unsigned long *)&f &= 0x7fffffff;
	return f;
}

//Возвести в квадрат
mathinline float mathcall sqrf(float f)
{
	return f*f;
}

//Случайное число
mathinline float mathcall Rnd(float max = 1.0f)
{
	return rand()*(max*(1.0f/RAND_MAX));
}

//Случайное число
mathinline float mathcall RRnd(float min, float max)
{
	return min + rand()*((max - min)*(1.0f/RAND_MAX));
}

//Ограничить float
mathinline float mathcall Clampf(float v, float min = 0.0f, float max = 1.0f)
{
	if(v < min) v = min;
	if(v > max) v = max;
	return v;
}

//Ограничить float
mathinline float mathcall Clampfr(float & v, float min = 0.0f, float max = 1.0f)
{
	if(v < min) v = min;
	if(v > max) v = max;
	return v;
}


//Привести угол к диапазону 0..2PI
mathinline float mathcall NormAngle2PI(float angle)
{
	static const float pi = 3.14159265358979323846f;
	if(angle >= 0.0f && angle <= 2*pi) return angle;
	return (angle/(2.0f*pi) - long(angle/(2.0f*pi)))*2.0f*pi;
}

//Привести угол к диапазону -PI..PI
mathinline float mathcall NormAnglePI(float angle)
{
	static const float pi = 3.14159265358979323846f;
	if(angle >= -pi && angle <= pi) return angle;
	return (angle/(2.0f*pi) - long(angle/(2.0f*pi)))*2.0f*pi - pi;
}

//Посчитать acos с ограничением диапазона
mathinline float mathcall safeACos(float ang)
{
	double d = (double)ang;
	if(d < -1.0) d = -1.0;
	if(d > 1.0) d = 1.0;
	d = acos(d);
	return (float)d;
}

//Посчитать asin с ограничением диапазона
mathinline float mathcall safeASin(float ang)
{
	double d = (double)ang;
	if(d < -1.0) d = -1.0;
	if(d > 1.0) d = 1.0;
	d = acos(d);
	return (float)d;
}

//Получить луч исходящий из точки на экране
mathinline void mathcall MakeRayFromScreenPoint(float screenX, float screenY,
														float vpWidth, float vpHeight,
														const Matrix & view, const Matrix & prj,
														Vector & rayDir, Vector & rayOrig)
{
	//Позиция луча
	rayOrig = view.GetCamPos();
	//Направление луча
	rayDir.x =  (((2.0f*screenX)/vpWidth) - 1.0f)/prj.m[0][0];
	rayDir.y = -(((2.0f*screenY)/vpHeight) - 1.0f)/prj.m[1][1];
	rayDir.z =  1.0f;
	rayDir = view.MulNormalByInverse(rayDir);
	rayDir.Normalize();
}

mathinline float mathcall Deg2Rad(float degrees)
{
	return degrees*(3.14159265358979323846f/180.0f);
}

mathinline float mathcall Rad2Deg(float radians)
{
	return radians*(180.0f/3.14159265358979323846f);
}

mathinline float mathcall Lerp(float val1, float val2, float lerp_k)
{
	return (val1 + (val2 - val1) * lerp_k);
}

mathinline float mathcall sign(float v)
{
	return v > 0.0f ? 1.0f : v < 0.0f ? -1.0f : 0.0f;
}

//Найти индекс старшего установленного бита
mathinline long mathcall MaxSetBit(dword v)
{
	long i = (sizeof(dword)*8 - 1);
	dword mask = 1 << i;
	while(mask)
	{
		if(mask & v) return i;
		i--;
		mask >>= 1;
	}
	return -1;
}

//Функция модификации коэфициента затухания сплайном k = [0..1]
mathinline float mathcall AttenuationBySpline(float k, float c1, float c2, float c3, float c4)
{
	float ki = 1.0f - k;
	k = 1.0f - k + ki*k*(k*k*(k*c4 + ki*c3) + ki*ki*(k*c2 + ki*c1));
	return k;
}


/*
Как задается fWidthAspectMultipler:

В случае, если нужно сделать принудительный аспект, отличный от разрешения экрана, то так:
[code]
float divider = 1.0f / (screenHeight / screenWidth);
float requredAspect = (3.0f / 4.0f);
float fWidthAspectMultipler = requredAspect * divider;
[/code]
Если же нужно аспект получать из разрешения сторон, то fWidthAspectMultipler = 1.0


Посчитать из FOV'a 16:9 фов который надо поставить камере в текущем разрешении

*/
mathinline float mathcall recalculateFov (float fWidth, float & fHeight, float fWidthAspectMultipler, float fInputFov)
{
	float aspect = (1.0f / fWidthAspectMultipler ) * (fWidth / fHeight);

	//Эталонный fov задан в аспекте 16:9
	float ethalohAspect = 9.0f / 16.0f;

	float kCorrection = aspect * ethalohAspect;
	//kCorrection = coremax(kCorrection, 0.0f);
	float tg = tanf(fInputFov * 0.5f);
	float ktg = tg * kCorrection;
	float ang = atanf(ktg) * 2.0f;

	fHeight = fWidth * ethalohAspect * fInputFov / ang;

	return ang;
}


/*

Обратная функция для recalculateFov

После того, как достали fov из матрицы проекции, позволяет узнать его реальное значение...

*/
mathinline float mathcall invRecalculateFov (float fWidth, float fHeight, float fWidthAspectMultipler, float fInputFov)
{
	float aspect = (1.0f / fWidthAspectMultipler ) * (fWidth / fHeight);

	//Эталонный fov задан в аспекте 16:9
	float ethalohAspect = 9.0f / 16.0f;

	float kCorrection = aspect * ethalohAspect;
	//kCorrection = coremax(kCorrection, 0.0f);

	float ktg = tanf(fInputFov * 0.5f);
	float tg = ktg / kCorrection;

	float ang = atanf(tg) * 2.0f;
	return ang;
}






#endif
