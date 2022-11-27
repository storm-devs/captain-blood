#ifndef FLYING_CANNON_BALLS
#define FLYING_CANNON_BALLS

#include "..\..\..\Common_h\Mission.h"
#include "..\..\..\Common_h\gmx.h"
#include "..\..\..\Common_h\QSort.h"

#include "..\..\..\Common_h\ITrail.h"

#include "..\TrailParams\TrailParams.h"

class FlyBalls : public MissionObject
{
	bool bRopedCamera;

	float RopedCameraDistance;

	float CameraTargetHeight;

	Vector vCameraPos;

	Vector vCameraLookTo;

	bool bParticlesTrajOrient;

	bool bBallRotation;
	float ballRotationSpeed;

	Vector start;
	Vector end;
	float trajectoryTime;
	float trajectoryHeight1;
	float trajectoryHeight2;
	float trajectoryHeight;

	Vector org_end;
	bool m_active;

	float org_d; // оригинальное значение расстояния броска
	float org_t; // оригинальное значение времени полета
	float org_h; // оригинальное значение высоты броска

	float kLerp;

	float m_a;
	float m_b;

	ConstString startParticles;
	ConstString startSound;

	ConstString flyParticles;
	ConstString flySound;
	const char *flyModel;

	ConstString endParticles;
	ConstString endSound;

	Matrix endMatrix;

	//// trail ////

	bool  useTrail;

	float trailBegin;
	float trailEnd;

	float trailDelayMin;
	float trailDelayMax;

	dword trailColor;

	float trailOffStr;

	Vector trailDir;
	float  trailVel;

	float forceValue;

	bool staticLive;

	///////////////

	ConstString trailName;

	MOSafePointer trailParams;

	float trailUpdated;

	///////////////

	float trailAlpha;

	///////////////

	MissionTrigger endTrigger;

	bool bExplode;
	float fExplodeRadius;
	float fExplodeDamage;
	float fExplodePower;

	float fTimeMultipler;

//	float testTimer;

	float fMinDelay;

	ConstString masterObject;

	float masterObjectDist;


	float randTime;

	Vector vPrevPos;
	Vector vMiddlePoint;

	IParticleSystem* pFlyParticles;
	ISound3D* pFlySound;

	bool resetFlyParticles; // перезапускать партиклы полета при выстреле


	IGMXScene* flyGMXModel;


	bool bFlyed;
	bool bfirstTime;
	float fActivationDelay;

	MissionObject* camController;

	bool bTeleported;
	bool bContinues;
	bool bCamShock;


	float fActiveTime;

//	array<Vector> trajectory;

	Vector mBallRotation;

	Vector vRotationVector;

//	void BuildTrajectory ();
	void BuildPath();


	void Reset ();


	float minY;
	float maxY;
	float difY;



	ILight* light;
	bool bBornLight;
	float LightRadius;
	Color LightColor;
	float LightTime;


	ILight* e_light;
	bool e_bBornLight;
	float e_LightRadius;
	Color e_LightColor;
	float e_LightTime;


	float explodeLightTime;

	ITrail *trail1;
	ITrail *trail2;

public:

	FlyBalls();
	~FlyBalls();

	void _cdecl RealizeView(float fDeltaTime, long level);
	void _cdecl Realize(float fDeltaTime, long level);
	void _cdecl EditModeRealize(float fDeltaTime, long level);

	void Activate(bool isActive);
	void Show(bool isShow);

	void Restart();

	bool Create(MOPReader & reader);
	bool EditMode_Update(MOPReader & reader);

	virtual void Command(const char * id, dword numParams, const char ** params);

	static const ConstString trailManagerId;
};


#endif