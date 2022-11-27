#ifndef _XBOX
#ifndef _PC_LIVEMANAGER_H_
#define _PC_LIVEMANAGER_H_

#include "..\ILiveManager.h"

//class ISteamUserStats;
//class ISteamRemoteStorage;
class CSteamWraper;

class PCLiveManager : public ILiveManager
{
public:
	PCLiveManager();
	~PCLiveManager();

	virtual bool Init();
	virtual void Frame();

	virtual bool WriteAchievement(ILiveManager::UserAchievement* pAchievementsBuffer, dword dwAchievementsQuantity);
	virtual bool IsWriteProcessAchievemets();
	virtual bool GetAchievementEarnedState(dword userIndex, dword achievementID, bool & isEarned);
	virtual bool GetAchievementString(dword achievementID, string & str);

	virtual bool LoadData( void* & pData, dword & size );
	virtual bool SaveData( void* pData, dword size );
	virtual bool IsSaveDataProcess();
	virtual bool LoadDataIsEnable();

	virtual bool GetProfileSettings(GP::SettingsData & settings);

	virtual dword GetSignedInUser();
	virtual void ShowSignInUI();
	virtual void SetCheckedUsers(long nUserQuantity) {}
	virtual dword GetCheckedUsers() {return 1;}
	virtual bool IsChangedProfile() {return false;}

	virtual bool ServiceIsReady();

	virtual ReadyState GetReadyState();

	virtual void ChangeContext(dword dwContext) {};

private:
	byte* m_pSaveData;
	dword m_dwSaveDataSize;

	bool m_bSignIn;
	bool m_bStrangerUser;
	bool m_bPluginController;

	bool m_bOnlyLocalSaves;

	string g_WorkPath;

	// steam data
//	ISteamUserStats *m_pSteamUserStats;
//	ISteamRemoteStorage *m_pSteamRemoteStorage;
	CSteamWraper* m_pSteamWraper;

	const char* GetProfileName();

	const char* GetAchievementNameByID(dword achievementID);
};

#endif
#endif
