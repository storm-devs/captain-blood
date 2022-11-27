

#include "ShipsAIService.h"

#include "ShipAIBattle.h"
#include "ShipAITrack.h"


CREATE_SERVICE ( ShipAIService, 120 )

ShipAIService::ShipAIService ()
{
}

ShipAIService::~ShipAIService ()
{
}

bool ShipAIService::Init ()
{
	return true;
}

IShipController* ShipAIService::CreateBattleShipAI(IShip & rShip, dword dwBase, dword dwMask)
{
	IMission* pMis = (IMission*)(dwBase ^ dwMask);
	IShipController* p = NEW Battle_ShipAI ( rShip, *pMis );
	return p;
}

IShipController* ShipAIService::CreateTrackShipAI(IShip & rShip, dword dwBase, dword dwMask)
{
	IMission* pMis = (IMission*)(dwBase ^ dwMask);
	IShipController* p = NEW TrackMove_ShipAI ( rShip, *pMis );
	return p;
}

IShipController* ShipAIService::CreateShipController ( const ConstString & sParamObjectName,  IShip & rShip, IMission & rMission )
{
	MOSafePointer safeptr;
	rShip.FindObject( sParamObjectName, safeptr );

	static const ConstString id_IShipControllerParams("IShipControllerParams");
	static const ConstString id_ShipBattleAIParams("ShipBattleAIParams");
	static const ConstString id_ShipTrackAIParams("ShipTrackAIParams");

	if ( safeptr.Ptr() && !safeptr.Ptr()->Is(id_IShipControllerParams) )
		safeptr.Reset();
	if( !safeptr.Ptr() ) return null;

	IShipControllerParams * pParamsObject = (IShipControllerParams*)safeptr.Ptr();
	if ( !pParamsObject )
		return NULL;

	IShipController * pNewShipAI = NULL;

	const ConstString strBattleParamsAI("ShipBattleAIParams");
	const ConstString strTrackParamsAI("ShipTrackAIParams");
	if( pParamsObject->Is ( strBattleParamsAI ) )
	{
		pNewShipAI = NEW Battle_ShipAI ( rShip, rMission );
	}
	else
	if( pParamsObject->Is ( strTrackParamsAI ) )
	{
		pNewShipAI = NEW TrackMove_ShipAI ( rShip, rMission );
	}

	if ( pNewShipAI )
		pNewShipAI->SetControllerParams ( pParamsObject );

	return pNewShipAI;
}

void ShipAIService::ReleaseShipController ( IShipController * pShipAI )
{
	Assert ( pShipAI );
	delete pShipAI;
}
