#pragma once
#include "..\..\common_h\mission.h"

class IShip;
class ShipPart;

class ShipTouch : public MissionObject
{
	// список кораблей участвующий в расчете столкновений
	array<IShip*> m_aShip;

	// список обнаруженных столкновений
	struct Touch
	{
		// столкнувшиеся корабли
		IShip* pShip1;
		IShip* pShip2;

		// части кораблей которые столкнулись
		ShipPart* pPart1;
		ShipPart* pPart2;

		// боксы участвовавшие в столкновении
		long nShape1;
		long nShape2;

		// флаг ипользования (для обновления и удаления столкновений на каждом кадре)
		bool bUse;
		// флаг первого столкновения (от него рождать звук удара и партиклы)
		bool bIsFirstTime;
	};
	array<Touch> m_aTouch;

	// параметры столкновений
	struct Params
	{
		float fDamage;
		float fDirectFactor;
	};
	Params m_params;

public:
	struct TouchData
	{
		long shape;
		ShipPart* part;
		bool knock;
		float fDamage;
	};

	ShipTouch();
	~ShipTouch();

	virtual bool Create(MOPReader & reader);
	virtual bool EditMode_Update(MOPReader & reader) {ReadMOPs(reader); return true;}
	MO_IS_FUNCTION(ShipTouch, MissionObject);

	void _cdecl Work(float fDeltaTime, long level);

	bool CheckShip(IShip* pShip, TouchData& td);

protected:
	void ReadMOPs(MOPReader & reader);

	bool CheckTouch(IShip* pShip1, IShip* pShip2, Touch& touch);
	void AddTouch(Touch& touch);

	void DebugDraw();
};
