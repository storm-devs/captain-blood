#ifndef AI_NPC
#define AI_NPC

#include "aiBrain.h"
#include "../Thoughts/aiStanding.h"


#include "../../auxiliary objects/NPCPoints/NPCPoints.h"

class aiNPC : public aiBrain
{
public:
	
	aiThoughtStanding* pThoughtStandingFear;
	aiThoughtStanding* pThoughtStandingTalk;

	int fear_pt_index;
	NPCPoints* pFearPoints;
	NPCPoints* pTalkPoints;
	bool isPanicOnStart;
	aiNPC(Character & ch, const char* szName);

	ConstString FearPointsName;
	ConstString TalkPointsName;

	float fPanicCooldown;
	bool  bPointTaken;
	void  GotoFearPoint(int index);
	void  TakePoint(bool take);

	virtual void UpdateMO();

	virtual void Reset();

	virtual void SetParams(CharacterControllerParams * params);	

	virtual void Tick();	
			
	bool    StartTalk();

	virtual void NotifyAboutAttack(Character* attacker,float damage);

	virtual void Update(float dltTime);
	
	virtual void StartPanic();

	virtual void Hit(float dmg,Character* _offender, const char* reaction, DamageReceiver::DamageSource source);		

	virtual void Death();

	virtual const ConstString & GetAIParamsName() { RETURN_CONSTSTRING("NPCParams"); };
};


#endif


