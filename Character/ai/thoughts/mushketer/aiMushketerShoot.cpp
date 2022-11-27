#include "aiMushketerShoot.h"
#include "../../Brains/aiBrain.h"
#include "../../../character/components/CharacterPhysics.h"
#include "../../../character/components/CharacterLogic.h"


aiMushketerShoot::aiMushketerShoot(aiBrain* Brain) : aiThought (Brain)
{	
	pTarget = NULL;
	needStrafe = false;
}

void aiMushketerShoot::Activate(Character* pChr,
								float fShootNearDistance, float fShootFarDistance,
								float fMinIdle, float fMaxIdle, float waittime,
								bool needstrafe)
{
	if (!pChr) return;

	needStrafe = needstrafe;

	pTarget = pChr;	

	fNearDistance = fShootNearDistance;
	fFarDistance = fShootFarDistance;

	fMinTimeToShoot=fMinIdle;
	fMaxTimeToShoot=fMaxIdle;

	fTimeToNextAttack = fMinTimeToShoot+Rnd(fMaxTimeToShoot-fMinTimeToShoot);
	fIdleTime = fTimeToNextAttack + 0.1f;

	vLastTrgPos = pTarget->physics->GetPos();

	fWaitTime = waittime;

	numstrafes = 0;

	aiThought::Activate();
}


bool aiMushketerShoot::Process (float fDeltaTime)
{
	if (fWaitTime > 0)
	{
		fWaitTime -= fDeltaTime;

		return true;		
	}

	Vector vTrgtPos = pTarget->physics->GetPos();
	Vector vMyPos = GetBrain()->GetBody()->physics->GetPos();	

	GetBrain()->OrinentToTarget();

	CharacterLogic::State st = GetBrain()->GetBody()->logic->GetState();

	if ((vTrgtPos-vMyPos).GetLength() >=fFarDistance ||
		(vTrgtPos-vMyPos).GetLength() <=fNearDistance)
	{
		return false;
	}

	if (st != CharacterLogic::state_shoot && st != CharacterLogic::state_strafe)
	{		
		if (fIdleTime > fTimeToNextAttack)
		{
			if (numstrafes==0)
			{						
				CharacterLogic::State trg_st = pTarget->logic->GetState();

				float fDist = (vLastTrgPos-vTrgtPos).GetLength() / fDeltaTime;

				GetBrain()->GetBody()->logic->SetShootTarget(pTarget,"Hit");

				bool shoot = true;
		
				if (pTarget->IsPlayer())
				{			
					if (!GetBrain()->GetBody()->arbiter->IsSphereInFrustrum(GetBrain()->GetBody()->physics->GetPos(), 0.5f))
					{
						GetBrain()->GetBody()->logic->SetShootTarget(NULL,"Hit");
						shoot = false;

						return false;
					}
				}

				if (shoot)
				{
					GetBrain()->GetBody()->animation->ActivateLink("Shoot1");
				}

				fTimeToNextAttack = fMinTimeToShoot+Rnd(fMaxTimeToShoot-fMinTimeToShoot);
				fIdleTime = 0;

				if (needStrafe)
				{
					numstrafes = (int)(Rnd() * 2.0f) + 1;
				}
			}
			else
			{
				if (Rnd()>0.5f)
				{
					GetBrain()->GetBody()->animation->ActivateLink("Step Left");
				}
				else
				if (Rnd()>0.5f)
				{
					GetBrain()->GetBody()->animation->ActivateLink("Step Right");
				}
				else
				{
					GetBrain()->GetBody()->animation->ActivateLink("Step Backward");
				}

				numstrafes--;
			}

			return true;
		}
		else
		{		
			fIdleTime += fDeltaTime;			
		}
	}

	vLastTrgPos = pTarget->physics->GetPos();

	return true;
}

void aiMushketerShoot::DebugDraw (const Vector& BodyPos, IRender* pRS)
{
	//pRS->Print(BodyPos + Vector (0.0f, 1.9f, 0.0f), 10000.0f, 0.0f, 0xFFFFFFFF, "wait");
}

void aiMushketerShoot::Stop ()
{
	bool bRes = GetBrain()->GetBody()->animation->ActivateLink("idle");
}

void aiMushketerShoot::Hit ()
{
	//Отвечать, если бьют...
	//fTimeToNextAttack = 0.05f;
}