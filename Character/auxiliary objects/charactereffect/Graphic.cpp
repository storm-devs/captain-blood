
#include "Graphic.h"



Graphic::Graphic():points(_FL_)
{

}

Graphic::~Graphic()
{
	Release();
}

void Graphic::Release()
{
	points.DelAll();
}

void Graphic::ReadParams(MOPReader &reader)
{
	points.Empty();

	int count = reader.Array();

	for (int i=0;i<count;i++)
	{
		TPoint* point = &points[points.Add()];

		point->time = reader.Float();
		point->value = reader.Float();
	}
}

float Graphic::ReadGraphic(float time)
{
	if (time<points[0].time)
	{
		return points[0].value;
	}

	if (time>points[points.Last()].time)
	{
		return points[points.Last()].value;
	}

	for (int i=0;i<(int)points.Size()-1;i++)
	{
		if (time<points[i+1].time)
		{
			float k = (time-points[i].time)/(points[i+1].time-points[i].time);

			return points[i].value + (points[i+1].value-points[i].value) * k;
		}
	}

	return 0.0f;
}

float Graphic::GraphicMaxTime()
{
	return points[points.Last()].time;
}
