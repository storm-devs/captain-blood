#include "aiChainMan.h"
#include "aiBrain.h"
#include "..\CharacterAI.h"
#include "../../character/components/CharacterLogic.h"
#include "../../character/components/CharacterPhysics.h"

#include "aiParams.h"

aiChainman::aiChainman(Character & ch, const char* szName) : aiBrain (ch, szName)
{
	animListener.pChainman = this;
	chr.animation->SetEventHandler(&animListener, (AniEvent)(&AnimListener::SplashDamage), "AI Event");

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
	
	pThoughtKick->AddAction("Attack1");
	pThoughtKick->AddAction("Combo1");	
	pThoughtKick->AddAction("Combo2");	
	pThoughtKick->PrepareActions();	
}

aiChainman::~aiChainman()
{
}

//Перезапустить контроллер при перерождении персонажа
void aiChainman::Reset()
{
	BrainState=EBS_IDLE;	

	aiBrain::Reset();	
}

bool aiChainman::FilterFindedTarget(Character* pTarget, bool statistSearch)
{
	if (InSafetyPoint(pTarget->physics->GetPos()))					
	{					
		return true;
	}

	if (statistSearch)
	{
		if (!pTarget->controller->IsStatist())
		{
			return true;
		}
	}
	else
	{
		if (!IsStatist())
		{
			if (pTarget->controller->IsStatist())
			{
				return true;
			}
		}
	}

	Vector vTmp = GetBody()->physics->GetPos() - pTarget->physics->GetPos();
	float dist=vTmp.GetLengthXZ2();

	if (pTarget==pChrPlayer)
	{		
		if (dist>fDistofViewPlayer*fDistofViewPlayer)
		{					
			return true;
		}
	}
	else
	{
		float fDist=fDistofView;

		//if (bInCombatPoint)
		{
			//fDist = pCPoints[iCurGr_CPoint]->Points[iCurPt_CPoint].radius;
		}		

		if (dist>fDist*fDist)
		{					
			return true;
		}
	}		

	if (!pTarget->IsShow())
	{
		return true;
	}

	if (!pTarget->IsActive())
	{
		return true;
	}

	if (pTarget->logic->IsActor())
	{
		return true;
	}

/*	if (pTarget->logic->IsPairMode())
	{
		return true;
	}*/

	if (pTarget->logic->IsDead())
	{
		return true;
	}	

	if (pTarget->logic->GetSide() == CharacterLogic::s_boss)
	{
		return true;
	}

	if (pTarget->logic->GetSide() == CharacterLogic::s_npc)
	{
		return true;
	}

	if (!pTarget->logic->IsEnemy(GetBody()))
	{
		return true;
	}

	if (AIZone)
	{
		Matrix mat;

		pTarget->GetMatrix(mat);					

		if (!InAIZone(mat.pos))
		{
			return true;
		}
	}

	return false;
}

void aiChainman::ActivateTargetPairStateThought()
{	
	BrainState=EBS_WAIT_FATALITY;
}

void aiChainman::TickWhenEmpty()
{
	if (BrainState==EBS_KICK)
	{
		if (pChrTarget!=NULL)
		{
			if (pChrTarget==pChrPlayer)
			{
				pThoughtStanding->Activate(Rnd(3)+0.2f,pChrTarget);
				BrainState=EBS_STANDING;
			}
			else
			{
				float fDist=(GetBody()->physics->GetPos()-pChrTarget->physics->GetPos()).GetLengthXZ();

				if (fDist<=GetAttackDistance())
				{
					pThoughtKick->Activate(pChrTarget);
					BrainState=EBS_KICK;					
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

bool aiChainman::AttackCharacter()
{
	float fDist=(GetBody()->physics->GetPos()-pChrTarget->physics->GetPos()).GetLengthXZ();

	pThoughtKick->CheckDist(fDist);
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
		}
	}
	else			
	{
		pThoughtPursuit->Activate(pChrTarget->physics->GetPos(), WayPointsName, PursuitDist(),true);
		BrainState=EBS_PURSUIT;	

		DeattachFromCP();
	}

	return true;
}

void aiChainman::Hit(float dmg,Character* _offender, const char* reaction, DamageReceiver::DamageSource source)
{	
	if (GetBody()->logic->GetState() == CharacterLogic::state_knockdown) return;

	iNumHits++;
	fHitTime=0;
	

	/*if (iNumHits>2)// && (GetBody()->logic->GetHP() < GetBody()->logic->GetMaxHP()*0.4f))
	{
		if (GetBody()->logic->PairModeAllowed(false,true))
		{		
			Reset();
		
			BrainState=EBS_IDLE;

			GetBody()->animation->ActivateLink("to stun",true);
			return;
		}		
	}*/

	if (BrainState!=EBS_KICK)
	{				
		Reset();		

		if (_offender!=NULL)
		{
			if (GetBody()->logic->IsEnemy(_offender))
			{			
				pChrTarget=_offender;
				float fDist=(GetBody()->physics->GetPos()-pChrTarget->physics->GetPos()).GetLengthXZ();

				if ((GetBody()->physics->GetPos()-pChrTarget->physics->GetPos()).GetLengthXZ()<=GetAttackDistance())
				{
					pThoughtKick->Activate(pChrTarget);
					BrainState=EBS_KICK;				
				}
			}
		}
	}
}

void aiChainman::Death()
{
	DeattachFromCP();
}

void aiChainman::SplashDamage()
{	
	Vector delta = Vector (0,0,1);

	delta.Rotate(GetBody()->physics->GetAy());

	delta.x *= 1.7f;
	delta.z *= 1.7f;

	GetBody()->arbiter->SplashDamage(DamageReceiver::ds_bomb, GetBody(), GetBody()->physics->GetPos() + delta, 2.55f, 20.0f,false,"GroundStrike",true);
}

DeclareCharacterController(aiChainman, "Chainman", 11)