#include "Pie.h"

TPie:: TPie(IRender *_pRS)
{
	pRS = _pRS;

	int a1 = 12;
	int a2 = a1/2 + 1;

	pVectors = NEW Vector[a1*a2*6*2];
}

TPie::~TPie()
{
	delete [] pVectors;
}

void TPie::Draw(float fRadius, float fStartAngle, float fEndAngle,
				dword dwColor, const Matrix &WorldMat/*,
				const char *pTechniqueName*/)
{
	Matrix OldWorldMat(pRS->GetWorld());

	pRS->SetWorld(WorldMat);

	int a1 = 24;

	int index = 0;

	float fTmpAngle = fStartAngle;

	if( fTmpAngle > fEndAngle )
		fTmpAngle -= 360;

	int k  = (int)(fTmpAngle/(360.0f/(float)a1));
	int k2 = (int)(fEndAngle/(360.0f/(float)a1));

	for( int j = k ; j < k2 ; j++ )
	{
		float x1 = sinf(-(float)(j)*360.0f/(float)a1*PI/180)*fRadius;
		float y1 = cosf(-(float)(j)*360.0f/(float)a1*PI/180)*fRadius;

		float x2 = sinf(-(float)(j + 1)*360.0f/(float)a1*PI/180)*fRadius;
		float y2 = cosf(-(float)(j + 1)*360.0f/(float)a1*PI/180)*fRadius;

		if( j == k )
		{
			x1 = sinf(-fStartAngle*PI/180)*fRadius;
			y1 = cosf(-fStartAngle*PI/180)*fRadius;
		}

		if( j + 1 == k2 )
		{
			x2 = sinf(-fEndAngle*PI/180)*fRadius;
			y2 = cosf(-fEndAngle*PI/180)*fRadius;
		}

		pVectors[index*3 + 1].x =  0;
		pVectors[index*3 + 1].y = y1;
		pVectors[index*3 + 1].z = x1;

		pVectors[index*3 + 0].x =  0;
		pVectors[index*3 + 0].y =  0;
		pVectors[index*3 + 0].z =  0;

		pVectors[index*3 + 2].x =  0;
		pVectors[index*3 + 2].y = y2;
		pVectors[index*3 + 2].z = x2;

		index++;
	}

	pRS->DrawPolygon(pVectors,index*3,dwColor,WorldMat,/*pTechniqueName*/"RagdollPolygon");

	pRS->SetWorld(OldWorldMat);
}
