#include "Cone.h"

TCone:: TCone(IRender *_pRS)
{
	pRS = _pRS;

	int a1 = 12;
	int a2 = a1/2 + 1;

	pVectors = NEW Vector[a1*a2*6*2];
}

TCone::~TCone()
{
	delete [] pVectors;
}

void TCone::Draw(float fRadius, float fAngle, dword dwColor, const Matrix &WorldMat/*,
				 const char *pTechniqueName*/)
{
	Matrix OldWorldMat(pRS->GetWorld());

	pRS->SetWorld(WorldMat);

	int index = 0;

	const long a1 = 24;
	const long a2 = a1/2 + 1;

	float r1 = sinf(fAngle*PI/180);
	float y1 = cosf(fAngle*PI/180)*fRadius;

	for( long j = 0 ; j < a1 ; j++ )
	{
		float x1 = sinf(2.0f*PI*j/float(a1))*fRadius;
		float z1 = cosf(2.0f*PI*j/float(a1))*fRadius;

		float x2 = sinf(2.0f*PI*(j + 1)/float(a1))*fRadius;
		float z2 = cosf(2.0f*PI*(j + 1)/float(a1))*fRadius;

		pVectors[index*3 + 0].y = 0;
		pVectors[index*3 + 0].x = 0;
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
