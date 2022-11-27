#pragma once

#include "..\..\Common_h\particles.h"
#include "..\..\Common_h\templates\array.h"
#include "..\Weapons\Weapon.h"
#include "ShipAni.h"


// бортовое орудие
class SideWeapon
{
	struct Params
	{
		Params() : owner_(NULL), weapon_(NEW DummyWeapon()) {}
		ConstString		weaponPatternName_;		// имя шаблона
		Weapon*			weapon_;				// оружие
		MissionObject*	owner_;
	} params_;
	
	// создает реальное орудие weapon_ по шаблону
	void MakeWeaponFromPattern();

public:

	// конструкторы
	SideWeapon(MissionObject * mo = NULL) { params_.owner_ = mo; }
	SideWeapon(const SideWeapon&);
	// оператор присваивания
	const SideWeapon& operator=(const SideWeapon&);
	// деструктор
	~SideWeapon() { DELETE (params_.weapon_); }

	bool Fire(const Vector& target) { return params_.weapon_->Fire(target, 1.f); }
	void Simulate(float deltaTime) { params_.weapon_->Simulate(deltaTime); }
	void Draw(float deltaTime) { params_.weapon_->Draw(deltaTime); }
		
	// get/set методы
    void SetPatternName(const ConstString & name) { params_.weaponPatternName_ = name; MakeWeaponFromPattern(); }
	const ConstString & GetPatternName() const { return params_.weaponPatternName_; }

	void SetPosition(const Vector& pos) { Assert(params_.weapon_ != NULL); params_.weapon_->SetPosition(pos); }
	const Vector& GetPosition() const { Assert(params_.weapon_ != NULL); return params_.weapon_->GetPosition(); }
 
	void SetDirAngle(float angle) { Assert(params_.weapon_ != NULL); params_.weapon_->SetDirectionAngle(angle); }
	float GetDirAngle() const { Assert(params_.weapon_ != NULL); return params_.weapon_->GetDirectionAngle(); }

	void SetAnimBoneName(const char* name) { Assert(params_.weapon_ != NULL); params_.weapon_->SetAnimBoneName(name); }
	const char* GetAnimBoneName() const { Assert(params_.weapon_ != NULL); return params_.weapon_->GetAnimBoneName(); }

	void SetParentTransform(const Matrix& m) { Assert(params_.weapon_ != NULL); params_.weapon_->SetParentTransform(m); }
	const Matrix& GetParentTransform() const { Assert(params_.weapon_ != NULL); return params_.weapon_->GetParentTransform(); }
	
	void SetOwner(MissionObject* mo) { Assert(mo); params_.owner_ = mo; Assert(params_.weapon_); params_.weapon_->SetOwner(mo); }
	MissionObject* GetOwner() const { return params_.owner_; }

	Weapon* GetWeapon() { return params_.weapon_; }
};

// борт корабля
class ShipSide
{
	struct Params
	{
		Params() : smokePoints_(__FILE__, __LINE__) {}
		const char*			m_GeometryName;		// имя анимационной модели (для общего рендера)
		const char*			smokeSfx_;			// эффект задымления при бортовом залпе
		array<Vector>		smokePoints_;		// точки эффекта задымления
		float				shootGrouping_;
		MissionObject*		owner_;
	} params_;

	array<SideWeapon>	weapons_;			// бортовые орудия
	IGMXScene*			m_pModel;			// полная модель орудий
	IAnimationProcedural* m_pAniProcedural;

	struct ReloadProgress
	{
		float fCannon;
		float fFlamethrower;
		float fMine;
	} m_ReloadProgress;

	float m_fDamageMultiply;

public:
	ShipSide(MissionObject * mo = NULL);
	ShipSide(const ShipSide&);
	ShipSide& operator=(const ShipSide&);
	~ShipSide(void);

	// стреляет и возвращает кол-во выстреливших орудий
	unsigned int Fire(const Matrix& targetMtx);

	float GetReloadProgress(Weapon::WeaponType wt);

	void FrameUpdate(float fDeltaTime, const Matrix& mtxParentTransform);

	unsigned int GetWeaponCount() const { return weapons_.Size(); }
	SideWeapon& GetWeapon(unsigned int index) { return weapons_[index]; }
	void SetWeapon(const SideWeapon& weapon, unsigned int index);
	unsigned int AddWeapon(const SideWeapon& weapon);
	void ClearWeapons();

	void SetGrouping(float grouping);
	float GetGrouping() const { return params_.shootGrouping_; }

	void SetSmokeSFX(const char* sfx) { params_.smokeSfx_ = sfx; }
	const char* GetSmokeSFX() const { return params_.smokeSfx_; }

	void SetOwner(MissionObject* mo);
	MissionObject* GetOwner() const { return params_.owner_; }

	void SetAniModel( const char* pcAniModelName );

	unsigned int GetSmokePointsCount() const { return params_.smokePoints_.Size(); }
	const Vector& GetSmokePoint(unsigned int index) const { return params_.smokePoints_[index]; }
	void SetSmokePoint(const Vector& point, unsigned int index) { params_.smokePoints_[index] = point; }
	unsigned int AddSmokePoint(const Vector& point) { return params_.smokePoints_.Add(point); }
	void ClearSmokePoints() { params_.smokePoints_.DelAll(); }

	void SetDamageMultiply(float fDamageMultiply) {m_fDamageMultiply=fDamageMultiply;}
	float GetDamageMultiply() {return m_fDamageMultiply;}

protected:
	void UnloadAniModel();
	void LoadAniModel();
};
