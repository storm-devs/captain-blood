#pragma once

#include "..\..\common_h\Mission.h"

class ControlsTrigger : public MissionObject
{
public:

	 ControlsTrigger();
	~ControlsTrigger();

public:

	bool Create			(MOPReader &reader);
	bool EditMode_Update(MOPReader &reader);

	void Show	 (bool isShow);
	void Activate(bool isActive);

	void Command(const char *id, dword numParams, const char **params);

private:

	void InitParams		(MOPReader &reader);

private:

	bool reset_JoyPad;

};
