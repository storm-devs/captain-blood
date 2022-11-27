#include "..\pch.h"
#include ".\cannonbonushelper.h"
#include "cannon.h"

CannonBonusHelper::CannonBonusHelper(const CannonBonusHelper& other) :
	bonusShotModel_(NULL), bonusDamageScale_(1.0f),
	isReverse_(false), cannonState_(__FILE__, __LINE__)
{
	*this = other;
}

CannonBonusHelper& CannonBonusHelper::operator=(const CannonBonusHelper& other)
{
	cannonState_ = other.cannonState_;

	if (bonusShotModel_)
		bonusShotModel_->Release();
	bonusShotModel_ = other.bonusShotModel_;
	if (bonusShotModel_)
		bonusShotModel_->AddRef();

	bonusDamageScale_ = other.bonusDamageScale_;

	curIndex_ = other.curIndex_;
	isReverse_ = other.isReverse_;

	return *this;
}

CannonBonusHelper::~CannonBonusHelper(void)
{
	if (bonusShotModel_)
		bonusShotModel_->Release();
}

void CannonBonusHelper::Visit(Cannon& cannon)
{
	if (!isReverse_)
		DoForwardAction(cannon);
	else
		DoReverseAction(cannon);
	++curIndex_;
}

void CannonBonusHelper::DoForwardAction(Cannon& cannon)
{
	// запоминаем текущее состояние
	CannonState state;
	state.damage = cannon.GetDamage();
	state.shotModel = cannon.GetShotModel();
	cannonState_.Add(state);

	// устанавливаем бонусное состояние
	cannon.SetDamage(bonusDamageScale_);
	cannon.SetShotModel(bonusShotModel_);
}

void CannonBonusHelper::DoReverseAction(Cannon& cannon)
{
	// возвращаем сохраненное состояние
	cannon.SetDamage(cannonState_[curIndex_].damage);
	cannon.SetShotModel(cannonState_[curIndex_].shotModel);
}