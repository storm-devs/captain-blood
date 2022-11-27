#ifdef _XBOX

#include "fft.h"

#define VPERM(x, y, z, w)	((unsigned int)(x << 6L) | (y << 4L) | (z << 2L) | w)

void fft::Construct()
{
	m_zeroFrameUnaligned = null;
	m_omegasUnaligned = null;
	m_wReImUnaligned = null;
	m_nxyUnaligned = null;
	m_gausRND = null;
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

	m_curFrameUnaligned[0] = NEW char[sizeof(dcomplex) * m_size * m_size + 128];
	m_curFrameUnaligned[1] = NEW char[sizeof(dcomplex) * m_size * m_size + 128];

	m_zeroFrameUnaligned = NEW char[sizeof(dcomplex) * m_size * m_size + 128];

	m_zeroFrame = (dcomplex*)((dword)m_zeroFrameUnaligned + 127 & ~127);

	m_omegasUnaligned = NEW char[sizeof(float) * m_size * m_size + 16];
	m_omegas = (float*)((dword)m_omegasUnaligned + 15 & ~15);

	m_wReImUnaligned = NEW char[sizeof(Vector4) * 2 * size + 16];
	m_wReIm = (Vector4*)((dword)m_wReImUnaligned + 15 & ~15);

	m_nxyUnaligned = NEW char[sizeof(float) * m_size + 16];
	m_nxy = (float*)((dword)m_nxyUnaligned + 15 & ~15);

	m_gausRND = NEW complex[m_size * m_size];
	for (int i=0; i<m_size * m_size; i++)
	{
		m_gausRND[i].Re = N1();
		m_gausRND[i].Im = N1();
	}

	m_curFrame = (dcomplex*)((dword)m_curFrameUnaligned[0] + 127 & ~127);
	m_nextFrame = (dcomplex*)((dword)m_curFrameUnaligned[1] + 127 & ~127);
}

void fft::Setup(float wave_len, float wave_amp, float wave_vel, float wave_x, float wave_y, float align_power)
{
	complex	k, wave_dir;
	float	m, k2, c2, L;

	m_waveLen = wave_len;
	m_waveAmp = wave_amp;
	m_waveVel = wave_vel;
	m_waveX = wave_x;
	m_waveY = wave_y;

	L = Sqr(m_waveVel) / 9.81f;
	wave_dir = complex(m_waveX, m_waveY); wave_dir = wave_dir / Mod(wave_dir);
	float k0 = 6.28318530717959f / m_waveLen;
	
	for(int y=0; y<m_size; y++)
		for(int x=0; x<m_size; x++)
		{
			int nx = x; if (nx >= m_size / 2) nx -= m_size;
			int ny = y; if (ny >= m_size / 2) ny -= m_size;

			k = complex(k0 * nx, k0 * ny); k2 = Mod2(k);
			c2 = Sqr(k.Re * wave_dir.Re + k.Im * wave_dir.Im) / k2;
			m = sqrtf(0.5f * m_waveAmp * exp( -1.0f / ( k2 * Sqr(L) ) ) * pow(c2, align_power) / Sqr(k2));

			float n1, n2;
			n1 = m_gausRND[x + y * m_size].Re;
			n2 = m_gausRND[x + y * m_size].Im;
			//float n1 = N1();
			//float n2 = N1();

			m_zeroFrame[x + y * m_size].c1 = complex(m * n1, m * n2);
		}

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
}

__forceinline __vector4 ComplexMul(__vector4 & src, __vector4 & pp, __vector4 & v1111)
{
	__vector4 v1 = __vpermwi(src, VPERM(0, 0, 2, 2));
	__vector4 v2 =  pp;
	__vector4 v3 = __vpermwi(src, VPERM(1, 1, 3, 3));
	__vector4 v4 = __vpermwi(pp, VPERM(1, 0, 3, 2));

	return (v1 * v2) + (v1111 * (v3 * v4));
}

__forceinline __vector4 ComplexMulAddH(__vector4 & src, __vector4 & pp, __vector4 & v1111)
{
	__vector4 res1 = ComplexMul(src, pp, v1111);
	__vector4 res2 = __vpermwi(res1, VPERM(2, 3, 3, 3));

	return res1 + res2;
}

void fft::Animate(float time)
{
	float k0 = 6.28318530717959f / m_waveLen;

	float dtime = time * (4096.0f / PIm2);
	float * __restrict py = m_nxy;

	__vector4 vx_shift_4444 = XMVectorReplicateInt(4);
	__vector4 _v1111 = XMVectorSet(-1.0f, 1.0f, -1.0f, 1.0f);
	__vector4 _dtime = XMVectorReplicate(dtime);
	__vector4 _v4095 = XMVectorReplicateInt(4095);

	__declspec(align(16)) byte byteControl[16] = {0,1,2,3, 4,5,6,7, 16,17,18,19, 20,21,22,23};
	__vector4 permControl = __lvlx(&byteControl, 0);

	__declspec(align(16)) byte byteControl12[16] = {0,1,2,3, 4,5,6,7, 16,17,18,19, 20,21,22,23};
	__vector4 permControl12 = __lvlx(&byteControl12, 0);

	__declspec(align(16)) byte byteControl34[16] = {0,1,2,3, 4,5,6,7, 24,25,26,27, 28,29,30,31};
	__vector4 permControl34 = __lvlx(&byteControl34, 0);

	__declspec(align(16)) byte byteControlK12[16] = {0,1,2,3, 16,17,18,19, 4,5,6,7, 20,21,22,23};
	__vector4 permControlK12 = __lvlx(&byteControlK12, 0);

	__declspec(align(16)) byte byteControlK34[16] = {8,9,10,11, 24,25,26,27, 12,13,14,15, 28,29,30,31};
	__vector4 permControlK34 = __lvlx(&byteControlK34, 0);

	__declspec(align(16)) int polarsIndices[128 * 4];

	//if (!m_original)
	{
		for (int y=0; y<m_size; y++)
		{
			float ny = *py++;
			__vector4 _ny = XMVectorReplicate(-ny);
			//py += 4;
			//float ny = *py++;

			dcomplex * __restrict curFrame = m_nextFrame + y * m_size;
			dcomplex * __restrict zeroFrame = m_zeroFrame + y * m_size;
			float * __restrict omegaFrame = m_omegas + y * m_size;
			float * __restrict px = m_nxy;

			int * __restrict polars = polarsIndices;
			float * __restrict omegas = omegaFrame;
			for (int x=0; x<m_size >> 4; x++)
			{
				__dcbt(64, omegas);

				__vector4 omegas1 = __lvlx(omegas, 0);
				__vector4 polars1 = __vand(__vctsxs(omegas1 * _dtime, 0), _v4095);
				polars1 = __vslw(polars1, vx_shift_4444);
				__stvx(polars1, polars, 0);

				__vector4 omegas2 = __lvlx(omegas, 16);
				__vector4 polars2 = __vand(__vctsxs(omegas2 * _dtime, 0), _v4095);
				polars2 = __vslw(polars2, vx_shift_4444);
				__stvx(polars2, polars, 16);

				__vector4 omegas3 = __lvlx(omegas, 32);
				__vector4 polars3 = __vand(__vctsxs(omegas3 * _dtime, 0), _v4095);
				polars3 = __vslw(polars3, vx_shift_4444);
				__stvx(polars3, polars, 32);

				__vector4 omegas4 = __lvlx(omegas, 48);
				__vector4 polars4 = __vand(__vctsxs(omegas4 * _dtime, 0), _v4095);
				polars4 = __vslw(polars4, vx_shift_4444);
				__stvx(polars4, polars, 48);

				polars += 16;
				omegas += 16;
			}

			polars = polarsIndices;
			for (int x=0; x<m_size >> 3; x++)
			{
				__dcbz128(0, curFrame); 
				__dcbt(0, zeroFrame);
				__dcbt(128, zeroFrame);

				for (int j=0; j<2; j++) 
				{
					__vector4 _nx = __lvlx(px, 0);
					px += 4;

					/*__declspec(align(16)) int polars_idx[4];
					__vector4 _omegas = __lvlx(omegaFrame, 0);
					__vector4 _polars = __vand(__vctsxs(_omegas * _dtime, 0), _v4095);
					__stvx(_polars, polars_idx, 0);*/

					__vector4 k12, k34;
					k12 = __vperm(_ny, _nx, permControlK12);
					k34 = __vperm(_ny, _nx, permControlK34);

					__vector4 pp[4];
					for (int i=0; i<4; i++)
						pp[i] = __lvlx(m_polars, polars[i]);

					__vector4 res[4];

					res[0] = ComplexMulAddH(__lvlx(zeroFrame, 00), pp[0], _v1111);
					res[1] = ComplexMulAddH(__lvlx(zeroFrame, 16), pp[1], _v1111);
					res[2] = ComplexMulAddH(__lvlx(zeroFrame, 32), pp[2], _v1111);
					res[3] = ComplexMulAddH(__lvlx(zeroFrame, 48), pp[3], _v1111);

					__vector4 c2[2];

					c2[0] = __vperm(res[0], res[1], permControl);
					c2[1] = __vperm(res[2], res[3], permControl);

					c2[0] = ComplexMul(c2[0], k12, _v1111);
					c2[1] = ComplexMul(c2[1], k34, _v1111);

					res[0] = __vperm(res[0], c2[0], permControl12);
					res[1] = __vperm(res[1], c2[0], permControl34);
					res[2] = __vperm(res[2], c2[1], permControl12);
					res[3] = __vperm(res[3], c2[1], permControl34);

					__stvx(res[0], curFrame, 0);
					__stvx(res[1], curFrame, 16);
					__stvx(res[2], curFrame, 32);
					__stvx(res[3], curFrame, 48);

					curFrame += 4;
					zeroFrame += 4;
					omegaFrame += 4;
					polars += 4;
				}
			}
		}
	}
	/*else
	{
		for (int y=0; y<m_size; y++)
		{
			dcomplex * curFrame = m_nextFrame + y * m_size;
			dcomplex * zeroFrame = m_zeroFrame + y * m_size;
			float * omegaFrame = m_omegas + y * m_size;

			for (int x=0; x<m_size; x++)
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

				curFrame++;
				zeroFrame++;
				omegaFrame++;
			}
		}
	}*/
}

void __forceinline fft::FFTHorizontal(dcomplex * __restrict data)
{
	for (int k = 0; k < m_swapsNum; k++)
	{
		int i = m_swapper[k] >> 16L;
		int j = m_swapper[k] & 0xFFFF;

		__vector4 mi = __lvlx(&data[i].c1, 0);
		__vector4 mj = __lvlx(&data[j].c1, 0);
		__stvx(mj, &data[i].c1, 0);
		__stvx(mi, &data[j].c1, 0);
	}

	FFT(data);
}

void __forceinline fft::FFTVertical(dcomplex * __restrict data)
{
	__declspec(align(16)) dcomplex linear_data[fft_max * 4];

	for (int i=0; i<m_size; i++)
	{
		const dcomplex * current = &data[m_linearSwapper[i]];
		__vector4 _v0 = __lvlx(current, 0);
		__vector4 _v1 = __lvlx(current, 16);
		__vector4 _v2 = __lvlx(current, 32);
		__vector4 _v3 = __lvlx(current, 48);
		__stvx(_v0, &linear_data[0 * fft_max + i].c1, 0);
		__stvx(_v1, &linear_data[1 * fft_max + i].c1, 0);
		__stvx(_v2, &linear_data[2 * fft_max + i].c1, 0);
		__stvx(_v3, &linear_data[3 * fft_max + i].c1, 0);
		//linear_data[i] = data[m_linearSwapper[i]];
	}

	FFT(&linear_data[0 * fft_max]);
	FFT(&linear_data[1 * fft_max]);
	FFT(&linear_data[2 * fft_max]);
	FFT(&linear_data[3 * fft_max]);

	for (int i=0, isize = 0; i<m_size; i++, isize += m_size)
	{
		const dcomplex * dst = &data[isize];
		__vector4 _v0 = __lvlx(&linear_data[0 * fft_max + i].c1, 0);
		__vector4 _v1 = __lvlx(&linear_data[1 * fft_max + i].c1, 0);
		__vector4 _v2 = __lvlx(&linear_data[2 * fft_max + i].c1, 0);
		__vector4 _v3 = __lvlx(&linear_data[3 * fft_max + i].c1, 0);
		__stvx(_v0, &data[isize], 0);
		__stvx(_v1, &data[isize], 16);
		__stvx(_v2, &data[isize], 32);
		__stvx(_v3, &data[isize], 48);
	}
}

void __forceinline fft::Setup_w_ReIm(Vector4 * * __restrict wReIm, __vector4 & wRe, __vector4 & wIm)
{
	wRe = __lvlx(*wReIm, 0);
	wIm = __lvlx(*wReIm, 16);
	*wReIm += 2;
}

void fft::FFT(dcomplex * data)
{
	int i, j, m, mmax, istep;
	int idx = 0;
	float theta;

	dcomplex	temp;
	complex		w, wp;

	//if (!m_original)
    {
		__vector4 wRe;// = XMVectorSplatOne();
		__vector4 wIm;// = __vspltisw(0);
		int h;
		Vector4 * wReIm = &m_wReIm[0];

		Setup_w_ReIm(&wReIm, wRe, wIm);
		for (h=0; h<128; h+=8*2)
			FFTn<8>(&data[h], &data[h+1], wRe, wIm, 2);

		Setup_w_ReIm(&wReIm, wRe, wIm);
		for (h=0; h<128; h+=8*4)
		{
			FFTn<8>(&data[h], &data[h+2], wRe, wIm, 4);
		}
		Setup_w_ReIm(&wReIm, wRe, wIm);
		for (h=0; h<128; h+=8*4)
		{
			FFTn<8>(&data[h+1], &data[h+1+2], wRe, wIm, 4);
		}

		for (h=0; h<4; h++)
		{
			Setup_w_ReIm(&wReIm, wRe, wIm);
			FFTn<8>(&data[h], &data[h+4], wRe, wIm, 8);
			FFTn<8>(&data[h+64], &data[h+64+4], wRe, wIm, 8);
		}

		for (h=0; h<8; h++)
		{
			Setup_w_ReIm(&wReIm, wRe, wIm);
			FFTn<8>(&data[h], &data[h+8], wRe, wIm, 16);
		}

		for (h=0; h<16; h++)
		{
			Setup_w_ReIm(&wReIm, wRe, wIm);
			FFTn<4>(&data[h], &data[h+16], wRe, wIm, 32);
		}

		for (h=0; h<32; h+=4)
		{
			Setup_w_ReIm(&wReIm, wRe, wIm);
			FFTn<2>(&data[h], &data[h+32], wRe, wIm, 64);
			Setup_w_ReIm(&wReIm, wRe, wIm);
			FFTn<2>(&data[h+1], &data[h+1+32], wRe, wIm, 64);
			Setup_w_ReIm(&wReIm, wRe, wIm);
			FFTn<2>(&data[h+2], &data[h+2+32], wRe, wIm, 64);
			Setup_w_ReIm(&wReIm, wRe, wIm);
			FFTn<2>(&data[h+3], &data[h+3+32], wRe, wIm, 64);
		}

		for (h=0; h<64; h+=8)
		{
			__vector4 wRe[8], wIm[8];
			for (int k=0; k<8; k++)
				Setup_w_ReIm(&wReIm, wRe[k], wIm[k]);
			FFTn<1>(&data[h], &data[h+64], wRe[0], wIm[0], 128);
			FFTn<1>(&data[h+1], &data[h+1+64], wRe[1], wIm[1], 128);
			FFTn<1>(&data[h+2], &data[h+2+64], wRe[2], wIm[2], 128);
			FFTn<1>(&data[h+3], &data[h+3+64], wRe[3], wIm[3], 128);
			FFTn<1>(&data[h+4], &data[h+4+64], wRe[4], wIm[4], 128);
			FFTn<1>(&data[h+5], &data[h+5+64], wRe[5], wIm[5], 128);
			FFTn<1>(&data[h+6], &data[h+6+64], wRe[6], wIm[6], 128);
			FFTn<1>(&data[h+7], &data[h+7+64], wRe[7], wIm[7], 128);
		}
	}
	/*else 
	{
		mmax = 1; theta = -(6.28318530717959f / 2.0f);
	    while (m_size > mmax)
		{
			istep = 2 * mmax;
			wp = complex(cos(theta), sin(theta));
			w  = complex(1.0f, 0.0f);

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

template<int nsize>
void __forceinline fft::FFTn(dcomplex * __restrict idata, dcomplex * __restrict jdata, __vector4 wRe, __vector4 wIm, int istep)
{
	__vector4 vi[nsize];
	__vector4 vj[nsize];

	for (int k=0, i=0; k<nsize; k++, i += istep)
	{
		vi[k] = __lvlx(idata, i * 16);
		vj[k] = __lvlx(jdata, i * 16);
	}

	for (int k=0; k<nsize; k++)
	{
		__vector4 ivj = __vpermwi(vj[k], VPERM(1, 0, 3, 2));
		
		__vector4 tmp = (wRe * vj[k]) + (wIm * ivj);

		vj[k] = vi[k] - tmp;
		vi[k] = vi[k] + tmp;
	}

	for (int k=0, i=0; k<nsize; k++, i+=istep)
	{
		__stvx(vi[k], idata, i * 16);
		__stvx(vj[k], jdata, i * 16);
	}
}

void fft::Execute(float deltaTime) 
{
	//m_original = api->DebugKeyState(ICore::xb_dpad_left);
	//if (m_original)
	//	return;

	m_time += deltaTime;
	
	dword t1, t2, t3;
	
	RDTSC_B(t1);
	Animate(m_time);
	RDTSC_E(t1);
	
	RDTSC_B(t2);
		for (int i=0, si = 0; i<m_size; i++, si += m_size) FFTHorizontal(m_nextFrame + si);
	RDTSC_E(t2);

	RDTSC_B(t3);
		for (int i=0; i<m_size; i+=4) FFTVertical(m_nextFrame + i);
	RDTSC_E(t3);

	//api->Trace("t1 = %d, t2 = %d, t3 = %d", t1, t2, t3);
}

float fft::N1()
{
	int		i;
	const	int		n = 200;
	const	float	S = sqrt(float(n) / 12.0f);
	const	float	M = float(n) / 2;
	int		ix = 0;
	float	x = 0.0;

	//for (i=0; i<n; i++) x += Rnd(); 
	for (i=0; i<n; i++) ix += rand();
	x = float(double(ix) * (1.0 / double(RAND_MAX)));
	x = (x - M) / S;
	return x;
}

#endif