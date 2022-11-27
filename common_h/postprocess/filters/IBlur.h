#pragma once

#include "..\..\..\Common_h\Render.h"
#include "..\..\..\Common_h\Mission.h"

class IBlur : public IRenderFilter
{
public:

	IBlur()
	{
	}
	virtual ~IBlur()
	{
	}

	virtual void UserInit() = 0;

	virtual void FilterImage(IBaseTexture *source, IRenderTarget *destination) = 0;

	virtual ITexture *GetInputTexture() = 0;

	virtual void SetParams(long BlurPasses, float blurFactor, bool radial) = 0;
	virtual void AddParams(long BlurPasses, float blurFactor, bool radial, float k) = 0;

	virtual bool turnedOff() = 0;

	virtual void enable(bool en, IMission &mis) = 0;
	virtual void update(IMission &mis) = 0;

};
