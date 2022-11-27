#pragma once
#include "ShipPart.h"

class Ship;
class SailWind;

class Rope : public ShipPart
{
public:
	// характеристики веревки
	struct Params : public ShipPart::PatternParams
	{
		Vector			pos_[2];			// позиции точек крепления на корабле
		bool			bind_[2];			// флаг привязки к точке в СК корабля
		float			thickness_;			// толщина
		float			motionDamping_;		//
		float			friction_;			//
		const char*		model_;				// имя модельки элемента веревки
	};

private:
	Params				params_;
	bool				bAttach_;			// флаг необходимости реаттачмента
											// в первом после создания веревки кадре

public:
	enum Index { First, Second };

	Rope();
	Rope(const Rope&);
	Rope(const Rope::Params&);
	Rope& operator=(const Rope&);
	virtual ~Rope(void);

	void Make();
	void Attach();
	void Detach();

	//////////////////////////////////////////////////////////////////////////
	// ShipPart
	//////////////////////////////////////////////////////////////////////////
private:
	virtual void Accept(IPartVisitor& visitor) { visitor.Visit(*this); }
	virtual void Draw(const Matrix&) {}
	virtual void DoWork(const Matrix&);
public:
	virtual ShipPart* Clone() { return NEW Rope(*this); }; // клонирование
	virtual float HandleDamage(const Vector&, float) { return 0.0f; }

	virtual const PatternParams& GetPatternParams() {return params_;}

	//////////////////////////////////////////////////////////////////////////

	// get/set методы
	void SetShipBindPoint(Index index, const Vector& position) { params_.pos_[index] = position; }
	const Vector& GetShipBindPoint(Index index) const { return params_.pos_[index]; }
};
