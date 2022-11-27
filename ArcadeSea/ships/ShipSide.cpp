#include "..\pch.h"
#include ".\shipside.h"
#include "..\Weapons\weapon.h"

#include "..\Weapons\cannon.h"

//////////////////////////////////////////////////////////////////////////
// Класс SideWeapon
//////////////////////////////////////////////////////////////////////////
SideWeapon::SideWeapon(const SideWeapon& other)
{
	*this = other;
}

const SideWeapon& SideWeapon::operator=(const SideWeapon& other)
{
	if (params_.weapon_)
		delete params_.weapon_;

	params_ = other.params_;

	params_.weapon_ = other.params_.weapon_->Clone();

	return *this;
}

void SideWeapon::MakeWeaponFromPattern()
{
	if( !params_.owner_ ) return;
	MOSafePointer safeptr;
	params_.owner_->FindObject(params_.weaponPatternName_, safeptr);
	static const ConstString typeId("WeaponPattern");
	if (safeptr.Ptr() && safeptr.Ptr()->Is(typeId))
	{
		WeaponPattern * wp = (WeaponPattern*)safeptr.Ptr();
		Weapon * newWeapon = wp->CloneWeapon();
		if (newWeapon)
		{
			if (params_.weapon_)
				delete params_.weapon_;

			params_.weapon_ = newWeapon;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// Класс ShipSide
//////////////////////////////////////////////////////////////////////////


ShipSide::ShipSide(MissionObject * mo) :
weapons_(__FILE__, __LINE__)
{
	params_.owner_ = mo;
	params_.m_GeometryName = NULL;
	m_pAniProcedural = NULL;
	m_pModel = NULL;
	m_ReloadProgress.fCannon = m_ReloadProgress.fFlamethrower = m_ReloadProgress.fMine = 0.f;
	m_fDamageMultiply = 1.f;
}

ShipSide::ShipSide(const ShipSide& other) :
weapons_(__FILE__, __LINE__)
{
	*this = other;
	m_pModel = NULL;
	m_pAniProcedural = NULL;
}

ShipSide& ShipSide::operator=(const ShipSide& other)
{
	ClearWeapons();
	params_ = other.params_;

	for (unsigned int i = 0; i < other.weapons_.Size(); ++i)
		weapons_.Add(other.weapons_[i]);

	return *this;
}

ShipSide::~ShipSide(void)
{
	ClearWeapons();
}

void ShipSide::SetWeapon(const SideWeapon& weapon, unsigned int index)
{
	weapons_[index] = weapon;
	weapons_[index].SetOwner( params_.owner_ );
}

unsigned int ShipSide::AddWeapon(const SideWeapon& weapon)
{
	unsigned int n = weapons_.Add(weapon);
	weapons_[n].SetOwner( params_.owner_ );
	return n;
}

void ShipSide::SetOwner(MissionObject* mo)
{
	Assert(mo);
	params_.owner_ = mo;
	for (unsigned int i = 0; i < weapons_.Size(); ++i)
		weapons_[i].SetOwner(mo);
	LoadAniModel();
}

void ShipSide::SetAniModel( const char* pcAniModelName )
{
	UnloadAniModel();

	params_.m_GeometryName = pcAniModelName;
	LoadAniModel();
}

void ShipSide::UnloadAniModel()
{
	if( m_pModel )
	{
		m_pModel->Release();
		m_pModel = NULL;
	}
	if( m_pAniProcedural )
	{
		m_pAniProcedural->Release();
		m_pAniProcedural = NULL;
	}
}

void ShipSide::LoadAniModel()
{
	if( m_pModel || !params_.m_GeometryName ) return;
	if( !GetOwner() ) return;

	m_pModel = GetOwner()->Geometry().CreateScene(params_.m_GeometryName, &GetOwner()->Animation(), &GetOwner()->Particles(), &GetOwner()->Sound(), __FILE__, __LINE__);
	if( m_pModel )
	{
		m_pModel->SetDynamicLightState(true);

		// ставим пустую анимацию и регистрируем блендер
		dword q = IShipAnimationMaker::CreateProcedureAnimation(m_pModel, m_pAniProcedural,0);
		if( m_pAniProcedural ) for( dword n=0; n<q; n++ )
			m_pAniProcedural->CollapseBone( n );
	}
}

unsigned int ShipSide::Fire(const Matrix& targetMtx)
{
	unsigned int count = 0;
	for (unsigned int i = 0; i < weapons_.Size(); ++i)
		count += weapons_[i].Fire(targetMtx.pos + Vector().Rand()*GetGrouping()) ? 1 : 0;

	Matrix m;
	IParticleSystem * ps;
	if (count > 0 && params_.owner_)
		for (unsigned int i = 0; i < params_.smokePoints_.Size(); ++i)
		{
			ps = params_.owner_->Particles().CreateParticleSystemEx(params_.smokeSfx_, __FILE__, __LINE__);

			if (ps)
			{
				ps->AutoDelete(true);
				GetOwner()->GetMatrix(m);
				m = Matrix().BuildPosition(params_.smokePoints_[i])*m;

				ps->SetTransform(m);
			}
		}

	return count;
}

void ShipSide::FrameUpdate(float fDeltaTime, const Matrix& mtxParentTransform)
{
	float fCannonCur(0.f), fCannonMax(0.f);
	float fFlameCur(0.f), fFlameMax(0.f);
	float fMineCur(0.f), fMineMax(0.f);

	Matrix mtx;
	for( long n=0; n<weapons_; n++ )
	{
		// переместить и проапдейтить
		weapons_[n].SetParentTransform( mtxParentTransform );
		weapons_[n].Simulate( fDeltaTime );

		// состояние зарядки
		Weapon* pw = weapons_[n].GetWeapon();
		if( pw ) switch (pw->GetType())
		{
			case Weapon::WeaponType_cannon: fCannonCur+=pw->GetReadyFactor(); fCannonMax+=1.f; break;
			case Weapon::WeaponType_flamethrower: fFlameCur+=pw->GetReadyFactor(); fFlameMax+=1.f; break;
			case Weapon::WeaponType_mine: fMineCur+=pw->GetReadyFactor(); fMineMax+=1.f; break;
		}

		// отрисовка
		if( m_pAniProcedural )
		{
			//mtx = weapons_[n].GetWeapon()->GetTransform();
			mtx.BuildRotateY( weapons_[n].GetDirAngle() );
			mtx.pos = weapons_[n].GetPosition();
			m_pAniProcedural->SetBoneMatrix( n, mtx );
		}
		weapons_[n].Draw( fDeltaTime );
	}

	// отрисовка одной моделью
	if( m_pModel )
	{
		m_pModel->SetTransform( mtxParentTransform );
		m_pModel->Draw();
	}

	// ставим прогресс зарядки
	m_ReloadProgress.fCannon = Clampf( fCannonMax>0.f ? fCannonCur / fCannonMax : 1.f );
	m_ReloadProgress.fFlamethrower = Clampf( fFlameMax>0.f ? fFlameCur / fFlameMax : 1.f );
	m_ReloadProgress.fMine = Clampf( fMineMax>0.f ? fMineCur / fMineMax : 1.f );
}

float ShipSide::GetReloadProgress(Weapon::WeaponType wt)
{
	switch( wt )
	{
	case Weapon::WeaponType_cannon: return m_ReloadProgress.fCannon; break;
	case Weapon::WeaponType_flamethrower: return m_ReloadProgress.fFlamethrower; break;
	case Weapon::WeaponType_mine: return m_ReloadProgress.fMine; break;
	}
	return 1.f;
}

void ShipSide::ClearWeapons()
{
	while (weapons_.Size())
		weapons_.DelIndex(weapons_.Last());
	UnloadAniModel();
}

void ShipSide::SetGrouping(float grouping)
{
	// хелпер для стрельбы из огнеметов
	class CannonsHelper : public IWeaponInspector
	{
		float grouping_;
		public:
			CannonsHelper(float grp) : grouping_(grp) {}
			
			virtual void Visit(FlameThrower&) {}
			virtual void Visit(Mine&) {}
			virtual void Visit(Cannon& cannon)
			{
				cannon.SetGrouping(grouping_);
			}
	 } helper(grouping);

	 for (unsigned int i = 0; i < weapons_.Size(); ++i) weapons_[i].GetWeapon()->AcceptVisitor(helper);

	 params_.shootGrouping_ = grouping;
}