#ifndef _IExplosionPatch_h_
#define _IExplosionPatch_h_

#include "Mission.h"

// базовый интерфейс корабля
class IExplosionPatch : public MissionObject
{
protected:
	IExplosionPatch() {}

public:
	// найти группу взрывов по заданому имени
	virtual long GetHitGroup(const char* groupName) = 0;

	// создать взрыв в точке (группа задана через ее индекс)
	virtual void MakeExplosion(const Vector& pos, long nHitGroupIndex) = 0;

	// создать взрыв в точке (группа задана через ее имя)
	virtual void MakeExplosion(const Vector& pos, const char* groupName) = 0;

	// создать взрыв в точке (группа задана через ее имя)
	virtual void MakeExplosion(const Vector& pos, const ConstString & groupName) = 0;

	// очистить все эффекты
	virtual void ClearExplosions() = 0;

	static IExplosionPatch* GetExplosionPatch(IMission& mis)
	{
		MOSafePointer sptr;
		ConstString idExplosionPatch("Explosion patch");
		if( mis.FindObject(idExplosionPatch,sptr) )
			return (IExplosionPatch*)sptr.Ptr();
		return NULL;
	}
};

#endif
