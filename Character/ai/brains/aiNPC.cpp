#include "aiNPC.h"
#include "aiBrain.h"
#include "../CharacterAI.h"
#include "../../character/components/CharacterLogic.h"
#include "../../character/components/CharacterPhysics.h"

class NPCParams : public CharacterControllerParams
{

public:

	bool  isPanic;
	ConstString WayPointsName;
	ConstString FearPointsName;
	ConstString TalkPointsName;

	
	NPCParams()
	{
	}

	virtual ~NPCParams()
	{
	}

	virtual bool IsControllerSupport(const ConstString & controllerClassName)
	{
		CHECK_CONTROLLERSUPPORT("NPCParams")
	}

	bool Create(MOPReader & reader)
	{
		Init(reader);
		return true;
	}

	bool EditMode_Create(MOPReader & reader)
	{
		Init(reader);
		return true;
	}

	bool EditMode_Update(MOPReader & reader)
	{
		Init(reader);
		return true;
	}


	void Init (MOPReader & reader)
	{			
		isPanic = reader.Bool();
		WayPointsName=reader.String();
		FearPointsName=reader.String();
		TalkPointsName=reader.String();
	}
};

MOP_BEGINLISTCG(NPCParams, "AI params NPC", '1.00', 90, "NPC Params", "Character")
	
	MOP_BOOL("Is panic", false)
	MOP_STRING("WayPoints Name", "WayPoints")
	MOP_STRING("FearPoints Name", "Fear Points")
	MOP_STRING("TalkPoints Name", "Talk Points")

MOP_ENDLIST(NPCParams)

aiNPC::aiNPC(Character & ch, const char* szName) : aiBrain (ch, szName)
{
	fDistofView = 7.0f;
	fDistofViewPlayer = 10.0f;

	iNumPlayerAttckers = 5;
	iNumNPCAttckers = 1;

	BrainState=EBS_IDLE;

	pChrTarget=NULL;

	bInCombatPoint = false;

	iCurPt_CPoint = -1;
	iCurGr_CPoint = -1;	

	static const ConstString defWayPoints("WayPoints"); 
	WayPointsName=defWayPoints;

	AIZone = NULL;

	fDistToLoseTarget = 15.0f;

	fTimeToNextAttack2 = 5.0f;
	fCurTimeToNextAttack = 0.0f;

	iMaxSimelouslyAttackers = 1;

	bIsStatist = false;
		
	pThoughtStandingFear = (aiThoughtStanding*)AddThought(NEW aiThoughtStanding(this));
	pThoughtStandingTalk = (aiThoughtStanding*)AddThought(NEW aiThoughtStanding(this));
		
	pThoughtStanding->AddAction("Happy");	
	pThoughtStanding->PrepareActions();
	
	pThoughtStandingFear->AddAction("Fear");	
	pThoughtStandingFear->PrepareActions();

	pThoughtStandingTalk->AddAction("Talk");	
	pThoughtStandingTalk->PrepareActions();

	fPanicCooldown = -1.0f;

	pFearPoints = null;
	pTalkPoints = null;

	fear_pt_index = -1;

	isPanicOnStart = false;

	bPointTaken = false;
}

void aiNPC::UpdateMO()
{
	if (NeedToFindMO)
	{
		MOSafePointerTypeEx<NPCPoints> points;				
		static const ConstString strTypeId("FearPoints");
		points.FindObject(&GetBody()->Mission(),FearPointsName,strTypeId);
		pFearPoints = points.Ptr();

		points.FindObject(&GetBody()->Mission(),TalkPointsName,strTypeId);
		pTalkPoints = points.Ptr();
	}

	aiBrain::UpdateMO();
}

void aiNPC::Reset()
{
	TakePoint(false);

	aiBrain::Reset();
}

void aiNPC::GotoFearPoint(int index)
{
	if (fPanicCooldown>=0)
	{
		if (index >= 0)
		{		
			TakePoint(false);

			fear_pt_index = index;
			pThoughtPursuit->Activate( pFearPoints->Points[index].position, WayPointsName, 0.5f, true);
			BrainState=EBS_GO_TO_CPOINT;

			TakePoint(true);
		}
	}
	else
	{
		if (index >= 0)
		{		
			TakePoint(false);

			fear_pt_index = index;
			pThoughtPursuit->Activate( pTalkPoints->Points[index].position, WayPointsName, 0.5f, true);
			//BrainState=EBS_GO_TO_PRAY;
			BrainState=EBS_PURSUIT;

			TakePoint(true);
		}
	}
}

void aiNPC::TakePoint(bool take)
{	
	if (bPointTaken == take) return;

	if (fPanicCooldown<=0)
	{
		if (fear_pt_index >= 0 && pTalkPoints)
		{
			pTalkPoints->TakePoint(fear_pt_index,take,GetBody());
			bPointTaken = take;
		}
	}
	else
	{
		if (fear_pt_index >= 0 && pFearPoints)
		{
			pFearPoints->TakePoint(fear_pt_index,take,GetBody());
			bPointTaken = take;
		}
	}
}

void aiNPC::SetParams(CharacterControllerParams * params)
{
	if (!params) return;

	isPanicOnStart = ((NPCParams*)params)->isPanic;	

	WayPointsName=((NPCParams*)params)->WayPointsName;
	FearPointsName=((NPCParams*)params)->FearPointsName;
	TalkPointsName=((NPCParams*)params)->TalkPointsName;	
}

void aiNPC::Update(float dltTime)
{
	aiBrain::Update(dltTime);

	if (chr.logic->GetHP()<0.1f) return;

	if (fPanicCooldown>0)
	{
		//fPanicCooldown -= dltTime;

		if (fPanicCooldown<0.0f)
		{
			fPanicCooldown = -1.0f;
			GetBody()->animation->Goto("Idle",0.1f);
			TakePoint(false);
		}
	}
}

void aiNPC::Tick()
{	
	if (chr.logic->GetState() == CharacterLogic::state_idle)
	{
		if (string::IsEqual(chr.animation->CurrentNode(),"Idle") && fPanicCooldown > 0.0f && chr.logic->GetHP()>0.0f)
		{
			chr.animation->Goto("Idle2",0.0f);		
		}
	}
	
	if (isPanicOnStart)
	{
		StartPanic();
		isPanicOnStart = false;
	}

	if (IsEmpty())
	{	
		if (BrainState == EBS_PURSUIT)
		{
			BrainState = EBS_STANDING;
		}

		if (BrainState == EBS_GO_TO_CPOINT)
		{
			pThoughtStandingFear->Activate( Rnd(10)+5, pChrTarget );
			BrainState=EBS_STANDING;
		}
		else
		if (fPanicCooldown<=0 && BrainState == EBS_STANDING && fear_pt_index != -1)
		{
			if (pTalkPoints->Points[fear_pt_index].num_chars == 2)
			{
				pThoughtStandingFear->Activate( Rnd(10)+5, pChrTarget );
				BrainState=EBS_STANDING;
			}
			else
			{
				BrainState=EBS_IDLE;
			}			
		}		
		else
		{
			BrainState=EBS_IDLE;
		}
	}

	if (fPanicCooldown<=0)
	{
		if (BrainState==EBS_PRAY)
		{
			MissionObject* frnd = pTalkPoints->GetFrined(fear_pt_index,GetBody());

			if (frnd)
			{
				if (!CheckDist((Character*)frnd,1.5f))
				{
					Reset();
				}
			}
			else
			{
				Reset();
			}
		}
		else
		if (BrainState == EBS_STANDING)
		{
			StartTalk();			
		}

		if (BrainState == EBS_GO_TO_PRAY)
		{
			if (!CheckDist(pChrPlayer,6.0f))
			{
				Reset();
			}
		}
		else
		{
			if (CheckDist(pChrPlayer,4.5f))
			{
				pThoughtStanding->Activate( Rnd(10)+5, pChrPlayer );
				BrainState=EBS_GO_TO_PRAY;
			}
		}		
	}

	if (BrainState==EBS_IDLE)
	{
		if (pFearPoints && fPanicCooldown>0)
		{				
			int index = pFearPoints->GivePoint(0);
			
			if ((fear_pt_index == index && pFearPoints->IsPointReached(fear_pt_index,chr.physics->GetPos()))|| index == -1)
			{
				pThoughtStandingFear->Activate( Rnd(10)+5, pChrTarget );
				BrainState=EBS_STANDING;
			}
			else
			{
				GotoFearPoint(index);
			}
		}
		else
		{
			if (pTalkPoints && fPanicCooldown<=0)
			{
				int index = -1;//pTalkPoints->GivePoint(0);

				if (Rnd()>0.5)
				{
					index = pTalkPoints->GivePoint(1);
				}

				if (index == -1)
				{
					index = pTalkPoints->GivePoint(0);
				}

				if (fear_pt_index == index || index == -1)
				{
					pThoughtStanding->Activate( Rnd(10)+5, pChrTarget );
					BrainState=EBS_STANDING;
				}
				else
				{
					GotoFearPoint(index);
				}
			}
			/*else
			{
				float r = 0;
				Vector pos;

				while (r<7.0f)
				{				
					pos.Rand(0.0f,1.0f);
					pos.y = 0;
					pos *= RRnd(-15,15);

					r = (GetBody()->logic->GetSpawnPoint() + pos - GetBody()->physics->GetPos()).GetLength();
				}

				pThoughtPursuit->Activate( GetBody()->logic->GetSpawnPoint() + pos, WayPointsName, 0.5f);
				BrainState=EBS_PURSUIT;
			}*/
		}
	}	
}

void aiNPC::StartPanic()
{	
	if (chr.logic->GetState() == CharacterLogic::state_shoot) return;

	if (chr.logic->GetHP()<0.1f) return;

	GetBody()->animation->Goto("Idle2",0.1f);

	if (fPanicCooldown<0)
	{
		Reset();
		fear_pt_index = -1;
	}

	fPanicCooldown = 15.0f;	

	if (!pFearPoints) return;

	if (BrainState != EBS_GO_TO_CPOINT)	GotoFearPoint(pFearPoints->GivePoint(0));
}

bool aiNPC::StartTalk()
{
	if (!pTalkPoints || BrainState==EBS_PRAY) return false;

	MissionObject* frnd = pTalkPoints->GetFrined(fear_pt_index,GetBody());
	
	if (frnd)
	{
		if (CheckDist((Character*)frnd,1.5f))
		{			
			pThoughtStandingTalk->Activate( Rnd(10)+5, (Character*)frnd );
			BrainState=EBS_PRAY;

			return true;
		}
	}

	return false;
}

void aiNPC::NotifyAboutAttack(Character* attacker,float damage)
{
	if (BrainState != EBS_GO_TO_CPOINT)	StartPanic();
}

void aiNPC::Hit(float dmg,Character* _offender, const char* reaction, DamageReceiver::DamageSource source)
{		
	aiBrain::Hit(dmg,_offender,reaction,source);

	//StartPanic();
}

void aiNPC::Death()
{
	TakePoint(false);

	aiBrain::Death();
}

DeclareCharacterController(aiNPC, "NPC", 11)