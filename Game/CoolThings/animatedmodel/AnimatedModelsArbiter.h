#pragma once

#include "..\..\..\Common_h\Mission.h"

class AnimatedModel;

class AnimatedModelsArbiter : public MissionObject
{
public:

	 AnimatedModelsArbiter();
	~AnimatedModelsArbiter();

public:

	virtual bool Create(MOPReader &reader);

	void _cdecl Draw(float dltTime, long level);

public:

	#ifndef NO_CONSOLE

	void _cdecl Console_AniModel(const ConsoleStack &params);

	#endif

public:

	void Activate(AnimatedModel *model, bool isActive);

private:

	array<AnimatedModel *> models;

	bool debugDraw;

	string filter;

};
