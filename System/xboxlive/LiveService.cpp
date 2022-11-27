#include "liveservice.h"
#include "..\..\Common_h\LocStrings.h"
#include "..\..\Common_h\FileService.h"
#include "..\..\Common_h\Render.h"
#include "..\..\Common_h\IPlayerChecker.h"

#ifdef _XBOX
	#include "xbox\XboxLiveManager.h"
#else
	#include "pc\PCLiveManager.h"
#endif

#include "..\..\Common_h\compression\compressor.h"

#define ToSignRange(x) (x  - 0.5f) * 2.0f

CREATE_SERVICE( LiveService, 1 )

LiveService::LiveService() :
	m_aAddAchievements(_FL_),
	m_aAchievements(_FL_),
	m_aRichAchievement(_FL_),
	m_aProfileData(_FL_),
	m_aOptionData(_FL_)
{
#ifdef PROTECT_ENABLE
	PSA_DummyFunction();
#endif

	m_bServiceIsReady = false;
	m_pLiveManager = NULL;
	m_bDemoMode = false;
	m_oldReadyState = ILiveManager::rs_ready;

	m_bLoadAchievement = false;
	m_bWriteAchievement = false;
	m_fRichOutTime = 0.f;

	m_bIsSaveData = false;
	m_dwHaveWaitingData = 0;

	m_bProfileSettingsIsReady = false;

	m_bShowDebugInfo = false;
	m_bInitConsole = false;
	m_pConsole = NULL;

	m_pFileService = NULL;
	m_pRenderService = NULL;

	m_dwCurContext = CONTEXT_PRESENCE_PRESENCE_MENU;

	m_pCurrentMission = NULL;

	// инициализация списка ачивментов
	m_aAchievements.AddElements(GP::Achievement_max_count);
	SetAchievementData (m_aAchievements[GP::First_Blood],			ACHIEVEMENT_FIRST_BLOOD,			1,		0);
	SetAchievementData (m_aAchievements[GP::Murderer],				ACHIEVEMENT_SLAYER,					500,	0);
	SetAchievementData (m_aAchievements[GP::Executor],				ACHIEVEMENT_EXECUTIONER,			2500,	0);
	SetAchievementData (m_aAchievements[GP::Reaper],				ACHIEVEMENT_RIPPER,					1,		0);
	SetAchievementData (m_aAchievements[GP::Seamen],				ACHIEVEMENT_SAILOR,					1,		0);
	SetAchievementData (m_aAchievements[GP::Musketeer],				ACHIEVEMENT_MUSKETEER,				10,		0);
	SetAchievementData (m_aAchievements[GP::Duelist],				ACHIEVEMENT_DUELIST,				15,		0);
	SetAchievementData (m_aAchievements[GP::Butcher],				ACHIEVEMENT_BUTCHER,				25,		0);
	SetAchievementData (m_aAchievements[GP::Dead_shot],				ACHIEVEMENT_SHARPSHOOTER,			10,		0);
	SetAchievementData (m_aAchievements[GP::Knight],				ACHIEVEMENT_KNIGHT,					30,		0);
	SetAchievementData (m_aAchievements[GP::Marksman],				ACHIEVEMENT_SNIPER,					4,		0);
	SetAchievementData (m_aAchievements[GP::Destroyer],				ACHIEVEMENT_DEMOLITION,				3,		0);
	SetAchievementData (m_aAchievements[GP::Slayer],				ACHIEVEMENT_SHREDDER,				100,	0);
	SetAchievementData (m_aAchievements[GP::Swashbuckler],			ACHIEVEMENT_HEADCUTTER,				3,		0);
	SetAchievementData (m_aAchievements[GP::Killa],					ACHIEVEMENT_MURDERER,				1,		0);
	SetAchievementData (m_aAchievements[GP::Rage],					ACHIEVEMENT_RAGE,					15,		0);
	SetAchievementData (m_aAchievements[GP::Weaponmaster],			ACHIEVEMENT_WEAPON_MASTER,			15,		0);
	SetAchievementData (m_aAchievements[GP::Miser],					ACHIEVEMENT_RACKETEER,				15,		0);
	SetAchievementData (m_aAchievements[GP::Ruthless],				ACHIEVEMENT_MERCILESS,				30,		0);
	SetAchievementData (m_aAchievements[GP::Berserker],				ACHIEVEMENT_BERSERKER,				10,		0);
	SetAchievementData (m_aAchievements[GP::Bosun],					ACHIEVEMENT_BOATSWAIN,				1,		0);
	SetAchievementData (m_aAchievements[GP::Captain],				ACHIEVEMENT_CAPTAIN,				1,		0);
	SetAchievementData (m_aAchievements[GP::Caper],					ACHIEVEMENT_CAPER,					1,		0);
	SetAchievementData (m_aAchievements[GP::Jolly_Roger],			ACHIEVEMENT_JOLLY_ROGER,			1,		0);
	SetAchievementData (m_aAchievements[GP::Pirate_King],			ACHIEVEMENT_PIRATE_KING,			1,		0);
	SetAchievementData (m_aAchievements[GP::Avenger],				ACHIEVEMENT_AVENGER,				1,		0);
	SetAchievementData (m_aAchievements[GP::Defender_of_the_Weak],	ACHIEVEMENT_DEFENDER_OF_THE_WEAK,	1,		0);
	SetAchievementData (m_aAchievements[GP::Master_of_Revenge],		ACHIEVEMENT_MASTER_OF_REVENGE,		1,		0);
	SetAchievementData (m_aAchievements[GP::Conqueror],				ACHIEVEMENT_PETER_THE_CONQUEROR,	1,		0);
	SetAchievementData (m_aAchievements[GP::Champion],				ACHIEVEMENT_PETER_THE_VICTORIOUS,	1,		0);
	SetAchievementData (m_aAchievements[GP::Captain_Blood],			ACHIEVEMENT_CAPTAIN_BLOOD,			1,		0);
	SetAchievementData (m_aAchievements[GP::Shark_food],			ACHIEVEMENT_FISH_FOOD,				5,		0);
	SetAchievementData (m_aAchievements[GP::Trickster],				ACHIEVEMENT_TRICKSTER,				1,		0);
	SetAchievementData (m_aAchievements[GP::Door_cracker],			ACHIEVEMENT_BURGLAR,				10,		0);
	SetAchievementData (m_aAchievements[GP::Juggernaut],			ACHIEVEMENT_UNSTOPPABLE,			1,		0);
	SetAchievementData (m_aAchievements[GP::Looter],				ACHIEVEMENT_LOOTER,					50,		0);
	SetAchievementData (m_aAchievements[GP::Moneybags],				ACHIEVEMENT_THE_MONEY_BAG,			5000,	0);
	SetAchievementData (m_aAchievements[GP::Treasure_hunter],		ACHIEVEMENT_TREASURE_HUNTER,		10000,	0);
	SetAchievementData (m_aAchievements[GP::Master],				ACHIEVEMENT_MASTER,					1,		0);
	SetAchievementData (m_aAchievements[GP::Gunmaster],				ACHIEVEMENT_GUNMASTER,				10,		0);
	SetAchievementData (m_aAchievements[GP::Catapult],				ACHIEVEMENT_CATAPULT,				30,		0);
	SetAchievementData (m_aAchievements[GP::Novice],				ACHIEVEMENT_NOVICE,					1,		adf_multiplay);
	SetAchievementData (m_aAchievements[GP::Survivor],				ACHIEVEMENT_SURVIVOR,				1,		adf_multiplay);
	SetAchievementData (m_aAchievements[GP::Veteran],				ACHIEVEMENT_VETERAN,				10,		adf_multiplay);
	SetAchievementData (m_aAchievements[GP::Manslaughter],			ACHIEVEMENT_MANSLAUGHTER,			20,		adf_multiplay);
	SetAchievementData (m_aAchievements[GP::Meat_grinder],			ACHIEVEMENT_MASSACRE,				50,		adf_multiplay);
	SetAchievementData (m_aAchievements[GP::Seawolf],				ACHIEVEMENT_SEA_WOLF,				1,		0);
	SetAchievementData (m_aAchievements[GP::Terror_of_the_Sea],		ACHIEVEMENT_TERROR_OF_THE_SEA,		24,		0);

	m_dwProtData[0] = 0x134;
	m_dwProtData[1] = 0x7132;
	m_dwProtData[2] = 0x87B;
}

LiveService::~LiveService()
{
	DELETE( m_pLiveManager );
	m_bServiceIsReady = false;
	m_pCurrentMission = NULL;
}

void __declspec(dllexport) LiveService::InitService()
{
	// заведем класс для работы с Live сервисом (для PC или Xbox)
#ifdef _XBOX
	m_pLiveManager = NEW XboxLiveManager;
#else
	m_pLiveManager = NEW PCLiveManager;
#endif
	Assert( m_pLiveManager );
	m_pLiveManager->Init();

	// ставим текущий контекст
	m_pLiveManager->ChangeContext( m_dwCurContext );

	// файловый сервис
	m_pFileService = (IFileService*)api->GetService("FileService");
	Assert(m_pFileService);
	// системный инишник
	IIniFile* pIni = m_pFileService->SystemIni();

	// строка ИД дефолтного языка локализации
	const char* pcDefLangID = pIni ? pIni->GetString("", "localization", ILocStrings_LocalizationDefaultValue) : ILocStrings_LocalizationDefaultValue;
	crt_strcpy(m_ProfileSettings.idLang, sizeof(m_ProfileSettings.idLang), pcDefLangID);

	// дефолтные значения для громкости звука
	m_ProfileSettings.globalVolume = pIni ? pIni->GetFloat("sound", "global volume", 1.f) : 1.f;
	m_ProfileSettings.fxVolume = pIni ? pIni->GetFloat("sound", "fx volume", 1.f) : 1.f;
	m_ProfileSettings.musicVolume = pIni ? pIni->GetFloat("sound", "music volume", 1.f) : 1.f;

	api->SetStartFrameLevel(this,Core_DefaultExecuteLevel + 20);
	api->SetEndFrameLevel(this,Core_DefaultExecuteLevel + 0x900);

	// первичные установки:
	GetProfileSettings();
	SetDataFromProfile();

	// для начала счиатем что сервис готов (если нет, то на первом кадре все поменяется)
	m_bServiceIsReady = true;
	api->Storage().SetString("Runtime.System.ProfileState","ready");
	m_oldReadyState = ILiveManager::rs_ready;
}

bool LiveService::Init()
{
	InitService();
	return true;
}

void _cdecl LiveService::Console_LiveInfo(const ConsoleStack &params)
{
	// help
	if( params.GetSize() < 1 )
	{
		m_pConsole->Trace(COL_CMD_OUTPUT,"Command xlive, use parameters:");
		m_pConsole->Trace(COL_CMD_OUTPUT,"  ac - switch on/off achievement info;");
		m_pConsole->Trace(COL_CMD_OUTPUT,"  mu - view multiplayer settings;");
		m_pConsole->Trace(COL_CMD_OUTPUT,"  st - view xlive service states;");
		m_pConsole->Trace(COL_CMD_OUTPUT,"  pr - view profile settings;");
	}

	// achiev
	if( string::IsEqual(params.GetParam(0),"ac") )
	{
		m_bShowDebugInfo = !m_bShowDebugInfo;
		m_pConsole->Trace( COL_CMD_OUTPUT,"XLive achievement info turn %s", m_bShowDebugInfo?"on":"off" );
	}
	else if( string::IsEqual(params.GetParam(0),"mu") )
	{
		m_pConsole->Trace( COL_CMD_OUTPUT,"Multiplayer settings:" );
		m_pConsole->Trace( COL_CMD_OUTPUT,"  check mask: %d", m_pLiveManager->GetCheckedUsers() );
		IPlayerChecker* pPlayerChecker = (IPlayerChecker*)api->GetService("PlayerChecker");
		if( pPlayerChecker )
		{
			for( long n=0; n<MAX_PLAYERS_COUNT; n++ )
			{
				m_pConsole->Trace( COL_CMD_OUTPUT,"  user %d (%s): %s%s%s",
					n, pPlayerChecker->GetPlayerName(n),
					pPlayerChecker->IsUsed(n)?"used, ":"not used, ",
					pPlayerChecker->IsPluged(n)?"pluged, ":"unpluged, ",
					pPlayerChecker->IsUpdated(n)?"updated":"not updated" );
			}
		}
		else
			m_pConsole->Trace( COL_CMD_OUTPUT,"  not finded service <PlayerChecker>" );
		m_pConsole->Trace( COL_CMD_OUTPUT,"MultiplayerTriggers:" );
		if( !m_pCurrentMission )
		{
			m_pConsole->Trace( COL_CMD_OUTPUT,"Can`t get info because current mission ptr = NULL" );
		}
		else
		{
			array<Object*> pTriggerList(_FL_);
			api->FindObject("MultiplayerTrigger",pTriggerList);
			m_pConsole->Trace( COL_CMD_OUTPUT,"Mission %s, multiplayer trigger objects finded %d:",
				m_pCurrentMission->GetMissionName(), pTriggerList.Size() );
			for( long n=0; n<pTriggerList; n++ )
			{
				IMultiplayerTrigger* pMPTrigger = (IMultiplayerTrigger*)pTriggerList[n];
//				if( &pMPTrigger->Mission() == m_pCurrentMission )
				m_pConsole->Trace( COL_CMD_OUTPUT,"  MultiplayerTrigger (%s)%s: %s in %s mode",
					pMPTrigger->Mission().GetMissionName(), pMPTrigger->GetObjectID().c_str(),
					pMPTrigger->IsActive()?"active":"inactive", pMPTrigger->LoadIsOn()?"load":"save" );
			}
		}
	}
	else if( string::IsEqual(params.GetParam(0),"st") )
	{
		m_pConsole->Trace( COL_CMD_OUTPUT,"XLive service states:" );
		m_pConsole->Trace( COL_CMD_OUTPUT,"  work: %s%s%s%s",
			m_bDemoMode?"demo, ":" ",
			m_bLoadAchievement?"load achiev, ":" ",
			m_bWriteAchievement?"write achiev, ":" ",
			m_bIsSaveData?"saving, ":" " );
		m_pConsole->Trace( COL_CMD_OUTPUT,"  context: %s, %d", api->Storage().GetString("Runtime.System.Context"), m_dwCurContext );
		m_pConsole->Trace( COL_CMD_OUTPUT,"  ready: %s, %d(old=%d)",
			api->Storage().GetString("Runtime.System.ProfileState"),
			m_pLiveManager->GetReadyState(), m_oldReadyState );
	}
	else if( string::IsEqual(params.GetParam(0),"pr") )
	{
		m_pConsole->Trace( COL_CMD_OUTPUT, "XLive profile settings:" );
		m_pConsole->Trace( COL_CMD_OUTPUT, "  no code now - sorry please :)" );
	}
}

void LiveService::StartFrame(float dltTime)
{
	if( !m_pLiveManager ) return;

	// проверяем состояние демки (если переменнная демки установлена, то включаем режим демки
	if( !m_bDemoMode && api->Storage().GetLong("Runtime.System.Demo",0)==1 )
		m_bDemoMode = true;

	// следим за записью ачивментов
	if( m_bWriteAchievement )
	{
		// запись закончилась
		if( !m_pLiveManager->IsWriteProcessAchievemets() )
			m_bWriteAchievement = false;
	}
	else
	{
		// есть ли ачивменты для записи
		if( m_aAddAchievements.Size()>0 )
			WriteAchievements();
	}

	// следим за записью сейва
	if( m_bIsSaveData )
	{
		// запись закончилась
		if( !m_pLiveManager->IsSaveDataProcess() )
			m_bIsSaveData = false;
	}
	else
	{
		// нет записи, но есть данные готовые для записи
		if( m_dwHaveWaitingData != 0 )
		{
			bool bSaveSuccess = true;
			// нет профайловых данных - надо взять их из предыдущего сейва
			if( (m_dwHaveWaitingData & 1) == 0 )
			{
				void * pData = 0;
				dword size = 0;
				if( m_pLiveManager->LoadData( pData, size ) )
				{
					m_aProfileData.DelAll();
					if( !GetProfileAndOption((byte*)pData,size, m_aProfileData, NULL) )
						bSaveSuccess = false;
				}
				else
					bSaveSuccess = false;
			}
			// запись
			if( bSaveSuccess )
				SaveProfileAndOption(m_aProfileData, m_aOptionData);
			// больше не нужны данные ожидающие записи
			m_aProfileData.DelAll();
			m_aOptionData.DelAll();
			m_dwHaveWaitingData = 0;
		}
	}

	// отработка внутри сервиса
	m_pLiveManager->Frame();

	// проверка сервиса на готовность
	ILiveManager::ReadyState rst = m_pLiveManager->GetReadyState();
	// в демо режиме мы всегда готовы
	if( m_bDemoMode )
		rst = ILiveManager::rs_ready;
	// сервис не готов (произошла бяка), предупредим об этом остальной движок - т.к. нужна пауза и т.д.
	if( rst != m_oldReadyState )
	{
		switch (rst)
		{
		// пользователь который был сигнед теперь отключился
		case ILiveManager::rs_not_signed:
			api->Storage().SetString("Runtime.System.ProfileState","notsigned");
			break;

		// пропало устройство на которое надо будет делать сейв (например вытащили MU)
		case ILiveManager::rs_no_storage:
			api->Storage().SetString("Runtime.System.ProfileState","nostorage");
			break;

		// отключили котроллер у пользователя который был засигнен к Live
		case ILiveManager::rs_unpluged:
			api->Storage().SetString("Runtime.System.ProfileState","lostcontroller");
			break;

		// пользователь сменился, а сейв остался от старого пользователя (в этом случае мы
		// не можем писать ачивменты, т.к. они основываются на достижениях другого игрока - требование TCR)
		case ILiveManager::rs_stranger_savedata:
			api->Storage().SetString("Runtime.System.ProfileState","stranger");
			break;

		default:
			api->Storage().SetString("Runtime.System.ProfileState","ready");
		}

		m_oldReadyState = rst;
	}
	if( rst == ILiveManager::rs_ready && !m_bServiceIsReady )
		api->Storage().SetString("Runtime.System.ProfileState","ready");
	m_bServiceIsReady = (rst == ILiveManager::rs_ready);

	// определяем текущий игровой контекст
	const char* pcCurContext = api->Storage().GetString("Runtime.System.Context");
	dword dwCurContext = GetContextDef(pcCurContext);
	// изменился контекст - меняем его в XBox
	if( dwCurContext != m_dwCurContext )
	{
		m_dwCurContext = dwCurContext;
		m_pLiveManager->ChangeContext(dwCurContext);
	}
}

void LiveService::EndFrame(float dltTime)
{
	DrawRichedAchievement( api->GetDeltaTime() );

	// для дебага (поддержка консоли)
	if( !m_bInitConsole )
	{
		m_bInitConsole = true;
		m_pConsole = (IConsole *)api->GetService("Console");
		if( m_pConsole )
		{
			m_pConsole->RegisterCommand("xlive","Show xlive service state (use parameters)", this,(CONSOLE_COMMAND)&LiveService::Console_LiveInfo);
		}
	}

	if( m_bShowDebugInfo )
		DrawDebugAchievement();
}

GP::LiveServiceState LiveService::GetServiceState()
{
	// возвращаем самое хреновое состояние (ничего не готово), если манагера нет
	if( !m_pLiveManager )
		return 0;

	ILiveManager::ReadyState rst = m_pLiveManager->GetReadyState();
	if( m_bDemoMode )
		rst = ILiveManager::rs_ready;

	GP::LiveServiceState st =
		// состояние готовности
		((rst == ILiveManager::rs_ready) ? GP::LSSM_Ready : 0) |
		// незасигненный пользователь или пользователь который не является хозяином текущей игры
		//((rst == ILiveManager::rs_not_signed || rst == ILiveManager::rs_stranger_savedata) ? 0 : GP::LSSM_SignIn) |
		(rst == ILiveManager::rs_stranger_savedata ? 0 : GP::LSSM_SignIn) |
		// отключение контроллера у пользователя
		((rst == ILiveManager::rs_unpluged) ? 0 : GP::LSSM_ControllerPluged);
	return st;
}

// включить слежение за определенным количеством пользователей
void LiveService::SetCheckedUsersQuantity(int n)
{
	if( m_pLiveManager )
		m_pLiveManager->SetCheckedUsers(n);
}

bool LiveService::Achievements_IsReady()
{
	return m_bServiceIsReady && !m_bLoadAchievement && !m_bWriteAchievement;
}

bool LiveService::Achievement_IsReached(dword dwPlayerIndex, dword dwAchievementID)
{
	if( !m_pLiveManager ) return false;
	bool bEarned = false;
	if( !m_pLiveManager->GetAchievementEarnedState(dwPlayerIndex, dwAchievementID, bEarned) )
		return false;
	return bEarned;
}

void LiveService::Achievement_Reach(dword dwPlayerIndex, dword dwAchievementID, dword dwAchievFlags)
{
	// проверяем на нахождение этой ачивки в списке готовых к записи
	for( long n=0; n<m_aAddAchievements; n++ )
		if( m_aAddAchievements[n].achievementID == dwAchievementID &&
			m_aAddAchievements[n].userIndex == dwPlayerIndex )
			break;
	if( n<m_aAddAchievements )
		return;

	// добавляем в список новую ачивку для записи
	m_aAddAchievements.Add();
	m_aAddAchievements.LastE().userIndex = dwPlayerIndex;
	m_aAddAchievements.LastE().achievementID = dwAchievementID;
	m_aAddAchievements.LastE().achievFlags = dwAchievFlags;

	// собственно запись если она уже не идет
	if( m_pLiveManager && !m_bWriteAchievement )
		WriteAchievements();
}


void LiveService::Achievement_Earn(dword liveID)
{
	Achievement_Reach(0, liveID, 0);
}

void LiveService::Achievement_EarnMP(dword player, dword liveID)
{
	Achievement_Reach(player, liveID, ILiveManager::af_multiplayer);
}

void LiveService::Achievement_Increment(dword id, int count)
{
	if( id >= m_aAchievements.Size() ) return;
	if( m_aAchievements[id].flags & adf_riched ) return;

	if( m_aAchievements[id].countCur[0] < m_aAchievements[id].countMax )
	{
		m_aAchievements[id].countCur[0] += count;
		if( m_aAchievements[id].countCur[0] >= m_aAchievements[id].countMax )
		{
			m_aAchievements[id].flags |= adf_riched;
			Achievement_Reach(0, m_aAchievements[id].liveID, 0);
		}
	}
}

void LiveService::Achievement_SetZero(dword id)
{
	if( id >= m_aAchievements.Size() ) return;
	if( m_aAchievements[id].flags & adf_riched ) return;
	m_aAchievements[id].countCur[0] = 0;
}

void LiveService::Achievement_IncrementMP(dword player, dword id, int count)
{
	if( id >= m_aAchievements.Size() || player >= MAX_PLAYERS_COUNT ) return;
	if( m_aAchievements[id].flags & adf_multiplay )
	{
		if( m_aAchievements[id].countCur[player] < m_aAchievements[id].countMax )
		{
			m_aAchievements[id].countCur[player] += count;
			if( m_aAchievements[id].countCur[player] >= m_aAchievements[id].countMax )
			{
				Achievement_Reach(player, m_aAchievements[id].liveID, ILiveManager::af_multiplayer);
			}
		}
	}
}

void LiveService::Achievement_SetZeroMP(dword player, dword id)
{
	if( m_aAchievements.Size() >= id || player >= MAX_PLAYERS_COUNT ) return;
	if( m_aAchievements[id].flags & adf_multiplay )
	{
		m_aAchievements[id].countCur[player] = 0;
	}
}

bool LiveService::Data_IsLoadEnable()
{
	if( !m_pLiveManager ) return false;
	return m_pLiveManager->LoadDataIsEnable();
}

bool LiveService::Data_Save()
{
	if( !m_pLiveManager ) return false;
	// если данные уже записываются, то сделаем подготовку текущих данных к записи
	if( m_pLiveManager->IsSaveDataProcess() )// return false;
		return PrepairWaitingSaveData(true,true);

	// обновить базу данных с данными для ачивментов
	AchievementsUpdateWrite();

	array<byte> aProfile(_FL_);
	array<byte> aOption(_FL_);
	api->Storage().Save("Profile", aProfile);
	api->Storage().Save("Options", aOption);

	return SaveProfileAndOption(aProfile, aOption);
}

bool LiveService::Data_Load()
{
	if( !m_pLiveManager ) return false;

	void * pData = 0;
	dword size = 0;

	// удалим старые опции
	api->Storage().Delete("Options");

	bool bUpdateAchievementCounters = !m_pLiveManager->LoadDataIsEnable();

	// если сменился профиль, то сначала перечитаем параметры из нового профиля
	if( m_pLiveManager->IsChangedProfile() )
	{
		// для измененного профайла меняем счетчики ачивментов
		bUpdateAchievementCounters = true;
		// профайл неготов
		m_bProfileSettingsIsReady = false;
		GetProfileSettings();
	}
	SetDataFromProfile();
	if( bUpdateAchievementCounters )
		AchievementsUpdateReset();

	// зачитаем общие данные
	if( !m_pLiveManager->LoadData( pData, size ) ) return false;

	// разделим данные на профайловые и опционные
	array<byte> aProfile(_FL_);
	array<byte> aOption(_FL_);
	if( !GetProfileAndOption((byte*)pData,size, aProfile, &aOption) ) return false;

	// запись профайловых данных в базу
	dword pointer = 0;
	if( aProfile.Size()>0 )
	{
		// удалим старые данные
		api->Storage().Delete("Profile");
		if( !api->Storage().Load("Profile", aProfile.GetBuffer(), aProfile.Size(), pointer) )
		{
			api->Trace("Error! LiveService::Data_Load() : Data base load error! Damaged save data.");
			return false;
		}
	}

	// запись опционных данных в базу
	pointer = 0;
	if( aOption.Size()>0 && !api->Storage().Load("Options", aOption.GetBuffer(), aOption.Size(), pointer) )
	{
		api->Trace("Error! LiveService::Data_Load() : Data base load error! Damaged save data.");
		return false;
	}

	// обновить таблицу ачивментов из базы
	if( bUpdateAchievementCounters )
		AchievementsUpdateRead();

	// состояние сейва
	api->Storage().SetString("Runtime.System.LoadState", aProfile.Size() == 0 ? "noprofile" : "normal");

	return true;
}

bool LiveService::Option_Save()
{
	if( !m_pLiveManager ) return false;
	if( m_pLiveManager->IsSaveDataProcess() )// return false;
		return PrepairWaitingSaveData(false,true);

	array<byte> data(_FL_);
	array<byte> option(_FL_);
	api->Storage().Save("Options", option);

	// есть предыдущие данные?
	//if( m_pLiveManager->LoadDataIsEnable() )
	{
		void * pData = 0;
		dword size = 0;
		//if( !m_pLiveManager->LoadData( pData, size ) ) return false;
		if( m_pLiveManager->LoadData( pData, size ) )
		{
			if( !GetProfileAndOption((byte*)pData,size, data, NULL) ) return false;
		}
	}

	return SaveProfileAndOption( data, option );
}

bool LiveService::Data_SaveLoadIsProcessing()
{
	return m_bIsSaveData;
}

bool LiveService::Data_SaveLoadIsSuccessful()
{
	if( !m_pLiveManager ) return false;
	return m_pLiveManager->LoadDataIsEnable();
}

const GP::SettingsData& LiveService::GetProfileSettings ()
{
	if( !m_bProfileSettingsIsReady )
	{
		// читаем установки из профайла
		m_ProfileSettings.voiceMuted = false;
		m_ProfileSettings.voiceVolume = 1.f;
		m_ProfileSettings.controlSensitivity = 1.f;
		m_ProfileSettings.gameDifficulty = 1;
		m_ProfileSettings.gameAutoaim = true;
		m_ProfileSettings.gameYAxisInversion = false;
		m_ProfileSettings.vibration = false;
		//crt_strcpy(m_ProfileSettings.idLang, sizeof(m_ProfileSettings.idLang), ILocStrings_LocalizationDefaultValue);
		if( m_pLiveManager )
			m_pLiveManager->GetProfileSettings(m_ProfileSettings);
/*			{
				// считаем данные из ини файла
				// строка ИД дефолтного языка локализации
				const char* pcDefLangID = pIni->GetString("", "localization", ILocStrings_LocalizationDefaultValue);
				crt_strcpy(m_ProfileSettings.idLang, sizeof(m_ProfileSettings.idLang), pcDefLangID);
			}*/
		m_bProfileSettingsIsReady = true;
	}
	return m_ProfileSettings;
}

// состояние готовности сервиса работы с контентами
bool LiveService::Content_IsReadyService()
{
	return false;
}

// количество доступных контентов
long LiveService::Content_GetQuantity()
{
	return 0;
}

// получить описание контента
bool LiveService::Content_GetDescribe(long nContent,GP::ContentDescribe& descr)
{
	return false;
}

// подключить контент к игре
bool LiveService::Content_Plugin(long nContent)
{
	return false;
}

// отключить контент из игры
bool LiveService::Content_Unplug(long nContent)
{
	return false;
}

// записать в БД движка установки из профайла пользователя
void LiveService::SetDataFromProfile()
{
	// устанавливаем язык по умолчанию
	api->Storage().SetString( ILocStrings_StorageLocalizationPath, m_ProfileSettings.idLang );

	api->Storage().SetFloat( "Options.ShoterSensitivity", m_ProfileSettings.controlSensitivity );
	api->Storage().SetFloat( "Options.ShoterInverse", m_ProfileSettings.gameYAxisInversion ? 1.f : 0.f );
	api->Storage().SetFloat( "Options.ShoterAIM", m_ProfileSettings.gameAutoaim ? 1.f : 0.f );
	api->Storage().SetFloat( "Options.CharacterSensitivity", m_ProfileSettings.controlSensitivity );
	api->Storage().SetFloat( "Options.GlobalVolume", m_ProfileSettings.globalVolume );
	api->Storage().SetFloat( "Options.FxVolume", m_ProfileSettings.fxVolume );
	api->Storage().SetFloat( "Options.MusicVolume", m_ProfileSettings.musicVolume );

	api->Storage().SetString( "Profile.Global.Difficulty", m_ProfileSettings.gameDifficulty==0 ? "Easy" : m_ProfileSettings.gameDifficulty==1 ? "Normal" : "Hard" );
	api->Storage().SetFloat( "Profile.Global.Demo", m_bDemoMode );

	api->Storage().SetFloat( "Options.Controls.vibrate", m_ProfileSettings.vibration ? 1.f : 0 );
	api->Storage().SetFloat( "Options.Controls.inv_mouse", m_ProfileSettings.gameYAxisInversion ? 1.f : 0.f );
	api->Storage().SetFloat( "Options.Controls.inv_joy_Y", m_ProfileSettings.gameYAxisInversion ? 1.f : 0.f );
	api->Storage().SetFloat( "Options.Controls.sen_joy", m_ProfileSettings.controlSensitivity );
}

__forceinline void LiveService::SetAchievementData(AchievementDescribe& ach, dword liveID, int maxCount, dword flags)
{
	ach.countMax = maxCount;
	ach.liveID = liveID;
	ach.flags = flags;
	for(dword n=0; n<MAX_PLAYERS_COUNT; n++)
		ach.countCur[n] = 0;
}

static void DrawSprite(float x, float y, float sx, float sy,dword color,IRender* renderer)
{
	RS_SPRITE spr[4];

	spr[0].vPos = Vector (  x ,  y, 0.0f);
	spr[1].vPos = Vector (x+sx,  y, 0.0f);
	spr[2].vPos = Vector (x+sx, y-sy , 0.0f);
	spr[3].vPos = Vector (  x , y-sy , 0.0f);

	spr[0].tv = 0.0f;
	spr[0].tu = 0.0f;

	spr[1].tv = 0.0f;
	spr[1].tu = 1.0f;

	spr[2].tv = 1.0f;
	spr[2].tu = 1.0f;

	spr[3].tv = 1.0f;
	spr[3].tu = 0.0f;

	for (int j=0;j<4;j++)
	{
		spr[j].dwColor = color;
	}

	renderer->DrawSprites(NULL,spr, 1);
}

void LiveService::DrawRichedAchievement(float fDeltaTime)
{
	// есть что показывать?
	if ( m_aRichAchievement.Size()>0 && m_aRichAchievement[0].ready )
	{
		// время идет
		m_fRichOutTime -= fDeltaTime;

		// показ закончен?
		if ( m_fRichOutTime<0.0f )
		{
			// выкидываем показанный ачивмент из списка
			m_aRichAchievement.DelIndex(0);

			// если есть еще что показать, то даем еще время на показ
			if (m_aRichAchievement.Size()>0)
				m_fRichOutTime = 4.0f;
		}

		// если есть время для показа, то показываем
		if ( m_aRichAchievement.Size()>0 && m_aRichAchievement[0].ready )
		{
			// размеры окна для вывода инфы об ачивменте
			float wgt = 200.0f;
			float hgt = 30;

			// цвет для показа
			Color cl;
			// в начале времени проявляем показ (альфа растет)
			if (m_fRichOutTime>3.5f)
			{
				cl.a = 1.0f - (m_fRichOutTime-3.5f)/0.5f;
				// и растет размер окна для показа
				wgt *= cl.a;
				hgt *= cl.a;
			}
			else
			// в конце времени гасим показ (альфа убывает)
			if (m_fRichOutTime<0.5f)
			{
				cl.a = m_fRichOutTime/0.5f;
			}
			else
			// в остальное время показ с полной силой
			{
				cl.a = 1.0f;
			}
			// цвет подложки будет черным
			cl.r = 0.0f;
			cl.g = 0.0f;
			cl.b = 0.0f;

			// позиция на экране для вывода инфы о полученном ачивменте
			RENDERVIEWPORT vp = GetRender()->GetViewport();

			float x = vp.Width * 0.5f - wgt * 0.5f;
			float y = vp.Height - 35.0f - hgt * 0.5f;

			float sx = wgt / vp.Width;
			float sy = hgt / vp.Height;

			// рисуем подложку
			DrawSprite( ToSignRange(x/vp.Width), ToSignRange(1 - y/vp.Height),
					    sx * 2.0f, sy * 2.0f,
						cl, GetRender() );

			// выводим инфу про ачивмент
			if (m_fRichOutTime<3.0f)
			{
				cl.g = 1.0f;
				GetRender()->Print(x+10,y+5.0f,cl,m_aRichAchievement[0].str.c_str());
			}
		}
	}

	// если данные для вывода ачивмента не готовы, то ждем пока они подготовятся
	if( m_aRichAchievement.Size()>0 && !m_aRichAchievement[0].ready )
	{
		m_aRichAchievement[0].ready = m_pLiveManager->GetAchievementString (m_aRichAchievement[0].achievID, m_aRichAchievement[0].str);
	}
}

void LiveService::DrawDebugAchievement()
{
	// цвет для показа
	Color cl(0.f,0.8f);

	// размеры вью порта
	RENDERVIEWPORT vp = GetRender()->GetViewport();


	// рисуем подложку
	float x = vp.Width * 0.05f;
	float y = vp.Height * 0.65f;
	float dx = vp.Width * 0.18f;
	float dy = vp.Height * 0.025f;
	DrawSprite( -0.9f, -0.3f, 1.8f, 0.5f, cl, GetRender() );

	// рисуем инфу
	string str;
	for( long n=0; n<m_aAchievements; n++ )
	{
		float cx = x + (n%5)*dx;
		float cy = y + (n/5)*dy;
		m_pLiveManager->GetAchievementString (m_aAchievements[n].liveID, str);
		//char achname[64];
		//crt_snprintf( achname, sizeof(achname), "ach%d", n );
		GetRender()->Print(cx+2,cy, (m_aAchievements[n].flags & adf_riched)?0xFF00FF00:0xFFFF0000,
			"%s(%d): %d(%d)",
			str.c_str(), m_aAchievements[n].liveID, m_aAchievements[n].countCur[0],m_aAchievements[n].countMax );
	}
}

IRender* LiveService::GetRender()
{
	if( m_pRenderService ) return m_pRenderService;
	m_pRenderService = (IRender*)api->GetService("DX9Render");
	return m_pRenderService;
}

void LiveService::AchievementsUpdateRead()
{
	ICoreStorageFolder* pFolder = api->Storage().GetItemFolder( "Profile.Achiev", _FL_ );
	if( !pFolder ) return;

	dword q = pFolder->GetItemsCount();
	for( dword n=0; n<q; n++ )
	{
		const char* achname = pFolder->GetNameByIndex(n);
		if( !achname ) continue;
		ICoreStorageString* achval = pFolder->GetItemString (achname, _FL_);
		if( !achval ) continue;
		const char* strval = achval->Get();
		if( !strval ) continue;
		long nID = -1;
		long nCount = -1;
		achval->Release();
		crt_sscanf( strval, "%d,%d", &nID, &nCount );
		if( nID<0 || nCount<0 || nID>=m_aAchievements )
		{
			api->Trace("Achievement data into database is corrupted %s,id=%d,count=%d", achname, nID, nCount);
			continue;
		}
		m_aAchievements[nID].countCur[0] = nCount;
	}
	pFolder->Release();
}

void LiveService::AchievementsUpdateWrite()
{
	api->Storage().Delete("Profile.Achiev");
	ICoreStorageFolder* pFolder = api->Storage().GetItemFolder( "Profile.Achiev", _FL_ );
	if( !pFolder ) return;

	// очистим старые значения
//	dword q = pFolder->GetItemsCount();
//	for( dword n=0; n<q; n++ )
//		pFolder->Delete( pFolder->GetNameByIndex(n) );

	// записываем новые значения
	for( dword n=0; n<m_aAchievements.Size(); n++ )
	{
		char achname[64];
		char achval[255];
		crt_snprintf( achname, sizeof(achname), "ach%d", n );
		crt_snprintf( achval, sizeof(achval), "%d,%d", n, m_aAchievements[n].countCur[0] );
		pFolder->SetString( achname, achval );
	}

	pFolder->Release();	
}

void LiveService::AchievementsUpdateReset()
{
	for( dword n=0; n<m_aAchievements.Size(); n++ )
	{
		m_aAchievements[n].countCur[0] = 0;
		// сбросим все флаги, кроме состояния мультиплеерного ачивмента
		m_aAchievements[n].flags &= adf_multiplay;
	}
}

void LiveService::WriteAchievements()
{
	if( m_pLiveManager->WriteAchievement( m_aAddAchievements.GetBuffer(), m_aAddAchievements.Size() ) )
	{
		// добавляем полученные ачивменты в список достигнутых ачивок для вывода
/*		for(dword n=0; n<m_aAddAchievements.Size(); n++)
		{
			long i = m_aRichAchievement.Add();
			m_aRichAchievement[i].ready = m_pLiveManager->GetAchievementString (m_aAddAchievements[n].achievementID, m_aRichAchievement[i].str);
			m_aRichAchievement[i].achievID = m_aAddAchievements[n].achievementID;
		}
		if( m_fRichOutTime <= 0.f )
			m_fRichOutTime = 4.f;*/

		m_aAddAchievements.DelAll();
		m_bWriteAchievement = m_pLiveManager->IsWriteProcessAchievemets();
	}
}

bool LiveService::SaveProfileAndOption(array<byte> & aProfile, array<byte> & aOption)
{
	Compressor gzip;
	dword dwSize;
	// данные для записи
	array<byte> data(_FL_);

	// жмем данные для профайла
	dwSize = aProfile.Size();
	if( dwSize > 0 )
	{
		gzip.Compress( aProfile.GetBuffer(), dwSize, Compressor::cm_nousememory );
		// добавляем данные с профайла в общий массив
		data.AddElements( gzip.Size() + 6 );
		// размер упакованных данных
		*(unsigned short*)&data[0] = gzip.Size();
		// размер неупакованных данных
		*(dword*)&data[2] = dwSize;
		// упакованные данные
		memcpy(&data[6], gzip.Buffer(), gzip.Size() );
	}
	else
	{
		data.AddElements( 6 );
		ZeroMemory(data.GetBuffer(), 6);
	}

	// жмем данные для опций
	dword dwStart = data.Size();
	dwSize = aOption.Size();
	if( dwSize > 0 )
	{
		gzip.Free();
		gzip.Compress( aOption.GetBuffer(), dwSize, Compressor::cm_nousememory );
		// добавляем данные с опций в общий массив
		data.AddElements( gzip.Size() + 6 );
		// размер упакованных данных
		*(unsigned short*)&data[dwStart] = gzip.Size();
		// размер неупакованных данных
		*(dword*)&data[dwStart+2] = dwSize;
		// упакованные данные
		memcpy(&data[dwStart+6], gzip.Buffer(), gzip.Size() );
	}
	else
	{
		data.AddElements( 6 );
		ZeroMemory(&data[dwStart], 6);
	}

	// собственно запись в сейв
	if( !m_pLiveManager->SaveData( data.GetBuffer(), data.Size() ) ) return false;
	m_bIsSaveData = m_pLiveManager->IsSaveDataProcess();
	return true;
}

bool LiveService::GetProfileAndOption(byte* pData, dword dwDataSize, array<byte> & aProfile, array<byte> * paOption)
{
	if( !pData || dwDataSize<12 ) return false;

	// выделяем часть данных для профайловых данных
	dword dwProfileSize = *(unsigned short*)pData;
	if( dwProfileSize + 12 > dwDataSize )
	{
		api->Trace("Error! LiveService::GetProfileAndOption() : Invalid save data (data size=%d, profile size=%d)",dwDataSize,dwProfileSize);
		return false;
	}

	// выделяем часть данных для опций
	dword dwOptionSize = *(unsigned short*)&pData[dwProfileSize+6];
	if( dwOptionSize + dwProfileSize + 12 != dwDataSize )
	{
		api->Trace("Error! LiveService::GetProfileAndOption() : Invalid save data (data size=%d, profile size=%d, option size=%d)",dwDataSize,(dword)dwProfileSize,(dword)dwOptionSize);
		return false;
	}

	dword dwSize;
	Compressor gzip;

	// подготавливаем массив данных для профайла
	aProfile.DelAll();
	dwSize = *(dword*)&pData[2];
	if( dwSize > 1000000 )
	{
		api->Trace("Error! LiveService::GetProfileAndOption() : Incorrect save data - profile data size = %d", dwSize);
		return false;
	}
	if( dwSize > 0 )
	{
		aProfile.AddElements( dwSize );
		// разархивируем эту часть данных
		Compressor::DecReturn ret = gzip.Decompress( &pData[6], dwProfileSize, aProfile.GetBuffer(), dwSize );
		if( ret != Compressor::dr_ok )
		{
			api->Trace("Error! LiveService::GetProfileAndOption() : Decomress error - 0x%x", ret);
			return false;
		}
	}

	// заполняем, если нужно, массив данных для опций
	if( paOption )
	{
		paOption->DelAll();
		dwSize = *(dword*)&pData[dwProfileSize+8];
		if( dwSize > 1000000 )
		{
			api->Trace("Error! LiveService::GetProfileAndOption() : Incorrect save data - option data size = %d", dwSize);
			return false;
		}
		if( dwSize > 0 )
		{
			paOption->AddElements( dwSize );
			// разархивируем эту часть данных
			gzip.Free();
			Compressor::DecReturn ret = gzip.Decompress( &pData[dwProfileSize+12], dwOptionSize, paOption->GetBuffer(), dwSize );
			if( ret != Compressor::dr_ok )
			{
				api->Trace("Error! LiveService::GetProfileAndOption() : Decomress error - 0x%x", ret);
				return false;
			}
		}
	}

	return true;
}

bool LiveService::PrepairWaitingSaveData(bool bProfile,bool bOption)
{
	if( bProfile )
	{
		m_dwHaveWaitingData |= 1;
		m_aProfileData.DelAll();
		api->Storage().Save("Profile", m_aProfileData);
	}
	if( bOption )
	{
		m_dwHaveWaitingData |= 2;
		m_aOptionData.DelAll();
		api->Storage().Save("Options", m_aOptionData);
	}
	return true;
}
