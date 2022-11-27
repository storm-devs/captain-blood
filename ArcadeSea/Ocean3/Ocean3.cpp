#include "Ocean.h"

#define VPERM(x, y, z, w)	((unsigned int)(x << 6L) | (y << 4L) | (z << 2L) | w)

#ifdef _XBOX
void __forceinline Ocean3::WaveXZ(const __vector4 & vin, Ocean3Service::SeaVertex * __restrict res)
{
	__declspec(align(16)) int ixy1[4];

	const int fftSize = m_oceans->fftWaves.GetSize();

	__vector4 _vin = vin;//__lvlx(&v, 0);
	__vector4 _tv = __vpermwi(_vin, VPERM(0, 2, 0, 2));

	// первая волна
	__vector4 _vxz = (_tv + vmove[0]) * vscale[0];
	__vector4 _ivxz = __vrfim(_vxz);

	__vector4 delta = _vxz - _ivxz;
	__vector4 vX1 = __vspltw(delta, 0);
	__vector4 vZ1 = __vspltw(delta, 1);

	_ivxz = _ivxz + v0011;
	_ivxz = __vand(__vctsxs(_ivxz, 0), vxwidth);
	_ivxz = __vslw(_ivxz, vx_shift_y_fftSize);	// y * fftSize

	__vector4 x1212 = __vpermwi(_ivxz, VPERM(0, 2, 0, 2));
	__vector4 y1122 = __vpermwi(_ivxz, VPERM(1, 1, 3, 3));
	_ivxz = __vadduws(x1212, y1122);

	//vcsxwfp(_ivxz, 0);

	__stvx(_ivxz, ixy1, 0);

	//const int & iX11 = ixy1[0];
	//const int & iY11 = ixy1[1];
	//const int & iX12 = ixy1[2];
	//const int & iY12 = ixy1[3];

	// TO-DO посчитать iX__ + iY__ * fftSize прямо в v4 регистрах
	__vector4 m1 = __lvlx(&m_fftCurFrame[ixy1[0]], 0);//iX11 + iY11 * fftSize], 0);
	__vector4 m2 = __lvlx(&m_fftCurFrame[ixy1[1]], 0);//iX12 + iY11 * fftSize], 0);
	__vector4 m3 = __lvlx(&m_fftCurFrame[ixy1[2]], 0);//iX11 + iY12 * fftSize], 0);
	__vector4 m4 = __lvlx(&m_fftCurFrame[ixy1[3]], 0);//iX12 + iY12 * fftSize], 0);

	__vector4 tmp;

	tmp = __vsubfp( __vsubfp( __vaddfp(m4, m1), m2), m3);	// (a4 + a1 - a2 - a3)
	tmp = __vmulfp(tmp, vX1 * vZ1);							// fX1*fZ1*
	tmp = __vaddfp(tmp, __vmulfp(__vsubfp(m3, m1), vZ1));	// fZ1*(a3 - a1) + 
	tmp = __vaddfp(tmp, __vmulfp(__vsubfp(m2, m1), vX1));	// fX1*(a2 - a1) + 
	tmp = __vaddfp(tmp, m1);								// a1 +

	__vector4 normalsZW = __vmaddfp(tmp, v_05_4096, v_05_4096);	// 0.0, 0.0f, nx(0..4096), nz(0..4096)
	normalsZW =__vslw(__vctsxs(normalsZW, 0), v_shift_normals);
	__vector4 normalsW = __vpermwi(normalsZW, VPERM(0, 0, 0, 2));
	normalsW = __vadduwm(normalsW, normalsZW);

	tmp = __vpermwi(__vand(tmp, vmask1000), VPERM(1, 0, 1, 1));	// 0, y, 0, 0
	tmp = tmp + _vin;
	tmp = __vperm(tmp, normalsW, vpermControl0127);

	__stvx(tmp, res, 0);
}
#endif

void Ocean3::PrepareIndicesForBlock(int blockIdx)
{
	SeaBlock * cur = &m_blocks[blockIdx];

	float	fStep = fGridStep * float(1 << cur->iLOD);
	float	fSize = fGridStep * cur->iSize;
	int		size0 = cur->iSize >> cur->iLOD;

	cur->iIStart = m_IStart;
	cur->VStart = m_VStart;

	dword * pTriangles2 = pTriangles;

	float x1 = float(cur->iTX * cur->iSize) * fGridStep;
	float y1 = float(cur->iTY * cur->iSize) * fGridStep;
	float x2 = x1 + float(size0) * fStep;
	float y2 = y1 + float(size0) * fStep;

	cur->iX1 = fftoi(x1 / fGridStep); 
	cur->iX2 = fftoi(x2 / fGridStep); 
	cur->iY1 = fftoi(y1 / fGridStep); 
	cur->iY2 = fftoi(y2 / fGridStep); 

	dword * pIndices = m_oceans->m_indices;

	// analyse 
	int i, j;
	for (i=0; i<blockIdx; i++) //if (i == 6)
	{
		SeaBlock * tst = &m_blocks[i];

		// Test Up & Down
		bool bTestedUp = cur->iY1 == tst->iY2;
		bool bTestedDown = cur->iY2 == tst->iY1;

		//if (!(GetAsyncKeyState('5')<0))
		if (bTestedUp || bTestedDown)
		{
			int iAddSrc = tst->iIStart + ((bTestedUp) ? (tst->iSize0 + 1) * 3 : 0);
			int iAddDst = cur->iIStart + ((bTestedUp) ? 0 : (cur->iSize0 + 1) * 3);

			if ((cur->iX1 >= tst->iX1 && cur->iX2 <= tst->iX2) || (cur->iX1 <= tst->iX1 && cur->iX2 >= tst->iX2))
			{
				int iMinX = Max(cur->iX1, tst->iX1);
				int iMaxX = Min(cur->iX2, tst->iX2);
				
				int iStartDstX = cur->iSize0 * (iMinX - cur->iX1) / (cur->iX2 - cur->iX1);
				int iStartSrcX = tst->iSize0 * (iMinX - tst->iX1) / (tst->iX2 - tst->iX1);

				int iEndDstX = cur->iSize0 * (iMaxX - cur->iX1) / (cur->iX2 - cur->iX1);
				int iEndSrcX = tst->iSize0 * (iMaxX - tst->iX1) / (tst->iX2 - tst->iX1);

				if (cur->iLOD == tst->iLOD)
					for (j=iStartDstX; j<=iEndDstX; j++) 
						pIndices[iAddDst + j] = pIndices[iAddSrc + iStartSrcX + (j - iStartDstX)];
				else
					for (j=iStartDstX; j<=iEndDstX; j++) 
						pIndices[iAddDst + j] = pIndices[iAddSrc + iStartSrcX + (j - iStartDstX) / 2];

				continue;
			}
		}

		// Test Left & Right
		bool bTestedLeft = cur->iX1 == tst->iX2;
		bool bTestedRight = cur->iX2 == tst->iX1;
		//if ((GetAsyncKeyState('6')<0))
		if (bTestedLeft || bTestedRight)
		{
			int iAddSrc = tst->iIStart + ((bTestedLeft) ? (tst->iSize0 + 1) * 2 : (tst->iSize0 + 1) * 1);
			int iAddDst = cur->iIStart + ((bTestedLeft) ? (cur->iSize0 + 1) * 1 : (cur->iSize0 + 1) * 2);

			if ((cur->iY1 >= tst->iY1 && cur->iY2 <= tst->iY2) || (cur->iY1 <= tst->iY1 && cur->iY2 >= tst->iY2))
			{
				int iMinY = Max(cur->iY1, tst->iY1);
				int iMaxY = Min(cur->iY2, tst->iY2);
				
				int iStartDstY = cur->iSize0 * (iMinY - cur->iY1) / (cur->iY2 - cur->iY1);
				int iStartSrcY = tst->iSize0 * (iMinY - tst->iY1) / (tst->iY2 - tst->iY1);

				int iEndDstY = cur->iSize0 * (iMaxY - cur->iY1) / (cur->iY2 - cur->iY1);
				int iEndSrcY = tst->iSize0 * (iMaxY - tst->iY1) / (tst->iY2 - tst->iY1);

				if (cur->iLOD == tst->iLOD)
					for (j=iStartDstY; j<=iEndDstY; j++) 
						pIndices[iAddDst + j] = pIndices[iAddSrc + (iStartSrcY + j - iStartDstY)];
				else
					for (j=iStartDstY; j<=iEndDstY; j++) 
						pIndices[iAddDst + j] = pIndices[iAddSrc + (iStartSrcY + (j - iStartDstY) / 2)];

				continue;
			}
		}
	}

	// TO-DO: Возможно стоит сделать копирование 4 краевых элементов в вертикальные линии


	int VStart = m_VStart;
	int VLast = VStart + (size0 - 1) * (size0 - 2);

	// заполняем серединку индексов треугольников (всегда с новыми вертексами)
	dword dyy = size0 - 1;
	dword yy = 0;
	for (int y=0; y<size0-2; y++, yy += dyy)
		for (int x=0; x<size0-2; x++) 
		{
			dword curV = dword(VStart + x + yy);
#ifdef _XBOX
	#define DW_F_2W(b, a)	dword(dword(a) | (dword(b) << 16L))
#else
	#define DW_F_2W(a, b)	dword(dword(a) | (dword(b) << 16L))
#endif

			*pTriangles2++ = DW_F_2W(curV, curV + 1);
			*pTriangles2++ = DW_F_2W(curV + dyy + 1, curV);
			*pTriangles2++ = DW_F_2W(curV + dyy + 1, curV + dyy);
		}

	m_IStart += 4 * (size0 + 1);
	m_TStart += Sqr(size0 - 2) * 2;
	m_VStart += Sqr(size0 - 1);

	dword * upIndices = &pIndices[cur->iIStart];
	dword * leftIndices = upIndices + (size0 + 1);
	dword * rightIndices = leftIndices + (size0 + 1);
	dword * downIndices = rightIndices + (size0 + 1);

	// верхние индексы
	for (int x=0; x<=size0; x++) 
		if (upIndices[x] == -1)
			upIndices[x] = m_VStart++;

	// левые индексы
	for (int y=1; y<size0; y++) 
		if (leftIndices[y] == -1)
			leftIndices[y] = m_VStart++;

	// правые индексы
	for (int y=1; y<size0; y++) 
		if (rightIndices[y] == -1)
			rightIndices[y] = m_VStart++;

	// нижние индексы
	for (int x=0; x<=size0; x++) 
		if (downIndices[x] == -1)
			downIndices[x] = m_VStart++;

	// Добавляем треугольники сверху/снизу/слева/справа
	// левый верхний угол
		*pTriangles2++ = DW_F_2W(upIndices[0], VStart);
		*pTriangles2++ = DW_F_2W(leftIndices[1], upIndices[0]);
		*pTriangles2++ = DW_F_2W(upIndices[1], VStart);
	// правый верхний угол
		*pTriangles2++ = DW_F_2W(upIndices[size0 - 1], rightIndices[1]);
		*pTriangles2++ = DW_F_2W(VStart + size0 - 2, upIndices[size0 - 1]);
		*pTriangles2++ = DW_F_2W(upIndices[size0], rightIndices[1]);
	// левый нижний угол
		*pTriangles2++ = DW_F_2W(leftIndices[size0 - 1], downIndices[1]);
		*pTriangles2++ = DW_F_2W(downIndices[0], leftIndices[size0 - 1]);
		*pTriangles2++ = DW_F_2W(VLast, downIndices[1]);
	// правый нижний угол
		*pTriangles2++ = DW_F_2W(VLast + (size0 - 2), downIndices[size0]);
		*pTriangles2++ = DW_F_2W(downIndices[size0 - 1], VLast + (size0 - 2));
		*pTriangles2++ = DW_F_2W(rightIndices[size0 - 1], downIndices[size0]);

	m_TStart += 4 * 2;

	// заполняем верхнюю линию треугольников
	for (int x=1; x<size0-1; x++) 
	{
		*pTriangles2++ = DW_F_2W(upIndices[x], VStart + x);
		*pTriangles2++ = DW_F_2W(VStart + x - 1, upIndices[x]);
		*pTriangles2++ = DW_F_2W(upIndices[x + 1], VStart + x);
	}

	// заполняем нижнюю линию треугольников
	for (int x=1; x<size0-1; x++) 
	{
		*pTriangles2++ = DW_F_2W(VLast + x - 1, downIndices[x + 1]);
		*pTriangles2++ = DW_F_2W(downIndices[x], VLast + x - 1);
		*pTriangles2++ = DW_F_2W(VLast + x, downIndices[x + 1]);
	}

	// заполняем левую линию треугольников
	for (int y=1; y<size0-1; y++) 
	{
		*pTriangles2++ = DW_F_2W(leftIndices[y], VStart + y * (size0 - 1));
		*pTriangles2++ = DW_F_2W(leftIndices[y + 1], leftIndices[y]);
		*pTriangles2++ = DW_F_2W(VStart + (y - 1) * (size0 - 1), VStart + y * (size0 - 1));
	}

	// заполняем правую линию треугольников
	int VRight = VStart + size0 - 2;
	for (int y=1; y<size0-1; y++) 
	{
		*pTriangles2++ = DW_F_2W(VRight + (y - 1) * (size0 - 1), rightIndices[y + 1]);
		*pTriangles2++ = DW_F_2W(VRight + y * (size0 - 1), VRight + (y - 1) * (size0 - 1));
		*pTriangles2++ = DW_F_2W(rightIndices[y], rightIndices[y + 1]);
	}

	m_TStart += (size0 - 2) * 2 * 4;

	pTriangles = pTriangles2;
}

void Ocean3::WaveXZBlock(SeaBlock & block)
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

	float seaHeight = m_seaHeight + m_seaFakeHeight;

#ifdef _XBOX
	__vector4 vin;
	__vector4 vaddx = XMVectorSet(fStep, 0.0f, 0.0f, 0.0f);
	__vector4 vaddz = XMVectorSet(0.0f, 0.0f, fStep, 0.0f);

	#define	Vector4Set(x,y,z,w)	XMVectorSet(x,y,z,w)
	#define	Vector4Add(a,b)		(a + b)
#else
	Vector4 vin;
	Vector4 vaddx = Vector4(fStep, 0.0f, 0.0f, 0.0f);
	Vector4 vaddz = Vector4(0.0f, 0.0f, fStep, 0.0f);

	#define	Vector4Set(x,y,z,w)	Vector4(x,y,z,w);
	#define	Vector4Add(a,b)		(Vector4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w))
#endif

	// обрабатываем середину (всегда с новыми вертексами)
	for (in.z = y1 + fStep, y=1; y<=size0-1; y++, in.z += fStep)
	{
		vin = Vector4Set(x1 + fStep, seaHeight, in.z, 0.0f);
		for (x=1; x<=size0-2; x+=2)
		{
			WaveXZ(vin, verts);
			vin = Vector4Add(vin, vaddx);
			verts++;

			WaveXZ(vin, verts);
			vin = Vector4Add(vin, vaddx);
			verts++;
		}
		WaveXZ(vin, verts);
		verts++;
	}

	dword * upIndices = &pIndices[block.iIStart];
	dword * leftIndices = upIndices + (size0 + 1);
	dword * rightIndices = leftIndices + (size0 + 1);
	dword * downIndices = rightIndices + (size0 + 1);

	// Верхние вертексы
		vin = Vector4Set(x1, seaHeight, y1, 0.0f);
		//in.z = y1;
		for (x=0; x<=size0; x++, vin = Vector4Add(vin, vaddx))
		{
			if (upIndices[x] >= block.VStart)
			{
				WaveXZ(vin, verts);
				verts++;
			}
		}
	// Левые вертексы
		//in.x = x1;
		vin = Vector4Set(x1, seaHeight, y1 + fStep, 0.0f);
		for (x=1; x<size0; x++, vin = Vector4Add(vin, vaddz))
		{
			if (leftIndices[x] >= block.VStart)
			{
				WaveXZ(vin, verts);
				verts++;
			}
		}
	// Правые вертексы
		//in.x = x2;
		vin = Vector4Set(x2, seaHeight, y1 + fStep, 0.0f);
		for (x=1; x<size0; x++, vin = Vector4Add(vin, vaddz))
		{
			if (rightIndices[x] >= block.VStart)
			{
				WaveXZ(vin, verts);
				verts++;
			}
		}
	// Нижнии вертексы
		//in.z = y2;
		vin = Vector4Set(x1, seaHeight, y2, 0.0f);
		for (x=0; x<=size0; x++, vin = Vector4Add(vin, vaddx))
		{
			if (downIndices[x] >= block.VStart)
			{
				WaveXZ(vin, verts);
				verts++;
			}
		}

	m_pVSea = verts;

	RDTSC_E(dw1);

	m_totalWaveXZRDTSC += dw1;
	m_totalWaveXZVerts += Sqr(size0 + 1);
}
