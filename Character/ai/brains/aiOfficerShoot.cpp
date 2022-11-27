#include "aiOfficerShoot.h"
#include "aiBrain.h"
#include "..\CharacterAI.h"
#include "../../character/components/CharacterLogic.h"
#include "../../character/components/CharacterPhysics.h"

class OfficerShootParams : public CharacterControllerParams
{

public:

	float fDistToShoot;
	float fDistofView;
	
	float fMinTimeToShoot;
	float fMaxTimeToShoot;
				
	bool  isSentinelMusketeer;	
	float fShootSektor; 

	float block_cooldown;

	int   iNumPlayerAttckers;
	int   iNumNPCAttckers;

	ConstString WayPointsName;

	ConstString AIZoneName;
	ConstString Target;

	OfficerShootParams()
	{
	}

	virtual ~OfficerShootParams()
	{
	}

	virtual bool IsControllerSupport(const ConstString & controllerClassName)
	{
		CHECK_CONTROLLERSUPPORT("OfficerShootParams")
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
	
		fMinTimeToShoot=reader.Float();
		fMaxTimeToShoot=reader.Float();	

		block_cooldown=reader.Float();

		iNumPlayerAttckers=reader.Long();
		iNumNPCAttckers=reader.Long();

		WayPointsName=reader.String();

		AIZoneName=reader.String();
		Target=reader.String();		
	}
};

MOP_BEGINLISTCG(OfficerShootParams, "AI params Officer Shoot", '1.00', 90, "AI params Musketeer", "Character")

	MOP_FLOATEX("ShootSector", 25.0f, 5.0f, 90.0f)		
	MOP_BOOL("isSentinel", false)

	MOP_FLOATEX("MaxDist To Shoot", 6.0f, 3.0f, 256.0f)
	MOP_FLOATEX("Dist of View", 10.0f, 3.0f, 256.0f)
	
	MOP_FLOATEX("MinTime To Shoot", 0.5f, 0.5f, 20.0f)	
	MOP_FLOATEX("MaxTime To Shoot", 1.2f, 0.5f, 20.0f)					

	MOP_FLOATEX("Block cooldown", 5.0f, 1.0f, 1024.0f)

	MOP_LONGEX("MAX Player Attckers", 5, 1, 20)
	MOP_LONGEX("MAX NPC Attckers", 1, 1, 20)

	MOP_STRING("WayPoints Name", "WayPoints")

	MOP_STRING("AIZone", "")
	MOP_STRING("Target", "")	

MOP_ENDLIST(OfficerShootParams)




aiOfficerShoot::aiOfficerShoot(Character & ch, const char* szName) : aiBrain (ch, szName)
{
	fDistToShoot = 7;
	fDistofView = 10;
	
	fMinTimeToShoot = 1.0f;
	fMaxTimeToShoot = 2.0f;
	
	BrainState=EBS_IDLE;

	static const ConstString defWayPoints("WayPoints"); 
	WayPointsName=defWayPoints;

	iNumPlayerAttckers = 4;
	iNumNPCAttckers = 1;

	pOfficerShootShoot = (aiOfficerShootShoot*)AddThought(NEW aiOfficerShootShoot(this));		
	pThoughtBlocking = (aiThoughtBlocking*)AddThought(NEW aiThoughtBlocking(this));	

	pThoughtBlocking->SetRunawayLink("Walk back");

	pThoughtKick->AddAction("Attack1");
	pThoughtKick->AddAction("Attack2");	
	pThoughtKick->PrepareActions();

	pThoughtStanding->AddAction("Angre");
	pThoughtStanding->AddAction("Klich");
	pThoughtStanding->AddAction("Pot");
	pThoughtStanding->AddAction("weakless1");
	pThoughtStanding->AddAction("weakless2");	
	pThoughtStanding->PrepareActions();
}

aiOfficerShoot::~aiOfficerShoot()
{
}

void aiOfficerShoot::SetParams(CharacterControllerParams * params)
{
	if (!params) return;	

	fDistToShoot=((OfficerShootParams*)params)->fDistToShoot;	

	fDistofView=((OfficerShootParams*)params)->fDistofView;	
	fDistofViewPlayer=fDistofView;	
	fDistToLoseTarget = fDistToLoseTarget + 5;

	fMinTimeToShoot=((OfficerShootParams*)params)->fMinTimeToShoot;
	fMaxTimeToShoot=((OfficerShootParams*)params)->fMaxTimeToShoot;
	
	block_timecooldown = ((OfficerShootParams*)params)->block_cooldown;

	iNumPlayerAttckers=((OfficerShootParams*)params)->iNumPlayerAttckers;
	iNumNPCAttckers=((OfficerShootParams*)params)->iNumNPCAttckers;

	WayPointsName=((OfficerShootParams*)params)->WayPointsName;
	AIZoneName=((OfficerShootParams*)params)->AIZoneName;

	WayPointsName=((OfficerShootParams*)params)->WayPointsName;
}

void aiOfficerShoot::ActivateTargetPairStateThought()
{
	pThoughtStanding->Activate( 15, pChrTarget);
	BrainState=EBS_WAIT_FATALITY;
}

//Обновить состояние контролера 10 раз в секунду (распределено между персонажами)
void aiOfficerShoot::TickWhenEmpty()
{		
	if (GoToFrustum(fDistToShoot * 0.8f)) return;

	BrainState=EBS_IDLE;	
}

bool aiOfficerShoot::AttackCharacter()
{
	float fDist=(GetBody()->physics->GetPos()-pChrTarget->physics->GetPos()).GetLengthXZ();

	if (fDist<=GetAttackDistance())
	{			
		pThoughtKick->Activate(pChrTarget);
		BrainState=EBS_KICK;
	}
	else	
	if (fDist<fDistToShoot * 1.15)
	{
		pOfficerShootShoot->Activate(pChrTarget,
									 GetAttackDistance(), fDistToShoot * 1.5f,
									 fMinTimeToShoot,fMaxTimeToShoot);
		BrainState=EBS_SHOOT;
	}
	else
	{
		pThoughtPursuit->Activate(pChrTarget->physics->GetPos(), WayPointsName, fDistToShoot * 0.8f, true);
		BrainState=EBS_PURSUIT;
	}

	return true;
}

void aiOfficerShoot::Hit(float dmg,Character* _offender, const char* reaction, DamageReceiver::DamageSource source)
{	
	aiBrain::Hit(dmg, _offender, reaction, source);

	iNumHits++;
	fHitTime = 0;

	bool blocking = StartBlocking(_offender, source);

	if (!blocking && Rnd()<0.45f && BrainState!=EBS_KICK)
	{	
		pThoughtKick->Activate(pChrTarget);
		BrainState=EBS_KICK;
	}	
}

void aiOfficerShoot::Shoot(float dmg)
{
	if (!pChrTarget)
	{
		GetBody()->logic->SetShootTarget(NULL,"Hit");	
		return;
	}	

	CharacterLogic::State trg_st = pChrTarget->logic->GetState();

	GetBody()->logic->SetShootTarget(pChrTarget,"Hit");		

	if ( trg_st == CharacterLogic::state_roll)// || trg_st == CharacterLogic::state_run)
	{
		GetBody()->logic->SetShootTarget(NULL,"Hit");
	}

	if (pChrTarget->IsPlayer())
	{			
		if (!GetBody()->arbiter->IsSphereInFrustrum(GetBody()->physics->GetPos(), 0.5f))
		{
			GetBody()->logic->SetShootTarget(NULL,"Hit");			
		}
	}	
}

DeclareCharacterController(aiOfficerShoot, "OfficerShoot", 11)