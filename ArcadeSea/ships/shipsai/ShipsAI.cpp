
#include "ShipsAI.h"
#include "..\..\Islands\IslandsMap.h"

#define SHIP_RADIUS_AI_FACTOR				4.5f
#define SHIP_ANGLE_SPEED					(PI/4)
#define ANGLE_ROTATION_INERTION_TIME		5.0f


//============================================================================================
//реализация базового класса  АИ корабля
//============================================================================================
ShipAI::ShipAI ( IShip & rShip, IMission & rMission )
: ShipController ( rShip, rMission )
, m_vCurrTargetPoint ( 0, 0, 0 )
, m_eTargetPointState ( NODE_STATE_CAN_PASS )
, m_bFinished ( true )
, m_dObstacleFields ( _FL_ )
, m_pObstacleExclusion ( NULL )
, m_fOptimumSpeed ( 0 )
, m_bOn ( true )
, m_fTimeFromLastRotChange ( 0 )
, m_zeroSpeedTime(0.0f)
{
	m_rShip.GetSizes ( m_fShipRadiusZ, m_fShipRadiusX );
	m_fShipRadiusZ = 0.65f*m_fShipRadiusZ;
	m_fShipRadiusX = 0.65f*m_fShipRadiusX;

	m_fShipAIObstaclesVelocityMax = 0.8f*m_fShipRadiusZ;
	m_fShipAIVelocityMax = 2.0f*m_fShipRadiusZ;

	m_fOptimumSpeed = m_rShip.GetMaxVelocity ();

	m_dObstacleFields.Reserve ( 10 );
	m_bFindIslandsMap = true;
	m_IslandsMap.Reset();
}

ShipAI::~ShipAI ()
{
}

// задать целевую точку
void	ShipAI::SetTargetPos ( const Vector& vTarget, NodeState eTargetState )
{
	m_vCurrTargetPoint = vTarget;
    m_vCurrTargetPoint.y = 0;
	m_bFinished = false;
	m_eTargetPointState = eTargetState;
}

// обработка движения корабля по линейному отрезку
bool	ShipAI::MoveProc ( float fDTime )
{
	m_dObstacleFields.DelAll();

	if ( fDTime < 1e-3f )
		return false;

	if ( m_bFinished )
		return false;

 	bool bNext = false;

	// если мы достигли заданной точки
	Vector vCurrPoint = m_rShip.GetPosition();
	Vector vNextPoint = m_vCurrTargetPoint;
	Vector vDistance = vNextPoint - vCurrPoint;
	float fDistance = vDistance.GetLengthXZ();

	float fMaxWayError = m_fShipRadiusZ;

	if ( NODE_STATE_EXACTLY == m_eTargetPointState )
	{
		fMaxWayError = m_fShipRadiusX;
	}
	else
	if ( NODE_STATE_CAN_PASS == m_eTargetPointState )
	{
		fMaxWayError = 2*m_fShipRadiusZ;
	}

	if ( fabs ( fDistance ) < fMaxWayError )
	{
		bNext = true;
	}

	float fShipAngle = m_rShip.GetCurrentDirectionAngle();
	Matrix  tmShipMatrix;
	tmShipMatrix.BuildRotateY( fShipAngle );
	Matrix tmShipInvMatrix = tmShipMatrix.Inverse();

	// учет препятствий на пути движения
	Vector vObstacleInflDirection( 0, 0, 0 );
	float fObstacleInflMaxVelocity = m_fOptimumSpeed;
	bool bMoveToNextNode = false;

	Vector vMinAreaCorner;
	Vector vMaxAreaCorner;

	vMinAreaCorner.x = ( vCurrPoint.x > vNextPoint.x ) ? vNextPoint.x : vCurrPoint.x;
	vMinAreaCorner.y = ( vCurrPoint.y > vNextPoint.y ) ? vNextPoint.y : vCurrPoint.y;
	vMinAreaCorner.z = ( vCurrPoint.z > vNextPoint.z ) ? vNextPoint.z : vCurrPoint.z;

	vMaxAreaCorner.x = ( vCurrPoint.x > vNextPoint.x ) ? vCurrPoint.x : vNextPoint.x;
	vMaxAreaCorner.y = ( vCurrPoint.y > vNextPoint.y ) ? vCurrPoint.y : vNextPoint.y;
	vMaxAreaCorner.z = ( vCurrPoint.z > vNextPoint.z ) ? vCurrPoint.z : vNextPoint.z;

	vMinAreaCorner -= 2*SHIP_RADIUS_AI_FACTOR*m_fShipRadiusZ;
	vMaxAreaCorner += 2*SHIP_RADIUS_AI_FACTOR*m_fShipRadiusZ;

	// максимальная скорость в области препятствий
	float fObstaclesAreaVelocityMax = m_fOptimumSpeed;

	if ( GetType() != SHIP_AI_STATE_BRANDER && GetType() != SHIP_AI_STATE_BATTLE )
		fObstaclesAreaVelocityMax = m_fShipAIObstaclesVelocityMax;

	if( IsPassObstacles() && ShipIterator() )
		for(ShipIterator()->Reset(); !ShipIterator()->IsDone(); ShipIterator()->Next() )
	{
		MissionObject * pMissionObject = ShipIterator()->Get();
		Assert  ( pMissionObject );

		// корабль не может быть препятствием сам себе
		if ( pMissionObject == &m_rShip )
			continue;

		// цель брандера не может быть eму препятствием
		if (  pMissionObject == m_pObstacleExclusion )
			continue;

		if( !pMissionObject->IsShow() )
			continue;

		// мертвый объект не мешает кораблю - можем плыть через него
		// проверка нужна из за того, что корабль тонет долго, и следующий корабль может кружить на треке,
		// если точка смерти корабля вблизи от ключевой точки трека
		if( pMissionObject->GetHP()<=0.f )
			continue;

		// рассчет влияния препятствий
		Matrix tmObstacleMatrix;
		tmObstacleMatrix = pMissionObject->GetMatrix( tmObstacleMatrix );
		Vector vObstaclePos = tmObstacleMatrix.pos;
		Vector vObstacleDist = vCurrPoint - vObstaclePos;
		float fObstacleDist = vObstacleDist.Normalize();

		// real critical dist
		float fRealCriticalDist = SHIP_RADIUS_AI_FACTOR*m_fShipRadiusZ;

		Vector vLocalObstaclePos = tmShipInvMatrix.MulNormal( -vObstacleDist );
		float fLocalObstacleAng = vLocalObstaclePos.GetAY( 0 );

		// учет формы корабля
		//float fDenominator = ( 0.5f + 1.5f*cosf( fLocalObstacleAng ) );
		float fDenominator = ( 1.f + cosf( fLocalObstacleAng ) );
		fObstacleDist /= fDenominator  < 1e-3f ? 1e-3f : fDenominator;
		if ( fObstacleDist < 1e-3f )
			fObstacleDist = 1e-3f;

		// замедление перед препятствием
		if ( fObstacleDist < fRealCriticalDist )
		{
			if ( fObstacleInflMaxVelocity > fObstaclesAreaVelocityMax )
				fObstacleInflMaxVelocity = fObstaclesAreaVelocityMax;
		}

		// если препятствие впереди по курсу и если оно достаточно близко, то учтем скорость препятствия
		if ( fabs ( fLocalObstacleAng ) < 0.5f*PI  )
		{
			float fSpeedCriticalDist = 0.6f*fRealCriticalDist;

			if ( fObstacleDist < fSpeedCriticalDist )
			{
				float fStartMaxVel = fObstaclesAreaVelocityMax;
				float fMaxVelocity = Clampf ( fStartMaxVel*fObstacleDist / fSpeedCriticalDist , 1.0f, fStartMaxVel  );

				// учитываем относительную скорость препятствия
				float fObstacleRelativeVelocity = 0;
				
				MO_IS_IF(tid, "Ship", pMissionObject)
				{
					IShip * pShip = (IShip*)pMissionObject;

					Vector vObstaclesRelVelocity = pShip->GetCurrentVelocity();
					fObstacleRelativeVelocity = vObstaclesRelVelocity.x* vObstacleDist.x +
					vObstaclesRelVelocity.y* vObstacleDist.y + vObstaclesRelVelocity.z* vObstacleDist.z;
				}

				if ( fObstacleRelativeVelocity > 0.5f )
					fMaxVelocity += fObstacleRelativeVelocity;

				if ( fObstacleInflMaxVelocity > fMaxVelocity )
					fObstacleInflMaxVelocity = fMaxVelocity;
			}
		}

		// вычисление суммарной силы полей препятствий
		float fPower =  0;

		if  ( fObstacleDist < fRealCriticalDist  )
		{
			fPower =  0.4f*fRealCriticalDist  / fObstacleDist - 0.4f;

			if ( fPower > 1.0f && fLocalObstacleAng*m_fOldAngleSpeed >= 0.f )
			{
				m_fTimeFromLastRotChange = 1e+3f;
				fPower = fPower*fPower;
			}
		}

		Vector vObstaclePower = fPower*vObstacleDist;
		vObstacleInflDirection += vObstaclePower;

		// проверка свободен ли следующий пункт маршрута
		Vector vObstacleToTargetDist = vNextPoint - vObstaclePos;
		float fObstacleToTargetDist = vObstacleToTargetDist.GetLength();
		float fWayPointMinDist = m_fShipRadiusZ + m_fShipRadiusX;

		if ( (fObstacleDist < fRealCriticalDist) && (fObstacleToTargetDist < fWayPointMinDist) )
		{
			if ( m_eTargetPointState == NODE_STATE_CAN_PASS   )
				bNext = true;

			if ( m_eTargetPointState == NODE_STATE_CANNOT_PASS  && fPower > 0.6f  )
				bNext = true;

			if ( m_eTargetPointState == NODE_STATE_EXACTLY  && fPower > 1.0f  )
				bNext = true;
		}

#ifdef DEBUG_DRAW_OBSTACLES_FORCES
		DebugArrow tFieldArrow;
		tFieldArrow.vStart = vCurrPoint;
		Vector vArrowVec = vObstaclePower*20.0f;

		tFieldArrow.vEnd = vCurrPoint  +  vArrowVec;
		//IShip * pShip = static_cast<IShip *> ( pMissionObject );

		//if  ( pShip  )
		//{
			tFieldArrow.dwColor = 0xFF00FF00;
			m_dObstacleFields.Add ( tFieldArrow );
		//}
#endif
	}

	// проверка на столкновение с островом ( и попытка увернуться от него )
	if( m_bFindIslandsMap )
	{
		m_bFindIslandsMap = false;

		static const ConstString islandMapId("IslandsMap");
		m_rMission.FindObject( islandMapId, m_IslandsMap );
		static const ConstString tipeId("IslandsMap");
		if( m_IslandsMap.Ptr() && !m_IslandsMap.Ptr()->Is(tipeId) )
			m_IslandsMap.Reset();
	}
	if( m_IslandsMap.Validate() )
	{
		Vector vObstaclePos;
		if( ((IslandsMap*)m_IslandsMap.Ptr())->FindNearIsland(&m_rShip, vObstaclePos) )
		{
			Vector vObstacleDist = vCurrPoint - vObstaclePos;
			float fObstacleDist = vObstacleDist.Normalize();

			// real critical dist
			float fRealCriticalDist = SHIP_RADIUS_AI_FACTOR*m_fShipRadiusZ;

			Vector vLocalObstaclePos = tmShipInvMatrix.MulNormal( -vObstacleDist );
			float fLocalObstacleAng = vLocalObstaclePos.GetAY( 0 );

			// учет формы корабля
			float fDenominator = ( 1.f + cosf( fLocalObstacleAng ) );
			fObstacleDist /= fDenominator  < 1e-3f ? 1e-3f : fDenominator;
			if ( fObstacleDist < 1e-3f )
				fObstacleDist = 1e-3f;

			// замедление перед препятствием
			if ( fObstacleDist < fRealCriticalDist )
			{
				if ( fObstacleInflMaxVelocity > fObstaclesAreaVelocityMax )
					fObstacleInflMaxVelocity = fObstaclesAreaVelocityMax;
			}

			// если препятствие впереди по курсу и если оно достаточно близко, то учтем скорость препятствия
			if ( fabs ( fLocalObstacleAng ) < 0.5f*PI  )
			{
				float fSpeedCriticalDist = 0.6f*fRealCriticalDist;

				if ( fObstacleDist < fSpeedCriticalDist )
				{
					float fStartMaxVel = fObstaclesAreaVelocityMax;
					float fMaxVelocity = Clampf ( fStartMaxVel*fObstacleDist / fSpeedCriticalDist , 1.0f, fStartMaxVel  );

					if ( fObstacleInflMaxVelocity > fMaxVelocity )
						fObstacleInflMaxVelocity = fMaxVelocity;
				}
			}

			// вычисление суммарной силы полей препятствий
			float fPower =  0;

			if  ( fObstacleDist < fRealCriticalDist  )
			{
				fPower =  0.4f*fRealCriticalDist  / fObstacleDist - 0.4f;

				if ( fPower > 1.0f && fLocalObstacleAng*m_fOldAngleSpeed >= 0.f )
				{
					m_fTimeFromLastRotChange = 1e+3f;
					fPower = fPower*fPower;
				}
			}

			Vector vObstaclePower = fPower*vObstacleDist;
			vObstacleInflDirection += vObstaclePower;

			// проверка свободен ли следующий пункт маршрута
			Vector vObstacleToTargetDist = vNextPoint - vObstaclePos;
			float fObstacleToTargetDist = vObstacleToTargetDist.GetLength();
			float fWayPointMinDist = m_fShipRadiusZ + m_fShipRadiusX;

			if ( (fObstacleDist < fRealCriticalDist) && (fObstacleToTargetDist < fWayPointMinDist) )
			{
				if ( m_eTargetPointState == NODE_STATE_CAN_PASS   )
					bNext = true;

				if ( m_eTargetPointState == NODE_STATE_CANNOT_PASS  && fPower > 0.6f  )
					bNext = true;

				if ( m_eTargetPointState == NODE_STATE_EXACTLY  && fPower > 1.0f  )
					bNext = true;
			}
		}
	}

	vDistance.Normalize();
	m_rShip.SetVelocity ( CalcVelocity(vDistance, fDistance, fObstacleInflMaxVelocity) );


	// код отлипания от физических препятсвий (патчей)
	if (m_rShip.GetCurrentVelocity() < 1.0f)
	{
		m_zeroSpeedTime += fDTime;

		if ( m_zeroSpeedTime > 2.0f )
		{
			m_rShip.SetRotate(5*m_rShip.GetMaxAngleSpeed());
			m_rShip.SetVelocity (-15*m_rShip.GetMaxVelocity());
			return false;
		}
	}
	else
	{
		m_zeroSpeedTime = 0.0f;
	}


	Vector vResultDirection = vDistance + vObstacleInflDirection;

#ifdef DEBUG_DRAW_OBSTACLES_FORCES
	DebugArrow tFieldArrow;
	tFieldArrow.vStart = vCurrPoint;
	tFieldArrow.vEnd = vCurrPoint  +  vObstacleInflDirection*20.0f;
	tFieldArrow.dwColor = 0xffffffff;
	m_dObstacleFields.Add ( tFieldArrow );
#endif

	float fNewAngleDir = vResultDirection.GetAY ();

	SetDirectionAngle (  fNewAngleDir, fDTime );

	return bNext;
}


float ShipAI::CalcVelocity(const Vector& vDistance, float fDistance, float fObstacleInflMaxVelocity)
{
	// возмем скорость с которой надо плыть что бы завершить доворот на целевой точке
	float fRotateTime = GetRotationTime ( vDistance.GetAY() );
	Clampf( fRotateTime, 0.01f,  10.0f );
	float fAccuracyFactor = 0.4f;
	float fOptVelocity =  fAccuracyFactor*fDistance / ( fRotateTime + 1e-2f );

	// ограничим эту скорость максимальной скорость корабля
	//if( fOptVelocity > m_rShip.GetMaxVelocity() )
	//	fOptVelocity = m_rShip.GetMaxVelocity();

	// ограничим скорость предрасчитанной оптимальной скоростью
	if ( fOptVelocity > fabs ( fObstacleInflMaxVelocity ) )
		fOptVelocity = fObstacleInflMaxVelocity;

	// ограничим скорость максимальной скорость позволенной в ИИ
	//if (  fOptVelocity > fabs ( m_fShipAIVelocityMax ) )
	//	fOptVelocity = m_fShipAIVelocityMax;

	// скорость у нас не может быть отрицательной
	if ( fOptVelocity < 0 )
		fOptVelocity = 0;

	return fOptVelocity;
}

// установить объект, не являющийся препятствием
void ShipAI::SetObstacleExclusion ( MissionObject * pNoObstacle )
{
	m_pObstacleExclusion = pNoObstacle;
}

// задать оптимальную скорость
void ShipAI::SetOptimumSpeed ( float fOptSpeed )
{
	m_fOptimumSpeed = fOptSpeed;
}

// задать требуемый угол оси корабля
void ShipAI::SetDirectionAngle ( float fTargetAngle, float fDTime )
{
	if ( fabs ( fDTime ) < 1e-4f )
		return;

	// direction processing
	float fCurAngle = m_rShip.GetCurrentDirectionAngle ();
	float fAngleSpeed = m_rShip.GetMaxAngleSpeed ();//SHIP_ANGLE_SPEED;
	float fAngleDelta = fDTime*fAngleSpeed;
	float fAngleError = fTargetAngle - fCurAngle;

	float fCurrAngleSpeed = 0;

	if ( fabs ( fAngleError ) > fAngleDelta  )
	{
		Matrix tmRotMatrix;
		float fSign = ( fAngleError > 0 ) ? 1.0f : -1.0f;

		if ( fabs ( fAngleError ) < PI  )
		{
			fCurrAngleSpeed = fSign*fAngleSpeed;
		}
		else
		{
			fCurrAngleSpeed = - fSign*fAngleSpeed;
		}
	}
	else
	{
		fCurrAngleSpeed =  fAngleError / fDTime;
	}

	float fMaxAngleError = PI / 10;//36;
	float fMinAngleError = PI / 36;

	m_fTimeFromLastRotChange += fDTime;

	if ( fabs ( fAngleError ) < fMaxAngleError )
	{
		float fSmoothRotate = fabs ( fAngleError ) / fMaxAngleError;
		fCurrAngleSpeed *= fSmoothRotate*fSmoothRotate;
	}

	if ( fabs ( fAngleError ) < fMinAngleError )
		fCurrAngleSpeed = 0;

	if ( m_fOldAngleSpeed*fCurrAngleSpeed < 0)
	{
		if ( m_fTimeFromLastRotChange > ANGLE_ROTATION_INERTION_TIME )
		{
			m_fTimeFromLastRotChange = 0;
		}
		else
		{
			if ( fabsf ( fCurrAngleSpeed ) > 1e-3f )
				fCurrAngleSpeed = ( m_fOldAngleSpeed > 0 ) ? 1e-3f : -1e-3f;
		}
	}

	m_rShip.SetRotate ( fCurrAngleSpeed  );

	m_fOldAngleSpeed = fCurrAngleSpeed;
}

// прекращение движения по маршруту
void ShipAI::StopMove ()
{
	m_bFinished = true;
	m_rShip.SetVelocity ( 0.0f );
	m_rShip.SetRotate ( 0.0f );
}

// получить время, необхожимое для разворота корабля
float ShipAI::GetRotationTime (  float fNewDirAngle  ) const
{
	float fAngleSpeed = m_rShip.GetMaxAngleSpeed ();

	if ( fAngleSpeed < 1e-6f )
		return 1e-6f;

	float fCurrAngle = m_rShip.GetCurrentDirectionAngle ();
	float fDeltaAngle = fabsf ( fNewDirAngle - fCurrAngle );

	if ( fDeltaAngle > PI )
		fDeltaAngle = 2*PI - fDeltaAngle;

	return fDeltaAngle / fAngleSpeed;
}

// отрисовка дебужной информации
void ShipAI::DrawDebugInfo ()
{
#ifndef DEBUG_DRAW_OBSTACLE_RADIUS
	return;
#endif

	IRender & tRender = m_rMission.Render ();

	// drawing obstacle radiuses
	DrawBoundingEllipse ( m_fShipRadiusZ, m_fShipRadiusZ );
	DrawBoundingEllipse ( SHIP_RADIUS_AI_FACTOR*m_fShipRadiusZ, SHIP_RADIUS_AI_FACTOR*m_fShipRadiusZ, false );

	// drawing obstacles influences
	for ( int i = 0; i < ( int )m_dObstacleFields.Size () ; ++i )
	{
		Vector vStart = m_dObstacleFields[i].vStart;
		Vector vEnd = m_dObstacleFields[i].vEnd;
		dword dwColor = m_dObstacleFields[i].dwColor;

		Vector vArrowDir = vEnd -  vStart;
		float fArrowAngle = vArrowDir.GetAY ();

		Matrix tmArrowMatrix;
		tmArrowMatrix.BuildRotateY ( fArrowAngle );

		Vector vLeftArrow;
		vLeftArrow.x = vEnd.x - 0.5f*cosf ( fArrowAngle );
		vLeftArrow.y = 0;
		vLeftArrow.z = vEnd.z + 0.5f*sinf ( fArrowAngle );

		Vector vRightArrow;
		vRightArrow.x = vEnd.x + 0.5f*cosf ( fArrowAngle );
		vRightArrow.y = 0;
		vRightArrow.z = vEnd.z - 0.5f*sinf ( fArrowAngle );

		Vector vEndArrow;
		vEndArrow.x = vEnd.x + 0.5f*sinf ( fArrowAngle );
		vEndArrow.y = 0;
		vEndArrow.z = vEnd.z + 0.5f*cosf ( fArrowAngle );

		tRender.DrawBufferedLine ( vLeftArrow, dwColor, vRightArrow, dwColor, false );
		tRender.DrawBufferedLine ( vLeftArrow, dwColor, vEndArrow, dwColor, false );
		tRender.DrawBufferedLine ( vRightArrow, dwColor, vEndArrow, dwColor, false );
		tRender.DrawBufferedLine ( vStart, dwColor, vEnd, dwColor, false );
	}
}

void ShipAI::DrawBoundingEllipse ( float fXRad, float fZRad, bool bSolid )
{
	Matrix tmCurMatrix;

	tmCurMatrix = m_rShip.GetMatrix  ( tmCurMatrix );

	IRender & tRender = m_rMission.Render();
	Vector vObjCenter = tmCurMatrix.pos;
	int iCirclePointsCount = 16;

	for ( int i = 0; i < iCirclePointsCount; ++i )
	{
		float fStartXFactor = sinf  ( 2*i*PI / iCirclePointsCount );
		float fStartZFactor = cosf  ( 2*i*PI / iCirclePointsCount );

		float fLineLen = 1.0f;

		if ( !bSolid )
			fLineLen = 0.5f;

		float fEndXFactor = sinf  ( 2*( i + fLineLen )*PI/iCirclePointsCount );
		float fEndZFactor = cosf  ( 2*( i + fLineLen )*PI/iCirclePointsCount );

		float fZFactor = fStartZFactor;

		float fStartRadius = fXRad + fabs ( fZFactor )*( fZRad - fXRad );
		float fEndRadius = fXRad + fabs ( fZFactor )*( fZRad - fXRad );

		Vector vStart =  Vector (  fXRad*fStartXFactor,  0,   fZRad*fStartZFactor );
		Vector vEnd =  Vector (  fXRad*fEndXFactor,  0,  fZRad*fEndZFactor );

		vStart = tmCurMatrix.MulVertex  ( vStart );
		vEnd = tmCurMatrix.MulVertex  ( vEnd );

		dword dwColor = 0xFF0000FF;
		tRender.DrawBufferedLine ( vStart, dwColor, vEnd, dwColor, false );
	}
}
