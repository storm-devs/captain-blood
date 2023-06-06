#pragma once

#include "..\..\Common_h\gmx.h"
#include "..\..\Common_h\templates\array.h"
#include "PhysForm.h"
#include "ShipPart.h"

class MissionObject;
class Ship;
class ShipsDrawer;

// рея
class Yard : public ShipPart
{
public:

	// характеристики реи
	struct Params : public ShipPart::PatternParams
	{
		Params() : PatternParams()
		{
			modelName_ = "";
			bNotRotable = false;
		}

		const char*					modelName_;			// модель

		float						hp_;				// кол-во жизней
		float						startHP_;			// начальное кол-во жизней
		float						shipDamageCoef_;	// коэффициет повреждения к кораблю
		DescrSFX					hitSFX_;			// эффект попадания
		PhysForm					form_;
		bool						bNotRotable;		// не вращать рею даже если мачта поворачивается
	};

private:
	Params				params_;				// параметры реи

	void Break();

public:
	Yard(void);
	Yard(const Yard&);
	Yard(const Yard::Params&);
	Yard& operator=(const Yard&);
	virtual ~Yard(void);

	//////////////////////////////////////////////////////////////////////////
	// ShipPart
	//////////////////////////////////////////////////////////////////////////
public:
	virtual ShipPart* Clone() { return NEW Yard(*this); }; // клонирование
	virtual float HandleDamage(const Vector& pos, float damage); // возвращает переданный дэмэдж
	virtual const PatternParams& GetPatternParams() {return params_;}
private:
	virtual void Draw(const Matrix& parent);
	virtual void Accept(IPartVisitor& visitor) { visitor.Visit(*this); }

public:

	void Rebuild(const Yard::Params&);

	void SetModel(const char* modelName) { params_.modelName_ = modelName; }
	const char* GetModel() const { return params_.modelName_; }

	void SetHP(float hp) { params_.hp_ = hp; }
	float GetHP() const { return params_.hp_; }
	float GetStartHP() const { return params_.startHP_; }

	void SetShipDamageCoef(float coef) { params_.shipDamageCoef_ = coef; }
	float GetShipDamageCoef() const { return params_.shipDamageCoef_; }

	void SetPhysForm(const PhysForm& form) { params_.form_ = form; }
	const PhysForm& GetPhysForm() const { return params_.form_; }
	PhysForm& GetPhysForm() { return params_.form_; }

	virtual bool IsNotRotable() {return params_.bNotRotable;}

	void SetHitSFX(const char* sfx) { params_.hitSFX_.name = sfx; }
	const char* GetHitSFX(bool isLod) { return params_.hitSFX_.GetSFXName(isLod); }

	virtual const char* GetDebugName() {return params_.modelName_;}
};
