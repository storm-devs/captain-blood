#pragma once

#include "..\..\..\common_h\render.h"

class TConeSecktor
{
	 Vector *pVectors;
	IRender *pRS;

public:

	 TConeSecktor(IRender *_pRS);
	~TConeSecktor();

	void Draw(float fRadius, float fAngle, float fStartAngle, float fEndAngle,
			  dword dwColor, const Matrix &WorldMat/*,
			  const char *pTechniqueName = "stdPolygon"*/);

};
