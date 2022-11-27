#pragma once

#include "..\..\..\Common_h\Mission.h"

class Splash;

class SplashManager : public MissionObject
{
public:

	 SplashManager();
	~SplashManager();

public:

	bool Create			(MOPReader &reader);
	bool EditMode_Update(MOPReader &reader);

	void Show	 (bool isShow);
	void Activate(bool isActive);

	void Command(const char *id, dword numParams, const char **params);
	
public:

	void AddBloodPuff();

private:

	void _cdecl Draw(float dltTime, long level);

	void InitParams		(MOPReader &reader);

private:

	Splash *splash; float bloodResp;

};
