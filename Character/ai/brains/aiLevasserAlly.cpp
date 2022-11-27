#include "aiLevasserAlly.h"
#include "aiBrain.h"
#include "..\CharacterAI.h"
#include "../../character/components/CharacterLogic.h"
#include "../../character/components/CharacterPhysics.h"

class LevasserAllyParams : public CharacterControllerParams
{

public:
	
	float fDistToKick;
	float fDistofView;	
				
	int   iNumPlayerAttckers;
	int   iNumNPCAttckers;

	float fMaxDistToPlayer;	

	ConstString WayPointsName;

	LevasserAllyParams()
	{
	}

	virtual ~LevasserAllyParams()
	{
	}

	virtual bool IsControllerSupport(const ConstString & controllerClassName)
	{
		CHECK_CONTROLLERSUPPORT("LevasserAllyParams")
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
		fDistToKick=reader.Float();	
		fDistofView=reader.Float();		

		iNumPlayerAttckers=reader.Long();
		iNumNPCAttckers=reader.Long();
		
		fMaxDistToPlayer=reader.Float();	

		WayPointsName=reader.String();
	}
};

MOP_BEGINLISTCG(LevasserAllyParams, "AI params LevasserAlly", '1.00', 90, "LevasserAlly Params", "Character")
		
	MOP_FLOATEX("MaxDist To Kick", 4.0f, 1.0f, 5.0f)	
	MOP_FLOATEX("Dist of View", 12.0f, 3.0f, 1000.0f)	

	MOP_LONGEX("MAX Player Attckers", 5, 1, 20)
	MOP_LONGEX("MAX NPC Attckers", 1, 1, 20)

	MOP_FLOATEX("MAX Distance to Player", 5.0f, 3.0f, 20.0f)
	
	MOP_STRING("WayPoints Name", "WayPoints")

MOP_ENDLIST(LevasserAllyParams)




aiLevasserAlly::aiLevasserAlly(Character & ch, const char* szName) : aiBrain (ch, szName)
{
	fDistToKick = 4.0f;
	fDistofView = 7.0f;

	iNumPlayerAttckers = 5;
	iNumNPCAttckers = 1;

	fMaxDistToPlayer = 5.0f;

	BrainState=EBS_IDLE;

	pChrTarget=NULL;	

	static const ConstString defWayPoints("WayPoints"); WayPointsName=defWayPoints;
	
	pLevasserAllyKick = (aiLevasserAllyKick*)AddThought(NEW aiLevasserAllyKick(this));
	pLevasserAllyRoll = (aiLevasserAllyRoll*)AddThought(NEW aiLevasserAllyRoll(this));
}

aiLevasserAlly::~aiLevasserAlly()
{
}

void aiLevasserAlly::SetParams(CharacterControllerParams * params)
{
	if (!params) return;
	
	fDistToKick=((LevasserAllyParams*)params)->fDistToKick;
	fDistofView=((LevasserAllyParams*)params)->fDistofView;	

	iNumPlayerAttckers=((LevasserAllyParams*)params)->iNumPlayerAttckers;
	iNumNPCAttckers=((LevasserAllyParams*)params)->iNumNPCAttckers;

	fMaxDistToPlayer=((LevasserAllyParams*)params)->fMaxDistToPlayer;

	WayPointsName=((LevasserAllyParams*)params)->WayPointsName;
}

//Перезапустить контроллер при перерождении персонажа
void aiLevasserAlly::Reset()
{
	BrainState=EBS_IDLE;

	aiBrain::Reset();	
}

//Обновить состояние контролера 10 раз в секунду (распределено между персонажами)
void aiLevasserAlly::Tick()
{
	if (!pChrPlayer)
	{
		UpdateMO();

		if (!pChrPlayer)
		{
			return;
		}
	}

	Character* NPC = GetBody();

	/*string sBrainState="none";

	switch (BrainState)
	{		
		case EBS_KICK:
		{
			sBrainState="Eat my boots!!!";
		}
		break;
		case EBS_PURSUIT:
		{
			sBrainState="I catch you!!!";
		}
		break;
		case EBS_IDLE:
		{
			sBrainState="Whe are you?!?!";
		}		
		break;
		case EBS_STRAFING:
		{
			sBrainState="Don't hert me";
		}
		break;		
	}

	GetBody()->Mission().Render().Print(GetBody()->physics->GetPos(),-1.0f, -8.0f, 0xFFFF0000, "%s", sBrainState.c_str());	

	if (pChrTarget!=NULL)
	{
		float fDist = (pChrTarget->physics->GetPos() - NPC->physics->GetPos()).GetLength();		

		GetBody()->Mission().Render().Print(GetBody()->physics->GetPos()+Vector(0,-1,0),-1.0f, -8.0f, 0xFFFF0000, "I have target at %f", fDist);
	}*/
	
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
		else
		if (pChrTarget->logic->GetState()==CharacterLogic::state_pair)
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
				/*if (pChrTarget==pChrPlayer)
				{
					//if (Rnd(1.0f)<0.75f)
					{
						AddThought(NEW aiThoughtStrafing( pChrTarget, 2+Rnd(4), this));	
						BrainState=EBS_STRAFING;
					}
					/*else
					{
					AddThought(NEW aiThoughtBlocking( pChrTarget, Rnd(3)+2, this));	
					BrainState=EBS_BLOCKING;
					}*/
				/*}
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
							pLevasserAllyKick->Activate(pChrTarget, fDistToKick);
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
	
	if (BrainState==EBS_STRAFING)
	{
		if (pChrTarget==NULL) pChrTarget=SearchTarget(true);

		if (pChrTarget!=NULL)
		{
			Reset();

			ExecuteThoughts(0);

			BrainState=EBS_IDLE;
		}
	}


	if (BrainState==EBS_IDLE)
	{
		if (pChrTarget==NULL) pChrTarget=SearchTarget(true);

		if (pChrTarget==NULL&&searchResult.Size()>0 )
		{
			//AddThought(NEW aiThoughtStrafing( (Character*)SectorSearchResult[0].pObject, Rnd(2)+2, this));	
			//BrainState=EBS_STRAFING;

			//AddThought(NEW aiThoughtStanding( Rnd(2)+2, this));	
			//BrainState=EBS_STANDING;			
		}
		else
		if (pChrTarget!=NULL)
		{
			AttackCharacter();
		}
		else
		if (pChrTarget==NULL )
		{			
			FollowPlayer();
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
	else
	if (BrainState==EBS_FOLLOWHERO&&IsEmpty())
	{	
		FollowPlayer();
	}
	else
	if (BrainState==EBS_FOLLOWHERO)
	{
		if (!IsEmpty()&&AllowRecreatePath(pChrPlayer))
		{			
			pThoughtPursuit->ChangeParams(pChrPlayer->physics->GetPos(), fMaxDistToPlayer );
		}
	}
}

bool aiLevasserAlly::AttackCharacter()
{
	float fDist=(GetBody()->physics->GetPos()-pChrTarget->physics->GetPos()).GetLengthXZ();

	if (fDist<=fDistToKick)
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
			float k = Rnd();			

			if (k<0.5f)
			{
				pLevasserAllyRoll->Activate(pChrTarget, 2.0f + Rnd(1.0f));
				BrainState=EBS_STRAFING;
			}			
			else
			{			
				pLevasserAllyKick->Activate(pChrTarget, fDistToKick);
				BrainState=EBS_KICK;
			}
		}
	}
	else			
	{
		pThoughtPursuit->Activate(pChrTarget->physics->GetPos(), WayPointsName, fDistToKick * 0.85f,true);
		BrainState=EBS_PURSUIT;	
	}

	return true;
}

void aiLevasserAlly::FollowPlayer()
{
	float fDist=(GetBody()->physics->GetPos()-pChrPlayer->physics->GetPos()).GetLengthXZ();

	if (fDist>fMaxDistToPlayer)
	{		
		pThoughtPursuit->Activate(pChrPlayer->physics->GetPos(), WayPointsName, fMaxDistToPlayer * 0.85f, true);
		BrainState=EBS_FOLLOWHERO;	
	}
}

void aiLevasserAlly::Hit(float dmg,Character* _offender, const char* reaction, DamageReceiver::DamageSource source)
{
	if (BrainState==EBS_STRAFING || BrainState==EBS_FATALITY ) return;
	
	//if (BrainState!=EBS_KICK)
	{
		Reset();
			
		ExecuteThoughts(0);

		BrainState=EBS_IDLE;		

		pChrTarget=SearchTarget(false, false);

		if (pChrTarget!=NULL)
		{
			float fDist=(GetBody()->physics->GetPos()-pChrTarget->physics->GetPos()).GetLengthXZ();

			if ((GetBody()->physics->GetPos()-pChrTarget->physics->GetPos()).GetLengthXZ()<=fDistToKick)
			{
				float k = Rnd();

				if (k<0.5f && BrainState!=EBS_STRAFING)
				{
					pLevasserAllyRoll->Activate(pChrTarget, 2.0f + Rnd(1.0f));
					BrainState=EBS_STRAFING;
				}
				else
				{			
					pLevasserAllyKick->Activate(pChrTarget, fDistToKick);
					BrainState=EBS_KICK;
				}
			}
		}
	}
}

void aiLevasserAlly::Death()
{
}

DeclareCharacterController(aiLevasserAlly, "LevasserAlly", 11)