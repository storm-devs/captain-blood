#ifndef _Graphic_h_
#define _Graphic_h_


#include "..\Mission.h"

#define MOP_GRAPHIC(name,comment) MOP_ARRAYBEGC(name, 2, 1000000, comment)  \
									MOP_FLOATEX("time", 0.0f, 0.0f, 1024.0f) \
									MOP_FLOATEX("value", 0.0f, 0.0f, 1.0f) \
									MOP_ARRAYEND

class Graphic
{	
	struct TPoint
	{
		float time;
		float value;
	};

	array<TPoint> points;

public:

	Graphic();
	virtual ~Graphic();

	void  ReadParams(MOPReader &reader);
	float ReadGraphic(float time);
	float GraphicMaxTime();
	void  Release();
};

#endif











