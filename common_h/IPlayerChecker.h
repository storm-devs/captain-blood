#ifndef IUSERCHECKERSERVICE_H
#define IUSERCHECKERSERVICE_H

#include "Mission.h"

class IPlayerChecker : public Service
{
protected:
	IPlayerChecker() {}

public:
	virtual ~IPlayerChecker() {}

	virtual bool IsUpdated(long idx) = 0;
	virtual bool IsPluged(long idx) = 0;
	virtual const char* GetPlayerName(long idx) = 0;
	virtual bool IsUsed(long idx) = 0;
};

class IMultiplayerTrigger : public MissionObject
{
public:
	virtual bool LoadIsOn() = 0;
};

#endif
