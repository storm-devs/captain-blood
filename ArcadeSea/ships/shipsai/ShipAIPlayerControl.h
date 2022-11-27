
#ifndef _PlayerShipControl_h_
#define _PlayerShipControl_h_

#include "ShipsAI.h"

#define CONTROL_DEBUG_DRAW false

class PlayerShipControllerParams : public IShipControllerParams
{
public:
	PlayerShipControllerParams ();

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
	MO_IS_FUNCTION(PlayerShipControllerParams, IShipControllerParams);

	long	m_nPlayerIndex;
	float	m_fMaxFireRadius;
	bool	m_bCamRelationalControl;
	float	m_fCannonFireLevel;
	float	m_fCannonBtnMaxTime;
	float	m_fMousePower;
	float	m_fMouseTime;

private:

	// чтение параметров
	bool Update (MOPReader & reader);
};

// объявление класса управления  корабля игроком
class PlayerShipController : public ShipController
{
public:
	PlayerShipController ( IShip & rShip, IMission & rMission );
	virtual ~PlayerShipController ();

	// управление кораблем
	virtual void Work( float dltTime );
	// установить объект параметров
	virtual void SetControllerParams( IShipControllerParams * pControllerParams );
	// получить текущее АИ-состояние
	virtual ShipControlType GetType() const { return SHIP_AI_STATE_PLAYER_CONTROL; }

protected:
	
	// отрисовка дебужной информации
	virtual void DrawDebugInfo ();

	void UpdateControlKeys();
	long FindControl(const char* pcBaseName,long nIndex);

private:

	PlayerShipControllerParams *	m_pPlayerShipControllerParams;
	long							m_moveForwardCtrl;
	long							m_rotateCtrl;
	long							m_fireCannonsCtrl;
	//long							m_fireKnippelsCtrl;
	long							m_fireFlameCtrl;
	long							m_fireMinesCtrl;

	long							m_mouseForwardCtrl;
	long							m_mouseSideCtrl;

	bool							m_bCannonBtnIsPressed;
	float							m_fCannonBtnTime;

	float m_mouseMoveMin;
	float m_mousePower;
	float m_mouseCurUD;
	float m_mouseCurLR;
	float m_mouseTime;
};


#endif