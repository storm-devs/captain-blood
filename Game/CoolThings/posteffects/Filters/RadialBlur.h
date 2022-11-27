#pragma once

#include "..\Common.h"

class RadialBlurController : public MissionObject
{
public:
	RadialBlurController();
	virtual ~RadialBlurController();

	virtual bool Create(MOPReader & reader);
	virtual void PostCreate();
	virtual bool EditMode_Update(MOPReader & reader);

	void __cdecl Execute(float deltaTime, long level);
	void __cdecl Realize(float deltaTime, long level);

	//Включение/выключение объекта
	virtual void Activate(bool isActive);

private:
	FiltersParams::RadialBlurParams * radialBlurParams;

	bool active;

	float focus;
	float power;
	float passes;
	float blurFactor;
	float blurScale;

	float fadeInTime, fadeOutTime;
	bool fadeStarted, fadeOut;
	float startPower, endPower;
	float curTime, fadeTime;

	ConstString targetName;
	Vector targetOffset;
	MOSafePointer targetPtr;
	Vector blurPosition, blurCenter;

};