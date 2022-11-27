#include "..\pch.h"
#include ".\yard.h"
#include "..\..\Common_h\Mission.h"
#include "ship.h"
#include "BrokenSystemMaker.h"

// конструктор
Yard::Yard(void)
{
	params_.modelName_ = "";
}

// конструктор копий
Yard::Yard(const Yard& other)
{
	params_ =  other.params_;
}

// конструктор по параметрам
Yard::Yard(const Yard::Params& other)
{
	params_ =  other;
	params_.startHP_ = params_.hp_;
}

// оператор присваивания
Yard& Yard::operator=(const Yard& other)
{
	params_ = other.params_;

	return *this;
}

// деструктор
Yard::~Yard(void)
{
}

void Yard::Rebuild(const Yard::Params& params)
{
	ShipPart::Rebuild();

	params_ = params;
}

void Yard::Break()
{
	if (IsBroken())
		return;
	if( !GetShipOwner() )
		return;

	BrokenSystemMaker maker(*GetShipOwner()->GetPhysView(), GetMOOwner()->Physics());
	maker.SetStartNode(this);

	ShipPart * p = this;
	while (p->GetParent())
		p = p->GetParent();

	p->AcceptVisitor(maker);

	maker.Make();

	SetBrokenSystem(maker.GetSystem());
}

void Yard::Draw(const Matrix& parent)
{
}

float Yard::HandleDamage(const Vector& pos, float damage)
{
	params_.hp_ -= damage;
	float fOwnerDamage = damage*params_.shipDamageCoef_;

	if( GetShipOwner() )
		AddHitAnimation(GetHitSFX(GetShipOwner()->IsLimitedParticles(pos)), 0.75f, pos);
	AddClothBurnSphere( pos );

	// делаем разрушение, если кончилась HP
	if (params_.hp_ < 0.0f)
	{
		if( GetShipOwner() && !IsBroken() )
			fOwnerDamage += GetShipOwner()->GetDamageFromMastBroke();
		Break();
	}

	return fOwnerDamage;
}
