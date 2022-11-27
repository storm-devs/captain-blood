#include "aiExecutorPray.h"
#include "../../Brains/aiBrain.h"
#include "../../../character/components/CharacterPhysics.h"
#include "../../../character/components/CharacterLogic.h"


aiExecutorPray::aiExecutorPray(aiBrain* Brain) : aiThought (Brain)
{

}

void aiExecutorPray::Activate(Character* pChr,bool allowFatality,float HealingRate,float HPLimit, int iNumHelpers,const ConstString * cHelpers)
{
	fIdleTime = 15.0f;	

	fHPLimit = HPLimit;

	GetBrain()->GetBody()->physics->Activate(false);
	GetBrain()->GetBody()->animation->ActivateLink("climb",true);
	GetBrain()->GetBody()->physics->Orient(GetBrain()->GetBody()->physics->GetPos() + Vector(0,0,1));	

	bAllowFatality = allowFatality;
	bRegeneration = true;
	
	fHealingRate = HealingRate;

	for (int i=0;i<5;i++)
	{
		pHelpers[i] = NULL;
	}

	bHelpersAlive = false;
	
	MOSafePointerTypeEx<Character> trgt;

	for (int i=0;i<iNumHelpers;i++)
	{				
		static const ConstString strTypeId("Character");
		trgt.FindObject(&GetBrain()->GetBody()->Mission(),cHelpers[i],strTypeId);
		pHelpers[i] = trgt.Ptr();

		if (pHelpers[i])
		{
			bHelpersAlive = true;			
		}
	}	

	aiThought::Activate();
}

bool aiExecutorPray::Process (float fDeltaTime)
{	
	CharacterLogic::State st = GetBrain()->GetBody()->logic->GetState();

	if (st == CharacterLogic::state_pair)
	{
		return false;
	}
	
	GetBrain()->GetBody()->PushCharcters(fDeltaTime,2.25f,true,false);
	GetBrain()->GetBody()->physics->Orient(GetBrain()->GetBody()->physics->GetPos() + Vector(0,0,1));
	

	if (fIdleTime<7.0f  && bRegeneration)	
	{
		GetBrain()->GetBody()->logic->SetHP(GetBrain()->GetBody()->logic->GetHP()+fDeltaTime*fHealingRate);

		if (GetBrain()->GetBody()->logic->GetHP()>GetBrain()->GetBody()->logic->GetMaxHP()*fHPLimit)
		{
			GetBrain()->GetBody()->logic->SetHP(GetBrain()->GetBody()->logic->GetMaxHP()*fHPLimit);
			bRegeneration = false;
		}
	}

	if (bHelpersAlive)
	{
		float HP = 0.0f;

		for (int i=0;i<5;i++)
		{
			if (pHelpers[i])
			{
				HP += pHelpers[i]->GetHP();

			}			
		}		

		if (HP<0.5f)
		{
			if (!bAllowFatality)
			{
				fIdleTime = 0.0f;
			}
			else
			{
				fIdleTime = 6.0f;
			}

			bHelpersAlive = false;			
		}
	}
	
	if (fIdleTime<=0.0f && !bRegeneration)//*/ && !bHelpersAlive)
	{		
		GetBrain()->GetBody()->SetCharacterCoin(Character::chrcoin_none);
		GetBrain()->GetBody()->animation->ActivateLink("jump down",true);		

		if (st != CharacterLogic::state_imbo)
		{
			 return false;
		}
	}
	else
	{
		if (!bHelpersAlive && bAllowFatality)
		{
			GetBrain()->GetBody()->SetCharacterCoin(Character::chrcoin_fatality);
		}
		else
		{
			GetBrain()->GetBody()->SetCharacterCoin(Character::chrcoin_none);
		}

		fIdleTime -= fDeltaTime;

		if (fIdleTime<0.0f) fIdleTime = 0.0f;
	}	

	return true;	
}

void aiExecutorPray::Stop ()
{	
	CharacterLogic::State st = GetBrain()->GetBody()->logic->GetState();
	if (st == CharacterLogic::state_pair) return;

	GetBrain()->GetBody()->physics->Activate(true);
}