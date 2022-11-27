#ifndef _DataSwizzle_h_
#define _DataSwizzle_h_


//Возвращает true, если надо проводить операции свизла
__forceinline bool __SwizzleDetect()
{
#ifndef _XBOX
	return false;
#else
	return true;
#endif
/*
	union Detector
	{
		__int32 i32;
		__int8 i[4];
	};

	Detector detector;
	detector.i32 = 1;
	return (detector.i[0] == 0);
*/
}


//Универсальный перестановщик
template<class T> __forceinline T __DataSwizzler(const T & value)
{
	union Swizzler
	{
		T value;
		__int8 bytes[sizeof(T)];
	};

	core_unaligned const T & tmp = value;
	Swizzler swizzler;
	swizzler.value = tmp;
	for(long i = 0; i < sizeof(T)/2; i++)
	{
		__int8 & b1 = swizzler.bytes[i];
		__int8 & b2 = swizzler.bytes[sizeof(T) - i - 1];
		__int8 tmp = b1;
		b1 = b2;
		b2 = tmp;
	}
	return swizzler.value;
}

//Универсальный перестановщик
template<class T> __forceinline void __RefDataSwizzler(T & value)
{
	union Swizzler
	{
		T value;
		__int8 bytes[sizeof(T)];
	};

	core_unaligned T & tmp = value;
	Swizzler & swizzler = (Swizzler &)tmp;
	for(long i = 0; i < sizeof(T)/2; i++)
	{
		__int8 & b1 = swizzler.bytes[i];
		__int8 & b2 = swizzler.bytes[sizeof(T) - i - 1];
		__int8 tmp = b1;
		b1 = b2;
		b2 = tmp;
	}
}

__forceinline word SwizzleWord(const word & w)
{
	if(!__SwizzleDetect()) return w;
	return __DataSwizzler(w);
}

__forceinline short SwizzleShort(const short & s)
{
	if(!__SwizzleDetect()) return s;
	return __DataSwizzler(s);
}

__forceinline dword SwizzleDWord(const dword & dw)
{
	if(!__SwizzleDetect()) return dw;
	return __DataSwizzler(dw);
}

__forceinline long SwizzleLong(const long & lg)
{
	if(!__SwizzleDetect()) return lg;
	return __DataSwizzler(lg);
}

__forceinline float SwizzleFloat(const float & f)
{
	if(!__SwizzleDetect()) return f;
	return __DataSwizzler(f);
}

__forceinline double SwizzleDouble(const double & d)
{
	if(!__SwizzleDetect()) return d;
	return __DataSwizzler(d);
}


//============================ joker ==========================================================


__forceinline void XSwizzleWord(word & w)
{
	if(!__SwizzleDetect()) return;
	__RefDataSwizzler(w);
}

__forceinline void XSwizzleShort(short & s)
{
	if(!__SwizzleDetect()) return;
	__RefDataSwizzler(s);
}

__forceinline void XSwizzleULong(unsigned long & dw)
{
	if(!__SwizzleDetect()) return;
	__RefDataSwizzler(dw);
}


__forceinline void XSwizzleDWord(dword & dw)
{
	if(!__SwizzleDetect()) return;
	__RefDataSwizzler(dw);
}

__forceinline void XSwizzleLong(long & lg)
{
	if(!__SwizzleDetect()) return;
	__RefDataSwizzler(lg);
}

__forceinline void XSwizzleFloat(float & f)
{
	if(!__SwizzleDetect()) return;
	__RefDataSwizzler(f);
}

__forceinline void XSwizzleDouble(double & d)
{
	if(!__SwizzleDetect()) return;
	__RefDataSwizzler(d);
}




#endif