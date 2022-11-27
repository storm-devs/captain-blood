#pragma once
#include "weapon.h"
#include "..\..\Common_h\Physics.h"

class IParticleSystem;
class IBurnZone;
// огнемет
class FlameThrower :
	public Weapon
{
	// работает в 2-х состояниях:
	// 1) Ready - готов метает огонь
	// 2) Firing - процесс метания в самом разгаре

	float				traceDelta_, curTraceDelta_;
	float				reloadTime_, curReloadTime_;	// время и текущее время перезарядки
	float				reloadSpeed_;
	float				maxDistance_;					// максимальная дистанция нанесения урона
	float				damageArea_;					// радиус основания конуса поражения
	unsigned int		raysCount_;						// кол-во лучей для коллижн-теста
	float				shootAngle_;					// угол к горизонту

	ISound3D*			fireSound_;						// звук выстрела
	IParticleSystem*	fireParticles_;					// партиклы выстрела
	IBurnZone*			m_pClothBurnZone;				// пламя сжигающие паруса
	float				m_effectTime, m_effectLiveTime;	// время жизни эффекта

	bool				bCheckFire;

	void ComputeCollision(IPhysicsScene & scene);
	void ComputeTransform();

	// принять инспектора
	virtual void Accept(IWeaponInspector & visitor) { visitor.Visit(*this); }

public:
	FlameThrower(void);
	//Конструктор копий и оператор присваивания по умолчанию устраиваиют
	//FlameThrower(const FlameThrower&);
	//FlameThrower& operator=(const FlameThrower&);
	virtual ~FlameThrower(void);

	virtual float GetReadyFactor()
	{
		if( curReloadTime_ >= reloadTime_ )
			return 1.f;
		if( curReloadTime_ <= 0.f )
			return 0.f;
		return curReloadTime_ / reloadTime_;
	}

	// выстрелить
	virtual bool Fire(const WeaponTargetZone& wtz, float fDamageMultiply);
	// производит симуляцию стрельбы
	virtual void Simulate(float deltaTime);
	// отрисока
	virtual void Draw(float deltaTime);
	// создание копии
	virtual Weapon* Clone() const { return NEW FlameThrower(*this); }

	virtual WeaponType GetType() {return Weapon::WeaponType_flamethrower;}

	// отрисовка информации в редакторе
	virtual void DrawEditorFeatures();
	virtual void DrawDebugInfo() {}

	// Get/Set
	void SetMaxDistance(float distance) { maxDistance_ = distance; }
	float GetMaxDistance() const { return maxDistance_; }

	void SetReloadTime(float time) { curReloadTime_ = reloadTime_ = time; }
	float GetReloadTime() const { return reloadTime_; }
	float GetCurrentReloadTime() const { return curReloadTime_; }
	void SetReloadSpeed(float speed) { reloadSpeed_ = speed; }

	void SetDamageArea(float area) { damageArea_ = area; }
	float GetDamageArea() const { return damageArea_; }

	void SetFireDeltaTime(float dTime) { traceDelta_ = dTime; curTraceDelta_ = 0.f; }

	void SetCollisionRaysCount(unsigned int count) { raysCount_ = count; }
	unsigned int GetCollisionRaysCount() const { return raysCount_; }

	void SetShootAngle(float angle) { shootAngle_ = angle; }
	float GetShootAngle() const { return shootAngle_; }

	void SetCheckFire(bool bDoCheck) {bCheckFire = bDoCheck;}
	bool GetCheckFire() {return bCheckFire;}
};
