#ifdef _XBOX

#ifndef _XBOX_LIVEMANAGER_H_
#define _XBOX_LIVEMANAGER_H_

#include "..\ILiveManager.h"

#define SignedUser_NotSigned	0xFFFFFFFF
#define ACHIEVEMENT_COUNT		50
#define USER_QUANTITY			4

class IControlsService;

class XboxLiveManager : public ILiveManager
{
	enum SaveDataState
	{
		SaveData_Loading,	// идет процесс чтения данных
		SaveData_Saving,	// идет процесс записи данных
		SaveData_Present,	// данные присутсвуют
		SaveData_None		// данные отсутсвуют
	};

	struct UserDescr
	{
		bool bControllerPluged;	// подключен ли контроллер
		bool bUserSigned;		// заассигнен ли пользователь
		bool bUserSignChanged;	// изменен ли пользователь
		XUID xuidUser;			// ИД пользователя в XBox Live
		bool bCheckable;		// флаг того что пользователь влияет на состояние сервиса
								// т.е. если он отключен или переназначен, то ругаемся
	};

public:
	XboxLiveManager();
	virtual ~XboxLiveManager();

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

	virtual dword GetSignedInUser() {return m_dwSignedInUserIndex;}
	virtual void ShowSignInUI();
	virtual void SetCheckedUsers(long nUserQuantity);
	virtual dword GetCheckedUsers() {return m_dwCheckableQuantity;}
	virtual bool IsChangedProfile();

	virtual bool ServiceIsReady();

	virtual ReadyState GetReadyState();

	virtual void ChangeContext(dword dwContext);

private:
	IControlsService* m_pControls;	// сервис котролов (у него мы узнаем о подключенных пользователях)
	dword m_dwSignedInUserIndex;	// индекс подключенного пользователя
	HANDLE m_hNotification;			// нотификация (отключение устройства хранения, переассигн пользователей и т.п.)
	bool m_bXShow;					// флаг работы интерфейса XBox
	dword m_dwCheckableQuantity;	// количество пользователей за которыми мы следим
	XUID m_xuidProfileUser;			// УИД пользователя который является хозяином профайла
	dword m_dwCurrentContext;		// текущий контекст исполнения

	//==========================================
	// Gamepad
	//------------------------------------------
	UserDescr m_UserDescr[USER_QUANTITY];	// состояние пользователя (включен или нет? засигнен или нет)
	//------------------------------------------

	//==========================================
	// save(load) data section
	//------------------------------------------
	XUID m_xuidSaveGameUser;				// УИД пользователя который является хозяином текущего сейва
	byte* m_pSaveData;						// массив данных для записи/чтения
	dword m_dwSaveDataSize;					// размер данных для записи/чтения
	SaveDataState m_SaveDataState;			// текущее состояние (запись / чтение / присутствие / отсутствие данных)
	XOVERLAPPED m_xovSaveLoad;				// объект синхронизации отложенных вызовов записи/чтения данных
	bool m_bUserIsCreator;					// флаг того что пользователь является владельцем текущего сейва
	bool m_bSignChanged;					// флаг того что пользователь поменялся
	bool m_bIsSetMasterUser;				// флаг того, что пользователь был назначен
	BYTE m_savebuf[MAX_SAVE_DATA_SIZE];		// буфер для сейва
	XUSER_PROFILE_SETTING m_aBlocksData[3];	// буфер для чтения профайл сеттингов
	//------------------------------------------

	//==========================================
	// write(load) achievement data section
	//------------------------------------------
	struct AchievmentDetail
	{
		XACHIEVEMENT_DETAILS * pAch;	// описание ачивментов
		DWORD dwCount;					// количество описанных ачивментов
	};
	AchievmentDetail m_UsrAchDetail[USER_QUANTITY];					// описание ачивментов для пользователя

	XUSER_ACHIEVEMENT m_AchievementsForSave[ACHIEVEMENT_COUNT];		// пары ачивмент+пользователь для записи в профайл
	dword m_dwAchievementForSaveQuantity;							// количество пар для записи
	bool m_bAchievementSaving;										// идет процесс записи ачивментов в профайл
	XOVERLAPPED m_xovAchievement;									// объект для синхронизации отложенной записи ачивментов
	//------------------------------------------

protected:
	void ReleaseCurrentSaveData();
	void GetCurrentSaveData( void* & pData, dword & size );
	void StopSaveLoadData();

	bool UserSignedIsValid();

	void FindSignedUser(bool bResign);

	void NotificationFrame();

	const char* GetLangId(dword dwProfileRegion);

	bool LoadAchievements();
};

#endif
#endif
