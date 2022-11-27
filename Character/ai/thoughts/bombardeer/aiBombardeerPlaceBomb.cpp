#include "aiBombardeerPlaceBomb.h"

#include "../../Brains/aiBrain.h"
#include "../../../character/components/CharacterPhysics.h"
#include "../../../character/components/CharacterLogic.h"

aiBombardeerPlaceBomb::aiBombardeerPlaceBomb(aiBrain* Brain) : aiThought (Brain)
{	
}

aiBombardeerPlaceBomb::~aiBombardeerPlaceBomb()
{
}

void aiBombardeerPlaceBomb::Activate()
{
	bStartPlaceBomb = false;

	aiThought::Activate();
}

bool aiBombardeerPlaceBomb::Process (float fDeltaTime)
{	
	CharacterLogic::State st = GetBrain()->GetBody()->logic->GetState();

	if (bStartPlaceBomb && st != CharacterLogic::state_dropbomb)
	{
		if (!GetBrain()->GetBody()->IsBombBusy())
		{
			GetBrain()->GetBody()->SetBombTarget(GetBrain()->GetBody()->physics->GetPos());
			GetBrain()->GetBody()->DropBomb(0.75f);
		}

		return false;
	}

	if (!bStartPlaceBomb && st != CharacterLogic::state_dropbomb)
	{
		bStartPlaceBomb = GetBrain()->GetBody()->animation->ActivateLink("Place Bomb");
	}
	
		
	
		
	return true;
}

void aiBombardeerPlaceBomb::Stop ()
{
	bool bRes = GetBrain()->GetBody()->animation->ActivateLink("idle");
}