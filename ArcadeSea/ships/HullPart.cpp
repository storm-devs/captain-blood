#include "..\pch.h"
#include ".\HullPart.h"
#include "..\..\Common_h\Mission.h"
#include "Ship.h"
#include "PartsClassificator.h"
#include "..\asspecial\clothburns.h"

// конструктор
HullPart::HullPart(void) :
isDamaged_(false)
{
	m_nDamageRecursionCounter = 0;
}

// конструктор копий
HullPart::HullPart(const HullPart& other) :
isDamaged_(false)
{
	params_ = other.params_;
	m_nDamageRecursionCounter = 0;
}

// конструктор по параметрам
HullPart::HullPart(const HullPart::Params& params) :
isDamaged_(false)
{
	params_ = params;
	params_.startHP_ = params_.hp_;
	m_nDamageRecursionCounter = 0;
}
							
// оператор присваивания
HullPart& HullPart::operator=(const HullPart& other)
{
	params_ = other.params_;
	m_nDamageRecursionCounter = 0;
	return *this;
}

// деструктор
HullPart::~HullPart(void)
{
}

// восстановление изначального вида части
void HullPart::Rebuild(const HullPart::Params& params)
{
	ShipPart::Rebuild();

	params_ = params;
	params_.startHP_ = params_.hp_;
	isDamaged_ = false;
}

void HullPart::Draw(const Matrix& parent)
{
	UpdateDamagedModelStatus();
}

void HullPart::UpdateDamagedModelStatus()
{
}

void HullPart::TurnOnBurnPoints()
{
	if( !GetShipOwner() )
		return;

	Matrix mtx(true);
	GetShipOwner()->GetMatrix(mtx);

	// зажигаем огонь на всех точках, где он задан
	for (unsigned int i = 0; i < params_.burnPoints_.Size(); i++)
	{
		// время жизни огня
		float fFireLiveTime = params_.m_fBurnLiveTime + params_.m_fBurnLiveTimeRandomAdd * rand() / RAND_MAX;
		// если был добавлен новый партикл, а не изменен старый, то добавим пламя "пожирающее" парус
		if( AddHitAnimation(params_.burnParam_, mtx.MulVertex(params_.burnPoints_[i]), ShipPart::hitgrp_burn, i, fFireLiveTime ) )
		{
			IClothBurns * pCBurns = GetShipOwner()->GetClothBurns();
			if( pCBurns )
			{
				Vector vFirePos = params_.burnPoints_[i];
				vFirePos.y += 3.f;
				pCBurns->AddBurnNotHosted( IClothBurns::group_shipfire, vFirePos, GetShipOwner(), fFireLiveTime );
			}
		}
	}
}

// обработка попаданий
float HullPart::HandleDamage(const Vector& pos, float damage)
{
	float fOwnerDamage = damage * params_.shipDamageCoef_;

	// если часть уже разрушена, то ничего с ней не делаем, но возвращаем полный дамаг - что бы он начислился кораблю
	if( params_.hp_ <= 0.f )
		return fOwnerDamage;

	params_.hp_ -= damage;

	// загружаем разрушенную модель
	if ( params_.hp_ <= 0.0f || m_nDamageRecursionCounter>0 )
	{
		if( !isDamaged_ )
		{
			// теперь часть считается разбитой
			isDamaged_ = true;
			UpdateBoneTransform();
		}

		// нет корабля, значит нечего больше делать
		if( !GetShipOwner() )
			return fOwnerDamage;

		// уведомим корабль о том, что было разрушение части
		GetShipOwner()->PartDestroyFlagSet(Ship::pdf_destroy_hull);

		// зажигаем огни на палубе
		TurnOnBurnPoints();

		// рождаем партикл взрыва
		Matrix mtx(true);
		GetShipOwner()->GetMatrix(mtx);
		mtx = Matrix().BuildPosition(params_.breakSfxPos_)*mtx;
		//GetShipOwner()->Particles().CreateParticleSystemEx2(params_.breakSfx_, mtx, true, _FL_);
		GetShipOwner()->CreateAutoParticle( params_.breakSfx_.GetSFXName(GetShipOwner()->IsLimitedParticles(mtx.pos)), mtx );

		// рождаем звук попадания
		const char* pHitSound = GetShipOwner()->GetSoundNameForEvent(ShipContent::se_ship_hit);
		if( pHitSound )
			GetShipOwner()->Sound().Create3D( pHitSound, mtx.pos, _FL_ );

		// повреждаем соседние части
		if( params_.damageReceive_.Size() > 0 )
		{
			PartsClassificator ownship;
			ownship.Visit( *GetShipOwner() );
			for( unsigned int i = 0; i<params_.damageReceive_.Size(); i++ )
			{
				if( params_.damageReceive_[i].fDamage > 0.f )
				{
					HullPart & rechull = *ownship.GetHullParts()[ params_.damageReceive_[i].nPart ];
					// если часть уже участвовала в распределении дамага, то не наносим ей ничего
					if( rechull.m_nDamageRecursionCounter > 0 )
						continue;
					rechull.m_nDamageRecursionCounter = m_nDamageRecursionCounter-1;
					fOwnerDamage += rechull.HandleDamage( rechull.GetInitialPoses()[0].pos, params_.damageReceive_[i].fDamage );
					rechull.m_nDamageRecursionCounter = 0;
				}
			}
		}

		// начисляем повторную жизнь
		params_.hp_ = params_.afterBreakHP_;
	}

	return fOwnerDamage;
}

float HullPart::HandleDamage(const Vector& pos, float damage, bool isFlameThrower, long nChildRecursion)
{
	if (isFlameThrower)
	{
		AddHitAnimation(GetFlamethrowerHitSFX(), 0.75f, pos);
		Matrix m;
		GetMOOwner()->GetMatrix(m);
		for( long n=0; n<params_.burnFromFlamethrowerPoints_; n++ )
		{
			float fFireLiveTime = params_.m_fBurnLiveTime + params_.m_fBurnLiveTimeRandomAdd * rand() / RAND_MAX;
			AddHitAnimation ( GetFlamethrowerHitSFX(), 0.75f, m.MulVertex(params_.burnFromFlamethrowerPoints_[n]), ShipPart::hitgrp_burn, n | 0x8000, fFireLiveTime );
		}
	}
	else
	{
		AddHitAnimation(
			GetBallHitSFX( GetShipOwner() ? GetShipOwner()->IsLimitedParticles(pos) : false ),
			0.75f, pos);
		AddClothBurnSphere( pos );
	}

	m_nDamageRecursionCounter = nChildRecursion;
	float fOwnerDamage = HandleDamage(pos, damage);
	m_nDamageRecursionCounter = 0;
	return fOwnerDamage;
}

void HullPart::StopHits()
{
	ReleaseHitParticles();
}

void HullPart::SetPhysActor(IPhysRigidBody* actor)
{
	ShipPart::SetPhysActor( actor );
	physdata_.maxDamping = 4+2*rand()/(float)RAND_MAX;
	physdata_.curDamping = 0.01f;
}