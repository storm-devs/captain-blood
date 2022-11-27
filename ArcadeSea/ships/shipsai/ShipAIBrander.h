
#ifndef _ShipsAIBrander_h_
#define _ShipsAIBrander_h_

#include "ShipAIBranderParams.h"

class Brander_ShipAI : public ShipAI
{
public:
	Brander_ShipAI (  IShip & rShip, IMission & rMission  );

	~Brander_ShipAI ();

	// управление кораблем
	void										Work ( float dltTime );
	// установить объект параметров
	void										SetControllerParams ( IShipControllerParams * pControllerParams  );
	// получить текущее АИ-состояние
	ShipControlType					GetType () const;
	// получить  объект преследования
	IShip *									GetTargetShip () const;

private:

	// отрисовка дебажной инфы
	void										DrawDebugInfo ();

	IShip *									m_pTarget;      // объект преследования

	bool										m_bPursuit;      // режим преследования

	ShipBranderAIParams *		m_pShipBranderAIParams; // параметры: цель преследования и др.
};

#endif