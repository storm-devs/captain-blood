#ifndef _LIVE_SERVICE_TRIGGER_H_
#define _LIVE_SERVICE_TRIGGER_H_

class ILiveService;

#include "..\..\..\Common_h\Mission.h"

class LiveServiceTrigger : public MissionObject
{
public:
	LiveServiceTrigger();
	~LiveServiceTrigger();

	virtual bool Create(MOPReader & reader);
	virtual void PostCreate();

	virtual void Activate(bool isActive);

	//Обработчик команд для объекта
	virtual void Command(const char * id, dword numParams, const char ** params);

	void _cdecl Work(float fDeltaTime, long level);

	void _cdecl ActivateEvent(const char * group, MissionObject * sender);
	void _cdecl DeactivateEvent(const char * group, MissionObject * sender);

protected:
	ILiveService* m_pLiveService;

	bool m_bIsReady;
	MissionTrigger m_Ready;
	MissionTrigger m_NotReady;

	bool m_bIsSignIn;
	MissionTrigger m_SignIn;
	MissionTrigger m_SignOut;

	bool m_bIsPlugedController;
	MissionTrigger m_PluginController;
	MissionTrigger m_LostController;

	bool m_bStartUpActivate;

	bool m_bPause;
};

#endif
