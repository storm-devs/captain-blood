#include "aiBoatswainWirst.h"
#include "../../Brains/aiBrain.h"
#include "../../../character/components/CharacterPhysics.h"
#include "../../../character/components/CharacterLogic.h"


aiBoatswainWirst::aiBoatswainWirst(aiBrain* Brain) : aiThought (Brain)
{	
}

aiBoatswainWirst::~aiBoatswainWirst()
{
}

void aiBoatswainWirst::Activate(Character* pChr,float _fWirstTime)
{
	pTarget = pChr;

	bHasKick = false;	

	fWirstTime = _fWirstTime;


	bStopped = false;

	CharacterLogic::State st = GetBrain()->GetBody()->logic->GetState();

	aiThought::Activate();
}

bool aiBoatswainWirst::Process (float fDeltaTime)
{
	if (bStopped) return false;
		
	Vector vTrgtPos = pTarget->physics->GetPos();
	Vector vMyPos = GetBrain()->GetBody()->physics->GetPos();	

	if (bHasKick)
	{	
		GetBrain()->GetBody()->physics->Orient(vTrgtPos);		
	}

	CharacterLogic::State st = GetBrain()->GetBody()->logic->GetState();

	//if (st == CharacterLogic::state_attack)
	//{
	//	bHasKick = true;
	//}


	if (st != CharacterLogic::state_attack && !bHasKick)
	{
		bHasKick = GetBrain()->GetBody()->animation->ActivateLink("Attack3");		
	}
	
	if (st == CharacterLogic::state_attack && bHasKick)
	{
		fWirstTime -= fDeltaTime;

		if (fWirstTime < 0) return false;
	}

		

	return true;
}

void aiBoatswainWirst::DebugDraw (const Vector& BodyPos, IRender* pRS)
{
	//pRS->Print(BodyPos + Vector (0.0f, 1.9f, 0.0f), 10000.0f, 0.0f, 0xFFFFFFFF, "wait");
}

void aiBoatswainWirst::Stop ()
{
	bool bRes = GetBrain()->GetBody()->animation->ActivateLink("idle");
}