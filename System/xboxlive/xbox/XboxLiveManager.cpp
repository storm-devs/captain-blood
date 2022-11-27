#ifdef _XBOX
#include "XBoxLiveManager.h"
#include "..\..\..\common_h\controls.h"
#include "..\..\..\common_h\InputSrvCmds.h"

XboxLiveManager::XboxLiveManager()
{
	m_pControls = null;
	m_dwSignedInUserIndex = SignedUser_NotSigned;
	m_xuidSaveGameUser = INVALID_XUID;
	m_bXShow = false;
	m_dwCheckableQuantity = 0;

	m_xuidProfileUser = INVALID_XUID;

	m_pSaveData = NULL;
	m_dwSaveDataSize = 0;
	m_SaveDataState = SaveData_None;

	ZeroMemory( &m_xovSaveLoad, sizeof(XOVERLAPPED) );
	ZeroMemory( &m_xovAchievement, sizeof(XOVERLAPPED) );

	m_bUserIsCreator = true;
	m_bSignChanged = false;
	m_bIsSetMasterUser = false;

	for(dword n=0; n<USER_QUANTITY; n++)
	{
		m_UserDescr[n].bControllerPluged = false;
		m_UserDescr[n].bUserSigned = false;
		m_UserDescr[n].bUserSignChanged = false;
		m_UserDescr[n].xuidUser = INVALID_XUID;
		m_UserDescr[n].bCheckable = false;

		m_UsrAchDetail[n].dwCount = 0;
		m_UsrAchDetail[n].pAch = NULL;
	}

	m_hNotification = NULL;

	m_dwAchievementForSaveQuantity = 0;
	m_bAchievementSaving = false;
}

XboxLiveManager::~XboxLiveManager()
{
	// остановим запись/чтение сейва
	StopSaveLoadData();
	// выгружаем сохраненные буферы чтения (записи)
	ReleaseCurrentSaveData();

	// остановим запись ачивментов
	if( m_bAchievementSaving )
	{
		// дождемся завершения записи ачивментов - ибо терять их нельзя
		DWORD dwRes;
		XGetOverlappedResult( &m_xovAchievement, &dwRes, TRUE );
		// закрываем уже ненужный хендл записи
		CloseHandle( m_xovAchievement.hEvent );
		m_xovAchievement.hEvent = NULL;
		m_bAchievementSaving = false;
	}

	for(dword n=0; n<USER_QUANTITY; n++)
	{
		m_UsrAchDetail[n].dwCount = 0;
		DELETE( m_UsrAchDetail[n].pAch );
	}

	XOnlineCleanup();
}

bool XboxLiveManager::Init()
{
	if( ERROR_SUCCESS != XOnlineStartup() )
	{
		SetError("Failed Init() Xbox Live Manager: failed XOnlineStartup()");
		return false;
	}

	m_pControls = (IControlsService*)api->GetService("ControlsService");

	// считаем что на момент запуска у нас не запущен интерфейс XBox
	m_bXShow = false;

	// состояние подключения пользователей и геймпадов
	for(dword n=0; n<USER_QUANTITY; n++)
	{
		XINPUT_STATE state;
		// геймпад подключен?
		m_UserDescr[n].bControllerPluged = XInputGetState(n,&state)==ERROR_SUCCESS;
		// пользователь залогинен?
		XUSER_SIGNIN_INFO uinf;
		if( ERROR_SUCCESS == XUserGetSigninInfo(n,XUSER_GET_SIGNIN_INFO_OFFLINE_XUID_ONLY,&uinf) )
		//XUID xuid;
		//if( ERROR_SUCCESS == XUserGetXUID(n,&xuid) )
		{
			m_UserDescr[n].bUserSigned = true;
			//m_UserDescr[n].xuidUser = xuid;
			m_UserDescr[n].xuidUser = uinf.xuid;
		}
		else
		{
			m_UserDescr[n].bUserSigned = false;
			m_UserDescr[n].xuidUser = INVALID_XUID;
		}
	}

	// Найдем хозяина.
	FindSignedUser(false);

	// с этого момента начинаем следить за системными эвентами:
	// надо не пропустить смену пользователя или устройства для хранения сейва
	m_hNotification = XNotifyCreateListener( XNOTIFY_SYSTEM );
    if( m_hNotification == NULL )
	{
		SetError("Failed Init() Xbox Live Manager: failed XNotifyCreateListener()");
        return false;
	}

	// одного пользователя всегда проверяем
	SetCheckedUsers(0);

	return true;
}

void XboxLiveManager::Frame()
{
	// смотрим что за евенты у нас случились за этот кадр
	NotificationFrame();

	// процесс работы с ачивментами
	if( m_bAchievementSaving )
	{
		//if( XGetOverlappedResult( &m_xovAchievement, NULL, FALSE ) != ERROR_IO_INCOMPLETE )
		if( XHasOverlappedIoCompleted( &m_xovAchievement ) )
		{
			CloseHandle( m_xovAchievement.hEvent );
			m_xovAchievement.hEvent = NULL;
			m_bAchievementSaving = false;
		}
	}

	// процесс записи в профайл
	if( m_SaveDataState == SaveData_Saving )
	{
		if( !m_xovSaveLoad.hEvent )
		{
			SetError("Save Data Error: Can`t write data into profile. Overlapped handler is NULL.");
			m_SaveDataState = SaveData_None;
		}
		else
			if( XHasOverlappedIoCompleted( &m_xovSaveLoad ) )
			{
				DWORD result;
				XGetOverlappedResult( &m_xovSaveLoad, &result, TRUE );
				if( result == ERROR_SUCCESS )
				{
					// запомним данные для последующих чтений
					m_dwSaveDataSize = *(DWORD*)m_savebuf;
					m_pSaveData = NEW byte[m_dwSaveDataSize];
					Assert(m_pSaveData);
					memcpy(m_pSaveData,&m_savebuf[4],m_dwSaveDataSize);
					m_SaveDataState = SaveData_Present;
				}
				else
				{
					result = XGetOverlappedExtendedError(&m_xovSaveLoad);
					char param[1024];
					crt_snprintf(param,sizeof(param), "Save Data Error: Can`t write data into profile. Error code = %x", result );
					SetError(param);
					m_SaveDataState = SaveData_None;
				}

				CloseHandle( m_xovSaveLoad.hEvent );
				m_xovSaveLoad.hEvent = NULL;
			}
	}

	// проверка на смену пользователя
	FindSignedUser(false);

	// если показывается интерфейс XBox то ничего больше не делаем
	if( m_bXShow )
		return;

	// ставим проверку пользователей
	//SetCheckedUsers(m_dwCheckableQuantity);
}

bool XboxLiveManager::WriteAchievement(UserAchievement* pAchievementsBuffer, dword dwAchievementsQuantity)
{
	// пока мы пишем мы не можем писать еще
	Assert( !m_bAchievementSaving );
	// лишние ачивменты писать не будем... и даже свалимся, чтоб не повадно было.
	Assert( dwAchievementsQuantity <= ACHIEVEMENT_COUNT );

	// заполняем массив для записи
	UserAchievement* pSrcAchievement = pAchievementsBuffer;
	m_dwAchievementForSaveQuantity = 0;
	for( dword i=0; i<dwAchievementsQuantity; i++ )
	{
		dword dwUserIndex = m_dwSignedInUserIndex;

		if( pSrcAchievement->userIndex>=USER_QUANTITY ) continue;
		// мультиплеерные ачивки не проверяем на хозяина
		if( pSrcAchievement->achievFlags & ILiveManager::af_multiplayer )
		{
			if( !m_pControls ) continue;

			// ремап пользователя через контрол сервис
			InputSrvQueryAssignIndex assignIdx( pSrcAchievement->userIndex );
			m_pControls->ExecuteCommand( assignIdx );
			if( assignIdx.assignIndex < 0 || assignIdx.assignIndex >= USER_QUANTITY )
				continue;
			dwUserIndex = assignIdx.assignIndex;
		}
		else if( pSrcAchievement->userIndex!=0 || m_bSignChanged || !m_bUserIsCreator )
			continue;

		m_AchievementsForSave[m_dwAchievementForSaveQuantity].dwUserIndex = dwUserIndex;
		m_AchievementsForSave[m_dwAchievementForSaveQuantity].dwAchievementId = pSrcAchievement->achievementID;
		pSrcAchievement++;
		m_dwAchievementForSaveQuantity++;
	}

	// если есть что писать, то и пишем
	if( m_dwAchievementForSaveQuantity > 0 )
	{
		// создаем евент для асинхронного вызова записи ачивментов
		ZeroMemory( &m_xovAchievement, sizeof(XOVERLAPPED) );
		m_xovAchievement.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
		if( m_xovAchievement.hEvent == NULL )
		{
			SetError("Couldn't create event.");
			m_dwAchievementForSaveQuantity = 0;
			return false;
		}

		// собственно запись (точнее ее запуск в асинхронном режиме, а завершится она на обработке фрейма)
		DWORD dwStatus = XUserWriteAchievements( m_dwAchievementForSaveQuantity, m_AchievementsForSave, &m_xovAchievement );
		if( dwStatus != ERROR_IO_PENDING )
		{
			SetError("Failed XUserWriteAchievements");
			CloseHandle( m_xovAchievement.hEvent );
			m_xovAchievement.hEvent = NULL;
		}
		m_bAchievementSaving = true;
	}
	return true;
}

bool XboxLiveManager::GetAchievementString(dword achievementID, string & str)
{
	const char* pAchName = "";
	switch(achievementID)
	{
		case ACHIEVEMENT_FIRST_BLOOD: pAchName="First Blood"; break;
		case ACHIEVEMENT_SLAYER: pAchName="Murderer"; break;
		case ACHIEVEMENT_EXECUTIONER: pAchName="Executor"; break;
		case ACHIEVEMENT_RIPPER: pAchName="Reaper"; break;
		case ACHIEVEMENT_SAILOR: pAchName="Seamen"; break;
		case ACHIEVEMENT_MUSKETEER: pAchName="Musketeer"; break;
		case ACHIEVEMENT_DUELIST: pAchName="Duelist"; break;
		case ACHIEVEMENT_BUTCHER: pAchName="Butcher"; break;
		case ACHIEVEMENT_SHARPSHOOTER: pAchName="Dead Shot"; break;
		case ACHIEVEMENT_KNIGHT: pAchName="Knight"; break;
		case ACHIEVEMENT_SNIPER: pAchName="Marksman"; break;
		case ACHIEVEMENT_DEMOLITION: pAchName="Destroyer"; break;
		case ACHIEVEMENT_SHREDDER: pAchName="Slayer"; break;
		case ACHIEVEMENT_HEADCUTTER: pAchName="Swashbuckler"; break;
		case ACHIEVEMENT_MURDERER: pAchName="Killa"; break;
		case ACHIEVEMENT_RAGE: pAchName="Rage"; break;
		case ACHIEVEMENT_WEAPON_MASTER: pAchName="Weaponmaster"; break;
		case ACHIEVEMENT_RACKETEER: pAchName="Miser"; break;
		case ACHIEVEMENT_MERCILESS: pAchName="Ruthless"; break;
		case ACHIEVEMENT_BERSERKER: pAchName="Berserker"; break;
		case ACHIEVEMENT_BOATSWAIN: pAchName="Bosun"; break;
		case ACHIEVEMENT_CAPTAIN: pAchName="Captain"; break;
		case ACHIEVEMENT_CAPER: pAchName="Caper"; break;
		case ACHIEVEMENT_JOLLY_ROGER: pAchName="Jolly Roger"; break;
		case ACHIEVEMENT_PIRATE_KING: pAchName="Pirate King"; break;
		case ACHIEVEMENT_AVENGER: pAchName="Avenger"; break;
		case ACHIEVEMENT_DEFENDER_OF_THE_WEAK: pAchName="Defender of the Weak"; break;
		case ACHIEVEMENT_MASTER_OF_REVENGE: pAchName="Master of Revenge"; break;
		case ACHIEVEMENT_PETER_THE_CONQUEROR: pAchName="Conqueror"; break;
		case ACHIEVEMENT_PETER_THE_VICTORIOUS: pAchName="Champion"; break;
		case ACHIEVEMENT_CAPTAIN_BLOOD: pAchName="Captain Blood"; break;
		case ACHIEVEMENT_FISH_FOOD: pAchName="Shark food"; break;
		case ACHIEVEMENT_TRICKSTER: pAchName="Trickster"; break;
		case ACHIEVEMENT_BURGLAR: pAchName="Door Cracker"; break;
		case ACHIEVEMENT_UNSTOPPABLE: pAchName="Juggernaut"; break;
		case ACHIEVEMENT_LOOTER: pAchName="Looter"; break;
		case ACHIEVEMENT_THE_MONEY_BAG: pAchName="Moneybags"; break;
		case ACHIEVEMENT_TREASURE_HUNTER: pAchName="Treasure Hunter"; break;
		case ACHIEVEMENT_MASTER: pAchName="Master"; break;
		case ACHIEVEMENT_GUNMASTER: pAchName="Gunmaster"; break;
		case ACHIEVEMENT_CATAPULT: pAchName="Catapult"; break;
		case ACHIEVEMENT_NOVICE: pAchName="Novice"; break;
		case ACHIEVEMENT_SURVIVOR: pAchName="Survivor"; break;
		case ACHIEVEMENT_VETERAN: pAchName="Veteran"; break;
		case ACHIEVEMENT_MANSLAUGHTER: pAchName="Manslaughter"; break;
		case ACHIEVEMENT_MASSACRE: pAchName="Meat Grinder"; break;
		case ACHIEVEMENT_SEA_WOLF: pAchName="Seawolf"; break;
		case ACHIEVEMENT_TERROR_OF_THE_SEA: pAchName="Terror of the Sea"; break;
		default:
			pAchName = "Unknown";
	}
	str.Format("%s",pAchName);
	return true;
}

bool XboxLiveManager::IsWriteProcessAchievemets()
{
	// не занят ли сервис чтением записью ачивментов? (выполнение асинхронных вызовов)
	return m_bAchievementSaving;
}

bool XboxLiveManager::GetAchievementEarnedState(dword userIndex, dword achievementID, bool & isEarned)
{
	// лишний пользователь
	if( userIndex >= USER_QUANTITY ) return false;
	for( dword n=0; n<m_UsrAchDetail[userIndex].dwCount; n++ )
	{
		if( m_UsrAchDetail[userIndex].pAch[n].dwId == achievementID )
			return m_UsrAchDetail[userIndex].pAch[n].dwFlags & XACHIEVEMENT_DETAILS_ACHIEVED;
	}
	return false;
}

//-------------------------------------------------------
// Чтение сейва
//-------------------------------------------------------
bool XboxLiveManager::LoadData( void* & pData, dword & size )
{
	// если данные уже есть, то их и возвращаем
	if( m_SaveDataState == SaveData_Present && m_bUserIsCreator && !m_bSignChanged )
	{
		GetCurrentSaveData(pData, size);
		return true;
	}

	// выкидываем старый сейв из памяти
	ReleaseCurrentSaveData();

	// обновляем пользователя
	FindSignedUser(true);
	// при чтении новых данных пользователь становиться создателем
	m_bUserIsCreator = true;
	// а флаг смены пользователя сбрасывается
	m_bSignChanged = false;

	// неподключенный пользователь ничего не может считать
	if( !UserSignedIsValid() )
	{
		SetError("Load Data Error: Hav`t signed user");
		return false;
	}

	const DWORD aSettingsIDs[3] = {XPROFILE_TITLE_SPECIFIC1, XPROFILE_TITLE_SPECIFIC2, XPROFILE_TITLE_SPECIFIC3};
	DWORD dwDataSize = 0;
	// узнаем размер буфера который нам нужен
	DWORD result = XUserReadProfileSettings (0, m_dwSignedInUserIndex, 3, aSettingsIDs, &dwDataSize, NULL, NULL);
	if( result != ERROR_SUCCESS && result != ERROR_INSUFFICIENT_BUFFER )
	{
		SetError("Load Data Error: Can`t get size of save data from profile");
		return false;
	}

	// выделяем буфер для получения данных
	if( dwDataSize==0 )
	{
		SetError("Load Data Error: Save data is null");
		return false;
	}
	BYTE* pBuf = NEW BYTE[dwDataSize];
	Assert(pBuf);

	// читаем данные в этот буфер
	result = XUserReadProfileSettings (0, m_dwSignedInUserIndex, 3, aSettingsIDs, &dwDataSize, (XUSER_READ_PROFILE_SETTING_RESULT*)pBuf, NULL);
	if( result != ERROR_SUCCESS )
	{
		delete pBuf;
		SetError("Load Data Error: Can`t read data from profile");
		return false;
	}

	// собираем блоки в один массив
	DWORD dst = 0;
	BYTE dstBuf[MAX_SAVE_DATA_SIZE];

	XUSER_READ_PROFILE_SETTING_RESULT* pBlock = (XUSER_READ_PROFILE_SETTING_RESULT*)pBuf;
	for( DWORD i=0; i<pBlock->dwSettingsLen && dst<MAX_SAVE_DATA_SIZE; i++ )
	{
		if( pBlock->pSettings[i].source != XSOURCE_NO_VALUE && pBlock->pSettings[i].source != XSOURCE_PERMISSION_DENIED &&
			pBlock->pSettings[i].data.type == XUSER_DATA_TYPE_BINARY )
		{
			DWORD srcSize = MIN( MAX_SAVE_DATA_SIZE-dst, pBlock->pSettings[i].data.binary.cbData );
			memcpy (&dstBuf[dst], pBlock->pSettings[i].data.binary.pbData, srcSize);
			dst += srcSize;
		}
	}

	DELETE(pBuf);
	// ничего не прочитали, то вернем пустой буфер и успешно завершимся... ибо вроде и не ошибка?!
	if( dst==0 )
	{
		pData = null;
		size = 0;
		return false;
	}

	// запомним данные для последующих чтений
	m_pSaveData = NEW byte[dst];
	Assert(m_pSaveData);
	m_dwSaveDataSize = *(DWORD*)dstBuf;
	if(m_dwSaveDataSize > dst-4)
		return false;
	memcpy(m_pSaveData,&dstBuf[4],m_dwSaveDataSize);

	m_SaveDataState = SaveData_Present;
	m_xuidSaveGameUser = m_UserDescr[m_dwSignedInUserIndex].xuidUser;

	GetCurrentSaveData(pData, size);

    return true;
}

//-------------------------------------------------------
// Запись сейва
//-------------------------------------------------------
bool XboxLiveManager::SaveData( void* pData, dword size )
{
	// неподключенный пользователь ничего не может сохранить
	if( !UserSignedIsValid() )
	{
		SetError("Save Data Error: Hav`t signed user.");
		return false;
	}
	// пользователь сменился - ничего писать не будем
	// чужой сейв нельзя сохранить как свой
	if( m_bSignChanged || !m_bUserIsCreator )
	{
		SetError("Save Data Error: Current save game owner is not signed player.");
		return false;
	}
	// пустые данные не записываем
	if( pData==NULL || size==0 )
	{
		SetError("Save Data Error: Can`t save empty data.");
		return false;
	}
	// ограничение максимального размера сейва
	if( size>(MAX_SAVE_DATA_SIZE-4) )
	{
		SetError("Save Data Error: Save data size overflow 3 KB limit.");
		return false;
	}

	// выкидываем старый сейв из памяти
	ReleaseCurrentSaveData();

	// размер данных для сейва должен быть ровно 3 чанка по максимому
	*(DWORD*)m_savebuf = size;
	memcpy(&m_savebuf[4],pData,size);

	// разбиваем данные на блоки (по 1000 байт 3 блока - столько позволено переменных в профайле, на каждый тайтл)
	static const dword nBlocksQ = 3; // не оставляем не используемых сеттингов, что бы они не путали нас при чтении
	BYTE* pSrc = m_savebuf;//(BYTE*)pData;
	ZeroMemory(m_aBlocksData,sizeof(XUSER_PROFILE_SETTING)*nBlocksQ);
	dword szLeft = MAX_SAVE_DATA_SIZE;//*3;//size;
	for( dword n=0; n<nBlocksQ; n++ )
	{
		m_aBlocksData[n].source = XSOURCE_TITLE;
		m_aBlocksData[n].user.dwUserIndex = m_dwSignedInUserIndex;
		m_aBlocksData[n].dwSettingId = n==0 ? XPROFILE_TITLE_SPECIFIC1 :
									n==1 ? XPROFILE_TITLE_SPECIFIC2 :
									XPROFILE_TITLE_SPECIFIC3;
		// размер данных
		dword dwCurSize = MIN( szLeft, XPROFILE_SETTING_MAX_SIZE );
		// если нет уже данных, то пишем что этот параметр нулевой (не будет использоваться при считывании)
		if( dwCurSize==0 )
			m_aBlocksData[n].data.type = XUSER_DATA_TYPE_NULL;
		// если данные есть то ставим указатель на них
		else
		{
			m_aBlocksData[n].data.type = XUSER_DATA_TYPE_BINARY;
			m_aBlocksData[n].data.binary.pbData = pSrc;
			m_aBlocksData[n].data.binary.cbData = dwCurSize;
			// двигаем указатель на исходные данные
			pSrc += dwCurSize;
			szLeft -= dwCurSize;
		}
	}

	// запись сейва в профайл
	ZeroMemory( &m_xovSaveLoad, sizeof(XOVERLAPPED) );
	m_xovSaveLoad.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
	if( m_xovSaveLoad.hEvent == NULL )
	{
		SetError("Save Data Error: Couldn't create event.");
		return false;
	}
	DWORD result = XUserWriteProfileSettings( m_dwSignedInUserIndex, nBlocksQ, m_aBlocksData, &m_xovSaveLoad );
	if( result == ERROR_IO_PENDING )
	{
		m_SaveDataState = SaveData_Saving;
		return true;
	}
	// запись не удалась - закрываем все ресурсы записи
	CloseHandle( m_xovSaveLoad.hEvent );
	m_xovSaveLoad.hEvent = NULL;

	// выдаем сообщение об ошибке
	SetError("Save Data Error: Can`t write data into profile.");
	char param[1024];
	crt_snprintf(param,sizeof(param),"SignedUserIndex=%d, WriteBlockQuantity=%d, ResultCode=%x", m_dwSignedInUserIndex, nBlocksQ, result);
	SetError(param);
	SetError("Write data:");
	for( n=0; n<nBlocksQ; n++ )
	{
		crt_snprintf(param,sizeof(param), "  %d: type=%s, size=%d, ptr=%0X",
			n,
			m_aBlocksData[n].data.type == XUSER_DATA_TYPE_NULL ? "XUSER_DATA_TYPE_NULL" : "XUSER_DATA_TYPE_BINARY",
			m_aBlocksData[n].data.binary.cbData,
			m_aBlocksData[n].data.binary.pbData);
		SetError(param);
	}

	return false;
}

bool XboxLiveManager::IsSaveDataProcess()
{
	return m_SaveDataState == SaveData_Saving;
	// Пока не задействовано асинхронное запись/чтение, все чтения и записи проводятся синхронно во время
	// исполнения функций записи/чтения. Ситуация может измениться если будет нужна запись на диск/флеш
	// (использовать для этого создание и енумерацию контентов), или если из записи/чтения профайла, отчего то
	// нужно будет быстро возвращаться в основной поток... тогда надо будет переделать.
	//return false;
}

bool XboxLiveManager::LoadDataIsEnable()
{
	return (m_pSaveData!=NULL && m_dwSaveDataSize>0);
}

bool XboxLiveManager::GetProfileSettings(GP::SettingsData & settings)
{
	// неназначенный пользователь идет лесом
	if( !UserSignedIsValid() )
	{
		SetError("Get Profile Settings: Error - No signed users");
		switch( XGetLanguage() )
		{
		case XC_LANGUAGE_ENGLISH: crt_strcpy(settings.idLang, sizeof(settings.idLang), "eng"); break;
		//case XC_LANGUAGE_JAPANESE:
		//case XC_LANGUAGE_GERMAN German 
		//case XC_LANGUAGE_FRENCH French 
		//case XC_LANGUAGE_SPANISH Spanish 
		//case XC_LANGUAGE_ITALIAN Italian 
		//case XC_LANGUAGE_KOREAN Korean 
		//case XC_LANGUAGE_TCHINESE Chinese (Traditional) 
		//case XC_LANGUAGE_PORTUGUESE Portuguese 
		//case XC_LANGUAGE_SCHINESE Chinese (Simplified) 
		case XC_LANGUAGE_POLISH: crt_strcpy(settings.idLang, sizeof(settings.idLang), "pol"); break;
		case XC_LANGUAGE_RUSSIAN: crt_strcpy(settings.idLang, sizeof(settings.idLang), "rus"); break;
		}
		// пытаемся считать общие данные для всех пользователей
		return false;
	}

	// набор значений которые мы пытаемся получить
	static const DWORD SettingQnt = 8;
	DWORD SettingIDs[SettingQnt] = {
		XPROFILE_GAMERCARD_REGION,
		XPROFILE_OPTION_VOICE_MUTED,
		XPROFILE_OPTION_VOICE_VOLUME,
		XPROFILE_GAMER_CONTROL_SENSITIVITY,
		XPROFILE_GAMER_DIFFICULTY,
		XPROFILE_GAMER_ACTION_AUTO_AIM,
		XPROFILE_GAMER_YAXIS_INVERSION,
		XPROFILE_OPTION_CONTROLLER_VIBRATION
	};
	DWORD dwSettingSizeMax = 0;

	m_xuidProfileUser = m_dwSignedInUserIndex<USER_QUANTITY ? m_UserDescr[m_dwSignedInUserIndex].xuidUser : INVALID_XUID;

	// получим размер нужный для буфера с установками
	DWORD dwErr = XUserReadProfileSettings( 0,				// A title in your family or 0 for the current title
                                      m_dwSignedInUserIndex,// Player index making the request
                                      SettingQnt,			// Number of settings to read
                                      SettingIDs,			// List of settings to read
                                      &dwSettingSizeMax,	// Results size
                                      NULL,					// Results go here
                                      NULL );				// Overlapped struct
	// проверим успешность получения размера буфера
	if( FAILED(dwErr) )
	{
		SetError("Get Profile Settings: Error - XUserReadProfileSettings() failed on get buffer size");
		return false;
	}
	if( dwSettingSizeMax == 0 )
	{
		SetError("Get Profile Settings: Error - needed settings buffer is zerro length");
		return false;
	}

	// выделяем буфер
	BYTE * pdwSettings = NEW BYTE[dwSettingSizeMax];
	Assert(pdwSettings);

	// считываем установки в этот буфер
	dwErr = XUserReadProfileSettings( 0,						// A title in your family or 0 for the current title
                                      m_dwSignedInUserIndex,	// Player index making the request
                                      SettingQnt,				// Number of settings to read
                                      SettingIDs,				// List of settings to read
                                      &dwSettingSizeMax,		// Results size
                                      (XUSER_READ_PROFILE_SETTING_RESULT*)pdwSettings,
                                      NULL );
	// проверим успешность получения данных
	if( FAILED(dwErr) )
	{
		SetError("Get Profile Settings: Error - XUserReadProfileSettings() failed on get data");
		delete pdwSettings;
		return false;
	}

	// парсим полученные даннные в нашу структуру
	for( dword n=0; n+sizeof(XUSER_READ_PROFILE_SETTING_RESULT) <= dwSettingSizeMax; )
	{
		XUSER_READ_PROFILE_SETTING_RESULT* pProfSet = (XUSER_READ_PROFILE_SETTING_RESULT*)&pdwSettings[n];
		n += pProfSet->dwSettingsLen;
		if( pProfSet->dwSettingsLen < sizeof(XUSER_PROFILE_SETTING) ||
			pProfSet->pSettings == NULL ||
			n > dwSettingSizeMax )
			break;

		const XUSER_PROFILE_SETTING& set = *pProfSet->pSettings;
		// смотрим что это за параметр
		switch( set.dwSettingId )
		{
		case XPROFILE_OPTION_VOICE_MUTED:
			settings.voiceMuted = set.data.nData!=0;
			break;
		case XPROFILE_OPTION_VOICE_VOLUME:
			settings.voiceVolume = set.data.fData;
			break;
		case XPROFILE_GAMER_CONTROL_SENSITIVITY:
			switch(set.data.nData)
			{
			case XPROFILE_CONTROL_SENSITIVITY_MEDIUM: settings.controlSensitivity = 1.f; break;
			case XPROFILE_CONTROL_SENSITIVITY_LOW: settings.controlSensitivity = 0.5f; break;
			case XPROFILE_CONTROL_SENSITIVITY_HIGH: settings.controlSensitivity = 2.f; break;
			}
			break;
		case XPROFILE_GAMER_DIFFICULTY:
			switch( set.data.nData )
			{
			case XPROFILE_GAMER_DIFFICULTY_NORMAL: settings.gameDifficulty = 1; break;
			case XPROFILE_GAMER_DIFFICULTY_EASY: settings.gameDifficulty = 0; break;
			case XPROFILE_GAMER_DIFFICULTY_HARD: settings.gameDifficulty = 2; break;
			}
			break;
		case XPROFILE_GAMER_ACTION_AUTO_AIM:
			settings.gameAutoaim = set.data.nData != XPROFILE_ACTION_AUTO_AIM_OFF;
			break;
		case XPROFILE_GAMER_YAXIS_INVERSION:
			settings.gameYAxisInversion = set.data.nData != 0;
			break;
		case XPROFILE_GAMERCARD_REGION:
			crt_strcpy( settings.idLang, sizeof(settings.idLang), GetLangId(set.data.nData) );
			break;
		case XPROFILE_OPTION_CONTROLLER_VIBRATION:
			settings.vibration = set.data.nData != 0;
			break;
		}
	}

	delete pdwSettings;
	return true;
}

void XboxLiveManager::ShowSignInUI()
{
	XShowSigninUI( 4, 0 );
}

void XboxLiveManager::SetCheckedUsers(long nUserQuantity)
{
	long n;

	m_dwCheckableQuantity = nUserQuantity;

	// всех отключаем
	for( n=0; n<USER_QUANTITY; n++ )
	{
		m_UserDescr[n].bCheckable = false;
	}

	// для одного игрока не включаем проверку многопользовательского режима
	if( nUserQuantity <= 1 )
	{
		ChangeContext(m_dwCurrentContext);
		return;
	}

	// нет сервиса управления, то считаем что не нашли пользователя.
	if( !m_pControls )
		m_pControls = (IControlsService*)api->GetService("ControlsService");
	if( !m_pControls )
		return;
	InputSrvQueryAssignIndex assignIdx(0);

	//
	long idx = 1;
	for( n=0; n<USER_QUANTITY; n++ )
	{
		// ремап пользователя на используемый порт
		assignIdx.joypadIndex = n;
		m_pControls->ExecuteCommand( assignIdx );
		if( assignIdx.assignIndex<0 || assignIdx.assignIndex>=USER_QUANTITY ) continue;

		// пользователя надо проверять?
		if( (nUserQuantity & idx) != 0 )
			m_UserDescr[assignIdx.assignIndex].bCheckable = true;
		else
			// сбрасываем флаг изменения пользователя, для убраного из проверки
			m_UserDescr[assignIdx.assignIndex].bUserSignChanged = false;

		// следующий бит маски
		idx = (idx << 1);
	}

	ChangeContext(m_dwCurrentContext);
}

bool XboxLiveManager::IsChangedProfile()
{
	// безусловно была смена профайла
	if( m_bSignChanged ) return true;

	// пользователь новый, а профайл от другого пользователя?
	if( m_dwSignedInUserIndex != SignedUser_NotSigned && m_xuidProfileUser != m_UserDescr[m_dwSignedInUserIndex].xuidUser )
		return true;

	// пользователя нет, а профайл от предыдущего пользователя?
	if( m_dwSignedInUserIndex == SignedUser_NotSigned && m_xuidProfileUser != INVALID_XUID )
		return true;

	return false;
}

bool XboxLiveManager::ServiceIsReady()
{
	return true;//UserSignedIsValid();
}

ILiveManager::ReadyState XboxLiveManager::GetReadyState()
{
	// проверяем является ли текущий хозяин действительно хозяином
	if( m_SaveDataState==SaveData_Present && !m_bUserIsCreator )
		return ILiveManager::rs_stranger_savedata;

	// сменившийся пользователь вызывает состояние неготовности
	if( m_bSignChanged )
		return ILiveManager::rs_stranger_savedata;

	// нет хозяина - сразу считаем что у нас бяка (нет назначенного пользователя)
	if( m_dwSignedInUserIndex == SignedUser_NotSigned )
		return ILiveManager::rs_not_signed;
	else
	{
		if( !m_UserDescr[m_dwSignedInUserIndex].bControllerPluged )
			return ILiveManager::rs_unpluged;
		if( !m_UserDescr[m_dwSignedInUserIndex].bUserSigned )
			return ILiveManager::rs_not_signed;
	}

	// бежим по всем пользователям (если надо следить за ними, то следим)
	for( dword n=0; n<USER_QUANTITY; n++ )
	{
		// за пользователем надо следить
		if( m_UserDescr[n].bCheckable )
		{
			// пользователь отключен - возвращаем состояние что он отключен
			if( !m_UserDescr[n].bControllerPluged )
				return ILiveManager::rs_unpluged;
			if( m_UserDescr[n].bUserSignChanged )
				//return ILiveManager::rs_not_signed;
				return ILiveManager::rs_stranger_savedata;
			// пользователь не назначен
			if( !m_UserDescr[n].bUserSigned )
				return ILiveManager::rs_not_signed;
		}
	}

	return ILiveManager::rs_ready;
}

void XboxLiveManager::ChangeContext(dword dwContext)
{
	m_dwCurrentContext = dwContext;

	for( dword n=0; n<USER_QUANTITY; n++ )
	{
		dword dwUserContext = CONTEXT_PRESENCE_PRESENCE_IDLE;
		// выбираем только засигненных пользователей
		if( m_UserDescr[n].bCheckable || n==m_dwSignedInUserIndex )
			dwUserContext = dwContext;
		XUserSetContext(n,X_CONTEXT_PRESENCE,dwUserContext);
	}
}




void XboxLiveManager::ReleaseCurrentSaveData()
{
	DELETE( m_pSaveData );
	m_dwSaveDataSize = 0;
	m_SaveDataState = SaveData_None;
}

void XboxLiveManager::GetCurrentSaveData( void* & pData, dword & size )
{
	pData = m_pSaveData;
	size = m_dwSaveDataSize;
}

void XboxLiveManager::StopSaveLoadData()
{
	// закрыть евент
	if( m_xovSaveLoad.hEvent != NULL )
	{
		// дождемся пока закончится запись/чтение - ибо при ошибке надо все таки доделать запись, что бы не потерять сейв
		DWORD dwRes;
		XGetOverlappedResult(&m_xovSaveLoad,&dwRes,TRUE);
		// больше хендл ненужен
		if( m_xovSaveLoad.hEvent != NULL )
			CloseHandle( m_xovSaveLoad.hEvent );
		m_xovSaveLoad.hEvent = NULL;
		ReleaseCurrentSaveData();
	}

	// недочитанные данные считаем как отсутствие данных
	if( m_SaveDataState == SaveData_Loading )
		ReleaseCurrentSaveData();
}

bool XboxLiveManager::UserSignedIsValid()
{
	// нет хозяина, то выдаем бяку
	if( m_dwSignedInUserIndex == SignedUser_NotSigned )
		return false;
	// назначен ли пользователь и все ли с ним в порядке
	if( !m_UserDescr[m_dwSignedInUserIndex].bUserSigned ||
		!m_UserDescr[m_dwSignedInUserIndex].bControllerPluged )
		return false;
	return true;
}

void XboxLiveManager::FindSignedUser(bool bResign)
{
	// нет сервиса управления, то считаем что не нашли пользователя
	// (это только на старте игры может быть)
	if( !m_pControls )
		m_pControls = (IControlsService*)api->GetService("ControlsService");
	if( !m_pControls )
		return;

	// спрашиваем у контролов какой пользователь является основным (с него можно запускать игру)
	InputSrvQueryAssignIndex assignIdx(0);
	m_pControls->ExecuteCommand( assignIdx );

	// потеря пользователя только на моменте меню старта
	// сбрасываем все флаги неготовности, так как пользователь еще не выбран
	if( assignIdx.assignIndex == -1 )
	{
		m_bIsSetMasterUser = false;
		m_dwSignedInUserIndex = SignedUser_NotSigned;
		SetCheckedUsers(0);
	}
	else
	// игра пошла - теперь надо следить за сменой пользователя
	{
		// пользователь не был раньше определен - теперь это определение мастер пользователя
		if( !m_bIsSetMasterUser )
		{
			m_bIsSetMasterUser = true;
			m_bSignChanged = false;
			m_bUserIsCreator = true;
			m_dwSignedInUserIndex = assignIdx.assignIndex;
			// сейв уже не нужен
			StopSaveLoadData();
			ReleaseCurrentSaveData();
			SetCheckedUsers(0);
			// сброс изменения пользователя у всех ждойстиков
			for( long n=0; n<USER_QUANTITY; n++ )
				m_UserDescr[n].bUserSignChanged = false;
		}
		else
		{
			// сменился пользователь - нельзя продолжать игру старого хозяина
			if( assignIdx.assignIndex != m_dwSignedInUserIndex )
			{
				{
					// флаг изменения засигненного пользователя
					m_bSignChanged = true;
					// если есть данные сейва, то проверяем пользователя на принадлежность сейву
					if( assignIdx.assignIndex != -1 && m_SaveDataState == SaveData_Present )
					{
						m_bUserIsCreator = m_xuidSaveGameUser == m_UserDescr[assignIdx.assignIndex].xuidUser;
					}
					if( bResign )
						m_dwSignedInUserIndex = assignIdx.assignIndex;
					SetCheckedUsers(0);
				}
			}
		}
	}
}

void XboxLiveManager::NotificationFrame()
{
	if( m_hNotification==NULL )
		return;

	DWORD n;

	// смотрим системные евенты
    DWORD dwNotificationId;
    DWORD ulParam;

	// включение интерфейса?
    if( XNotifyGetNext( m_hNotification, XN_SYS_UI, &dwNotificationId, &ulParam ) )
		m_bXShow = ulParam;

	// включение/отключение устройств ввода(геймпад)?
	if( XNotifyGetNext( m_hNotification, XN_SYS_INPUTDEVICESCHANGED, &dwNotificationId, &ulParam ) )
	{
		XINPUT_STATE xst;
		for(n=0; n<USER_QUANTITY; n++) 
			m_UserDescr[n].bControllerPluged = XInputGetState(n,&xst)==ERROR_SUCCESS;
	}

	// реассигн пользователя?
    if( XNotifyGetNext( m_hNotification, XN_SYS_SIGNINCHANGED, &dwNotificationId, &ulParam ) )
	{
		// бегем по всем пользователям и получаем на них состояние
		for(n=0; n<USER_QUANTITY; n++)
		{
			// пользователь назначен?
			if( eXUserSigninState_NotSignedIn == XUserGetSigninState(n) )
			{
				if( n==m_dwSignedInUserIndex && m_UserDescr[n].xuidUser != INVALID_XUID )
					m_bSignChanged = true;
				if( m_SaveDataState!=SaveData_None && n==m_dwSignedInUserIndex )
					m_bUserIsCreator = false;

				m_UserDescr[n].bUserSigned = false;
				m_UserDescr[n].xuidUser = INVALID_XUID;
			}
			else
			{
				m_UserDescr[n].bUserSigned = true;
				XUSER_SIGNIN_INFO uinf;
				if( ERROR_SUCCESS == XUserGetSigninInfo(n,XUSER_GET_SIGNIN_INFO_OFFLINE_XUID_ONLY,&uinf) )
				// проверка на смену хозяина
				if( n==m_dwSignedInUserIndex )
				{
					// если пользователь поменялся, то ахтунг!
					if( m_UserDescr[n].xuidUser != uinf.xuid )
						m_bSignChanged = true;
					// если есть сейв, то может быть он уже невалидный
					if( m_SaveDataState!=SaveData_None && m_xuidSaveGameUser != uinf.xuid )
						m_bUserIsCreator = false;
				}
				// если пользователь поменялся, то ахтунг!
				if( m_UserDescr[n].bCheckable && m_UserDescr[n].xuidUser != uinf.xuid )
					m_UserDescr[n].bUserSignChanged = true;
				m_UserDescr[n].xuidUser = uinf.xuid;
			}
		}
		// обновим хозяина (если можно)
		FindSignedUser(false);
		// обновим контекстное состояние
		ChangeContext( m_dwCurrentContext );
	}
}

// Тут мы получаем символьный идентификатор языка (применяемого для дефолтного запуска)
// для неустановленных языков возвращаем "eng" - англицкий tongue
// FIX ME: требуется доработка для субмита в мелкомягкого
// пока все закоменчено, порядок следования всех дефайнов в порядке их определения (по возрастанию)
// язык "dev" я так понимаю русский (*непечатный язык разработчика), потом надо переопределить
const char* XboxLiveManager::GetLangId(dword dwProfileRegion)
{
	switch( dwProfileRegion )
	{
	//case XONLINE_COUNTRY_UNITED_ARAB_EMIRATES:
	//case XONLINE_COUNTRY_ALBANIA:
	//case XONLINE_COUNTRY_ARMENIA:
	//case XONLINE_COUNTRY_ARGENTINA:
	//case XONLINE_COUNTRY_AUSTRIA:
	//case XONLINE_COUNTRY_AUSTRALIA:
	//case XONLINE_COUNTRY_AZERBAIJAN:
	//case XONLINE_COUNTRY_BELGIUM:
	//case XONLINE_COUNTRY_BULGARIA:
	//case XONLINE_COUNTRY_BAHRAIN:
	//case XONLINE_COUNTRY_BRUNEI_DARUSSALAM:
	//case XONLINE_COUNTRY_BOLIVIA:
	//case XONLINE_COUNTRY_BRAZIL:
	//case XONLINE_COUNTRY_BELARUS:
	//case XONLINE_COUNTRY_BELIZE:
	//case XONLINE_COUNTRY_CANADA:
	//case XONLINE_COUNTRY_SWITZERLAND:
	//case XONLINE_COUNTRY_CHILE:
	//case XONLINE_COUNTRY_CHINA:
	//case XONLINE_COUNTRY_COLOMBIA:
	//case XONLINE_COUNTRY_COSTA_RICA:
	//case XONLINE_COUNTRY_CZECH_REPUBLIC:
	//case XONLINE_COUNTRY_GERMANY: return "ger"; break;
	//case XONLINE_COUNTRY_DENMARK:
	//case XONLINE_COUNTRY_DOMINICAN_REPUBLIC:
	//case XONLINE_COUNTRY_ALGERIA:
	//case XONLINE_COUNTRY_ECUADOR:
	//case XONLINE_COUNTRY_ESTONIA:
	//case XONLINE_COUNTRY_EGYPT:
	//case XONLINE_COUNTRY_SPAIN: return "spa"; break
	//case XONLINE_COUNTRY_FINLAND:
	//case XONLINE_COUNTRY_FAROE_ISLANDS:
	//case XONLINE_COUNTRY_FRANCE: return "fra"; break;
	case XONLINE_COUNTRY_GREAT_BRITAIN: return "eng"; break;
	//case XONLINE_COUNTRY_GEORGIA:
	//case XONLINE_COUNTRY_GREECE:
	//case XONLINE_COUNTRY_GUATEMALA:
	//case XONLINE_COUNTRY_HONG_KONG:
	//case XONLINE_COUNTRY_HONDURAS:
	//case XONLINE_COUNTRY_CROATIA:
	//case XONLINE_COUNTRY_HUNGARY:
	//case XONLINE_COUNTRY_INDONESIA:
	//case XONLINE_COUNTRY_IRELAND:
	//case XONLINE_COUNTRY_ISRAEL:
	//case XONLINE_COUNTRY_INDIA:
	//case XONLINE_COUNTRY_IRAQ:
	//case XONLINE_COUNTRY_IRAN:
	//case XONLINE_COUNTRY_ICELAND:
	//case XONLINE_COUNTRY_ITALY: return "ita"; break;
	//case XONLINE_COUNTRY_JAMAICA:
	//case XONLINE_COUNTRY_JORDAN:
	//case XONLINE_COUNTRY_JAPAN:
	//case XONLINE_COUNTRY_KENYA:
	//case XONLINE_COUNTRY_KYRGYZSTAN:
	//case XONLINE_COUNTRY_KOREA:
	//case XONLINE_COUNTRY_KUWAIT:
	//case XONLINE_COUNTRY_KAZAKHSTAN:
	//case XONLINE_COUNTRY_LEBANON:
	//case XONLINE_COUNTRY_LIECHTENSTEIN:
	//case XONLINE_COUNTRY_LITHUANIA:
	//case XONLINE_COUNTRY_LUXEMBOURG:
	//case XONLINE_COUNTRY_LATVIA:
	//case XONLINE_COUNTRY_LIBYA:
	//case XONLINE_COUNTRY_MOROCCO:
	//case XONLINE_COUNTRY_MONACO:
	//case XONLINE_COUNTRY_MACEDONIA:
	//case XONLINE_COUNTRY_MONGOLIA:
	//case XONLINE_COUNTRY_MACAU:
	//case XONLINE_COUNTRY_MALDIVES:
	//case XONLINE_COUNTRY_MEXICO:
	//case XONLINE_COUNTRY_MALAYSIA:
	//case XONLINE_COUNTRY_NICARAGUA:
	//case XONLINE_COUNTRY_NETHERLANDS:
	//case XONLINE_COUNTRY_NORWAY:
	//case XONLINE_COUNTRY_NEW_ZEALAND:
	//case XONLINE_COUNTRY_OMAN:
	//case XONLINE_COUNTRY_PANAMA:
	//case XONLINE_COUNTRY_PERU:
	//case XONLINE_COUNTRY_PHILIPPINES:
	//case XONLINE_COUNTRY_PAKISTAN:
	case XONLINE_COUNTRY_POLAND: return "pol"; break;
	//case XONLINE_COUNTRY_PUERTO_RICO:
	//case XONLINE_COUNTRY_PORTUGAL:
	//case XONLINE_COUNTRY_PARAGUAY:
	//case XONLINE_COUNTRY_QATAR:
	//case XONLINE_COUNTRY_ROMANIA:
	case XONLINE_COUNTRY_RUSSIAN_FEDERATION: return "dev"; break;//return "rus"; break;
	//case XONLINE_COUNTRY_SAUDI_ARABIA:
	//case XONLINE_COUNTRY_SWEDEN:
	//case XONLINE_COUNTRY_SINGAPORE:
	//case XONLINE_COUNTRY_SLOVENIA:
	//case XONLINE_COUNTRY_SLOVAK_REPUBLIC:
	//case XONLINE_COUNTRY_EL_SALVADOR:
	//case XONLINE_COUNTRY_SYRIA:
	//case XONLINE_COUNTRY_THAILAND:
	//case XONLINE_COUNTRY_TUNISIA:
	//case XONLINE_COUNTRY_TURKEY:
	//case XONLINE_COUNTRY_TRINIDAD_AND_TOBAGO:
	//case XONLINE_COUNTRY_TAIWAN:
	//case XONLINE_COUNTRY_UKRAINE:
	case XONLINE_COUNTRY_UNITED_STATES: return "eng"; break;
	//case XONLINE_COUNTRY_URUGUAY:
	//case XONLINE_COUNTRY_UZBEKISTAN:
	//case XONLINE_COUNTRY_VENEZUELA:
	//case XONLINE_COUNTRY_VIET_NAM:
	//case XONLINE_COUNTRY_YEMEN:
	//case XONLINE_COUNTRY_SOUTH_AFRICA:
	//case XONLINE_COUNTRY_ZIMBABWE:
		default:
			return "eng";
	}
}

bool XboxLiveManager::LoadAchievements()
{
	return false;
/*    HANDLE hEnum;
    DWORD cbBuffer;

	// занято!
	if( m_bAchievementSaving )
		return false;

	// пользователь не хозяин или невалидный
	if( !m_bUserIsCreator || !UserSignedIsValid() )
		return false;

	// ну, сначала, мы считаем что у нас 0 ачивментов
	m_dwAchievementCount = 0;

    // Получаем хедер для прохода по списку ачивментов
    DWORD dwStatus;
    dwStatus = XUserCreateAchievementEnumerator(
        0,                              // Enumerate for the current title
        m_dwSignedInUserIndex,
        INVALID_XUID,					// If INVALID_XUID, the current user's achievements
                                        // are enumerated
        XACHIEVEMENT_DETAILS_ALL,
        0,                              // starting achievement index
        ACHIEVEMENT_COUNT,              // number of achievements to return
        &cbBuffer,                      // bytes needed
        &hEnum );

	// не смогли создать список - выходим с ошибкой
	if ( dwStatus != ERROR_SUCCESS )
		return false;

	// выгребаем ачивменты из списка
	long nq = cbBuffer / XACHIEVEMENT_SIZE_FULL;
	if( nq > 0 )
	{
		// проверка ачивментов на максимальное их количество
		Assert( nq <= ACHIEVEMENT_COUNT );

		// теперь достаем ачивменты из списка
		DWORD dwItems;
		if( XEnumerate( hEnum, m_AchievementDetails, XACHIEVEMENT_SIZE_FULL * nq,
						&dwItems, NULL ) == ERROR_SUCCESS )
		{
			m_dwAchievementCount = dwItems;
		}
	}

	// грохнем хедер списка, он нам уже не нужен
	CloseHandle( hEnum );
	return true;*/
}

#endif
