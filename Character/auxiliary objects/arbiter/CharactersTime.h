#pragma once

#include "..\..\..\Common_h\mission.h"

class CharactersTime
{
public:
	CharactersTime();
	~CharactersTime();

	void Execute(float deltaTime);

	__forceinline double GetTime() const { return db_time; }
	__forceinline dword GetTimeDword() const { return dw_time; }
	__forceinline float GetTimeFloat() const { return float(db_time); }

	dword Convert2Dword(float time) const;

private:
	dword dw_time;
	double db_time;
};

