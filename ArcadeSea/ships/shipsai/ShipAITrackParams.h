
#ifndef _ShipAITrackParams_h_
#define _ShipAITrackParams_h_

#include "..\..\..\common_h\mission.h"
#include "ShipController.h"

class MOPShipWayPoint
{
public:
	enum BehaviourType
	{
		Behaviour_follow,
		Behaviour_strafe
	};

	MOPShipWayPoint();
	~MOPShipWayPoint() {}

	void ReadMOPs(MOPReader& reader);
	void Empty();

public:
	const char* pcEnemyName;

	// behaviour describe
	BehaviourType eBehaviourType;
	float fBehaviourTime;

	// behaviour parameters
	//----- follow --------
	float speedK;
	Vector vPos;
	float fDetectorRadius;
	//----- strafe --------
	float strafeZoneWidth;
	float strafeZoneHeight;
	float fStrafeRotSpeed;
	float fStrafeMaxAngle;
	float fStrafeMaxSpeed;
	float fStrafeBaseAngle;
	Vector vStrafePosOffset;
	float fStrafeAngOffset;

	MissionTrigger triggerCome;
	long nNextPoint;
};

// параметры АИ боя
class ShipTrackAIParams : public ShipControllerParams
{
public:
	struct BehaviourParams
	{
		ConstString pcEnemyName;

		float fStrafeRotSpeed;
		float fStrafeMaxAngle;
		float fStrafeMaxSpeed;
		float fStrafeBaseAngle;
	};

private:
	ConstString m_pcTransformObjName;
	MOSafePointer m_sptrTransformObj;
	Matrix m_mtxTransform;

public:
	ShipTrackAIParams();
	~ShipTrackAIParams();

	//-----------------------------------------------------------------------------
	// интерфейс, наследуемый от MissionObject
	//-----------------------------------------------------------------------------
	//Инициализировать объект
	virtual bool Create ( MOPReader & reader );
	virtual void PostCreate();
	//Инициализировать объект в режиме редактирования
	virtual bool EditMode_Create ( MOPReader & reader );
	//Обновить параметры в режиме редактирования
	virtual bool EditMode_Update ( MOPReader & reader );
	//
	MO_IS_FUNCTION(ShipTrackAIParams, ShipControllerParams);
	//
	Matrix & GetMatrix(Matrix& mtx);
	//
	void _cdecl DrawDebug(float fDeltaTime, long level);

public:
	bool m_bShowDebugInfo;

	array<MOPShipWayPoint> m_aWayTrack;

	BehaviourParams m_params;
	bool m_bLoopTrack;

private:
	bool Update( MOPReader & reader );
};



#endif