#include "aiFatGuy.h"
#include "aiBrain.h"
#include "..\CharacterAI.h"
#include "../../character/components/CharacterLogic.h"
#include "../../character/components/CharacterPhysics.h"

#include "aiParams.h"

aiFatGuy::aiFatGuy(Character & ch, const char* szName) : aiBrain (ch, szName)
{
	animListener.pFatGuy = this;
	chr.animation->SetEventHandler(&animListener, (AniEvent)(&AnimListener::SplashDamage), "AI Event");

	fDistofView = 7.0f;
	fDistofViewPlayer = 7.0f;

	iNumPlayerAttckers = 5;
	iNumNPCAttckers = 1;

	BrainState=EBS_IDLE;

	pChrTarget=NULL;

	bInCombatPoint = false;

	iCurPt_CPoint = -1;
	iCurGr_CPoint = -1;


	fDistToLoseTarget = 15.0f;	

	static const ConstString defWayPoints("WayPoints"); 
	WayPointsName=defWayPoints;	

	pThoughtBlocking = (aiFatGuyBlocking*)AddThought(NEW aiFatGuyBlocking(this));
	pThoughtStanding = (aiThoughtStanding*)AddThought(NEW aiThoughtStanding(this));	

	pThoughtKick->AddAction("Attack1");
	pThoughtKick->AddAction("Attack2");
	pThoughtKick->AddAction("Attack4");
	pThoughtKick->AddAction("Combo1");
	pThoughtKick->AddAction("Combo2");
	pThoughtKick->AddAction("Rage");	
	pThoughtKick->PrepareActions();


	pThoughtStanding->AddAction("Angre");
	pThoughtStanding->AddAction("Klich");
	pThoughtStanding->AddAction("Pot");
	pThoughtStanding->AddAction("weakless");	
	pThoughtStanding->PrepareActions();	
}

void aiFatGuy::ActivateTargetPairStateThought()
{
	pThoughtStrafing->Activate( (Character*)pChrTarget, 15.0f);
	BrainState=EBS_WAIT_FATALITY;
}
/*
bool aiFatGuy::CheckCharState(Character* target)
{
	if (!target) return false;

	if (InSafetyPoint(target->physics->GetPos()))
	{
		return false;
	}
	else
		if (!target->IsShow())
		{			
			return false;
		}
		else
			if (!target->IsActive())
			{
				return false;
			}
			else
				if (target->logic->IsDead() && IsEmpty())
				{
					return false;
				}
				else
					if (target->logic->GetHP()<0.1f)
					{
						return false;
					}
					else
						if (target->logic->IsActor())
						{
							return false;
						}	

						return true;
}*/

void aiFatGuy::TickWhenEmpty()
{
	if (BrainState==EBS_KICK)
	{
		if (pChrTarget!=NULL)
		{
			if (pChrTarget==pChrPlayer)
			{					
				pThoughtStrafing->Activate(pChrTarget, 2+Rnd(4));						
				BrainState=EBS_STRAFING;				
			}
			else
			{
				if (GetPowDistToTarget()<=GetAttackDistancePow())
				{
					Vector vPos = GetBody()->physics->GetPos();

					float dy=fabs(vPos.y - pChrTarget->physics->GetPos().y);

					if (dy>0.5f)
					{
						pChrTarget=NULL;

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
	if (BrainState==EBS_WAITPLAYER)
	{
		pThoughtPursuit->Activate(GetBody()->logic->GetSpawnPoint(), WayPointsName, 0.5f, true);
		pChrTarget = NULL;
		BrainState=EBS_RETURNTOSPAWN;	
	}
	else		
	{
		BrainState=EBS_IDLE;
	}
}

void aiFatGuy::Hit(float dmg, Character* _offender, const char* reaction, DamageReceiver::DamageSource source)
{
	aiBrain::Hit(dmg, _offender, reaction, source);

	if (pChrTarget && pChrPlayer != _offender) return;
		
	iNumHits++;
	fHitTime=0;

	bool blocking = StartBlocking(_offender, source);

	if (!blocking && BrainState!=EBS_KICK)
	{				
		if (_offender!=NULL)
		{
			if (!IsStatist())
			{
				if (_offender->controller->IsStatist()) return;
			}

			if (GetBody()->logic->IsEnemy(_offender))
			{			
				pChrTarget=_offender;				
	
				if (GetPowDistToTarget()<=GetAttackDistancePow())
				{	
					Reset();
					pThoughtKick->Activate(pChrTarget);
					BrainState=EBS_KICK;				
				}
			}
		}
	}
}

void aiFatGuy::NotifyAboutAttack(Character* attacker,float damage)
{
	if (BrainState!=EBS_BLOCKING && Rnd()<0.4f)
	{	
		//iNumHits++;
		//fHitTime=0;

		//if (iNumHits>3)
		{
			//pFatGuyBlocking->Activate(attacker,3);
			//BrainState=EBS_BLOCKING;
			//return;
		}
	}
}

void aiFatGuy::SplashDamage()
{
	Vector delta = Vector (0,0,1);

	delta.Rotate(GetBody()->physics->GetAy());

	delta.x *= 1.7f;
	delta.z *= 1.7f;

	GetBody()->arbiter->SplashDamage(DamageReceiver::ds_bomb, GetBody(), GetBody()->physics->GetPos() + delta, 2.55f, 20.0f,false,"GroundStrike",true);
}

DeclareCharacterController(aiFatGuy, "FatGuy", 11)