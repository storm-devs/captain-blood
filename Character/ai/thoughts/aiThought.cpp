
#include "aiThought.h"
#include "..\Brains\aiBrain.h"

void aiThought::Activate()
{
	pBrain->ActivateThought(this);
}