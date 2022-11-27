#pragma once

#include "Mission.h"

struct IGUIText
{
	enum Align {Left,Center,Right,Top,Bottom};

	virtual bool Prepare(IFont *font, float w, float h, const char *text,
		Align hor = Center,
		Align ver = Center, bool useSafeFrame = true) = 0;

	struct Effect
	{
		dword Beg;
		dword Len; float Width;
	};

	virtual void Draw	(IFont *font, float x, float y,
		bool animate = true,
		Effect  *eff = null) = 0;

	virtual void Update(float time) = 0;

	virtual void SetScale(float k) = 0;

	virtual void SetColor(const Color &c) = 0;
	virtual void SetAlpha(float a) = 0;

	virtual void GetPos(float &x, float &y) = 0;

	virtual void Release() = 0;
};

struct IGUITextManager : public MissionObject
{
	virtual IGUIText *CreateText() = 0;
};
