#include "..\pch.h"
#include "cannon.h"
#include "CannonBall.h"
#include "WeaponPattern.h"
#include "..\Ships\Ship.h"
#include "CannonBallBatcher.h"
#include "..\TargetPoints.h"

const float BALL_TIME_SCALE = 1.0f;

Cannon::Cannon(void) :
shotModel_(NULL),
shootPS_(NULL),
grouping_(0.0f)
{
	SetFireState(Ready);
	params_.fFlySoundTime_ = 0.f;
	m_fDelayShootSound = -1.f;
	nModelBatcher_ = -1;
}

// копирующий конструктор
Cannon::Cannon(const Cannon& cannon) :
shotModel_(NULL),
shootPS_(NULL),
grouping_(cannon.grouping_),
Weapon(cannon)
{
	*this = cannon;
}

// оператор присваивания
const Cannon& Cannon::operator=(const Cannon& cannon)
{
	Weapon::operator=(cannon);

	if (shotModel_)
		shotModel_->Release();

	shotModel_ = cannon.shotModel_;
	nModelBatcher_ = cannon.nModelBatcher_;

	if (shotModel_)
		shotModel_->AddRef();

	params_ = cannon.params_;

	m_pCannonBallContainer = cannon.m_pCannonBallContainer;

	m_fDelayShootSound = -1.f;

	return *this;
}

Cannon::~Cannon(void)
{
	// найдем контейнер для ядер
	static const ConstString id_CannonBallContainer("CannonBallContainer");
	if( GetOwner() )
		GetOwner()->FindObject(id_CannonBallContainer,m_pCannonBallContainer.GetSPObject());
	// удалим из контейнера ядра принадлежащие этой пушке
	if( m_pCannonBallContainer.Ptr() )
		m_pCannonBallContainer.Ptr()->DeleteByWeapon(this);

	RELEASE(shotModel_);
	RELEASE(shootPS_);
}


void Cannon::SetMaxDistance(float maxDist)
{
	const float gravity = 9.8f;
	params_.maxDistance_ = maxDist;
	if (params_.shotSpeed_ < sqrt(maxDist*gravity))
		params_.shotSpeed_ = sqrt(maxDist*gravity);
}

#include "..\DebugInfoShower.h"
// выстрелить
bool Cannon::Fire(const WeaponTargetZone& wtz, bool bPowerShoot)
{
	if (GetFireState() == Ready)
	{
		// найти точку по которой можно стрелять
		Vector target;
		//bool canHit = GetTargetPointFromZone( wtz, target );
		bool canHit = false;

		Vector vSrc = GetParentTransform().MulVertex( GetPosition() );
		Vector vRay = GetParentTransform().MulNormal( GetDirection2D() );
		long nz = wtz.GetTargetZoneByCone( vSrc, vRay, params_.shootSectorAngle_, 0 );
		if( nz >= 0 )
		{
			target = wtz.target + wtz.z[nz].c;
			canHit = true;
		}
		else
			//if( wtz.z.Size()==0 )
			{
				target = wtz.target;
				canHit = true;
			}

		// debug draw
		/*Vector v[4];
		Matrix msect(0.f,params_.shootSectorAngle_,0.f);
		v[0] = vSrc;
		v[1] = vSrc + msect.MulNormalByInverse(vRay) * GetMaxDistance();
		v[2] = vSrc + vRay * GetMaxDistance();
		v[3] = vSrc + msect.MulNormal(vRay) * GetMaxDistance();
		v[0].y = v[1].y = v[2].y = v[3].y = 4.f;
 		DebugInfoShower::AddPoligon4( GetOwner(), "ShipFireZone", "", 0x80808080, v[0],v[1],v[2],v[3] );*/

		// ограничеие по максимальной/минимальной дистанции выстрела
		if( canHit )
		{
			float dist = ~(target - vSrc);
			if( dist > params_.maxDistance_ * params_.maxDistance_ )
				canHit = false;
			else if( dist < params_.minDistance_ * params_.minDistance_ )
				canHit = false;
		}

		if (canHit)
		{
			// добавим случайный разброс
			target += Vector().Rand()*grouping_;

			CannonBallContainer* pContainer = GetBallContainer();
			if( !pContainer )
				return false;

			canHit = pContainer->AddBall( this, params_.waterHitSFX_, nModelBatcher_, shotModel_,
				//GetParentTransform().MulVertex(GetPosition()), target,
				vSrc, target,
				params_.shotSpeed_, params_.minShootAngle_,
				wtz.bKnippels ? params_.maxShootAngleKnippels_ : params_.maxShootAngle_,
				bPowerShoot ? params_.trail_power : params_.trail_normal,
				bPowerShoot ? params_.damageMultiplier : 1.f,
				params_.flySound_);

			if( canHit )
			{
				// рестарт партикловой анимации
				if (GetOwner())
				{
					// партикл
					if (shootPS_)
						shootPS_->Release(), shootPS_ = NULL;
					shootPS_ = GetOwner()->Particles().CreateParticleSystemEx2(GetSFX(), GetTransform(), false, __FILE__, __LINE__);
					if( shootPS_ )
						shootPS_->AutoHide( true );
					// звук
					m_fDelayShootSound = FRAND(Weapon::params_.shootSoundDelay_);
				}
				// установить пушку на перезарядку
				SetFireState(Reloading);
				params_.reloadTime_ = params_.maxReloadTime_*0.5f*rand()/RAND_MAX;
			}
			// for debug
			/*else
			{
				DebugInfoShower::AddLine( GetOwner(), "ShipFireZone", "", 0xFFFF0000, vSrc, target );
			}*/
		}

		return canHit;
	}
	return false;
}

bool Cannon::FireByPos(const Vector& pos)
{
	return false;
}

bool Cannon::FireByPoint(TargetPoints* pTargPoints,long pointIdx)
{
	CannonBallContainer* pContainer = GetBallContainer();
	if( !pContainer )
		return false;

	if ( pTargPoints != NULL && GetFireState() == Ready )
	{
		Vector vSrc = GetParentTransform().MulVertex( GetPosition() );
		Vector target = pTargPoints->GetPointPos(pointIdx);

		pContainer->AddBallWithoutCheck( this, vSrc, target, params_.shotSpeed_, pTargPoints, pointIdx,
			nModelBatcher_, shotModel_, params_.trail_normal, params_.waterHitSFX_, params_.flySound_);

		// рестарт партикловой анимации
		if (GetOwner())
		{
			// партикл
			if (shootPS_)
				shootPS_->Release(), shootPS_ = NULL;
			shootPS_ = GetOwner()->Particles().CreateParticleSystemEx2(GetSFX(), GetTransform(), false, __FILE__, __LINE__);
			if( shootPS_ )
				shootPS_->AutoHide( true );
			// звук
			m_fDelayShootSound = FRAND(Weapon::params_.shootSoundDelay_);
		}

		// установить пушку на перезарядку
		SetFireState(Reloading);
		params_.reloadTime_ = params_.maxReloadTime_*0.5f*rand()/RAND_MAX;
	}

	return false;
}

// симулировать стрельбу
void Cannon::Simulate(float deltaTime)
{
	params_.reloadTime_ += deltaTime;

	if( m_fDelayShootSound >= 0.f )
	{
		m_fDelayShootSound -= deltaTime;
		if( m_fDelayShootSound < 0.f )
			GetOwner()->Sound().Create3D(GetShootSound(), GetParentTransform().MulVertex(GetPosition()), __FILE__, __LINE__);
	}


	if (params_.reloadTime_ > params_.maxReloadTime_)
		SetFireState(Ready);

	ComputeTransform();
}

float Cannon::ComputeRollback(float t) const // 0.0f <= t <= 1.0f
{
	if (t > 1.0f)
		t = 1.0f;
	if (t < 0.0f)
		t = 0.0f;
	// линейный откат/возврат
	const float backPart = 0.3f;
	if (t < backPart)
		return params_.rollbackDistance_/backPart*t;

	return params_.rollbackDistance_/(1-backPart)*(1-t);
}


void Cannon::ComputeTransform()
{
	Vector dir(0,0,1);
	Matrix transform;
	transform = Matrix().BuildPosition(0,0,-ComputeRollback(params_.reloadTime_/params_.maxReloadTime_));
	transform *= Matrix().BuildRotateY(GetDirectionAngle()); // ориентируем дуло на направление выстрела
	transform *= Matrix().BuildPosition(GetPosition());
	transform *= GetParentTransform();
	SetTransform(transform);
}

void Cannon::SetShotModel(IGMXScene* model)
{
	if( model == shotModel_ ) return;

	if( model )
		model->AddRef();

	if (shotModel_)
		shotModel_->Release();
	shotModel_ = model;
}

// отрисовка информации в редакторе
void Cannon::DrawEditorFeatures()
{
	if( GetOwner() )
		DrawInfo( GetOwner()->Render(), Matrix() );
}

void Cannon::DrawDebugInfo()
{
	if( GetOwner() )
	{
		Matrix mtx(true);
		DrawInfo( GetOwner()->Render(), GetOwner()->GetMatrix(mtx) );
	}
}

void Cannon::DrawInfo(IRender & render, Matrix & mroot)
{
	Vector pos = mroot.MulVertex( GetPosition() );
	float fDirAngle = mroot.GetAngles().y + GetDirectionAngle();

	render.DrawXZCircle(pos, GetMaxDistance(), 0xFF00FF00);
	render.DrawXZCircle(pos, GetMinDistance(), 0xFF00FF00);

	const unsigned int tessLevel = 10;
	Vector p1[tessLevel];
	Vector p2[tessLevel];
	Vector poly[(tessLevel-1)*2*3];

	for (unsigned int i = 0; i < tessLevel; ++i)
	{
		float angle = -GetShootSectorAngle()/2 + GetShootSectorAngle()*i/(float)(tessLevel-1) + fDirAngle;

		p1[i] = Vector(GetMinDistance()*sin(angle), 0, GetMinDistance()*cos(angle)) + pos;
		p2[i] = Vector(GetMaxDistance()*sin(angle), 0, GetMaxDistance()*cos(angle)) + pos;
	}

	for (unsigned int i = 0; i < (tessLevel-1); ++i)
	{
		poly[i*6 + 0] = p1[i+0];
		poly[i*6 + 1] = p2[i+0];
		poly[i*6 + 2] = p2[i+1];

		poly[i*6 + 3] = p1[i+0];
		poly[i*6 + 4] = p2[i+1];
		poly[i*6 + 5] = p1[i+1];
	}
	Vector points[] =
	{
		Vector(GetMinDistance()*sin(-GetShootSectorAngle()/2 + fDirAngle), 0, GetMinDistance()*cos(-GetShootSectorAngle()/2 + fDirAngle)) + pos,
		Vector(GetMaxDistance()*sin(-GetShootSectorAngle()/2 + fDirAngle), 0, GetMaxDistance()*cos(-GetShootSectorAngle()/2 + fDirAngle)) + pos,
		Vector(GetMaxDistance()*sin(GetShootSectorAngle()/2 + fDirAngle), 0, GetMaxDistance()*cos(GetShootSectorAngle()/2 + fDirAngle)) + pos,
		Vector(GetMinDistance()*sin(GetShootSectorAngle()/2 + fDirAngle), 0, GetMinDistance()*cos(GetShootSectorAngle()/2 + fDirAngle)) + pos,
	};
	Matrix m;
	render.SetWorld(m);
	for (unsigned int i = 0; i < tessLevel-1; ++i)
		render.DrawPolygon(poly + i*6, 6, 0x8000FF00);
	
	unsigned int lineTessLevel = 20;
	Ballistics t[4];
	t[0].SetTrajectoryParams(pos, points[0], GetShotSpeed());
	t[1].SetTrajectoryParams(pos, points[1], GetShotSpeed());
	t[2].SetTrajectoryParams(pos, points[2], GetShotSpeed());
	t[3].SetTrajectoryParams(pos, points[3], GetShotSpeed());
	float fTimeStep = 1.f / (float)lineTessLevel;
	for (unsigned int j = 0; j < sizeof(t)/sizeof(t[0]); j++)
	{
		float fTotalTime = t[j].GetTotalFlightTime();
		dword color = (j==0 || j==3) ? 0xFFFF0000 : 0xFF0000FF;
		for (unsigned int i = 0; i < lineTessLevel; ++i)
			render.DrawLine(
				t[j].GetPosition(fTotalTime*i*fTimeStep), color,
				t[j].GetPosition(fTotalTime*(i+1)*fTimeStep), color );
	}

	Vector vdir = 5.f * Vector(sin(fDirAngle), sin(GetMinAngle()), cos(fDirAngle));
	render.DrawLine( pos, 0xFFFF00FF, pos + vdir, 0xFFFF00FF );
	vdir.y = 5.f * sin(GetMaxAngle());
	render.DrawLine( pos, 0xFFFF00FF, pos + vdir, 0xFFFF00FF );
}

void Cannon::Draw(float deltaTime)
{
	if ( GetModel() )
	{
		GetModel()->SetTransform(GetTransform());
		GetModel()->Draw();
	}
	
	if (shootPS_)
	{
		Matrix mtx;
		shootPS_->SetTransform(GetTransform());
		shootPS_->CancelHide();
	}

	static const ConstString typeId("Ship");
	if (shootPS_ && GetOwner()->Is(typeId))
	{
		shootPS_->AdditionalStartVelocity(((Ship*)GetOwner())->GetPhysView()->GetLinearVelocity());
	}
}
