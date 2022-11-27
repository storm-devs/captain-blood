
#include "ShipAIBattleParams.h"

//============================================================================================
//реализация класса объекта параметров для АИ боя
//============================================================================================
ShipBattleAIParams::ShipBattleAIParams () :
m_fOptimumFireDistance ( 50 ),    // оптимальная дальность до противника при стрельбе
m_eBehaviorType ( SHIP_BATTLE_TYPE_FIRE ),
m_fMaxFireRadius ( MAX_SHIP_FIRE_RADIUS ),
m_fFireLevel(0.f)
{
}


//-----------------------------------------------------------------------------
// интерфейс, наследуемый от MissionObject
//-----------------------------------------------------------------------------
//Инициализировать объект
bool ShipBattleAIParams::Create ( MOPReader & reader )
{
	return Update ( reader );
}

//Инициализировать объект в режиме редактирования
bool ShipBattleAIParams::EditMode_Create ( MOPReader & reader )
{
	SetUpdateForDebugShow();
	return Update ( reader );
}

//Обновить параметры в режиме редактирования
bool ShipBattleAIParams::EditMode_Update ( MOPReader & reader )
{
	return Update ( reader );
}

//прочитать параметры  
bool ShipBattleAIParams::Update ( MOPReader & reader )
{
	ConstString pcEnemyName = reader.String ();

	FindObject ( pcEnemyName, m_pEnemy );

	static const ConstString typeId("IShip");
	if ( m_pEnemy.Ptr() && !m_pEnemy.Ptr()->Is(typeId) )
		m_pEnemy.Reset();

	m_fOptimumFireDistance = reader.Float ();
	m_fMaxFireRadius = reader.Float ();
	m_fEnemyDiscoverDistance = reader.Float();
	m_fFireLevel = reader.Float();

	ConstString pcBehavior = reader.Enum ();

	static const ConstString id_Boarding("Boarding");
	static const ConstString id_Run("Run");
	
	
	if ( pcBehavior == id_Boarding )
		m_eBehaviorType = SHIP_BATTLE_TYPE_BOARDING;
	else
	if ( pcBehavior == id_Run )
		m_eBehaviorType = SHIP_BATTLE_TYPE_RUN;
	else
		m_eBehaviorType = SHIP_BATTLE_TYPE_FIRE;

	return ShipControllerParams::ReadMOPs(reader);
}


MOP_BEGINLISTG  ( ShipBattleAIParams, "ShipBattleAIParams", '1.00', 0, "Arcade Sea" )
MOP_STRINGC ( "Enemy Ship Name", "", "Target Ship for Attack" )
MOP_FLOATEXC  ( "OptimumFireDistance", 70, 0, 500, "Optimum Fire Distance" )
MOP_FLOATEXC ( "MaxFireDistance", 200, 0, 500, "Maximum Fire Distance" )
MOP_FLOATEXC ( "Enemy discover distance", 1000, 0, 5000, "Enemy discover distance" )
MOP_FLOATEXC ( "Cannon fire level", 0.f, 0.f, 1.f, "Target for cannon fire: 0 = hull, 1 = sails, 0.0 - 1.0 = probability for shoot to sail" )
MOP_ENUMBEG ( "BehaviorType" )
  MOP_ENUMELEMENT ( "Fire" )
  MOP_ENUMELEMENT ( "Boarding" )
  MOP_ENUMELEMENT ( "Run" )
MOP_ENUMEND
MOP_ENUM  ( "BehaviorType", "Attacking Behavior Type" )
MOP_ShipControllerParams
MOP_ENDLIST ( ShipBattleAIParams )
