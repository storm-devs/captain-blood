#pragma once

#include "..\..\..\Common_h\Mission.h"

class Vibrator : public MissionObject
{
public:

	 Vibrator();
	~Vibrator();

public:

	bool Create			(MOPReader &reader);
	bool EditMode_Update(MOPReader &reader);

	void Show	 (bool isShow);
	void Activate(bool isActive);

	void Restart();

	void Command(const char *id, dword numParams, const char **params);

private:

	void _cdecl Draw(float dltTime, long level);

	void InitParams		(MOPReader &reader);

private:

	IForce *force; string profile; long deviceIndex;

	bool looped; bool started;

	bool m_active;

};
