#pragma once
#include "ShipAIMovePath.h"

class IsterlingShipAIParams : public ShipWayMoveAIParams
{
public:
	struct Params
	{
		ConstString	targetName;		// имя объекта по которому выравнивать скорость
		float		minDist;		// дистанция увеличения скорости
		float		maxDist;		// дистанция уменьшения скорости
		float		speedBoost;		// коэффициент ускорения/замедления
	};

private:
	Params params_;

	void ReadMOPs(MOPReader & reader);

public:
	IsterlingShipAIParams(void);
	virtual ~IsterlingShipAIParams(void);

	const Params& GetParams() const { return params_; }

	//-----------------------------------------------------------------------------
	// интерфейс, наследуемый от MissionObject
	//-----------------------------------------------------------------------------
	//Инициализировать объект
	virtual bool Create ( MOPReader & reader );
	//Инициализировать объект в режиме редактирования
	virtual bool EditMode_Create ( MOPReader & reader );
	//Обновить параметры в режиме редактирования
	virtual bool EditMode_Update ( MOPReader & reader );
	MO_IS_FUNCTION(IsterlingShipAIParams, ShipWayMoveAIParams);
};
