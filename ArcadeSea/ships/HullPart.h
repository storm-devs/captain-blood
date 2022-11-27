#pragma once

#include "..\..\Common_h\gmx.h"
#include "PhysForm.h"
#include "ShipPart.h"

class Ship;
class MissionObject;
class ShipsDrawer;

// часть корпуса корабля
class HullPart : public ShipPart
{
public:
	// характеристики части корпуа
	struct Params : public ShipPart::PatternParams
	{
		struct CoDamagedPart
		{
			long nPart;
			float fDamage;
		};

		const char*				modelName_;			// цельная моделька
		const char*				modelDamagedName_;	// поврежденная моделька
		OwnerDependedParticles  burnParam_;			// эффект горения
		DescrSFX				ballHitSfx_;		// эффект попадания ядра
		const char*				flamethrowerHitSfx_;// эффект попадания огнеметом
		DescrSFX				breakSfx_;			// эффект разлома

		Vector					breakSfxPos_;		// позиция эффекта разлома
		float					hp_;				// кол-во жизней
		float					startHP_;			// начальное кол-во жизней
		float					shipDamageCoef_;	// коэф. повреждения к кораблю
		float					afterBreakHP_;		// количество жизни начисляемое после уничтожения части (для повторной возможности уничтожения)
		array<CoDamagedPart>	damageReceive_;		// части которым наносится дополнительное повредждение при взрыве части
		float					touchDamageFactor_; // коэффициент для повреждения от столкновений
		Vector					touchSfxPos_;		// позиция эффекта столкновения
		const char*				touchSfx_;			// эффект столкновения

		PhysForm		form_;
		array<Vector>	burnPoints_;
		array<Vector>	burnFromFlamethrowerPoints_;
		float			m_fBurnLiveTime;
		float			m_fBurnLiveTimeRandomAdd;

		Params() : PatternParams(),
				burnPoints_(__FILE__, __LINE__),
				burnFromFlamethrowerPoints_(_FL_),
				damageReceive_(_FL_)
		{
			modelName_ = "";
			modelDamagedName_ = "";
			flamethrowerHitSfx_ = "";
		}
	};

	long				m_nDamageRecursionCounter;

private:
	Params				params_;
	bool				isDamaged_;

	//////////////////////////////////////////////////////////////////////////
	// ShipPart
	//////////////////////////////////////////////////////////////////////////
public:
	virtual ShipPart* Clone() { return NEW HullPart(*this); }

private:
	virtual float HandleDamage(const Vector& pos, float damage); // возвращает переданный дэмэдж
	virtual float GetTouchDamageFactor() {return params_.touchDamageFactor_;} // получить коеффициент использования дамага от столкновения
	virtual void Draw(const Matrix& parent);
	virtual void Accept(IPartVisitor& visitor) { visitor.Visit(*this); }
	virtual void UpdateDamagedModelStatus();

	void TurnOnBurnPoints();
	
public:

	// конструктор
	HullPart(void);
	// конструктор копий
	HullPart(const HullPart&);
	// конструктор по параметрам
	HullPart(const HullPart::Params&);
	// оператор присваивания
	HullPart& operator=(const HullPart&);
	// деструктор
	virtual ~HullPart(void);

	// получить стандартные параметры паттерна
	virtual const PatternParams& GetPatternParams() {return params_;}

	// восстановление изначальноего вида части
	void Rebuild(const HullPart::Params&);
	float HandleDamage(const Vector& pos, float damage, bool isFlameThrower, long nChildRecursion); // возвращает переданный дэмэдж

	// get/set методы
	void SetModelName(const char* model) { params_.modelName_ = model; }
	const char* GetModelName() const { return params_.modelName_; }

	void SetDamagedModelName(const char* model) { params_.modelDamagedName_ = model; }
	const char* GetDamagedModelName() const { return params_.modelDamagedName_; }

	void SetBallHitSFX(const char* sfx) { params_.ballHitSfx_.name = sfx; }
	const char* GetBallHitSFX(bool isLod) {return params_.ballHitSfx_.GetSFXName(isLod);}
	//{ return isSmall ? params_.ballHitSmallSfx_ : params_.ballHitSfx_; }

	void SetFlamethrowerHitSFX(const char* sfx) { params_.flamethrowerHitSfx_ = sfx; }
	const char* GetFlamethrowerHitSFX() const { return params_.flamethrowerHitSfx_; }

	void SetBreakSFX(const char* sfx) { params_.breakSfx_.name = sfx; }
	const char* GetBreakSFX(bool isLod) { return params_.breakSfx_.GetSFXName(isLod); }

	void SetBreakSFXPos(const Vector & pos) { params_.breakSfxPos_ = pos; }
	const Vector & GetBreakSFXPos() const { return params_.breakSfxPos_; }

	const char* GetTouchSFX() const { return params_.touchSfx_; }
	const Vector & GetTouchSFXPos() const { return params_.touchSfxPos_; }

	void SetHP(float hp) { params_.hp_ = hp; }
	float GetHP() const { return params_.hp_; }
	float GetStartHP() const { return params_.startHP_; }

	void SetShipDamageCoef(float coef) { params_.shipDamageCoef_ = coef; }
	float GetShipDamageCoef() const { return params_.shipDamageCoef_; }

	void SetForm(const PhysForm& form) { params_.form_ = form; }
	const PhysForm& GetForm() const { return params_.form_; }
	PhysForm& GetForm() { return params_.form_; }

	// выключает партикловые системы попаданий
	void StopHits();
	
	bool IsDamaged() {return isDamaged_;}

	virtual void SetPhysActor(IPhysRigidBody* actor);
};
