
#include "aiFinalBoss.h"
#include "../../character/components/CharacterLogic.h"
#include "../../character/components/CharacterPhysics.h"

class aiFinalBossParams : public CharacterControllerParams
{

public:	

	ConstString WayPointsName;
	float fDistofView;

	float damage_to_block;
	float dist_to_block;
	float strafe_cooldown;

	float fTimeToNextAttack;

	aiFinalBossParams();
	~aiFinalBossParams();

	virtual bool IsControllerSupport(const ConstString & controllerClassName);

	bool Create(MOPReader & reader);

	bool EditMode_Create(MOPReader & reader);

	bool EditMode_Update(MOPReader & reader);

	void Init (MOPReader & reader);	
};

aiFinalBossParams::aiFinalBossParams()
{
}

aiFinalBossParams::~aiFinalBossParams()
{
}

bool aiFinalBossParams::IsControllerSupport(const ConstString & controllerClassName)
{
	CHECK_CONTROLLERSUPPORT("aiFinalBossParams")
}

bool aiFinalBossParams::Create(MOPReader & reader)
{
	Init(reader);
	return true;
}

bool aiFinalBossParams::EditMode_Create(MOPReader & reader)
{
	Init(reader);
	return true;
}

bool aiFinalBossParams::EditMode_Update(MOPReader & reader)
{
	Init(reader);
	return true;
}

void aiFinalBossParams::Init(MOPReader & reader)
{
	WayPointsName = reader.String();
	fDistofView = reader.Float();	
	damage_to_block = reader.Float();
	dist_to_block = reader.Float();
	strafe_cooldown = reader.Float();
	fTimeToNextAttack = reader.Float();
}

MOP_BEGINLISTCG(aiFinalBossParams, "AI FinalBoss params", '1.00', 90, "AI FinalBoss Params", "Character")	

	MOP_STRING("WayPoints Name", "WayPoints")
	MOP_FLOATEX("Dist of View", 12.0f, 3.0f, 1000.0f)
	MOP_FLOATEX("Damage when strafe |v2|", 11.0f, 3.0f, 1000.0f)
	MOP_FLOATEX("Distance when strafe |v2|", 3.5f, 0.5f, 7.0f)
	MOP_FLOATEX("Strafe cooldown |v2|", 6.0f, 0.0f, 25.0f)
	MOP_FLOATEX("Time To Next Attack", 3.0f, 1.0f, 1000.0f)

MOP_ENDLIST(aiFinalBossParams)


aiFinalBoss::aiFinalBoss(Character & ch, const char* szName) : aiBrain (ch, szName)
{
	strafeTime = 0.0f;

	fDistofView = 7.0f;
	fDistofViewPlayer = 10.0f;
	fDistToLoseTarget = fDistofView +5.0f;

	iNumPlayerAttckers = 5;
	iNumNPCAttckers = 1;	

	damage_to_block = 25.0;
	dist_to_block = 3.0f;

	strafe_cooldown = 0.0f;

	BrainState=EBS_IDLE;

	pChrTarget=NULL;

	static const ConstString defWayPoints("WayPoints"); WayPointsName=defWayPoints;
		
	pThoughtKick->AddAction("Strafe1");	
	pThoughtKick->PrepareActions();

	pThoughtCounterKick = (aiThoughtKick*)AddThought(NEW aiThoughtKick(this));
	pThoughtCounterKick->AddAction("Strafe1");	
	pThoughtCounterKick->PrepareActions();
}

aiFinalBoss::~aiFinalBoss()
{
}

void aiFinalBoss::SetParams(CharacterControllerParams * params)
{
	if (!params) return;
	
	WayPointsName=((aiFinalBossParams*)params)->WayPointsName;

	fDistofView = ((aiFinalBossParams*)params)->fDistofView;
	fDistofViewPlayer = fDistofView;

	fDistToLoseTarget = fDistofView +5.0f;

	damage_to_block = ((aiFinalBossParams*)params)->damage_to_block;
	dist_to_block = ((aiFinalBossParams*)params)->dist_to_block;

	strafe_cooldown = ((aiFinalBossParams*)params)->strafe_cooldown;

	fTimeToNextAttack2 = ((aiFinalBossParams*)params)->fTimeToNextAttack;
}

//Обновить состояние контролера 10 раз в секунду (распределено между персонажами)
void aiFinalBoss::Tick()
{		
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
		else
		if (pChrTarget->logic->IsPairMode())
		{
			Reset();

			ExecuteThoughts(0);

			BrainState=EBS_IDLE;

			pChrTarget=NULL;
		}
	}
	
	if (IsEmpty())
	{		
		if (BrainState==EBS_KICK)
		{
			if (pChrTarget!=NULL)
			{
				//if (pChrTarget==pChrPlayer)
				//{
					//if (Rnd(1.0f)<0.75f)
					{
						//AddThought(NEW aiThoughtStrafing( pChrTarget, 2+Rnd(4), this));	
						//BrainState=EBS_STRAFING;
					}
					/*else
					{
					AddThought(NEW aiThoughtBlocking( pChrTarget, Rnd(3)+2, this));	
					BrainState=EBS_BLOCKING;
					}*/
				//}
				//else
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
		
		if (pChrTarget==NULL&&searchResult.Size()>0)
		{
			//AddThought(NEW aiThoughtStrafing( (Character*)SectorSearchResult[0].pObject, Rnd(2)+2, this));	
			//BrainState=EBS_STRAFING;

			//AddThought(NEW aiThoughtStanding( Rnd(2)+2, this));	
			//BrainState=EBS_STANDING;			
		}
		else
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
		if (!IsEmpty() && AllowRecreatePath(pChrTarget))
		{
			pThoughtPursuit->ChangeParams(pChrTarget->physics->GetPos(), PursuitDist() );
		}
	}
}

bool aiFinalBoss::AttackCharacter()
{
	float fDist=(GetBody()->physics->GetPos()-pChrTarget->physics->GetPos()).GetLengthXZ();

	if (fDist<=GetAttackDistance() * 1.5f)
	{			
		Vector vPos = GetBody()->physics->GetPos();

		float dy=fabs(vPos.y - pChrTarget->physics->GetPos().y);

		if (dy>0.8f)
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
			
			fCurTimeToNextAttack = GetTimeToNextAttack();
		}
	}
	else			
	{
		pThoughtPursuit->Activate(pChrTarget->physics->GetPos(), WayPointsName, PursuitDist(), true);
		BrainState=EBS_PURSUIT;	
	}

	return true;
}

//Мыслить ;)
void aiFinalBoss::ExecuteThoughts (float fDeltaTime)
{
	strafeTime -= fDeltaTime;

	aiBrain::ExecuteThoughts(fDeltaTime);
}

void aiFinalBoss::NotifyAboutAttack(Character* attacker,float damage)
{
	if (!attacker) return;

	CharacterLogic::State st = GetBody()->logic->GetState();

	if (st != CharacterLogic::state_attack && damage >= damage_to_block && strafeTime <= 0.0f)
	{
		float dist = (GetBody()->physics->GetPos() - attacker->physics->GetPos()).GetLength2();
		if (dist < Sqr(dist_to_block) && rand()%100 < 70)
		{
			strafeTime = RRnd(0.5f, 1.0f) * strafe_cooldown;

			pThoughtCounterKick->Activate(pChrTarget);
			BrainState = EBS_KICK;

			return;
		}
	}
}

void aiFinalBoss::Hit(float dmg,Character* _offender, const char* reaction, DamageReceiver::DamageSource source)
{	
	if (BrainState != EBS_KICK)
	{									
		if (_offender!=NULL)
		{
			pChrTarget=_offender;
			AttackCharacter();
		}
	}
}

DeclareCharacterController(aiFinalBoss, "FinalBoss", 12)