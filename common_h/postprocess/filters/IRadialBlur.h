#pragma once

#include "..\..\..\Common_h\Render.h"
#include "..\..\..\Common_h\Mission.h"

class IRadialBlur : public IRenderFilter
{
public:

	IRadialBlur()
	{
	}
	virtual ~IRadialBlur()
	{
	}

	virtual void UserInit() = 0;

	virtual void FilterImage(IBaseTexture *source, IRenderTarget *destination) = 0;

	virtual ITexture *GetInputTexture() = 0;

	virtual void SetParams(long blurPasses, float blurFactor, float blurScale, bool linear) = 0;
	virtual void SetCenter(const Vector &blurCenter) = 0;

	virtual void AddParams(long blurPasses, float blurFactor, float blurScale, bool linear, float k) = 0;
	virtual void AddCenter(const Vector &blurCenter) = 0;

	virtual void AddFocus(const Vector &dir, float asp, float rad) = 0;

	virtual bool turnedOff() = 0;

	virtual void enable(bool en, IMission &mis) = 0;
	virtual void update(IMission &mis) = 0;

};
