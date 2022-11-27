#pragma once

#include "Cannon.h"
#include "FlameThrower.h"
#include "Mine.h"

//////////////////////////////////////////////////////////////////////////
class CannonPattern : public WeaponPattern
{
	Cannon	cannon_;
	Vector	vPos_;

	void ReadParams(MOPReader&);
public:
	CannonPattern() { cannon_.SetOwner(this); }
	~CannonPattern() { cannon_.SetOwner(NULL); }

	Cannon& GetCannon() { return cannon_; }

	// реализация IWeaponPattern
	virtual Weapon* CloneWeapon() { return NEW Cannon(cannon_); }

    // 
	MO_IS_FUNCTION(CannonPattern, WeaponPattern);

	virtual bool Create(MOPReader & reader)
	{
		ReadParams(reader);
		if( EditMode_IsOn() )
			SetUpdate(&CannonPattern::Draw, ML_GEOMETRY2);
		return true;
	}
	virtual bool EditMode_Update(MOPReader & reader);
	virtual void EditMode_GetSelectBox(Vector & min, Vector & max);
	virtual Matrix & GetMatrix(Matrix & mtx);
	void _cdecl Draw( float deltaTime, long level);
};



//////////////////////////////////////////////////////////////////////////
class FlameThrowerPattern : public WeaponPattern
{
	FlameThrower	ft_;
	Vector			vPos_;

	void ReadParams(MOPReader&);
public:
	FlameThrowerPattern() { ft_.SetOwner(this); }

	FlameThrower& GetFlameThrower() { return ft_; }

	// реализация IWeaponPattern
	virtual Weapon* CloneWeapon() { return NEW FlameThrower(ft_); }

	//
	MO_IS_FUNCTION(FlameThrowerPattern, WeaponPattern);

	virtual bool Create(MOPReader & reader)
	{
		ReadParams(reader);
		if( EditMode_IsOn() )
			SetUpdate(&FlameThrowerPattern::Draw, ML_GEOMETRY2);
		return true;
	}
	virtual bool EditMode_Update(MOPReader & reader);
	virtual void EditMode_GetSelectBox(Vector & min, Vector & max);
	virtual Matrix & GetMatrix(Matrix & mtx);
	void _cdecl Draw( float deltaTime, long level);
};


//////////////////////////////////////////////////////////////////////////
class MinePattern : public WeaponPattern
{
	Mine	mine_;
	Vector	vPos_;

	void ReadParams(MOPReader&);
public:
	MinePattern() { mine_.SetOwner(this); }

	Mine& GetMine() { return mine_; }

	// реализация IWeaponPattern
	virtual Weapon* CloneWeapon() { return NEW Mine(mine_); }

	//
	MO_IS_FUNCTION(MinePattern, WeaponPattern);

	virtual bool Create(MOPReader & reader)
	{
		mine_.SetOwner(this);
		ReadParams(reader);
		if( EditMode_IsOn() )
			SetUpdate(&MinePattern::Draw, ML_GEOMETRY2);
		return true;
	}
	virtual bool EditMode_Update(MOPReader & reader);
	virtual void EditMode_GetSelectBox(Vector & min, Vector & max);
	virtual Matrix & GetMatrix(Matrix & mtx);
	void _cdecl Draw( float deltaTime, long level);
};