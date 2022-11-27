#include "ocean.h"

#ifdef _XBOX

#define VPERM(x, y, z, w)	((unsigned int)(x << 6L) | (y << 4L) | (z << 2L) | w)

void __forceinline Ocean2::CalcWaveXZCoords(__vector4 vin, WaveXZIntegerStore * __restrict res)
{
	// координаты для первого вертекса
	__vector4 tv = __vpermwi(vin, VPERM(0, 2, 0, 2));
	__vector4 vxz = (tv + __VMXGetReg(vmove)) * __VMXGetReg(vscale);
	__vector4 ivxz = __vrfim(vxz);

	__vector4 delta = vxz - ivxz;
	//vX1 = __vspltw(delta, 0);
	//vZ1 = __vspltw(delta, 1);

	ivxz = ivxz + __VMXGetReg(v0011);
	ivxz = __vand(__vctsxs(ivxz, 0), __VMXGetReg(vxwidth));
	ivxz = __vslw(ivxz, __VMXGetReg(vx_shift_y_fftSize));	// * 16, y * fftSize * 16, * 16, y * fftSize * 16

	__vector4 x1212 = __vpermwi(ivxz, VPERM(0, 2, 0, 2));
	__vector4 y1122 = __vpermwi(ivxz, VPERM(1, 1, 3, 3));
	ivxz = __vadduws(x1212, y1122);

	delta = __vperm(delta, vin, __VMXGetReg(vpermControl_VXVZ_XZ));

	__stvx(delta, res, 0);
	__stvx(ivxz, res, 16);
}

void __forceinline Ocean2::CalcWaveYNormal(__vector4 vin, WaveXZIntegerStore * __restrict data, Ocean3Service::SeaVertex * __restrict result)
{
	__vector4 tmp;

	__vector4 delta = __lvlx(&data->v4, 0);
	__vector4 vX = __vspltw(delta, 0);
	__vector4 vZ = __vspltw(delta, 1);
	 __vector4 xz = __vpermwi(delta, VPERM(2, 0, 3, 0));
	 xz = __vand(xz, __VMXGetReg(vmask1010));
	 vin = vin + xz;

	__vector4 m1 = __lvlx(m_fftCurFrame, data->i4[0]);
	__vector4 m2 = __lvlx(m_fftCurFrame, data->i4[1]);
	__vector4 m3 = __lvlx(m_fftCurFrame, data->i4[2]);
	__vector4 m4 = __lvlx(m_fftCurFrame, data->i4[3]);

	// FIX-ME тут можно несколько команд заменить на "mad"
	tmp = __vsubfp( __vsubfp( __vaddfp(m4, m1), m2), m3);	// (a4 + a1 - a2 - a3)
	tmp = __vmulfp(tmp, vX * vZ);							// fX1*fZ1*
	tmp = __vaddfp(tmp, __vmulfp(__vsubfp(m3, m1), vZ));	// fZ1*(a3 - a1) + 
	tmp = __vaddfp(tmp, __vmulfp(__vsubfp(m2, m1), vX));	// fX1*(a2 - a1) + 
	tmp = __vaddfp(tmp, m1);								// a1 +

	__vector4 normalsZW = __vmaddfp(tmp, __VMXGetReg(v_05_4096), __VMXGetReg(v_05_4096));	// 0.0, 0.0f, nx(0..4096), nz(0..4096)
	normalsZW =__vslw(__vctsxs(normalsZW, 0), __VMXGetReg(v_shift_normals));
	__vector4 normalsW = __vpermwi(normalsZW, VPERM(0, 0, 0, 2));
	normalsW = __vadduwm(normalsW, normalsZW);
	normalsW = __vor(normalsW, __VMXGetReg(valpha));

	tmp = __vpermwi(__vand(tmp, __VMXGetReg(vmask1000)), VPERM(1, 0, 1, 1));	// 0, y, 0, 0
	tmp = tmp + vin;
	tmp = __vperm(tmp, normalsW, __VMXGetReg(vpermControl0127));

	__stvx(tmp, result, 0);
}

void Ocean2::WaveXZBlock_x360(SeaBlock & block)
{
	//return;
	dword dw1;
	RDTSC_B(dw1);
	float	fStep = fGridStep * float(1 << block.iLOD);
	float	fSize = fGridStep * block.iSize;
	int	x, y, size0 = block.iSize >> block.iLOD;
	
	float x1 = float(block.iTX * block.iSize) * fGridStep;
	float y1 = float(block.iTY * block.iSize) * fGridStep;
	float x2 = x1 + float(size0) * fStep;
	float y2 = y1 + float(size0) * fStep;

	x1 += vSeaCenterPos.x;	x2 += vSeaCenterPos.x;
	y1 += vSeaCenterPos.z;	y2 += vSeaCenterPos.z;

	dword * pIndices = m_oceans->m_indices;

	__declspec(align(16)) Vector4 in;
	in.y = 0.0f;

	Ocean3Service::SeaVertex * __restrict verts = m_pVSea;

	float seaHeight = 0.0f;//m_seaHeight + m_seaFakeHeight;

	__vector4 vin, vz, vy;
	__vector4 vaddx = XMVectorSet(fStep, 0.0f, 0.0f, 0.0f);
	__vector4 vaddz = XMVectorSet(0.0f, 0.0f, fStep, 0.0f);

	// [0] = x,z,deltax,deltaz	[1] = i1,i2,i3,i4
	__declspec(align(16)) WaveXZIntegerStore istore[128 * 2];

	vy = XMVectorSet(0.0f, seaHeight, 0.0f, 0.0f);
	vz = XMVectorSet(x1 + fStep, seaHeight, y1 + fStep, 0.0f);
	// обрабатываем середину (всегда с новыми вертексами)
	for (y=1; y<=size0-1; y++)
	{
		vin = vz;
		WaveXZIntegerStore * __restrict pstore = istore;
		for (x=1; x<=size0-2; x+=2)
		{
			CalcWaveXZCoords(vin, pstore);
			vin += vaddx;
			pstore++;

			CalcWaveXZCoords(vin, pstore);
			vin += vaddx;
			pstore++;
		}
		CalcWaveXZCoords(vin, pstore);
		
		pstore = istore;
		for (x=1; x<=size0-2; x+=2)
		{
			__dcbt(pstore[2].i4[0], m_fftCurFrame);
			__dcbt(pstore[2].i4[2], m_fftCurFrame);
			__dcbt(pstore[3].i4[0], m_fftCurFrame);
			__dcbt(pstore[3].i4[2], m_fftCurFrame);

			CalcWaveYNormal(vy, pstore, verts);
			pstore++;
			verts++;
			CalcWaveYNormal(vy, pstore, verts);
			pstore++;
			verts++;
		}
		CalcWaveYNormal(vy, pstore, verts);
		verts++;
		
		vz += vaddz;
	}

	dword * upIndices = &pIndices[block.iIStart];
	dword * leftIndices = upIndices + (size0 + 1);
	dword * rightIndices = leftIndices + (size0 + 1);
	dword * downIndices = rightIndices + (size0 + 1);

	
	WaveXZIntegerStore * __restrict pstore;
	WaveXZIntegerStore * __restrict pread;

	// Верхние вертексы
		vin = XMVectorSet(x1, seaHeight, y1, 0.0f);
		pstore = istore;
		for (x=0; x<=size0; x++)
		{
			if (upIndices[x] >= block.VStart)
			{
				CalcWaveXZCoords(vin, pstore);
				pstore++;
			}
			vin += vaddx;
		}

		pread = istore;
		while (pread < pstore)
		{
			CalcWaveYNormal(vy, pread, verts);
			pread++;
			verts++;
		}

	// Левые вертексы
		vin = XMVectorSet(x1, seaHeight, y1 + fStep, 0.0f);
		pstore = istore;
		for (x=1; x<size0; x++)
		{
			if (leftIndices[x] >= block.VStart)
			{
				CalcWaveXZCoords(vin, pstore);
				pstore++;
			}
			vin += vaddz;
		}

		pread = istore;
		while (pread < pstore)
		{
			CalcWaveYNormal(vy, pread, verts);
			pread++;
			verts++;
		}

	// Правые вертексы
		vin = XMVectorSet(x2, seaHeight, y1 + fStep, 0.0f);
		pstore = istore;
		for (x=1; x<size0; x++)
		{
			if (rightIndices[x] >= block.VStart)
			{
				CalcWaveXZCoords(vin, pstore);
				pstore++;
			}
			vin += vaddz;
		}

		pread = istore;
		while (pread < pstore)
		{
			CalcWaveYNormal(vy, pread, verts);
			pread++;
			verts++;
		}

	// Нижнии вертексы
		vin = XMVectorSet(x1, seaHeight, y2, 0.0f);
		pstore = istore;
		for (x=0; x<=size0; x++)
		{
			if (downIndices[x] >= block.VStart)
			{
				CalcWaveXZCoords(vin, pstore);
				pstore++;
			}
			vin += vaddx;
		}
	
		pread = istore;
		while (pread < pstore)
		{
			CalcWaveYNormal(vy, pread, verts);
			pread++;
			verts++;
		}

	m_pVSea = verts;

	RDTSC_E(dw1);

	m_totalWaveXZRDTSC += dw1;
	m_totalWaveXZVerts += Sqr(size0 + 1);
}

#endif
