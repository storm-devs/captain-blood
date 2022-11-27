#ifndef AI_BRAIN_CLASS
#define AI_BRAIN_CLASS

#include "..\Thoughts\aiThought.h"
#include "..\..\..\Common_h\Mission.h"
#include "..\CharacterAI.h"

#include "../../auxiliary objects/CombatPoints/CombatPoints.h"
#include "../../auxiliary objects/SafatyPoints/SafatyPoints.h"
#include "../../auxiliary objects/AI Zones/AIZones.h"

#include "../Thoughts/aiKick.h"
#include "../Thoughts/aiPursuit.h"
#include "../Thoughts/aiStanding.h"
#include "../Thoughts/aiStrafing.h"
#include "../Thoughts/aiStrafing.h"
#include "../Thoughts/aiBlocking.h"

#include "../../character/components/CharacterInitAI.h"

//Базовый класс для мозга, являеться менеджером мыслей...
class aiBrain :  public CharacterController
{
	class BrainAnimListener : public IAnimationListener
	{
	public:
		aiBrain* brain;

		BrainAnimListener()
		{
			brain = NULL;
		}

		void _cdecl ToPair(IAnimation * ani, const char * name, const char ** params, dword numParams)
		{
			if (numParams<1)
			{
				return;
			}

			if(brain && string::IsEqual(params[0],"Pair"))
			{
				if (!brain->GetChrTarget()) return;
				brain->GetBody()->logic->PrepareForPairMode(brain->GetChrTarget(),params[1],FatalityParams::ingame);
			}
		};
	};

	BrainAnimListener brainAnimListener;

private:
	Character* pBody;

	struct BrainThought
	{		
		aiThought* Task;
	};

	array<BrainThought> Thoughts;

	int iActiveThought;	
		
	ICoreStorageFloat * enemyAttackSpeedModifier;

protected:
	
	enum EnemyBrainState
	{
		EBS_IDLE = 0,
		EBS_PURSUIT,		
		EBS_KICK,		
		EBS_STRAFING,
		EBS_BLOCKING,
		EBS_STANDING,		
		EBS_RUNAWAY,
		EBS_SHOOT,
		EBS_DROPBOMB,
		EBS_PLACEBOMB,
		EBS_SUICIDE,
		EBS_GO_TO_CPOINT,
		EBS_KNOCKDOWN,
		EBS_RAGE,
		EBS_WAIT_FATALITY,
		EBS_MOVE_AWAY,
		EBS_PRAY,
		EBS_GO_TO_PRAY,
		EBS_FATALITY,
		EBS_FOLLOWHERO,
		EBS_WAITPLAYER,
		EBS_RETURNTOSPAWN,
		EBS_DRINK
	};		

	EnemyBrainState BrainState;


	ConstString WayPointsName;
	ConstString Target;

	int        NumAtackers;
	Character* pChrTarget;

	Character* pChrPlayer;

	int   iNumPlayerAttckers;
	int   iNumNPCAttckers;

	float fDistofView;
	float fDistofViewPlayer;

	float fDistToLoseTarget;

	float fTimeToNextAttack2;
	float fCurTimeToNextAttack;

	int   iMaxSimelouslyAttackers;

	float fDistToReconstructPath;

	bool  bIsStatist;

	int   iNumHits;
	float fHitTime;		
	
	float fTime_to_return_to_spawn;		

	enum action_type
	{
		act_ai = 0,
		act_waiting = 1,
		act_alarming = 2,
		act_patrolling = 3
	};

	action_type act_type;
	float time_to_alarm;

	int   patrol_point;
	float patrol_time_action;

	float block_cooldown;
	float block_timecooldown;

public:	
	
	struct TFinder
	{		
		Character* chr;		
	};

	struct Stage
	{
		float hp;
		const char* param;
	};

	array<Stage> stages;
	int cur_stage;

	virtual const ConstString & GetWayPointsName() { return WayPointsName; };	
	virtual void SetWayPointsName(const ConstString & _WayPointsName) { WayPointsName = _WayPointsName; };

	virtual const ConstString & GetAIZoneName() { return AIZoneName; };
	virtual void SetAIZoneName(const ConstString & _AIZoneName) { AIZoneName = _AIZoneName; };

	virtual const ConstString & GetTarget() { return Target; };
	virtual void SetTarget(const ConstString & _Target) { Target = _Target; };

	virtual int  GetNumAtackers() { return NumAtackers; };
	virtual void SetNumAtackers(int _NumAtackers);

	virtual float GetTimeToNextAttack() { return fTimeToNextAttack2; }
	virtual float GetStrafeModifier();
	virtual float GetIdleAfterAttackModifier();

	virtual Character* GetChrTarget() { return pChrTarget; };
	virtual void SetChrTarget(Character* _pChrTarget) { pChrTarget = _pChrTarget; };

	int  iCurPt_CPoint;
	int  iCurGr_CPoint;
	bool bInCombatPoint;

	array<CombatPoints*> pCPoints;
	array<SaftyPoints*> pSPoints;

	ConstString AIZoneName;
	AIZones* AIZone;	

	bool NeedToFindMO;

	array<TFinder> searchResult;

	void SectorSearch (const Vector& vPos, float fAy, float fNearPlane, float fFarPlane, float min_ang, float max_ang, GroupId group);

	void SectorSearchDraw (const Vector& vPos, float fAy, float fNearPlane, float fFarPlane, float min_ang, float max_ang);

	void CircleSearch (const Vector& vPos, const float fRadius, GroupId group);

	virtual bool StartBlocking(Character * offender, DamageReceiver::DamageSource source);

public:	
	
	aiThoughtPursuit*  pThoughtPursuit;
	aiThoughtStanding* pThoughtStanding;
	aiThoughtStrafing* pThoughtStrafing;
	aiThoughtKick*	   pThoughtKick;
	aiThoughtBlocking* pThoughtBlocking;

	aiBrain(Character & ch, const char* szName);
	virtual ~aiBrain();


	virtual const ConstString & GetAIParamsName(){ RETURN_CONSTSTRING("aiParams"); }	

	//установка параметров
	virtual void SetParams(CharacterControllerParams * params);	

	virtual void Init();

	//Перезапустить контроллер при перерождении персонажа
	virtual void Reset();

	//Обновить состояние контролера на каждом кадре
	virtual void Update(float dltTime);
	
	//Обновить состояние контролера 10 раз в секунду (распределено между персонажами)
	
	
	virtual bool PreTick();
	virtual bool BaseTick();
	virtual void TickWhenEmpty();
	virtual void GuardTick();
	virtual void Tick();


	//Получить тело, которым управляем...
	virtual Character* GetBody ();

	//Мыслить ;)
	virtual void ExecuteThoughts (float fDeltaTime);	

	virtual aiThought* AddThought (aiThought* Task);	
	
	virtual void ActivateThought(aiThought* thought);	

	virtual bool IsEmpty ();
	virtual dword GetThoughtsCount ();	

	virtual bool AllowRestartAnim();
	virtual void PlayerAchtung();

	//Персонаж получил люлей
	virtual void Hit(float dmg, Character* _offender, const char* reaction, DamageReceiver::DamageSource source);	

	virtual void Shoot(float dmg);

	//Персонаж умирает
	virtual void Death(){ DeattachFromCP(); Reset(); };

	virtual void ActivatePairLink(char* action);	

    bool bAllowUpdateInPairModel;

	enum CPAction
	{
		CPA_NOTHING = 0,
		CPA_PURSUIT = 1,				
		CPA_STANDING = 2
	};

	virtual void UpdateMO();	

	virtual CPAction FindCPoint();
	virtual CPAction IsCPReached();
	virtual void AttachToCP(int group,int index);
	virtual void DeattachFromCP();	
	virtual bool InAIZone(const Vector & pos, bool check_buffer = false);
	virtual bool InSafetyPoint(Vector pos);

	virtual bool CheckDist(Character* target,float fRadius);
	virtual bool ValidateTargetRules(Character* target,float fRadius);	

	virtual bool AttackCharacter();
	virtual bool GoToFrustum(float dist);
	virtual bool CanRunaway(Vector& vDestination, float dist_to_run);
	
	virtual bool FilterFindedTarget(Character* pTarget, bool statistSearch);
	virtual Character * GetPlayerTarget();
	virtual Character * SearchTarget(bool bCheckMaxAllowedAttackers, float serachDist = -1.0f, bool statistSearch = false);
	virtual bool SearchPlayer(bool bCheckMaxAllowedAttackers, float serachDist = -1.0f);	

	virtual void CheckTargetState(bool doRestart = true);
	virtual bool CheckCharState(Character* target);
	virtual void CheckTargetPairState();
	virtual void ActivateTargetPairStateThought() {};

	virtual const ConstString & GetControllerState();
	virtual void DrawBrainState();
	virtual void ShowDebugInfo();

	virtual bool AllowRecreatePath(Character* pTarget);
	virtual bool AllowAttack();

	virtual bool AllowBlocking();	

	virtual bool CheckAttackerHeight();

	virtual float GetPowDistToTarget();
	virtual float GetDistToTarget();
	virtual void  OrinentToTarget();

	virtual void  ChangeStage(int stage, const char* param) {};
	int GetCurStage();

	CombatPoints::CombatPoint* GetCombatPoint(int group,int index);
	CombatPoints::CombatPoint* GetCurCombatPoint();

	int* GetCPtNumAlly(int group,int index,bool enemy);
	int* GetCurCPtNumAlly(bool enemy);

	int* GetCPtNumEnemy(int group,int index,bool enemy);
	int* GetCurCPtNumEnemy(bool enemy);

	bool IsCPocpStatist(int group,int index);

	bool RayCast(Vector from,Vector to,dword mask = phys_mask(phys_world));

	void ActivateObject(const ConstString & name,bool activate);	

	bool stopWhenDebug;
	virtual bool StopWhenDebug() { return stopWhenDebug; };
	virtual void SetStopWhenDebug(bool enable) { stopWhenDebug = enable; };

	virtual void MoveAway(Vector pos);

	virtual void Command(dword numParams, const char ** params);

	float GetAttackDistance();
	float GetAttackDistancePow();
	
    virtual float PursuitDist();
};

__forceinline bool aiBrain::RayCast(Vector from,Vector to,dword mask)
{
	if (GetBody()->Physics().Raycast(from, to, mask))
	{
		return true;
	}

	if (GetBody()->Physics().Raycast(to, from, mask))	
	{
		return true;
	}

	return false;
}

__forceinline bool aiBrain::CheckAttackerHeight()
{
	if (!pChrTarget) return false;

	Vector vPos = GetBody()->physics->GetPos();

	float dy=fabs(vPos.y - pChrTarget->physics->GetPos().y);

	if (dy>0.5f) return false;

	return true;
}

__forceinline float aiBrain::GetPowDistToTarget()
{
	if (!pChrTarget) return 0;

	return chr.arbiter->GetPowDistance(GetBody(),pChrTarget);
}

__forceinline float aiBrain::GetDistToTarget()
{
	if (!pChrTarget) return -1.0f;

	return (GetBody()->physics->GetPos()-pChrTarget->physics->GetPos()).GetLengthXZ();
}

__forceinline void aiBrain::OrinentToTarget()
{
	if (!pChrTarget) return;

	if (GetBody()->logic->AllowOrient())
	{		
		GetBody()->physics->Orient(pChrTarget->physics->GetPos());
	}
}

__forceinline float aiBrain::GetAttackDistance()
{
	return pThoughtKick->GetAttackDistance();
}

__forceinline float aiBrain::GetAttackDistancePow()
{
	return GetAttackDistance() * GetAttackDistance();
}

__forceinline int aiBrain::GetCurStage()
{
	return cur_stage;
}


#endif