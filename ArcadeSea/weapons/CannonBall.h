#pragma once

#include "..\..\Common_h\gmx.h"
#include "..\..\Common_h\Render.h"
#include "..\ballistics.h"

#include "..\..\Common_h\ITrail.h"

// пушечное ядро, движущееся по баллистической траектории
class NxScene;
class Weapon;
class TargetPoints;

class CannonBall
{
	Ballistics			solver_;				// симулятор баллистики
	bool				collisionDetected_;		// есть ли столкновение
	bool				active_;				// 
	IGMXScene*			model_;					// модель ядра
	float				damage_;				// урон
	Weapon*				parent_;				// родительское орудие
	Vector				prevPos_;
	const char*			waterHitSFX_;
	long m_nBatcherID;							// номер используемого батчера
	TargetPoints*		pTargPoints_;			// объект таргет поинта
	long				pointIdx_;				// номер точки в наборе таргет поинтов
	bool				doCheckCollision_;		// проверять коллизиб с объектами

	const char *flySound;
	ISound3D* pFlySound;
	float fFlySoundDelay;

	ITrail		*trail;

//	array<RS_LINE>	debugTrack_;			// траектория для дебажной отрисовки

	// расчитывает есть ли столкновения ядра с другими объектами
	void ComputeCollision(const Vector& v1, const Vector& v2);

public:
	CannonBall(Weapon* parent = NULL);
	CannonBall(const CannonBall&);
	const CannonBall& operator=(const CannonBall&);
	~CannonBall(void);

	// устанавливает модель ядра
	void SetModel(IGMXScene* model);
	IGMXScene* GetModel() {return model_;}

	// возвращает угол, под которым был сделан выстрел
	float GetAngle() const { return solver_.GetAngle(); }
	const Vector& GetPosition() const { return solver_.GetPosition(); }

	// возвращает активность ядра
	bool IsActive() const { return active_; }

	void InActivate();
	void SetActive(bool bActive) {active_=bActive;}

	void SetFlySound(const char* pcFlySound);

	// выстреливает ядро по траектории
	bool ShootBall(const Vector& from, const Vector& to, float speed, float minAngle = 0, float maxAngle = PI/3);
	// ставит траекторию не проверяя ограничения стрельбы по углам
	void SetTrajectory(const Vector& from, const Vector& to, float minTime, float minHeight);
	// получить предрасчетное время полета ядра
	float GetPrecalculateFlyTime() { return solver_.GetPrecalculateTotalTime(); }

	// симулирует полет
	void Step(float deltaTime);

	// возвращает true если при последней симуляции было столкновение
	bool HaveCollision() const { return collisionDetected_; }

	// Get/Set
	void SetDamage(float damage) { damage_ = damage; }
	float GetDamage() const { return damage_; }

	void SetParent(Weapon * parent) { parent_ = parent;}
	Weapon* GetParent() const { return parent_; }

	void SetWaterHitSFX(const char* sfx) { waterHitSFX_ = sfx; }
	const char* GetWaterHitSFX() const { return waterHitSFX_; }

	long GetBatcherID() { return m_nBatcherID; }
	void SetBatcherID(long nBatcherID) { m_nBatcherID = nBatcherID; }

	void SetTrailParams(ITrailManager* tm, float fInitSize,float fFinalSize,float fMinFadeout,float fMaxFadeout,const Color& cColor,float fOffsetStrength);
	void SetTrailNull() {trail=0;}

	void SetTargetPoint(TargetPoints* pTargPoints, long pointIdx) { pTargPoints_ = pTargPoints; pointIdx_ = pointIdx; }

	void EnableCollision(bool bOnCollision) { doCheckCollision_ = bOnCollision; }
};
