#include "capsule.h"



TCapsule::TCapsule(IRender* _pRS) : aPolygons(_FL_)
{
	pRS=_pRS;	
}

TCapsule::~TCapsule()
{
	aPolygons.DelAll();
}

void TCapsule::Draw(float fRadius, float fHeight, dword dwColor, const Matrix &WorldMat,
					const char * pTechniqueName)
{
	ShaderId shaderId;
	pRS->GetShaderId(pTechniqueName, shaderId);


	Matrix OldWorldMat;

	OldWorldMat=pRS->GetWorld();

	pRS->SetWorld(WorldMat);

	int index=0;
	
	const long a1 = 12;
	const long a2 = (a1/2)+1;

	POLYGON_VERTEX pVertex;

	aPolygons.Empty();

	//Заполняем вершины
	for(long i = 0 ; i < a2; i++)
	{
		float dHgt=fHeight*0.5f;

		float r1 = sinf(PI*i/float(a2));
		float y1 = cosf(PI*i/float(a2))*fRadius;

		float r2 = sinf(PI*(i + 1)/float(a2));
		float y2 = cosf(PI*(i + 1)/float(a2))*fRadius;

		if (y1>0)
		{
			y1+=dHgt;			
		}
		else
		{
			y1-=dHgt;		
		}

		if (y2>0)
		{			
			y2+=dHgt;
		}
		else
		{		
			y2-=dHgt;
		}


		for(long j = 0; j < a1; j++)
		{
			float x1 = sinf(2.0f*PI*j/float(a1))*fRadius;
			float z1 = cosf(2.0f*PI*j/float(a1))*fRadius;
			float x2 = sinf(2.0f*PI*(j + 1)/float(a1))*fRadius;
			float z2 = cosf(2.0f*PI*(j + 1)/float(a1))*fRadius;
			//0
			pVertex.vPos.x = r1*x1;
			pVertex.vPos.y = y1;
			pVertex.vPos.z = r1*z1;

			pVertex.dwColor=dwColor;

			aPolygons.Add(pVertex);

			//1
			pVertex.vPos.x = r2*x1;
			pVertex.vPos.y = y2;
			pVertex.vPos.z = r2*z1;

			pVertex.dwColor=dwColor;

			aPolygons.Add(pVertex);

			//2
			pVertex.vPos.x = r1*x2;
			pVertex.vPos.y = y1;
			pVertex.vPos.z = r1*z2;

			pVertex.dwColor=dwColor;

			aPolygons.Add(pVertex);
			//3 = 2

			aPolygons.Add(pVertex);

			aPolygons[index*3 + 3] = aPolygons[index*3 + 2];
			//4 = 1

			aPolygons.Add(pVertex);

			aPolygons[index*3 + 4] = aPolygons[index*3 + 1];
			//5
			pVertex.vPos.x = r2*x2;
			pVertex.vPos.y = y2;
			pVertex.vPos.z = r2*z2;

			pVertex.dwColor=dwColor;

			aPolygons.Add(pVertex);

			index+=2;
		}
	}
			
	pRS->DrawPrimitiveUP(shaderId, PT_TRIANGLELIST, index, &aPolygons[0], sizeof(POLYGON_VERTEX));

	pRS->SetWorld(OldWorldMat);
}