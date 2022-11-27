#include "JoystickTrigger.h"
#include "..\..\..\Common_h/InputSrvCmds.h"

JoystickTrigger::JoystickTrigger(void) :
m_prevPauseState(false)
{
}

JoystickTrigger::~JoystickTrigger(void)
{
}

bool JoystickTrigger::Create( MOPReader & reader )
{
	m_inTrigger.Init(reader);
	m_outTrigger.Init(reader);
	Activate(reader.Bool());
	return true;
}

void _cdecl JoystickTrigger::Work( float dTime, long level )
{	
	InputSrvQueryNeedPause query;
	Controls().ExecuteCommand(query);

	if (query.needPause && !m_prevPauseState)
		m_outTrigger.Activate(Mission(), false, this);

	if (!query.needPause && m_prevPauseState)
		m_inTrigger.Activate(Mission(), false, this);

	m_prevPauseState = query.needPause;
}

void JoystickTrigger::Activate( bool isActive )
{
	MissionObject::Activate(isActive);

	DelUpdate(&JoystickTrigger::Work);
	if (isActive)
		SetUpdate(&JoystickTrigger::Work, ML_TRIGGERS);
}


const char Description[] = "Trigger for joystick plugging in/out.";
MOP_BEGINLISTCG(JoystickTrigger, "Joystick Trigger", '1.00', 101, Description, "Logic");
	MOP_MISSIONTRIGGER("Plug-in event")
	MOP_MISSIONTRIGGER("Plug-out event")
	MOP_BOOL("Active", true)
MOP_ENDLIST(JoystickTrigger)