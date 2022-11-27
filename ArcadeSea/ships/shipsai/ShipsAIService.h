
#ifndef _ShipsAIService_h_
#define _ShipsAIService_h_

#include "..\..\..\Common_h\IShipControlService.h"

//  сервис для создания объектов ИИ корабля
class ShipAIService : public IShipControlService
{
public:
	ShipAIService ();
	virtual ~ShipAIService();

	bool Init();
	IShipController* CreateShipController ( const ConstString & sParamObjectName, IShip & rShip, IMission & rMission );
	void ReleaseShipController ( IShipController * pShipAI );

	static IShipController* __cdecl CreateBattleShipAI(IShip & rShip, dword dwBase, dword dwMask);
	static IShipController* __cdecl CreateTrackShipAI(IShip & rShip, dword dwBase, dword dwMask);
};


#endif