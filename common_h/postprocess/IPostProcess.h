#pragma once

#include "..\Core.h"
#include "..\Render.h"

#include "Filters\IAfterImage.h"
#include "Filters\IBlur.h"
#include "Filters\IMotionBlur.h"
#include "Filters\IRadialBlur.h"

//Простейший сервис, просто добавляет фильтры в рендер....
class IPostProcessService : public Service
{
public:

	IPostProcessService()
	{
	}
	virtual ~IPostProcessService()
	{
	}

	//virtual IBlur		*GetBlurFilter		() = 0;
	//virtual IMotionBlur *GetMotionBlurFilter() = 0;

};
