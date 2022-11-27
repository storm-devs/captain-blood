#include "ShipAIPlayerControlStorm.h"

PlayerShipControllerParamsStorm::PlayerShipControllerParamsStorm () :
m_minSpeed(0.0f),
m_stormForce1(0.0f),
m_stormFreq1(0.0f),
m_stormForce2(0.0f),
m_stormFreq2(0.0f)
{
}


//-----------------------------------------------------------------------------
// интерфейс, наследуемый от MissionObject
//-----------------------------------------------------------------------------
//Инициализировать объект
bool PlayerShipControllerParamsStorm::Create ( MOPReader & reader )
{
	return Update ( reader );
}

//Инициализировать объект в режиме редактирования
bool PlayerShipControllerParamsStorm::EditMode_Create ( MOPReader & reader )
{
	return Update ( reader );
}

//Обновить параметры в режиме редактирования
bool PlayerShipControllerParamsStorm::EditMode_Update ( MOPReader & reader )
{
	return Update ( reader );
}

// чтение параметров
bool PlayerShipControllerParamsStorm::Update (  MOPReader & reader  )
{
	m_minSpeed = reader.Float();
	m_stormForce1 = reader.Float();
	m_stormFreq1 = reader.Float();
	m_stormForce2 = reader.Float();
	m_stormFreq2 = reader.Float();
	
	return true;
}

//============================================================================================
//контроллер корабля управляемый игроком для шторма
//============================================================================================
PlayerShipControllerStorm::PlayerShipControllerStorm ( IShip & rShip, IMission & rMission ) :
ShipController ( rShip, rMission ),
m_pPlayerShipControllerParams ( NULL ),
m_time(0.0f)
{
}

PlayerShipControllerStorm::~PlayerShipControllerStorm()
{
}

// управление кораблем
void	PlayerShipControllerStorm::Work ( float dltTime )
{
	ShipController::Work( dltTime );

	if ( !m_pPlayerShipControllerParams )
		return;

	m_time += dltTime;

	float fForwardMotion = m_rMission.Controls().GetControlStateFloat ( "Ship_Move" );
	float fRotationMotion = m_rMission.Controls().GetControlStateFloat ( "Ship_Turn" );
	if (fForwardMotion < 0.0f) fForwardMotion = 0.0f;

	//////////////////////////////////////////////////////////////////////////////
	// управление относительно камеры
	//////////////////////////////////////////////////////////////////////////////
	Vector vShipPos = m_rShip.GetPosition ();
	Matrix tmView =  m_rMission.Render ().GetView ();

	Vector vCamPosition = tmView.GetCamPos ();
	//Vector vCamDirection = tmView.Inverse();
	Vector vCamShipDir = vShipPos - vCamPosition;
	vCamShipDir.y = 0;
	vCamShipDir.Normalize ();

	Vector vCamSide ( vCamShipDir.z,  0, - vCamShipDir.x );
	Vector vDesiredDir = fRotationMotion*vCamSide + fForwardMotion*vCamShipDir;

	Vector vCurrentDirection;

	vCurrentDirection.MakeXZ ( m_rShip.GetCurrentDirectionAngle () );

	float fDAngle = vCurrentDirection.GetAngleXZ ( vDesiredDir );
	float fEps = PI / 36;

	float fSign = ( fDAngle > 0 ) ? 1.0f : -1.0f;

	float fFactor = 1.0f;

	if ( fabs ( fDAngle ) < fEps  ) 
	{
		fFactor = fabs ( fDAngle ) / fEps;
		fFactor = fFactor * fFactor;
	}

	fRotationMotion = fSign*fFactor;
	fForwardMotion  = ( fabs ( fRotationMotion ) > 1e-6f  || fabs ( fForwardMotion ) > 1e-6f ) ? 1.0f : 0.0f;

	if ( fabs ( fDAngle ) < PI / 2.0f  )
		fForwardMotion *= cosf ( fDAngle );
	else
		fForwardMotion = 0.0f;

	//////////////////////////////////////////////////////////////////////////////

	float fMaxRotation = m_rShip.GetMaxAngleSpeed();
	float fMaxVelocity = m_rShip.GetMaxVelocity();

	float minSpeed = m_pPlayerShipControllerParams->m_minSpeed;
	float stormForce1 = m_pPlayerShipControllerParams->m_stormForce1;
	float stormFreq1 = m_pPlayerShipControllerParams->m_stormFreq1;
	float stormForce2 = m_pPlayerShipControllerParams->m_stormForce2;
	float stormFreq2 = m_pPlayerShipControllerParams->m_stormFreq2;
	
	m_rShip.SetRotate ( (	fRotationMotion*Max(0.0f, 1.0f-0.5f*(stormForce1+stormForce2)) +
							stormForce1*sinf(stormFreq1*m_time + PI/2*cosf(stormFreq1*m_time)*sinf(stormFreq2*m_time)) +
							stormForce2*cosf(stormFreq2*m_time + PI/2*cosf(stormFreq2*m_time)*sinf(stormFreq1*m_time)) )*fMaxRotation);
	m_rShip.SetVelocity ( ((1.0f-minSpeed)*fForwardMotion + minSpeed)*fMaxVelocity );
}

// установить объект параметров
void	PlayerShipControllerStorm::SetControllerParams ( IShipControllerParams * pControllerParams  )
{
	Assert ( pControllerParams );
	ShipController::SetControllerParams(pControllerParams);

	MO_IS_IF(tid, "PlayerShipControllerParamsStorm", pControllerParams)
	{
		PlayerShipControllerParamsStorm * pPlayerShipControllerParams =  static_cast <PlayerShipControllerParamsStorm *> ( pControllerParams );
		Assert ( pPlayerShipControllerParams );

		m_pPlayerShipControllerParams = pPlayerShipControllerParams;
	}
}


MOP_BEGINLISTG  ( PlayerShipControllerParamsStorm, "PlayerShipControllerParamsStorm", '1.00', 0, "Arcade Sea" )
	MOP_FLOATEX("Min forward speed", 0.5f, 0.0f, 1.0f)
	MOP_FLOATEX("Storm force 1", 0.4f, 0.0f, 1.0f);
	MOP_FLOATEX("Storm frequency 1", 1.5f, 0.0f, 100.0f);
	MOP_FLOATEX("Storm force 2", 0.3f, 0.0f, 1.0f);
	MOP_FLOATEX("Storm frequency 2", 1.0f, 0.0f, 100.0f);
MOP_ENDLIST ( PlayerShipControllerParamsStorm )

