#include "aiStrafing.h"

#include "../Brains/aiBrain.h"
#include "../../character/components/CharacterPhysics.h"
#include "../../character/components/CharacterLogic.h"


aiThoughtStrafing::aiThoughtStrafing(aiBrain* Brain) : aiThought (Brain)
{
}

void aiThoughtStrafing::Activate(Character* pChr, float fWaitTime)
{
	fTime = 0.0f;
	fMaxWaitTime = fWaitTime * GetBrain()->GetStrafeModifier();

	pTarget=pChr;

	GetBrain()->GetBody()->animation->ActivateLink("Wait Attack");

	bAllowStepBack=true;

	aiThought::Activate();
}

void aiThoughtStrafing::Activate(Vector _vTaraget, float fWaitTime)
{
	fTime = 0.0f;
	fMaxWaitTime = fWaitTime * GetBrain()->GetStrafeModifier();
	
	pTarget=NULL;

	GetBrain()->GetBody()->animation->ActivateLink("Wait Attack",true);

	vTaraget = _vTaraget;

	iNumBackSteps=0;

	bAllowStepBack=false;

	aiThought::Activate();
}


bool aiThoughtStrafing::Process (float fDeltaTime)
{		
	Vector vDist;

	if (pTarget!=NULL)
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
	}	

		
	fTime += fDeltaTime;

	if (fTime >= fMaxWaitTime && GetBrain()->GetBody()->logic->GetState()!=CharacterLogic::state_strafe)
	{
		bool bRes = GetBrain()->GetBody()->animation->ActivateLink("idle");

		return false;
	}

	fStrafeTime -= fDeltaTime;

	if (fStrafeTime<0.0f && GetBrain()->GetBody()->logic->GetState()!=CharacterLogic::state_strafe)
	{
		GetBrain()->GetBody()->animation->ActivateLink("Wait Attack");
		fStrafeTime = 0.0f;
	}

	if (GetBrain()->GetBody()->logic->GetState()==CharacterLogic::state_strafe||
		GetBrain()->GetBody()->logic->GetState()==CharacterLogic::state_attack) return true;	
	
	
	if (GetBrain()->GetBody()->logic->GetState() == CharacterLogic::state_idle)
	{		
		bool bAllowedDirections[3];

		Character & chr = *GetBrain()->GetBody();

		Matrix mtx;
		const Vector & pos = chr.physics->GetPos();
		float ay =  chr.physics->GetAy();
		chr.GetMatrix(mtx);

		{
			GetBrain()->SectorSearch(pos, ay + PI/2, 0.75f, 1.0f, -15*(PI/180), 15*(PI/180), MG_CHARACTER);			
			bAllowedDirections[0] = GetBrain()->searchResult.Size() == 0;

			GetBrain()->SectorSearch(pos, ay - PI/2, 0.75f, 1.0f, -15*(PI/180), 15*(PI/180), MG_CHARACTER);		
			bAllowedDirections[1] = GetBrain()->searchResult.Size() == 0;

			GetBrain()->SectorSearch(pos, ay + PI, 0.75f, 1.0f, -15*(PI/180), 15*(PI/180), MG_CHARACTER);			
			bAllowedDirections[2] = GetBrain()->searchResult.Size() == 0;
		
			const Vector & vp = Vector(0.0f, 1.6f, 0.0f);
			const Vector & vm = Vector(0.0f, -5.0f, 0.0f);

			IPhysicsScene::RaycastResult detail;

			const static float max_delta_y = 2.0f;
			const static float trace_dist_side = 2.0f;
			const static float trace_dist_back = 2.5f;
			// right
			if (bAllowedDirections[0])
			{
				Vector vrt = mtx.MulVertex(Vector(trace_dist_side, 0.0f, 0.0f));
				IPhysBase * pb = chr.Physics().Raycast(vrt + vp, vrt + vm, phys_mask(phys_character), &detail);
				if (!pb || vrt.y - detail.position.y > max_delta_y)
					bAllowedDirections[0] = false;
			}

			// left
			if (bAllowedDirections[1])
			{
				Vector vrt = mtx.MulVertex(Vector(-trace_dist_side, 0.0f, 0.0f));
				IPhysBase * pb = chr.Physics().Raycast(vrt + vp, vrt + vm, phys_mask(phys_character), &detail);
				if (!pb || vrt.y - detail.position.y > max_delta_y)
					bAllowedDirections[1] = false;
			}

			// back
			if (bAllowedDirections[2])
			{
				Vector vrt = mtx.MulVertex(Vector(0.0f, 0.0f, -trace_dist_back));
				IPhysBase * pb = chr.Physics().Raycast(vrt + vp, vrt + vm, phys_mask(phys_character), &detail);
				if (!pb || vrt.y - detail.position.y > max_delta_y)
					bAllowedDirections[2] = false;
			}
		}

		fStrafeTime = Rnd()+1.0f;

		if (vDist.GetLength2() < Sqr(2.5f) && bAllowedDirections[2])
		{
			chr.animation->ActivateLink("Step Backward");
			iNumBackSteps++;

			fStrafeTime = 0.75f;
		}
		else
		if (bAllowedDirections[0]&&bAllowedDirections[1]&&bAllowedDirections[2])
		{
			float fChoose=Rnd() * 5.0f;		
				
			if (fChoose>3.25f)
			{						
				chr.animation->ActivateLink("Step Right");
				if (iNumBackSteps>0) iNumBackSteps--;
			}		
			else
			if (fChoose>1.5f)
			{			
				chr.animation->ActivateLink("Step Left");
				if (iNumBackSteps>0) iNumBackSteps--;
			}		
			else
			{
				if (vDist.GetLength2() < Sqr(4.0f) && iNumBackSteps<1&&bAllowStepBack)
				{
					if (vDist.GetLength2() < Sqr(1.8f) && bAllowedDirections[2])
					{
						chr.animation->ActivateLink("Step Backward");
						iNumBackSteps++;
					}
					else
					{
						chr.animation->ActivateLink("Step Left");
					}
				}
				else
				{
					chr.animation->ActivateLink("Step Left");
				}
			}
		}
		else
		{
			if (bAllowedDirections[0])
			{
				chr.animation->ActivateLink("Step Right");
				if (iNumBackSteps>0) iNumBackSteps--;
			}
			else
			if (bAllowedDirections[1])
			{
				chr.animation->ActivateLink("Step Left");
				if (iNumBackSteps>0) iNumBackSteps--;
			}
			else
			if (bAllowedDirections[2]&&bAllowStepBack)
			{
				if (vDist.GetLength2() < Sqr(4.0f) && iNumBackSteps<1)
				{
					if (vDist.GetLength2() < Sqr(1.8f) && bAllowedDirections[2])
					{
						chr.animation->ActivateLink("Step Backward");
						iNumBackSteps++;
					}
					else
					{
						chr.animation->ActivateLink("Step Rigth");
					}
				}
				else
				{
					chr.animation->ActivateLink("Step Rigth");
				}
			}
		}		
	}	
	
	return true;
}

void aiThoughtStrafing::DebugDraw (const Vector& BodyPos, IRender* pRS)
{
	//pRS->Print(BodyPos + Vector (0.0f, 1.9f, 0.0f), 10000.0f, 0.0f, 0xFFFFFFFF, "wait");
}

void aiThoughtStrafing::Stop ()
{
	bool bRes = GetBrain()->GetBody()->animation->ActivateLink("idle");
}