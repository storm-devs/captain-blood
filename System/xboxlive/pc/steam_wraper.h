// убрать дефайн если не требуется использование стима для дебага (библиотека АЙПИ стима все равно подключается)
//#define USE_STEAMWORK	TRUE

#ifndef _XBOX
#ifdef USE_STEAMWORK

#ifndef STREAM_WRAPER_H
#define STREAM_WRAPER_H

#include "..\..\..\common_h\core.h"

#pragma warning( push )
// warning C4996: 'strncpy': This function or variable may be unsafe. Consider using strncpy_s instead.
#pragma warning( disable : 4996 ) 
#include "steam\steam_api.h"
#pragma warning( pop )

class CSteamWraper
{
	struct Achievement_t
	{
		int m_eAchievementID;
		const char *m_pchAchievementID;
		char m_rgchName[128];
		char m_rgchDescription[256];
		bool m_bAchieved;
		int m_iIconImage;
	};

private:
	static Achievement_t g_Achievements[];
	__int64 m_iAppID;				// Our current AppID
	bool m_bInitialized;			// Have we called Request stats and received the callback?

	Achievement_t *m_pAchievements;	// Achievements data
	int m_iNumAchievements;			// The number of Achievements

	// steam data
	ISteamUserStats *m_pSteamUserStats;			//
	ISteamRemoteStorage *m_pSteamRemoteStorage;	//

public:
	CSteamWraper();
	~CSteamWraper();

	bool Init();
	void Shutdown();
	void Frame();

	bool RequestStats();
	bool SetAchievement(int id);

	bool ReadData(dword & dwDataSize, byte* & pData, const char* pcFileName);
	bool SaveData(dword dwDataSize, const byte* pData, const char* pcFileName);

	STEAM_CALLBACK( CSteamWraper, OnUserStatsReceived, UserStatsReceived_t,
		m_CallbackUserStatsReceived );
	STEAM_CALLBACK( CSteamWraper, OnUserStatsStored, UserStatsStored_t,
		m_CallbackUserStatsStored );
	STEAM_CALLBACK( CSteamWraper, OnAchievementStored, UserAchievementStored_t,
		m_CallbackAchievementStored );

private:
	int FindAchievementByID(int id);
};

#endif

#endif
#endif
