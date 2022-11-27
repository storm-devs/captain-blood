#pragma once

class Cannon;
class Mine;
class FlameThrower;
class ShipSide;

class IWeaponInspector
{
	ShipSide* m_pWeaponSide;
public:
	virtual ~IWeaponInspector(void) {}

	virtual void Visit(Cannon&) = 0;
	virtual void Visit(Mine&) = 0;
	virtual void Visit(FlameThrower&) = 0;

	void SetWeaponSide( ShipSide* pSW ) {m_pWeaponSide=pSW;}
	ShipSide* GetWeaponSide() {return m_pWeaponSide;}
};
