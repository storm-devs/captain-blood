#ifndef ShipStatistics_h
#define ShipStatistics_h

#include "..\..\common_h\mission.h"

class ShipStatistics
{
public:
	enum StatType
	{
		stt_unknown,
		stt_first,

		stt_receivedDamage = stt_first,
		stt_makedDamage,

		stt_curTime,			stt_timerFirst = stt_curTime,
		stt_lastAttackTime,
		stt_lastDamageTime,		stt_timerLast = stt_lastDamageTime,

		stt_size
	};

public:
	ShipStatistics();
	~ShipStatistics();

	void ResetStatistics();
	void AddStatistic(StatType stt, float val);
	void SetStatistic(StatType stt, float val);
	float GetStatistic(StatType stt);

	void Update(float fDeltaTime);

protected:
	float aValues[stt_size];
};

#endif
