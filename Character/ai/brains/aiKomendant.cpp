#include "aiKomendant.h"
#include "aiBrain.h"
#include "../CharacterAI.h"
#include "../../character/components/CharacterLogic.h"
#include "../../character/components/CharacterPhysics.h"

class aiKomendantParams : public CharacterControllerParams
{

public:

	ConstString WayPointsName;	

	virtual bool IsControllerSupport(const ConstString & controllerClassName)
	{
		CHECK_CONTROLLERSUPPORT("KomendantParams")
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
	}
};

MOP_BEGINLISTCG(aiKomendantParams, "AI params Komendant", '1.00', 90, "Komendant params", "Character")

	MOP_STRINGC("Waypoint", "WayPoints", "Name of WayPoint System")	

MOP_ENDLIST(aiKomendantParams)

aiKomendant::aiKomendant(Character & ch, const char* szName) : aiBrain (ch, szName)
{
	animListener.pKomendant = this;

	chr.animation->SetEventHandler(&animListener, (AniEvent)(&AnimListener::SplashDamage), "AI Event");

	fDistofView = 7.0f;
	fDistofViewPlayer = 10.0f;

	iNumPlayerAttckers = 5;
	iNumNPCAttckers = 1;

	fTimeToNextAttack2 = 2.5f;
	BrainState=EBS_IDLE;

	pChrTarget=NULL;

	static const ConstString defWayPoints("WayPoints");
	WayPointsName = defWayPoints;
	
	pThoughtKick->AddAction("Attack1");
	pThoughtKick->AddAction("Attack2");	
	pThoughtKick->PrepareActions();

	pray_pos = 0.0f;
}

aiKomendant::~aiKomendant()
{
}

void aiKomendant::SetParams(CharacterControllerParams * params)
{
	if (!params) return;
	
	WayPointsName=((aiKomendantParams*)params)->WayPointsName;	
}

//Перезапустить контроллер при перерождении персонажа
void aiKomendant::Reset()
{
	BrainState=EBS_IDLE;

	aiBrain::Reset();	
}

void aiKomendant::Update(float dltTime)
{
	if (BrainState == EBS_GO_TO_PRAY)
	{
		if (!IsEmpty())
		{			
			GetBody()->PushCharcters(dltTime,1.75f,true,false);
		}
	}

	aiBrain::Update(dltTime);
}

//Обновить состояние контролера 10 раз в секунду (распределено между персонажами)
void aiKomendant::Tick()
{			
	if (BrainState == EBS_GO_TO_PRAY)
	{
		if (IsEmpty())
		{
			if ((GetBody()->physics->GetPos()-pray_pos).GetLength()>0.75)
			{
				pThoughtPursuit->Activate(pray_pos, WayPointsName, 0.1f, false);
			}
		}		

		return;	
	}

	Character* NPC = GetBody();	

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

bool aiKomendant::AttackCharacter()
{
	float fDist=(GetBody()->physics->GetPos()-pChrTarget->physics->GetPos()).GetLengthXZ();

	if (fDist<=GetAttackDistance())
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

void aiKomendant::Hit(float dmg,Character* _offender, const char* reaction, DamageReceiver::DamageSource source)
{
	if (BrainState!=EBS_SHOOT && string::IsEqual(reaction,"kick") && BrainState!=EBS_GO_TO_PRAY)
	{		
		Reset();
			
		ExecuteThoughts(0);

		BrainState=EBS_IDLE;				

		if (_offender != NULL)
		{
			pChrTarget = _offender;

			pThoughtKick->Activate(pChrTarget);
			BrainState=EBS_KICK;	
		}
	}
}

void aiKomendant::SplashDamage(const char* locator)
{
	Vector pos;

	if (!GetBody()->FindLocator(locator,pos))
	{	
		pos = Vector (0,0,1);

		pos.Rotate(GetBody()->physics->GetAy());

		pos.x *= 2.0f;
		pos.z *= 2.0f;

		pos += GetBody()->physics->GetPos();
	}

	GetBody()->arbiter->SplashDamage(DamageReceiver::ds_bomb, GetBody(), pos, 2.55f, 20.0f, false, "GroundStrike",true);
}

void aiKomendant::Boom(const Vector & pos)
{
	chr.arbiter->Boom(&chr, DamageReceiver::ds_bomb, pos, 1.5f, 20.0f, 1.0f);

	//Рождаем эффекты
	IParticleSystem * p = null;	

	p = chr.Particles().CreateParticleSystem("ExplosionBomb");
	chr.Sound().Create3D("bmb_blast", pos, _FL_);

	if(p)
	{
		p->Teleport(Matrix(Vector(0.0f), pos));
		p->AutoDelete(true);
	}
}

DeclareCharacterController(aiKomendant, "Komendant", 11)