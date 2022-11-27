#pragma once

#include "..\Common.h"

class SharpnessController : public MissionObject
{
public:
	SharpnessController();
	virtual ~SharpnessController();

	virtual bool Create(MOPReader & reader);
	virtual bool EditMode_Update(MOPReader & reader);

	void __cdecl Execute(float deltaTime, long level);
	//Включение/выключение объекта
	virtual void Activate(bool isActive);

private:
	FiltersParams::SharpnessParams * m_sharpnessParams;

	float m_power;
};