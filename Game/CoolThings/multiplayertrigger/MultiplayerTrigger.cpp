#include "MultiplayerTrigger.h"
#include "UserCheckerService.h"
#include "..\..\..\Common_h\ILiveService.h"

MultiplayerTrigger::MultiplayerTrigger()
{
	m_pChecker = NULL;
	m_bEnableStart = false;

	for( long n=0; n<dwPlayerQnt; n++ )
	{
		m_player[n].isPluged = false;
		m_player[n].checkPlugin = false;
		m_player[n].checkPlugout = false;
		m_player[n].joinControl = m_player[n].leaveControl = m_player[n].startControl = "";
	}
}

MultiplayerTrigger::~MultiplayerTrigger()
{
	if(m_pChecker) m_pChecker->SetControlService(NULL);
	m_pChecker = NULL;
	LiveService().SetCurrentMissionPtr( NULL );
}

void MultiplayerTrigger::Command(const char * id, dword numParams, const char ** params)
{
	if( !id ) return;
	if (string::IsEqual(id, "useron"))
	{
		if(numParams >= 1)
		{
			m_pChecker->SetUsed( atoi(params[0])-1, true );
			UpdateLiveServicePlayerMask();
		}
	}
	else if(string::IsEqual(id, "useroff"))
	{
		if(numParams >= 1)
		{
			m_pChecker->SetUsed( atoi(params[0])-1, false );
			UpdateLiveServicePlayerMask();
		}
	}
}


void _cdecl MultiplayerTrigger::Work(float fDeltaTime, long level)
{
	Assert(m_pChecker);

	long nPlayerQ = 0;
	for( long n=0; n<dwPlayerQnt; n++ )
	{
		// подсоединяем/отсоединяем игроков если нажимаем кнопки
		if( m_player[n].isPluged )
		{
			if( !m_pChecker->IsUsed(n) && m_player[n].joinControl && Controls().GetControlStateBool(m_player[n].joinControl) )
			{
				if( !m_bLoadConfig )
				{
					m_pChecker->SetUsed(n,true);
					UpdateLiveServicePlayerMask();
				}
				m_player[n].join.Activate( Mission(), false );
			}
			else
			if( m_pChecker->IsUsed(n) && m_player[n].leaveControl && Controls().GetControlStateBool(m_player[n].leaveControl) )
			{
				if( !m_bLoadConfig )
				{
					m_pChecker->SetUsed(n,false);
					UpdateLiveServicePlayerMask();
				}
				m_player[n].leave.Activate( Mission(), false );
			}
		}

		if( m_bLoadConfig && !m_pChecker->IsUsed(n) )
			continue;
		if( m_pChecker->IsUsed(n) ) nPlayerQ++;
		CheckPlayerdata( n, m_player[n] );
	}

	if( !m_bLoadConfig )
	{
		// можем ли запускать миссию
		if( nPlayerQ > 1 &&	!m_bEnableStart )
		{
			m_bEnableStart = true;
			m_enableStart.Activate( Mission(), false );
		}
		if( nPlayerQ < 2 &&	m_bEnableStart )
		{
			m_bEnableStart = false;
			m_disableStart.Activate( Mission(), false );
		}

		// надо ли запускать миссию
		if( m_bEnableStart )
		{
			for( n=0; n<dwPlayerQnt; n++ )
				if( m_pChecker->IsUsed(n) && m_player[n].startControl && Controls().GetControlStateBool(m_player[n].startControl) )
					break;
			if( n<dwPlayerQnt )
				m_launchStart.Activate( Mission(), false );
		}
	}
}

bool MultiplayerTrigger::Create(MOPReader & reader)
{
	// создадим проверялщик состояния подключения и т.п.
	m_pChecker = (PlayerChecker*)api->GetService( "PlayerChecker" );
	Assert( m_pChecker );
	// запомним старые контролки (что бы вернуть их после завершения миссии)
	if( !EditMode_IsOn() )
		m_pChecker->SetControlService(&Controls());

	ReadMOPs(reader);

	SetUpdate(&MultiplayerTrigger::Work, ML_TRIGGERS);

	LiveService().SetCurrentMissionPtr( &Mission() );

	return true;
}

void MultiplayerTrigger::PostCreate()
{
	if( m_bLoadConfig )
		LoadDatabase();
	else
		SaveDatabase();
}


void MultiplayerTrigger::ReadMOPs(MOPReader & reader)
{
	m_bLoadConfig = reader.Bool();

	m_enableStart.Init(reader);
	m_disableStart.Init(reader);
	m_launchStart.Init(reader);

	ReadPlayerSetting( reader, m_player[0] );
	ReadPlayerSetting( reader, m_player[1] );
	ReadPlayerSetting( reader, m_player[2] );
	ReadPlayerSetting( reader, m_player[3] );
}

void MultiplayerTrigger::ReadPlayerSetting(MOPReader & reader, PlayerConfig& conf)
{
	long n,q;

	conf.aNameGUI.DelAll();
	q = reader.Array();
	conf.aNameGUI.AddElements(q);
	for(n=0; n<q; n++)
	{
		conf.aNameGUI[n] = reader.String();
	}

	conf.joinControl = reader.String().c_str();
	conf.leaveControl = reader.String().c_str();
	conf.startControl = reader.String().c_str();

	conf.plugin.Init(reader);
	conf.plugout.Init(reader);
	conf.join.Init(reader);
	conf.leave.Init(reader);
}

void MultiplayerTrigger::LoadDatabase()
{
	Assert(m_pChecker);
	for( long n=0; n<dwPlayerQnt; n++ )
	{
		// никаких проверок на подключения во время игры
		m_player[n].checkPlugin = false;
		// если игрок участвует в игре, то ставим проверку на его отключение
		if( m_pChecker->IsUsed(n) )
		{
			m_player[n].checkPlugout = true;
			SetPlayerName(n);
			if( m_pChecker->IsPluged(n) )
				PluginPlayer(n);
		}
		else
		// если игрок не участвует, то никаких ему проверок
		{
			m_player[n].checkPlugout = false;
			m_player[n].isPluged = false;
			SetPlayerName(n);
		}
	}
	UpdateLiveServicePlayerMask();
}

void MultiplayerTrigger::SaveDatabase()
{
	for( long n=0; n<dwPlayerQnt; n++ )
	{
		m_player[n].checkPlugin = true;
		m_player[n].checkPlugout = true;
		SetPlayerName(n);
		m_pChecker->SetUsed(n,false);
		if( m_pChecker->IsPluged(n) )
			PluginPlayer(n);
		else
			UnplugPlayer(n);
	}
	UpdateLiveServicePlayerMask();
}

void MultiplayerTrigger::CheckPlayerdata( long idx, PlayerConfig& conf )
{
	bool bUpdateName = m_pChecker->IsUpdated(idx);

	if( conf.checkPlugin && m_pChecker->IsPluged(idx) )
	{
		PluginPlayer(idx);
		bUpdateName = true;
		conf.checkPlugin = false;
		conf.checkPlugout = true;
	}
	else
	if( conf.checkPlugout && !m_pChecker->IsPluged(idx) )
	{
		UnplugPlayer(idx);
		bUpdateName = true;
		conf.checkPlugout = false;
		conf.checkPlugin = true;
	}

	if( bUpdateName )
	{
		SetPlayerName(idx);
		UpdateLiveServicePlayerMask();
	}
}

void MultiplayerTrigger::UpdateLiveServicePlayerMask()
{
	Assert(m_pChecker);
	long nPlugedUserQuantity = 0;
	long nIdx = 1;
	for( long n=0; n<dwPlayerQnt; n++ )
	{
		// если игрок участвует в игре, то ставим в маске его флажек
		if( m_pChecker->IsUsed(n) )
			nPlugedUserQuantity |= nIdx;
		nIdx = nIdx << 1;
	}
	// ставим количество пользователей за которыми надо следить
	LiveService().SetCheckedUsersQuantity(nPlugedUserQuantity);
}

void MultiplayerTrigger::SetPlayerName(long idx)
{
	const char* apcName[1];
	apcName[0] = m_pChecker->GetPlayerName( idx );
	for( long n=0; n<m_player[idx].aNameGUI; n++ )
	{
		MOSafePointer pMO;
		FindObject( m_player[idx].aNameGUI[n], pMO );
		if( pMO.Ptr() )
		{
			pMO.Ptr()->Command("SetString", 1, apcName);
		}
	}
}

void MultiplayerTrigger::PluginPlayer(long idx)
{
	m_player[idx].isPluged = true;
	m_player[idx].plugin.Activate( Mission(), false );
}

void MultiplayerTrigger::UnplugPlayer(long idx)
{
	m_player[idx].isPluged = false;
	m_player[idx].plugout.Activate( Mission(), false );
}




static char TriggerDescription[] =
"-= Multiplayer mode joypad trigger =-\n"
"Supported commands:\n"
"useron <player index {1,4}> - join player to multiplayer\n"
"useroff <player index {1,4}> - exit player from multiplayer";

#define MOP_PLAYERCONFIG(s) \
	MOP_ARRAYBEGC(s##"NameGUI",0,10,"Array of widgets showing player name") \
		MOP_STRINGC("WidgetName","","Name of widget object which receive player name") \
	MOP_ARRAYEND \
	MOP_STRINGC(s##"Join control","","Name of control to join player") \
	MOP_STRINGC(s##"Leave control","","Name of control to leave player") \
	MOP_STRINGC(s##"Start control","","Name of control to start multiplayer match") \
	MOP_MISSIONTRIGGER(s##"Plug-in event") \
	MOP_MISSIONTRIGGER(s##"Plug-out event") \
	MOP_MISSIONTRIGGER(s##"Join event") \
	MOP_MISSIONTRIGGER(s##"Leave event")

MOP_BEGINLISTCG(MultiplayerTrigger, "MultiplayerTrigger", '1.00', 100, TriggerDescription, "logic")
	MOP_BOOLC("Load setting",false,"Load player setting from database")
	MOP_GROUPBEGC("Start switcher","Events for start button")
		MOP_MISSIONTRIGGER("Enable ")
		MOP_MISSIONTRIGGER("Disable ")
		MOP_MISSIONTRIGGER("Launch ")
	MOP_GROUPEND()
	MOP_GROUPBEG("Player1") MOP_PLAYERCONFIG("Player1") MOP_GROUPEND()
	MOP_GROUPBEG("Player2")	MOP_PLAYERCONFIG("Player2")	MOP_GROUPEND()
	MOP_GROUPBEG("Player3")	MOP_PLAYERCONFIG("Player3")	MOP_GROUPEND()
	MOP_GROUPBEG("Player4")	MOP_PLAYERCONFIG("Player4")	MOP_GROUPEND()
MOP_ENDLIST(MultiplayerTrigger)
