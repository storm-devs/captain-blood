#pragma once

#include "..\Common.h"

class DepthOfField : public MissionObject
{
public:
	DepthOfField();
	virtual ~DepthOfField();

	virtual bool Create(MOPReader & reader);
	virtual void PostCreate();
	virtual bool EditMode_Update(MOPReader & reader);

	void __cdecl Execute(float deltaTime, long level);
	//Включение/выключение объекта
	virtual void Activate(bool isActive);

private:
	bool m_active;
	float m_blurFactor, blurPower, m_focusDistance, m_focusPoint;
	bool m_blurFar, m_blurNear;
	IPostEffects * peService;
	FiltersParams::DOFParams * m_DOFParams;

	float fadeInTime, fadeOutTime;
	bool fadeStarted, fadeOut;
	float startPower, endPower;
	float curTime, fadeTime;
};