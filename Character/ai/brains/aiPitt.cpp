#include "aiPitt.h"
#include "aiBrain.h"
#include "..\CharacterAI.h"
#include "../../character/components/CharacterLogic.h"
#include "../../character/components/CharacterPhysics.h"

class PittParams : public CharacterControllerParams
{

public:
	
	float fDistofView;
				
	int   iNumPlayerAttckers;
	int   iNumNPCAttckers;

	float fMaxDistToPlayer;	
	float fDistToResetAndFollowPlayer;

	ConstString WayPointsName;

	PittParams()
	{
	}

	virtual ~PittParams()
	{
	}

	virtual bool IsControllerSupport(const ConstString & controllerClassName)
	{
		CHECK_CONTROLLERSUPPORT("PittParams")
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
		fDistofView=reader.Float();		

		iNumPlayerAttckers=reader.Long();
		iNumNPCAttckers=reader.Long();
		
		fMaxDistToPlayer=reader.Float();
		fDistToResetAndFollowPlayer=reader.Float();

		WayPointsName=reader.String();
	}
};

MOP_BEGINLISTCG(PittParams, "AI params Pitt", '1.00', 90, "Pitt Params", "Character")
		
	MOP_FLOATEX("Dist of View", 12.0f, 3.0f, 1000.0f)	

	MOP_LONGEX("MAX Player Attckers", 5, 1, 20)
	MOP_LONGEX("MAX NPC Attckers", 1, 1, 20)

	MOP_FLOATEX("MAX Distance to Player", 5.0f, 3.0f, 200.0f)
	MOP_FLOATEX("Dist to Reset and Follow Player", 10.0f, 3.0f, 200.0f)
	
	MOP_STRING("WayPoints Name", "WayPoints")

MOP_ENDLIST(PittParams)




aiPitt::aiPitt(Character & ch, const char* szName) : aiBrain (ch, szName)
{
	fDistofView = 7.0f;

	iNumPlayerAttckers = 5;
	iNumNPCAttckers = 1;

	fMaxDistToPlayer = 5.0f;
	fDistToResetAndFollowPlayer = 10.0f;

	BrainState=EBS_IDLE;

	pChrTarget=NULL;

	pChrPlayer = NULL;

	static const ConstString defWayPoints("WayPoints"); 
	WayPointsName=defWayPoints;
	
	bLeadMode = false;
	destination = 0.0f;
		
	pPittFatality =		(aiPittFatality*)AddThought(NEW aiPittFatality(this));	
	pThoughtBlocking =	(aiPittBlocking*)AddThought(NEW aiPittBlocking(this));
	pPittRoll =			(aiPittRoll*)AddThought(NEW aiPittRoll(this));		
		
	pThoughtKick->AddAction("Attack1");
	pThoughtKick->AddAction("Attack2");
	pThoughtKick->AddAction("Attack3");	
	pThoughtKick->AddAction("Attack4");
	pThoughtKick->AddAction("Combo1");
	pThoughtKick->AddAction("Combo2");
	pThoughtKick->AddAction("Combo3");
	pThoughtKick->PrepareActions();
}

aiPitt::~aiPitt()
{
}

void aiPitt::SetParams(CharacterControllerParams * params)
{
	if (!params) return;
	
	fDistofView=((PittParams*)params)->fDistofView;	

	iNumPlayerAttckers=((PittParams*)params)->iNumPlayerAttckers;
	iNumNPCAttckers=((PittParams*)params)->iNumNPCAttckers;

	fMaxDistToPlayer=((PittParams*)params)->fMaxDistToPlayer;
	fDistToResetAndFollowPlayer=((PittParams*)params)->fDistToResetAndFollowPlayer;

	WayPointsName=((PittParams*)params)->WayPointsName;
}

bool aiPitt::CheckCharState(Character* target)
{
	if (target->logic->IsPairMode())
	{
		return false;
	}

	return aiBrain::CheckCharState(target);
}

bool aiPitt::FilterFindedTarget(Character* pTarget, bool statistSearch)
{
	if ((pTarget->physics->GetPos()-pChrPlayer->physics->GetPos()).GetLength()>fDistToResetAndFollowPlayer*0.9f)
	{
		return true;
	}	

	//if (pTarget->controller->GetChrTarget() == pChrPlayer)
	//{
	//	return true;
	//}

	return aiBrain::FilterFindedTarget(pTarget, statistSearch);

}

//Обновить состояние контролера 10 раз в секунду (распределено между персонажами)
void aiPitt::Tick()
{
	if (!pChrPlayer)
	{
		UpdateMO();

		if (!pChrPlayer)
		{
			return;
		}
	}	

	if (!pChrPlayer->IsActive() || !pChrPlayer->IsShow()) return;
	
	if (bLeadMode)
	{
		float fDist = GetBody()->arbiter->GetPowDistance(GetBody(),pChrPlayer);

		if (BrainState==EBS_PURSUIT)
		{
			if (fDist>fMaxDistToPlayer*fMaxDistToPlayer + 4.0f)
			{		
				Reset();
			}
		}
		else
		if (BrainState==EBS_IDLE)
		{
			if (fDist<=fMaxDistToPlayer*fMaxDistToPlayer)
			{		
				pThoughtPursuit->Activate(destination, WayPointsName, 1.0f,true);
				BrainState=EBS_PURSUIT;	
			}
		}
		
		return;
	}	
	
	CheckTargetState();

	/*if (pChrTarget)
	{
		if (pChrPlayer->physics->GetPos()-pChrTarget->physics->GetPos()>fDistToResetAndFollowPlayer*0.9f)
		{
			Reset();
			ExecuteThoughts(0);

			BrainState=EBS_IDLE;
			pChrTarget=NULL;
		}
	}*/

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


	if (GetBody()->arbiter->GetPowDistance(GetBody(),pChrPlayer)>fDistToResetAndFollowPlayer*fDistToResetAndFollowPlayer && BrainState != EBS_FOLLOWHERO)
	{
		FollowPlayer();
	}

	if (BrainState==EBS_IDLE)
	{
		if (pChrTarget==NULL) pChrTarget=SearchTarget(true);
		
		if (pChrTarget==NULL )
		{
			pChrTarget=SearchTarget(false);
		}

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
			pThoughtPursuit->ChangeParams(pChrTarget->physics->GetPos(), GetAttackDistance() * 0.85f );			
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
		if (!IsEmpty()&&AllowRecreatePath(pChrTarget))
		{
			pThoughtPursuit->ChangeParams(pChrPlayer->physics->GetPos(), fMaxDistToPlayer );			
		}
	}
}

bool aiPitt::AttackCharacter()
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
			float k = Rnd();

			if (k<0.25f)
			{
				pPittRoll->Activate(pChrTarget, 2.0f + Rnd(1.0f));
				BrainState=EBS_STRAFING;
			}
			else
			if (k<0.5f)
			{
				/*char pairLink[128];
				FatalityParams::TFatalityType type;

				if (GetBody()->logic->CheckPairLinks(pChrTarget,"pair",pairLink,type))
				{
					pPittFatality->Activate(pChrTarget);
					BrainState=EBS_FATALITY;
				}
				else*/
				{
					pThoughtKick->Activate(pChrTarget);
					BrainState=EBS_KICK;
				}
			}
			else
			{			
				pThoughtKick->Activate(pChrTarget);
				BrainState=EBS_KICK;
			}
		}
	}
	else			
	{
		pThoughtPursuit->Activate(pChrTarget->physics->GetPos(), WayPointsName, GetAttackDistance() * 0.85f, true);
		BrainState=EBS_PURSUIT;	
	}

	return true;
}

void aiPitt::FollowPlayer()
{
	if (!pChrPlayer->IsActive() || !pChrPlayer->IsShow()) return;

	if (GetBody()->arbiter->GetPowDistance(GetBody(),pChrPlayer)>fMaxDistToPlayer*fMaxDistToPlayer)
	{		
		Reset();
		pChrTarget = NULL;

		pThoughtPursuit->Activate(pChrPlayer->physics->GetPos(), WayPointsName, fMaxDistToPlayer * 0.55f, true);
		BrainState=EBS_FOLLOWHERO;	
	}
}

void aiPitt::Hit(float dmg,Character* _offender, const char* reaction, DamageReceiver::DamageSource source)
{
	if (BrainState==EBS_STRAFING || BrainState==EBS_FATALITY || BrainState==EBS_FOLLOWHERO ) return;	

	//if (BrainState!=EBS_KICK)
	{
		Reset();
			
		ExecuteThoughts(0);

		BrainState=EBS_IDLE;		

		pChrTarget=SearchTarget(false, false);

		if (pChrTarget!=NULL)
		{
			float fDist=(GetBody()->physics->GetPos()-pChrTarget->physics->GetPos()).GetLengthXZ();

			if ((GetBody()->physics->GetPos()-pChrTarget->physics->GetPos()).GetLengthXZ()<=GetAttackDistance())
			{
				float k = Rnd();

				/*if (k<0.2f && BrainState!=EBS_BLOCKING)
				{
					AddThought(NEW aiPittBlocking(pChrTarget, 2.5f + Rnd(2.0f), this));
					BrainState=EBS_BLOCKING;
				}
				else*/
				if (k<0.5f && BrainState!=EBS_STRAFING)
				{
					pPittRoll->Activate(pChrTarget, 2.0f + Rnd(1.0f));
					BrainState=EBS_STRAFING;
				}
				else
				if (k<0.7f)
				{
					/*char pairLink[128];
					FatalityParams::TFatalityType type;

					if (GetBody()->logic->CheckPairLinks(pChrTarget,"pair",pairLink,type))
					{
						pPittFatality->Activate(pChrTarget);
						BrainState=EBS_FATALITY;
					}
					else*/
					{
						pThoughtKick->Activate(pChrTarget);
						BrainState=EBS_KICK;
					}
				}
				else
				{			
					pThoughtKick->Activate(pChrTarget);
					BrainState=EBS_KICK;
				}
			}
		}
	}
}

void aiPitt::Command(dword numParams, const char ** params)
{	
	if (string::IsEqual(params[0],"LeadTo") && numParams>=2)
	{
		MOSafePointer obj;
		GetBody()->FindObject(ConstString(params[1]),obj);		

		Reset();

		if (obj.Ptr())
		{
			bLeadMode = true;
			Matrix mat;
			obj.Ptr()->GetMatrix(mat);
			destination = mat.pos;			
		}
		else
		{
			bLeadMode = false;
		}		
	}
}

DeclareCharacterController(aiPitt, "Pitt", 11)