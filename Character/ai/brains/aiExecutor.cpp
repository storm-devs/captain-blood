#include "aiExecutor.h"
#include "aiBrain.h"
#include "../CharacterAI.h"
#include "../../character/components/CharacterLogic.h"
#include "../../character/components/CharacterPhysics.h"
#include "../../character/components/CharacterAnimation.h"
#include "../../character/components/ButtonMG.h"


class ExecutorParams : public CharacterControllerParams
{

public:

	ConstString WayPointsName;
	Vector pray_point;
	
	ConstString cHelpersTriger;

	int    iNumHelpers;
	ConstString cHelpers[20];	

	float fHealingRate;

	ConstString cFt1Triger;
	ConstString cFt2Triger;

	struct TPhase
	{
		float when_pray;
		float pray_until;
	};

	int    iNumPhases;
	TPhase phases[5];

	virtual bool IsControllerSupport(const ConstString & controllerClassName)
	{
		CHECK_CONTROLLERSUPPORT("ExecutorParams")
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
		WayPointsName = reader.String();
		pray_point = reader.Position();	

		fHealingRate = reader.Float();

		cHelpersTriger = reader.String();		

		iNumHelpers = reader.Array();

		for (int i=0;i<iNumHelpers;i++)
		{
			cHelpers[i] = reader.String();			
		}				

		iNumPhases = reader.Array();

		for (int i=0;i<iNumPhases;i++)
		{
			phases[i].when_pray = reader.Float();
			phases[i].pray_until = reader.Float();
		}

		cFt1Triger = reader.String(); 
		cFt2Triger = reader.String(); 
	}
};

MOP_BEGINLISTCG(ExecutorParams, "AI params Executor", '1.00', 90, "Executor params", "Character")

	MOP_STRINGC("Waypoint", "WayPoints", "Name of WayPoint System")
	MOP_POSITION("Pray point",0.0f)	

	MOP_FLOATEX("Healing Rate", 1.0f, 1.0f, 256.0f)

	MOP_STRINGC("Helpers Triger", "", "Name of triger that spawn Helpers")	

	MOP_ARRAYBEGC("Helpers", 0, 20, "Helpers of Boss")		
		
		MOP_STRINGC("Helper", "", "Name of Helper Character")

	MOP_ARRAYEND

	MOP_ARRAYBEGC("Phases", 0, 5, "Pray Phases")		
	
		MOP_FLOATEX("When Pray", 0.25f, 0.05f, 0.45f)
		MOP_FLOATEX("Pray Until", 0.25f, 0.05f, 0.45f)

	MOP_ARRAYEND	

	MOP_STRINGC("Ft1 Triger", "", "Name of Triget that activate first boss fatality")
	MOP_STRINGC("Ft2 Triger", "", "Name of Triget that activate second boss fatality")

MOP_ENDLIST(ExecutorParams)


aiExecutor::aiExecutor(Character & ch, const char* szName) : aiBrain (ch, szName)
{
	fDistofView = 7.0f;
	fDistofViewPlayer = 10.0f;

	iNumPlayerAttckers = 5;
	iNumNPCAttckers = 1;

	BrainState=EBS_IDLE;

	pChrTarget=NULL;

	count = 0;

	static const ConstString defWayPoints("WayPoints");
	WayPointsName=defWayPoints;
	
	pExecutorPray =     (aiExecutorPray*)AddThought(NEW aiExecutorPray(this));

	pThoughtKick->AddAction("Attack1");	
	pThoughtKick->AddAction("Attack2");	
	pThoughtKick->AddAction("Attack3");	
	pThoughtKick->PrepareActions();

	fHealingRate = 0.0f;

	iNumPhases = 1;
	phases[0].when_pray = 0.15f;
	phases[0].pray_until = 0.45f;

	iNumHelpers = 0;

}

aiExecutor::~aiExecutor()
{
}

void aiExecutor::SetParams(CharacterControllerParams * params)
{
	if (!params) return;
	
	WayPointsName=((ExecutorParams*)params)->WayPointsName;

	pray_pos=((ExecutorParams*)params)->pray_point;
	
	fHealingRate=((ExecutorParams*)params)->fHealingRate;

	cHelpersTriger = ((ExecutorParams*)params)->cHelpersTriger;

	iNumHelpers = ((ExecutorParams*)params)->iNumHelpers;
	
	for (int i=0;i<iNumHelpers;i++)
	{
		cHelpers[i] = ((ExecutorParams*)params)->cHelpers[i];
	}		


	iNumPhases = ((ExecutorParams*)params)->iNumPhases;

	
	for (int i=0;i<iNumPhases;i++)
	{
		phases[i].when_pray = ((ExecutorParams*)params)->phases[i].when_pray;
		phases[i].pray_until = ((ExecutorParams*)params)->phases[i].pray_until;
	}

	iCurPhase = 0;

	cFt1Triger = ((ExecutorParams*)params)->cFt1Triger;
	cFt2Triger = ((ExecutorParams*)params)->cFt2Triger;
}

//Перезапустить контроллер при перерождении персонажа
void aiExecutor::Reset()
{
	BrainState=EBS_IDLE;

	aiBrain::Reset();		
}

//Обновить состояние контролера на каждом кадре
void aiExecutor::Update(float dltTime)
{	
	if (chr.logic->GetState() == CharacterLogic::state_pair)
	{
		Reset();
	}

	if (GetBody()->logic->GetHP()<0.1f) return;
	
	if (chr.logic->GetState() == CharacterLogic::state_pair || GetBody()->logic->GetHP()<GetBody()->logic->GetMaxHP()*0.5f)
	{
		GetBody()->animation->SetPlaySpeed(1.0f);
		GetBody()->SetAddColor(Color(0,0,0,0));		

		//chr.logic->SetMinHPLimitter(0.04f);
	}
	else
	{		
		GetBody()->animation->SetPlaySpeed(1.0f + (1.0f-GetBody()->logic->GetHP()/GetBody()->logic->GetMaxHP())*1.25f);	
		GetBody()->SetAddColor(Color(0.4f * (1.0f-GetBody()->logic->GetHP()/GetBody()->logic->GetMaxHP()),0,0,0));	

		//chr.logic->SetMinHPLimitter(0.5f);
	}

	if (chr.logic->GetState() == CharacterLogic::state_pair)
	{
		float hp = GetBody()->logic->GetHP();
		float max_hp = GetBody()->logic->GetMaxHP();
		const char * hackMGid = "executioner blood pair";
		if (hp >= max_hp * 0.55f && string::IsEqual(chr.logic->last_pair_link, hackMGid))
		{
		}
		else
		{
			count++;

			if (count>2)
			{
				if (hp >= max_hp * 0.5f)
				{
					ActivateObject(cFt1Triger,true);
				}
				else
				{
					ActivateObject(cFt2Triger,true);
				}

				Reset();

				count = 0;

				return;
			}
		}
	}
	else
	{
		count = 0;
	}


	aiBrain::Update(dltTime);
}

//Обновить состояние контролера 10 раз в секунду (распределено между персонажами)
void aiExecutor::Tick()
{
	if (chr.logic->GetState() == CharacterLogic::state_pair) return;

	if (pChrTarget!=NULL)
	{		
		if (!pChrTarget->IsShow())
		{			
			Reset();

			ExecuteThoughts(0);

			BrainState=EBS_IDLE;

			pChrTarget=NULL;
		}
		else
		if (!pChrTarget->IsActive())
		{
			Reset();

			ExecuteThoughts(0);

			BrainState=EBS_IDLE;

			pChrTarget=NULL;
		}
		else
		if (pChrTarget->logic->IsDead() && IsEmpty())
		{
			Reset();

			ExecuteThoughts(0);

			BrainState=EBS_IDLE;

			pChrTarget=NULL;
		}
		else
		if (pChrTarget->logic->IsActor())
		{
			Reset();

			ExecuteThoughts(0);

			BrainState=EBS_IDLE;

			pChrTarget=NULL;
		}
		/*else
		if (pChrTarget->logic->IsPairMode())
		{
			Reset();

			ExecuteThoughts(0);

			BrainState=EBS_IDLE;

			pChrTarget=NULL;
		}*/
	}
	
	if (IsEmpty())
	{		
		CharacterLogic::State st = GetBody()->logic->GetState();
		if (st == CharacterLogic::state_pair) return;

		if (BrainState==EBS_GO_TO_PRAY)
		{
			if ((GetBody()->physics->GetPos()-pray_pos).GetLength()>1.25f)
			{				
				pChrTarget = NULL;
				pThoughtPursuit->Activate(pray_pos, WayPointsName, 0.5f,false);
				BrainState=EBS_GO_TO_PRAY;

				return;
			}
			
			{
				//GetBody()->physics->SetMovePosition(pray_pos);
				GetBody()->physics->SetPos(pray_pos);

				ActivateObject(cHelpersTriger,true);
				
				pExecutorPray->Activate(pChrTarget,(iCurPhase==iNumPhases-1),fHealingRate,phases[iCurPhase].pray_until,iNumHelpers,cHelpers);
				BrainState=EBS_PRAY;				

				if (iCurPhase<iNumPhases-1) iCurPhase++;
			}

			return;
		}

		if (BrainState==EBS_KICK)
		{
			if (pChrTarget!=NULL)
			{				
				{
					float fDist=(GetBody()->physics->GetPos()-pChrTarget->physics->GetPos()).GetLengthXZ();

					if ((GetBody()->physics->GetPos()-pChrTarget->physics->GetPos()).GetLengthXZ()<=GetAttackDistance())
					{
						Vector vPos = GetBody()->physics->GetPos();
						
						float dy=fabs(vPos.y - pChrTarget->physics->GetPos().y);

						if (dy>0.5f)
						{
							pChrTarget=NULL;
							
							ExecuteThoughts(0);

							BrainState=EBS_IDLE;

							Reset();
						}
						else
						{				
							pThoughtKick->Activate(pChrTarget);							
							BrainState=EBS_KICK;
						}
					}
					else
					{
						BrainState=EBS_IDLE;
					}
				}
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
	
	if (BrainState==EBS_IDLE)
	{
		if (pChrTarget==NULL) pChrTarget=GetPlayerTarget();//SearchTarget(true);
		
		if (pChrTarget!=NULL)
		{
			//Vano: добавил проверку чтобы босс не дергался
			if (pChrTarget->logic->IsActor() || pChrTarget->logic->IsPairMode())
				pChrTarget = null;
			else
				AttackCharacter();		
		}					
	}
	else
	if (BrainState==EBS_PURSUIT&&IsEmpty())
	{	
		AttackCharacter();		
	}
	else
	if (BrainState==EBS_PURSUIT)
	{
		if (!IsEmpty()&&AllowRecreatePath(pChrTarget))
		{
			pThoughtPursuit->ChangeParams(pChrTarget->physics->GetPos(), PursuitDist() );			
		}
	}
}

bool aiExecutor::AttackCharacter()
{
	float fDist=(GetBody()->physics->GetPos()-pChrTarget->physics->GetPos()).GetLengthXZ();

	if (fDist<=GetAttackDistance())
	{			
		pThoughtKick->Activate(pChrTarget);
		BrainState=EBS_KICK;		
	}
	else			
	{
		pThoughtPursuit->Activate(pChrTarget->physics->GetPos(), WayPointsName, PursuitDist(), true);		
		BrainState=EBS_PURSUIT;	
	}

	return true;
}

void aiExecutor::Hit(float dmg,Character* _offender, const char* reaction, DamageReceiver::DamageSource source)
{
	if (BrainState == EBS_GO_TO_PRAY) return;	
	
	if (GetBody()->logic->GetHP()<=GetBody()->logic->GetMaxHP() * phases[iCurPhase].when_pray)
	{
		Reset();		
		
		GetBody()->animation->Goto("second fake idle",0.2f);

		pThoughtPursuit->Activate(pray_pos, WayPointsName, 0.2f, false);
		BrainState=EBS_GO_TO_PRAY;

		return;
	}

	if (BrainState!=EBS_KICK)
	{		
		Reset();
			
		ExecuteThoughts(0);

		BrainState=EBS_IDLE;		

		pChrTarget=GetPlayerTarget();//SearchTarget(false);

		if (pChrTarget!=NULL)
		{
			float fDist=(GetBody()->physics->GetPos()-pChrTarget->physics->GetPos()).GetLengthXZ();

			if ((GetBody()->physics->GetPos()-pChrTarget->physics->GetPos()).GetLengthXZ()<=GetAttackDistance())
			{
				pThoughtKick->Activate(pChrTarget);
				BrainState=EBS_KICK;
			}
		}
	}
}

void aiExecutor::Command(dword numParams, const char ** params)
{	
	if (string::IsEqual(params[0],"goto2Idle"))
	{
		GetBody()->animation->Goto("second idle",0.0f);		
	}
	else
	if (string::IsEqual(params[0],"goto2Die"))
	{
		GetBody()->animation->Goto("ft Die",0.0f);		
	}
	else
	{
		aiBrain::Command(numParams, params);
	}
}

DeclareCharacterController(aiExecutor, "Executor", 11)