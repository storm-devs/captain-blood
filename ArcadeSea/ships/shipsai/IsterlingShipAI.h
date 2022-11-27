#pragma once
#include "shipaimovepath.h"

class IsterlingShipAI :	public MovePath_ShipAI
{
	MOSafePointer distTarget_;		// объект по которому выравнивать дистанцию
	float minDist_;					// минимальная дистанция, после которой увеличивать скорость
	float maxDist_;					// максимальная дистанция, после которой уменьшать скорость
	float speedBoost_;

	// посчитать скорость
	virtual float CalcVelocity();
	// установить объект параметров
	virtual void SetControllerParams( IShipControllerParams * pControllerParams );

public:
	IsterlingShipAI(IShip& ship, IMission& mission);
	virtual ~IsterlingShipAI(void);
};
