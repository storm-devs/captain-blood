
#include "ShipAIBattle.h"
#include "..\Ship.h"


//============================================================================================
//реализация класса АИ боя
//============================================================================================
Battle_ShipAI::Battle_ShipAI (  IShip & rShip, IMission & rMission  )
: ShipAI ( rShip, rMission )
, m_pShipBattleAIParams ( NULL )
, m_pEnemy ( NULL )		       
, m_fOptimumFireDistance ( MAX_SHIP_FIRE_RADIUS / 3 )
, m_fMaxFireRadius ( MAX_SHIP_FIRE_RADIUS )
, m_eBehaviorType ( SHIP_BATTLE_TYPE_FIRE )
, m_iAttackStage ( AttackStage_SwimToTarget )
, m_vTargetPos ( 0, 0, 0 )
{
};

Battle_ShipAI ::~Battle_ShipAI ()
{
};

// управление кораблем
void	Battle_ShipAI ::Work ( float dltTime )
{
	ShipAI::Work(dltTime);

	if ( dltTime > 0.05f )
		dltTime = 0.05f;

	if ( dltTime < 1e-3f )
		return;

	Vector vShipPos = m_rShip.GetPosition ();
	float fLookRadius = m_fEnemyDiscoverDistance;

	if( IsInsideAIZone() )
	{
		// если противник не установлен, ищется противник
		if ( !m_pEnemy )
		{
			m_pEnemy = FindNearEnemy(m_fEnemyDiscoverDistance);
			if( m_pEnemy )
				m_Maneur.SetShips( &m_rShip, m_pEnemy );
		}

		if ( m_pEnemy  )
		{
			Matrix tmEnemyMatrix;
			m_pEnemy->GetMatrix ( tmEnemyMatrix );
			Vector vEnemPos = tmEnemyMatrix.pos;
			Vector vEnemyToShipDir = vShipPos - vEnemPos; 

			m_Maneur.Update( dltTime );

			if  ( m_eBehaviorType == SHIP_BATTLE_TYPE_RUN  )
			{
				// бегство от противника
				vEnemyToShipDir.Normalize ();
				m_vTargetPos = vShipPos + vEnemyToShipDir*fLookRadius;
				SetTargetPos ( m_vTargetPos, NODE_STATE_CAN_PASS  );
			}
			else
			{
				//в зависимости от типа поведения ведутся боевые действия в отношении противника
				Vector vEnemyBoardOrient = Vector ( 1.0f, 0, 0 );
				Vector vEnemyShipDir = Vector ( 0, 0, 1.0f );

				vEnemyBoardOrient =  tmEnemyMatrix.MulNormal ( vEnemyBoardOrient );
				vEnemyShipDir =  tmEnemyMatrix.MulNormal ( vEnemyShipDir );

				if  ( m_eBehaviorType == SHIP_BATTLE_TYPE_FIRE  )
				{
					// возмем половину оптимальной дистанции что бы не получалось подъезжаем к кораблю и не стреляем,
					// т.к. находимся на границе выстрела
					vEnemyBoardOrient = 0.5f*m_fOptimumFireDistance*vEnemyBoardOrient;
					vEnemyShipDir = m_fShipRadiusZ*vEnemyShipDir;//m_fOptimumFireDistance*vEnemyShipDir;
				} else
				if  ( m_eBehaviorType == SHIP_BATTLE_TYPE_BOARDING  )
				{
					vEnemyBoardOrient = 1.5f*m_fShipRadiusX*vEnemyBoardOrient;
					vEnemyShipDir = 1.3*m_fShipRadiusZ*vEnemyShipDir;

					SetObstacleExclusion ( m_pEnemy );
				}

				Vector vBoardingPointLeft = vEnemPos - vEnemyBoardOrient;
				Vector vBoardingPointRight = vEnemPos + vEnemyBoardOrient;

				float fLeftBoardingDist = ( vShipPos - vBoardingPointLeft ).GetLength ();
				float fRightBoardingDist = ( vShipPos - vBoardingPointRight ).GetLength ();

				// выберем ближайшую точку для атаки
				m_vTargetPos = vBoardingPointRight;
				if ( fLeftBoardingDist < fRightBoardingDist  )
					m_vTargetPos = vBoardingPointLeft;

				if( m_iAttackStage != AttackStage_Maneur && m_Maneur.IsManeurNeed() && m_Maneur.StartManeur() )
				{
					m_iAttackStage = AttackStage_Maneur;
				}

				switch ( m_iAttackStage )
				{
				case AttackStage_SwimToTarget:
					{
						SetOptimumSpeed ( m_rShip.GetMaxVelocity () );

						//m_vTargetPos = m_vTargetPos - vEnemyShipDir;
						float fCurrDist = ( vShipPos - m_vTargetPos ).GetLength ();

						if ( fCurrDist < 0.5f*m_fShipRadiusX )
							m_iAttackStage = AttackStage_FireToTarget;
					}
					break;
				case AttackStage_FireToTarget:
					{
						SetOptimumSpeed ( m_rShip.GetMaxVelocity () );

						if (m_eBehaviorType == SHIP_BATTLE_TYPE_FIRE ) 
							m_vTargetPos = m_vTargetPos + vEnemyShipDir;

						if (m_eBehaviorType == SHIP_BATTLE_TYPE_BOARDING )
							m_vTargetPos = m_vTargetPos + 0.1f*vEnemyShipDir;

						float fCurrDist = ( vShipPos - m_vTargetPos ).GetLength ();

						if ( fCurrDist > 0.5f*m_fShipRadiusX )
						{
							if (m_eBehaviorType == SHIP_BATTLE_TYPE_FIRE ) 
								m_iAttackStage = AttackStage_SwimToTarget;

							if (m_eBehaviorType == SHIP_BATTLE_TYPE_BOARDING ) 
								m_iAttackStage = AttackStage_BoardTarget;
						}
					}
					break;
				case AttackStage_BoardTarget:
					{
						m_vTargetPos = m_vTargetPos + vEnemyShipDir;

						IShip * pEnemShip = static_cast<IShip *> ( m_pEnemy  );

						if ( !pEnemShip )
							break;

						//SetOptimumSpeed ( m_rShip.GetMaxVelocity () );
						SetOptimumSpeed ( pEnemShip->GetCurrentVelocity () );

						Vector vTargetDir = m_vTargetPos - vShipPos;
						float fCurrDist = vTargetDir.GetLength ();
						float fEnemyDot = -(vEnemyToShipDir | vTargetDir); //-vEnemyToShipDir.x*vTargetDir.x - vEnemyToShipDir.y*vTargetDir.y - vEnemyToShipDir.z*vTargetDir.z;

						if ( fEnemyDot > 0 )
							m_iAttackStage = AttackStage_FireToTarget;

						if ( fCurrDist < 0.2f*m_fShipRadiusX || vEnemyToShipDir.GetLength () > 1.5f*m_fShipRadiusZ  )
							m_iAttackStage = AttackStage_SwimToTarget;

					}
					break;
				case AttackStage_Maneur:
					{
						if( m_Maneur.ManeurIsDone() )
						{
							m_Maneur.StopManeur();
							m_iAttackStage = AttackStage_SwimToTarget;
						}
						else
						{
							m_vTargetPos = m_Maneur.GetSwimTarget();
							SetOptimumSpeed ( m_Maneur.GetSwimSpeed() );
						}
					}
					break;
				}

				if  ( m_eBehaviorType == SHIP_BATTLE_TYPE_BOARDING  )
					SetTargetPos ( m_vTargetPos, NODE_STATE_EXACTLY );
				else
					SetTargetPos ( m_vTargetPos, NODE_STATE_CAN_PASS );
			}

			ProcessAIFire ( dltTime );

			if ( m_pEnemy->IsDead () || !m_pEnemy->IsActive() || !m_pEnemy->IsShow() )
				StopAttacking ();
		}
		else
		{
			// если противника нет, корабль случайно рыскает по морю в его поисках
			if (  ( m_vTargetPos - vShipPos ).GetLength ()  < 1.0f )
			{
				Vector vRandom;

				vRandom.RandXZ ();
				float fRndDist = 2*m_fShipRadiusZ * ( 1.0f + Rnd () );
				vRandom *= fRndDist;
				m_vTargetPos = vShipPos + vRandom;

				SetTargetPos ( m_vTargetPos, NODE_STATE_CAN_PASS  );
			}
		}
	}
	// если мы вышли за зону АИ, то сбрасываем все установки - дабы не уплывать далеко
	else if( m_pEnemy || IsFinishMove() )
	{
		StopAttacking ();
		m_pEnemy = NULL;
		m_vTargetPos = Vector(0.f);
		SetTargetPos ( m_vTargetPos, NODE_STATE_CAN_PASS  );
	}

	// обработка движения в заданную точку
	bool bNodeReached = MoveProc ( dltTime );

	if ( bNodeReached )
		m_vTargetPos = vShipPos;

#ifdef DEBUG_DRAW
	DrawDebugInfo ();
#endif
};

// установить объект параметров
void	Battle_ShipAI ::SetControllerParams ( IShipControllerParams * pControllerParams  )
{
	Assert ( pControllerParams );
	ShipAI::SetControllerParams(pControllerParams);

	MO_IS_IF(tid, "ShipBattleAIParams", pControllerParams)
	{
		ShipBattleAIParams * pBattleAIParams =  static_cast <ShipBattleAIParams *> ( pControllerParams );
		Assert ( pBattleAIParams );

		m_pShipBattleAIParams = pBattleAIParams;

		m_pEnemy = m_pShipBattleAIParams->m_pEnemy.Ptr();
		m_fOptimumFireDistance = m_pShipBattleAIParams->m_fOptimumFireDistance;
		m_eBehaviorType = m_pShipBattleAIParams->m_eBehaviorType;
		m_fMaxFireRadius = m_pShipBattleAIParams->m_fMaxFireRadius;
		m_fEnemyDiscoverDistance = m_pShipBattleAIParams->m_fEnemyDiscoverDistance;
		m_fFireLevel = m_pShipBattleAIParams->m_fFireLevel;
	};

	m_iAttackStage = AttackStage_SwimToTarget;
	SetOptimumSpeed ( m_rShip.GetMaxVelocity () );
	m_vTargetPos = m_rShip.GetPosition ();
};

// получить текущее АИ-состояние
ShipControlType	Battle_ShipAI ::GetType () const
{
	return SHIP_AI_STATE_BATTLE;
};

// получить  объект преследования
MissionObject *	 Battle_ShipAI ::GetEnemy () const
{
	return m_pEnemy;
};

// АИ стрельбы
void	Battle_ShipAI ::ProcessAIFire ( float fDTime )
{
	if ( !m_pEnemy )
		return;

	Vector vShipPos = m_rShip.GetPosition ();
	Matrix tmEnemyMatrix;
	m_pEnemy->GetMatrix ( tmEnemyMatrix );
	Vector vEnemPos = tmEnemyMatrix.pos;

	Vector vShipToEnemy = vEnemPos - vShipPos;
	float fEnemyDist = vShipToEnemy.GetLength ();

	// корабль который хочет брать на абордаж не стреляет на сверхмалой дистанции
	if ( SHIP_BATTLE_TYPE_BOARDING == m_eBehaviorType && fEnemyDist < 2.0f*m_fShipRadiusZ  )
		return;

	// стрельба на дистанции больше позволенной не производится
	if ( fEnemyDist > m_fMaxFireRadius  )
		return;

	//m_rShip.FireCannons ( m_pEnemy, m_fFireLevel, 0.f );
	m_rShip.FireCannons ( m_pEnemy, m_fFireLevel );
	m_rShip.FireFlamethrower();
	m_rShip.FireMines();
};

void Battle_ShipAI::StopAttacking ()
{
	m_pEnemy = NULL;
	m_iAttackStage = AttackStage_SwimToTarget;
	m_vTargetPos = m_rShip.GetPosition ();
	StopMove();
};

// отрисовка дебажной инфы
void	Battle_ShipAI ::DrawDebugInfo ()
{
#ifndef DEBUG_DRAW
	return;
#endif

	ShipAI::DrawDebugInfo ();

	IRender & tRender = m_rMission.Render ();

#ifdef DEBUG_DRAW_TARGET_LINES
	Vector vStart = m_rShip.GetPosition ();
	Vector vEnd = m_vTargetPos;

	dword dwColor = 0xFFFFFFFF;

	tRender.DrawBufferedLine ( vStart, dwColor, vEnd, dwColor, false );
#endif

};




ShipAIManeurChecker::ShipAIManeurChecker()
{
	m_bManeurProcessing = false;
}

ShipAIManeurChecker::~ShipAIManeurChecker()
{
}

void ShipAIManeurChecker::SetShips(MissionObject* pSelf, MissionObject* pEnemy)
{
	m_pSelf = pSelf;
	m_pEnemy = pEnemy;
	m_bManeurProcessing = false;
	ClearStatistic();
}

void ShipAIManeurChecker::Update(float fDeltaTime)
{
	// нет объектов или мы в процессе маневра, то не выполняем  сбор статистики
	if( !m_pSelf || !m_pEnemy )
		return;
	float fLastAttackTime = ((Ship*)m_pSelf)->GetStatistics().GetStatistic( ShipStatistics::stt_lastAttackTime );
	float fDamage = ((Ship*)m_pSelf)->GetStatistics().GetStatistic( ShipStatistics::stt_receivedDamage );
	if( m_bManeurProcessing )
		m_Statistic.fAfterShootTime = ((Ship*)m_pSelf)->GetStatistics().GetStatistic( ShipStatistics::stt_lastAttackTime );
	else
	{
		if( fLastAttackTime < m_Statistic.fAfterShootTime )
			ClearStatistic();
		else if( fDamage > m_Statistic.fRecievedDamage && m_Statistic.fAfterShootTime==0.f )
		{
			m_Statistic.fAfterShootTime = fLastAttackTime;
		}
	}

	m_fManeurTime -= fDeltaTime;
}

bool ShipAIManeurChecker::IsManeurNeed()
{
	// в течении длительного времени мы сами не стреляли, но дамаг получали
	// надо применять маневр - так как игрок "читерит"
	if( m_pSelf )
	{
		float fDamage = ((Ship*)m_pSelf)->GetStatistics().GetStatistic(ShipStatistics::stt_receivedDamage) - m_Statistic.fRecievedDamage;
		if( fDamage > 0 )
		{
			float fTime = ((Ship*)m_pSelf)->GetStatistics().GetStatistic(ShipStatistics::stt_lastAttackTime) - m_Statistic.fAfterShootTime;
			if( fTime > 10.f )
				return true;
		}
	}

	return false;
}

bool ShipAIManeurChecker::StartManeur()
{
	ClearStatistic();
	CalculateTargPos( ChooseType() );
	m_bManeurProcessing = true;
	return true;
}

bool ShipAIManeurChecker::ManeurIsDone()
{
	// закончилось время маневра или достигли точки завершения - конец маневру
	if( m_fManeurTime <= 0.f ||
		(m_pSelf && ~(((IShip*)m_pSelf)->GetPosition() - m_vManeurTargetPos) < 10.f) )
		return true;

	return false;
}

void ShipAIManeurChecker::StopManeur()
{
	m_bManeurProcessing = false;
	ClearStatistic();
}

void ShipAIManeurChecker::ClearStatistic()
{
	if( m_pSelf )
	{
		m_Statistic.fRecievedDamage = ((Ship*)m_pSelf)->GetStatistics().GetStatistic( ShipStatistics::stt_receivedDamage );
		m_Statistic.fAfterShootTime = 0.f;
	}
}

void ShipAIManeurChecker::CalculateTargPos(ManeurType mt)
{
	m_fManeurShipSpeed = 0.f;
	m_fManeurTime = 5.f;
	if( !m_pSelf ) {
		m_vManeurTargetPos = 0.f;
		return;
	}
	if( !m_pEnemy ) {
		m_vManeurTargetPos = ((IShip*)m_pSelf)->GetPosition();
		return;
	}

	switch( mt )
	{
	case ManeurType_Turning:
		{
			Vector vTarg;
			// наше положение
			Matrix m(true);
			m_pSelf->GetMatrix( m );
			// позиция врага по отношению к нам
			Vector vEnemy = m.MulVertexByInverse( ((IShip*)m_pEnemy)->GetPosition() );

			// выбор стороны поворота в зависимости от того где находится противник
			if( vEnemy.x < 0.f )
				vTarg = Vector( -20.f, 0.f, 10.f );
			else
				vTarg = Vector( 20.f, 0.f, 10.f );

			// получаем цель в глобальной системе координат
			m_vManeurTargetPos = m.MulVertex( vTarg );
			// скорость
			m_fManeurShipSpeed = ((IShip*)m_pSelf)->GetMaxVelocity();

			// время на маневр
			m_fManeurTime = 5.f;
			return;
		}
		break;

	case ManeurType_Interception:
		{
			static const float fInterceptionTime = 5.f;

			// позиция и скорость врага
			Matrix mEnemy(true);
			m_pEnemy->GetMatrix( mEnemy );
			float fEnemySpeed = ((IShip*)m_pEnemy)->GetCurrentVelocity();

			// наша позиция и максимальная скорость
			Matrix mSelf(true);
			m_pSelf->GetMatrix( mSelf );
			float fSelfMaxSpeed = ((IShip*)m_pSelf)->GetMaxVelocity();

			// позиция для перехвата
			m_vManeurTargetPos = mEnemy.MulVertex( Vector(0.f,0.f,fInterceptionTime*fEnemySpeed) );
			// дистанция до перехвата
			float fInterceptionDistance = (m_vManeurTargetPos - ((IShip*)m_pSelf)->GetPosition()).GetLength();
			// оптимальная скорость для перехвата
			m_fManeurShipSpeed = 5.f * fInterceptionDistance / fInterceptionTime;
			// время на маневр
			m_fManeurTime = fSelfMaxSpeed > 0.f ? fInterceptionDistance / fSelfMaxSpeed : fInterceptionTime;
			if( m_fManeurTime < fInterceptionTime )
				m_fManeurTime = fInterceptionTime;
			return;
		}
		break;
	}

	// получаем цель в глобальной системе координат
	m_vManeurTargetPos = ((IShip*)m_pSelf)->GetPosition();
	// скорость
	m_fManeurShipSpeed = ((IShip*)m_pSelf)->GetMaxVelocity();
}

ShipAIManeurChecker::ManeurType ShipAIManeurChecker::ChooseType()
{
	ManeurType mt = ManeurType_Turning;
	if( m_pSelf && m_pEnemy )
	{
		Matrix mSelf(true);
		m_pSelf->GetMatrix( mSelf );

		Matrix mEnemy(true);
		m_pEnemy->GetMatrix( mEnemy );

		Vector vLocalEnemyPos = mSelf.MulVertexByInverse( mEnemy.pos );
		if( vLocalEnemyPos.z < 0.f )
			mt = ManeurType_Interception;
	}

	return mt;
}
