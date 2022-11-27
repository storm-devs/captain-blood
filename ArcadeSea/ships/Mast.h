#pragma once

#include "..\..\Common_h\gmx.h"
#include "..\..\Common_h\particles.h"
#include "..\..\Common_h\templates\array.h"
#include "PhysForm.h"
#include "Yard.h"
#include "Sail.h"
#include "ShipPart.h"

class Ship;
class MissionObject;
class Mast;
class ShipsDrawer;

// часть (ярус) мачты
class MastPart : public ShipPart
{
public:
	// параметры части
	struct Params : public ShipPart::PatternParams
	{
		Params() : PatternParams(),
			yards_(_FL_)
		{
			modelName_ = "";
		}
		float					hp_;				// кол-во жизней части (яруса)
		float					startHP_;			// начальное кол-во жизней части (яруса)
		const char*				modelName_;			// модель части мачты
		array<Yard::Params>		yards_;				// реи
		PhysForm				form_;				// форма части мачты
	};
private:

	Params							params_;
	Mast&							mast_;
	
	//////////////////////////////////////////////////////////////////////////
	// ShipPart
	//////////////////////////////////////////////////////////////////////////
public:
	virtual ShipPart* Clone() { return NEW MastPart(*this); }
	virtual float HandleDamage(const Vector& pos, float damage);
private:
	virtual void Accept(IPartVisitor& visitor) { visitor.Visit(*this); }

public:
	
	//////////////////////////////////////////////////////////////////////////
	// конструктор
	MastPart(Mast& mast);
	// конструктор по параметрам
	MastPart(Mast& mast, const MastPart::Params& params);
	// конструктор копий
	MastPart(const MastPart&);
	// оператор присваивания
	MastPart& operator=(const MastPart&);
	// деструктор
	virtual ~MastPart(void);

	virtual const PatternParams& GetPatternParams() {return params_;}

	void Rebuild(const MastPart::Params& params);

	Mast& GetMast() { return mast_; }

	unsigned int GetYardsCount() const { return params_.yards_.Size(); }
	Yard::Params& GetYard(unsigned int index) { return params_.yards_[index]; }
	void SetYard(const Yard::Params& yard, unsigned int index) { params_.yards_[index] = yard; }
	void AddYard(const Yard::Params& yard) { params_.yards_.Add(yard); }

	void SetForm(const PhysForm& form ) { params_.form_ = form; }
	const PhysForm& GetForm() const { return params_.form_; }
	PhysForm& GetForm() { return params_.form_; }

	float GetHP() const { return params_.hp_; }
	float GetStartHP() const { return params_.startHP_; }
};

// мачта корабля
class Mast : public ShipPart
{
public:
	// описание мачты
	struct Params : public ShipPart::PatternParams
	{
		Params() : PatternParams(),
			partsDesc_(_FL_),
			sails_(_FL_) {}
		float					shipDamageCoef_;	// коэффициет повреждения к кораблю
		DescrSFX				hitSFX_;			// эффект попадания
		array<MastPart::Params>	partsDesc_;			// части мачты
		array<Sail::Params>		sails_;				// реи
		bool					isRotatable_;		// флаг вращаемости 
		Vector					rotationAxisBegin_;	// начало оси вращения
		Vector					rotationAxisEnd_;	// конец оси вращения
	};

private:
	Params					params_;				// параметры мачты
	array<MastPart*>		parts_;

	Vector m_vOrientBasis[3][3];			// множители для поиска матрицы поворота
	void RecalculateOrientBasis();

	
	
public:
	// конструктор
	Mast(void);
	// конструтор копий
	Mast(const Mast&);
	// конструктор по параметрам
	Mast(const Mast::Params&);
	// оператор присваивания
	Mast& operator=(const Mast&);
	// деструктор
	virtual ~Mast(void);

	//////////////////////////////////////////////////////////////////////////
	// ShipPart
	//////////////////////////////////////////////////////////////////////////
public:
	virtual ShipPart* Clone() { return NEW Mast(*this); }
	virtual float HandleDamage(const Vector&, float) { return 0.0f; }

	virtual const PatternParams& GetPatternParams() {return params_;}
private:
	virtual void Draw(const Matrix&) {}
	virtual void Accept(IPartVisitor& visitor) { visitor.Visit(*this); }
	virtual void DoWorkChildren(const Matrix&);
	virtual void DoWork(const Matrix&);
	
public:
	//////////////////////////////////////////////////////////////////////////
	void Rebuild(const Mast::Params&);

	// ломает мачту начиная с заданной части
	void BreakAt(MastPart* part);
	// ломает всю мачту
	void Break();

	void SetShipDamageCoef(float coef){ params_.shipDamageCoef_ = coef; }
	float GetShipDamageCoef() const { return params_.shipDamageCoef_; }

	void SetHitSFX(const char* sfx) { params_.hitSFX_.name = sfx; }
	const char* GetHitSFX(bool isLod) { return params_.hitSFX_.GetSFXName(isLod); }

	unsigned int GetPartsCount() const { return parts_.Size(); }
	MastPart* GetPart(unsigned int index) { return index>=0 && index<parts_.Size() ? parts_[index] : NULL; }	
	void SetPart(MastPart* part, unsigned int index) { if(index>=0 && index<parts_.Size()) parts_[index] = part; }
	void AddPart(MastPart* part) { parts_.Add(part); }
	void DelPart(MastPart* part) { parts_.Del(part); }

	unsigned int GetSailCount() const { return params_.sails_.Size(); }
	Sail::Params& GetSail(unsigned int index) { return params_.sails_[index]; }	

	Matrix GetRotationLocal(const Matrix& parent) const;
};