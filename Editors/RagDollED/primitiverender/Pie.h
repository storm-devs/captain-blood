#pragma once

#include "..\..\..\common_h\render.h"

class TPie
{
	 Vector *pVectors;
	IRender *pRS;

public:

	 TPie(IRender *_pRS);
	~TPie();

	void Draw(float fRadius, float fStartAngle, float fEndAngle,
			  dword dwColor, const Matrix &WorldMat/*,
			  const char *pTechniqueName = "stdPolygon"*/);

};
