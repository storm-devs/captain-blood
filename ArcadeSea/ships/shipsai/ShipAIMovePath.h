#pragma once
#include "../../../common_h/ai/aiPath.h"
#include "ShipsAI.h"

class ShipWayMoveAIParams : public ShipControllerParams
{
	float		m_radius;
	float		m_rotSpeed;
	Vector		m_target;
	const char*	m_nodesObject;
protected:
	void ReadMOPs(MOPReader & reader);
public:
	ShipWayMoveAIParams() {}
	float GetRadius() const { return m_radius; }
	float GetSpeed() const { return m_rotSpeed; }
	const Vector& GetTarget() const { return m_target; }
	const char* GetNodesObject() const { return m_nodesObject; }

	//-----------------------------------------------------------------------------
	// MissionObject
	//-----------------------------------------------------------------------------
	//Инициализировать объект
	virtual bool Create ( MOPReader & reader );
	MO_IS_FUNCTION(ShipWayMoveAIParams, ShipControllerParams);
};
#define MOP_ShipWayMoveAIParams \
	MOP_FLOATC( "Radius", 5.0f, "Rotation radius" ) \
	MOP_FLOATEXC( "Rotate speed", 1.0f, 0.0f, 1.0f, "Rotation speed" ) \
	MOP_POSITION( "Target point", Vector(0.0f) ) \
	MOP_STRING("Way nodes object", "WayPoints") \
	MOP_ShipControllerParams


class MovePath_ShipAI : public ShipController
{
	float			m_rotSpeed;
	float			m_radius;
	Vector			m_target;
	Path			m_path;
	unsigned int	m_curNode;
	const char*		m_nodesObject;

	void BuildWay();
	float ComputePotentialFieldAngle();
public:

	MovePath_ShipAI ( IShip & rShip, IMission & rMission );
	virtual ~MovePath_ShipAI ();
	virtual float CalcVelocity() { return m_rShip.GetMaxVelocity(); }
	IShip& GetShip() { return m_rShip; }
	

	//////////////////////////////////////////////////////////////////////////
	// IShipController
	//////////////////////////////////////////////////////////////////////////
	// управление кораблем
	virtual void Work( float dltTime );
	// установить объект параметров
	virtual void SetControllerParams( IShipControllerParams * pControllerParams );
	
};