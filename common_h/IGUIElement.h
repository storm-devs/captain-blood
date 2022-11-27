#pragma once

#include "Mission.h"

class IGUIElement : public MissionObject
{
public:

/*	virtual void Draw(float x, float y, float alpha = 1.0f, bool inner = false)
	{
	}

	virtual bool IsPlayingAnimation()
	{
		return false;
	}

	virtual void GetPosition(float &x, float &y) {}
	virtual void SetPosition(float  x, float  y) {}

	virtual void GetSize(float &w, float &h) {}
	virtual void SetSize(float  w, float  h) {}*/

//	//// DELETE THIS ////
//	virtual void Draw(float x, float y, float alpha = 1.0f, bool inner = false)
//	{
//	}
//	/////////////////////

	enum Layout{OnCenter,OnLeft,OnRight};

public:

	virtual void Draw() = 0;			// отрисовать с текущими параметрами
	virtual void Update(float dltTime) = 0;		// обновить состоние

	virtual bool IsPlayingAnimation()
	{
		return false;
	}

	virtual void GetPosition(float &x, float &y) = 0;
	virtual void SetPosition(float  x, float  y) = 0;

	virtual void GetSize(float &w, float &h) = 0;
	virtual void SetSize(float  w, float  h) = 0;

	virtual float GetAlpha() = 0;
	virtual float GetNativeAlpha() = 0;

	virtual void  SetAlpha(float val) = 0;

	virtual Layout GetAlign() = 0;
	virtual void   SetAlign(Layout align) = 0;

	////

	virtual void SetFontScale(float k)
	{
	}

	virtual void GetNatPosition(float &x, float &y) = 0;
	virtual void GetNatSize	   (float &w, float &h) = 0;

	virtual Layout GetNatAlign() = 0;

	////

public:

	virtual float GetAspect() = 0;

	MO_IS_FUNCTION(IGUIElement, MissionObject);

};
