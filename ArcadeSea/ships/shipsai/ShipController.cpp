
#include "ShipController.h"
#include "..\..\SeaMissionParams.h"



ShipControllerParams::ShipControllerParams()
{
	m_AIZone.rad = 0.f;
	m_AIZone.center = 0.f;
}

bool ShipControllerParams::ReadMOPs( MOPReader & reader )
{
	m_AIZone.rad = reader.Float();
	m_AIZone.center = reader.Position();
	m_bPassObstacles = reader.Bool();
	return true;
}

void ShipControllerParams::SetUpdateForDebugShow()
{
	SetUpdate(&ShipControllerParams::Work, ML_DEBUG);
}

void ShipControllerParams::Work(float fDeltaTime, long level)
{
	if( m_AIZone.rad > 0.f )
		Render().DrawXZCircle( m_AIZone.center+Vector(0.f,1.f,0.f), m_AIZone.rad, 0x80FFFFFF );
}





ShipController::ShipController ( IShip & rShip, IMission & rMission ) :
	m_rShip ( rShip ),
	m_rMission ( rMission )
{
	m_pShipIterator = NULL;
	m_AIZone.rad = 0.f;
	m_AIZone.center = 0.f;
	m_bPassObstacles = false;
}

void ShipController::SetControllerParams ( IShipControllerParams * pControllerParams )
{
	static const ConstString tipeId("ShipControllerParams");
	if( pControllerParams && pControllerParams->Is(tipeId) )
	{
		m_AIZone = ((ShipControllerParams*)pControllerParams)->GetAIZone();
		m_bPassObstacles = ((ShipControllerParams*)pControllerParams)->IsPassObstacles();
	}
}

bool ShipController::IsInsideAIZone()
{
	if( m_AIZone.rad<=0.f ) return true;
	Matrix m(true);
	m_rShip.GetMatrix(m);
	if( (~(m.pos - m_AIZone.center)) < m_AIZone.rad*m_AIZone.rad )
		return true;
	return false;
}

MissionObject* ShipController::FindNearEnemy(float fLookRadius)
{
	MissionObject* pEnemy = NULL;

	// ищем только пока находимся в рабочей зоне
	bool bInside = m_AIZone.rad<=0.f;
	if( !bInside )
	{
		Matrix m(true);
		m_rShip.GetMatrix(m);
		float fdistK2 = (~(m.pos - m_AIZone.center)) / (m_AIZone.rad*m_AIZone.rad);
		// мы в пределах 70% зоны 0.7*0.7=0.49 (значит ищем противника - это, что бы постоянно не искать противника на границе зоны)
		bInside = fdistK2 < 0.49f;
	}
	if( bInside )
	{
		Vector vShipPos = m_rShip.GetPosition();
		Vector vMinAreaCorner =  vShipPos + Vector ( -fLookRadius, -fLookRadius, -fLookRadius );
		Vector vMaxAreaCorner =  vShipPos + Vector ( fLookRadius, fLookRadius, fLookRadius );

		int iTargetsCount = m_rShip.QTFindObjects(MG_DAMAGEACCEPTOR, vMinAreaCorner, vMaxAreaCorner);

		float fMinDistance = fLookRadius;

		for ( int i = 0 ; i < iTargetsCount; ++i )
		{
			IMissionQTObject * pObjFinder = m_rShip.QTGetObject( i );
			Assert ( pObjFinder );

			MissionObject * pMissionObject = &pObjFinder->GetMissionObject ();
			Assert  ( pMissionObject );

			if ( pMissionObject == &m_rShip )
				continue;

			if ( pMissionObject->IsDead () || !pMissionObject->IsActive() || !pMissionObject->IsShow())
				continue;

			Matrix tmObjectMatr;

			pMissionObject->GetMatrix ( tmObjectMatr );
			Vector vObjPos = tmObjectMatr.pos;

			Vector vTargetOrient  = vObjPos - vShipPos;
			float fTargetDist = vTargetOrient.GetLength ();

			ShipParty eTargetPaty = SHIP_SIDE_NEUTRAL;

			static const ConstString tipeId("Ship");
			IShip * pTargetShip = pMissionObject->Is(tipeId) ? (IShip*)pMissionObject : null;

			if ( pTargetShip )
				eTargetPaty = pTargetShip->GetParty ();

			ShipParty eMyPaty = m_rShip.GetParty ();

			bool bHostile = eMyPaty!=eTargetPaty && eTargetPaty!=SHIP_SIDE_NEUTRAL; // не та же партия и не нейтральная

			if ( bHostile && fTargetDist < fMinDistance )
			{
				pEnemy  = pMissionObject;
				fMinDistance = fTargetDist;
			}
		}
	}

	return pEnemy;
}

MGIterator* ShipController::ShipIterator()
{
	if( !m_pShipIterator )
	{
		MOSafePointer sp;
		static const ConstString paramsId("SeaMissionParams");
		m_rMission.FindObject( paramsId, sp );
		static const ConstString tipeId("SeaMissionParams");
		if( sp.Ptr() && sp.Ptr()->Is(tipeId) )
		{
			m_pShipIterator = ((SeaMissionParams*)sp.Ptr())->GetShipIterator();
		}
	}
	return m_pShipIterator;
}
