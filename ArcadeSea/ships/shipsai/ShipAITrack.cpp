
#include "ShipAITrack.h"
#include "..\Ship.h"


//============================================================================================
//реализация класса АИ боя
//============================================================================================
TrackMove_ShipAI::TrackMove_ShipAI (  IShip & rShip, IMission & rMission  ) :
	ShipAI ( rShip, rMission )
{
	m_pShipTrackAIParams = NULL;
	m_vTargetPos = 0.f;
	m_bShowDebugInfo = false;
};

TrackMove_ShipAI ::~TrackMove_ShipAI ()
{
};

// управление кораблем
void	TrackMove_ShipAI ::Work ( float dltTime )
{
	Matrix mtxShip(true);
	Matrix mtxOffset(true);
	m_pShipTrackAIParams->GetMatrix(mtxOffset);

	Matrix mtx(true);//( m_rShip.GetMatrix(mtxShip), mtxOffset );
	m_rShip.GetMatrix(mtx);

	const ShipWayPoint& wp = m_shipWayTrack.GetWayPoint( dltTime, mtx );

	if( wp.bFollowToPoint )
	{
		ShipAI::Work(dltTime);

		m_vTargetPos = wp.pos;//mtxOffset.MulVertexByInverse(wp.pos);
		SetTargetPos ( m_vTargetPos, NODE_STATE_EXACTLY );

		// ограничение скорости
		float fOptimumSpeed = wp.speedK * m_rShip.GetMaxVelocity();
		// если таргет находится не по курсу ( cos(угла до цели)<0.7 ), то уменьшаем максимальную скорость в 10 раз
		if( ( (!(m_vTargetPos - mtx.pos)) | mtx.vz ) < 0.7f )
			fOptimumSpeed *= 0.1f;
		SetOptimumSpeed( fOptimumSpeed );

		// обработка движения в заданную точку
		m_rShip.SetSideVelocity( 0.f );
		MoveProc( dltTime );
	}
	else
	{
		m_rShip.SetRotate( wp.rotate );
		m_rShip.SetVelocity( wp.velocity );
		m_rShip.SetSideVelocity( wp.offset );
	}

	if( IsInsideAIZone() )
		ProcessAIFire( dltTime );

	if( m_bShowDebugInfo )
		DrawDebugInfo ();
};

// установить объект параметров
void TrackMove_ShipAI ::SetControllerParams ( IShipControllerParams * pControllerParams  )
{
	Assert ( pControllerParams );
	ShipAI::SetControllerParams(pControllerParams);

	MO_IS_IF(tid, "ShipTrackAIParams", pControllerParams)
	{
		m_pShipTrackAIParams = (ShipTrackAIParams*)pControllerParams;
		Assert ( m_pShipTrackAIParams );

		m_shipWayTrack.InitWayTrack( m_pShipTrackAIParams );
		m_shipWayTrack.ResetTrack();

		m_bShowDebugInfo = m_pShipTrackAIParams->m_bShowDebugInfo;
	};

	SetOptimumSpeed ( m_rShip.GetMaxVelocity () );
	m_vTargetPos = m_rShip.GetPosition ();
};

// получить текущее АИ-состояние
ShipControlType	TrackMove_ShipAI ::GetType ()
{
	return SHIP_AI_STATE_WAY_MOVE;
};

// получить  объект преследования
MissionObject *	 TrackMove_ShipAI ::GetEnemy ()
{
	return m_shipWayTrack.GetEnemy();
};

// АИ стрельбы
void	TrackMove_ShipAI ::ProcessAIFire ( float fDTime )
{
	if ( !GetEnemy() || GetEnemy()->IsDead() )
		return;

	Vector vShipPos = m_rShip.GetPosition ();
	Matrix tmEnemyMatrix;
	GetEnemy()->GetMatrix ( tmEnemyMatrix );
	Vector vEnemPos = tmEnemyMatrix.pos;

	Vector vShipToEnemy = vEnemPos - vShipPos;
	float fEnemyDist = vShipToEnemy.GetLength ();

	m_rShip.FireCannons ( GetEnemy(), 0.f );
	m_rShip.FireFlamethrower();
	m_rShip.FireMines();
};

void TrackMove_ShipAI::StopAttacking ()
{
};

// отрисовать дебажную инфу
void TrackMove_ShipAI::ViewDebugInfo()
{
	// показать от базового класса
	ShipAI::ViewDebugInfo();

	DrawDebugInfo ();

	Matrix mtx(true);
	m_rShip.GetMatrix(mtx);
	MissionObject* enemy = GetEnemy();

	IRender & tRender = m_rMission.Render ();
	tRender.Print( mtx.pos + Vector(0.f,60.f,0.f), 0.0f, 0.0f, 0xFFFFFFFF,
		"AI type: track AI\n"
		"AI params: %s\n"
		"Enemy: %s\n",
		m_pShipTrackAIParams==NULL ? "none" : m_pShipTrackAIParams->GetObjectID().c_str(),
		enemy==NULL ? "none" : enemy->GetObjectID().c_str()
	);
}

// отрисовка дебажной инфы
void TrackMove_ShipAI::DrawDebugInfo ()
{
	ShipAI::DrawDebugInfo ();

	IRender & tRender = m_rMission.Render ();

	//Компенсируем свинг, если активна опция
	Matrix oldview(true);
	oldview = tRender.GetView();
	tRender.SetView((m_rMission.GetInverseSwingMatrix()*Matrix(oldview).Inverse()).Inverse());

	if( m_pShipTrackAIParams )
		m_pShipTrackAIParams->DrawDebug(0.f,0);

	m_shipWayTrack.ShowDebugInfo( tRender );
	tRender.DrawVector( m_rShip.GetPosition(), m_vTargetPos, 0xFFFF00FF );

	tRender.SetView(oldview);
};
