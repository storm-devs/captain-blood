#include "ConeSecktor.h"

TConeSecktor:: TConeSecktor(IRender *_pRS)
{
	pRS = _pRS;

	int a1 = 12;
	int a2 = a1/2 + 1;

	pVectors = NEW Vector[a1*a2*6*2];
}

TConeSecktor::~TConeSecktor()
{
	delete [] pVectors;
}

void TConeSecktor::Draw(float fRadius, float fAngle, float fStartAngle, float fEndAngle,
						dword dwColor, const Matrix &WorldMat/*,
						const char *pTechniqueName*/)
{
	Matrix OldWorldMat(pRS->GetWorld());

	pRS->SetWorld(WorldMat);

	int index = 0;

	const long a1 = 24;
	const long a2 = a1/2 + 1;

	float r1 = sinf(fAngle*PI/180);
	float y1 = cosf(fAngle*PI/180)*fRadius;

	float fTmpAngle = fStartAngle;

	if( fTmpAngle > fEndAngle )
		fTmpAngle -= 360;

	int k  = (int)(fTmpAngle/(360.0f/(float)a1));
	int k2 = (int)(fEndAngle/(360.0f/(float)a1));

	for( int j = k ; j < k2 ; j++ )
	{
		float x1 = sinf((float)(j)*360.0f/(float)a1*PI/180)*fRadius;
		float z1 = cosf((float)(j)*360.0f/(float)a1*PI/180)*fRadius;

		float x2 = sinf((float)(j + 1)*360.0f/(float)a1*PI/180)*fRadius;
		float z2 = cosf((float)(j + 1)*360.0f/(float)a1*PI/180)*fRadius;

		if( j == k )
		{
			x1 = sinf(fStartAngle*PI/180)*fRadius;
			z1 = cosf(fStartAngle*PI/180)*fRadius;
		}

		if ( j + 1 == k2 )
		{
			x2 = sinf(fEndAngle*PI/180)*fRadius;
			z2 = cosf(fEndAngle*PI/180)*fRadius;
		}		

		pVectors[index*3 + 0].x = 0;
		pVectors[index*3 + 0].y = 0;
		pVectors[index*3 + 0].z = 0;

		pVectors[index*3 + 1].x = r1*x1;
		pVectors[index*3 + 1].y = y1;
		pVectors[index*3 + 1].z = r1*z1;				

		pVectors[index*3 + 2].x = r1*x2;
		pVectors[index*3 + 2].y = y1;
		pVectors[index*3 + 2].z = r1*z2;

		index++;
	}

	pRS->DrawPolygon(pVectors,index*3,dwColor,WorldMat,/*pTechniqueName*/"RagdollPolygon");

	pRS->SetWorld(OldWorldMat);
}
