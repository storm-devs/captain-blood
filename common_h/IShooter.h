#ifndef ISHOOTER_MINIGAME_OBJECT
#define ISHOOTER_MINIGAME_OBJECT

#include "Mission.h"

class IShooter : public MissionObject
{
public:

	MO_IS_FUNCTION(Shooter, MissionObject);

	virtual float GetRadius() = 0;
	virtual float GetDamage() = 0;
	virtual float GetPower() = 0;

	virtual const ConstString & GetCharParticle() = 0;
	virtual const ConstString & GetCharSound() = 0;

	// проверить находится ли точка в стороне стрельбы
	virtual bool CheckDirection(const Vector& pos) = 0;
};

class IShooterKickOut : public MissionObject
{
public:
	MO_IS_FUNCTION(ShooterKickOut, MissionObject);

	// вход выход из шутера
	virtual void InShooter(IShooter* pShooter) = 0;
	virtual void OutShooter(IShooter* pShooter) = 0;

	// попытаться выкинуть из шутера
	virtual bool KickOut(const Vector& srcPos, const Vector& pos) = 0;
	// подготовиться к тому что скоро будет выкидывание из шутера
	// нужно для того что бы выкидывание в зашедший шутер было обязательным (нельзя прятаться в шутере от выстрелов)
	virtual void KickOutPrepair() = 0;
};

#endif