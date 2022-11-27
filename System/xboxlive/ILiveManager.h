#ifndef _ILIVEMANAGER_H_
#define _ILIVEMANAGER_H_

#include "..\..\Common_h\core.h"
#include "..\..\Common_h\ILiveService.h"
#include "..\..\XENGINE\UnderJollyRoger.spa.h"

#define SignedUser_NotSignedIn		0xFFFFFFFF

#define XPROFILE_SETTING_MAX_SIZE	1000
#define MAX_SAVE_DATA_SIZE			XPROFILE_SETTING_MAX_SIZE * 3

class ILiveManager
{
public:
	enum AchievementFlag
	{
		af_multiplayer = 1
	};

	struct UserAchievement
	{
		dword userIndex;
		dword achievementID;
		dword achievFlags;
	};

	enum ReadyState
	{
		rs_ready,

		rs_not_signed,
		rs_no_storage,
		rs_unpluged,
		rs_stranger_savedata
	};

	ILiveManager() {}
	virtual ~ILiveManager() {}

	virtual bool Init() = 0;
	virtual void Frame() = 0;

	virtual bool WriteAchievement(UserAchievement* pAchievementsBuffer, dword dwAchievementsQuantity) = 0;
	virtual bool IsWriteProcessAchievemets() = 0;
	virtual bool GetAchievementEarnedState(dword userIndex, dword achievementID, bool & isEarned) = 0;
	virtual bool GetAchievementString(dword achievementID, string & str) = 0;

	virtual bool LoadData( void* & pData, dword & size ) = 0;
	virtual bool SaveData( void* pData, dword size ) = 0;
	virtual bool IsSaveDataProcess() = 0;
	virtual bool LoadDataIsEnable() = 0;

	virtual bool GetProfileSettings(GP::SettingsData & settings) = 0;

	virtual dword GetSignedInUser() = 0;
	virtual void ShowSignInUI() = 0;
	virtual void SetCheckedUsers(long nUserQuantity) = 0;
	virtual dword GetCheckedUsers() = 0;
	virtual bool IsChangedProfile() = 0;

	virtual bool ServiceIsReady() = 0;

	virtual ReadyState GetReadyState() = 0;

	virtual void ChangeContext(dword dwContext) = 0;

	static void SetError(const char* pcErrorText)
	{
		api->Trace("%s", pcErrorText);
	}
};

#endif
