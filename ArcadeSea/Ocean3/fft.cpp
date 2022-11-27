// =====================================
// Сборник общих функций для PC/XBox FFT
// =====================================

#include "fft.h"

int fft::refCount = 0;
char * fft::m_polarsUnaligned = null;
fft::dcomplex *	fft::m_polars = null;

fft::fft()
{
	m_curFrameUnaligned[0] = null;
	m_curFrameUnaligned[1] = null;

	m_curFrame = null;
	m_nextFrame = null;

	Construct();

	if (refCount == 0)
	{
		m_polarsUnaligned = NEW char[sizeof(dcomplex) * 4096 + 16];
		m_polars = (dcomplex*)((dword)m_polarsUnaligned + 15 & ~15);

		for (int n=0; n<4096; n++)
		{
			m_polars[n].c1 = Polar(1.0f, float(float(n) / 4096.0f * PIm2));
			m_polars[n].c2 = ~m_polars[n].c1;
		}
	}
	refCount++;
}

fft::~fft()
{
	DELETE(m_curFrameUnaligned[0]);
	DELETE(m_curFrameUnaligned[1]);

	Destruct();

	refCount--;
	if (refCount == 0)
	{
		DELETE(m_polarsUnaligned);
	}
}

void fft::ChangeCurFrame()
{
	Swap(m_curFrame, m_nextFrame);
}
