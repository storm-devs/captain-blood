
#include "ShipAIPlayerControl.h"
#include "..\..\SeaMissionParams.h"
#include "..\ship.h"

#define ANGLE_DISTANCE_FACTOR 100
#define PLAYER_ONE		"Player One"
#define PLAYER_TWO		"Player Two"
#define PLAYER_THREE	"Player Three"
#define PLAYER_FOUR		"Player Four"

//============================================================================================
//реализация класса объекта параметров для АИ следования по пути
//============================================================================================
PlayerShipControllerParams::PlayerShipControllerParams () :
m_fMaxFireRadius ( MAX_SHIP_FIRE_RADIUS ),
m_bCamRelationalControl ( true )
{
	m_nPlayerIndex = 0;
	m_fCannonFireLevel = 0.2f;
	m_fCannonBtnMaxTime = 2.f;
	m_fMousePower = 10.f;
	m_fMouseTime = 1.f;
}


//-----------------------------------------------------------------------------
// интерфейс, наследуемый от MissionObject
//-----------------------------------------------------------------------------
//Инициализировать объект
bool PlayerShipControllerParams::Create ( MOPReader & reader )
{
	return Update ( reader );
}

//Инициализировать объект в режиме редактирования
bool PlayerShipControllerParams::EditMode_Create ( MOPReader & reader )
{
	return Update ( reader );
}

//Обновить параметры в режиме редактирования
bool PlayerShipControllerParams::EditMode_Update ( MOPReader & reader )
{
	return Update ( reader );
}

// чтение параметров
bool PlayerShipControllerParams::Update (  MOPReader & reader  )
{
	m_bCamRelationalControl = reader.Bool();
	m_fMaxFireRadius = reader.Float();
	m_fCannonFireLevel = reader.Float();
	m_fCannonBtnMaxTime = reader.Float();
	m_fMousePower = reader.Float();
	m_fMouseTime = reader.Float();
	ConstString pcPlayer = reader.Enum();
	static const ConstString id_plONE(PLAYER_ONE);
	static const ConstString id_plTWO(PLAYER_TWO);
	static const ConstString id_plTHREE(PLAYER_THREE);
	static const ConstString id_plFOUR(PLAYER_FOUR);
	

	if( pcPlayer.NotEmpty() )
	{
		if( pcPlayer == id_plONE )
			m_nPlayerIndex = 0;
		else if( pcPlayer == id_plTWO )
			m_nPlayerIndex = 1;
		else if( pcPlayer == id_plTHREE )
			m_nPlayerIndex = 2;
		else if( pcPlayer == id_plFOUR )
			m_nPlayerIndex = 3;
	}

	return true;
}

//============================================================================================
//реализация класса АИ корабля для следования по пути
//============================================================================================
PlayerShipController::PlayerShipController ( IShip & rShip, IMission & rMission ) :
ShipController ( rShip, rMission ),
m_pPlayerShipControllerParams ( NULL )
{
	m_moveForwardCtrl = rMission.Controls().FindControlByName("Ship_Move");
	m_rotateCtrl = rMission.Controls().FindControlByName("Ship_Turn");
	m_fireCannonsCtrl = rMission.Controls().FindControlByName("ShipFireCannons");
	//m_fireKnippelsCtrl = rMission.Controls().FindControlByName("ShipFireKnippels");
	m_fireFlameCtrl = rMission.Controls().FindControlByName("ShipFireFlamethrowers");
	m_fireMinesCtrl = rMission.Controls().FindControlByName("ShipFireMines");

	m_mouseForwardCtrl = rMission.Controls().FindControlByName("Ship_MouseMove");
	m_mouseSideCtrl = rMission.Controls().FindControlByName("Ship_MouseTurn");

	m_bCannonBtnIsPressed = false;
	m_fCannonBtnTime = 0.f;

	m_mouseMoveMin = 0.1f;
	m_mouseCurUD = 0.f;
	m_mouseCurLR = 0.f;
	m_mouseTime = 0.5f;
}

PlayerShipController::~PlayerShipController()
{
}

// управление кораблем
void	PlayerShipController::Work ( float dltTime )
{
	ShipController::Work( dltTime );

	if ( dltTime < 1e-3f )
		return;

	if ( !m_pPlayerShipControllerParams )
		return;

	float fForwardMotion = m_rMission.Controls().GetControlStateFloat ( m_moveForwardCtrl );
	float fRotationMotion = m_rMission.Controls().GetControlStateFloat ( m_rotateCtrl );

	float fMouseUD = -m_rMission.Controls().GetControlStateFloat ( m_mouseForwardCtrl );
	float fMouseLR = m_rMission.Controls().GetControlStateFloat ( m_mouseSideCtrl );
	// ограничение и усиление движения мышки
	if( fabs(fMouseUD) < m_mouseMoveMin ) fMouseUD = 0.f;
	else fMouseUD *= m_pPlayerShipControllerParams->m_fMousePower;
	if( fabs(fMouseLR) < m_mouseMoveMin ) fMouseLR = 0.f;
	else fMouseLR *= m_pPlayerShipControllerParams->m_fMousePower;
	// смена или сглаживание перемещения
	if( fMouseUD != 0.f || fMouseLR != 0.f )
	{
		// смена направления применяем сразу
		if( fMouseUD < 0.f && m_mouseCurUD > 0.f ||
			fMouseUD > 0.f && m_mouseCurUD < 0.f ||
			fMouseLR < 0.f && m_mouseCurLR > 0.f ||
			fMouseLR > 0.f && m_mouseCurLR < 0.f )
		{
			m_mouseCurUD = fMouseUD;
			m_mouseCurLR = fMouseLR;
		}
		// берем максимальное направление
		else
		{
			if( fabs(fMouseUD) > fabs(m_mouseCurUD) )
				m_mouseCurUD = fMouseUD;
			if( fabs(fMouseLR) > fabs(m_mouseCurLR) )
				m_mouseCurLR = fMouseLR;
		}
	}
	else
	{
		m_mouseTime -= dltTime;
		if( m_mouseTime < 0.f )
		{
			m_mouseTime = m_pPlayerShipControllerParams->m_fMouseTime;
			m_mouseCurUD = m_mouseCurLR = 0.f;
		}
	}

	if( fabs(fForwardMotion) < fabs(m_mouseCurUD) ) fForwardMotion = m_mouseCurUD;
	if( fabs(fRotationMotion) < fabs(m_mouseCurLR) ) fRotationMotion = m_mouseCurLR;

	//if( fabs(fMouseUD)

	Vector vShipPos = m_rShip.GetPosition ();
	Matrix tmView =  m_rMission.Render ().GetView ();

	Vector vCamPosition = tmView.GetCamPos ();
	//Vector vCamDirection = tmView.Inverse();
	Vector vCamShipDir = vShipPos - vCamPosition;
	vCamShipDir.y = 0;
	vCamShipDir.Normalize ();

	// если есть команды на движение или разворот, они обрабатываются
	if (  ( fabs ( fRotationMotion  ) < 1e-6f ) && ( fabs ( fForwardMotion  ) < 1e-6f ) )
	{
		m_rShip.SetRotate ( 0.0f );
		m_rShip.SetVelocity ( 0.0f );
	}
	else
	{
		if ( m_pPlayerShipControllerParams->m_bCamRelationalControl   )
		{
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
		}

		float fMaxRotation = m_rShip.GetMaxAngleSpeed();
		float fMaxVelocity = m_rShip.GetMaxVelocity();

		m_rShip.SetRotate ( fRotationMotion*fMaxRotation );
		m_rShip.SetVelocity ( fForwardMotion*fMaxVelocity );
	}


	// управление боем
	MissionObject * pFireTarget = NULL;

	bool fireCannons = false;
	bool bMightCannon = false;
	float fFireToSailProbability = m_pPlayerShipControllerParams->m_fCannonFireLevel;//0.2f;//0.f;

	ControlStateType cst = m_rMission.Controls().GetControlStateType( m_fireCannonsCtrl );
	if( cst == CST_INACTIVATED )
	{
		fireCannons = true;
		m_fCannonBtnTime = 0.f;
	}
	else
	if( cst == CST_ACTIVE )
	{
		m_fCannonBtnTime += dltTime;
		if( m_fCannonBtnTime >= m_pPlayerShipControllerParams->m_fCannonBtnMaxTime )
		{
			fireCannons = true;
			bMightCannon = true;
			m_fCannonBtnTime = 0.f;
		}
	}
	else
	{
		m_fCannonBtnTime = 0.f;
	}
	float fMightCannonProgress = (m_pPlayerShipControllerParams->m_fCannonBtnMaxTime > 0.01f) ? (m_fCannonBtnTime / m_pPlayerShipControllerParams->m_fCannonBtnMaxTime) : 0.f;
	((Ship*)&m_rShip)->SetCannonPowerShoot(bMightCannon, Clampf(fMightCannonProgress));
/*	if( m_rMission.Controls().GetControlStateBool ( m_fireCannonsCtrl ) )
	{
		fireCannons = true;
	}
	if( m_rMission.Controls().GetControlStateBool ( m_fireKnippelsCtrl ) )
	{
		fireCannons = true;
		fFireToSailProbability = 1.f;
	}*/

	bool fireFlame = m_rMission.Controls().GetControlStateBool ( m_fireFlameCtrl );
	bool fireMines = m_rMission.Controls().GetControlStateBool ( m_fireMinesCtrl );

	float fMaxFireRadius = m_pPlayerShipControllerParams->m_fMaxFireRadius;

	const unsigned int TargetsCount = 4;
	// система наведения
	// принцип работы:
	// 1. найти ближайших противников
	// 2. отсортировать по возрастанию дистанции
	// 3. выстрелить по TargetsCount противникам по порядку
	// 4. отстрелить остаток
	//
	if ( fireCannons )
	{
		// определяем цели в радиусе стрельбы
 		Vector vMinAreaCorner =  vShipPos + Vector ( -fMaxFireRadius, -fMaxFireRadius, -fMaxFireRadius );
		Vector vMaxAreaCorner =  vShipPos + Vector ( fMaxFireRadius, fMaxFireRadius, fMaxFireRadius );

		array<MissionObject*> targets(__FILE__, __LINE__);

		Matrix mtxtmp(true);
		Matrix selfmtx(true);
		m_rShip.GetMatrix( selfmtx );
		bool bEnemyLeft = false;
		bool bEnemyRight = false;
	
		// составляем список потенциальных целей
		if( ShipIterator() ) for( ShipIterator()->Reset(); !ShipIterator()->IsDone(); ShipIterator()->Next() )
		{
			MissionObject * pMissionObject = ShipIterator()->Get();
			Assert  ( pMissionObject );

			if ( pMissionObject == &m_rShip )
				continue;

			if ( pMissionObject->IsDead() || !pMissionObject->IsActive() || !pMissionObject->IsShow())
				continue;

			targets.Add(pMissionObject);

			// проверить с какой стороны противник
			if( !(bEnemyLeft && bEnemyRight) )
			{
				Vector vloctarg = selfmtx.MulVertexByInverse( pMissionObject->GetMatrix(mtxtmp).pos );
				// проверим чтобы цель была в секторе 1/4круга в сторону от корабля:
				if( abs(vloctarg.z) < abs(vloctarg.x) )
				{
					if( selfmtx.MulVertexByInverse( pMissionObject->GetMatrix(mtxtmp).pos ).x < 0.f )
						bEnemyLeft = true;
					else
						bEnemyRight = true;
				}
			}
		}

		//bEnemyLeft = bEnemyRight = true;
		// стреляем по TargetsCount наилучшим целям
		Matrix mtx;
		for (unsigned int i = 0; i < TargetsCount; ++i)
		{
			unsigned int best = 0xFFFFFFFF;
			float bestDist = 1e38f;
			for (unsigned int j = 0; j < targets.Size(); ++j)
			{
				float dist = (targets[j]->GetMatrix(mtx).pos - vShipPos).GetLength();
				if ( dist < bestDist )
				{
					bestDist = dist;
					best = j;
				}
			}
			if (best != 0xFFFFFFFF)
			{
				m_rShip.FireCannons ( targets[best], fFireToSailProbability );
				targets.DelIndex(best);
			}
		}

		m_rShip.FireCannonsRemainder(bEnemyLeft,bEnemyRight);
	}

	if (fireFlame) m_rShip.FireFlamethrower();
	if (fireMines) m_rShip.FireMines();

#ifdef CONTROL_DEBUG_DRAW
	DrawDebugInfo ();
#endif

}

// установить объект параметров
void	PlayerShipController::SetControllerParams ( IShipControllerParams * pControllerParams  )
{
	Assert ( pControllerParams );
	ShipController::SetControllerParams(pControllerParams);

	MO_IS_IF(tid, "PlayerShipControllerParams", pControllerParams)
	{
		PlayerShipControllerParams * pPlayerShipControllerParams =  static_cast <PlayerShipControllerParams *> ( pControllerParams );
		Assert ( pPlayerShipControllerParams );

		m_pPlayerShipControllerParams = pPlayerShipControllerParams;
		UpdateControlKeys();
	}
}

// отрисовка дебужной информации
void	PlayerShipController::DrawDebugInfo ()
{
	/*
	IRender & tRender = m_rMission.Render ();
	tRender.DrawBufferedLine ( vPred, 0xffffffff, vNext, 0xffffffff, false );
	*/
}

void PlayerShipController::UpdateControlKeys()
{
	long nPlayerIdx = 0;
	if( m_pPlayerShipControllerParams )
		nPlayerIdx = m_pPlayerShipControllerParams->m_nPlayerIndex;

	m_moveForwardCtrl = FindControl("Ship_Move",nPlayerIdx);
	m_rotateCtrl = FindControl("Ship_Turn",nPlayerIdx);
	m_fireCannonsCtrl = FindControl("ShipFireCannons",nPlayerIdx);
	//m_fireKnippelsCtrl = FindControl("ShipFireKnippels",nPlayerIdx);
	m_fireFlameCtrl = FindControl("ShipFireFlamethrowers",nPlayerIdx);
	m_fireMinesCtrl = FindControl("ShipFireMines",nPlayerIdx);

	m_mouseForwardCtrl = FindControl("Ship_MouseMove",nPlayerIdx);
	m_mouseSideCtrl = FindControl("Ship_MouseTurn",nPlayerIdx);
}

long PlayerShipController::FindControl(const char* pcBaseName,long nIndex)
{
	if( !m_pPlayerShipControllerParams ) return -1;

	if( nIndex == 0 )
		return m_pPlayerShipControllerParams->Controls().FindControlByName(pcBaseName);

	char pcName[256];
	crt_snprintf(pcName,sizeof(pcName),"%s%n",pcBaseName,nIndex);
	return m_pPlayerShipControllerParams->Controls().FindControlByName(pcName);
}


MOP_BEGINLISTG  ( PlayerShipControllerParams, "PlayerShipControllerParams", '1.00', 0, "Arcade Sea" )
MOP_BOOLC( "CameraRelation", true, "Camera Relation Control" )
MOP_FLOATEXC( "MaximumFireDistance", 500, 0, 5000, "Maximum Fire Distance" )
MOP_FLOATEXC( "CannonFireLevel", 0.2f, 0, 1.f, "Probability cannon shoot to sails 0-shoot to hull, 1.0-shoot to sails" )
MOP_FLOATC( "CannonPowerShootTime", 2.f, "Timeout for cannon mighty shoot" )
MOP_FLOAT( "MouseControlPower", 10.f )
MOP_FLOAT( "MouseControlTime", 1.f )
MOP_ENUMBEG("PlayerIndex")
	MOP_ENUMELEMENT( PLAYER_ONE )
	MOP_ENUMELEMENT( PLAYER_TWO )
	MOP_ENUMELEMENT( PLAYER_THREE )
	MOP_ENUMELEMENT( PLAYER_FOUR )
MOP_ENUMEND
MOP_ENUM("PlayerIndex", "Player Index")
MOP_ENDLIST ( PlayerShipControllerParams )
