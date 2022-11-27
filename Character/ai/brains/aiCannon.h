#ifndef AI_Cannon
#define AI_Cannon

#include "aiBrain.h"

class aiCannon : public aiBrain
{		
	enum EnemyBrainState
	{
		EBS_IDLE = 0,
		EBS_PURSUIT,
		EBS_RUNAWAY,
		EBS_DROPBOMB,
		EBS_KICK,		
		EBS_STRAFING,
		EBS_PLACEBOMB,
		EBS_SUICIDE
	};

	EnemyBrainState BrainState;
	
	float fDistDeadZone;
	float fDistToShoot;	
	
	float fMinTimeToShoot;
	float fMaxTimeToShoot;

	float fTimeToShoot;

	bool  bAngleHasBeenInit;
	float fInitialAngle;
	float fShootSektor;

public:

	aiCannon(Character & ch, const char* szName);
	virtual ~aiCannon();

	virtual void SetParams(CharacterControllerParams * params);	

	//Перезапустить контроллер при перерождении персонажа
	virtual void Reset();	

	//Обновить состояние контролера на каждом кадре
	virtual void Update(float dltTime);

	//Обновить состояние контролера 10 раз в секунду (распределено между персонажами)
	virtual void Tick();

	void Boom(const Vector & pos);	

	//Поиск цели
	virtual Character* SearchTarget(bool bCheckMaxAllowedAttackers);

	virtual const ConstString & GetAIParamsName(){ RETURN_CONSTSTRING("CanonParams"); }
};


#endif


