#ifndef _XBOX
#include "PCLiveManager.h"
#include "..\..\..\Common_h\FileService.h"
#include "..\..\..\common_h\winworkpath.h"

#include "steam_wraper.h"

PCLiveManager::PCLiveManager()
{
	m_pSaveData = NULL;
	m_dwSaveDataSize = 0;

	m_bSignIn = true;
	m_bStrangerUser = false;
	m_bPluginController = true;

	m_pSteamWraper = NULL;
	m_bOnlyLocalSaves = true;
}

PCLiveManager::~PCLiveManager()
{
	DELETE(m_pSaveData);
#ifdef USE_STEAMWORK
	DELETE(m_pSteamWraper);
#endif
}

bool PCLiveManager::Init()
{
#ifdef USE_STEAMWORK
	bool bUseSteam = true;
	IFileService* pFileService = (IFileService*)api->GetService("FileService");
	if( pFileService )
	{
		// системный инишник
		IIniFile* pIni = pFileService->SystemIni();
		if( pIni )
		{
			m_bOnlyLocalSaves = (pIni->GetLong("","localsave",0) != 0);
			bUseSteam = (pIni->GetLong("","steamturnon",0) == 1);
		}
	}

	if( bUseSteam )
	{
		m_pSteamWraper = NEW CSteamWraper();
		// сейв будет локальным если стима нет или пользователь не подключен
		if( !m_pSteamWraper || !m_pSteamWraper->Init() )
			m_bOnlyLocalSaves = true;
	}
#endif

	return true;
}

void PCLiveManager::Frame()
{
#ifdef USE_STEAMWORK
	if( m_pSteamWraper )
		m_pSteamWraper->Frame();
#endif
}

bool PCLiveManager::WriteAchievement(ILiveManager::UserAchievement* pAchievementsBuffer, dword dwAchievementsQuantity)
{
	bool bWrited = false;

#ifdef USE_STEAMWORK
	if( !m_pSteamWraper ) return true;
	for( dword n=0; n<dwAchievementsQuantity; n++ )
	{
		// ачивменты добавляем только основному игроку (без мультиплеера)
		if( pAchievementsBuffer[n].userIndex != 0 ) continue;

		if( m_pSteamWraper->SetAchievement( pAchievementsBuffer[n].achievementID ) )
			bWrited = true;
	}
#endif

	// ничего не записывали, считаем что все удачно - что бы удалить ачивки из списка для записи
	if( !bWrited )
		return true;

	return true;
}

bool PCLiveManager::GetAchievementString(dword achievementID, string & str)
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

bool PCLiveManager::IsWriteProcessAchievemets()
{
	return false;
}

bool PCLiveManager::GetAchievementEarnedState(dword userIndex, dword achievementID, bool & isEarned)
{
	return false;
}

bool PCLiveManager::LoadData( void* & pData, dword & size )
{
	// неподключенный пользователь ничего не может загрузить
	if( !m_bSignIn )
	{
		SetError("Load Data Error: Hav`t signed user");
		return false;
	}

	// уже есть сейв в памяти и текущий пользователь является его владельцем, то этот сейв и возвращаем
	if( !m_bStrangerUser && m_pSaveData && m_dwSaveDataSize > 0 )
	{
		pData = m_pSaveData;
		size = m_dwSaveDataSize;
		return true;
	}

	// Нам надо загрузить новый сейв, то удаляем из памяти старый
	DELETE(m_pSaveData);
	m_dwSaveDataSize = 0;
	m_bStrangerUser = false;

	// делать локальный сейв/лоад
	bool bMakeLocalLoad = true;

#ifdef USE_STEAMWORK
	// использовать локальный сейв?
	if( !m_bOnlyLocalSaves )
	{
		// использовать стим?
		if( m_pSteamWraper )
		{
			bMakeLocalLoad = false;
			if( !m_pSteamWraper->ReadData( m_dwSaveDataSize, m_pSaveData, GetProfileName() ) )
			{
				SetError("Load Data Error: Streamwork FileRead() failed.");
			}
		}
	}
#endif
	if( bMakeLocalLoad )
	{
		// заглушка для сейва при нерабочем стиме
		IFileService * fs = (IFileService *)api->GetService("FileService");
		if( fs )
		{
			IDataFile* pDatFile = fs->OpenDataFile( GetProfileName(), file_open_fromdisk, _FL_ );
			//ILoadBuffer * data = fs->LoadData(GetProfileName(), _FL_);
			//if( data )
			if( pDatFile )
			{
				m_dwSaveDataSize = pDatFile->Size();
				if( m_dwSaveDataSize > 0 )
				{
					m_pSaveData = NEW byte[m_dwSaveDataSize];
					Assert(m_pSaveData);
					if( pDatFile->Read(m_pSaveData,m_dwSaveDataSize) != m_dwSaveDataSize )
					{
						DELETE(m_pSaveData);
						m_dwSaveDataSize = 0;
					}
					//memcpy( m_pSaveData, data->Buffer(), m_dwSaveDataSize );
				}
				//data->Release();
				pDatFile->Release();
			}
		}
	}


	if( m_dwSaveDataSize > 0 )
	{
		pData = m_pSaveData;
		size = m_dwSaveDataSize;
		return true;
	}

	SetError("Load Data Message: no save data");
	return  false;
}

bool PCLiveManager::SaveData( void* pData, dword size )
{
	// неподключенный пользователь ничего не может сохранить
	if( !m_bSignIn )
	{
		SetError("Save Data Error: Hav`t signed user");
		return false;
	}
	// чужой сейв нельзя сохранить как свой
	if( m_bStrangerUser )
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
	if( size>MAX_SAVE_DATA_SIZE )
	{
		SetError("Save Data Error: Save data size overflow 3 KB limit.");
		return false;
	}

	// Release old save data
	DELETE(m_pSaveData);
	m_dwSaveDataSize = 0;

	bool bSuccessWriteData = false;

	// делать локальный сейв/лоад
	bool bMakeLocalLoad = true;

#ifdef USE_STEAMWORK
	if( !m_bOnlyLocalSaves )
	{
		// запрещаем локальный
		if( m_pSteamWraper )
		{
			bMakeLocalLoad = false;
			bSuccessWriteData = m_pSteamWraper->SaveData(size, (const byte*)pData, GetProfileName());
		}
	}
#endif
	if( bMakeLocalLoad )
	{
		IFileService * fs = (IFileService *)api->GetService("FileService");
		if( fs )
			if( fs->SaveData(GetProfileName(), pData, size) )
				bSuccessWriteData = true;
	}

	if( bSuccessWriteData )
	{
		// store into memory new save data
		m_pSaveData = NEW byte[size];
		Assert(m_pSaveData);
		memcpy( m_pSaveData, pData, size );
		m_dwSaveDataSize = size;
		return true;
	}

	return false;
}

bool PCLiveManager::IsSaveDataProcess()
{
	return false;
}

bool PCLiveManager::LoadDataIsEnable()
{
	return (m_pSaveData!=NULL && m_dwSaveDataSize>0);
}

bool PCLiveManager::GetProfileSettings(GP::SettingsData & settings)
{
	return true;
}

dword PCLiveManager::GetSignedInUser()
{
	return 0;
}

void PCLiveManager::ShowSignInUI()
{
}

bool PCLiveManager::ServiceIsReady()
{
	return m_bSignIn && !m_bStrangerUser && m_bPluginController;
}

PCLiveManager::ReadyState PCLiveManager::GetReadyState()
{
	if( api->DebugKeyState(VK_SHIFT,VK_CONTROL,'J') )
		m_bSignIn = true;
	if( api->DebugKeyState(VK_SHIFT,VK_CONTROL,'K') )
		m_bSignIn = false;

	if( !m_bSignIn )
		return ILiveManager::rs_not_signed;
	if( m_bStrangerUser )
		return ILiveManager::rs_stranger_savedata;
	if( !m_bPluginController )
		return ILiveManager::rs_unpluged;

	return ILiveManager::rs_ready;
}

const char* PCLiveManager::GetProfileName()
{
	bool bSteamFolderSave = false;
#ifdef USE_STEAMWORK
	if( !m_bOnlyLocalSaves && m_pSteamWraper )
		bSteamFolderSave = true;
#endif
	if(bSteamFolderSave)
		return "capblood.prf";

	SystemWorkPath workPath(SystemWorkPath::t_profile);
	workPath.AppendFileName("test.profile");
	g_WorkPath = workPath.GetPath();
	return g_WorkPath.c_str();
}

const char* PCLiveManager::GetAchievementNameByID(dword achievementID)
{
	switch(achievementID)
	{
		case ACHIEVEMENT_FIRST_BLOOD: return "FirstBlood"; break;
		case ACHIEVEMENT_SLAYER: return "Murderer"; break;
		case ACHIEVEMENT_EXECUTIONER: return "Executor"; break;
		case ACHIEVEMENT_RIPPER: return "Reaper"; break;
		case ACHIEVEMENT_SAILOR: return "Seamen"; break;
		case ACHIEVEMENT_MUSKETEER: return "Musketeer"; break;
		case ACHIEVEMENT_DUELIST: return "Duelist"; break;
		case ACHIEVEMENT_BUTCHER: return "Butcher"; break;
		case ACHIEVEMENT_SHARPSHOOTER: return "DeadShot"; break;
		case ACHIEVEMENT_KNIGHT: return "Knight"; break;
		case ACHIEVEMENT_SNIPER: return "Marksman"; break;
		case ACHIEVEMENT_DEMOLITION: return "Destroyer"; break;
		case ACHIEVEMENT_SHREDDER: return "Slayer"; break;
		case ACHIEVEMENT_HEADCUTTER: return "Swashbuckler"; break;
		case ACHIEVEMENT_MURDERER: return "Killa"; break;
		case ACHIEVEMENT_RAGE: return "Rage"; break;
		case ACHIEVEMENT_WEAPON_MASTER: return "Weaponmaster"; break;
		case ACHIEVEMENT_RACKETEER: return "Miser"; break;
		case ACHIEVEMENT_MERCILESS: return "Ruthless"; break;
		case ACHIEVEMENT_BERSERKER: return "Berserker"; break;
		case ACHIEVEMENT_BOATSWAIN: return "Bosun"; break;
		case ACHIEVEMENT_CAPTAIN: return "Captain"; break;
		case ACHIEVEMENT_CAPER: return "Caper"; break;
		case ACHIEVEMENT_JOLLY_ROGER: return "JollyRoger"; break;
		case ACHIEVEMENT_PIRATE_KING: return "PirateKing"; break;
		case ACHIEVEMENT_AVENGER: return "Avenger"; break;
		case ACHIEVEMENT_DEFENDER_OF_THE_WEAK: return "DefenderOfTheWeak"; break;
		case ACHIEVEMENT_MASTER_OF_REVENGE: return "MasterOfRevenge"; break;
		case ACHIEVEMENT_PETER_THE_CONQUEROR: return "Conqueror"; break;
		case ACHIEVEMENT_PETER_THE_VICTORIOUS: return "Champion"; break;
		case ACHIEVEMENT_CAPTAIN_BLOOD: return "Captain Blood"; break;
		case ACHIEVEMENT_FISH_FOOD: return "SharkFood"; break;
		case ACHIEVEMENT_TRICKSTER: return "Trickster"; break;
		case ACHIEVEMENT_BURGLAR: return "Door Cracker"; break;
		case ACHIEVEMENT_UNSTOPPABLE: return "Juggernaut"; break;
		case ACHIEVEMENT_LOOTER: return "Looter"; break;
		case ACHIEVEMENT_THE_MONEY_BAG: return "Moneybags"; break;
		case ACHIEVEMENT_TREASURE_HUNTER: return "TreasureHunter"; break;
		case ACHIEVEMENT_MASTER: return "Master"; break;
		case ACHIEVEMENT_GUNMASTER: return "Gunmaster"; break;
		case ACHIEVEMENT_CATAPULT: return "Catapult"; break;
		case ACHIEVEMENT_NOVICE: return "Novice"; break;
		case ACHIEVEMENT_SURVIVOR: return "Survivor"; break;
		case ACHIEVEMENT_VETERAN: return "Veteran"; break;
		case ACHIEVEMENT_MANSLAUGHTER: return "Manslaughter"; break;
		case ACHIEVEMENT_MASSACRE: return "MeatGrinder"; break;
		case ACHIEVEMENT_SEA_WOLF: return "Seawolf"; break;
		case ACHIEVEMENT_TERROR_OF_THE_SEA: return "TerrorOfTheSea"; break;
	}

	static char achid[256];
	crt_snprintf(achid,sizeof(achid), "ach%d", achievementID);
	return achid;
}
#endif
