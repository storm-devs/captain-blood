#include "IsterlingShipAI.h"
#include "IsterlingShipAIParams.h"

IsterlingShipAI::IsterlingShipAI(IShip& ship, IMission& mission) :
MovePath_ShipAI(ship, mission)
{
}

IsterlingShipAI::~IsterlingShipAI(void)
{
}

float IsterlingShipAI::CalcVelocity()
{
	float vel = MovePath_ShipAI::CalcVelocity();

	if (distTarget_.Ptr())
	{
		Matrix mtx;
		distTarget_.Ptr()->GetMatrix(mtx);
		float dist = (mtx.pos - GetShip().GetPosition()).GetLength();

		if ( dist > maxDist_ )
			vel /= speedBoost_;
		if ( dist < minDist_ )
			vel *= speedBoost_;
	}

	return vel;
}

void IsterlingShipAI::SetControllerParams( IShipControllerParams * controllerParams )
{
	MO_IS_IF(tid, "IsterlingShipAIParams", controllerParams)
	{
		IsterlingShipAIParams* params = (IsterlingShipAIParams*)controllerParams;

		params->FindObject(params->GetParams().targetName, distTarget_);

		minDist_ = params->GetParams().minDist;
		maxDist_ = params->GetParams().maxDist;
		speedBoost_ = params->GetParams().speedBoost;

		MovePath_ShipAI::SetControllerParams(controllerParams);
	}
	else
		api->Trace("Invalid parameter object passed for IsterlingShipAI.");
}