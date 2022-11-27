#pragma once

#include "..\..\..\common_h\render.h"

class TCone
{
	 Vector *pVectors;
	IRender *pRS;

public:

	 TCone(IRender *_pRS);
	~TCone();

	void Draw(float fRadius, float fAngle, dword dwColor, const Matrix &WorldMat/*,
		      const char *pTechniqueName = "stdPolygon"*/);

};
