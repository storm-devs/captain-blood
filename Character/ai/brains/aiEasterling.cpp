
#include "aiEasterling.h"
#include "../../character/components/CharacterLogic.h"
#include "../../character/components/CharacterPhysics.h"

class aiEasterlingParams : public CharacterControllerParams
{

public:	

	ConstString WayPointsName;
	float fDistofView;	

	float fDrinkHealth;
	float fRageTime;
	float fRageRotTime;

	array<aiEasterling::Attack> attacks;

	aiEasterlingParams();
	~aiEasterlingParams();

	virtual bool IsControllerSupport(const ConstString & controllerClassName);

	bool Create(MOPReader & reader);

	bool EditMode_Create(MOPReader & reader);

	bool EditMode_Update(MOPReader & reader);

	void Init (MOPReader & reader);	
};

aiEasterlingParams::aiEasterlingParams(): attacks(_FL_)
{
}

aiEasterlingParams::~aiEasterlingParams()
{
}

bool aiEasterlingParams::IsControllerSupport(const ConstString & controllerClassName)
{
	CHECK_CONTROLLERSUPPORT("aiEasterlingParams")
}

bool aiEasterlingParams::Create(MOPReader & reader)
{
	Init(reader);
	return true;
}

bool aiEasterlingParams::EditMode_Create(MOPReader & reader)
{
	Init(reader);
	return true;
}

bool aiEasterlingParams::EditMode_Update(MOPReader & reader)
{
	Init(reader);
	return true;
}

void aiEasterlingParams::Init(MOPReader & reader)
{
	WayPointsName=reader.String();
	fDistofView=reader.Float();	
	fDrinkHealth=reader.Float();
	fRageTime=reader.Float();
	fRageRotTime=reader.Float();

	int count = reader.Array();

	for (int i=0;i<count;i++)
	{
		aiEasterling::Attack& attack = attacks[attacks.Add()];

		attack.hp = reader.Float()*0.01f;
		attack.link = reader.String();

	}	
}

MOP_BEGINLISTCG(aiEasterlingParams, "AI Easterling params", '1.00', 90, "AI Easterling Params", "Character")	

	MOP_STRING("WayPoints Name", "WayPoints")
	MOP_FLOATEX("Dist of View", 12.0f, 3.0f, 1000.0f)
	MOP_FLOATEX("Drink Health", 10.0f, 0.1f, 100.0f)
	MOP_FLOATEX("Rage Time", 3.5f, 0.1f, 100.0f)
	MOP_FLOATEX("Rage RotationTime", 1.75f, 0.1f, 100.0f)

	MOP_ARRAYBEGC("Attacks", 0, 1000, "Attacks")
		MOP_FLOATEX("HP", 100.0f, 0.1f, 100.0f)
		MOP_STRINGC("Attack Link", "", "Attack Link")		
	MOP_ARRAYEND

MOP_ENDLIST(aiEasterlingParams)


aiEasterling::aiEasterling(Character & ch, const char* szName) : aiBrain (ch, szName),attacks(_FL_)
{
	fDistofView = 7.0f;
	fDistofViewPlayer = 10.0f;
	fDistToLoseTarget = fDistofView +5.0f;

	iNumPlayerAttckers = 5;
	iNumNPCAttckers = 1;
	
		
	fDrinkHealth = 10.0f;
	fRageTime = 5.0f;
	fRageRotTime = 2.0f;


	BrainState=EBS_IDLE;

	pChrTarget=NULL;

	static const ConstString defWayPoints("WayPoints");
	WayPointsName=defWayPoints;

	fDamageCharger = 0;
		
	pEasterlingRage =   (aiEasterlingRage*)AddThought(NEW aiEasterlingRage(this));
	pEasterlingDrink = (aiEasterlingDrink*)AddThought(NEW aiEasterlingDrink(this));
	
	pThoughtKick->AddAction("Attack2");
	pThoughtKick->AddAction("Attack3");	
	pThoughtKick->AddAction("Attack4");	
	pThoughtKick->AddAction("Attack5");	
	pThoughtKick->AddAction("Attack6");
	pThoughtKick->AddAction("Grab");
	pThoughtKick->PrepareActions();
}

aiEasterling::~aiEasterling()
{
}

void aiEasterling::SetParams(CharacterControllerParams * params)
{
	if (!params) return;
	
	WayPointsName=((aiEasterlingParams*)params)->WayPointsName;

	fDistofView = ((aiEasterlingParams*)params)->fDistofView;
	fDistofViewPlayer = fDistofView;

	fDistToLoseTarget = fDistofView +5.0f;

	fDrinkHealth = ((aiEasterlingParams*)params)->fDrinkHealth;
	fRageTime = ((aiEasterlingParams*)params)->fRageTime;
	fRageRotTime = ((aiEasterlingParams*)params)->fRageRotTime;

	attacks.Empty();

	for (int i=0;i<(int)((aiEasterlingParams*)params)->attacks.Size();i++)
	{
		attacks.Add(((aiEasterlingParams*)params)->attacks[i]);
	}
}

//Обновить состояние контролера 10 раз в секунду (распределено между персонажами)
void aiEasterling::Tick()
{	
	UpdateAttacks();

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
		if (!IsEmpty()&&AllowRecreatePath(pChrTarget))
		{
			pThoughtPursuit->ChangeParams(pChrTarget->physics->GetPos(), PursuitDist() );
		}
	}
}

bool aiEasterling::AttackCharacter()
{
	float fDist=(GetBody()->physics->GetPos()-pChrTarget->physics->GetPos()).GetLengthXZ();

	if (fDist<=GetAttackDistance()*1.2f)
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
		pThoughtPursuit->Activate(pChrTarget->physics->GetPos(), WayPointsName, PursuitDist(), true);
		BrainState=EBS_PURSUIT;	
	}

	return true;
}

void aiEasterling::Hit(float dmg,Character* _offender, const char* reaction, DamageReceiver::DamageSource source)
{
	if (BrainState==EBS_RAGE)
	{
		return;
	}
	
	if (BrainState==EBS_DRINK)
	{
		pEasterlingRage->Activate( pChrTarget, fRageTime,fRageRotTime);	
		BrainState=EBS_RAGE;

		return;
	}

	fDamageCharger += dmg;

	if (fDamageCharger>chr.logic->GetMaxHP() * 0.1f && pChrTarget!=NULL)
	{					
		pEasterlingDrink->Activate(fDrinkHealth);	
		BrainState=EBS_DRINK;		

		fDamageCharger = 0;

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

void aiEasterling::UpdateAttacks()
{
	bool need_update = false;

	for (int i=0;i<(int)attacks.Size();i++)
	{
		if (attacks[i].hp * chr.logic->GetMaxHP() >= chr.logic->GetHP())
		{
			if (!attacks[i].used)
			{
				need_update = true;
				break;
			}
		}
		else
		{
			if (attacks[i].used)
			{
				need_update = true;
				break;
			}
		}
	}

	if (need_update)
	{
		pThoughtKick->ResetStack();

		for (int i=0;i<(int)attacks.Size();i++)
		{
			if (attacks[i].hp * chr.logic->GetMaxHP() >= chr.logic->GetHP())
			{
				pThoughtKick->AddAction(attacks[i].link.c_str());
				attacks[i].used = true;
			}
			else
			{
				attacks[i].used = false;
			}
		}

		pThoughtKick->PrepareActions();
	}
}

DeclareCharacterController(aiEasterling, "Easterling", 11)