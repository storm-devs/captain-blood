#pragma once
#ifndef _RENDER_ADVANCED_FONT_
#define _RENDER_ADVANCED_FONT_

#include "IResource.h"


class IAdvFont : public IResource
{

protected:

	IAdvFont() {};
	virtual ~IAdvFont() {};

public:

	enum Align
	{
		Left,
		Center,
		Right,
		Top,
		Bottom
	};

	struct Effect
	{
		dword Beg;
		dword Len; float Width;
	};


	//Координаты относительно экрана 0.0, левый край и верх, 1.0 правый край и низ
	virtual void Print(bool coreThread, float relative_x, float relative_y, float relative_w, float relative_h, const char *text, Align hor = Center, Align ver = Center, bool useSafeFrame = true, bool animate = true, Effect * eff = null) = 0;


	virtual void Update(float time) = 0;

	virtual void SetScale(float k) = 0;

	virtual void SetColor(const Color &c) = 0;

	virtual void SetAlpha(float a) = 0;

	virtual void GetPos(float &x, float &y) = 0;

	virtual void SetKerning (float kerningValue) = 0;
};


#endif
