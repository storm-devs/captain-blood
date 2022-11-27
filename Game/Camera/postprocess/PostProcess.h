#pragma once

#include "..\..\..\Common_h\PostProcess\IPostProcess.h"

#include "Filters\BlackAndWhite.h"
#include "Filters\Blur.h"
#include "Filters\MotionBlur.h"
#include "Filters\MotionEffect.h"
#include "Filters\Splash.h"


//Простейший сервис, просто добавляет фильтры в рендер....
class PostProcessService : public IPostProcessService
{
	IRender *pRS;

	//BlackAndWhite filter_bw;
	//Blur		  filter_blur;
	//MotionBlur	  filter_motion;
	//MotionEffect  filter_effect;
	Splash		  filter_splash;

	struct Node
	{
		const char *effect;
	//	const char *object;
		string object;

		float weight;
		float k;

		Node()
		{
			reset();
		}

		bool operator !() const
		{
		//	return !effect || !object;
			return !effect;
		}

		void set(const char *effect, const char *object, float weight, float k)
		{
			this->effect = effect;
			this->object = object;

			this->weight = weight;
			this->k		 = k;
		}

		void reset()
		{
			set(null,null,0.0f,0.0f);
		}
	};

	array<Node> nodes; int count; float weight;

	bool empty;

	bool trace;
	bool key;

	bool dflag;
	bool enabled;

public:

	PostProcessService();
	virtual ~PostProcessService();

	virtual bool Init();

	virtual void StartFrame(float dltTime);
	virtual void   EndFrame(float dltTime);

	void FilterNotify(const char *effect, const char *object, float weight, float k);

	//virtual IBlur		*GetBlurFilter		();
	//virtual IMotionBlur *GetMotionBlurFilter();

	//BlackAndWhite *GetBlackFilter();

	//MotionEffect  *GetMotionEffectFilter();

	Splash		  *GetSplashFilter();

public:

	void _cdecl Console_EnableDraw(const ConsoleStack &params);
	void _cdecl Console_EnableList(const ConsoleStack &params);

public:

	bool Enabled()
	{
		return enabled;
	}

};
