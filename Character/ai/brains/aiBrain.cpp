#include "aiBrain.h"
#include "../../character/components/CharacterLogic.h"
#include "../../character/components/CharacterPhysics.h"
#include "aiParams.h"

aiBrain::aiBrain(Character & ch, const char* szName) : CharacterController(ch, szName), Thoughts (_FL_), searchResult(_FL_), pCPoints(_FL_), pSPoints(_FL_), stages(_FL_)
{
	pBody = &ch;

	pChrTarget=NULL;
	pChrPlayer = NULL;

	bAllowUpdateInPairModel = false;

	iActiveThought = -1;

	NeedToFindMO = true;	

	iCurPt_CPoint = -1;
	iCurGr_CPoint = -1;		
	bInCombatPoint = false;

	AIZone = NULL;

	static const ConstString defWayPoints("WayPoints");
	WayPointsName=defWayPoints;

	BrainState=EBS_IDLE;

	fDistofView = 7.0f;
	fDistofViewPlayer = 10.0f;
		
	fDistToLoseTarget = 15.0f;
	fDistToReconstructPath = 5.555f;

	fTimeToNextAttack2 = 5.0f;
	fCurTimeToNextAttack = 0.0f;

	iMaxSimelouslyAttackers = 1;

	iNumPlayerAttckers = 3;
	iNumNPCAttckers = 1;

	bIsStatist = false;

	iNumHits = 0;
	fHitTime = 0;

	bIsStatist = false;

	stopWhenDebug = false;

	fTime_to_return_to_spawn = 0.0f;

	time_to_alarm = 1.0f;

	patrol_point = 0;
	patrol_time_action = -1.0f;

	cur_stage = 0;

	block_cooldown = 0;
	block_timecooldown = 5.0f;

	brainAnimListener.brain = this;
	chr.animation->SetEventHandler(&brainAnimListener, (AniEvent)(&BrainAnimListener::ToPair), "AI Event");

	pThoughtPursuit =  (aiThoughtPursuit*)AddThought(NEW aiThoughtPursuit(this));
	pThoughtStrafing = (aiThoughtStrafing*)AddThought(NEW aiThoughtStrafing(this));	
	pThoughtStanding = (aiThoughtStanding*)AddThought(NEW aiThoughtStanding(this));
	pThoughtKick =     (aiThoughtKick*)AddThought(NEW aiThoughtKick(this));	
	pThoughtBlocking = null;

	enemyAttackSpeedModifier = GetBody()->arbiter->GetEnemyAttackSpeedModifier();
}

aiBrain::~aiBrain()
{
	for (int i = 0; i < (int)Thoughts.Size(); i++)
	{
		if (Thoughts[i].Task) delete Thoughts[i].Task;
	}

	Thoughts.DelAll();
}

void aiBrain::Init()
{	
	cur_stage = 0;

	if (chr.chrInitAI->init_action == CharacterInitAI::act_init_none)
	{
		act_type = act_ai;
		return;
	}

	if (chr.chrInitAI->init_action == CharacterInitAI::act_init_patrolling ||
		chr.chrInitAI->init_action == CharacterInitAI::act_init_patrolling_and_alarm)
	{
		act_type = act_patrolling;
		pThoughtPursuit->SetActLink("walk");
		
	}
	else
	{
		act_type = act_waiting;
		GetBody()->animation->Goto(chr.chrInitAI->init_node, 0.0f);
	}
}

void aiBrain::SetParams(CharacterControllerParams * params)
{
	if (!params) return;

	fDistofView=((aiParams*)params)->fDistofView;	
	fDistofViewPlayer=((aiParams*)params)->fDistofViewPlayer;	

	fDistToLoseTarget =((aiParams*)params)->fDistToLoseTarget;	

	if (fDistToLoseTarget<fDistofViewPlayer||
		fDistToLoseTarget<fDistofView)
	{
		fDistToLoseTarget = coremax(fDistofViewPlayer,fDistofView) + 1.0f;
	}

	iNumPlayerAttckers=((aiParams*)params)->iNumPlayerAttckers;
	iNumNPCAttckers=((aiParams*)params)->iNumNPCAttckers;

	WayPointsName=((aiParams*)params)->WayPointsName;

	bIsStatist = ((aiParams*)params)->isStatist;

	if (bIsStatist)
	{
		GetBody()->logic->SetAttackReaction(((aiParams*)params)->attack_react);
		GetBody()->logic->SetAttackDamage(((aiParams*)params)->attack_damage);
	}
	else
	{
		GetBody()->logic->SetAttackReaction(null);
		GetBody()->logic->SetAttackDamage(-1.0f);
	}

	AIZoneName=((aiParams*)params)->AIZoneName;

	Target=((aiParams*)params)->Target;

	int count = ((aiParams*)params)->stages.Size();
	for (int i=0; i<count; i++)
	{
		stages.Add(((aiParams*)params)->stages[i]);
	}

	fTimeToNextAttack2 =((aiParams*)params)->fTimeToNextAttack;
	iMaxSimelouslyAttackers =((aiParams*)params)->iMaxSimelouslyAttackers;	

	//block_timecooldown = ((aiParams*)params)->block_cooldown;
	block_timecooldown = GetBody()->block_cooldown;
}

void aiBrain::ExecuteThoughts (float fDeltaTime)
{
	if (iActiveThought!=-1)
	{
		bool bStilAlive = Thoughts[iActiveThought].Task->Process(fDeltaTime);		

		if (!bStilAlive)
		{
			Thoughts[iActiveThought].Task->Stop();
			iActiveThought = -1;
		}		
	}
}

void aiBrain::ShowDebugInfo()
{
	if (iActiveThought!=-1)
	{		
		Thoughts[iActiveThought].Task->DebugDraw(pBody->physics->GetPos(), &pBody->Mission().Render());
	}

	if (pChrTarget)
	{
		chr.Render().DrawLine(chr.physics->GetPos() + Vector(0,1.5,0),0xff0000ff,
							  pChrTarget->physics->GetPos() + Vector(0,1.5,0),0xff0000ff);
	}

	chr.Render().DrawLine(chr.physics->GetPos() + Vector(0,1.5,0),0xffff00ff,
						  chr.logic->GetSpawnPoint() + Vector(0,0.0,0),0xffff00ff);

	chr.Render().DrawSphere(chr.logic->GetSpawnPoint() + Vector(0,0.0,0),0.25f,0xffff00ff);
	
					
	if (act_type !=  act_ai)
	{
		Matrix mat(0,GetBody()->physics->GetAy(),0);

		Vector p1 = mat.vz;
		p1.Rotate(chr.chrInitAI->sect_angle);
	
		Vector p2 = mat.vz;
		p2.Rotate(-chr.chrInitAI->sect_angle);
		p1 *= chr.chrInitAI->allarm_dist;
		p2 *= chr.chrInitAI->allarm_dist;

		chr.Render().DrawLine(chr.physics->GetPos(),0xffff00ff,
							  chr.physics->GetPos() + p1,0xffff00ff);

		chr.Render().DrawLine(chr.physics->GetPos(),0xffff00ff,
							  chr.physics->GetPos() + p2,0xffff00ff);

		chr.Render().DrawLine(chr.physics->GetPos() + p1,0xffff00ff,
							  chr.physics->GetPos() + p2,0xffff00ff);
	}
}

//Получить тело, которым управляем...
Character* aiBrain::GetBody ()
{
	return pBody;
}

void aiBrain::Reset()
{	
	if (StopWhenDebug())
	{
		int k =0;

		k++;
	}

	BrainState=EBS_IDLE;
	
	//pChrTarget = NULL;

	GetBody()->controller->SetNumAtackers(0);

	DeattachFromCP();

	if (iActiveThought!=-1)
	{
		Thoughts[iActiveThought].Task->Stop();		

		iActiveThought = -1;
	}

	ExecuteThoughts(0);
}

aiThought* aiBrain::AddThought (aiThought* Task)
{
	BrainThought NewTask;
	NewTask.Task = Task;
	Thoughts.Add(NewTask);

	return Task;
}

void aiBrain::ActivateThought(aiThought* thought)
{
	if (iActiveThought!=-1)
	{
		Thoughts[iActiveThought].Task->Stop();		

		iActiveThought = -1;
	}

	for (int i = 0; i < (int)Thoughts.Size(); i++)
	{
		if (thought == Thoughts[i].Task)
		{
			iActiveThought = i;

			return;
		}		
	}	
}

dword aiBrain::GetThoughtsCount ()
{
	return Thoughts.Size();
}

bool aiBrain::IsEmpty ()
{
	if (iActiveThought == -1)
	{
		return true;
	}

	return false;
}

bool aiBrain::AllowRestartAnim()
{
	if (act_type != act_ai)
		return false;

	if (stages.Len() && cur_stage != 0)
		return false;

	return true;
}

void aiBrain::PlayerAchtung()
{
	if (act_type == act_ai || act_type == act_alarming) return;
		
	if (chr.chrInitAI->init_action == CharacterInitAI::act_init_patrolling ||
		chr.chrInitAI->init_action == CharacterInitAI::act_init_patrolling_and_alarm)
	{
		pThoughtPursuit->SetActLink("run");
	}

	if (chr.chrInitAI->init_action == CharacterInitAI::act_init_wait_and_alarm ||
		chr.chrInitAI->init_action == CharacterInitAI::act_init_patrolling_and_alarm)
	{
		Reset();		

		act_type =  act_alarming;
		chr.animation->ActivateLink("Alarm");

		time_to_alarm = chr.chrInitAI->time_to_alarm;
	}
	else
	{
		chr.animation->ActivateLink("Act");
		act_type = act_ai;
	}	
}

void aiBrain::Shoot(float dmg)
{	
}

void aiBrain::SetNumAtackers(int _NumAtackers)
{
	NumAtackers = _NumAtackers;
	//Assert(NumAtackers>=0);
	if (NumAtackers<0) NumAtackers = 0;
}

//Клипует плоскостями... как фрустум....
void aiBrain::SectorSearch (const Vector& vPos, float fAy, float fNearPlane, float fFarPlane, float min_ang, float max_ang, GroupId group)
{
	searchResult.DelAll();	

	Vector dir(0.0f,0.0f,1.0f);
	dir.Rotate(fAy);

	float fObjectRadius = 0.2f;
	Matrix m;

	float cs = cosf(max_ang);

	const array<Character *> &chars  = chr.arbiter->GetActiveCharacters();

	for (int i=0;i<(int)chars.Size();i++)
	{
		Character* pObject = chars[i];

		if (GetBody()==pObject) continue;
		if (!pObject->IsActive()) continue;
		if (pObject->logic->IsActor()) continue;
		if (pObject->logic->IsPairMode()) continue;

		Vector objPos = pObject->GetMatrix(m).pos - (vPos  - dir * 0.0f);

		if ((objPos | dir)<cs) continue;
		
		float r = objPos.GetLength();
		
		if (fNearPlane>r || r>fFarPlane) continue;

		if (GetBody()!=pObject)
		{
			TFinder* pFind = &searchResult[searchResult.Add()];			
			pFind->chr = pObject;
		}
	}



	/*float fSin = sinf(fAy);
	float fCos = cosf(fAy);

	float fSin_min = sinf(min_ang+fAy);
	float fCos_min = cosf(min_ang+fAy);

	float fSin_max = sinf(max_ang+fAy);
	float fCos_max = cosf(max_ang+fAy);

	Vector Forward = Vector (fSin, 0.0f, fCos);

	Vector vLineLeft_s = Vector(fSin_min*fNearPlane, 0.0f, fCos_min*fNearPlane);
	Vector vLineLeft_e = Vector(fSin_min*fFarPlane, 0.0f, fCos_min*fFarPlane);
	Vector vLineLeft = vLineLeft_e-vLineLeft_s;
	vLineLeft.Rotate_PI2_CW();
	vLineLeft.Normalize();
	
	Vector vLineRight_s = Vector(fSin_max*fNearPlane, 0.0f, fCos_max*fNearPlane);
	Vector vLineRight_e = Vector(fSin_max*fFarPlane, 0.0f, fCos_max*fFarPlane);
	Vector vLineRight = vLineRight_e - vLineRight_s;
	vLineRight.Rotate_PI2_CCW();
	vLineRight.Normalize();

	Plane clip[4];
	clip[0].n = Forward;
	clip[0].d = fNearPlane;
	clip[1].n = -Forward;
	clip[1].d = -fFarPlane;
	clip[2] = Plane (vLineLeft, vLineLeft_e);
	clip[3] = Plane (vLineRight, vLineRight_e);


	float fObjectRadius = 0.2f;
	Matrix m;
	
	const array<Character *> &chars  = chr.arbiter->GetActiveCharacters();

	for (int i=0;i<(int)chars.Size();i++)
	{
		Character* pObject = chars[i];

		if (!pObject->IsActive()) continue;
		if (pObject->logic->IsActor()) continue;
		if (pObject->logic->IsPairMode()) continue;

		Vector ObjPos = pObject->GetMatrix(m).pos - vPos;

		bool bIsVisible = true;
		for(long p = 0; p < 4; p++)
		{
			//GetBody()->Render().DrawVector(vPos, vPos + clip[p].n, 0xFFFF0000);
			if(((ObjPos | clip[p].n) - clip[p].d) < -fObjectRadius)
			{
				bIsVisible = false;
				break;
			}
		}

		if (bIsVisible&&GetBody()!=pObject)
		{
			TFinder* pFind = &searchResult[searchResult.Add()];			
			pFind->chr = pObject;
		}
	}*/
}

void aiBrain::SectorSearchDraw (const Vector& vPos, float fAy, float fNearPlane, float fFarPlane, float min_ang, float max_ang)
{
	float fNear = fNearPlane;
	float fFar = fFarPlane;

	Vector v2 = vPos + Vector(sin(max_ang+fAy)*fNear, 0.0f, cos(max_ang+fAy)*fNear);
	Vector v3 = vPos + Vector(sin(min_ang+fAy)*fNear, 0.0f, cos(min_ang+fAy)*fNear);
	Vector v4 = vPos + Vector(sin(max_ang+fAy)*fFar, 0.0f, cos(max_ang+fAy)*fFar);
	Vector v5 = vPos + Vector(sin(min_ang+fAy)*fFar, 0.0f, cos(min_ang+fAy)*fFar);

	GetBody()->Render().DrawLine(v2, 0xFFFF0000, v3, 0xFFFF0000);
	GetBody()->Render().DrawLine(v4, 0xFFFF0000, v5, 0xFFFF0000);
	GetBody()->Render().DrawLine(v2, 0xFFFF0000, v4, 0xFFFF0000);
	GetBody()->Render().DrawLine(v3, 0xFFFF0000, v5, 0xFFFF0000);
}

void aiBrain::CircleSearch (const Vector& vPos, const float fRadius, GroupId group)
{	
	searchResult.DelAll();	
	chr.arbiter->FindCircle(chr, fRadius);	

	array<CharacterFind> &chars  = chr.arbiter->find;

	for (int i=0;i<(int)chars.Size();i++)
	{
		Character* pObject = chars[i].chr;
		
		if (pObject->logic->IsActor()) continue;
		if (pObject->logic->IsPairMode()) continue;
		if (pObject->logic->IsSlave()) continue;

		TFinder* pFind = &searchResult[searchResult.Add()];		
		pFind->chr = pObject;
	}	
}

void aiBrain::UpdateMO()
{
	if (NeedToFindMO)
	{
		if (pCPoints.Size()==0)
		{
			MGIterator* iter;

			iter = &GetBody()->Mission().GroupIterator(COMBAT_POINTS_GROUP, _FL_);

			for (;!iter->IsDone();iter->Next())
			{
				pCPoints.Add((CombatPoints*)iter->Get());
			}

			iter->Release();
		}	

		if (pSPoints.Size()==0)
		{
			MGIterator* iter;

			iter = &GetBody()->Mission().GroupIterator(SAFTY_POINTS_GROUP, _FL_);

			for (;!iter->IsDone();iter->Next())
			{
				pSPoints.Add((SaftyPoints*)iter->Get());
			}		

			iter->Release();
		}

		if (AIZone==NULL && AIZoneName.NotEmpty())
		{
			MGIterator* iter;

			iter = &GetBody()->Mission().GroupIterator(AIZones_GROUP, _FL_);

			while(!iter->IsDone() && AIZone==NULL)
			{
				AIZone = (AIZones*)iter->Get();

				if (AIZone->GetObjectID() != AIZoneName)
				{
					AIZone = NULL;
				}	

				iter->Next();
			}

			iter->Release();
		}
		
		MOSafePointerTypeEx<Character> mo;
		static const ConstString playerId("Player");
		static const ConstString strTypeId("Character");
		mo.FindObject(&GetBody()->Mission(),playerId,strTypeId);

		pChrPlayer = mo.Ptr();
		
		if (!pChrPlayer)		
		{
			return;
		}

		NeedToFindMO = false;
	}
}

//Обновить состояние контролера на каждом кадре
void aiBrain::Update(float dltTime)
{
	if (cur_stage<(int)stages.Size())
	{		
		if (stages[cur_stage].hp > chr.logic->GetAbsoluteHP())
		{
			ChangeStage(cur_stage+1, stages[cur_stage].param);
			cur_stage++;
		}
	}

	GetBody()->logic->UpdateAttacks(pThoughtKick->GetActionStack());

	if (iNumHits>0)
	{
		fHitTime +=dltTime;

		if (fHitTime>2)
		{
			fHitTime = 0;
			iNumHits = 0;
		}
	}	

	if (GetBody()->logic->GetState() !=  CharacterLogic::state_block)
	{
		block_cooldown -= dltTime;

		if (block_cooldown<0.0f) block_cooldown = 0.0f;
	}

	fCurTimeToNextAttack -= dltTime;

	if (fCurTimeToNextAttack<0.0f)
	{
		fCurTimeToNextAttack = 0.0f;
	}	

	if (fTime_to_return_to_spawn>0)
	{
		fTime_to_return_to_spawn -= dltTime;

		if (fTime_to_return_to_spawn<0.0f)
		{
			fTime_to_return_to_spawn = 0.0f;
		}
	}

	if (NeedToFindMO) UpdateMO();

	//if (GetBody()->logic->IsPairMode()&&!bAllowUpdateInPairModel) return;

	chr.logic->PairProcess(dltTime, 0);

	ExecuteThoughts(dltTime);


	if (pChrTarget)	
	{
		Vector dir = pChrTarget->physics->GetPos() - chr.physics->GetPos();
		dir.Normalize();

		Vector chr_dir = Vector(1,0,0);
		chr_dir.Rotate(chr.physics->GetAy());		

		float angle = chr_dir.GetAngle(dir);
		
		angle = Clampf (angle,-PI,PI);
		angle = - angle + PI * 0.5f;
		chr.bodyparts.SetRotation(angle,true);

		//chr.Render().Print(10,10,0xff00ffff,"%f",angle);
	}
	else
	{
		chr.logic->OrientBodyPart(true);
	}

	//DrawBrainState();
}

bool aiBrain::PreTick()
{
	if (act_type !=  act_ai)
	{
		if (act_type == act_alarming)
		{		
			OrinentToTarget();

			time_to_alarm -= api->GetDeltaTime();

			if (time_to_alarm>0.0f)
			{
				CircleSearch(GetBody()->physics->GetPos(), chr.chrInitAI->argo_dist, MG_CHARACTER);	

				// Filter Characters
				for (int i = 0; i < (int)searchResult.Size(); i++)
				{
					Character* finded = searchResult[i].chr;

					if (finded->logic->GetHP()<0.1f) continue;
					if (finded->logic->IsEnemy(finded)) continue;
			
					finded->controller->PlayerAchtung();
				}
			}
			else
			{
				act_type =  act_ai;
			}			
		}
		else
		{
			Character* pChrTarget=SearchTarget(true,chr.chrInitAI->allarm_dist);
			
			if (pChrTarget)
			{				
				Vector pl_dir = pChrTarget->physics->GetPos() - GetBody()->physics->GetPos();
				float len = pl_dir.Normalize();

				Matrix mat(0,GetBody()->physics->GetAy(),0);
				float ang = pl_dir | mat.vz;

				if (ang>chr.chrInitAI->sect_cos_angle ||
					len < chr.chrInitAI->hear_dist)
				{
					Vector vHeight(0.0f, 0.8f, 0.0f);
				
					if (RayCast(GetBody()->physics->GetPos()+vHeight,
							    pChrTarget->physics->GetPos()+vHeight,
								phys_mask(phys_character)))
					{
						pChrTarget = null;
					}		
					else
					{
						PlayerAchtung();
					}
				}
				else
				{
					pChrTarget = null;
				}
			}			
			
			if (!pChrTarget)
			{
				if (chr.chrInitAI->init_action == CharacterInitAI::act_init_patrolling ||
					chr.chrInitAI->init_action == CharacterInitAI::act_init_patrolling_and_alarm)
				{
					if (IsEmpty())
					{
						if (patrol_time_action>0.0f)
						{
							patrol_time_action -= api->GetDeltaTime();							

							if (patrol_time_action<0.0f) patrol_time_action = -1.0f;
						}
						else				
						{
							bool go = false;

							if ((chr.chrInitAI->patrol_points[patrol_point].pos - chr.physics->GetPos()).GetLength2()<1.75f)
							{					
								if (chr.chrInitAI->patrol_points[patrol_point].wait_time>0.0f)
								{
									patrol_time_action = chr.chrInitAI->patrol_points[patrol_point].wait_time;
									GetBody()->physics->Orient(GetBody()->physics->GetPos() + 
															   chr.chrInitAI->patrol_points[patrol_point].dir);
									
									GetBody()->animation->Goto(chr.chrInitAI->init_node, 0.2f);				
								}
								else
								{
									go = true;							
								}

								patrol_point++;

								if (patrol_point>(int)chr.chrInitAI->patrol_points.Size()-1)
								{
									patrol_point = 0;
								}
							}
							else
							{
								go = true;
							}

							if (go)					
							{
								pThoughtPursuit->Activate(chr.chrInitAI->patrol_points[patrol_point].pos, WayPointsName, 0.5f, true, 0.2f);
							}
						}				
					}
				}
			}		
		}		

		return true;
	}

	return false;
}

bool aiBrain::BaseTick()
{
	if (!AIZone) return false;

	CheckTargetState(false);
	CheckTargetPairState();
	
	//if (BrainState==EBS_WAITPLAYER && !pChrTarget)
	//{
	//	Reset();
	//}

	if (BrainState==EBS_WAIT_FATALITY) return false;	

	if (BrainState==EBS_PURSUIT||
		BrainState==EBS_KICK||
		BrainState==EBS_STRAFING)
	{
		if (pChrTarget)
		{
			if (!InAIZone(GetBody()->physics->GetPos(),false)&&
				!InAIZone(pChrTarget->physics->GetPos()))
			{
				fTime_to_return_to_spawn = 5.0f;

				pThoughtStanding->Activate( Rnd(2)+10, pChrTarget );
				BrainState=EBS_WAITPLAYER;
				return false;
			}			
		}
	
		/*else
		if (!InAIZone(GetBody()->physics->GetPos(),false))
		{
		fTime_to_return_to_spawn = 5.0f;

		pThoughtStanding->Activate( Rnd(2)+10, pChrTarget );
		BrainState=EBS_WAITPLAYER;
		return;
		}*/
	}	

	if (BrainState==EBS_WAITPLAYER)
	{
		if (pChrTarget)
		{
			if (InAIZone(pChrTarget->physics->GetPos()))
			{
				Reset();
				AttackCharacter();

				return false;				
			}
		}

		if (!InAIZone(GetBody()->physics->GetPos()))
		{	
			if (InAIZone(GetBody()->logic->GetSpawnPoint()))
			{
				pThoughtPursuit->Activate(GetBody()->logic->GetSpawnPoint(), WayPointsName, 0.5f, true);
				pChrTarget = NULL;
				BrainState=EBS_RETURNTOSPAWN;
			}			
		}

		if (fTime_to_return_to_spawn<=0.0f)
		{
			if ((GetBody()->logic->GetSpawnPoint()-GetBody()->physics->GetPos()).GetLength2() > Sqr(1.75f))
			{
				pThoughtPursuit->Activate(GetBody()->logic->GetSpawnPoint(), WayPointsName, 0.5f, true);
				pChrTarget = NULL;
				BrainState=EBS_RETURNTOSPAWN;	
			}
			else
			{
				fTime_to_return_to_spawn = 5.0f;
			}
		}

		return false;
	}	

	return true;
}

void aiBrain::TickWhenEmpty()
{
	BrainState=EBS_IDLE;
}

void aiBrain::GuardTick()
{
}

void aiBrain::Tick()
{
	// если персонаж является slave, то аи не надо тикать
	if (chr.logic && chr.logic->master)  
		return;

	if (PreTick()) return;

	if (!BaseTick()) return;

	if (IsEmpty())
	{	
		TickWhenEmpty();		
	}


	if (BrainState==EBS_IDLE||BrainState==EBS_STANDING)
	{
		if (pChrTarget==NULL)
		{
			pChrTarget=SearchTarget(true);
		}

		CheckTargetState(false);

		if (pChrTarget!=NULL)
		{			
			AttackCharacter();
			return;			
		}
		else
		{
			if (BrainState!=EBS_STANDING)
			{
				if ((GetBody()->logic->GetSpawnPoint()-GetBody()->physics->GetPos()).GetLength2() > Sqr(1.75f))
				{
					//fTime_to_return_to_spawn = 5.0f;

					pThoughtStanding->Activate( Rnd(2)+10, pChrTarget );
					BrainState=EBS_WAITPLAYER;

					//pThoughtPursuit->Activate(GetBody()->logic->GetSpawnPoint(), WayPointsName, 0.5f);
					//pChrTarget = NULL;
					//BrainState=EBS_RETURNTOSPAWN;	
				}
				else
				if (searchResult.Size()>0)
				{
					Character* trg = (Character*)searchResult[0].chr;

					if (CheckDist(trg,10.0f))
					{
						pThoughtStrafing->Activate( trg, 4+Rnd(4));	
						BrainState=EBS_STRAFING;
					}
					else				
					{
						pThoughtStanding->Activate( 4+Rnd(4), pChrPlayer);	
						BrainState=EBS_STANDING;
					}
				}
				else				
				{
					pThoughtStanding->Activate( 4+Rnd(4), pChrPlayer);	
					BrainState=EBS_STANDING;
				}
			}
		}
	}	
	else
	if (BrainState==EBS_PURSUIT&&IsEmpty())
	{	
		AttackCharacter();
	}
	else
	if (BrainState==EBS_PURSUIT)
	{
		if (!IsEmpty()&&AllowRecreatePath(pChrTarget))
		{
			pThoughtPursuit->ChangeParams(pChrTarget->physics->GetPos(), PursuitDist() );			
		}
	}	
	else
	if (BrainState==EBS_RETURNTOSPAWN)
	{
		if (SearchPlayer(true))
		{			
			if (!InAIZone(pChrTarget->physics->GetPos()))
			{
				pChrTarget = NULL;
			}
			if (pChrTarget)
			{				
				AttackCharacter();
			}
		}		
	}
}

void aiBrain::Hit(float dmg, Character* _offender, const char* reaction, DamageReceiver::DamageSource source)
{
	if (act_type != act_ai)
	{
		act_type =  act_ai;
	}

	if (iActiveThought!=-1)
	{		
		Thoughts[iActiveThought].Task->Hit();
	}
}

void aiBrain::ActivatePairLink(char* action)
{
	Character* pair = chr.logic->GetPairCharacter();

	if(pair && chr.logic->IsPairMode())
	{		
		if(chr.logic->GetState() == CharacterLogic::state_topair)
		{
			//Анализируем тип врага, и в соответствии в этим выбираем захват
			if(pair->controller)
			{
				if(action)
				{
					//Запускаем анимацию, если есть для данного типа противника
					bool testSelf = chr.animation->TestActivateLink(action);
					bool testPair = pair->animation->TestActivateLink(action);
					if(testSelf && testPair)
					{
						pair->animation->ActivateLink(action);
						chr.animation->ActivateLink(action);
					}
					if(testSelf != testPair || !testSelf)
					{
						if(api->DebugKeyState(VK_NUMPAD0))
						{
							api->Trace("Can't continue pair animation \"%s\":", action);
							Character * c = pair;
							if(testSelf)
							{
								c = &chr;
							}
							api->Trace("    obect: \"%s\"", c->GetObjectID().c_str());
							api->Trace("    animation: \"%s\"", c->animation->AnimationName());
							api->Trace("    current node: \"%s\"", c->animation->CurrentNode());
						}
					}
				}
			}
		}
	}
}

aiBrain::CPAction aiBrain::IsCPReached()
{
	Vector vDist = GetBody()->physics->GetPos() - GetCurCombatPoint()->position;

	if (vDist.GetLength2() < Sqr(GetCurCombatPoint()->radius))
	{			
		bInCombatPoint = true;

		return CPA_STANDING;
	}
	
	bInCombatPoint=false;

	return CPA_PURSUIT;	
}

aiBrain::CPAction aiBrain::FindCPoint()
{		
	//ProfileTimer timer;
	//timer.AddToCounter("Char FindCP");
	


	bool bIsActiveCPoints = false;

	// проверка на наличие активных комбот-пойнтов
	for (int i=0;i<(int)pCPoints.Size();i++)
	{
		if (pCPoints[i]->IsActive())
		{
			bIsActiveCPoints = true;

			break;
		}
	}

	if (!bIsActiveCPoints)
	{		
		// активных комбот пойнтов нет, выходим
		DeattachFromCP();

		return CPA_NOTHING;
	}
	
	if (iCurPt_CPoint!=-1&&iCurGr_CPoint!=-1)
	{
		if (Rnd()>0.456f)
		{
			CPAction action = IsCPReached();

			if (action == CPA_STANDING) return CPA_STANDING;
		}
	}

	if (pCPoints.Size()>0)
	{			
		for (int i=0;i<(int)pCPoints.Size();i++)
		{
			if (pCPoints[i]->IsActive())
			{
				for (int j=0;j<(int)pCPoints[i]->Points.Size();j++)					
				{
					pCPoints[i]->Points[j].itmp=j;
					pCPoints[i]->Points[j].itmpGr=i;
				}
			}
		}
		
		// сортируем комбот пойнты по стпени удаленности от персонажа
		Vector vTmp;
		Vector vPos = GetBody()->physics->GetPos();

		for (int l=0;l<(int)pCPoints.Size();l++)
		{
			if (pCPoints[l]->IsActive())
			{
				for (int k=0;k<(int)pCPoints[l]->Points.Size()-1;k++)					
				{						
					for (int i=l;i<(int)pCPoints.Size();i++)
					{
						int index = 0;

						if (i==l)
						{
							index = k+1;
						}

						if (pCPoints[i]->IsActive())
						{							
							for (int j=index;j<(int)pCPoints[i]->Points.Size();j++)								
							{
								vTmp = vPos - GetCombatPoint(i,j)->position;
								float dist1 = vTmp.GetLength2();

								vTmp = vPos - GetCombatPoint(l,k)->position;
								float dist2 = vTmp.GetLength2();

								if (dist1 < dist2)		
								{
									int tmp = pCPoints[l]->Points[k].itmp;
									pCPoints[l]->Points[k].itmp = pCPoints[i]->Points[j].itmp;
									pCPoints[i]->Points[j].itmp = tmp;

									tmp = pCPoints[l]->Points[k].itmpGr;
									pCPoints[l]->Points[k].itmpGr = pCPoints[i]->Points[j].itmpGr;
									pCPoints[i]->Points[j].itmpGr = tmp;
								}
							}
						}
					}
				}
			}
		}

		// рандомно тусуем комбот пойнты
		if (!IsStatist())
		{
			for (int i=0; i<250;i++)
			{
				int gr1 = int(Rnd(0.99f) * pCPoints.Size());
				int pt1 = int(Rnd(0.99f) * pCPoints[gr1]->Points.Size());

				int gr2 = int(Rnd(0.99f) * pCPoints.Size());
				int pt2 = int(Rnd(0.99f) * pCPoints[gr2]->Points.Size());

	
				if (!pCPoints[gr1]->IsActive() || !pCPoints[gr2]->IsActive()) continue;

				int tmp = pCPoints[gr1]->Points[pt1].itmp;
				pCPoints[gr1]->Points[pt1].itmp = pCPoints[gr2]->Points[pt2].itmp;
				pCPoints[gr2]->Points[pt2].itmp = tmp;

				tmp = pCPoints[gr1]->Points[pt1].itmpGr;
				pCPoints[gr1]->Points[pt1].itmpGr = pCPoints[gr2]->Points[pt2].itmpGr;
				pCPoints[gr2]->Points[pt2].itmpGr = tmp;
			}
		}


		bool isEnemy = GetBody()->logic->IsEnemy(pChrPlayer);

		if (iCurPt_CPoint!=-1&&iCurGr_CPoint!=-1)
		{	
			if (IsStatist())
			{
				for (int i=0;i<(int)pCPoints.Size();i++)
				{	
					if (pCPoints[i]->IsActive())
					{
						for (int j=0;j<(int)pCPoints[i]->Points.Size();j++)
							if (InAIZone(GetCombatPoint(i,j)->position))
							{
								CombatPoints::CombatPoint* c_point = GetCombatPoint(i,j);;

								int num_enemy;

								if (isEnemy)
								{
									num_enemy = c_point->NumEnemyAttackers;
								}
								else
								{
									num_enemy = c_point->NumAllyAttackers;
								}

								if (c_point->NumEnemyAttackers!=0 || c_point->NumAllyAttackers!=0)
								{							
									if (IsStatist())
									{
										if (!c_point->bIsStatistPoint) continue;
									}
									else
									{
										if (c_point->bIsStatistPoint) continue;
									}
								}

								if (num_enemy<c_point->MaxAttackers && InAIZone(c_point->position))
								{						
									AttachToCP(pCPoints[i]->Points[j].itmpGr,pCPoints[i]->Points[j].itmp);

									return IsCPReached();
								}	
							}
					}
				}

				return CPA_NOTHING;
			}

			if (*GetCurCPtNumAlly(isEnemy)==0)
			{
				bool IsAllyThere = false;

				for (int i=0;i<(int)pCPoints.Size();i++)
				{	
					if (pCPoints[i]->IsActive())
					{
						for (int j=0;j<(int)pCPoints[i]->Points.Size();j++)
							if (InAIZone(GetCombatPoint(i,j)->position))
							{
								if (*GetCPtNumAlly(i,j,isEnemy)>0)
								{									
									vTmp = vPos - GetCombatPoint(i,j)->position;
									float fDist=vTmp.GetLength2();														

									if (fDist < Sqr(15.0f))											
									{
										IsAllyThere = true;
										break;
									}
								}
							}
					}
				}
				
				if (IsAllyThere)					
				{	
					for (int i=0;i<(int)pCPoints.Size();i++)
					{	
						if (pCPoints[i]->IsActive())
						{
							for (int j=0;j<(int)pCPoints[i]->Points.Size();j++)
								if (InAIZone(GetCombatPoint(i,j)->position))
								{
									if (*GetCPtNumAlly(i,j,isEnemy)>0&&IsCPocpStatist(i,j)&&
										*GetCPtNumEnemy(i,j,isEnemy)<GetCombatPoint(i,j)->MaxAttackers&&
										(iCurPt_CPoint != pCPoints[i]->Points[j].itmp || iCurGr_CPoint != pCPoints[i]->Points[j].itmpGr))
									{		
										AttachToCP(pCPoints[i]->Points[j].itmpGr,pCPoints[i]->Points[j].itmp);

										return IsCPReached();
									}
								}
						}
					}
				}
				else
				{
					bool bThereFreeCP = false;						

					for (int i=0;i<(int)pCPoints.Size();i++)
					{	
						if (pCPoints[i]->IsActive())
						{
							for (int j=0;j<(int)pCPoints[i]->Points.Size();j++)
								if (InAIZone(GetCombatPoint(i,j)->position))
								{
									CombatPoints::CombatPoint* c_point = GetCombatPoint(i,j);

									if (c_point->NumEnemyAttackers!=0 || c_point->NumAllyAttackers!=0)
									{							
										if (IsStatist())
										{
											if (!c_point->bIsStatistPoint) continue;
										}
										else
										{
											if (c_point->bIsStatistPoint) continue;
										}
									}

									if (*GetCPtNumEnemy(i,j,isEnemy)<c_point->MaxAttackers)
									{
										bThereFreeCP = true;
										break;
									}
								}
						}
					}

					bool bCPointFinded = false;

					int iGroup;
					int iCPoint;

					while(!bCPointFinded && bThereFreeCP)
					{
						iGroup = int(Rnd(0.99f) * pCPoints.Size());							
						iCPoint = int(Rnd(0.99f) * pCPoints[iGroup]->Points.Size());

						CombatPoints::CombatPoint* c_point = &pCPoints[iGroup]->Points[iCPoint];

						int num_enemy;

						if (isEnemy)
						{
							num_enemy = c_point->NumEnemyAttackers;
						}
						else
						{
							num_enemy = c_point->NumAllyAttackers;
						}

						if (c_point->NumEnemyAttackers!=0 || c_point->NumAllyAttackers!=0)
						{							
							if (IsStatist())
							{
								if (!c_point->bIsStatistPoint) continue;
							}
							else
							{
								if (c_point->bIsStatistPoint) continue;
							}
						}


						if (num_enemy<c_point->MaxAttackers && pCPoints[iGroup]->IsActive()
							&& InAIZone(pCPoints[iGroup]->Points[iCPoint].position))
						{
							bCPointFinded = true;								

							AttachToCP(iGroup,iCPoint);
							
							return IsCPReached();
						}						
					}
				}
			}

			return IsCPReached();		
		}
		else
		{
			for (int i=0;i<(int)pCPoints.Size();i++)
			{	
				if (pCPoints[i]->IsActive())
				{
					for (int j=0;j<(int)pCPoints[i]->Points.Size();j++)
					{
						CombatPoints::CombatPoint* c_point = GetCombatPoint(i,j);

						if (InAIZone(c_point->position))
						{
							if (isEnemy)
							{
								if (c_point->NumEnemyAttackers>=c_point->MaxAttackers)
								{
									continue;
								}

							}
							else
							{
								if (c_point->NumAllyAttackers>=c_point->MaxAttackers)
								{
									continue;
								}
							}	

							if (c_point->NumAllyAttackers!=0 || c_point->NumEnemyAttackers!=0)
							{							
								if (IsStatist())
								{
									if (!c_point->bIsStatistPoint) continue;
								}
								else
								{
									if (c_point->bIsStatistPoint) continue;
								}
							}

							AttachToCP(pCPoints[i]->Points[j].itmpGr,pCPoints[i]->Points[j].itmp);

							return IsCPReached();
						}
					}
				}
			}
		}
	}

	DeattachFromCP();

	return CPA_NOTHING;
}

void aiBrain::AttachToCP(int group,int index)
{
	DeattachFromCP();

	iCurPt_CPoint = index;
	iCurGr_CPoint = group;

	CombatPoints::CombatPoint* cpt = GetCombatPoint(group,index);

	if (*GetCurCPtNumEnemy(GetBody()->logic->IsEnemy(pChrPlayer))==0 &&
		*GetCurCPtNumAlly(GetBody()->logic->IsEnemy(pChrPlayer))==0)
	{		
		cpt->bIsStatistPoint = IsStatist();
	}
			
	*GetCurCPtNumEnemy(GetBody()->logic->IsEnemy(pChrPlayer))+=1;	
	cpt->Chars.Add(GetBody());
}

void aiBrain::DeattachFromCP()
{	
	if (iCurPt_CPoint!=-1&&iCurGr_CPoint!=-1)
	{
		for (int i=0;i<(int)pCPoints[iCurGr_CPoint]->Points[iCurPt_CPoint].Chars.Size();i++)
		{		
			if (pCPoints[iCurGr_CPoint]->Points[iCurPt_CPoint].Chars[i] == GetBody())
			{
				pCPoints[iCurGr_CPoint]->Points[iCurPt_CPoint].Chars.DelIndex(i);
				break;
			}			
		}

		if (GetBody()->logic->IsEnemy(pChrPlayer))
		{							
			pCPoints[iCurGr_CPoint]->Points[iCurPt_CPoint].NumEnemyAttackers--;

			if (pCPoints[iCurGr_CPoint]->Points[iCurPt_CPoint].NumEnemyAttackers<0)
			{
				pCPoints[iCurGr_CPoint]->Points[iCurPt_CPoint].NumEnemyAttackers = 0;
			}
		}
		else
		{				
			pCPoints[iCurGr_CPoint]->Points[iCurPt_CPoint].NumAllyAttackers--;

			if (pCPoints[iCurGr_CPoint]->Points[iCurPt_CPoint].NumAllyAttackers)
			{
				pCPoints[iCurGr_CPoint]->Points[iCurPt_CPoint].NumAllyAttackers = 0;
			}
		}
	}

    iCurPt_CPoint=-1;
	iCurGr_CPoint=-1;	

	bInCombatPoint = false;
}

bool aiBrain::InAIZone(const Vector & pos, bool check_buffer)
{
	if (AIZone)
	{				

		return AIZone->IsInside(pos,check_buffer);
	}

	return true;
}

bool aiBrain::InSafetyPoint(Vector pos)
{	
	for (int j=0;j<(int)pSPoints.Size();j++)	
		if (pSPoints[j]->IsActive())
		{	
			for (unsigned int i=0;i<pSPoints[j]->Points.Size();i++)
			{
				Vector vDist = pSPoints[j]->Points[i].position-pos;

				if (vDist.GetLength2() < Sqr(pSPoints[j]->Points[i].radius))
				{
					return true;
				}
			}
		}		

	return false;
}

bool aiBrain::CheckDist(Character* target,float fRadius)
{
	if (target!=NULL)
	{
		Vector vDist = GetBody()->physics->GetPos()-target->physics->GetPos();

		if (vDist.GetLength2()<fRadius*fRadius)
		{
			return true;
		}
	}

	return false;
}

bool aiBrain::ValidateTargetRules(Character* target,float fRadius)
{
	if (target!=NULL)
	{		
		int iAttackersCount=iNumNPCAttckers;							

		if (string::IsEqual(target->controller->Name(),"Player"))
		{
			iAttackersCount=iNumPlayerAttckers;				
		}				

		if (GetBody()->arbiter->CheckNumAttckers(target, iAttackersCount+2,(char*)Name()))
		{
			return true;
		}

		return CheckDist(target, fRadius);		
	}

	return false;
}

bool aiBrain::AttackCharacter()
{
	if (!AllowAttack())
	{
		pThoughtStrafing->Activate( pChrTarget, 2+Rnd(4));	
		BrainState=EBS_STRAFING;

		return true;
	}

	float dist = GetDistToTarget();
	pThoughtKick->CheckDist(dist);

	if (GetDistToTarget()<=GetAttackDistance())
	{			
		DeattachFromCP();
				
		pThoughtKick->Activate(pChrTarget);				
		BrainState=EBS_KICK;

		fCurTimeToNextAttack = GetTimeToNextAttack();	
	}
	else			
	{		
		pThoughtPursuit->Activate(pChrTarget->physics->GetPos(), WayPointsName, PursuitDist(), true);
		BrainState=EBS_PURSUIT;	

		DeattachFromCP();		
	}

	return true;
}

float aiBrain::GetIdleAfterAttackModifier()
{
	float value = 1.0f;
	
	if (enemyAttackSpeedModifier)
		value = GetBody()->GetIndividualAttackSpeedModifier() * (1.0f + enemyAttackSpeedModifier->Get(0.0f));

	value = Max(0.2f, Min(2.0f, value));

	return 1.0f / powf(value, 3.5f);
}

float aiBrain::GetStrafeModifier()
{
	float value = 1.0f;
	
	if (enemyAttackSpeedModifier)
		value = GetBody()->GetIndividualAttackSpeedModifier() * (1.0f + enemyAttackSpeedModifier->Get(0.0f));

	value = Max(0.2f, Min(2.0f, value));

	return 1.0f / powf(value, 3.5f);
}

bool aiBrain::StartBlocking(Character * offender, DamageReceiver::DamageSource source)
{
	if (BrainState==EBS_BLOCKING)
		return true;

	if (!pThoughtBlocking || !AllowBlocking())
		return false;

	float blockProbabilityPerSource;
	if (source == DamageReceiver::ds_bullet)
		blockProbabilityPerSource = GetBody()->fBlockProbabilityPerShoot;
	else
		blockProbabilityPerSource = GetBody()->fBlockProbabilityPerHit;

	float anx_prob = (offender) ? offender->logic->GetBlockState() : -1.0f;
	float probability = (anx_prob >= 0.0f) ? anx_prob : (float)iNumHits * blockProbabilityPerSource;

	if (Rnd() < probability)
	{
		iNumHits = 0;
		fHitTime = 0;
		pThoughtBlocking->Activate(offender, 1.5f);
		block_cooldown = block_timecooldown;	
		BrainState=EBS_BLOCKING;
		return true;
	}

	return false;
}

bool aiBrain::GoToFrustum(float dist)
{
	if (!pChrPlayer) return false;

	if (pChrTarget)
	{
		if (!GetBody()->arbiter->IsSphereInFrustrum(GetBody()->physics->GetPos(), 0.1f))
		{
			Matrix mView;
			mView = GetBody()->Render().GetView();

			mView.Inverse();

			Vector dir = mView.vz;
			dir.y = 0.0f;
			dir.Normalize();

			IPhysicsScene::RaycastResult res;

			Vector dest = pChrPlayer->physics->GetPos() + dir * dist * 0.8f + Vector (0,0.8f,0.0f);			

			if (GetBody()->Physics().Raycast(pChrTarget->physics->GetPos() + Vector (0,0.8f,0.0f),dest,phys_mask(phys_character),&res))
			{		
				dest = pChrPlayer->physics->GetPos() + dir * (res.distance - 1.0f) + Vector (0,0.8f,0.0f);
			}			

			pThoughtPursuit->Activate(dest, WayPointsName, 1.25f, false);
			BrainState=EBS_RUNAWAY;

			return true;
		}
	}

	return false;
}

bool aiBrain::CanRunaway(Vector& vDestination, float dist_to_run)
{
	if (!pChrTarget) return false;

	Vector vDir=GetBody()->physics->GetPos()-pChrTarget->physics->GetPos();
	vDir.NormalizeXZ();

	vDestination = GetBody()->physics->GetPos() + vDir * dist_to_run;

	Vector vHeight(0.0f, 0.8f, 0.0f);			
	
	Vector dir[3];
	IPhysicsScene::RaycastResult res[3];

	Vector pos = GetBody()->physics->GetPos();
	dir[0] = vDir;
	dir[1] = Vector(vDir).Rotate_PI2_CW();
	dir[2] = Vector(vDir).Rotate_PI2_CCW();
	
	Vector bestDir;
	float bestDistance = -1.0f;

	for (int i=0; i<3; i++)
	{
		if (!GetBody()->Physics().Raycast(pos + vHeight, pos + dir[i] * dist_to_run + vHeight, phys_mask(phys_character), &res[i]))
		{		
			if (!GetBody()->Physics().Raycast(pos + dir[i] * dist_to_run + vHeight, pos + vHeight, phys_mask(phys_character)))
			{
				Vector pnt = pos + dir[i] * dist_to_run;
				if (GetBody()->Physics().Raycast(pnt + vHeight, pnt + Vector(0.0f, -3.0f, 0.0f), phys_mask(phys_character)))
				{
					vDestination = pos + dir[i] * dist_to_run;
					return true;
				}
			}		
		}

		if (bestDistance < res[i].distance && res[i].distance > 2.5f)
		{
			Vector pnt = dir[i] * (res[i].distance - 1.0f);

			if (GetBody()->Physics().Raycast(pos + pnt + vHeight, pos + pnt + Vector(0.0f, -3.0f, 0.0f), phys_mask(phys_character)))
			{
				bestDistance = res[i].distance;
				bestDir = pnt;
			}
		}
	}

	if (bestDistance > 2.5f)
	{
		vDestination = pos + bestDir;
		return true;
	}

	return false;
}

bool aiBrain::FilterFindedTarget(Character* pTarget, bool statistSearch)
{
	if (InSafetyPoint(pTarget->physics->GetPos()))					
	{					
		return true;
	}

	if (statistSearch)
	{
		if (!pTarget->controller->IsStatist())
		{
			return true;
		}
	}
	else
	{
		if (!IsStatist())
		{
			if (pTarget->controller->IsStatist())
			{
				return true;
			}
		}
	}

	Vector vTmp = GetBody()->physics->GetPos() - pTarget->physics->GetPos();
	float dist=vTmp.GetLengthXZ2();

	if (pTarget==pChrPlayer)
	{		
		if (dist>fDistofViewPlayer*fDistofViewPlayer)
		{					
			return true;
		}
	}
	else
	{
		float fDist=fDistofView;

		//if (bInCombatPoint)
		{
			//fDist = pCPoints[iCurGr_CPoint]->Points[iCurPt_CPoint].radius;
		}		

		if (dist>fDist*fDist)
		{					
			return true;
		}
	}		

	if (!pTarget->IsShow())
	{
		return true;
	}

	if (!pTarget->IsActive())
	{
		return true;
	}

	if (pTarget->logic->IsActor())
	{
		return true;
	}

/*	if (pTarget->logic->IsPairMode())
	{
		return true;
	}*/

	if (pTarget->logic->IsDead())
	{
		return true;
	}	

	if (pTarget->logic->GetSide() == CharacterLogic::s_boss)
	{
		return true;
	}

	if (pTarget->logic->GetSide() == CharacterLogic::s_npc)
	{
		return true;
	}

	if (!GetBody()->logic->IsEnemy(pTarget))
	{
		return true;
	}

	if (AIZone)
	{
		Matrix mat;

		pTarget->GetMatrix(mat);					

		if (!InAIZone(mat.pos))
		{
			return true;
		}
	}

	return false;
}

Character * aiBrain::GetPlayerTarget()
{
	Character * player = chr.arbiter->GetPlayer();
	if (player && CheckCharState(player))
		return player;

	return null;
}

Character* aiBrain::SearchTarget(bool bCheckMaxAllowedAttackers, float serachDist, bool statistSearch)
{
	if (!statistSearch && Target.IsEmpty())	
	{
		if (SearchPlayer(bCheckMaxAllowedAttackers, serachDist)) return pChrTarget;
	}	

	Character* pTarget = NULL;	

	float fSerachRadius = fDistofView;

	if (serachDist>0.0f)
	{
		fSerachRadius = serachDist;
	}
	else
	{
		fSerachRadius = coremax (fSerachRadius,fDistofViewPlayer);
	}	

	if (Target.NotEmpty())
	{
		searchResult.DelAll();

		TFinder target;

		MOSafePointerTypeEx<Character> trgt;
		static const ConstString strTypeId("Character");
		trgt.FindObject(&GetBody()->Mission(),Target,strTypeId);

		target.chr = trgt.Ptr();

		if  (target.chr && CheckCharState(target.chr))
		{
			searchResult.Add(target);
		}
		else
		{
			CircleSearch(GetBody()->physics->GetPos(), fSerachRadius, MG_CHARACTER);
		}							
	}
	else
	{			
		CircleSearch(GetBody()->physics->GetPos(), fSerachRadius, MG_CHARACTER);
	}

	// Filter Characters
	for (int i = 0; i < (int)searchResult.Size(); i++)
	{
		if (FilterFindedTarget(searchResult[i].chr,statistSearch))
		{
			searchResult.DelIndex(i);
			i--;			
		}		
	}			

	pTarget=NULL;

	//if (IsStatist())
	{
		for (int i = 0; i < (int)searchResult.Size()-1; i++)
		{
			for (int j = i+1; j < (int)searchResult.Size(); j++)
			{
				if (chr.arbiter->GetPowDistance(GetBody(),(Character*)searchResult[i].chr)>
					chr.arbiter->GetPowDistance(GetBody(),(Character*)searchResult[j].chr))
				{
					TFinder tmp = searchResult[i];
					searchResult[i] = searchResult[j];
					searchResult[j] = tmp;

				}
			}
		}
	}
	
	for (int i = 0; i < (int)searchResult.Size(); i++)
	{
		pTarget = searchResult[i].chr;

		int iAttackersCount=iNumNPCAttckers;							

		if (pChrPlayer==pTarget)
		{
			iAttackersCount=iNumPlayerAttckers;
		}				

		if (!chr.arbiter->CheckNumAttckers(pTarget, iAttackersCount,(char*)Name()) && bCheckMaxAllowedAttackers)
		{
			pTarget=NULL;
		}
		else
		{
			break;
		}
	}

	if (pTarget && IsStatist())
	{
		if (pTarget->controller->IsStatist())
		{
			pTarget->controller->Reset();

			if (pTarget->controller->GetTarget().IsEmpty())
			{			
				pTarget->controller->SetChrTarget(&chr);
			}
		}
	}

	return pTarget;
}

bool aiBrain::SearchPlayer(bool bCheckMaxAllowedAttackers, float serachDist)
{
	if (!pChrPlayer) return false;

	//if (pChrPlayer->logic->IsPairMode()) return false;

	pChrTarget = pChrPlayer;
	CheckTargetState(false);

	if (!pChrTarget) return false;
	
	if (!chr.logic->IsEnemy(pChrPlayer))
	{
		pChrTarget = NULL;

		return false;
	}

	float dist = serachDist;
	if (dist<0)
	{
		dist = fDistofViewPlayer*fDistofViewPlayer;
	}
	else
	{
		dist *= serachDist;
	}

	if (chr.arbiter->GetPowDistance(GetBody(),pChrPlayer)<dist)
	{
		if (chr.arbiter->CheckNumAttckers(pChrPlayer, iNumPlayerAttckers,(char*)Name()) || !bCheckMaxAllowedAttackers)
		{			
			return true;
		}		
	}

	pChrTarget = NULL;

	return false;
}

void aiBrain::CheckTargetState(bool doRestart)
{
	if (!pChrTarget) return;

	bool targetSatisfy = CheckCharState(pChrTarget);	

	if (!targetSatisfy)
	{
		if (doRestart) Reset();
		pChrTarget=NULL;
	}
}

bool aiBrain::CheckCharState(Character* target)
{
	if (!target) return false;

	if (InSafetyPoint(target->physics->GetPos()))
	{
		return false;
	}
	else
	if (!target->IsShow())
	{			
		return false;
	}
	else
	if (!target->IsActive())
	{
		return false;
	}
	else
	if (target->logic->IsDead() && IsEmpty())
	{
		return false;
	}
	else
	if (target->logic->GetHP()<0.1f)
	{
		return false;
	}
	else
	if (target->logic->IsActor() || target->logic->IsSlave())
	{
		return false;
	}
	else	
	{
		if (AIZone)
		{
			Matrix mat;

			target->GetMatrix(mat);

			if (!InAIZone(mat.pos))					
			{
				return false;
			}
		}		
	}

	return true;
}

void aiBrain::CheckTargetPairState()
{
	if (pChrTarget!=NULL)
	{		
		//if (pChrTarget->logic->GetState() == CharacterLogic::state_pair && BrainState!=EBS_WAIT_FATALITY)
		if (pChrTarget->logic->IsPairMode() && BrainState!=EBS_WAIT_FATALITY)
		{
			Reset();	

			ActivateTargetPairStateThought();
		}
		/*else	
		if (BrainState==EBS_WAIT_FATALITY)
		{
			if (!CheckDist(pChrTarget,fDistToLoseTarget))
			{
				Reset();			

				pChrTarget=NULL;
			}		
		}*/
	}

	if (BrainState==EBS_WAIT_FATALITY)
	{
		if (pChrTarget)
		{		
			if (!pChrTarget->logic->IsPairMode())
			{			
				Reset();				
			}
			else
			{
				if (IsEmpty())
				{
					ActivateTargetPairStateThought();
				}
			}
		}
		else
		{
			Reset();			
		}		
	}
}

const ConstString & aiBrain::GetControllerState()
{
	switch (BrainState)
	{		
	case EBS_KICK:
		{
			RETURN_CONSTSTRING("Melee attack");
		}
		break;
	case EBS_PURSUIT:
		{
			RETURN_CONSTSTRING("pursuit");
		}
		break;
	case EBS_IDLE:
		{
			RETURN_CONSTSTRING("idle");
		}		
		break;
	case EBS_STRAFING:
		{
			RETURN_CONSTSTRING("Strafing");
		}
		break;
	case EBS_STANDING:
		{
			RETURN_CONSTSTRING("Standing");
		}
		break;		
	case EBS_GO_TO_CPOINT:
		{
			RETURN_CONSTSTRING("Goto ComboPoint");
		}
		break;		
	case EBS_WAIT_FATALITY:
		{
			RETURN_CONSTSTRING("Wait Fatality");
		}
		break;		
	case EBS_MOVE_AWAY:
		{
			RETURN_CONSTSTRING("Move away");
		}
		break;	
	case EBS_BLOCKING:
		{
			RETURN_CONSTSTRING("Blocking");
		}
		break;		
	case EBS_SHOOT:
		{
			RETURN_CONSTSTRING("Shooting");
		}
		break;
	case EBS_DROPBOMB:
		{
			RETURN_CONSTSTRING("Drop Bomb");
		}
		break;
	case EBS_PLACEBOMB:
		{
			RETURN_CONSTSTRING("Place Bomb");
		}
		break;
	case EBS_SUICIDE:
		{
			RETURN_CONSTSTRING("Suicide");
		}
		break;
	case EBS_RAGE:
		{
			RETURN_CONSTSTRING("Rage");
		}
		break;
	case EBS_PRAY:
		{
			RETURN_CONSTSTRING("Pray");
		}
		break;
	case EBS_GO_TO_PRAY:
		{
			RETURN_CONSTSTRING("Goto Pray");
		}
		break;
	case EBS_FATALITY:
		{
			RETURN_CONSTSTRING("Fatality");
		}
		break;		
	case EBS_FOLLOWHERO:
		{
			RETURN_CONSTSTRING("Follow Hero");
		}
		break;
	case EBS_WAITPLAYER:
		{
			RETURN_CONSTSTRING("Wait Player");
		}
		break;				
	case EBS_RETURNTOSPAWN:
		{
			RETURN_CONSTSTRING("Return to spawn");
		}
		break;	
	case EBS_DRINK:
		{
			RETURN_CONSTSTRING("Drinking vodka");
		}
		break;	
	}	

	RETURN_CONSTSTRING("none");
}

void aiBrain::DrawBrainState()
{
	GetBody()->Mission().Render().Print(GetBody()->physics->GetPos(),-1.0f, -8.0f, 0xFFFF0000, "%s", GetControllerState().c_str());	

	if (pChrTarget!=NULL)
	{
		float fDist = (pChrTarget->physics->GetPos() - GetBody()->physics->GetPos()).GetLength();		

		GetBody()->Mission().Render().Print(GetBody()->physics->GetPos()+Vector(0,-1,0),-1.0f, -8.0f, 0xFFFF0000, "I have target at %f", fDist);
	}

}

bool aiBrain::AllowRecreatePath(Character* pTarget)
{
	if (pTarget!=NULL)
	{
		Vector pos1 = GetBody()->physics->GetPos();
		Vector pos2 = pTarget->physics->GetPos();

		float dist2 = (pos1-pos2).GetLength2();

		if (iActiveThought != -1 && dist2 < Sqr(20.0f))
		{
			if (Thoughts[iActiveThought].Task == (aiThought*)pThoughtPursuit)
				if (pThoughtPursuit->GetNumPathPoints() <= 2)
					return true;
		}

		if (dist2 < Sqr(fDistToReconstructPath) && fabs(pos1.y - pos2.y) < 0.1f)
		{						
			return true;
		}
	}

	return false;
}

bool aiBrain::AllowAttack()
{
	if (!pChrTarget) return false;

	if (pChrTarget->controller->GetNumAtackers()<iMaxSimelouslyAttackers &&	fCurTimeToNextAttack < 0.1f) return true;

	return false;
}

bool aiBrain::AllowBlocking()
{
	return (block_cooldown<0.001f && BrainState!=EBS_KICK);
}

CombatPoints::CombatPoint* aiBrain::GetCombatPoint(int group,int index)
{
	return &pCPoints[pCPoints[group]->Points[index].itmpGr]->Points[pCPoints[group]->Points[index].itmp];
}

CombatPoints::CombatPoint* aiBrain::GetCurCombatPoint()
{
	return &pCPoints[iCurGr_CPoint]->Points[iCurPt_CPoint];
}

int* aiBrain::GetCPtNumAlly(int group,int index,bool enemy)
{
	if (enemy)
	{
		return &pCPoints[pCPoints[group]->Points[index].itmpGr]->Points[pCPoints[group]->Points[index].itmp].NumAllyAttackers;
	}

	return &pCPoints[pCPoints[group]->Points[index].itmpGr]->Points[pCPoints[group]->Points[index].itmp].NumEnemyAttackers;
}

int* aiBrain::GetCurCPtNumAlly(bool enemy)
{
	if (enemy)
	{
		return &pCPoints[iCurGr_CPoint]->Points[iCurPt_CPoint].NumAllyAttackers;
	}

	return &pCPoints[iCurGr_CPoint]->Points[iCurPt_CPoint].NumEnemyAttackers;
}

int* aiBrain::GetCPtNumEnemy(int group,int index,bool enemy)
{	
	return GetCPtNumAlly(group,index,!enemy);
}

int* aiBrain::GetCurCPtNumEnemy(bool enemy)
{
	return GetCurCPtNumAlly(!enemy);
}

bool aiBrain::IsCPocpStatist(int group,int index)
{
	CombatPoints::CombatPoint* c_point = &pCPoints[pCPoints[group]->Points[index].itmpGr]->Points[pCPoints[group]->Points[index].itmp];

	if (c_point->NumEnemyAttackers!=0 || c_point->NumAllyAttackers!=0)
	{							
		if (IsStatist())
		{
			if (!c_point->bIsStatistPoint) return false;
		}
		else
		{
			if (c_point->bIsStatistPoint) return false;
		}
	}

	return true;
}

void aiBrain::MoveAway(Vector pos)
{
	if (BrainState==EBS_MOVE_AWAY) return;

	Reset();	

	pThoughtPursuit->Activate(pos, WayPointsName, 0.5f, false);
	BrainState=EBS_MOVE_AWAY;	

	DeattachFromCP();
}

void aiBrain::ActivateObject(const ConstString & name,bool activate)
{
	MOSafePointer triger;		
	GetBody()->FindObject(name, triger);
	
	if (triger.Ptr())
	{
		triger.Ptr()->Activate(activate);
	}
}

void aiBrain::Command(dword numParams, const char ** params)
{
	if (numParams<2) return;

	if (string::IsEqual(params[0],"attack"))
	{
		pChrTarget = pChrPlayer;

		pThoughtKick->SetCurAction(params[1]);
		pThoughtKick->Activate(pChrTarget);
		BrainState=EBS_KICK;		
	}
}

float aiBrain::PursuitDist()
{
	return GetAttackDistance() * 0.8f;
}