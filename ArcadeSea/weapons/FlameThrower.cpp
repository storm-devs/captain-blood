//////////////////////////////////////////////////////////////////////////
// Modenov Ivan 2006
//////////////////////////////////////////////////////////////////////////
#include "..\pch.h"
#include ".\flamethrower.h"
#include "WeaponPattern.h"
#include "..\Ships\Ship.h"
#include "..\asspecial\clothburns.h"
#include <cmath>

FlameThrower::FlameThrower(void)
{
	bCheckFire = true;

	fireParticles_ = NULL;
	fireSound_ = NULL;
	m_pClothBurnZone = NULL;

	curTraceDelta_ = 0.f;
	reloadSpeed_ = 1.f;
	reloadTime_ = curReloadTime_ = 3.f;

	m_effectTime = m_effectLiveTime = 1.f;
}

FlameThrower::~FlameThrower(void)
{
	RELEASE ( fireParticles_ );
	RELEASE ( fireSound_ );

	DELETE( m_pClothBurnZone );
}


// выстрелить
bool FlameThrower::Fire(const WeaponTargetZone& wtz, float fDamageMultiply)
{
	bool result = false;

	//if (GetFireState() == Ready)
	if( curReloadTime_ > 0.f )
	{
		bool canHit = true;

		// направление на цель
		Vector vdir = wtz.target - GetParentTransform().MulVertex( GetPosition() );

		// дистанция выстрела
		if( bCheckFire && canHit )
		{
			float fFireDist = vdir.GetLength();
			// проверим дистанцию на допустимость
			if ( fFireDist > maxDistance_ )
				canHit = false;
		}

		// угол выстрела
		if( bCheckFire && canHit )
		{
			Vector d = GetParentTransform().MulNormal( GetDirection2D() );
			float ang2targ = d.GetAngleXZ(vdir);
			// проверим угол на допустимость
			float maxang = (maxDistance_ > 0.1f) ? (0.5f * damageArea_ / maxDistance_) : 0.f;
			if( abs(ang2targ) > maxang )
				canHit = false;
		}

		if( canHit )
		{
			result = true;
//			curTraceDelta_ = 0.f;
			SetFireState(Firing);

			m_effectTime = m_effectLiveTime;
			if ( !fireParticles_ )
			{
				fireParticles_ = GetOwner()->Particles().CreateParticleSystemEx(GetSFX(), __FILE__, __LINE__);
				if( fireParticles_ )
					fireParticles_->AutoHide( true );
			}
			else
			{
				fireParticles_->PauseEmission(false);
			}

			if( !fireSound_ )
			{
				fireSound_ = GetOwner()->Sound().Create3D( GetShootSound(), GetTransform().MulVertex(GetPosition()), __FILE__, __LINE__, true, false );
			}
			else
			{
				fireSound_->FadeIn( 0.2f );
			}

			// рождаем огонь который жгет паруса
			if( !m_pClothBurnZone )
			{
				static const ConstString typeId("Ship");
				IClothBurns * pCBurns = GetOwner()->Is(typeId) ? ((Ship*)GetOwner())->GetClothBurns() : 0;
				if( pCBurns )
				{
					m_pClothBurnZone = pCBurns->AddBurnLine(IClothBurns::group_flamethrower, GetPosition(), GetPosition()+(GetDirection2D()+Vector(0.f,0.1f,0.f)) * maxDistance_, GetOwner() );
				}
			}
			RELEASE ( fireSound_ );
		}
	}

	return result;
}

// отрисовка информации в редакторе
void FlameThrower::DrawEditorFeatures()
{
	IRender & render = GetOwner()->Render();

	render.DrawXZCircle(GetPosition() - Vector(0, GetMaxDistance()*sinf(GetShootAngle()), 0),
						GetMaxDistance()*cos(GetShootAngle()), 0xFF00FF00);

	unsigned int tessFactor = 20;
	Vector end;
	Matrix m, t;
	for (unsigned int i = 0; i < tessFactor; ++i)
	{
		end.x = sin(2*PI*i/tessFactor)*GetDamageArea();
		end.y = cos(2*PI*i/tessFactor)*GetDamageArea();
		end.z = 0;
		end += Vector(0,0,1)*GetMaxDistance();

		m.BuildRotateX(GetShootAngle());
		t.BuildRotateY(GetDirectionAngle());
		m *= t;
		end = m.MulNormal(end);

		render.DrawLine(GetPosition(), 0xFF00FF00, GetPosition() + end, 0xFF00FF00);
	}
}

// отрисока
void FlameThrower::Draw(float deltaTime)
{
	if (fireParticles_)
	{
		fireParticles_->CancelHide();
		fireParticles_->SetTransform(GetTransform());
		static const ConstString typeId("Ship");
		if ( GetOwner()->Is(typeId) )
		{
			fireParticles_->AdditionalStartVelocity(((Ship*)GetOwner())->GetPhysView()->GetLinearVelocity());
		}
	}

	if( fireSound_ )
	{
		fireSound_->SetPosition( GetTransform().pos );
	}

	if (GetModel())
	{
		GetModel()->SetTransform(GetTransform());
		GetModel()->Draw();
	}
}

// производит симуляцию стрельбы
void FlameThrower::Simulate(float deltaTime)
{
	if (GetFireState() == Firing)
	{
		curReloadTime_ -= deltaTime;
	}
	else
	{
		if( fireParticles_ )
			fireParticles_->PauseEmission(true);
		if( fireSound_ )
			fireSound_->FadeOut(0.2f);

		m_effectTime -= deltaTime;
		if( m_effectTime <= 0.f )
		{
			RELEASE( fireParticles_ );
			RELEASE( fireSound_ );
			// убъем пламя которое сжигало паруса
			DELETE( m_pClothBurnZone );
		}

		if( m_effectTime <= 0.f )
			curReloadTime_ += reloadSpeed_ * deltaTime;
	}

	if( fireParticles_ )
	{
		curTraceDelta_ -= deltaTime;
		if (curTraceDelta_ <= 0.0f)
		{
			ComputeCollision(GetOwner()->Physics());
			curTraceDelta_ = traceDelta_;
		}
	}

	if( curReloadTime_ > reloadTime_ )
		curReloadTime_ = reloadTime_;
	if( curReloadTime_ < 0.f )
		curReloadTime_ = 0.f;
	SetFireState(Reloading);

	ComputeTransform();
}

// расчет трансформации
void FlameThrower::ComputeTransform()
{
	Matrix m, transform;
	transform.BuildRotateX(shootAngle_);
	m.BuildRotateY(GetDirectionAngle());
	transform *= m;
	m.BuildPosition(GetPosition());
	transform *= m;

	transform *= GetParentTransform();
	SetTransform(transform);
}

// поиск и оповещение объектов, которым наносится урон
void FlameThrower::ComputeCollision(IPhysicsScene & scene)
{
	Vector v1,v2;

	v1 = GetParentTransform().MulVertex(GetPosition());
	Vector dir(0,0,maxDistance_);
	Vector deviation(rand()/(float)RAND_MAX-0.5f, rand()/(float)RAND_MAX-0.5f, 0.0f);
	Matrix mtx;
	deviation.Normalize();
	dir += deviation*this->GetDamageArea();
	mtx.BuildRotateX(this->GetShootAngle());
	dir = mtx.MulNormal(dir);
	dir.Rotate(GetDirectionAngle());
	v2 = v1 + GetParentTransform().MulNormal(dir);

	dword n = GetOwner()->QTFindObjects(MG_DAMAGEACCEPTOR, v1, v2);

	dword hq = 0;
	DamageReceiver* aHitsObj[100];

	for (dword i=0; i<n && hq<100; i++)
	{
		MissionObject * mo = &GetOwner()->QTGetObject(i)->GetMissionObject();

		if ( mo == GetOwner() )
			continue;

		aHitsObj[hq++] = (DamageReceiver*)mo;
	}

	for( i=0; i<hq; i++ )
	{
		aHitsObj[i]->Attack(GetOwner(), DamageReceiver::ds_flame, GetDamage(), v1, v2 );
	}
}
