#include "IsterlingShipAIParams.h"

IsterlingShipAIParams::IsterlingShipAIParams(void)
{
}

IsterlingShipAIParams::~IsterlingShipAIParams(void)
{
}

void IsterlingShipAIParams::ReadMOPs(MOPReader & reader)
{
	params_.targetName = reader.String();
	params_.minDist = reader.Float();
	params_.maxDist = reader.Float();
	params_.speedBoost = reader.Float();

	ShipWayMoveAIParams::ReadMOPs(reader);
}

bool IsterlingShipAIParams::Create( MOPReader & reader )
{
	ReadMOPs(reader);
	return true;
}


//Инициализировать объект в режиме редактирования
bool IsterlingShipAIParams::EditMode_Create ( MOPReader & reader )
{
	ReadMOPs(reader);
	return true;
}

//Обновить параметры в режиме редактирования
bool IsterlingShipAIParams::EditMode_Update ( MOPReader & reader )
{
	ReadMOPs(reader);
	return true;
}


MOP_BEGINLISTG  ( IsterlingShipAIParams, "Isterling Ship AI Params", '1.00', 0, "Arcade Sea" )
	MOP_STRING("Target", "Player")
	MOP_FLOAT("Min distance", 50)
	MOP_FLOAT("Max distance", 100)
	MOP_FLOATEX("Speed boost", 2.0f, 1.0f, 10.0f)

	MOP_ShipWayMoveAIParams
MOP_ENDLIST ( IsterlingShipAIParams )
