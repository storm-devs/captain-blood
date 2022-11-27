#pragma once

#include "..\..\..\Common_h\Mission.h"

class IStainManager;

class StainKiller : public MissionObject
{
public:

	 StainKiller();
	~StainKiller();

	bool Create			(MOPReader &reader);
	bool EditMode_Update(MOPReader &reader);

	void Show	 (bool isShow);
	void Activate(bool isActive);

	void Restart()
	{
		ReCreate();
	}

	void Command(const char *id, dword numParams, const char **params);

private:

	void _cdecl Draw(float dltTime, long level);

	void InitParams		(MOPReader &reader);

	void CheckStains();

private:

	Vector m_pos;
	float  m_rad;

	bool m_debug;

	float m_time;

	IStainManager *m_man;

};
