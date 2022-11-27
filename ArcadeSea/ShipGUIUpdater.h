#pragma once
#include "..\common_h\mission.h"

class Ship;

class ShipGUIUpdater : public MissionObject
{
public:
	ShipGUIUpdater();
	~ShipGUIUpdater() {}

	virtual bool Create(MOPReader & reader);
	virtual void PostCreate();
	virtual bool EditMode_Update(MOPReader & reader) {ReadMOPs(reader); return true;}
	MO_IS_FUNCTION(ShipGUIUpdater, MissionObject);

protected:
	void ReadMOPs(MOPReader & reader);
	void _cdecl Work(float fDeltaTime, long level);
	void EnableObjects(array<MOSafePointer> & aMO, bool bEnable);

	struct Params
	{
		ConstString pcLeftCannonProgress;
		ConstString pcRightCannonProgress;
		ConstString pcPowerShootProgress;

		ConstString pcFlamethrowerProgress;

		ConstString pcMineProgress;
		ConstString pcMineCountWidget;
		array<float> aMineSeparators;

		array<ConstString> aLQuadDamageEnableWidgets;
		array<ConstString> aLQuadDamageDisableWidgets;
		array<ConstString> aRQuadDamageEnableWidgets;
		array<ConstString> aRQuadDamageDisableWidgets;

		array<ConstString> aFlamethrowerEnableWidgets;
		array<ConstString> aMineEnableWidgets;

		Params() :
			aLQuadDamageEnableWidgets(_FL_),
			aLQuadDamageDisableWidgets(_FL_),
			aRQuadDamageEnableWidgets(_FL_),
			aRQuadDamageDisableWidgets(_FL_),
			aFlamethrowerEnableWidgets(_FL_),
			aMineEnableWidgets(_FL_),
			aMineSeparators(_FL_)
		{
		}
	} m_MOParams;

	struct GUIPointers
	{
		MOSafePointer pLeftCannonProgress;
		MOSafePointer pRightCannonProgress;
		MOSafePointer pFlamethowerProgress;
		MOSafePointer pMineProgress;
		MOSafePointer pMineCount;
		MOSafePointer pPowerShootProgress;

		array<MOSafePointer> aLQuadDamageEnableWidgets;
		array<MOSafePointer> aLQuadDamageDisableWidgets;

		array<MOSafePointer> aRQuadDamageEnableWidgets;
		array<MOSafePointer> aRQuadDamageDisableWidgets;

		array<MOSafePointer> aFlamethrowerEnableWidgets;
		array<MOSafePointer> aMineEnableWidgets;

		bool bLQuadEnable;
		bool bRQuadEnable;
		bool bFlameEnable;
		bool bMineEnable;

		GUIPointers() :
			aLQuadDamageEnableWidgets(_FL_),
			aLQuadDamageDisableWidgets(_FL_),
			aRQuadDamageEnableWidgets(_FL_),
			aRQuadDamageDisableWidgets(_FL_),
			aFlamethrowerEnableWidgets(_FL_),
			aMineEnableWidgets(_FL_),
			bLQuadEnable(true),
			bRQuadEnable(true),
			bFlameEnable(true),
			bMineEnable(true)
			{}
	} m_GUIPointers;

	MOSafePointerType<Ship> m_pShip;
};
