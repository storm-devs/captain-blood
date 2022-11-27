#pragma once

#include "..\..\..\Common_h\Mission.h"
#include "..\..\..\Common_h\ITrail.h"

class Trailer : public MissionObject
{
	struct Object
	{
		MissionObject *p; ITrail *t;
	};

public:

	 Trailer();
	~Trailer();

public:

	bool Create			(MOPReader &reader);
	bool EditMode_Update(MOPReader &reader);

	void Show	 (bool isShow);
	void Activate(bool isActive);

	void Command(const char *id, dword numParams, const char **params);

private:

	void _cdecl Draw(float dltTime, long level);

	void InitParams		(MOPReader &reader);

private:

	array<Object> objects;

};
