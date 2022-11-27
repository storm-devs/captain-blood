#pragma once

#include "..\Common.h"

class GlowController : public MissionObject
{
public:
	GlowController();
	virtual ~GlowController();

	virtual bool Create(MOPReader & reader);
	virtual void PostCreate();
	virtual bool EditMode_Update(MOPReader & reader);

	void __cdecl Execute(float deltaTime, long level);
	//Включение/выключение объекта
	virtual void Activate(bool isShow);

private:
	array<FiltersParams::GlowObject> glowObjects;
	FiltersParams::GlowParams * glowParams;

	float glowPower, initialPower;
	float blurriness;
	float threshold;
	float backFactor, foreFactor;
	float realColors;
	bool highQuality;
	bool postCreateDone;
	
	float fadeInTime, fadeOutTime;
	bool fadeStarted, fadeOut;
	float startPower, endPower;
	float curTime, fadeTime;
};