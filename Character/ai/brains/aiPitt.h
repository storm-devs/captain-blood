#ifndef AI_PITT_BRAIN
#define AI_PITT_BRAIN

#include "aiBrain.h"
#include "../Thoughts/Pitt/aiPittFatality.h"
#include "../Thoughts/Pitt/aiPittBlocking.h"
#include "../Thoughts/Pitt/aiPittRoll.h"

class aiPitt : public aiBrain
{		
	aiPittFatality*   pPittFatality;	
	aiPittRoll*       pPittRoll;	

	float fMaxDistToPlayer;
	float fDistToResetAndFollowPlayer;

	bool   bLeadMode;
	Vector destination;

public:

	aiPitt(Character & ch, const char* szName);
	virtual ~aiPitt();

	virtual void SetParams(CharacterControllerParams * params);	
	
	//Обновить состояние контролера 10 раз в секунду (распределено между персонажами)
	virtual void Tick();
	
	//Персонаж получил люлей
	virtual void Hit(float dmg,Character* _offender, const char* reaction, DamageReceiver::DamageSource source);	

	virtual bool FilterFindedTarget(Character* pTarget, bool statistSearch);

	virtual bool CheckCharState(Character* target);

	virtual bool AttackCharacter();

	virtual void FollowPlayer();	

	virtual const ConstString & GetAIParamsName(){ RETURN_CONSTSTRING("PittParams"); }

	virtual bool AllowToBeBlowed() { return false; };

	virtual void Command(dword numParams, const char ** params);
};


#endif


