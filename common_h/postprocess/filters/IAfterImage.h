#pragma once

#include "..\..\..\Common_h\Render.h"
#include "..\..\..\Common_h\Mission.h"

class IAfterImage : public IRenderFilter
{
public:

	IAfterImage()
	{
	}
	virtual ~IAfterImage()
	{
	}

	virtual void UserInit() = 0;

	virtual void FilterImage(IBaseTexture *source, IRenderTarget *destination) = 0;

	virtual ITexture *GetInputTexture() = 0;

	virtual void SetParams(float  fOriginal, float  fAfterImage) = 0;
	virtual void GetParams(float &fOriginal, float &fAfterImage) = 0;

	virtual void Reset() = 0;

	virtual bool turnedOff() = 0;

};
