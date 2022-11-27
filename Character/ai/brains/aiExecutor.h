#ifndef AI_Executor
#define AI_Executor

#include "aiBrain.h"
#include "../Thoughts/Executor/aiExecutorPray.h"

class aiExecutor : public aiBrain
{	
	aiExecutorPray*   pExecutorPray;	

	Vector pray_pos;	

	int   iNumHelpers;
	ConstString cHelpers[5];

	ConstString cHelpersTriger;

	float fHealingRate;

	struct TPhase
	{
		float when_pray;
		float pray_until;
	};

	int    iNumPhases;
	TPhase phases[5];

	int    iCurPhase;

	ConstString cFt1Triger;
	ConstString cFt2Triger;

	int   count;
public:

	aiExecutor(Character & ch, const char* szName);
	virtual ~aiExecutor();

	virtual void SetParams(CharacterControllerParams * params);	

	//Перезапустить контроллер при перерождении персонажа
	virtual void Reset();
	
	//Обновить состояние контролера 10 раз в секунду (распределено между персонажами)
	virtual void Tick();
	
	//Персонаж получил люлей
	virtual void Hit(float dmg,Character* _offender, const char* reaction, DamageReceiver::DamageSource source);

	virtual bool AttackCharacter();

	//Обновить состояние контролера на каждом кадре
	virtual void Update(float dltTime);	

	virtual const ConstString & GetAIParamsName(){ RETURN_CONSTSTRING("ExecutorParams"); }

	virtual bool AllowToBeBlowed() { return false; };

	virtual void Command(dword numParams, const char ** params);
};


#endif


