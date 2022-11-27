#include "aiWarrior.h"
#include "aiBrain.h"
#include "../CharacterAI.h"
#include "../../character/components/CharacterLogic.h"
#include "../../character/components/CharacterPhysics.h"

aiWarrior::aiWarrior(Character & ch, const char* szName) : aiBrain (ch, szName)
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
		
	pThoughtStrafing = (aiThoughtStrafing*)AddThought(NEW aiThoughtStrafing(this));		
	pThoughtBlocking = (aiThoughtBlocking*)AddThought(NEW aiThoughtBlocking(this));	

	fTime_to_return_to_spawn = 0.0f;
		
	pThoughtKick->AddAction("Attack1");
	pThoughtKick->AddAction("Attack2");
	pThoughtKick->AddAction("Attack3");	
	pThoughtKick->AddAction("Combo1");
	pThoughtKick->AddAction("Combo2");	
	pThoughtKick->PrepareActions();

	pThoughtStanding->AddAction("Angre");
	pThoughtStanding->AddAction("Klich");
	pThoughtStanding->AddAction("Pot");
	pThoughtStanding->AddAction("weakless1");
	pThoughtStanding->AddAction("weakless2");	
	pThoughtStanding->PrepareActions();	
}

void aiWarrior::ActivateTargetPairStateThought()
{
	pThoughtStrafing->Activate( (Character*)pChrTarget, 15.0f);
	BrainState=EBS_WAIT_FATALITY;
}

void aiWarrior::Update(float dltTime)
{	
	aiBrain::Update(dltTime);	
}

//Обновить состояние контролера 10 раз в секунду (распределено между персонажами)
void aiWarrior::StatistTick()
{
	aiBrain::CheckTargetState();
	CheckTargetPairState();

	if (BrainState==EBS_WAIT_FATALITY) return;	

	if (IsEmpty())
	{		
		if (BrainState==EBS_KICK)
		{
			if (pChrTarget!=NULL)
			{
				if (pChrTarget==pChrPlayer)/// || IsStatist())
				{					
					pThoughtStrafing->Activate( pChrTarget, 2+Rnd(4));	


					BrainState=EBS_STRAFING;		
				}
				else
				{
					if (GetPowDistToTarget()<=GetAttackDistancePow())
					{
						if (!CheckAttackerHeight())
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
		{
			BrainState=EBS_IDLE;
		}
	}

	if (iCurGr_CPoint!=-1)
	{		
		if (!pCPoints[iCurGr_CPoint]->IsActive())
		{
			DeattachFromCP();
		}
	}

	if (BrainState==EBS_STANDING && iCurGr_CPoint!=-1)
	{		
		//if (IsStatist())
		{
			pChrTarget=SearchTarget(true,-1.0f,GetBody()->arbiter->IsEnemyStatistAllive(GetBody()));

			//if (!pChrTarget) pChrTarget=SearchTarget(true,false);
		}
		//else
		//{
		//	pChrTarget=SearchTarget(true,false);
		//}

		if (pChrTarget!=NULL)
		{	
			Reset();

			BrainState=EBS_IDLE;
		}
	}	

	if (BrainState==EBS_IDLE)
	{
		//if (bIsStatist)
		{
			if (pChrTarget==NULL)
			{
				CPAction action = FindCPoint();

				if (action == CPA_STANDING)
				{
					Character* target = null;
					if (searchResult.Size()>0) target = searchResult[0].chr;

					pThoughtStanding->Activate( Rnd(2)+2, target );
					BrainState=EBS_STANDING;			
				}
				else
				if (action == CPA_PURSUIT)
				{
					pThoughtPursuit->Activate(pCPoints[iCurGr_CPoint]->Points[iCurPt_CPoint].position, WayPointsName,
											  pCPoints[iCurGr_CPoint]->Points[iCurPt_CPoint].radius * 0.75f, true);
					BrainState=EBS_GO_TO_CPOINT;			
				}
				else
				{
					pChrTarget=SearchTarget(true,-1.0f,GetBody()->arbiter->IsEnemyStatistAllive(GetBody()));

					if (!pChrTarget)
					{
						Character* target = null;
						if (searchResult.Size()>0) target = searchResult[0].chr;

						pThoughtStanding->Activate( Rnd(2)+2, target );
						BrainState=EBS_STANDING;			
					}
				}				
			}			
		}
		/*else
		{
			if (pChrTarget==NULL) pChrTarget=SearchTarget(true,bIsStatist);

			if (pChrTarget==NULL)
			{
				CPAction action = FindCPoint();

				if (action == CPA_STANDING)
				{				
					pThoughtStanding->Activate( Rnd(2)+2, null );
					BrainState=EBS_STANDING;			
				}
				else
					if (action == CPA_PURSUIT)
					{
						pThoughtPursuit->Activate(pCPoints[iCurGr_CPoint]->Points[iCurPt_CPoint].position, WayPointsName,
							pCPoints[iCurGr_CPoint]->Points[iCurPt_CPoint].radius * 0.75f);
						BrainState=EBS_GO_TO_CPOINT;			
					}
			}
		}*/	

		if (pChrTarget==NULL&&searchResult.Size()>0 && BrainState!=EBS_GO_TO_CPOINT && BrainState!=EBS_STANDING)
		{
			DeattachFromCP();

			pThoughtStrafing->Activate( (Character*)searchResult[0].chr, Rnd(2)+2);
			BrainState=EBS_STRAFING;		
		}
		else
			if (pChrTarget!=NULL && BrainState!=EBS_GO_TO_CPOINT )
			{
				DeattachFromCP();

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
			pThoughtPursuit->ChangeParams(pChrTarget->physics->GetPos(), GetAttackDistance() * 0.75f );			
		}
	}
	else
	if (BrainState==EBS_GO_TO_CPOINT)
	{
		if (!IsStatist() || (IsStatist() && !GetBody()->arbiter->IsEnemyStatistAllive(GetBody())))
		{
			if (SearchPlayer(true))
			{
				DeattachFromCP();
				AttackCharacter();
			}
		}
	}
}
/*
bool aiWarrior::CheckCharState(Character* target)
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

void aiWarrior::Tick()
{
	if (PreTick()) return;

	if (IsStatist())
	{
		StatistTick();

		return;
	}

	if (!BaseTick()) return;

	if (IsEmpty())
	{		
		if (BrainState==EBS_KICK)
		{
			pThoughtStrafing->Activate( pChrTarget, 2.0f + Rnd(4.0f));

			BrainState=EBS_STRAFING;		
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
	
	if (BrainState==EBS_IDLE || BrainState==EBS_STANDING)
	{
		if (pChrTarget==NULL)
		{
			pChrTarget=SearchTarget(true,-1.0f,bIsStatist);
		}

		CheckTargetState(false);
		
		if (pChrTarget!=NULL)
		{
			AttackCharacter();
			return;		
		}
		else
		{
			if (BrainState!=EBS_STANDING)
			{
				if ((GetBody()->logic->GetSpawnPoint()-GetBody()->physics->GetPos()).GetLength2() > Sqr(1.75f))
				{
					pThoughtPursuit->Activate(GetBody()->logic->GetSpawnPoint(), WayPointsName, 0.5f, true);
					pChrTarget = NULL;
					BrainState=EBS_RETURNTOSPAWN;	
				}
				else
				if (searchResult.Size()>0 && BrainState!=EBS_GO_TO_CPOINT)
				{
					Character* trg = (Character*)searchResult[0].chr;

					if (CheckDist(trg,10.0f))
					{
						pThoughtStrafing->Activate( trg, 4+Rnd(4));	
						BrainState=EBS_STRAFING;
					}
					else
					{
						pThoughtStanding->Activate( 4+Rnd(4), pChrPlayer);	
						BrainState=EBS_STANDING;
					}
				}
				else
				{
					pThoughtStanding->Activate( 4+Rnd(4), pChrPlayer);	
					BrainState=EBS_STANDING;
				}
			}
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
			pThoughtPursuit->ChangeParams(pChrTarget->physics->GetPos(), GetAttackDistance() * 0.75f );			
		}
	}	
	else
	if (BrainState==EBS_RETURNTOSPAWN)
	{
		if (SearchPlayer(true))
		{			
			if (!InAIZone(pChrTarget->physics->GetPos()))
			{
				pChrTarget = NULL;
			}

			if (pChrTarget)
			{
				DeattachFromCP();
				AttackCharacter();
			}
		}		
	}
}

void aiWarrior::Hit(float dmg, Character* _offender, const char* reaction, DamageReceiver::DamageSource source)
{		
	aiBrain::Hit(dmg, _offender, reaction, source);

	if (Target.NotEmpty()) return;

	if (pChrTarget && pChrPlayer != _offender) return;
	
	iNumHits++;
	fHitTime=0;

	bool blocking = StartBlocking(_offender, source);

	if (!blocking && BrainState != EBS_KICK)
	{		
		if (_offender!=NULL)
		{
			if (IsStatist())
			{
				if (!_offender->IsPlayer())
				{
					return;
				}
			}
			else
			{
				if (_offender->controller->IsStatist()) return;
			}


			if (GetBody()->logic->IsEnemy(_offender))
			{			
				pChrTarget=_offender;

				float fDist=(GetBody()->physics->GetPos()-pChrTarget->physics->GetPos()).GetLengthXZ2();

				if (GetPowDistToTarget()<=GetAttackDistancePow())
				{
					DeattachFromCP();
					pThoughtKick->Activate(pChrTarget);
					BrainState=EBS_KICK;
				}			
			}
		}
	}
}

DeclareCharacterController(aiWarrior, "Warrior", 11)