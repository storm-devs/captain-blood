
#ifndef _PlayerShipControlStorm_h_
#define _PlayerShipControlStorm_h_

#include "ShipsAI.h"

class PlayerShipControllerParamsStorm : public IShipControllerParams
{
public:
	PlayerShipControllerParamsStorm ();

	//-----------------------------------------------------------------------------
	// интерфейс, наследуемый от MissionObject
	//-----------------------------------------------------------------------------
	//Инициализировать объект
	bool Create ( MOPReader & reader );
	//Инициализировать объект в режиме редактирования
	bool EditMode_Create ( MOPReader & reader );
	//Обновить параметры в режиме редактирования
	bool EditMode_Update ( MOPReader & reader );
	//
	MO_IS_FUNCTION(PlayerShipControllerParamsStorm, IShipControllerParams);

	float	m_minSpeed;
	float	m_stormForce1;
	float	m_stormFreq1;
	float	m_stormForce2;
	float	m_stormFreq2;
	

private:

	// чтение параметров
	bool Update (MOPReader & reader);
};

// объявление класса управления  корабля игроком
class PlayerShipControllerStorm : public ShipController
{
	float	m_time;

public:
	PlayerShipControllerStorm ( IShip & rShip, IMission & rMission );
	virtual ~PlayerShipControllerStorm ();

	// управление кораблем
	virtual void Work( float dltTime );
	// установить объект параметров
	virtual void SetControllerParams( IShipControllerParams * pControllerParams );
	// получить текущее АИ-состояние
	virtual ShipControlType GetType() const { return SHIP_AI_STATE_PLAYER_CONTROL; }

private:

	PlayerShipControllerParamsStorm * m_pPlayerShipControllerParams;
};


#endif