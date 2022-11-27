#ifndef _XBOX
#ifdef USE_STEAMWORK

#include "steam_wraper.h"
#include "..\ILiveManager.h"

#pragma comment(lib,"xboxlive\\pc\\steam_api.lib")

#define _ACH_ID( id, name ) { id, #id, name, "", 0, 0 }

CSteamWraper::Achievement_t CSteamWraper::g_Achievements[] = 
{
	_ACH_ID( ACHIEVEMENT_FIRST_BLOOD, "FirstBlood" ),
	_ACH_ID( ACHIEVEMENT_SLAYER, "Murderer" ),
	_ACH_ID( ACHIEVEMENT_EXECUTIONER, "Executor" ),
	_ACH_ID( ACHIEVEMENT_RIPPER, "Reaper" ),
	_ACH_ID( ACHIEVEMENT_SAILOR, "Seamen" ),
	_ACH_ID( ACHIEVEMENT_MUSKETEER, "Musketeer" ),
	_ACH_ID( ACHIEVEMENT_DUELIST, "Duelist" ),
	_ACH_ID( ACHIEVEMENT_BUTCHER, "Butcher" ),
	_ACH_ID( ACHIEVEMENT_SHARPSHOOTER, "DeadShot" ),
	_ACH_ID( ACHIEVEMENT_KNIGHT, "Knight" ),
	_ACH_ID( ACHIEVEMENT_SNIPER, "Marksman" ),
	_ACH_ID( ACHIEVEMENT_DEMOLITION, "Destroyer" ),
	_ACH_ID( ACHIEVEMENT_SHREDDER, "Slayer" ),
	_ACH_ID( ACHIEVEMENT_HEADCUTTER, "Swashbuckler" ),
	_ACH_ID( ACHIEVEMENT_MURDERER, "Killa" ),
	_ACH_ID( ACHIEVEMENT_RAGE, "Rage" ),
	_ACH_ID( ACHIEVEMENT_WEAPON_MASTER, "Weaponmaster" ),
	_ACH_ID( ACHIEVEMENT_RACKETEER, "Miser" ),
	_ACH_ID( ACHIEVEMENT_MERCILESS, "Ruthless" ),
	_ACH_ID( ACHIEVEMENT_BERSERKER, "Berserker" ),
	_ACH_ID( ACHIEVEMENT_BOATSWAIN, "Bosun" ),
	_ACH_ID( ACHIEVEMENT_CAPTAIN, "Captain" ),
	_ACH_ID( ACHIEVEMENT_CAPER, "Caper" ),
	_ACH_ID( ACHIEVEMENT_JOLLY_ROGER, "JollyRoger" ),
	_ACH_ID( ACHIEVEMENT_PIRATE_KING, "PirateKing" ),
	_ACH_ID( ACHIEVEMENT_AVENGER, "Avenger" ),
	_ACH_ID( ACHIEVEMENT_DEFENDER_OF_THE_WEAK, "DefenderOfTheWeak" ),
	_ACH_ID( ACHIEVEMENT_MASTER_OF_REVENGE, "MasterOfRevenge" ),
	_ACH_ID( ACHIEVEMENT_PETER_THE_CONQUEROR, "Conqueror" ),
	_ACH_ID( ACHIEVEMENT_PETER_THE_VICTORIOUS, "Champion" ),
	_ACH_ID( ACHIEVEMENT_CAPTAIN_BLOOD, "Captain Blood" ),
	_ACH_ID( ACHIEVEMENT_FISH_FOOD, "SharkFood" ),
	_ACH_ID( ACHIEVEMENT_TRICKSTER, "Trickster" ),
	_ACH_ID( ACHIEVEMENT_BURGLAR, "Door Cracker" ),
	_ACH_ID( ACHIEVEMENT_UNSTOPPABLE, "Juggernaut" ),
	_ACH_ID( ACHIEVEMENT_LOOTER, "Looter" ),
	_ACH_ID( ACHIEVEMENT_THE_MONEY_BAG, "Moneybags" ),
	_ACH_ID( ACHIEVEMENT_TREASURE_HUNTER, "TreasureHunter" ),
	_ACH_ID( ACHIEVEMENT_MASTER, "Master" ),
	_ACH_ID( ACHIEVEMENT_GUNMASTER, "Gunmaster" ),
	_ACH_ID( ACHIEVEMENT_CATAPULT, "Catapult" ),
	_ACH_ID( ACHIEVEMENT_NOVICE, "Novice" ),
	_ACH_ID( ACHIEVEMENT_SURVIVOR, "Survivor" ),
	_ACH_ID( ACHIEVEMENT_VETERAN, "Veteran" ),
	_ACH_ID( ACHIEVEMENT_MANSLAUGHTER, "Manslaughter" ),
	_ACH_ID( ACHIEVEMENT_MASSACRE, "MeatGrinder" ),
	_ACH_ID( ACHIEVEMENT_SEA_WOLF, "Seawolf" ),
	_ACH_ID( ACHIEVEMENT_TERROR_OF_THE_SEA, "TerrorOfTheSea" )
};

#pragma warning( push )
#pragma warning( disable : 4355 )
CSteamWraper::CSteamWraper() :
	m_iAppID( 0 ),
	m_bInitialized( false ),
	m_CallbackUserStatsReceived( this, &CSteamWraper::OnUserStatsReceived ),
	m_CallbackUserStatsStored( this, &CSteamWraper::OnUserStatsStored ),
	m_CallbackAchievementStored( this, &CSteamWraper::OnAchievementStored )
{
	m_pAchievements = g_Achievements;
	m_iNumAchievements = sizeof(g_Achievements) / sizeof(Achievement_t);

	m_pSteamUserStats = NULL;
	m_pSteamRemoteStorage = NULL;

	if( !SteamAPI_Init() )
	{
		ILiveManager::SetError("Steam error: Hav`t init steam api");
	}
	else
	{
		m_iAppID = SteamUtils()->GetAppID();
	}
}
#pragma warning( pop )

CSteamWraper::~CSteamWraper()
{
	Shutdown();
}

bool CSteamWraper::Init()
{
	m_pSteamUserStats = SteamUserStats();
	if( !m_pSteamUserStats )
		ILiveManager::SetError("Steam error: Can`t get steam user stats");

	m_pSteamRemoteStorage = SteamRemoteStorage();
	if( !m_pSteamRemoteStorage )
		ILiveManager::SetError("Steam error: Can`t get steam remote storage");

	return RequestStats();
}

void CSteamWraper::Shutdown()
{
	SteamAPI_Shutdown();
	m_pSteamUserStats = NULL;
	m_pSteamRemoteStorage = NULL;
}

void CSteamWraper::Frame()
{
	SteamAPI_RunCallbacks();
}

bool CSteamWraper::RequestStats()
{
	// Is Steam loaded? If not we can't get stats.
	if ( !m_pSteamUserStats || NULL == SteamUser() )
		return false;

	// Is the user logged on?  If not we can't get stats.
	if ( !SteamUser()->BLoggedOn() )
		return false;

	// Request user stats.
	return m_pSteamUserStats->RequestCurrentStats();
}

bool CSteamWraper::SetAchievement(int id)
{
	// Have we received a call back from Steam yet?
	if (m_bInitialized)
	{
		int n = FindAchievementByID(id);
		if( m_pAchievements && n>=0 && n < m_iNumAchievements )
		{
			m_pSteamUserStats->SetAchievement( m_pAchievements[n].m_pchAchievementID );
			return m_pSteamUserStats->StoreStats();
		}
	}
	// If not then we can't set achievements yet
	return false;
}

bool CSteamWraper::ReadData(dword & dwDataSize, byte* & pData, const char* pcFileName)
{
	if( m_pSteamRemoteStorage )
	{
		dwDataSize = m_pSteamRemoteStorage->GetFileSize( pcFileName );
		if( dwDataSize > 0 )
		{
			pData = NEW byte[dwDataSize];
			Assert(pData);
			// при чтении прочитали все
			if( m_pSteamRemoteStorage->FileRead( pcFileName, pData, dwDataSize ) == dwDataSize )
				return true;
			// удаляем ошибочные данные
			DELETE(pData);
			dwDataSize = 0;
			ILiveManager::SetError("SteamWraper: Steamwork FileRead() failed.");
		}
		else
			ILiveManager::SetError("SteamWraper: No save data.");
	}
	return false;
}

bool CSteamWraper::SaveData(dword dwDataSize, const byte* pData, const char* pcFileName)
{
	if( m_pSteamRemoteStorage )
		if( m_pSteamRemoteStorage->FileWrite( pcFileName, pData, dwDataSize ) )
			return true;

	ILiveManager::SetError("SteamWraper: FileWrite() failed.");
	return false;
}

void CSteamWraper::OnUserStatsReceived( UserStatsReceived_t *pCallback )
{
	// we may get callbacks for other games' stats arriving, ignore them
	if ( m_iAppID == pCallback->m_nGameID )
	{
		if ( k_EResultOK == pCallback->m_eResult )
		{
			ILiveManager::SetError("Received stats and achievements from Steam\n");
			m_bInitialized = true;

			// load achievements
			for ( int iAch = 0; iAch < m_iNumAchievements; ++iAch )
			{
				Achievement_t &ach = m_pAchievements[iAch];

				m_pSteamUserStats->GetAchievement(ach.m_pchAchievementID, &ach.m_bAchieved);
				crt_snprintf( ach.m_rgchName, sizeof(ach.m_rgchName), "%s",
					m_pSteamUserStats->GetAchievementDisplayAttribute(ach.m_pchAchievementID, "name"));
				crt_snprintf( ach.m_rgchDescription, sizeof(ach.m_rgchDescription), "%s",
					m_pSteamUserStats->GetAchievementDisplayAttribute(ach.m_pchAchievementID, "desc"));
			}
		}
		else
		{
			char buffer[128];
			crt_snprintf( buffer, 128, "RequestStats - failed, %d\n", pCallback->m_eResult );
			ILiveManager::SetError( buffer );
		}
	}
}

void CSteamWraper::OnUserStatsStored( UserStatsStored_t *pCallback )
{
	// we may get callbacks for other games' stats arriving, ignore them
	if ( m_iAppID == pCallback->m_nGameID )	
	{
		if ( k_EResultOK == pCallback->m_eResult )
		{
			ILiveManager::SetError( "Stored stats for Steam\n" );
		}
		else
		{
			char buffer[128];
			crt_snprintf( buffer, 128, "StatsStored - failed, %d\n", pCallback->m_eResult );
			ILiveManager::SetError( buffer );
		}
	}
}

void CSteamWraper::OnAchievementStored( UserAchievementStored_t *pCallback )
{
     // we may get callbacks for other games' stats arriving, ignore them
     if ( m_iAppID == pCallback->m_nGameID )
     {
          ILiveManager::SetError( "Stored Achievement for Steam\n" );
     }
}

int CSteamWraper::FindAchievementByID(int id)
{
	if( !m_pAchievements )
		return -1;
	for( int n=0; n<m_iNumAchievements; n++ )
		if( m_pAchievements[n].m_eAchievementID == id )
			return n;
	return -1;
}

#endif
#endif
