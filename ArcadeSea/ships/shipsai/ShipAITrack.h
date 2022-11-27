
#ifndef _ShipsAITrack_h_
#define _ShipsAITrack_h_

#include "ShipsAI.h"
#include "ShipAITrackParams.h"
#include "WayTrack.h"

class TrackMove_ShipAI : public ShipAI
{
public:
	TrackMove_ShipAI ( IShip & rShip, IMission & rMission );
	~TrackMove_ShipAI ();

	// управление кораблем
	void Work( float dltTime );
	// установить объект параметров
	void SetControllerParams( IShipControllerParams * pControllerParams );
	// отрисовать дебажную инфу
	virtual void ViewDebugInfo();

	// получить текущее АИ-состояние
	ShipControlType GetType();
	// получить  объект преследования
	MissionObject *	GetEnemy();

private:
	// АИ стрельбы
	void ProcessAIFire ( float fDTime );
	// прекращение атаки выбранной цели
	void StopAttacking ();
	// отрисовка дебажной инфы
	void DrawDebugInfo ();

	ShipTrackAIParams *		m_pShipTrackAIParams;	// объект параметров
	ShipWayTrack			m_shipWayTrack;			// трек

	Vector					m_vTargetPos;			// текущая целевая точка

	bool m_bShowDebugInfo;
};

#endif
