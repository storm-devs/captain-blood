#include "../../../common_h/ai/aiPathFinder.h"
#include "ShipAIMovePath.h"

//============================================================================================
//реализация класса АИ корабля для следования по пути
//============================================================================================
MovePath_ShipAI::MovePath_ShipAI ( IShip & rShip, IMission & rMission ) :
ShipController(rShip,rMission),
m_rotSpeed(1.0f),
m_radius(5.0f),
m_curNode(0)
{
}

MovePath_ShipAI::~MovePath_ShipAI()
{

}

// расчет угла отклонения от траектории (обход препятствий, потенциальные поля)
float MovePath_ShipAI::ComputePotentialFieldAngle()
{
	return 0.0f;
	MissionObject * mo = (MissionObject*)&m_rShip;
	Assert(mo);

	Matrix mtx(true);
	const float FieldRadius = 50.0f;
	float angle = 0.0f;
	if( ShipIterator() ) for( ShipIterator()->Reset(); !ShipIterator()->IsDone(); ShipIterator()->Next() )
	{
		if ( ShipIterator()->Get() && ShipIterator()->Get()!=mo )
		{
			ShipIterator()->Get()->GetMatrix(mtx);
			Vector shipDir = m_rShip.GetPosition() - mtx.pos;
			float dist = shipDir.Normalize();
			if( dist > FieldRadius )
				continue;
			angle += 5.0f*shipDir.GetAY()/dist;
		}
	}
	return angle;
}

// управление кораблем
void MovePath_ShipAI::Work( float dltTime )
{
	ShipController::Work( dltTime );

	if (m_curNode < m_path.size())
	{
		m_rShip.SetVelocity(CalcVelocity());

		Vector dirToNode = m_path[m_curNode] - m_rShip.GetPosition();
		float distToNode = dirToNode.Normalize();
		
		float deltaAngle = dirToNode.GetAY() - m_rShip.GetCurrentDirectionAngle();
		deltaAngle += ComputePotentialFieldAngle();
		m_rShip.SetRotate(m_rShip.GetMaxVelocity()*sin(deltaAngle)*m_rotSpeed);

		if (distToNode <= m_radius)
			++m_curNode;
	}
	else
	{
		m_rShip.SetVelocity(0.0f);
		m_rShip.SetRotate(0.0f);
	}
}

// установить объект параметров
void MovePath_ShipAI::SetControllerParams( IShipControllerParams * pControllerParams )
{
	Assert ( pControllerParams );
	ShipController::SetControllerParams(pControllerParams);

	MO_IS_IF(tid, "ShipWayMoveAIParams", pControllerParams)
	{
		ShipWayMoveAIParams * wayMoveAIParams = static_cast<ShipWayMoveAIParams*>( pControllerParams );

		m_rotSpeed = wayMoveAIParams->GetSpeed();
		m_radius = wayMoveAIParams->GetRadius();
		m_target = wayMoveAIParams->GetTarget();
		m_nodesObject = wayMoveAIParams->GetNodesObject();
		BuildWay();
	}
}

// создать маршрут до целевой точки
void MovePath_ShipAI::BuildWay()
{
	/*aiPathFinder * pPathFinder =  ( aiPathFinder* )m_rMission.FindObject ( m_nodesObject );

	if ( pPathFinder )
	{
		//pPathFinder->GeneratePath_Dijkstra ( m_rShip.GetPosition(), m_target, m_path );
		m_curNode = 0;
	}*/
}

bool ShipWayMoveAIParams::Create ( MOPReader & reader )
{
	ReadMOPs(reader);
	return true;
}

void ShipWayMoveAIParams::ReadMOPs( MOPReader & reader )
{
	m_radius = reader.Float();
	m_rotSpeed = reader.Float();
	m_target = reader.Position();
	m_nodesObject = reader.String().c_str();
	ShipControllerParams::ReadMOPs(reader);
}

MOP_BEGINLISTG  ( ShipWayMoveAIParams, "ShipWayMoveAIParams", '1.00', 0, "Arcade Sea" )
	MOP_ShipWayMoveAIParams
MOP_ENDLIST ( ShipWayMoveAIParams )
