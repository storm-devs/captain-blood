#include "aiCannon.h"
#include "aiBrain.h"
#include "..\CharacterAI.h"
#include "../../character/components/CharacterLogic.h"
#include "../../character/components/CharacterPhysics.h"

class CannonParams : public CharacterControllerParams
{

public:

	float fDistDeadZone;
	float fDistToShoot;	

	float fMinTimeToShoot;
	float fMaxTimeToShoot;
	
	float fShootSektor; 

	float fBombSpeed;

	CannonParams()
	{
	}

	virtual ~CannonParams()
	{
	}

	virtual bool IsControllerSupport(const ConstString & controllerClassName)
	{
		CHECK_CONTROLLERSUPPORT("CanonParams")
	}

	bool Create(MOPReader & reader)
	{
		Init(reader);
		return true;
	}

	bool EditMode_Create(MOPReader & reader)
	{
		Init(reader);
		return true;
	}

	bool EditMode_Update(MOPReader & reader)
	{
		Init(reader);
		return true;
	}


	void Init (MOPReader & reader)
	{
		fShootSektor=reader.Float();		

		fDistDeadZone=reader.Float();
		fDistToShoot=reader.Float();

		fMinTimeToShoot=reader.Float();
		fMaxTimeToShoot=reader.Float();	

		fBombSpeed=reader.Float();	
	}
};


MOP_BEGINLISTCG(CannonParams, "AI params CanonParams", '1.00', 90, "AI params CanonParams", "Character")

MOP_FLOATEX("ShootSector", 25.0f, 5.0f, 90.0f)		

MOP_FLOATEX("MaxDist of DeadZone", 2.0f, 1.0f, 50.0f)
MOP_FLOATEX("MaxDist To Shoot", 15.0f, 3.0f, 256.0f)

MOP_FLOATEX("MinTime To Shoot", 6.0f, 0.5f, 100.0f)	
MOP_FLOATEX("MaxTime To Shoot", 6.0f, 0.5f, 100.0f)

MOP_FLOATEX("Bomb Speed", 7.0f, 0.5f, 1024.0f)

MOP_ENDLIST(CannonParams)


aiCannon::aiCannon(Character & ch, const char* szName) : aiBrain (ch, szName)
{
	fDistDeadZone = 2;
	fDistToShoot = 15;	
	
	fMinTimeToShoot = 6.0f;
	fMaxTimeToShoot = 6.0f;
	
	BrainState=EBS_IDLE;

	fInitialAngle = ch.physics->GetAy();

	fShootSektor = 85.0f * (PI / 180);

	bAngleHasBeenInit = false;

	fTimeToShoot = RRnd(fMinTimeToShoot,fMaxTimeToShoot);
}

aiCannon::~aiCannon()
{
}

void aiCannon::SetParams(CharacterControllerParams * params)
{
	if (!params) return;

	fShootSektor=((CannonParams*)params)->fShootSektor * (PI / 180);

	fDistDeadZone = ((CannonParams*)params)->fDistDeadZone;
	fDistToShoot=((CannonParams*)params)->fDistToShoot;		

	fMinTimeToShoot=((CannonParams*)params)->fMinTimeToShoot;
	fMaxTimeToShoot=((CannonParams*)params)->fMaxTimeToShoot;

	fTimeToShoot = RRnd(fMinTimeToShoot,fMaxTimeToShoot);

	GetBody()->SetBombSpeed(((CannonParams*)params)->fBombSpeed);
	GetBody()->SetUseBombPhysics(false);

	fInitialAngle = chr.physics->GetAy();
}

//Перезапустить контроллер при перерождении персонажа
void aiCannon::Reset()
{
	fTimeToShoot = RRnd(fMinTimeToShoot,fMaxTimeToShoot);

	if (!bAngleHasBeenInit)
	{
		fInitialAngle = GetBody()->physics->GetAy();
		bAngleHasBeenInit = true;
	}
	aiBrain::Reset();
}

void aiCannon::Update(float dltTime)
{
	fTimeToShoot -= dltTime;

	if (fTimeToShoot<0)
	{
		fTimeToShoot = 0;
	}

	aiBrain::Update(dltTime);
}

void aiCannon::Tick()
{	
	if (pChrTarget==NULL)
	{
		pChrTarget=SearchTarget(true);
	}

	if (pChrTarget!=NULL)
	{
		if (!pChrTarget->IsShow())
		{			
			Reset();			

			BrainState=EBS_IDLE;
			pChrTarget=NULL;
		}
		else
		if (!pChrTarget->IsActive())
		{
			Reset();			

			BrainState=EBS_IDLE;
			pChrTarget=NULL;
		}
		else
		if (pChrTarget->logic->IsDead() && IsEmpty())
		{
			Reset();			

			BrainState=EBS_IDLE;
			pChrTarget=NULL;
		}
		else
		if (pChrTarget->logic->IsActor())
		{
			Reset();

			BrainState=EBS_IDLE;
			pChrTarget=NULL;
		}
		else
		{
			Vector dir = Vector(sin(fInitialAngle),0,cos(fInitialAngle));

			Vector dir2target = (pChrTarget->physics->GetPos() - GetBody()->physics->GetPos());			
			float angle  = dir.GetAngleXZ(dir2target);
			
			if (fabs(angle)>fShootSektor * 0.5f ||dir2target.GetLength()>fDistToShoot)
			{
				Reset();
	
				BrainState=EBS_IDLE;
				pChrTarget=NULL;
			}
		}
	}	

	if (pChrTarget)
	{		
		Vector dir = Vector(sin(fInitialAngle),0,cos(fInitialAngle));

		float angle  = dir.GetAngle((pChrTarget->physics->GetPos() - GetBody()->physics->GetPos()));		

		if (fabs(angle)<fShootSektor * 0.5f&& BrainState!=EBS_KICK)
		{
			GetBody()->physics->Orient(pChrTarget->physics->GetPos());
		}

		float fDist = GetPowDistToTarget();

		if (fTimeToShoot==0 && (fDistDeadZone*fDistDeadZone<=fDist && fDist<=fDistToShoot*fDistToShoot))
		{
			if (!GetBody()->IsBombBusy())
			{
				fTimeToShoot = RRnd(fMinTimeToShoot,fMaxTimeToShoot);

				IPhysicsScene::RaycastResult res;

				IPhysBase* pRayCastRes = GetBody()->Physics().Raycast(pChrTarget->physics->GetPos(),
																	  GetBody()->physics->GetPos(),
																	  phys_mask(phys_player),&res);
				
				if (!pRayCastRes || (pRayCastRes && res.mtl == pmtlid_air))
				{					
					Matrix mtx;
					GetBody()->SetBombTarget(pChrTarget->GetMatrix(mtx).pos);

					GetBody()->animation->ActivateLink("Drop Bomb");					
				}
				else
				{
					GetBody()->animation->ActivateLink("fake shoot");
				}
			}		
		}
	}	
}

void aiCannon::Boom(const Vector & pos)
{
	chr.arbiter->Boom(&chr, DamageReceiver::ds_cannon, pos, 1.5f, 20.0f, 1.0f);

	//Рождаем эффекты
	IParticleSystem * p = null;	

	p = chr.Particles().CreateParticleSystem("ExplosionBomb");
	chr.Sound().Create3D("bmb_blast", pos, _FL_);

	if(p)
	{
		p->Teleport(Matrix(Vector(0.0f), pos));
		p->AutoDelete(true);
	}
}

Character* aiCannon::SearchTarget(bool bCheckMaxAllowedAttackers)
{
	Character* pTarget;

	Character* NPC = GetBody();				
			
	//MissionObject* pChrPlayer = GetBody()->Mission().Player();
		
	//Если не сам себя нашел...
	{
		const array<Character*> &char_list = chr.arbiter->GetActiveCharacters();
		
		{
			CircleSearch(NPC->physics->GetPos(), fDistToShoot, MG_CHARACTER);

			// Filter Characters

			for (int i = 0; i < (int)searchResult.Size(); i++)
			{
				pTarget = searchResult[i].chr;

				if (!pTarget->IsShow())
				{
					pTarget=NULL;
					//В невидимых тоже не стреляем
					searchResult.DelIndex(i);
					i--;
					continue;
				}

				if (!pTarget->IsActive())
				{
					pTarget=NULL;
					//В неактивных тоже не стреляем
					searchResult.DelIndex(i);
					i--;
					continue;
				}

				if (pTarget->logic->IsDead())
				{
					pTarget=NULL;
					//В мертвых тоже не стреляем
					searchResult.DelIndex(i);
					i--;
					continue;
				}

				if (!pTarget->logic->IsEnemy(NPC))
				{
					pTarget=NULL;
					//В мертвых тоже не стреляем
					searchResult.DelIndex(i);
					i--;
					continue;
				}

				if (pTarget->logic->IsActor())
				{
					pTarget=NULL;
					//В мертвых тоже не стреляем
					searchResult.DelIndex(i);
					i--;
					continue;
				}

				if (pTarget->logic->GetSide() == CharacterLogic::s_boss)
				{
					pTarget=NULL;
					//В мертвых тоже не стреляем
					searchResult.DelIndex(i);
					i--;
					continue;
				}


				{			
					Vector vDir=GetBody()->physics->GetPos()-pTarget->physics->GetPos();

					vDir.Normalize();

					Vector vDestination = GetBody()->physics->GetPos() + vDir * (fDistToShoot - (GetBody()->physics->GetPos()-pTarget->physics->GetPos()).GetLengthXZ());

					IPhysBase* pRayCastRes=NULL;

					Vector vHeight(0.0f, 0.8f, 0.0f);

					bool bCanReach = true;

					pRayCastRes=NULL;

					IPhysicsScene::RaycastResult res;

					pRayCastRes = GetBody()->Physics().Raycast(GetBody()->physics->GetPos()+vHeight,
															   pTarget->physics->GetPos()+vHeight,
															   phys_mask(phys_player),&res);
					
					if (pRayCastRes && res.mtl != pmtlid_air) bCanReach = false;

					pRayCastRes = GetBody()->Physics().Raycast(vDestination+vHeight,
															   pTarget->physics->GetPos()+vHeight,
															   phys_mask(phys_player),&res);
					
					if (pRayCastRes && res.mtl != pmtlid_air) bCanReach = false;

					if (!bCanReach)
					{					
						pTarget=NULL;
						//В мертвых тоже не стреляем
						searchResult.DelIndex(i);
						i--;
						continue;
					}
				}
			}

			BrainState=EBS_IDLE;

			pTarget=NULL;

			for (int i = 0; i < (int)searchResult.Size(); i++)
			{
				pTarget = (Character*)searchResult[i].chr;					

				int iAttackersCount=1;
				
				bool IsTargetPlayer=false;

				if (pChrPlayer==pTarget)
				{
					iAttackersCount=5;
					IsTargetPlayer=true;
				}

				if (!chr.arbiter->CheckNumAttckers(pTarget, iAttackersCount,(char*)Name()))
				{
					pTarget=NULL;
				}
				
				if (pTarget!=NULL)
				{					
					break;
				}
			}
		}
	}	

	return pTarget;
}

DeclareCharacterController(aiCannon, "Cannon", 11)