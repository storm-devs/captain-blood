
#ifndef _ShipsWayTrack_h_
#define _ShipsWayTrack_h_

#include "..\..\..\common_h\mission.h"
#include "ShipAITrackParams.h"

struct ShipWayPoint
{
	MissionObject* pEnemy;
	bool bFollowToPoint;

	union
	{
		struct // Follow
		{
			Vector pos;
			float speedK;
		};

		struct // Behaviour
		{
			float rotate;
			float velocity;
			float offset;
		};
	};

	//Empty constructor
	ShipWayPoint() {}
};

class ShipWayTrack
{
public:

public:
	ShipWayTrack();
	~ShipWayTrack();

	bool InitWayTrack(ShipTrackAIParams* pAIParams);
	void ResetTrack();

	MissionObject* GetEnemy() {return m_wayPoint.pEnemy;}
	const ShipWayPoint& GetWayPoint( float dltTime, const Matrix & mtx );

	void ShowDebugInfo(IRender & render);

private:
	void FollowToWayPoint(long n);
	bool LaunchBehaviour(const Matrix & mtx);
	void ExecuteStrafe(float dltTime, const Matrix& mtx);

	ShipTrackAIParams* m_pAIParams;
	array<MOPShipWayPoint>	m_aTrackData;		// текущая целевая точка

	MissionObject* m_pDefaultEnemy;

	long m_nCurWayNum;
	ShipWayPoint m_wayPoint;

	float m_fBehaviourTime;

	struct StrafeBehaviour
	{
		float rotSpeed;
		float basesin,basecos;
		float maxAngle;
		float maxSpeed;
		float x, z;
		float asin, acos;
	} m_targ;
	void SetStrafePoint(const Matrix& mtx);
	void SetStrafeAng(const Matrix& mtx);
};

#endif
