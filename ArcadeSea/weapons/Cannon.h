#pragma once
#include "Weapon.h"
#include "..\..\Common_h\templates\array.h"
#include "CannonBallBatcher.h"

class CannonBall;
// пушка, стреляющая ядрами
class Cannon : public Weapon
{
private:
	// работает в 2-х состояниях:
	// 1) Ready - пушка готова сделать выстрел
    // 2) Reloading - пушка перезаряжается

	struct Params
	{
		const char* waterHitSFX_;		// партиклы попадания в воду
		const char* waterHitSound_;		// звук попадания в воду

		const char* colliderHitSFX_;	// партикл попадания по коллидеру (не по кораблю)
		const char* colliderHitSound_;	// звук попадания по коллидеру (не по кораблю)

		const char* flySound_;			// звук полета ядра

		float damageMultiplier;			// множитель дамага для мощного выстрела
		// параметры трейла от ядра
		WeaponTrailParams trail_normal;
		WeaponTrailParams trail_power;

		float minDistance_;				// минимальная дистанция стрельбы
		float maxDistance_;				// максимальная дистанция стрельбы
		float rollbackDistance_;		// расстояние отката
	
		float minShootAngle_;			// минимальный угол выстрела 0 = горизонтально
		float maxShootAngle_;			// максимальный угол выстрела
		float maxShootAngleKnippels_;	// максимальный угол выстрела книпелями
		float shotSpeed_;				// скорость полета снаряда

		float shootSectorAngle_;		// угол сектора обстрела
	
		float minReloadTime_;			// минимальное время перезарядки
		float maxReloadTime_;			// максимальное время перезарядки
		float reloadTime_;				// время прошедшее после последнего выстрела
		bool bAutoReload_;				// автоперезарядка

		float fFlySoundTime_;			// время звучания полета ядра (в конце пути)
	} params_;

	long				nModelBatcher_;
	IGMXScene*			shotModel_;			// моделька ядра
	IParticleSystem*	shootPS_;
	float				grouping_;
	MOSafePointerType<CannonBallContainer>	m_pCannonBallContainer;
	float m_fDelayShootSound;

private:

	float ComputeRollback(float t) const;	// расчет дистанции отката 0 <= t <= 1
	void ComputeTransform();				// считает трансформацию модельки

	// принять инспектора
	virtual void Accept(IWeaponInspector & visitor) { visitor.Visit(*this); }

public:
	// Конструкторы/деструктор
	Cannon();
	Cannon(const Cannon&);
	virtual ~Cannon(void);

	// оператор присваивания
	const Cannon& operator=(const Cannon&);
	
	virtual float GetReadyFactor()
	{
		if( GetFireState()==Reloading )
			if( params_.reloadTime_ < params_.maxReloadTime_ )
				return params_.reloadTime_ / params_.maxReloadTime_;
		return 1.f;
	}

	// выстрелить
	virtual bool Fire(const WeaponTargetZone& wtz, float fDamageMultiply) { return Fire(wtz,false); }
	virtual bool Fire(const WeaponTargetZone& wtz, bool bPowerShoot);
	virtual bool FireByPoint(TargetPoints* pTargPoints,long pointIdx);
	virtual bool FireByPos(const Vector& pos);
	// производит симуляцию стрельбы 
	virtual void Simulate(float deltaTime);
	// отрисовка
	virtual void Draw(float deltaTime);
	// создание копии
	virtual Weapon* Clone() const { return NEW Cannon(*this); }

	virtual WeaponType GetType() {return Weapon::WeaponType_cannon;}

	// отрисовка информации в редакторе
	virtual void DrawEditorFeatures();
	virtual void DrawDebugInfo();
	void DrawInfo(IRender & render, Matrix & mroot);

	// Get/Set методы
	void SetMinDistance(float minDist) { params_.minDistance_ = minDist; }
	float GetMinDistance() const { return params_.minDistance_; }

	void SetMaxDistance(float maxDist);
	float GetMaxDistance() const { return params_.maxDistance_; }

	void SetRollbackDistance(float rbDist) { params_.rollbackDistance_ = rbDist; }
	float GetRollbackDistance() const { return params_.minDistance_; }

	void SetMinAngle(float angle) { params_.minShootAngle_ = angle; }
	float GetMinAngle() const { return params_.minShootAngle_; }

	void SetMaxAngle(float angle) { params_.maxShootAngle_ = angle; }
	float GetMaxAngle() const { return params_.maxShootAngle_; }

	void SetMaxAngleKnippels(float angle) { params_.maxShootAngleKnippels_ = angle; }
	float GetMaxAngleKnippels() const { return params_.maxShootAngleKnippels_; }

	void SetShotSpeed(float shotSpeed) { params_.shotSpeed_ = shotSpeed; }
	float GetShotSpeed() const { return params_.shotSpeed_; }

	void SetMinReloadTime(float time) { params_.minReloadTime_ = time; }
	float GetMinReloadTime() const { return params_.minReloadTime_; }

	void SetMaxReloadTime(float time) { params_.maxReloadTime_ = time; }
	float GetMaxReloadTime() const { return params_.maxReloadTime_; }

	void SetAutoReload(bool autoReload) { params_.bAutoReload_ = autoReload; }
	bool GetAutoReload() const { return params_.bAutoReload_; }

	void SetShotModel(IGMXScene* model);
	void SetBallBatcher(long nModelBatcher) {nModelBatcher_ = nModelBatcher;}
	IGMXScene* GetShotModel() { return shotModel_; }

	void SetShootSectorAngle(float angle) { params_.shootSectorAngle_ = angle; }
	float GetShootSectorAngle() const { return params_.shootSectorAngle_;}

	void SetWaterHitSFX(const char* sfx) { params_.waterHitSFX_ = sfx; }
	const char* GetWaterHitSFX() const { return params_.waterHitSFX_; }

	void SetWaterHitSound(const char* sound) { params_.waterHitSound_ = sound; }
	const char* GetWaterHitSound() const { return params_.waterHitSound_; }

	void SetColliderHitSFX(const char* sfx) { params_.colliderHitSFX_ = sfx; }
	const char* GetColliderHitSFX() const { return params_.colliderHitSFX_; }

	void SetColliderHitSound(const char* sound) { params_.colliderHitSound_ = sound; }
	const char* GetColliderHitSound() const { return params_.colliderHitSound_; }

	void SetFlySound(const char* sound) { params_.flySound_ = sound; }
	const char* GetFlySound() const { return params_.flySound_; }

	WeaponTrailParams& GetTrailParameters() {return params_.trail_normal;}
	WeaponTrailParams& GetTrailParametersPower() {return params_.trail_power;}

	float GetPowerMultiplier() {return params_.damageMultiplier;}
	void SetPowerMultiplier(float mul) {params_.damageMultiplier = mul;}

	void SetGrouping(float grouping) { grouping_ = grouping; }
	// считает текущий откат пушки
	float GetCurrentRollback() const { return ComputeRollback(params_.reloadTime_/params_.maxReloadTime_); }

	CannonBallContainer* GetBallContainer()
	{
		if( !m_pCannonBallContainer.Ptr() )
		{
			static const ConstString id_CannonBallContainer("CannonBallContainer");
			if( GetOwner() )
				if( !GetOwner()->FindObject(id_CannonBallContainer,m_pCannonBallContainer.GetSPObject()) )
					GetOwner()->Mission().CreateObject( m_pCannonBallContainer.GetSPObject(), "CannonBallContainer", id_CannonBallContainer );
		}
		return m_pCannonBallContainer.Ptr();
	}

	virtual float GetFlySoundTime() { return params_.fFlySoundTime_; }
	void SetFlySoundTime(float fTime) { params_.fFlySoundTime_ = fTime; }
};
