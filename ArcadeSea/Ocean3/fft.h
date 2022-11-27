#pragma once

#include "..\..\Common_h\Mission.h"

class complex
{
public:
	float Re, Im;

	complex() : Re(0.0f), Im(0.0f) {}
	complex(float re, float im) : Re(re), Im(im) {}

	__forceinline complex  operator ~ () const { return complex( Re, -Im); }

	__forceinline complex operator + (const complex & other) const
	{
		return complex(Re + other.Re, Im + other.Im);
	}

	__forceinline complex operator * (const complex & other)
	{
		return complex(	Re * other.Re - Im * other.Im,
						Re * other.Im + Im * other.Re);
	}

    __forceinline friend complex operator * (const complex & z1, const complex & z2)
	{ 
		return complex(z1.Re * z2.Re - z1.Im * z2.Im, z1.Im * z2.Re + z1.Re * z2.Im); 
	}

	__forceinline complex operator - (const complex & other) const
	{
		return complex(Re - other.Re, Im - other.Im);
	}

	__forceinline complex & operator += (const complex & other)
	{
		Re += other.Re;
		Im += other.Im;
		return *this;
	}

	__forceinline friend float Mod(complex const & z) 
	{ 
		return float( sqrtf(z.Re * z.Re + z.Im * z.Im)); 
	}
	
	__forceinline friend float  Mod2(complex const & z) 
	{ 
		return (z.Re * z.Re + z.Im * z.Im); 
	}

	__forceinline friend  complex operator / (complex const & z, float s) 
	{ 
		return complex(z.Re / s, z.Im / s); 
	}

};

__forceinline complex Polar(float mod, float arg) 
{ 
	return complex(mod * cosf(arg), mod * sinf(arg)); 
}

class fft
{
public:
	enum
	{
		fft_max = 128
	};

	struct dcomplex
	{
		complex c1, c2;
	};

	fft();
	~fft();

	// platform specific constructor/destructor
	void Construct();
	void Destruct();

	void Init(int size);

	void Setup(float wave_len, float wave_amp, float wave_vel, float wave_x, float wave_y, float align_power);
	void Execute(float deltaTime);
	void ChangeCurFrame();

	__forceinline int GetSize() const { return m_size; };
	__forceinline dcomplex * GetCurFrame() const { return m_curFrame; };

private:
	static int		refCount;
	static char		* m_polarsUnaligned;
	static dcomplex	* m_polars;

	char		* m_curFrameUnaligned[2];
	
	dcomplex	* m_curFrame, * m_nextFrame;

	char		* m_zeroFrameUnaligned;
	dcomplex	* m_zeroFrame;

	char		* m_omegasUnaligned;
	float		* m_omegas;

	char		* m_wReImUnaligned;
	Vector4		* m_wReIm;

	char		* m_nxyUnaligned;
	float		* m_nxy;

	complex		* m_gausRND;

	float m_waveLen, m_waveAmp, m_waveVel;
	float m_waveX, m_waveY;
	float m_time;

	int		m_size;
	bool	m_original;

	dcomplex	polars[4096];
	dword		m_linearSwapper[fft_max];
	dword		m_swapper[fft_max];
	int			m_swapsNum;

	float N1();
	void Animate(float time);
	void FFT(dcomplex * data);
	void __forceinline FFTHorizontal(dcomplex * __restrict data);
	void __forceinline FFTVertical(dcomplex * __restrict data);

#ifdef _XBOX
	template<int nsize> void __forceinline FFTn(dcomplex * __restrict idata, dcomplex * __restrict jdata, __vector4 wRe, __vector4 wIm, int istep);
	void __forceinline Setup_w_ReIm(Vector4 * * __restrict wReIm, __vector4 & wRe, __vector4 & wIm);
#endif
};
