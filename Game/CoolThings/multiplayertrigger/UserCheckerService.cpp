#include "UserCheckerService.h"
#include "..\..\..\common_h/InputSrvCmds.h"

PlayerChecker::PlayerChecker()
{
	for(long n=0; n<dwPlayerQnt; n++)
	{
		m_bReassign[n] = false;
		m_bPluged[n] = true;
		m_sPlayerName[n] = string("Player") + (n+1);
		m_bUseUser[n] = false;
		m_dwUserIndex[n] = -1;
	}
}

PlayerChecker::~PlayerChecker()
{
}

bool PlayerChecker::Init()
{
	api->SetStartFrameLevel(this, Core_DefaultExecuteLevel);
	Start();
	return true;
}

void PlayerChecker::StartFrame(float dltTime)
{
	Update();
}

bool PlayerChecker::IsUpdated(long idx)
{
	if( idx<0 || idx>=dwPlayerQnt )
		return false;
	return m_bReassign[idx];
}

bool PlayerChecker::IsPluged(long idx)
{
	if( idx<0 || idx>=dwPlayerQnt )
		return true;
	return m_bPluged[idx];
}

const char* PlayerChecker::GetPlayerName(long idx)
{
	if(idx<0 || idx>=dwPlayerQnt) return "";
	return m_sPlayerName[idx].GetBuffer();
}

bool PlayerChecker::IsUsed(long idx)
{
	if( idx<0 || idx>=dwPlayerQnt )
		return false;
	return m_bUseUser[idx];
}

void PlayerChecker::SetUsed(long idx, bool bUse)
{
	if( idx>=0 && idx<dwPlayerQnt )
		m_bUseUser[idx] = bUse;
}



#ifdef _XBOX
CREATE_SERVICE_NAMED( "PlayerChecker", XBoxPlayerChecker, 120 )

XBoxPlayerChecker::XBoxPlayerChecker()
{
	m_hNotify = NULL;
}

XBoxPlayerChecker::~XBoxPlayerChecker()
{
	if( m_hNotify != NULL )
	{
		CloseHandle( m_hNotify );
		m_hNotify = NULL;
	}
}

void XBoxPlayerChecker::Start()
{
	m_hNotify = XNotifyCreateListener( XNOTIFY_SYSTEM );
	for( long n=0; n<dwPlayerQnt; n++ )
	{
		XINPUT_STATE xst;
		m_bPluged[n] = m_dwUserIndex[n] < 0 ? false : XInputGetState(m_dwUserIndex[n],&xst)==ERROR_SUCCESS;
		UpdateUser(n);
	}
}

void XBoxPlayerChecker::Update()
{
	Assert( m_hNotify != NULL );

	DWORD dwNotifyID, dwParam;

	// проверка на подсоединение отсоединение контроллера
	if( XNotifyGetNext(m_hNotify, XN_SYS_INPUTDEVICESCHANGED, &dwNotifyID, &dwParam) )
	{
		XINPUT_STATE xst;
		m_bPluged[0] = m_dwUserIndex[0] < 0 ? false : XInputGetState(m_dwUserIndex[0],&xst)==ERROR_SUCCESS;
		m_bPluged[1] = m_dwUserIndex[1] < 0 ? false : XInputGetState(m_dwUserIndex[1],&xst)==ERROR_SUCCESS;
		m_bPluged[2] = m_dwUserIndex[2] < 0 ? false : XInputGetState(m_dwUserIndex[2],&xst)==ERROR_SUCCESS;
		m_bPluged[3] = m_dwUserIndex[3] < 0 ? false : XInputGetState(m_dwUserIndex[3],&xst)==ERROR_SUCCESS;
	}

	// проверка на реасигнед
	if( XNotifyGetNext(m_hNotify, XN_SYS_SIGNINCHANGED, &dwNotifyID, &dwParam) )
	{
		m_bReassign[0] = m_bReassign[1] = m_bReassign[2] = m_bReassign[3] = true;

		UpdateUser(0);
		UpdateUser(1);
		UpdateUser(2);
		UpdateUser(3);
	}
	else
	{
		m_bReassign[0] = m_bReassign[1] = m_bReassign[2] = m_bReassign[3] = false;
	}
}

IControls* XBoxPlayerChecker::SetControlService(IControls* pControl)
{
	if( pControl )
	{
		InputSrvQueryAssignIndex assignIdx(0);

		// включаем проверку только для тех кого надо проверить
		XINPUT_STATE xst;
		for( long n=0; n<dwPlayerQnt; n++ )
		{
			// спрашиваем у контролов какой разьем для каждого пользователя по порядку
			assignIdx.joypadIndex = n;
			pControl->ExecuteCommand( assignIdx );

			if( assignIdx.assignIndex>=0 && assignIdx.assignIndex<dwPlayerQnt )
			{
				m_dwUserIndex[n] = assignIdx.assignIndex;
				m_bPluged[n] = XInputGetState(m_dwUserIndex[n],&xst)==ERROR_SUCCESS;
			}
			else
			{
				m_dwUserIndex[n] = -1;
				m_bPluged[n] = false;
			}
			UpdateUser(n);
		}
	}
	return NULL;
}

void XBoxPlayerChecker::UpdateUser(long n)
{
	DWORD dwRes;
	char param[1024];

	if( m_dwUserIndex[n] < 0 || XUserGetSigninState(m_dwUserIndex[n]) == eXUserSigninState_NotSignedIn )
		m_sPlayerName[n] = string("Player")+(n+1);
	else
	{
		dwRes = XUserGetName( m_dwUserIndex[n], param, sizeof(param) );
		if( dwRes == ERROR_SUCCESS )
		{
			m_sPlayerName[n] = param;
			api->Trace("XBoxPlayerChecker: Update user name index=%d, name=%s",n,param);
		}
		else
		{
			api->Trace("XBoxPlayerChecker Error! XUserGetName(%d) error code %d",n,dwRes);
			m_sPlayerName[n] = string("Player")+(n+1);
		}
	}
}

#else

CREATE_SERVICE_NAMED( "PlayerChecker", PCPlayerChecker, 120 )

PCPlayerChecker::PCPlayerChecker()
{
	m_pControlService = NULL;
}

PCPlayerChecker::~PCPlayerChecker()
{
	m_pControlService = NULL;
}

void PCPlayerChecker::Start()
{
}

void PCPlayerChecker::Update()
{
	if( m_pControlService )
	{
		InputSrvQueryJoypadConnect query(0);
		m_pControlService->ExecuteCommand(query);
		m_bPluged[0] = query.isConnect;

		query.joypadIndex = 1;
		m_pControlService->ExecuteCommand(query);
		m_bPluged[1] = query.isConnect;

		query.joypadIndex = 2;
		m_pControlService->ExecuteCommand(query);
		m_bPluged[2] = query.isConnect;

		query.joypadIndex = 3;
		m_pControlService->ExecuteCommand(query);
		m_bPluged[3] = query.isConnect;
	}
}

#endif
