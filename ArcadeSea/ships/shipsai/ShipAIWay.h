

#ifndef _ShipAIWay_h_
#define _ShipAIWay_h_

#include "..\..\..\Common_h\ai\aiPathFinder.h"

#include "ShipAIWayNode.h"


// объявление класса пути корабля
class ShipWay 
{
public: 
	ShipWay ();

	ShipWay ( const Path * pPath, NodeState eWayState = NODE_STATE_CAN_PASS  );

	~ShipWay ();
public:
	//добавить ноду
	void								AddNode ( const WayNode & tNode );
	//удалить ноду    
	void							    DeleteNode ( int iNode );
	//получить ноду
	const WayNode  &		GetNode ( int iNode ) const;
	//получить длину пути
	int								    GetNodesCount () const;
	// очистить
	void							    Free ();

private:

	array<WayNode>			m_dNodes;
};

#endif