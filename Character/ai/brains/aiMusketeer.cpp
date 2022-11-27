#include "aiMusketeer.h"
#include "aiBrain.h"
#include "..\CharacterAI.h"
#include "../../character/components/CharacterLogic.h"
#include "../../character/components/CharacterPhysics.h"

class MusketeerParams : public CharacterControllerParams
{

public:

	float fDistToShoot;
	float fDistofView;

	float fDeltaDistShoot;
	
	float fMinTimeToShoot;
	float fMaxTimeToShoot;
				
	float fWaitTime;

	bool  isSentinelMusketeer;	
	float fShootSektor; 

	int   iNumPlayerAttckers;
	int   iNumNPCAttckers;

	ConstString WayPointsName;

	ConstString AIZoneName;
	ConstString Target;

	bool  bAutoSwitchFromSentinel;
	float fRaduisWhenSwitchFromSentinel;

	MusketeerParams()
	{
	}

	virtual ~MusketeerParams()
	{
	}

	virtual bool IsControllerSupport(const ConstString & controllerClassName)
	{
		CHECK_CONTROLLERSUPPORT("MusketeerParams")
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
		isSentinelMusketeer=reader.Bool();

		fDistToShoot=reader.Float();
		fDistofView=reader.Float();

		fDeltaDistShoot=reader.Float();
	
		fMinTimeToShoot=reader.Float();
		fMaxTimeToShoot=reader.Float();	

		fWaitTime=reader.Float();

		iNumPlayerAttckers=reader.Long();
		iNumNPCAttckers=reader.Long();

		WayPointsName=reader.String();

		AIZoneName=reader.String();
		Target=reader.String();

		bAutoSwitchFromSentinel = reader.Bool();;
		fRaduisWhenSwitchFromSentinel = reader.Float();
	}
};

MOP_BEGINLISTCG(MusketeerParams, "AI params Musketeer", '1.00', 90, "AI params Musketeer", "Character")

	MOP_FLOATEX("ShootSector", 25.0f, 5.0f, 90.0f)		
	MOP_BOOL("isSentinel", false)
	
	MOP_FLOATEX("MaxDist To Shoot", 6.0f, 3.0f, 500.0f)
	MOP_FLOATEX("Dist of View", 10.0f, 3.0f, 500.0f)
	
	MOP_FLOATEX("Delta Dist Shoot", 0.0f, 0.0f, 500.0f)

	MOP_FLOATEX("MinTime To Shoot", 0.5f, 0.5f, 20.0f)	
	MOP_FLOATEX("MaxTime To Shoot", 1.2f, 0.5f, 20.0f)					

	MOP_FLOATEX("WaitTime to shoot", 0.0f, 0.0f, 20.0f)					

	MOP_LONGEX("MAX Player Attckers", 5, 1, 20)
	MOP_LONGEX("MAX NPC Attckers", 1, 1, 20)

	MOP_STRING("WayPoints Name", "WayPoints")

	MOP_STRING("AIZone", "")
	MOP_STRING("Target", "")

	MOP_BOOL("AutoSwitchFromSentinel", false)
	MOP_FLOATEX("RaduisWhenSwitchFromSentinel", 5.555f, 0.5f, 100.0f)

MOP_ENDLIST(MusketeerParams)




aiMusketeer::aiMusketeer(Character & ch, const char* szName) : aiBrain (ch, szName)
{
	fDistToShoot = 6;
	fDistofView = 10;
	
	fMinTimeToShoot = 1.0f;
	fMaxTimeToShoot = 2.0f;
	
	BrainState=EBS_IDLE;

	static const ConstString defWayPoints("WayPoints"); 
	WayPointsName=defWayPoints;

	isGuard = false;
	
	fInitialAngle = 0.0f;

	fShootSektor = 25.0f * (PI / 180);

	bAngleHasBeenInit = false;

	iNumPlayerAttckers = 4;
	iNumNPCAttckers = 1;	

	bAutoSwitchFromSentinel = false;
	fRaduisWhenSwitchFromSentinel = 5.555f;

	fWaitTime =0.0f;
		
	allow_kick = false;

	pMushketerShoot =           (aiMushketerShoot*)AddThought(NEW aiMushketerShoot(this));	
	
	pThoughtKick->AddAction("Attack1");	
	pThoughtKick->PrepareActions();

	pThoughtStanding->AddAction("Angre");	
	pThoughtStanding->PrepareActions();	
}

aiMusketeer::~aiMusketeer()
{
}

void aiMusketeer::SetParams(CharacterControllerParams * params)
{
	if (!params) return;	

	fShootSektor=((MusketeerParams*)params)->fShootSektor * (PI / 180);
	isGuard=((MusketeerParams*)params)->isSentinelMusketeer;	

	fDistToShoot=((MusketeerParams*)params)->fDistToShoot;	
	fDistofView=((MusketeerParams*)params)->fDistofView;

	fDistofViewPlayer=fDistofView;
	fDistToLoseTarget = fDistToLoseTarget + 5;

	fDistToShoot+=RRnd(-((MusketeerParams*)params)->fDeltaDistShoot,((MusketeerParams*)params)->fDeltaDistShoot);	

	fMinTimeToShoot=((MusketeerParams*)params)->fMinTimeToShoot;
	fMaxTimeToShoot=((MusketeerParams*)params)->fMaxTimeToShoot;
	
	fWaitTime=((MusketeerParams*)params)->fWaitTime;

	iNumPlayerAttckers=((MusketeerParams*)params)->iNumPlayerAttckers;
	iNumNPCAttckers=((MusketeerParams*)params)->iNumNPCAttckers;

	WayPointsName=((MusketeerParams*)params)->WayPointsName;
	AIZoneName=((MusketeerParams*)params)->AIZoneName;

	Target=((MusketeerParams*)params)->Target;

	bAutoSwitchFromSentinel = ((MusketeerParams*)params)->bAutoSwitchFromSentinel;
	fRaduisWhenSwitchFromSentinel = ((MusketeerParams*)params)->fRaduisWhenSwitchFromSentinel;

	GetBody()->physics->SetAllowMove(!isGuard);

	fDistToReconstructPath = fDistToShoot * 1.25f;
}

//Перезапустить контроллер при перерождении персонажа
void aiMusketeer::Reset()
{	
	if (!bAngleHasBeenInit)
	{
		fInitialAngle = GetBody()->physics->GetAy();
		bAngleHasBeenInit = true;
	}

	aiBrain::Reset();	
}

void aiMusketeer::GuardTick()
{
	Character* NPC = GetBody();	

	if (pChrTarget!=NULL && bAutoSwitchFromSentinel)
	{
		float fDist2 = (pChrTarget->physics->GetPos() - GetBody()->physics->GetPos()).GetLength2();

		if (fDist2 < Sqr(fRaduisWhenSwitchFromSentinel))
		{
			isGuard = false;
			GetBody()->physics->SetAllowMove(!isGuard);

			Reset();
			ExecuteThoughts(0);
			BrainState=EBS_IDLE;

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
		if (pChrTarget->logic->IsPairMode())
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
		/*else
		if (-fShootSektor+fInitialAngle>GetBody()->physics->GetAy()||
			 GetBody()->physics->GetAy()>fShootSektor+fInitialAngle)
		{
			Reset();

			ExecuteThoughts(0);

			BrainState=EBS_IDLE;

			pChrTarget=NULL;
		}*/
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
			if ((GetBody()->physics->GetPos()-pChrTarget->physics->GetPos()).GetLengthXZ2() < Sqr(fDistToShoot))
			{			
				pMushketerShoot->Activate(pChrTarget,
										  0.1f, fDistToShoot * 1.2f,
										  fMinTimeToShoot,fMaxTimeToShoot,fWaitTime, false);
				BrainState=EBS_SHOOT;
			}		
		}
	}
}

void aiMusketeer::ActivateTargetPairStateThought()
{
	pThoughtStrafing->Activate( (Character*)pChrTarget, 15.0f);
	BrainState=EBS_WAIT_FATALITY;
}

void aiMusketeer::Tick()
{
	if (isGuard)
	{
		GuardTick();
	}
	else
	{
		aiBrain::Tick();
	}
}

//Обновить состояние контролера 10 раз в секунду (распределено между персонажами)
void aiMusketeer::TickWhenEmpty()
{	
	if (GoToFrustum(fDistToShoot * 0.8f)) return;

	if (pChrTarget)
	{		
		if (BrainState==EBS_RUNAWAY)
		{
			pMushketerShoot->Activate(pChrTarget,
									  fDistToShoot*0.4f, fDistToShoot * 1.2f ,
									  fMinTimeToShoot,fMaxTimeToShoot,fWaitTime, true);
			BrainState=EBS_SHOOT;

			return;
		}
	}

	if (BrainState == EBS_KICK && pChrTarget)
	{
		Vector vDest;
		if (CanRunaway(vDest,fDistToShoot * 0.95f))
		{
			pThoughtPursuit->Activate(vDest, WayPointsName, 1.0f, false);
			BrainState=EBS_RUNAWAY;
			allow_kick = true;
		}
		else
		{
			pMushketerShoot->Activate(pChrTarget,
									  0.2f, fDistToShoot * 1.2f,
									  fMinTimeToShoot,fMaxTimeToShoot,fWaitTime, true);
			BrainState=EBS_SHOOT;
			//BrainState=EBS_IDLE;
		}
	}
	else
	{
		BrainState=EBS_IDLE;
	}
}

bool aiMusketeer::AttackCharacter()
{
	float fDist2 = (GetBody()->physics->GetPos()-pChrTarget->physics->GetPos()).GetLength2();

	if (fDist2 <= Sqr(GetAttackDistance()) && allow_kick)
	{
		Reset();

		pThoughtKick->Activate(pChrTarget);
		BrainState=EBS_KICK;

		allow_kick = false;

		return true;
	}
	else
	if (fDist2 < Sqr(fDistToShoot * 0.7f))
	{	
		Vector vDest;
		if (CanRunaway(vDest, fDistToShoot * 0.95f))
		{
			pThoughtPursuit->Activate(vDest, WayPointsName, 1.0f, false);	
			BrainState=EBS_RUNAWAY;
			allow_kick = true;
		}
		else
		{
			pMushketerShoot->Activate(pChrTarget,
									  GetAttackDistance(), fDistToShoot * 1.2f,
									  fMinTimeToShoot,fMaxTimeToShoot,fWaitTime, true);
			BrainState=EBS_SHOOT;
		}

		return true;
	}
	else
	if (fDist2 < Sqr(fDistToShoot))
	{
		Reset();			

		pMushketerShoot->Activate(pChrTarget,
								  GetAttackDistance(), fDistToShoot * 1.2f,
								  fMinTimeToShoot,fMaxTimeToShoot,fWaitTime, true);
		BrainState=EBS_SHOOT;

		return true;
	}	
	else
	{
		pThoughtPursuit->Activate(pChrTarget->physics->GetPos(), WayPointsName, fDistToShoot * 0.8f, true);
		BrainState=EBS_PURSUIT;		
	}

	return true;
}

void aiMusketeer::Hit(float dmg,Character* _offender, const char* reaction, DamageReceiver::DamageSource source)
{
	if (bAutoSwitchFromSentinel && isGuard)
	{
		isGuard = false;
		GetBody()->physics->SetAllowMove(!isGuard);

		Reset();
		ExecuteThoughts(0);
		BrainState=EBS_IDLE;
	}

	if (pChrPlayer != _offender) return;

	if (BrainState!=EBS_KICK && allow_kick)
	{
		Reset();
			
		ExecuteThoughts(0);

		BrainState=EBS_IDLE;		

		pChrTarget=SearchTarget(false);

		if (pChrTarget!=NULL)
		{
			if ((GetBody()->physics->GetPos()-pChrTarget->physics->GetPos()).GetLengthXZ2() <= Sqr(GetAttackDistance()))
			{
				pThoughtKick->Activate(pChrTarget);
				BrainState=EBS_KICK;

				allow_kick = false;
			}
		}
	}
	else
	{
		if (!isGuard)
		{
			Vector vDest;
			if (CanRunaway(vDest, fDistToShoot * 0.95f))
			{
				pThoughtPursuit->Activate(vDest, WayPointsName, 1.0f, false);	
				BrainState=EBS_RUNAWAY;
				allow_kick = true;
			}
		}
	}
}

DeclareCharacterController(aiMusketeer, "Musketeer", 11)