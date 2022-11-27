
#include "ShipAIWay.h"

//============================================================================================
//реализация класса пути корабля
//============================================================================================
ShipWay::ShipWay ()
:m_dNodes( _FL_ )
{
	m_dNodes.Reserve ( 20 );
};

ShipWay::ShipWay ( const Path * pPath, NodeState eWayState )
:m_dNodes (_FL_)
{
	if ( !pPath )
		return;

	int iNodesCount = pPath->size ();
	m_dNodes.Reserve ( iNodesCount );

	for ( int i = 0; i < iNodesCount; ++i )
	{
		Vector vPoint = pPath->getPoint ( i );
		WayNode tNode ( vPoint.x, 0, vPoint.z, eWayState );

		if ( iNodesCount - 1 ==  i )
		{
			tNode.m_eState = NODE_STATE_EXACTLY;
		};

		m_dNodes.Add ( tNode );
	};
};

ShipWay::~ShipWay ()
{
	Free ();
};

void	ShipWay::AddNode ( const WayNode & tNode )
{
	m_dNodes.Add ( tNode );
};

void	ShipWay::DeleteNode ( int iNode )
{
	m_dNodes.DelIndex ( iNode );
};

const WayNode	&	ShipWay::GetNode ( int iNode ) const
{
	Assert ( iNode >= 0 && iNode < ( int )m_dNodes.Size ()  );
	return m_dNodes[iNode];
};

int	ShipWay::GetNodesCount () const
{
	return m_dNodes.Size ();
};

void	ShipWay::Free ()
{
	m_dNodes.DelAll ();
};
