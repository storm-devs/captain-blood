#include "aiOfficerShootShoot.h"
#include "../../Brains/aiBrain.h"
#include "../../../character/components/CharacterPhysics.h"
#include "../../../character/components/CharacterLogic.h"


aiOfficerShootShoot::aiOfficerShootShoot(aiBrain* Brain) : aiThought (Brain)
{	
}

aiOfficerShootShoot::~aiOfficerShootShoot()
{
}

void aiOfficerShootShoot::Activate(Character* pChr,
								   float fShootNearDistance, float fShootFarDistance,
								   float fMinIdle, float fMaxIdle)
{
	if (!pChr) return;

	pTarget = pChr;
	fNearDistance = fShootNearDistance;
	fFarDistance = fShootFarDistance;	

	fMinTimeToShoot=fMinIdle;
	fMaxTimeToShoot=fMaxIdle;	

	fIdleTime = fMinTimeToShoot+Rnd(fMaxTimeToShoot-fMinTimeToShoot);

	stoped = false;
	started = false;

	aiThought::Activate();
}

bool aiOfficerShootShoot::Process (float fDeltaTime)
{		
	if (!started)
	{
		started = GetBrain()->GetBody()->animation->ActivateLink("Shoot");

		if (!started) return true;
	}

	if (stoped) return false;	

	GetBrain()->OrinentToTarget();		

	CharacterLogic::State st = GetBrain()->GetBody()->logic->GetState();


	if (st == CharacterLogic::state_attack)
	{
		if (GetBrain()->GetPowDistToTarget() >=fFarDistance * fFarDistance ||
			GetBrain()->GetPowDistToTarget() <=fNearDistance * fNearDistance)
		{
			return false;
		}	

		GetBrain()->OrinentToTarget();		

		fIdleTime -= fDeltaTime;

		if (fIdleTime<0.0f)
		{
			fIdleTime = fMinTimeToShoot+Rnd(fMaxTimeToShoot-fMinTimeToShoot);							
			GetBrain()->GetBody()->animation->ActivateLink("Shoot");			
		}
	}

	return true;
}

void aiOfficerShootShoot::Stop ()
{			
	GetBrain()->GetBody()->animation->ActivateLink("end shoot");
}

void aiOfficerShootShoot::Hit()
{
	stoped = true;
}