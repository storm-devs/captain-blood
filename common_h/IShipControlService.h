#ifndef _IShipControlService_h_
#define _IShipControlService_h_

#include "IShip.h"

// интерфейс сервиса для создания объектов ИИ корабля
class IShipControlService : public Service
{
public:
  
	virtual bool Init() = 0;
	virtual IShipController* CreateShipController ( const ConstString & sParamObjectName, IShip & rShip, IMission & rMission ) = 0;
	virtual void ReleaseShipController ( IShipController * pShipController ) = 0;
};


#endif