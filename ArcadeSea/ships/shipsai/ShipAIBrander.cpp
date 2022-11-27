
#include "ShipAIBrander.h"

//============================================================================================
//реализация класса АИ брандера
//============================================================================================
Brander_ShipAI::Brander_ShipAI (  IShip & rShip, IMission & rMission  )
: ShipAI ( rShip, rMission )
, m_pTarget  ( NULL )
, m_pShipBranderAIParams ( NULL )
, m_bPursuit ( false )
{
}

Brander_ShipAI ::~Brander_ShipAI ()
{
}

// управление кораблем
void	Brander_ShipAI ::Work ( float dltTime )
{
	ShipAI::Work( dltTime );

	if ( !m_pTarget || dltTime < 1e-3f )
	{
		StopMove ();
		return;
	}

	float fCurrVelocity = m_rShip.GetCurrentVelocity ();

	if ( fCurrVelocity < 0.1f )
		fCurrVelocity = 0.1f;

	Vector vTargetPos = m_pTarget->GetPosition ();
	//float fTimeToCatch = ( m_rShip.GetPosition() - vTargetPos ).GetLength () / fCurrVelocity;
	SetObstacleExclusion ( m_pTarget );

	float fTargetAngle = m_pTarget->GetCurrentDirectionAngle ();
	Vector vEstimateTargetDirection;
	vEstimateTargetDirection.MakeXZ ( fTargetAngle );

	//Vector vEstimateTargetVelocity = vEstimateTargetDirection*m_pTarget->GetCurrentVelocity ();
	//Vector vRealTargetPos = vTargetPos + vEstimateTargetVelocity*fTimeToCatch;

	Vector vRealTargetPos = vTargetPos + 0.5f*vEstimateTargetDirection*m_fShipRadiusZ;

	SetTargetPos ( vTargetPos, NODE_STATE_EXACTLY  );

	// обработка движения в заданную точку
	bool bNodeReached = MoveProc ( dltTime );

#ifdef DEBUG_DRAW
	DrawDebugInfo ();
#endif
};

// установить объект параметров
void	Brander_ShipAI ::SetControllerParams ( IShipControllerParams * pControllerParams  )
{
	Assert ( pControllerParams );
	ShipAI::SetControllerParams(pControllerParams);

	MO_IS_IF(tid, "ShipBranderAIParams", pControllerParams)
	{
		ShipBranderAIParams * pBranderAIParams =  static_cast <ShipBranderAIParams *> ( pControllerParams );
		Assert ( pBranderAIParams );

		m_pShipBranderAIParams = pBranderAIParams;

		if  ( m_pShipBranderAIParams->m_pTarget )
		{
			m_pTarget = m_pShipBranderAIParams->m_pTarget;
		}
	}
}

// получить текущее АИ-состояние
ShipControlType	Brander_ShipAI::GetType () const
{
	return SHIP_AI_STATE_BRANDER;
}

// получить  объект преследования
IShip *	 Brander_ShipAI::GetTargetShip () const
{
	return m_pTarget;
}

// отрисовка дебажной инфы
void Brander_ShipAI::DrawDebugInfo ()
{
}
