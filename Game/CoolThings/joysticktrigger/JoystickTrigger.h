#pragma once
#include "../../..\common_h/Mission.h"

class JoystickTrigger :	public MissionObject
{
	MissionTrigger m_inTrigger, m_outTrigger;
	bool m_prevPauseState;

	void _cdecl Work(float dTime, long level);
public:
	JoystickTrigger(void);
	virtual ~JoystickTrigger(void);

	virtual bool Create(MOPReader & reader);
	virtual void Activate(bool isActive);
};
