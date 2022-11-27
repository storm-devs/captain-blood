
#ifndef _ShipsAI_h_
#define _ShipsAI_h_

#include "ShipController.h"

//#include "..\..\..\Common_h\IShip.h"

#include "ShipAIDebug.h"

enum NodeState
{
	NODE_STATE_CAN_PASS = 0,
	NODE_STATE_CANNOT_PASS,
	NODE_STATE_EXACTLY
};

// объявление класса АИ корабля
class ShipAI : public ShipController
{
public:
	ShipAI (  IShip & rShip, IMission & rMission  );
	virtual	~ShipAI ();

	// управление кораблем
	virtual void Work ( float dltTime ) {ShipController::Work(dltTime);}
	// установить объект параметров
	virtual void SetControllerParams ( IShipControllerParams * pControllerParams  ){ShipController::SetControllerParams(pControllerParams);};
	// получить текущее АИ-состояние
	virtual ShipControlType	GetType () const { return SHIP_AI_STATE_WAIT; }

	// отрисовать дебажную инфу
	virtual void ViewDebugInfo() {}

	bool IsFinishMove() {return m_bFinished;}

protected:
	// задать целевую точку
	void SetTargetPos( const Vector& vTarget, NodeState eTargetState = NODE_STATE_CAN_PASS  );
	// обработка  хождения по пути к заданной точке, возвращает true, еслм цель достигнута, или находится на препятствии
	bool MoveProc( float fDTime );
	// установить объект, не являющийся препятствием
	void SetObstacleExclusion( MissionObject * pNoObstacle );
	// задать оптимальную скорость
	void SetOptimumSpeed( float fOptSpeed );
	// остановка  движения 
	void StopMove();
	// получить время, необхожимое для разворота корабля
	float GetRotationTime( float fNewDirAngle )  const;
	// отрисовка дебужной информации
	virtual void DrawDebugInfo();
	// посчитать скорость
	virtual float CalcVelocity(const Vector&, float, float);

	float m_fShipRadiusZ;			// длина корабля
	float m_fShipRadiusX;			// ширина корабля
	float m_fShipAIVelocityMax;
	float m_fShipAIObstaclesVelocityMax;
	bool m_bOn;

private:
	
	// задать требуемое положение оси корабля
	void SetDirectionAngle ( float fTargetAngle, float fDTime );
	// отрисовка дебажных эллипсов
	void DrawBoundingEllipse ( float fXRad, float fZRad, bool bSolid = true ); 

	Vector					m_vCurrTargetPoint;			// текущая целевая точка
	NodeState				m_eTargetPointState;		// нужно ли обязательно достигать точку
	bool					m_bFinished;				// закончено ли движение
	float					m_fOptimumSpeed;			// оптимальная скорость
	MissionObject *			m_pObstacleExclusion;		// объект, который в данный момент не может считаться препятствием
	array<DebugArrow>		m_dObstacleFields;			// дебажная инфа
	float					m_fOldAngleSpeed;			// запомненная угловая скорость
	float					m_fTimeFromLastRotChange;	// время с последней смены разворота
	float					m_zeroSpeedTime;

	bool			m_bFindIslandsMap;
	MOSafePointer	m_IslandsMap;
};


#endif