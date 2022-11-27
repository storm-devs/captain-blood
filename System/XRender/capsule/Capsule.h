#ifndef CAPSULE_RENDER_H
#define CAPSULE_RENDER_H



#include "..\..\..\common_h\render.h"


class TCapsule
{
	struct POLYGON_VERTEX
	{
		Vector		vPos;
		dword		dwColor;
	};

	array<POLYGON_VERTEX> aPolygons;
	IRender* pRS;

public:

	TCapsule(IRender* _pRS);
	~TCapsule();


	void Draw(float fRadius, float fHeight, dword dwColor, const Matrix &WorldMat,
		      const char * pTechniqueName = "stdPolygon");

};

#endif