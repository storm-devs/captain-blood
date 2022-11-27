//============================================================================================
// Spirenkov Maxim, 2006
//============================================================================================
// MissionsManager
//============================================================================================

#ifndef _MissionsManager_h_
#define _MissionsManager_h_

#include "..\..\common_h\mission.h"

class Mission;
class MissionsManagerDeleteAgent;

class MissionsManager : public RegObject  
{
	ClassThread

	friend class Mission;
	friend class MissionsManagerDeleteAgent;

	struct MissionDesc
	{
		Mission * pointer;
		string name;
		bool isLock;
	};

	struct MissionExecuter
	{
		Mission * pointer;
		bool isStop;
		bool isBackgroung;
	};

	enum CommandId
	{
		cmd_load,
		cmd_unload,
		cmd_unload_all,
		cmd_start,
		cmd_start_exclusive,
		cmd_stop,
		cmd_restart,
	};

	struct Command
	{
		CommandId id;
		string name;
	};

public:
	MissionsManager();
	virtual ~MissionsManager();

public:
	//Загрузить миссию
	void LoadMission(const char * name);
	//Загружена ли миссия
	bool IsLoadMission(const char * name);
	//Выгрузить миссию
	void UnloadMission(const char * name);
	//Залочить миссию, запрещая выгрузку
	void LockMission(const char * name);
	//Разлочить миссию, разрешая выгрузку
	void UnLockMission(const char * name);
	//Выгрузить все миссии
	void UnloadAllMissions(bool unlockAll = false);
	//Запустить миссию
	void StartMission(const char * name, bool isExclusive);
	//Остановить исполнение текущей миссии
	void StopMission(const char * name);
	//Перезапустить все объекты миссии
	void Restart(const char * name);
	//Перезапустить все объекты миссии немедленно
	void RestartAll();
	//Узнать активна ли текущая миссия
	bool IsActive(IMission * mis);
	//Получить указатель на объект менеджера
	static MissionsManager * Ptr();
	//Создать миссию
	static Mission * SysCreateMission(const char * name);
	//Удалить миссию
	static void SysDeleteMission(Mission * mis);
	//Исполнение команд
	void ExecuteCommands();

private:
	//Инициализация
	virtual bool Init();
	//Ожидание прекэширования
	void __fastcall WaitPrecache(float dltTime);
	//Исполнение миссий
	void __fastcall Update(float dltTime);
	//Загрузить миссию
	void CmdLoadMission(const char * name, float percents, float & counter);
	//Выгрузить миссию
	void CmdUnloadMission(const char * name);
	//Выгрузить все миссии
	void CmdUnloadAllMissions();
	//Запустить миссию
	void CmdStartMission(const char * name, bool isExclusive);
	//Остановить исполнение текущей миссии
	void CmdStopMission(const char * name);
	//Отметить что миссия удаляется
	void CmdDeleteMission(IMission & mis);
	//Перезапустить миссию(и)
	void CmdRestartMission(const char * name);

	//Найти миссию по имени
	Mission * GetMission(const char * name, long * index = null);
	//Добавить команду
	void AddCommand(CommandId id, const char * name);
	//Удалить миссию из стека исполнения
	bool RemoveMissionFromExecute(IMission * mis);

private:
	array<MissionDesc> missions;
	array<MissionExecuter> executeStack;
	array<Command> commands;
	long commandsCount;
	Mission * currentMission;
	ICoreStorageLong * waitPrecache;
	dword isTraceCommandsLog;
	IRender * render;
	static MissionsManager * manager;
};

class MissionsManagerService : public Service
{
public:
	MissionsManagerService();
	virtual ~MissionsManagerService();
	
	//Запустить исполнение сервиса
	virtual void Activate();
	//Исполнение в конце кадра
	virtual void EndFrame(float dltTime);


#ifndef NO_TOOLS
#ifndef _XBOX
	//Создать миссию
	virtual IMission * CreateMission(const char * name);
	//Удалить миссию
	virtual void DeleteMission(IMission * mis);
	//Удалить мэнеджер
	virtual void DeleteManager();
#endif
#endif
};

inline MissionsManager * MissionsManager::Ptr()
{
	return manager;
}


class MissionsManagerDeleteAgent : public MissionObject
{
public:
	virtual ~MissionsManagerDeleteAgent();
};

#endif
