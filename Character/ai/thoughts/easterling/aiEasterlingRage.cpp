#include "aiEasterlingRage.h"
#include "../../Brains/aiBrain.h"
#include "../../../character/components/CharacterPhysics.h"
#include "../../../character/components/CharacterLogic.h"


aiEasterlingRage::aiEasterlingRage(aiBrain* Brain) : aiThought (Brain)
{	
}

aiEasterlingRage::~aiEasterlingRage()
{
}

void aiEasterlingRage::Activate(Character* pChr,float _fRageTime, float rotTime)
{
	pTarget = pChr;

	fRageRotTime = rotTime;
	bHasKick = false;	

	fRageTime = _fRageTime;
	bMaxRageTime = _fRageTime;


	bStopped = false;

	CharacterLogic::State st = GetBrain()->GetBody()->logic->GetState();

	aiThought::Activate();
}

bool aiEasterlingRage::Process (float fDeltaTime)
{
	if (bStopped) return false;
		
	Vector vTrgtPos = pTarget->physics->GetPos();
	Vector vMyPos = GetBrain()->GetBody()->physics->GetPos();	

	if (bHasKick && (bMaxRageTime - fRageTime)<fRageRotTime)
	{	
		GetBrain()->GetBody()->physics->Orient(vTrgtPos,true);		
	}

	CharacterLogic::State st = GetBrain()->GetBody()->logic->GetState();

	//if (st == CharacterLogic::state_attack)
	//{
	//	bHasKick = true;
	//}


	if (st != CharacterLogic::state_attack && !bHasKick)
	{
		bHasKick = GetBrain()->GetBody()->animation->ActivateLink("Rage",true);		
	}
	
	if (st == CharacterLogic::state_attack && bHasKick)
	{
		fRageTime -= fDeltaTime;

		if (fRageTime < 0) return false;
	}

		

	return true;
}

void aiEasterlingRage::DebugDraw (const Vector& BodyPos, IRender* pRS)
{
	//pRS->Print(BodyPos + Vector (0.0f, 1.9f, 0.0f), 10000.0f, 0.0f, 0xFFFFFFFF, "wait");
}

void aiEasterlingRage::Stop ()
{
	bool bRes = GetBrain()->GetBody()->animation->ActivateLink("idle");
}