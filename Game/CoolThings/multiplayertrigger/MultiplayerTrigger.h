#ifndef MULTIPLAYERTRIGGER_H
#define MULTIPLAYERTRIGGER_H

#include "..\..\..\Common_h\IPlayerChecker.h"

class PlayerChecker;

class MultiplayerTrigger : public IMultiplayerTrigger
{
	struct PlayerConfig
	{
		array<ConstString> aNameGUI;	// имена виджетов которым надо ставить имя игрока
		const char* joinControl;		// имя контрола для присоединения игрока
		const char* leaveControl;		// имя контрола для отсоединения игрока
		const char* startControl;		// имя контрола для запуска матча
		MissionTrigger plugin;			// эвенты на подключение джойстика
		MissionTrigger plugout;			// эвенты на отключение джойстика
		MissionTrigger join;			// эвенты на присоединение игрока
		MissionTrigger leave;			// эвенты на выход игрока

		bool isPluged;					// джойстик подключен?

		bool checkPlugin;				// ловить включение джойстика
		bool checkPlugout;				// ловить отключение джойстика

		PlayerConfig() : aNameGUI(_FL_) {joinControl=leaveControl=startControl=""; isPluged=checkPlugin=checkPlugout=false;}
	};
	static const unsigned int dwPlayerQnt = 4;
	PlayerChecker* m_pChecker;
	MissionTrigger m_enableStart;
	MissionTrigger m_disableStart;
	MissionTrigger m_launchStart;
	bool m_bEnableStart;

public:
	MultiplayerTrigger();
	virtual ~MultiplayerTrigger();

	MO_IS_FUNCTION(MultiplayerTrigger, MissionObject);

	virtual bool LoadIsOn() {return m_bLoadConfig;}

	//Обработчик команд для объекта
	virtual void Command(const char * id, dword numParams, const char ** params);

	void _cdecl Work(float fDeltaTime, long level);
	bool Create(MOPReader & reader);
	void PostCreate();
	bool EditMode_Update(MOPReader & reader) { ReadMOPs(reader); return true; }

protected:
	void ReadMOPs(MOPReader & reader);
	void ReadPlayerSetting(MOPReader & reader, PlayerConfig& conf);

	void LoadDatabase();
	void SaveDatabase();
	void CheckPlayerdata( long idx, PlayerConfig& conf );

	void UpdateLiveServicePlayerMask();

	void SetPlayerName(long idx);
	void PluginPlayer(long idx);
	void UnplugPlayer(long idx);

	HANDLE m_hNotify;
	bool m_bLoadConfig;
	PlayerConfig m_player[dwPlayerQnt];
};

#endif
