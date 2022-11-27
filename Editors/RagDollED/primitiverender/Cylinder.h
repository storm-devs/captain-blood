#ifndef CYLINDER_RENDER_H
#define CYLINDER_RENDER_H



#include "..\..\..\common_h\render.h"


class TCylinder
{
	struct POLYGON_VERTEX
	{
		Vector		vPos;
		dword		dwColor;
	};

	array<POLYGON_VERTEX> aPolygons;
	IRender* pRS;

public:

	TCylinder(IRender* _pRS);
	~TCylinder();


	void Draw(float fRadius, float fHeight, dword dwColor, Matrix& WorldMat,
		      const char * pTechniqueName = "stdPolygon");

};

#endif