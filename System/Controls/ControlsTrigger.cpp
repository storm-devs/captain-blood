#include "ControlsTrigger.h"
#include "JoystickManager.h"

extern JoystickManager *joystickManager;

ControlsTrigger:: ControlsTrigger()
{
}

ControlsTrigger::~ControlsTrigger()
{
}

bool ControlsTrigger::Create		 (MOPReader &reader)
{
	InitParams(reader);

	return true;
}

bool ControlsTrigger::EditMode_Update(MOPReader &reader)
{
	InitParams(reader);

	return true;
}

void ControlsTrigger::Show(bool isShow)
{
	MissionObject::Show(isShow);

	//
}

void ControlsTrigger::Activate(bool isActive)
{
	MissionObject::Activate(isActive);

	if( isActive )
	{
		if( reset_JoyPad )
		{
			joystickManager->ResetTable();

			LogicDebug("Reset JoyPad table.");
		}
	}
}

void ControlsTrigger::Command(const char *id, dword numParams, const char **params)
{
	if( string::IsEmpty(id))
		return;

	if( false )
	{
	}
	else
	{
		LogicDebugError("Unknown command \"%s\".",id);
	}
}

void ControlsTrigger::InitParams(MOPReader &reader)
{
	reset_JoyPad = reader.Bool();

	Activate(reader.Bool());
}

static char desc[] =

"ControlsTrigger\n\n"
"    Use to reset controls service";

MOP_BEGINLISTCG(ControlsTrigger, "ControlsTrigger", '1.00', 2000, desc, "Interface")

	MOP_GROUPBEG("Reset")

		MOP_BOOLC("JoyPad table", true, "Сбросить таблицу джойстиков")

	MOP_GROUPEND()

	MOP_BOOL("Active", false)

MOP_ENDLIST(ControlsTrigger)
