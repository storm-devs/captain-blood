
#ifndef _ShipAIBattleParams_h_
#define _ShipAIBattleParams_h_

#include "ShipsAI.h"

enum BattleBehaviorType
{
	SHIP_BATTLE_TYPE_FIRE = 0,
	SHIP_BATTLE_TYPE_BOARDING,
	SHIP_BATTLE_TYPE_RUN
};

// параметры АИ боя
class ShipBattleAIParams : public ShipControllerParams
{
public:

	ShipBattleAIParams();

	//-----------------------------------------------------------------------------
	// интерфейс, наследуемый от MissionObject
	//-----------------------------------------------------------------------------
	//Инициализировать объект
	virtual bool Create ( MOPReader & reader );
	//Инициализировать объект в режиме редактирования
	virtual bool EditMode_Create ( MOPReader & reader );
	//Обновить параметры в режиме редактирования
	virtual bool EditMode_Update ( MOPReader & reader );
	//
	MO_IS_FUNCTION(ShipBattleAIParams, ShipControllerParams);

	MOSafePointer			m_pEnemy;					// объект противника
	float					m_fOptimumFireDistance;     // оптимальная дальность до противника при стрельбе
	BattleBehaviorType		m_eBehaviorType;
	float					m_fMaxFireRadius;
	float					m_fEnemyDiscoverDistance;
	float					m_fFireLevel;				// уровень стрельбы из пушек (0.f-по корпусу, 1.f-по парусам, между ними - вероятность)

private:

	bool Update( MOPReader & reader );
};



#endif