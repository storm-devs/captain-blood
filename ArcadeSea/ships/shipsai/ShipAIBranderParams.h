
#ifndef _ShipAIBranderParams_h_
#define _ShipAIBranderParams_h_

#include "ShipsAI.h"

class ShipBranderAIParams : public ShipControllerParams
{
public:
	ShipBranderAIParams ();

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
	MO_IS_FUNCTION(ShipBranderAIParams, ShipControllerParams);

	IShip * m_pTarget;		        // объект преследования

private:

	bool Update ( MOPReader & reader );
};

#endif