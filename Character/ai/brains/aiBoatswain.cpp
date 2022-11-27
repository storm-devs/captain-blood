#include "aiBoatswain.h"
#include "aiBrain.h"
#include "..\CharacterAI.h"
#include "../../character/components/CharacterLogic.h"
#include "../../character/components/CharacterItems.h"
#include "../../character/components/CharacterPhysics.h"

class aiBoatswainParams : public CharacterControllerParams
{

public:	

	ConstString WayPointsName;
	ConstString AIZoneName;
	float hp_sec_stage;
	ConstString triger_sec_stage;

	float fTimeToNextAttack;
	int iMaxSimelouslyAttackers;
	int iNumPlayerAttckers;

	aiBoatswainParams();
	~aiBoatswainParams();

	virtual bool IsControllerSupport(const ConstString & controllerClassName);

	bool Create(MOPReader & reader);

	bool EditMode_Create(MOPReader & reader);

	bool EditMode_Update(MOPReader & reader);

	void Init (MOPReader & reader);	
};

aiBoatswainParams::aiBoatswainParams()
{
	hp_sec_stage = 40.0f;
	fTimeToNextAttack = 5.0f;
	iNumPlayerAttckers = 20;
	iMaxSimelouslyAttackers = 1;
}

aiBoatswainParams::~aiBoatswainParams()
{
}

bool aiBoatswainParams::IsControllerSupport(const ConstString & controllerClassName)
{
	CHECK_CONTROLLERSUPPORT("aiBoatswainParams")
}

bool aiBoatswainParams::Create(MOPReader & reader)
{
	Init(reader);
	return true;
}

bool aiBoatswainParams::EditMode_Create(MOPReader & reader)
{
	Init(reader);
	return true;
}

bool aiBoatswainParams::EditMode_Update(MOPReader & reader)
{
	Init(reader);
	return true;
}

void aiBoatswainParams::Init(MOPReader & reader)
{
	WayPointsName = reader.String();
	AIZoneName = reader.String();
	hp_sec_stage = reader.Float();
	triger_sec_stage = reader.String();
	fTimeToNextAttack = reader.Float();
	iMaxSimelouslyAttackers = reader.Long();
	iNumPlayerAttckers = reader.Long();
}

MOP_BEGINLISTCG(aiBoatswainParams, "AI params Boatswain", '1.00', 90, "AI params Boatswain", "Character")	
MOP_STRING("WayPoints Name", "WayPoints")
MOP_STRING("AIZone", "")
MOP_FLOATEX("HP of second stage", 40,0,100)
MOP_STRINGC("Triger when begin 2 stage", "", "Triger when begin 2 stage")
MOP_FLOATEX("Time To Next Attack", 5.0f, 0.01f, 1000.0f)
MOP_LONGEX("Max Simelously Attackers", 1, 1, 20)
MOP_LONGEX("MAX Player Attckers", 20, 1, 20)
MOP_ENDLIST(aiBoatswainParams)



aiBoatswain::aiBoatswain(Character & ch, const char* szName) : aiBrain (ch, szName)
{
	fDistToFarKick = 4.0f;
	fDistToMidleKick = 3.10f;
	fDistToKick = 2.15f;
	
	fDistofView = 12.0f;
	fDistofViewPlayer = 12.0f;

	iNumPlayerAttckers = 20;
	iNumNPCAttckers = 1;

	BrainState=EBS_IDLE;

	pChrTarget=NULL;	

	bInCombatPoint = false;

	iCurPt_CPoint = -1;
	iCurGr_CPoint = -1;	

	fDamageCharger = 0;

	sec_stage = false;
	hp_sec_stage = 0.4f;

	static const ConstString defWayPoints("WayPoints");
	WayPointsName=defWayPoints;	

	pThoughtKickFar =    (aiThoughtKick*)AddThought(NEW aiThoughtKick(this));
	pThoughtKickMidle =  (aiThoughtKick*)AddThought(NEW aiThoughtKick(this));
	pThoughtKickNear =   (aiThoughtKick*)AddThought(NEW aiThoughtKick(this));
	//pBoatswainWirst =    (aiBoatswainWirst*)AddThought(NEW aiBoatswainWirst(this));		
	pThoughtBlocking =   (aiThoughtBlocking*)AddThought(NEW aiThoughtBlocking(this));
	
	pThoughtKickFar->AddAction("Attack1");
	pThoughtKickFar->AddAction("Attack5");
	pThoughtKickFar->PrepareActions();
	
	pThoughtKickMidle->AddAction("Attack4");	
	pThoughtKickMidle->PrepareActions();

	pThoughtKickNear->AddAction("Attack2");
	pThoughtKickNear->AddAction("Attack3");
	pThoughtKickNear->PrepareActions();	
	
	pThoughtKickNear->SetFarAttack("Attack1");

	blocking = false;
}

aiBoatswain::~aiBoatswain()
{
}

void aiBoatswain::SetParams(CharacterControllerParams * params)
{
	if (!params) return;

	WayPointsName=((aiBoatswainParams*)params)->WayPointsName;
	AIZoneName=((aiBoatswainParams*)params)->AIZoneName;	
	hp_sec_stage=((aiBoatswainParams*)params)->hp_sec_stage * 0.01f;
	triger_sec_stage=((aiBoatswainParams*)params)->triger_sec_stage;

	fTimeToNextAttack2 = ((aiBoatswainParams*)params)->fTimeToNextAttack;
	iMaxSimelouslyAttackers = ((aiBoatswainParams*)params)->iMaxSimelouslyAttackers;
	iNumPlayerAttckers = ((aiBoatswainParams*)params)->iNumPlayerAttckers;

	chr.items->SetAlwaysArmor(false);

	blocking = false;
}

//Обновить состояние контролера 10 раз в секунду (распределено между персонажами)
void aiBoatswain::Tick()
{	
	if (!BaseTick()) return;

	if (chr.logic->GetHP()/chr.logic->GetMaxHP()>hp_sec_stage)
	{
		sec_stage = false;
	}

	if (!sec_stage)
	{
		if (chr.logic->GetHP()/chr.logic->GetMaxHP()<hp_sec_stage)
		{
			ActivateObject(triger_sec_stage,true);
			sec_stage = true;
			//chr.items->SetAlwaysArmor(true);
		}
	}	
	
	CheckTargetState(false);
	CheckTargetPairState();

	if (BrainState==EBS_WAIT_FATALITY) return;

	if (IsEmpty())
	{		
		if (fDamageCharger>90 && pChrTarget!=NULL)
		{
			fDamageCharger = 0.0f;

			pThoughtKickNear->SetCurAction("Attack3");			
			BrainState=EBS_RAGE;			
		}
		else
		if (BrainState==EBS_KICK)
		{
			if (pChrTarget!=NULL)
			{
				/*if (pChrTarget==pChrPlayer)
				{
					//if (Rnd(1.0f)<0.75f)
					{
						AddThought(NEW aiThoughtStrafing( pChrTarget, 2+Rnd(4), this));	
						BrainState=EBS_STRAFING;
					}
				}
				else*/
				{
					float fDist=(GetBody()->physics->GetPos()-pChrTarget->physics->GetPos()).GetLengthXZ();

					if ((GetBody()->physics->GetPos()-pChrTarget->physics->GetPos()).GetLengthXZ()<=fDistToKick)
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
							AttackCharacter();
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
		if (pChrTarget==NULL) 
			pChrTarget = GetPlayerTarget();//SearchTarget(true);
				
		if (pChrTarget!=NULL )
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
			pThoughtPursuit->ChangeParams(pChrTarget->physics->GetPos(), fDistToKick * 0.85f );			
		}
	}
}

bool aiBoatswain::AttackCharacter()
{
	float fDist=(GetBody()->physics->GetPos()-pChrTarget->physics->GetPos()).GetLengthXZ();

	// Vano: добавил тут, если есть attack params, то используются они
	if (GetBody()->logic->attackParams.Ptr())
	{
		return aiBrain::AttackCharacter();
		//pThoughtKick->Activate(pChrTarget);
		//BrainState = EBS_KICK;
	}
	else
	{
		if (fDist<=fDistToKick)
		{					
			pThoughtKickNear->Activate(pChrTarget);
			BrainState=EBS_KICK;
		}
		else			
		if (fDist<=fDistToMidleKick)
		{					
			pThoughtKickMidle->Activate(pChrTarget);
			BrainState=EBS_KICK;
		}
		else
		if (fDist<=fDistToFarKick)
		{					
			pThoughtKickFar->Activate(pChrTarget);
			BrainState=EBS_KICK;
		}
		else
		{
			float dist = fDistToKick;
			if (Rnd()>0.5f) dist = fDistToMidleKick;
			if (Rnd()>0.5f) dist = fDistToFarKick;

			pThoughtPursuit->Activate(pChrTarget->physics->GetPos(), WayPointsName, dist * 0.85f, true);
			BrainState=EBS_PURSUIT;	
		}
	}

	return true;
}

void aiBoatswain::NotifyAboutAttack(Character* attacker,float damage)
{
	if (!attacker) return;
	if (BrainState!=EBS_BLOCKING && blocking)
	{
		if ((GetBody()->physics->GetPos() - attacker->physics->GetPos())<3.0f)
		{
			pThoughtBlocking->Activate(attacker,3);
			BrainState=EBS_BLOCKING;

			return;
		}
	}
}

void aiBoatswain::Hit(float dmg,Character* _offender, const char* reaction, DamageReceiver::DamageSource source)
{		
	if (BrainState==EBS_BLOCKING) return;

	fDamageCharger += dmg;

	if (BrainState!=EBS_KICK && BrainState!=EBS_RAGE)
	{									
		if (_offender!=NULL)
		{
			pChrTarget=_offender;
			AttackCharacter();
		}
	}
}

void aiBoatswain::Command(dword numParams, const char ** params)
{	
	if (numParams<1) return;

	if (string::IsEqual(params[0],"blocking"))
	{
		blocking = false;
		if (params[1][0] == 'Y' || params[1][0] == 'y')
		{
			blocking = true;
		}		
	}
	else
	{
		aiBrain::Command(numParams, params);
	}
}

DeclareCharacterController(aiBoatswain, "Boatswain", 11)