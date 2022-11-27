#include "Box.h"

TBox::POLYGON_VERTEX TBox::InitBoxMesh[] = {

	Vector(-0.50,-0.50, 0.50), 0xffffffff,
	Vector( 0.50,-0.50, 0.50), 0xffffffff,
	Vector( 0.50, 0.50, 0.50), 0xffffffff,
	Vector( 0.50, 0.50, 0.50), 0xffffffff,
	Vector(-0.50, 0.50, 0.50), 0xffffffff,
	Vector(-0.50,-0.50, 0.50), 0xffffffff,
	Vector(-0.50, 0.50,-0.50), 0xffffffff,
	Vector( 0.50,-0.50,-0.50), 0xffffffff,
	Vector(-0.50,-0.50,-0.50), 0xffffffff,
	Vector( 0.50, 0.50,-0.50), 0xffffffff,
	Vector(-0.50, 0.50,-0.50), 0xffffffff,
	Vector( 0.50,-0.50,-0.50), 0xffffffff,

	Vector( 0.50,-0.50,-0.50), 0xffffffff,
	Vector( 0.50,-0.50, 0.50), 0xffffffff,
	Vector( 0.50, 0.50, 0.50), 0xffffffff,
	Vector( 0.50, 0.50, 0.50), 0xffffffff,
	Vector( 0.50, 0.50,-0.50), 0xffffffff,
	Vector( 0.50,-0.50,-0.50), 0xffffffff,
	Vector(-0.50, 0.50,-0.50), 0xffffffff,
	Vector(-0.50,-0.50, 0.50), 0xffffffff,
	Vector(-0.50,-0.50,-0.50), 0xffffffff,
	Vector(-0.50, 0.50, 0.50), 0xffffffff,
	Vector(-0.50, 0.50,-0.50), 0xffffffff,
	Vector(-0.50,-0.50, 0.50), 0xffffffff,

	Vector(-0.50, 0.50,-0.50), 0xffffffff,
	Vector(-0.50, 0.50, 0.50), 0xffffffff,
	Vector( 0.50, 0.50, 0.50), 0xffffffff,
	Vector( 0.50, 0.50, 0.50), 0xffffffff,
	Vector( 0.50, 0.50,-0.50), 0xffffffff,
	Vector(-0.50, 0.50,-0.50), 0xffffffff,
	Vector( 0.50,-0.50,-0.50), 0xffffffff,
	Vector(-0.50,-0.50, 0.50), 0xffffffff,
	Vector(-0.50,-0.50,-0.50), 0xffffffff,
	Vector( 0.50,-0.50, 0.50), 0xffffffff,
	Vector( 0.50,-0.50,-0.50), 0xffffffff,
	Vector(-0.50,-0.50, 0.50), 0xffffffff,
};

TBox:: TBox(IRender *_pRS) : aPolygons(_FL_)
{
	pRS = _pRS;

	pRS->GetShaderId("RagdollPolygon",id);
}

TBox::~TBox()
{
	aPolygons.DelAll();
}

void TBox::Draw(const Vector &vMin, const Vector &vMax, dword dwColor, const Matrix &WorldMat/*,
				const char *pTechniqueName*/)
{
	Vector vScale  = vMax -  vMin;
	Vector vCenter = vMin + (vMax - vMin)/2.0f;

	Matrix OldWorldMat(pRS->GetWorld());

	pRS->SetWorld(WorldMat);
	
	POLYGON_VERTEX pVertex;

	aPolygons.Empty();

	//Заполняем вершины
	for( long i = 0 ; i < 36 ; i++ )
	{		
		pVertex. vPos	=  vScale*InitBoxMesh[i].vPos + vCenter;
		pVertex.dwColor = dwColor;

		aPolygons.Add(pVertex);
	}

//	ShaderId id;
//	pRS->GetShaderId(pTechniqueName,id);

	pRS->DrawPrimitiveUP(id,PT_TRIANGLELIST,12,&aPolygons[0],sizeof(POLYGON_VERTEX));

	pRS->SetWorld(OldWorldMat);
}
