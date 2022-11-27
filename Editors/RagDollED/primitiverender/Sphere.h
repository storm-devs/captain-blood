#ifndef SPHERE_RENDER_H
#define SPHERE_RENDER_H



#include "..\..\..\common_h\render.h"


class TSphere
{
	struct POLYGON_VERTEX
	{
		Vector		vPos;
		dword		dwColor;
	};

	array<POLYGON_VERTEX> aPolygons;
	IRender* pRS;

public:

	TSphere(IRender* _pRS);
	~TSphere();


	void Draw(float fRadius, dword dwColor, Matrix& WorldMat,
		      const char * pTechniqueName = "stdPolygon");

};

#endif