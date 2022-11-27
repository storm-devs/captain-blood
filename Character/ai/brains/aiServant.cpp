
#include "aiServant.h"
#include "../../character/components/CharacterLogic.h"
#include "../../character/components/CharacterPhysics.h"


aiServant::aiServant(Character & ch, const char* szName) : aiBombardeer (ch, szName)
{
	pThoughtKick->ResetStack();
	pThoughtKick->AddAction("Attack1");
	pThoughtKick->AddAction("Attack2");	
	pThoughtKick->AddAction("Attack3");	
	pThoughtKick->AddAction("Attack4");	
	pThoughtKick->PrepareActions();

	fWaitBombTime = -1.0f;
}

aiServant::~aiServant()
{
}

void aiServant::TickWhenEmpty()
{	
	/*if ((BrainState == EBS_KICK || BrainState==EBS_PLACEBOMB)&& pChrTarget)
	{
		/pThoughtKick->Activate(pChrTarget);
		/BrainState=EBS_KICK;		
	}
	else		*/
	{
		BrainState=EBS_IDLE;
	}
}

void aiServant::Update(float dltTime)
{
	if (BrainState!=EBS_DROPBOMB)
	{
		fWaitBombTime -= dltTime;

		if (fWaitBombTime<0.0f)
		{
			fWaitBombTime = -1.0f;
		}
	}
	
	aiBombardeer::Update(dltTime);
}

bool aiServant::AttackCharacter()
{
	if (AllowAttack())
	{
		float fDist=(GetBody()->physics->GetPos()-pChrTarget->physics->GetPos()).GetLengthXZ();

		if (fDist<fDistToShoot&&fWaitBombTime<=0.0f)
		{			
			if (Rnd()>0.5f)
			{
				pBombardeerDropBomb->Activate(pChrTarget,
											  0.1f, fDistToShoot,
											  fMinTimeToShoot,fMaxTimeToShoot,true);
				BrainState=EBS_DROPBOMB;
				ResetWaitBomb();
			}
			else
			{
				pThoughtPursuit->Activate(pChrTarget->physics->GetPos(), WayPointsName, GetAttackDistance() * 0.8f, true);
				BrainState=EBS_PURSUIT;	
			}
		}
		else
		if (fDist<=GetAttackDistance())
		{						
			pThoughtKick->Activate(pChrTarget);
			BrainState=EBS_KICK;			
		}		
		else
		{
			float dist = fDistToShoot * 0.8f;

			if (Rnd()>0.5f) dist = GetAttackDistance() * 0.8f;

			pThoughtPursuit->Activate(pChrTarget->physics->GetPos(), WayPointsName, dist, true);
			BrainState=EBS_PURSUIT;	
		}				
	}
	else
	{
		pThoughtStrafing->Activate(pChrTarget, Rnd(2)+2);	
		BrainState=EBS_STRAFING;		
	}

	return true;
}

void aiServant::Hit(float dmg,Character* _offender, const char* reaction, DamageReceiver::DamageSource source)
{
	if (chr.logic->GetHP()/chr.logic->GetMaxHP()<0.2f)
	{
		GetBody()->animation->ActivateLink("To stun");
	}

	aiBombardeer::Hit(dmg, _offender, reaction, source);
}

void aiServant::ResetWaitBomb()
{
	fWaitBombTime = fMinTimeToShoot+Rnd(fMaxTimeToShoot-fMinTimeToShoot);;	
}

DeclareCharacterController(aiServant, "Servant", 11)