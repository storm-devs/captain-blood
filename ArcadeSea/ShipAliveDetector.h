#ifndef _ShipAliveDetector_h_
#define _ShipAliveDetector_h_

#include "..\Common_h\Mission\Mission.h"

class ShipAliveDetector : public MissionObject
{
//--------------------------------------------------------------------------------------------
public:
	ShipAliveDetector();
	virtual ~ShipAliveDetector();

//--------------------------------------------------------------------------------------------
public:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Активировать
	virtual void Activate(bool isActive);

//--------------------------------------------------------------------------------------------
private:
	//Работа детектора
	void _cdecl Work(float dltTime, long level);
	//Проверить жив или нет
	bool IsAlive(MissionObject * obj);


	//Пересоздать объект
	virtual void Restart()
	{
		ReCreate();
	};

	bool ReadDieEventIndexWithCheck( MOPReader & reader, const char* pcDieTypeName, long& nVal );

//--------------------------------------------------------------------------------------------
protected:
	MOSafePointer m_Object;

	array<MissionTrigger> m_events;

	bool m_bWaitTrigger;

	long m_nDefaultDie;
	long m_nFireDie;
	long m_nCannonDie;
	long m_nMineDie;
	long m_nRamDie;

	float m_time;
	float m_delayTime;
};

#endif

