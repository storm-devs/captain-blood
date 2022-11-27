#include "aiPittRoll.h"

#include "../../Brains/aiBrain.h"
#include "../../../character/components/CharacterPhysics.h"
#include "../../../character/components/CharacterLogic.h"

aiPittRoll::aiPittRoll(aiBrain* Brain) : aiThought (Brain)
{	
}

aiPittRoll::~aiPittRoll()
{
}

void aiPittRoll::Activate(Character* pChr, float fWaitTime)
{
	fTime = 0.0f;
	fMaxWaitTime = fWaitTime;

	pTarget=pChr;

	//GetBrain()->GetBody()->logic->ActivateLink("Wait Attack");

	bAllowStepBack=true;

	aiThought::Activate();
}

bool aiPittRoll::Process (float fDeltaTime)
{
	Vector vDist;

	/*if (pTarget!=NULL)
	{	
		Vector vTrgtPos = pTarget->physics->GetPos();
		Vector vMyPos = GetBrain()->GetBody()->physics->GetPos();	

		vDist = vTrgtPos - vMyPos;

		if (GetBrain()->GetBody()->logic->GetState()!=CharacterLogic::state_hit)
		{	
			GetBrain()->GetBody()->physics->Orient(vTrgtPos);
		}
	}
	else
	{	
		Vector vTrgtPos = vTaraget;
		Vector vMyPos = GetBrain()->GetBody()->physics->GetPos();	

		vDist = vTrgtPos - vMyPos;

		if (GetBrain()->GetBody()->logic->GetState()!=CharacterLogic::state_hit)
		{	
			GetBrain()->GetBody()->physics->Orient(vTrgtPos);
		}
	}*/	

		
	fTime += fDeltaTime;

	if (fTime >= fMaxWaitTime)
	{
		bool bRes = GetBrain()->GetBody()->animation->ActivateLink("idle");

		return false;
	}

	//if (GetBrain()->GetBody()->logic->GetState()==CharacterLogic::state_roll||
	//	GetBrain()->GetBody()->logic->GetState()==CharacterLogic::state_attack) return true;	
	
	
	if (GetBrain()->GetBody()->logic->GetState()!=CharacterLogic::state_roll)
	{		
		bool bAllowedDirections[3];

		{
			GetBrain()->SectorSearch(GetBrain()->GetBody()->physics->GetPos(),
									 GetBrain()->GetBody()->physics->GetAy()+PI/2, 0.75f, 1.0f,
									 -15*(PI/180), 15*(PI/180), MG_CHARACTER);			

			if (GetBrain()->searchResult.Size()==0)
			{
				bAllowedDirections[0]=true;
			}
			else
			{
				bAllowedDirections[0]=false;
			}

			GetBrain()->SectorSearch(GetBrain()->GetBody()->physics->GetPos(),
									 GetBrain()->GetBody()->physics->GetAy()-PI/2, 0.75f, 1.0f,
									 -15*(PI/180), 15*(PI/180), MG_CHARACTER);		
            				
			if (GetBrain()->searchResult.Size()==0)
			{
				bAllowedDirections[1]=true;
			}			
			else
			{
				bAllowedDirections[1]=false;
			}

			GetBrain()->SectorSearch(GetBrain()->GetBody()->physics->GetPos(),
									 GetBrain()->GetBody()->physics->GetAy()+PI, 0.75f, 1.0f,
								     -15*(PI/180), 15*(PI/180), MG_CHARACTER);			
			
			if (GetBrain()->searchResult.Size()==0)
			{
				bAllowedDirections[2]=true;
			}
			else
			{
				bAllowedDirections[2]=false;
			}            
		}

		if (bAllowedDirections[0]&&bAllowedDirections[1]&&bAllowedDirections[2])
		{
			float fChoose=Rnd() * 5.0f;		
				
			if (fChoose>3.25f)
			{						
				GetBrain()->GetBody()->animation->ActivateLink("strafe right",true);
				if (iNumBackSteps>0) iNumBackSteps--;
			}		
			else
			if (fChoose>1.5f)
			{			
				GetBrain()->GetBody()->animation->ActivateLink("strafe left",true);
				if (iNumBackSteps>0) iNumBackSteps--;
			}		
			else
			{
				if (vDist.GetLength()<4 && iNumBackSteps<1&&bAllowStepBack)
				{
					GetBrain()->GetBody()->animation->ActivateLink("strafe back",true);
					iNumBackSteps++;
				}
				else
				{
					GetBrain()->GetBody()->animation->ActivateLink("strafe left",true);
				}
			}
		}
		else
		{
			if (bAllowedDirections[0])
			{
				GetBrain()->GetBody()->animation->ActivateLink("strafe right",true);
				if (iNumBackSteps>0) iNumBackSteps--;
			}
			else
			if (bAllowedDirections[1])
			{
				GetBrain()->GetBody()->animation->ActivateLink("strafe left",true);
				if (iNumBackSteps>0) iNumBackSteps--;
			}
			else
			if (bAllowedDirections[2]&&bAllowStepBack)
			{
				if (vDist.GetLength()<4 && iNumBackSteps<1)
				{
					GetBrain()->GetBody()->animation->ActivateLink("strafe back",true);
					iNumBackSteps++;
				}
				else
				{
					GetBrain()->GetBody()->animation->ActivateLink("strafe rigth",true);
				}
			}
		}
	}	
	
	return true;
}

void aiPittRoll::Stop ()
{
	bool bRes = GetBrain()->GetBody()->animation->ActivateLink("idle");
}