#include "aiEasterlingDrink.h"

#include "../../Brains/aiBrain.h"
#include "../../../character/components/CharacterPhysics.h"
#include "../../../character/components/CharacterLogic.h"

aiEasterlingDrink::aiEasterlingDrink(aiBrain* Brain) : aiThought (Brain)
{	
}

aiEasterlingDrink::~aiEasterlingDrink()
{
}

void aiEasterlingDrink::Activate(float health)
{
	fDrinkHealth = health * 0.01f;
	bStartDrink = false;

	aiThought::Activate();
}

bool aiEasterlingDrink::Process (float fDeltaTime)
{	
	CharacterLogic::State st = GetBrain()->GetBody()->logic->GetState();

	if (bStartDrink && st != CharacterLogic::state_block)
	{
		GetBrain()->GetBody()->logic->SetHP(GetBrain()->GetBody()->logic->GetHP()+GetBrain()->GetBody()->logic->GetMaxHP() * fDrinkHealth);

		return false;
	}

	if (!bStartDrink && st != CharacterLogic::state_block)
	{
		bStartDrink = GetBrain()->GetBody()->animation->ActivateLink("Drink",true);
	}
	
				
	return true;
}

void aiEasterlingDrink::Stop ()
{
	bool bRes = GetBrain()->GetBody()->animation->ActivateLink("idle");
}