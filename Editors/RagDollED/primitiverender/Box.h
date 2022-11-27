#pragma once

#include "..\..\..\common_h\render.h"

class TBox
{
	struct POLYGON_VERTEX
	{
		Vector vPos; dword dwColor;
	};

	static POLYGON_VERTEX InitBoxMesh[]; array<POLYGON_VERTEX> aPolygons;

	IRender *pRS;

	ShaderId id;

public:

	 TBox(IRender *_pRS);
	~TBox();

	void Draw(const Vector &vMin, const Vector &vMax, dword dwColor, const Matrix &WorldMat/*,
		      const char *pTechniqueName = "stdPolygon"*/);

};
