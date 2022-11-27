#include "SplashManager.h"

const float blood_resp = 0.50f;

#include "../PostProcess/PostProcess.h"

SplashManager:: SplashManager()
{
	PostProcessService *p = (PostProcessService *)api->GetService("PostProcessService");
	
	if( p )
		splash = p->GetSplashFilter();
	else
		splash = null;

	bloodResp = 0.0f;
}

SplashManager::~SplashManager()
{
	if( splash )
		splash->Release(this);
}

bool SplashManager::Create		  (MOPReader &reader)
{
	InitParams(reader);

	Show	(true);
	Activate(true);

	return true;
}

bool SplashManager::EditMode_Update(MOPReader &reader)
{
	InitParams(reader);

	return true;
}

void SplashManager::Show(bool isShow)
{
	MissionObject::Show(isShow);

	if( isShow )
		SetUpdate(&SplashManager::Draw,ML_ALPHA5);
	else
		DelUpdate(&SplashManager::Draw);
}

void SplashManager::Activate(bool isActive)
{
	MissionObject::Activate(isActive);

	//
}

void _cdecl SplashManager::Draw(float dltTime, long level)
{
	if( splash )
		splash->Update(this,dltTime);

/*	bloodResp -= dltTime;

	if( bloodResp < 0.0f )
	{
		bloodResp = blood_resp;

		AddBloodPuff();
	}*/
}

void SplashManager::Command(const char *id, dword numParams, const char **params)
{
	//
}

void SplashManager::AddBloodPuff()
{
	if( splash )
		splash->AddBloodPuff(this);
}

void SplashManager::InitParams(MOPReader &reader)
{
	//
}
/*
MOP_BEGINLISTCG(SplashManager, "SplashManager", '1.00', 2000, "SplashManager", "Default")
MOP_ENDLIST(SplashManager)
*/
MOP_BEGINLIST(SplashManager, "", '1.00', 2000)
MOP_ENDLIST(SplashManager)
