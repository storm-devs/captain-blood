#include "ShipStatistics.h"

ShipStatistics::ShipStatistics()
{
	aValues[stt_unknown] = 0.f;
	ResetStatistics();
}

ShipStatistics::~ShipStatistics()
{
}

void ShipStatistics::ResetStatistics()
{
	for( long n=stt_first; n<stt_size; n++ )
	{
		aValues[n] = 0.f;
	}
}

void ShipStatistics::AddStatistic(StatType stt, float val)
{
	if(stt>stt_unknown && stt<stt_size)
	{
		aValues[stt] += val;
	}
}

void ShipStatistics::SetStatistic(StatType stt, float val)
{
	if(stt>stt_unknown && stt<stt_size)
	{
		aValues[stt] = val;
	}
}

float ShipStatistics::GetStatistic(StatType stt)
{
	if(stt>stt_unknown && stt<stt_size)
		return aValues[stt];
	return aValues[stt_unknown];
}

void ShipStatistics::Update(float fDeltaTime)
{
	for( long n=stt_timerFirst; n<=stt_timerLast; n++ )
		aValues[n] += fDeltaTime;
}
