#pragma once
#include "weaponinspector.h"
#include "..\..\Common_h\core.h"
#include "..\..\Common_h\templates\array.h"

class IGMXScene;
class CannonBonusHelper : public IWeaponInspector
{
	// некоторые параметры до применения бонуса,
	// которые мы должны вернуть по окончании действия бонуса
	struct CannonState
	{
		float damage;
		IGMXScene*	shotModel;
	};
	array<CannonState>		cannonState_;

	IGMXScene*	bonusShotModel_;
	float		bonusDamageScale_;

	unsigned int curIndex_;
	bool	isReverse_;		// флаг возврата пушек в добонусное состояние

	void DoForwardAction(Cannon&);
	void DoReverseAction(Cannon&);

public:
	CannonBonusHelper(IGMXScene* bonusShotModel, float damageScale) :
		bonusShotModel_(bonusShotModel), bonusDamageScale_(damageScale),
		isReverse_(false), cannonState_(__FILE__, __LINE__) {}
	CannonBonusHelper(const CannonBonusHelper&);
	CannonBonusHelper& operator=(const CannonBonusHelper&);
	virtual ~CannonBonusHelper(void);

	virtual void Visit(Cannon&);
	virtual void Visit(Mine&) {}
	virtual void Visit(FlameThrower&) {}

	void SetReverse(bool isReverse)
	{
		isReverse_ = isReverse; curIndex_ = 0;
		if (!isReverse_)
			cannonState_.DelAll();
	}
};
