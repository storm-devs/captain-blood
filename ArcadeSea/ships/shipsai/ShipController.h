
#ifndef _ShipController_h_
#define _ShipController_h_

#include "..\..\..\Common_h\IShip.h"

class ShipControllerParams : public IShipControllerParams
{
public:
	struct AIZoneParam
	{
		float rad;
		Vector center;
	};

private:
	AIZoneParam m_AIZone;
	bool m_bPassObstacles;

public:
	ShipControllerParams();

	bool ReadMOPs( MOPReader & reader );

	const AIZoneParam& GetAIZone() {return m_AIZone;}
	bool IsPassObstacles() {return m_bPassObstacles;}
	void _cdecl Work(float fDeltaTime, long level);

	void SetUpdateForDebugShow();

	//-----------------------------------------------------------------------------
	// MissionObject
	//-----------------------------------------------------------------------------
	//Инициализировать объект
	MO_IS_FUNCTION(ShipControllerParams, IShipControllerParams);

};
#define MOP_ShipControllerParams \
	MOP_FLOATC("AIZoneRadius",0.f,"AI zone radius") \
	MOP_POSITIONC("AIZonePosition",Vector(0.f),"AI zone position") \
	MOP_BOOLC("WalkObstacles",false,"Walk around all obstacles")


// базовый класс управления  поведением корабля
class ShipController : public IShipController
{
public:
    ShipController ( IShip & rShip, IMission & rMission );
											
	// задать объект параметров
	virtual void SetControllerParams ( IShipControllerParams * pControllerParams );
	// управление кораблем
	virtual	void Work ( float dltTime ) {};

	// отрисовать дебажную инфу
	virtual void ViewDebugInfo() {}

	MGIterator* ShipIterator();

	bool IsInsideAIZone();
	// найти ближайшего противника
	MissionObject* FindNearEnemy(float fLookRadius);

	bool IsPassObstacles() {return m_bPassObstacles;}

protected:
	IShip&		m_rShip;  // управляемый корабль
	IMission&	m_rMission; // объект миссии
	MGIterator*	m_pShipIterator;

	ShipControllerParams::AIZoneParam m_AIZone;
	bool m_bPassObstacles;
};

#endif
