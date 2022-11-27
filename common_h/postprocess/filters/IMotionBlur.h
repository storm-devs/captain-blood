#pragma once

#include "..\..\..\Common_h\Render.h"
#include "..\..\..\Common_h\Mission.h"

class IMotionBlur : public IRenderFilter
{
public:

	IMotionBlur()
	{
	}
	virtual ~IMotionBlur()
	{
	}

	virtual void UserInit() = 0;

	virtual void FilterImage(IBaseTexture *source, IRenderTarget *destination) = 0;

	virtual ITexture *GetInputTexture() = 0;

	virtual void SetParams(float  fOriginal, float  fAfterImage) = 0;
	virtual void GetParams(float &fOriginal, float &fAfterImage) = 0;

	virtual bool turnedOff() = 0;

};
