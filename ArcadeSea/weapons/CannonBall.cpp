//////////////////////////////////////////////////////////////////////////
// Modenov Ivan 2006
//////////////////////////////////////////////////////////////////////////
#include "..\pch.h"
#include "CannonBall.h"
#include "Weapon.h"
#include "..\Ships\Ship.h"

#include "..\..\Common_h\Physics.h"
#include "..\..\Physics\PhysicsService.h"

#include "CannonBallBatcher.h"
#include "Cannon.h"

#include "..\TargetPoints.h"


CannonBall::CannonBall(Weapon * parent) :
collisionDetected_(false),
active_(false),
model_(NULL),
parent_ (parent)
//debugTrack_(__FILE__, __LINE__)
{
	trail = null;
	m_nBatcherID = -1;

	flySound = "";
	pFlySound = null;

	pTargPoints_ = NULL;
	pointIdx_ = -1;
	fFlySoundDelay = 0.f;
}

CannonBall::CannonBall(const CannonBall& ball)
//debugTrack_(__FILE__, __LINE__)
{
	trail = null;

	flySound = "";
	pFlySound = null;

	doCheckCollision_ = true;

	*this = ball;
}

const CannonBall& CannonBall::operator =(const CannonBall& ball)
{
	parent_					= ball.parent_;

	solver_					= ball.solver_;
	collisionDetected_		= ball.collisionDetected_;
	active_					= ball.active_;
	damage_					= ball.damage_;
	prevPos_				= ball.prevPos_;

	waterHitSFX_			= ball.waterHitSFX_;

	SetFlySound(ball.flySound);

//	debugTrack_				= ball.debugTrack_;

	SetModel(ball.model_);

	m_nBatcherID = ball.m_nBatcherID;

	pTargPoints_ = ball.pTargPoints_;
	pointIdx_ = ball.pointIdx_;

	doCheckCollision_ = ball.doCheckCollision_;

	return *this;
}

void CannonBall::SetFlySound(const char* pcFlySound)
{
	flySound = pcFlySound;
	RELEASE(pFlySound);

	if( flySound && parent_ && parent_->GetOwner() )
	{
		pFlySound = parent_->GetOwner()->Sound().Create3D(flySound,prevPos_,_FL_,false,false);
		fFlySoundDelay = solver_.GetPrecalculateTotalTime() - parent_->GetFlySoundTime();
	}
}

CannonBall::~CannonBall(void)
{
	if( trail )
		trail->Release();
	trail = 0;
	RELEASE(pFlySound);
}

// расчитывает есть ли столкновения ядра с другими объектами
void CannonBall::ComputeCollision(const Vector& v1, const Vector& v2)
{
	dword n = parent_->GetOwner()->QTFindObjects(MG_DAMAGEACCEPTOR, v1, v2);

	dword hq = 0;
	DamageReceiver* aHitsObj[100];

	for (dword i=0; i<n && hq<100; i++)
	{
		MissionObject * mo = &parent_->GetOwner()->QTGetObject(i)->GetMissionObject();

		if ( mo == parent_->GetOwner() )
			continue;
		if ( !mo->IsShow() )
			continue;
		if ( !mo->IsActive() )
			continue;
		if ( mo->IsDead() )
			continue;

		aHitsObj[hq++] = (DamageReceiver*)mo;
	}

	for( i=0; i<hq; i++ )
	{
		collisionDetected_ = aHitsObj[i]->Attack( parent_->GetOwner(), DamageReceiver::ds_cannon, damage_, v1, v2 );
		if( collisionDetected_ && parent_ && parent_->GetOwner() )
		{
			static const ConstString typeId("Ship");
			if( parent_->GetOwner()->Is(typeId) )
			{
				((Ship*)parent_->GetOwner())->GetStatistics().AddStatistic( ShipStatistics::stt_makedDamage, damage_ );
				((Ship*)parent_->GetOwner())->GetStatistics().SetStatistic(	ShipStatistics::stt_lastDamageTime, 0.f );
			}
		}
		if( collisionDetected_ )
			InActivate();
	}
}

// выстреливает ядро по траектории
bool CannonBall::ShootBall(const Vector& from, const Vector& to, float speed, float minAngle, float maxAngle)
{
	bool result = solver_.SetTrajectoryParams(from, to, speed, minAngle, maxAngle);
	if( result )
	{
		prevPos_ = from;
		collisionDetected_ = false;
		active_ = true;

		//debugTrack_.DelAll();
		//debugTrack_.Add(RS_LINE(from, 0xFF00FF00));
	}

	return result;
}

void CannonBall::SetTrajectory(const Vector& from, const Vector& to, float minTime, float minHeight)
{
	if( solver_.SetTrajectoryParamsByTime(from, to, minHeight, minTime) )
	{
		prevPos_ = from;
		collisionDetected_ = false;
		active_ = true;
	}
}

// симулирует полет
void CannonBall::Step(float deltaTime)
{
	if( !active_ )
		return;

	if( trail && solver_.GetTime() == 0.0f )
		trail->Reset();

	solver_.Step(deltaTime);
	Vector curPos = solver_.GetPosition();
	
//	debugTrack_.Add(RS_LINE(curPos, 0xFF00FF00));

	if( doCheckCollision_ )
		ComputeCollision(prevPos_, curPos);
	else
	{
		// время вышло - долетели куда надо
		if( solver_.GetTime() >= solver_.GetPrecalculateTotalTime() )
		{
			pTargPoints_->MakePointDamage(pointIdx_);
			InActivate();
		}
	}

	if( trail && !collisionDetected_ )
	{
		Matrix m;

		m.BuildOriented(curPos,curPos + curPos - prevPos_,Vector(0.0f,0.1f,0.0f));

		trail->Update(m);
	}

	if( pFlySound )
	{
		if( pFlySound->IsPlay() )
			pFlySound->SetPosition(curPos);
		else
		{
			fFlySoundDelay -= deltaTime;
			if( fFlySoundDelay <= 0.f )
			{
				pFlySound->SetPosition(curPos);
				pFlySound->Play();
			}
		}
	}

	prevPos_ = curPos;

	// если ушли под воду - то деактивируемся
	if (curPos.y < 0.0f)
	{
		parent_->GetOwner()->Particles().CreateParticleSystemEx2(waterHitSFX_, Matrix().BuildPosition(curPos), true, __FILE__, __LINE__);

		InActivate();

		if( ((Cannon*)parent_)->GetWaterHitSound() )
			parent_->GetOwner()->Sound().Create3D( ((Cannon*)parent_)->GetWaterHitSound(), curPos, _FL_ );
	}
}

void CannonBall::InActivate()
{
	active_ = false;

	if( trail )
		trail->Release();
	trail = 0;

	if( pFlySound )
		pFlySound->Release();
	pFlySound = 0;

	// сбросим точку прицеливания (ядро долетело)
	if( pTargPoints_ && pointIdx_ >= 0 )
	{
		pTargPoints_->ResetPoint(pointIdx_);
	}
}

void CannonBall::SetTrailParams(ITrailManager* tm, float fInitSize,float fFinalSize,float fMinFadeout,float fMaxFadeout,const Color& cColor,float fOffsetStrength)
{
	if( !trail )
	{
		Assert(tm)
		trail = tm->Add();
		Assert(trail);
	}
	if( trail )
		trail->SetParams(fInitSize,fFinalSize,fMinFadeout,fMaxFadeout,cColor.GetDword(),fOffsetStrength);
}

void CannonBall::SetModel( IGMXScene* model )
{
	model_ = model;

	if( !model_ )
		return;
}
