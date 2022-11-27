#include "aiBombardeer.h"
#include "aiBrain.h"
#include "..\CharacterAI.h"
#include "../../character/components/CharacterLogic.h"
#include "../../character/components/CharacterPhysics.h"

class BombardeerParams : public CharacterControllerParams
{

public:

	float suicideRushDist;
	float fDistToShoot;
	float fDistofView;
	float fDistofViewPlayer;

	float fDeltaDistShoot;

	float fMinTimeToShoot;
	float fMaxTimeToShoot;

	int   iNumPlayerAttckers;
	int   iNumNPCAttckers;

	float fTimeToNextAttack;
	int   iMaxSimelouslyAttackers;

	ConstString WayPointsName;
	ConstString AIZoneName;
	ConstString Target;

	bool  isSentinel;	
	float fShootSektor; 

	float minTimeToSuicide; 
	float maxTimeToSuicide; 

	bool  bAutoSwitchFromSentinel;
	float fRaduisWhenSwitchFromSentinel;

	BombardeerParams()
	{
	}

	virtual ~BombardeerParams()
	{
	}

	virtual bool IsControllerSupport(const ConstString & controllerClassName)
	{
		CHECK_CONTROLLERSUPPORT("BombardeerParams")
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
		isSentinel=reader.Bool();

		suicideRushDist=reader.Float();
		fDistToShoot=reader.Float();
		fDistofView=reader.Float();
		fDistofViewPlayer=reader.Float();

		iNumPlayerAttckers=reader.Long();
		iNumNPCAttckers=reader.Long();

		fDeltaDistShoot=reader.Float();

		fMinTimeToShoot=reader.Float();
		fMaxTimeToShoot=reader.Float();	

		WayPointsName=reader.String();

		AIZoneName=reader.String();
		Target=reader.String();

		minTimeToSuicide=reader.Float();
		maxTimeToSuicide=reader.Float();

		bAutoSwitchFromSentinel = reader.Bool();;
		fRaduisWhenSwitchFromSentinel = reader.Float();

		fTimeToNextAttack = reader.Float();
		iMaxSimelouslyAttackers=reader.Long();
	}
};


MOP_BEGINLISTCG(BombardeerParams, "AI params Bombardeer", '1.00', 90, "AI params Bombardeer", "Character")
	MOP_FLOATEX("ShootSector", 25.0f, 5.0f, 90.0f)		
	MOP_BOOL("isSentinel", false)

	MOP_FLOATEX("Dist to suicide | v2 |", 8.0f, 1.0f, 256.0f)
	MOP_FLOATEX("MaxDist To Shoot", 7.0f, 3.0f, 256.0f)
	MOP_FLOATEX("Dist of View", 10.0f, 3.0f, 256.0f)
	MOP_FLOATEX("Dist of View Player", 40.0f, 3.0f, 256.0f)

	MOP_LONGEX("MAX Player Attckers", 5, 1, 20)
	MOP_LONGEX("MAX NPC Attckers", 1, 1, 20)

	MOP_FLOATEX("Delta Dist Shoot", 0.0f, 0.0f, 500.0f)

	MOP_FLOATEX("MinTime To Shoot", 0.75f, 0.5f, 20.0f)	
	MOP_FLOATEX("MaxTime To Shoot", 1.75f, 0.5f, 20.0f)

	MOP_STRING("WayPoints Name", "WayPoints")

	MOP_STRING("AIZone", "")
	MOP_STRING("Target", "")

	MOP_FLOATEX("MinTime To Suicide", 0.0f, 0.0f, 512.0f)	
	MOP_FLOATEX("MaxTime To Suicide", 0.0f, 0.0f, 512.0f)

	MOP_BOOL("AutoSwitchFromSentinel", false)
	MOP_FLOATEX("RaduisWhenSwitchFromSentinel", 5.555f, 0.5f, 100.0f)

	MOP_FLOATEX("Time To Next Attack", 5.0f, 1.0f, 1000.0f)
	MOP_LONGEX("Max Simelously Attackers", 1, 1, 20)
MOP_ENDLIST(BombardeerParams)


aiBombardeer::aiBombardeer(Character & ch, const char* szName) : aiBrain (ch, szName)
{	
	animListener.pBombardeer = this;

	chr.animation->SetEventHandler(&animListener, (AniEvent)(&AnimListener::Suicide), "AI Event");

	suicideRushDist = 2;
	fDistToShoot = 7;
	fDistofView = 10;
	fDistofViewPlayer = 10.0f;
	
	fMinTimeToShoot = 0.75f;
	fMaxTimeToShoot = 1.75f;
	
	BrainState=EBS_IDLE;

	static const ConstString defWayPoints("WayPoints"); 
	WayPointsName=defWayPoints;	

	fInitialAngle = 0.0f;

	fShootSektor = 25.0f * (PI / 180);

	bAngleHasBeenInit = false;

	isGuard = false;

	bAutoSwitchFromSentinel = false;
	fRaduisWhenSwitchFromSentinel = 5.555f;

	fTimeToNextAttack2 = 5.0f;
	fCurTimeToNextAttack = 0.0f;

	in_suicide = false;
	in_rush = false;

	iMaxSimelouslyAttackers = 1;
		
	pBombardeerDropBomb =   (aiBombardeerDropBomb*)AddThought(NEW aiBombardeerDropBomb(this));	
	pBombardeerPlaceBomb = (aiBombardeerPlaceBomb*)AddThought(NEW aiBombardeerPlaceBomb(this));		

	pThoughtKick->AddAction("Attack1");
	pThoughtKick->PrepareActions();	

	pThoughtStanding->AddAction("Angre");
	pThoughtStanding->AddAction("Klich");
	pThoughtStanding->AddAction("Pot");
	pThoughtStanding->AddAction("weakless1");
	pThoughtStanding->AddAction("weakless2");	
	pThoughtStanding->PrepareActions();
}

aiBombardeer::~aiBombardeer()
{
}

void aiBombardeer::SetParams(CharacterControllerParams * params)
{
	if (!params) return;

	fShootSektor=((BombardeerParams*)params)->fShootSektor * (PI / 180);
	isGuard=((BombardeerParams*)params)->isSentinel;	

	suicideRushDist = ((BombardeerParams*)params)->suicideRushDist;
	fDistToShoot = ((BombardeerParams*)params)->fDistToShoot;	
	fDistofView = ((BombardeerParams*)params)->fDistofView;
	fDistofViewPlayer = ((BombardeerParams*)params)->fDistofViewPlayer;

	iNumPlayerAttckers=((BombardeerParams*)params)->iNumPlayerAttckers;
	iNumNPCAttckers=((BombardeerParams*)params)->iNumNPCAttckers;

	fDistToShoot+=RRnd(-((BombardeerParams*)params)->fDeltaDistShoot,((BombardeerParams*)params)->fDeltaDistShoot);

	fMinTimeToShoot=((BombardeerParams*)params)->fMinTimeToShoot;
	fMaxTimeToShoot=((BombardeerParams*)params)->fMaxTimeToShoot;

	WayPointsName=((BombardeerParams*)params)->WayPointsName;

	AIZoneName=((BombardeerParams*)params)->AIZoneName;

	Target=((BombardeerParams*)params)->Target;
	
	timeToSuicide = RRnd(((BombardeerParams*)params)->minTimeToSuicide,((BombardeerParams*)params)->maxTimeToSuicide);
	curTimeToSuicide = timeToSuicide;

	bAutoSwitchFromSentinel = ((BombardeerParams*)params)->bAutoSwitchFromSentinel;
	fRaduisWhenSwitchFromSentinel = ((BombardeerParams*)params)->fRaduisWhenSwitchFromSentinel;

	fTimeToNextAttack2 = ((BombardeerParams*)params)->fTimeToNextAttack;
	iMaxSimelouslyAttackers = ((BombardeerParams*)params)->iMaxSimelouslyAttackers;

	GetBody()->physics->SetAllowMove(!isGuard);
}

void aiBombardeer::Init()
{
	in_suicide = false;
	in_rush = false;
	curTimeToSuicide = timeToSuicide;

	aiBrain::Init();
}

//Перезапустить контроллер при перерождении персонажа
void aiBombardeer::Reset()
{	
	if (!bAngleHasBeenInit)
	{
		fInitialAngle = GetBody()->physics->GetAy();
		bAngleHasBeenInit = true;
	}

	aiBrain::Reset();	
}

void aiBombardeer::Update(float dltTime)
{
	if (timeToSuicide>0.1f)
	{
		curTimeToSuicide -= dltTime;

		if (curTimeToSuicide<0)
		{
			curTimeToSuicide = 0.0f;
		}
	}

	aiBrain::Update(dltTime);
}

void aiBombardeer::GuardTick()
{
	if (pChrTarget!=NULL && bAutoSwitchFromSentinel)
	{
		float fDist2 = (pChrTarget->physics->GetPos() - GetBody()->physics->GetPos()).GetLength2();		

		if (fDist2 < Sqr(fRaduisWhenSwitchFromSentinel))
		{
			isGuard = false;

			GetBody()->physics->SetAllowMove(!isGuard);

			return;
		}
	}

	if (pChrTarget!=NULL)
	{
		if (!pChrTarget->IsShow())
		{			
			Reset();

			ExecuteThoughts(0);

			BrainState=EBS_IDLE;

			pChrTarget=NULL;
		}
		else
		if (!pChrTarget->IsActive())
		{
			Reset();

			ExecuteThoughts(0);

			BrainState=EBS_IDLE;
			pChrTarget=NULL;
		}
		else
		if (pChrTarget->logic->IsDead())// && IsEmpty())
		{
			Reset();

			ExecuteThoughts(0);

			BrainState=EBS_IDLE;

			pChrTarget=NULL;
		}
		else
		if (pChrTarget->logic->IsActor())
		{
			Reset();

			ExecuteThoughts(0);

			BrainState=EBS_IDLE;

			pChrTarget=NULL;
		}		
	}

	if (IsEmpty())
	{
		BrainState=EBS_IDLE;
	}

	if (BrainState==EBS_IDLE)
	{
		pChrTarget=SearchTarget(true);
		
		if (pChrTarget!=NULL)
		{				
			if ((GetBody()->physics->GetPos()-pChrTarget->physics->GetPos()).GetLengthXZ2() <= Sqr(GetAttackDistance()))
			{
				pThoughtKick->Activate(pChrTarget);
				BrainState=EBS_KICK;
			}
			else
			if ((GetBody()->physics->GetPos()-pChrTarget->physics->GetPos()).GetLengthXZ2() < Sqr(fDistToShoot))
			{
				pBombardeerDropBomb->Activate(pChrTarget,
											  GetAttackDistance(), fDistToShoot,
											  fMinTimeToShoot,fMaxTimeToShoot,false);
				BrainState=EBS_DROPBOMB;
			}		
		}
	}
}

void aiBombardeer::ActivateTargetPairStateThought()
{
	pThoughtStrafing->Activate( (Character*)pChrTarget, 15.0f);
	BrainState=EBS_WAIT_FATALITY;
}

void aiBombardeer::TickWhenEmpty()
{
	if (GoToFrustum(fDistToShoot * 0.8f)) return;

	if ((BrainState == EBS_KICK || BrainState==EBS_PLACEBOMB)&& pChrTarget)
	{
		Vector vDest;
		if (CanRunaway(vDest,fDistToShoot * 0.95f))
		{
			pThoughtPursuit->Activate(vDest, WayPointsName, 1.0f, false);
			BrainState=EBS_RUNAWAY;
		}
		else
	
		{
			pThoughtKick->Activate(pChrTarget);
			BrainState=EBS_KICK;
		}			
	}
	else		
	{
		BrainState=EBS_IDLE;
	}
}

void aiBombardeer::SuicideThought()
{
	if (in_rush)
	{
		if (GetBody()->logic->GetState() != CharacterLogic::state_run)
		{
			in_rush = false;
		}
	}

	float fDist2 = (GetBody()->physics->GetPos()-pChrTarget->physics->GetPos()).GetLengthXZ2();

	if (fDist2 <= Sqr(1.5f) && !in_rush)
	{
		chr.animation->ActivateLink("suicide boom");
		in_rush = true;
	}

	OrinentToTarget();
	
	if (fDist2 <= Sqr(suicideRushDist) && !in_rush)
	{		
		const char* act = "suicide catch";

		if (Rnd() > 0.5f && fDist2 <= Sqr(3.0f)) 
		{
			//act = "suicide rush";
			act = "Boom";
		}

		if (chr.animation->ActivateLink(act))
		{
			in_rush = true;
		}		
	}
	else
	if (fDist2 <= Sqr(fDistofView) && !in_rush)
	{
		chr.animation->ActivateLink("suicide");

		pThoughtPursuit->Activate(pChrTarget->physics->GetPos(), WayPointsName, GetAttackDistance() * 0.8f,true);
		BrainState=EBS_PURSUIT;	
	}	
}

bool aiBombardeer::AttackCharacter()
{
	if (!in_suicide)
	{
		if (curTimeToSuicide<0.01f && timeToSuicide>0.1f)
		{
			in_suicide = true;
		}
		else // FIX-ME сделать в паттерне
		if (GetBody()->logic->GetAbsoluteHP() < 0.8f)
		{
			if (Rnd()>0.5f) in_suicide = true;
		}
	}

	if (in_suicide)
	{
		SuicideThought();
		return true;
	}

	if (AllowAttack())
	{
		float fDist2 = (GetBody()->physics->GetPos()-pChrTarget->physics->GetPos()).GetLengthXZ2();

		if (fDist2 <= Sqr(GetAttackDistance()))
		{			
			float k = Rnd();				

			if (k<0.5f)
			{				
				Vector vDest;
				if (CanRunaway(vDest,fDistToShoot * 0.95f) && !GetBody()->IsBombBusy())
				{
					pBombardeerPlaceBomb->Activate();
					BrainState=EBS_PLACEBOMB;
				}
				else
				{
					pThoughtKick->Activate(pChrTarget);
					BrainState=EBS_KICK;
				}				
			}
			else
			{
				pThoughtKick->Activate(pChrTarget);
				BrainState=EBS_KICK;
			}
		}
		else
		if ((GetBody()->physics->GetPos()-pChrTarget->physics->GetPos()).GetLengthXZ2() < Sqr(fDistToShoot - 2.0f))
		{				
			float k = Rnd();				

			if (k<0.4f)
			{				
				Vector vDest;
				if (CanRunaway(vDest,fDistToShoot * 0.95f))
				{
					if (!GetBody()->IsBombBusy())
					{													
						pBombardeerPlaceBomb->Activate();
						BrainState=EBS_PLACEBOMB;
					}
					else
					{
						pThoughtPursuit->Activate(vDest, WayPointsName, 1.0f, false);	
						BrainState=EBS_RUNAWAY;
					}
				}
				else
				{
					pBombardeerDropBomb->Activate(pChrTarget,
												  GetAttackDistance(), fDistToShoot,
												  fMinTimeToShoot,fMaxTimeToShoot,false);
					BrainState=EBS_DROPBOMB;
				}
			}
			else
			if (k<0.8f)
			{
				pBombardeerDropBomb->Activate(pChrTarget,
											  GetAttackDistance(), fDistToShoot,
											  fMinTimeToShoot,fMaxTimeToShoot,false);
				BrainState=EBS_DROPBOMB;
			}
			else				
			{
				pThoughtStrafing->Activate(pChrTarget, Rnd(2)+2);	
				BrainState=EBS_STRAFING;
			}
		}
		else
		if ((GetBody()->physics->GetPos()-pChrTarget->physics->GetPos()).GetLengthXZ2() < Sqr(fDistToShoot))
		{			
			pBombardeerDropBomb->Activate(pChrTarget,
										  GetAttackDistance(), fDistToShoot,
										  fMinTimeToShoot,fMaxTimeToShoot,false);
			BrainState=EBS_DROPBOMB;
		}
		else
		{
			pThoughtPursuit->Activate(pChrTarget->physics->GetPos(), WayPointsName, fDistToShoot * 0.8f, true);
			BrainState=EBS_PURSUIT;	
		}				
	}
	else
	{
		if ((GetBody()->physics->GetPos()-pChrTarget->physics->GetPos()).GetLengthXZ2() < Sqr(fDistToShoot - 2.0f))
		{			
			Vector vDest;
			if (CanRunaway(vDest,fDistToShoot * 0.95f))
			{
				pThoughtPursuit->Activate(vDest, WayPointsName, 1.0f, false);	
				BrainState=EBS_RUNAWAY;					
			}
			else
			{
				pThoughtStrafing->Activate(pChrTarget, Rnd(2)+2);	
				BrainState=EBS_STRAFING;
			}
		}
		else				
		{
			pThoughtStrafing->Activate(pChrTarget, Rnd(2)+2);	
			BrainState=EBS_STRAFING;
		}
	}

	return true;
}

void aiBombardeer::Hit(float dmg, Character* _offender, const char* reaction, DamageReceiver::DamageSource source)
{
	if (bAutoSwitchFromSentinel && isGuard)
	{
		isGuard = false;
		GetBody()->physics->SetAllowMove(!isGuard);
	}

	if (BrainState!=EBS_KICK && BrainState!=EBS_PLACEBOMB && BrainState!=EBS_SUICIDE)
	{
		Reset();
			
		ExecuteThoughts(0);

		BrainState=EBS_IDLE;		

		pChrTarget=SearchTarget(false);

		if (pChrTarget!=NULL)
		{
			if ((GetBody()->physics->GetPos() - pChrTarget->physics->GetPos()).GetLengthXZ2() <= Sqr(GetAttackDistance()))
			{
				pThoughtKick->Activate(pChrTarget);
				BrainState=EBS_KICK;
			}
		}
	}
}

void aiBombardeer::Boom(const Vector & pos)
{
	chr.arbiter->Boom(&chr, DamageReceiver::ds_bomb, pos, 1.5f, 20.0f, 1.0f);

	//Рождаем эффекты
	/*IParticleSystem * p = null;	

	p = chr.Particles().CreateParticleSystem("ExplosionBomb.xps");
	chr.Sound().Create3D("bmb_blast", pos, _FL_);

	if(p)
	{
		p->Teleport(Matrix(Vector(0.0f), pos));
		p->AutoDelete(true);
	}*/
}

void aiBombardeer::Suicide(float dmg, float radius)
{
	Matrix mat;
	GetBody()->physics->GetMatrixWithBoneOffset(mat);

	chr.arbiter->BlowSelf(&chr, mat.pos, radius, dmg, 0.9f);
	chr.logic->SetHP(0.0f);
}

DeclareCharacterController(aiBombardeer, "Bombardeer", 11)