#ifndef _IShip_h_
#define _IShip_h_

#include "Math3D.h"
#include "core.h"
#include "templates\array.h"
#include "Mission.h"

enum ShipParty
{
	SHIP_SIDE_ENEMY,
	SHIP_SIDE_NEUTRAL,
	SHIP_SIDE_FRIEND,
	SHIP_SIDE_PLAYER_1 = SHIP_SIDE_FRIEND,
	SHIP_SIDE_PLAYER_2,
	SHIP_SIDE_PLAYER_3,
	SHIP_SIDE_PLAYER_4
};

#define MAX_SHIP_FIRE_RADIUS 200

// базовый интерфейс корабля
class IShip : public DamageReceiver
{
public:
	// получить текущую позицию корабля
	virtual Vector GetPosition () const = 0;
	// задать угловую скорость
	virtual void SetRotate ( float fRotSpeed ) = 0;
	// задать  скорость движения
	virtual void SetVelocity ( float fShipVelocity ) = 0;
	// задать скорость смещения
	virtual void SetSideVelocity( float fShipOffset ) = 0;
	//получить текущий курсовой угол
	virtual float GetCurrentDirectionAngle () = 0;
	// получить текущую скорость
	virtual float GetCurrentVelocity () const = 0;
	// получить максимально возможную скорость корабля
	virtual float GetMaxVelocity () const = 0;
	// получить максимально возможную угловую скорость
	virtual float GetMaxAngleSpeed () const = 0;
	// стрельнуть пушками по цели (fFireLevel=0.f - палим по корпусу fFireLevel=1.f - палим по парусам, между 0.f и 1.f - вероятность стрельбы по парусам)
	virtual void FireCannons ( MissionObject * pFireTarget, float fFireLevel ) = 0;
	// стрельнуть огнеметами
	virtual void FireFlamethrower() = 0;
	// стрельнуть минами
	virtual void FireMines() = 0;
	// установить  сторону корабля
	virtual void SetParty ( ShipParty eShipParty ) = 0;
	// установить  сторону корабля
	virtual ShipParty GetParty () const = 0;
	// получить размеры корабля ( длину и ширину )
	virtual void GetSizes ( float & fLength, float & fWidth ) const = 0;
};

class IShipControllerParams : public MissionObject
{
public:
	virtual	~IShipControllerParams() {}
	MO_IS_FUNCTION(IShipControllerParams, MissionObject);

};

enum ShipControlType
{
	SHIP_AI_STATE_WAIT = 0,
	SHIP_AI_STATE_WAY_MOVE,
	SHIP_AI_STATE_BRANDER,
	SHIP_AI_STATE_BATTLE,
	SHIP_AI_STATE_PLAYER_CONTROL
};

// интерфейс управления кораблем

class IShipController
{
public:
		
	virtual ~IShipController() {};

	// установка параметров ИИ
	virtual void SetControllerParams ( IShipControllerParams * pControllerParams  ) = 0;
	// управление кораблем
	virtual void Work ( float dltTime ) = 0;

	// отрисовать дебажную инфу
	virtual void ViewDebugInfo() = 0;

	// сбор статистики
	enum StatisticEnum
	{
		Statistic_damage,
		Statistic_attack
	};
	struct StatisticVal
	{
		union
		{
			float fVal;
			bool bVal;
			long nVal;
		} val;
		StatisticVal() {}
		StatisticVal(float fVal) {val.fVal=fVal;}
		StatisticVal(bool bVal) {val.bVal=bVal;}
		StatisticVal(long nVal) {val.nVal=nVal;}
	};
	virtual void SetStatistic( IShipController::StatisticEnum eStat, IShipController::StatisticVal& val ) {}
};

#endif