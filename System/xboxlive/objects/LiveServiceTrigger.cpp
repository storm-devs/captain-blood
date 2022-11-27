#include "LiveServiceTrigger.h"
#include "..\..\..\common_h\ILiveService.h"

LiveServiceTrigger::LiveServiceTrigger()
{
	m_pLiveService = NULL;

	m_bStartUpActivate = false;

	m_bIsReady = true;
	m_bIsSignIn = true;
	m_bIsPlugedController = true;

	m_bPause = false;
}

LiveServiceTrigger::~LiveServiceTrigger()
{
}

bool LiveServiceTrigger::Create(MOPReader & reader)
{
	m_pLiveService = (ILiveService*)api->GetService("LiveService");
	Assert( m_pLiveService );

	// по умолчанию проверяем только одно подключение (сингл)
	// если будет многопользовательская арена, то тригер мултиплеера сам поставит требуемое количество
	// пользователей (установка будет только в режиме игры, т.е. в меню логина так и останется один)
	m_pLiveService->SetCheckedUsersQuantity(0);

	// считаем, что в начале миссии у нас сервис полность готов.
	// если реально он не готов, то на первом кадре сработает тригер
	m_bIsReady = true;
	m_Ready.Init( reader );
	m_NotReady.Init( reader );

	// В начале миссии считаем что мы заассигнены
	// если реально это не так, то на первом кадре сработает тригер
	m_bIsSignIn = true;
	m_SignIn.Init( reader );
	m_SignOut.Init( reader );

	// В начале миссии считаем что контроллер для засигненного игрока подключен
	// если реально нет, то сработает тригер на первом кадре
	m_bIsPlugedController = true;
	m_PluginController.Init( reader );
	m_LostController.Init( reader );

	// Активировать объект при создании
	m_bStartUpActivate = reader.Bool();

	// устанавливаем начальные значения, что бы первый раз сервис не срабатывал
	GP::LiveServiceState st = m_pLiveService->GetServiceState();
	// общая готовность
	m_bIsReady = (st & GP::LSSM_Ready);
	if( !m_bIsReady )
	{
		// подключен пользователь?
		m_bIsPlugedController = (st & GP::LSSM_ControllerPluged)!=0;
		// при подключенном пользователе проверяем сигнед состояние
		if( m_bIsPlugedController )
			m_bIsSignIn = (st & GP::LSSM_SignIn)!=0;
		// при отключенном считаем, что мы незасигнены
		else
			m_bIsSignIn = false;

		// все таки вернем обратно состояние подключения - ибо потерю управления определять надо
		m_bIsPlugedController = true;
	}

	// Изначально объект считаем выключенным
	MissionObject::Activate( false );
	return true;
}

void LiveServiceTrigger::PostCreate()
{
	Activate( m_bStartUpActivate );
}

void LiveServiceTrigger::Activate(bool isActive)
{
	MissionObject::Activate(isActive);

	if( isActive )
	{
		SetUpdate(&LiveServiceTrigger::Work, ML_TRIGGERS + 10);
		Registry(ACTIVATE_EVENT_GROUP, &LiveServiceTrigger::ActivateEvent, 0);
		Registry(DEACTIVATE_EVENT_GROUP, &LiveServiceTrigger::DeactivateEvent, 0);
	}
	else
	{
		DelUpdate(&LiveServiceTrigger::Work);
		Unregistry(ACTIVATE_EVENT_GROUP);
		Unregistry(DEACTIVATE_EVENT_GROUP);
	}
}

void LiveServiceTrigger::Command(const char * id, dword numParams, const char ** params)
{
	// Добавить слежение за еще одним пользователем
	if( string::IsEqual(id,"AddPlayer") )
	{
//		m_bNoAutoTriggerWork = false;
	}
}

void _cdecl LiveServiceTrigger::Work(float fDeltaTime, long level)
{
	if( m_bPause ) return;

	if( m_pLiveService )
	{
		// работа тригера готовности
		GP::LiveServiceState st = m_pLiveService->GetServiceState();
		if( m_bIsReady == !(st & GP::LSSM_Ready) )
		{
			m_bIsReady = !m_bIsReady;
			if( m_bIsReady )
				m_Ready.Activate( Mission(), false );
			else
				m_NotReady.Activate( Mission(), false );
		}

		// продолжать проверку
		bool bContinue = true;

		// работа тригера отключения контроллера
		if( bContinue && m_bIsPlugedController == !(st & GP::LSSM_ControllerPluged) )
		{
			m_bIsPlugedController = !m_bIsPlugedController;
			if( m_bIsPlugedController )
				m_PluginController.Activate( Mission(), false );
			else
				m_LostController.Activate( Mission(), false );
			// если не подключен контроллер, то тригер на сигнед не актуален (ибо приоритет нарушается)
			bContinue = m_bIsPlugedController;
		}

		// работа тригера подключения пользователя
		if( bContinue && m_bIsSignIn == !(st & GP::LSSM_SignIn) )
		{
			m_bIsSignIn = !m_bIsSignIn;
			if( m_bIsSignIn )
				m_SignIn.Activate( Mission(), false );
			else
				m_SignOut.Activate( Mission(), false );
		}
	}
}

void _cdecl LiveServiceTrigger::ActivateEvent(const char * group, MissionObject * sender)
{
	m_bPause = false;
}

void _cdecl LiveServiceTrigger::DeactivateEvent(const char * group, MissionObject * sender)
{
	m_bPause = true;
}


MOP_BEGINLISTCG(LiveServiceTrigger, "XLive Triggers", '1.00', 110, "Live Service Triggers", "Logic")
	MOP_GROUPBEGC("Ready State","Trigger for change Ready state")
		MOP_MISSIONTRIGGER("Ready ")
		MOP_MISSIONTRIGGER("Not Ready ")
	MOP_GROUPEND()
	MOP_GROUPBEGC("SignIn State","Trigger for change SignIn state")
		MOP_MISSIONTRIGGER("SignIn ")
		MOP_MISSIONTRIGGER("SignOut ")
	MOP_GROUPEND()
	MOP_GROUPBEGC("Controller State","Trigger for change Controller state")
		MOP_MISSIONTRIGGER("Plugin Controller ")
		MOP_MISSIONTRIGGER("Lost Controller ")
	MOP_GROUPEND()
	MOP_BOOL("Active",true)
MOP_ENDLIST(LiveServiceTrigger)
