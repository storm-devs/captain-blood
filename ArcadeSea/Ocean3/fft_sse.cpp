#include "fft.h"

#ifndef _XBOX

void fft::Construct()
{
	m_zeroFrameUnaligned = null;
	m_omegasUnaligned = null;
	m_wReImUnaligned = null;
	m_nxyUnaligned = null;
	m_gausRND = null;
	m_time = 0.0f;
	
	m_original = false;
}

void fft::Destruct()
{
	DELETE(m_zeroFrameUnaligned);
	DELETE(m_omegasUnaligned);
	DELETE(m_wReImUnaligned);
	DELETE(m_nxyUnaligned);
	DELETE(m_gausRND);
}

void fft::Init(int size)
{
	Assert(size <= fft_max);
	m_size = size;
	m_time = 0.0f;

	m_curFrameUnaligned[0] = NEW char[sizeof(dcomplex) * m_size * m_size + 16];
	m_curFrameUnaligned[1] = NEW char[sizeof(dcomplex) * m_size * m_size + 16];

	m_zeroFrameUnaligned = NEW char[sizeof(dcomplex) * m_size * m_size + 16];

	m_zeroFrame = (dcomplex*)((dword)m_zeroFrameUnaligned + 15 & ~15);

	m_omegasUnaligned = NEW char[sizeof(float) * m_size * m_size + 16];
	m_omegas = (float*)((dword)m_omegasUnaligned + 15 & ~15);

	m_wReImUnaligned = NEW char[sizeof(Vector4) * 2 * size + 16];
	m_wReIm = (Vector4*)((dword)m_wReImUnaligned + 15 & ~15);

	m_nxyUnaligned = NEW char[sizeof(float) * m_size + 16];
	m_nxy = (float*)((dword)m_nxyUnaligned + 15 & ~15);

	srand(GetTickCount());
	m_gausRND = NEW complex[m_size * m_size];
	for (int i=0; i<m_size * m_size; i++)
	{
		m_gausRND[i].Re = N1();
		m_gausRND[i].Im = N1();
	}

	m_curFrame = (dcomplex*)((dword)m_curFrameUnaligned[0] + 15 & ~15);
	m_nextFrame = (dcomplex*)((dword)m_curFrameUnaligned[1] + 15 & ~15);
}

void N2(float & y1, float & y2)
{
	float x1, x2, w;
 
	do 
	{
		x1 = 2.0f * Rnd() - 1.0f;
		x2 = 2.0f * Rnd() - 1.0f;
		w = x1 * x1 + x2 * x2;
	} while ( w >= 1.0f );

	w = sqrt( (-2.0f * logf( w ) ) / w );
	y1 = x1 * w;
	y2 = x2 * w;
}

void fft::Setup(float wave_len, float wave_amp, float wave_vel, float wave_x, float wave_y, float align_power)
{
	complex	k, wave_dir;
	float	m, k2, c2, L;

	ProfileTimer t3;
	//api->Trace("t1 = %d, t2 = %d, t3 = %d", t1.GetTime32(), t2.GetTime32(), t3.GetTime32());

	m_waveLen = wave_len;
	m_waveAmp = wave_amp;
	m_waveVel = wave_vel;
	m_waveX = wave_x;
	m_waveY = wave_y;

	L = Sqr(m_waveVel) / 9.81f;
	wave_dir = complex(m_waveX, m_waveY); wave_dir = wave_dir / Mod(wave_dir);
	float k0 = 6.28318530717959f / m_waveLen;

	ProfileTimer t4;
	for(int y=0; y<m_size; y++)
		for(int x=0; x<m_size; x++)
		{
			int nx = x; if (nx >= m_size / 2) nx -= m_size;
			int ny = y; if (ny >= m_size / 2) ny -= m_size;

			k = complex(k0 * nx, k0 * ny); k2 = Mod2(k);
			c2 = Sqr(k.Re * wave_dir.Re + k.Im * wave_dir.Im) / k2;
			//c2 = 1.0f;
			//m = sqrtf(0.5f * m_waveAmp * exp( -1.0f / ( k2 * Sqr(L) ) ) * c2 * c2 * c2 / Sqr(k2));
			m = sqrtf(0.5f * wave_amp * exp( -1.0f / ( k2 * Sqr(L) ) ) * pow(c2, align_power) / Sqr(k2));
			//float k = exp(-k2 * Sqr(0.0001f));
			//m *= k;

			float n1, n2;
			//n1 = N1(); n2 = N1();
			n1 = m_gausRND[x + y * m_size].Re;
			n2 = m_gausRND[x + y * m_size].Im;
			//api->Trace("n1 = %.2f, n2 = %.2f", n1, n2);

			m_zeroFrame[x + y * m_size].c1 = complex(m * n1, m * n2);
		}
	t4.Stop();

	m_zeroFrame[0].c1 = complex(0.0f, 0.0f);

	for (int y=0; y<m_size; y++)
		for (int x=0; x<m_size; x++)
		{
			int nx = x; if (nx >= m_size/2) nx -= m_size;
			int ny = y; if (ny >= m_size/2) ny -= m_size;

			k = complex(k0 * nx, k0 * ny); 
			m_omegas[x + y * m_size] = sqrt(Mod(k) * 9.81f);

			int inx = -nx; if (inx < 0) inx += m_size;
			int iny = -ny; if (iny < 0) iny += m_size;
			m_zeroFrame[x + y * m_size].c2 = ~m_zeroFrame[inx + iny * m_size].c1;
		}

	for (int i=0; i<m_size; i++)
	{
		int ni = i; if (ni >= m_size/2) ni -= m_size;
		m_nxy[i] = k0 * ni;
	}

	for (int i = 0; i < m_size; i++)
		m_linearSwapper[i] = i * m_size;

	int j = 0;
	m_swapsNum = 0;
	for(int i = 0; i < m_size; i++)
	{
		if (j > i)
		{
			Swap(m_linearSwapper[i], m_linearSwapper[j]);
			m_swapper[m_swapsNum++] = j + (i << 16L);
		}
		
		int m = m_size >> 1;
		while(m >= 1 && j >= m) { j -= m; m >>= 1; }
		j += m;
	}

	int mmax = 1, idx = 0;  
	float theta = -(6.28318530717959f / 2.0f);
    while(m_size > mmax)
	{
		complex wp = complex(cos(theta), sin(theta));
		complex w  = complex(1.0f, 0.0f);
		for(m = 0; m < mmax; m++)
		{
			m_wReIm[idx*2+0] = Vector4(w.Re, w.Re, w.Re, w.Re);
			m_wReIm[idx*2+1] = Vector4(-w.Im, w.Im, -w.Im, w.Im);
			idx++;
			w = w * wp;
		}
		mmax *= 2; 
		theta = theta * 0.5f;
	}
	t3.Stop();

	//api->Trace("Setup time: %d, %d", t3.GetTime32(), t4.GetTime32());
}

void fft::Animate(float time)
{
	float k0 = 6.28318530717959f / m_waveLen;

	float dtime = time * (4096.0f / PIm2);
	float dtime2 = time * 4096.0f;
	float * py = m_nxy;

	complex k;
	for (int y=0; y<m_size; y++)
	{
		k.Im = *py++;

		dcomplex * curFrame = m_nextFrame + y * m_size;
		dcomplex * zeroFrame = m_zeroFrame + y * m_size;
		float * omegaFrame = m_omegas + y * m_size;

		float * px = m_nxy;
		for (int x=0; x<m_size; x++)
		{
			//if (!m_original)
			{
				k.Re = *px++;
				float omega = *omegaFrame;

				const dcomplex & pp = m_polars[int(omega * dtime) & 4095L];
				complex temp = zeroFrame->c1 * pp.c1;
				curFrame->c1 = temp + zeroFrame->c2 * pp.c2;//(~pp1);
				curFrame->c2 = curFrame->c1 * complex(-k.Im, k.Re);
			}
			/*else
			{
				int nx = x; if (nx >= m_size/2) nx -= m_size;
				int ny = y; if (ny >= m_size/2) ny -= m_size;
				complex k = complex(k0 * nx, k0 * ny); 
				float omega = sqrt(Mod(k) * 9.81f);
				complex temp = zeroFrame->c1 * Polar(1.0f, omega * time);
				int inx = -nx; if (inx < 0) inx += m_size;
				int iny = -ny; if (iny < 0) iny += m_size;
				curFrame->c1 = temp + (~m_zeroFrame[inx + iny * m_size].c1) * Polar(1.0f, -omega * time);
				curFrame->c2 = curFrame->c1 * complex(-k.Im, k.Re);
			}*/
			curFrame++;
			zeroFrame++;
			omegaFrame++;
		}
	}
}

void fft::FFTHorizontal(dcomplex * data)
{
	for (int k = 0; k < m_swapsNum; k++)
	{
		int i = m_swapper[k] >> 16L;
		int j = m_swapper[k] & 0xFFFF;

		__m128 mi = _mm_load_ps((float*)&data[i].c1);
		__m128 mj = _mm_load_ps((float*)&data[j].c1);
		_mm_store_ps((float*)&data[i].c1, mj);
		_mm_store_ps((float*)&data[j].c1, mi);
	}

	FFT(data);
}

void fft::FFTVertical(dcomplex * data)
{
	__declspec(align(16)) dcomplex linear_data[fft_max];

	for (int i=0; i<m_size; i++)
		linear_data[i] = data[m_linearSwapper[i]];

	FFT(linear_data);

	for (int i=0; i<m_size; i++)
		data[i * m_size] = linear_data[i];
}

void fft::FFT(dcomplex * data)
{
	int i, j, m, mmax, istep;
	int idx1 = 0;
	mmax = 1; 

	int idx = 0;
	//if (!m_original)
	{
		while (m_size > mmax)
		{
			istep = 2 * mmax;

			for(m = 0; m < mmax; m++)
			{
				__m128 wre = _mm_load_ps(&m_wReIm[idx + 0].v4[0]);
				__m128 wim = _mm_load_ps(&m_wReIm[idx + 1].v4[0]);
				idx += 2;

				for(i = m; i < m_size; i += istep)
				{
					j = i + mmax;

					__m128 mj = _mm_load_ps((float*)&data[j].c1);
					__m128 mi = _mm_load_ps((float*)&data[i].c1);

					__m128 imj = _mm_shuffle_ps(mj, mj, _MM_SHUFFLE(2, 3, 0, 1));

					__m128 mtemp = _mm_mul_ps(wre, mj);
					mtemp = _mm_add_ps(mtemp, _mm_mul_ps(wim, imj));

					_mm_store_ps((float*)&data[j].c1, _mm_sub_ps(mi, mtemp));
					_mm_store_ps((float*)&data[i].c1, _mm_add_ps(mi, mtemp));
				}
			}
			
			mmax = istep;  
		}
	}
	/*else 
	{
		dcomplex temp;
		mmax = 1; 
		float theta = -(6.28318530717959f / 2.0f);
	    while (m_size > mmax)
		{
			istep = 2 * mmax;
			complex wp = complex(cos(theta), sin(theta));
			complex w  = complex(1.0f, 0.0f);

			for (m = 0; m < mmax; m++)
			{
				for (i = m; i < m_size; i += istep)
				{
					j = i + mmax;
					temp.c1 = w * data[j].c1;
					temp.c2 = w * data[j].c2;
					data[j].c1 = data[i].c1 - temp.c1;
					data[j].c2 = data[i].c2 - temp.c2;
					data[i].c1 = data[i].c1 + temp.c1;
					data[i].c2 = data[i].c2 + temp.c2;
				}
				w = w * wp;
			}

			mmax = istep; theta = theta * 0.5f;
		}
   }*/
}

void fft::Execute(float deltaTime)
{
	//m_original = api->DebugKeyState('D');

	m_time += deltaTime;
	
	ProfileTimer t1;
	Animate(m_time);
	t1.Stop();
	ProfileTimer t2;
		for (int i=0; i<m_size; i++) FFTHorizontal(m_nextFrame + i * m_size);
	t2.Stop();
	ProfileTimer t3;
		for (int i=0; i<m_size; i++) FFTVertical(m_nextFrame + i);
	t3.Stop();

	//api->Trace("t1 = %d, t2 = %d, t3 = %d", t1.GetTime32(), t2.GetTime32(), t3.GetTime32());
}

float fft::N1()
{
	int		i;
	const	int		n = 200;
	const	float	S = sqrt(float(n) / 12.0f);
	const	float	M = float(n) / 2;
	float	x = 0.0;
	int		ix = 0;

	//for (i=0; i<n; i++) x += Rnd(); 
	for (i=0; i<n; i++) ix += rand();
	x = float(double(ix) * (1.0 / double(RAND_MAX)));
	x = (x - M) / S;
	return x;
}

#endif