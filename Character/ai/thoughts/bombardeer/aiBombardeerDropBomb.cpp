#include "aiBombardeerDropBomb.h"
#include "../../Brains/aiBrain.h"
#include "../../../character/components/CharacterPhysics.h"
#include "../../../character/components/CharacterLogic.h"


aiBombardeerDropBomb::aiBombardeerDropBomb(aiBrain* Brain) : aiThought (Brain)
{	
}

aiBombardeerDropBomb::~aiBombardeerDropBomb()
{
}

 void aiBombardeerDropBomb::Activate(Character* pChr,
									 float fShootNearDistance, float fShootFarDistance,
									 float fMinIdle, float fMaxIdle, bool only_once)
{
	pTarget = pChr;
	fNearDistance = fShootNearDistance;
	fFarDistance = fShootFarDistance;
	fIdleTime = 0.0f;

	fMinTimeToShoot=fMinIdle;
	fMaxTimeToShoot=fMaxIdle;

	one_drop = only_once;
	bomb_was_droped = false;

	fTimeToNextAttack = fMinTimeToShoot+Rnd(fMaxTimeToShoot-fMinTimeToShoot);

	fTimeToNextAttack *=0.15f;

	vLastTrgPos = pTarget->physics->GetPos();

	//pTarget->controller->SetNumAtackers(pTarget->controller->GetNumAtackers()+1);

	aiThought::Activate();
}

bool aiBombardeerDropBomb::Process (float fDeltaTime)
{
	Vector vTrgtPos = pTarget->physics->GetPos();
	Vector vMyPos = GetBrain()->GetBody()->physics->GetPos();	

	//if ((vTrgtPos-vMyPos).GetLength() > fDistance)
	//{
	//	return false;
	//}

	GetBrain()->GetBody()->physics->Orient(vTrgtPos);

	CharacterLogic::State st = GetBrain()->GetBody()->logic->GetState();

	if (st != CharacterLogic::state_dropbomb && (fIdleTime > fTimeToNextAttack || (!one_drop && bomb_was_droped)))
	{		
		if ((vTrgtPos-vMyPos).GetLength() >=fFarDistance ||
			(vTrgtPos-vMyPos).GetLength() <=fNearDistance)
		{
			return false;
		}

		if (!GetBrain()->GetBody()->arbiter->IsSphereInFrustrum(GetBrain()->GetBody()->physics->GetPos(), 0.1f))
		{
			fIdleTime = 0;
			return false;
		}
		else
		if (!GetBrain()->GetBody()->IsBombBusy())
		{		
			Vector vTrgPos = pTarget->physics->GetPos();

			CharacterLogic::State trg_st = pTarget->logic->GetState();

			float fDist = (vLastTrgPos-vTrgPos).GetLength() / fDeltaTime;

			GetBrain()->GetBody()->animation->ActivateLink("Drop Bomb");		

			Matrix mtx;
			GetBrain()->GetBody()->SetBombTarget(pTarget->GetMatrix(mtx).pos);

			fTimeToNextAttack = fMinTimeToShoot+Rnd(fMaxTimeToShoot-fMinTimeToShoot);

			fIdleTime = 0;

			bomb_was_droped = true;
		}
	}
	else
	{		
		if (st != CharacterLogic::state_dropbomb)
		{
			if (bomb_was_droped && one_drop)
			{
				return false;
			}
			
			if ((vTrgtPos-vMyPos).GetLength() >=fFarDistance ||
				(vTrgtPos-vMyPos).GetLength() <=fNearDistance)
			{
				return false;
			}

			fIdleTime += fDeltaTime;
		}
	}

	vLastTrgPos = pTarget->physics->GetPos();

	return true;
}

void aiBombardeerDropBomb::Stop ()
{
	bool bRes = GetBrain()->GetBody()->animation->ActivateLink("idle");
	//pTarget->controller->SetNumAtackers(pTarget->controller->GetNumAtackers()-1);
}

void aiBombardeerDropBomb::Hit ()
{
	//Отвечать, если бьют...
	//fTimeToNextAttack = 0.05f;
}