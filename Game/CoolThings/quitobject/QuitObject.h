#pragma once

#include "..\..\..\Common_h\Mission.h"
#include "..\..\..\Common_h\ITrail.h"

class QuitObject : public MissionObject
{

public:

	QuitObject();
	virtual ~QuitObject();

public:

	bool Create			(MOPReader &reader);
	bool EditMode_Update(MOPReader &reader);

	void Activate(bool isActive);
};
