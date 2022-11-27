#ifndef Stalactite_MISSION_OBJECT
#define Stalactite_MISSION_OBJECT

#include "..\..\..\Common_h\Mission.h"
#include "..\..\..\Common_h\gmx.h"
#include "..\..\..\Common_h\QSort.h"





class Stalactite : public MissionObject
{
	enum States
	{
		STATE_IDLE = 0,
		STATE_VIBRATION = 1,
		STATE_DROP = 2,
		STATE_DIE = 3,

		STATE_SLEEP = 4,


		STATE_FORCE_DWORD = 0x7fffffff
	};



	States state;

	float vibrationTime;
	float TotalVibrationTime;

	float fDamage; 
	float fRandTime;

	float fCurrentRandTime;

	IGMXScene* GMXModel;
	Vector start_pos;
	Vector pos;

	Vector velocity;

	Vector ang;


	MissionTrigger endTrigger;
	
	float timeFromLastDamage;

	float activateRadius;
	bool bShowActivator;


	float height;
	float sizeX;
	float sizeZ;

	string target;
	bool bTryToFind;
	//MissionObject * targetPtr;
	MOSafePointer targetPtr;

	string waterSplashSFX;
	string vibrationSFX;

public:

	Stalactite();
	~Stalactite();

	void _cdecl Realize(float fDeltaTime, long level);

	void Activate(bool isActive);
	void Show(bool isShow);

	bool Create(MOPReader & reader);
	bool EditMode_Update(MOPReader & reader);

	
	void EditMode_GetSelectBox(Vector & min, Vector & max);
	Matrix& GetMatrix(Matrix & mtx);

};


#endif