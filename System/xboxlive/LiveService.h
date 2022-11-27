#ifndef _LIVESERVICE_H_
#define _LIVESERVICE_H_

#include "..\..\Common_h\ILiveService.h"
#include "ILiveManager.h"
#include "..\..\XENGINE\UnderJollyRoger.spa.h"
#include "..\..\Common_h\IConsole.h"

#ifndef _XBOX
#include "pc\pcutils_temp\utils_temp.h"
#endif

class IFileService;
class IRender;
class IMission;

#define MAX_PLAYERS_COUNT	4

enum AchievementDescribeFlags
{
	adf_riched = 1,
	adf_multiplay = 2,
	adf_xlive_sinc = 4
};

struct AchievementDescribe
{
	int countMax;
	dword liveID;
	dword flags;
	int countCur[MAX_PLAYERS_COUNT];
};

class LiveService : public ILiveService
{
	// ачивмент для вывода при получении ачивмента
	struct reached_list
	{
		dword achievID;
		string str;
		bool ready;
	};

public:
	LiveService();
	virtual ~LiveService();

	void __declspec(dllexport) InitService();

	virtual bool Init();
	virtual void StartFrame(float dltTime);
	virtual void EndFrame(float dltTime);

	// готовность сервиса
	virtual GP::LiveServiceState GetServiceState();

	// включить слежение за определенным количеством пользователей
	virtual void SetCheckedUsersQuantity(int n);

	//--------------------------------------------------------------------
	// Работа с ачивментами
	//--------------------------------------------------------------------
	bool Achievements_IsReady();
	bool Achievement_IsReached(dword dwPlayerIndex, dword dwAchievementID);
	//
	virtual void Achievement_Earn(dword liveID);
	virtual void Achievement_EarnMP(dword player, dword liveID);

	virtual void Achievement_Increment(dword id, int count);
	virtual void Achievement_SetZero(dword id);
	virtual void Achievement_IncrementMP(dword player, dword id, int count);
	virtual void Achievement_SetZeroMP(dword player, dword id);

	//--------------------------------------------------------------------
	// Работа с сейвом
	//--------------------------------------------------------------------
	virtual bool Data_IsLoadEnable();
	virtual bool Data_Save();
	virtual bool Data_Load();
	virtual bool Option_Save();
	virtual bool Data_SaveLoadIsProcessing();
	virtual bool Data_SaveLoadIsSuccessful();

	//--------------------------------------------------------------------
	// Работа с профилем (данные профиля)
	//--------------------------------------------------------------------
	// получить данные из профайла
	virtual const GP::SettingsData& GetProfileSettings ();

	//--------------------------------------------------------------------
	// Работа с подгружаемым контентом
	//--------------------------------------------------------------------
	// состояние готовности сервиса работы с контентами
	virtual bool Content_IsReadyService();
	// количество доступных контентов
	virtual long Content_GetQuantity();
	// получить описание контента
	virtual bool Content_GetDescribe(long nContent,GP::ContentDescribe& descr);
	// подключить контент к игре
	virtual bool Content_Plugin(long nContent);
	// отключить контент из игры
	virtual bool Content_Unplug(long nContent);

	//--------------------------------------------------------------------
	// Для дебага
	//--------------------------------------------------------------------
	virtual void SetCurrentMissionPtr(void* pMis) { m_pCurrentMission = (IMission*)pMis; }
	void _cdecl Console_LiveInfo(const ConsoleStack &params);

protected:
	bool m_bServiceIsReady;
	ILiveManager* m_pLiveManager;
	bool m_bDemoMode;
	ILiveManager::ReadyState m_oldReadyState;
	IFileService* m_pFileService;
	IRender* m_pRenderService;

	IMission* m_pCurrentMission;

	// =======================================================
	// работа с ачивментами
	// -------------------------------------------------------
	bool m_bLoadAchievement;									// в текущий момент идет чтение ачивментов из профайла
	bool m_bWriteAchievement;									// в текущий момент идет запись ачивментов в профайл
	array<ILiveManager::UserAchievement> m_aAddAchievements;	// список ачивментов готовых для записи
	array<AchievementDescribe> m_aAchievements;					// ачивменты с описаниями
	array<reached_list> m_aRichAchievement;						// список достигнутых ачивментов для вывода
	float m_fRichOutTime;										// время показа достигнутого ачивмента
	// -------------------------------------------------------

	// =======================================================
	// работа с сейвом
	// -------------------------------------------------------
	bool m_bIsSaveData;											// в текущий момент идет запись сейва
	dword m_dwHaveWaitingData;									// имеются данные которые ждут записи (бит 0-для профайла, бит 1-для опций)
	array<byte> m_aProfileData;									// данные профайла ожидающие записи
	array<byte> m_aOptionData;									// данные опций ожидающие записи
	// -------------------------------------------------------

	// =======================================================
	// работа с профайлом
	// -------------------------------------------------------
	bool m_bProfileSettingsIsReady;
	GP::SettingsData m_ProfileSettings;
	// -------------------------------------------------------

	// дебажные данные
	// -------------------------------------------------------
	bool m_bShowDebugInfo;
	bool m_bInitConsole;
	IConsole * m_pConsole;

	// текущий контекст игры
	dword m_dwCurContext;

	// протект дата
	static const int nProtectDataQnt = 5;
	dword m_dwProtData[nProtectDataQnt];

private:
	void SetDataFromProfile();

	void SetAchievementData(AchievementDescribe& ach, dword liveID, int maxCount, dword flags);

	void DrawRichedAchievement(float fDeltaTime);

	void DrawDebugAchievement();

	IRender* GetRender();

	void AchievementsUpdateRead();
	void AchievementsUpdateWrite();
	void AchievementsUpdateReset();

	void Achievement_Reach(dword dwPlayerIndex, dword dwAchievementID, dword dwAchievFlags);
	void WriteAchievements();

	bool SaveProfileAndOption(array<byte> & aProfile, array<byte> & aOption);
	bool GetProfileAndOption(byte* pData, dword dwDataSize, array<byte> & aProfile, array<byte> * paOption);
	bool PrepairWaitingSaveData(bool bProfile,bool bOption);

	dword GetContextDef(const char* pcContext);
};

__forceinline dword LiveService::GetContextDef(const char* pcContext)
{
	if(!pcContext) return CONTEXT_PRESENCE_PRESENCE_MENU;
	//if( pcContext[0]=='g' || pcContext[0]=='G' ) return CONTEXT_PRESENCE_PRESENCE_GAMEPLAY;
	//if( pcContext[0]=='m' || pcContext[0]=='M')
	//	if( pcContext[1]=='u' || pcContext[1]=='U' ) return CONTEXT_PRESENCE_PRESENCE_MULTIPLAYER;

	// Сutscene ?
	if( pcContext[0]=='c' || pcContext[0]=='C' ) return CONTEXT_PRESENCE_PRESENCE_CUTSCENE;
	if( pcContext[0]=='m' || pcContext[0]=='M' )
	{
		// Multiplayer ?
		if( pcContext[1]=='u' || pcContext[1]=='U' ) return CONTEXT_PRESENCE_PRESENCE_MULTIPLAYER;
		// Mission ?
		if( (pcContext[1]=='i' || pcContext[1]=='i') && pcContext[2] && pcContext[3] && pcContext[4] && pcContext[5] && pcContext[6] )
		{
			// Mission 1 or 15 ?
			if( pcContext[7]=='1' )
				if( pcContext[8]=='5' )
					return CONTEXT_PRESENCE_PRESENCE_MISSION15;
				else
					return CONTEXT_PRESENCE_PRESENCE_MISSION1;
			if( pcContext[7]=='2' )
				return CONTEXT_PRESENCE_PRESENCE_MISSION2;
			if( pcContext[7]=='3' )
				return CONTEXT_PRESENCE_PRESENCE_MISSION3;
			if( pcContext[7]=='5' )
				return CONTEXT_PRESENCE_PRESENCE_MISSION5;
			if( pcContext[7]=='6' )
				return CONTEXT_PRESENCE_PRESENCE_MISSION6;
			if( pcContext[7]=='7' )
				return CONTEXT_PRESENCE_PRESENCE_MISSION7;
		}
	}

	return CONTEXT_PRESENCE_PRESENCE_MENU;
}

#endif
