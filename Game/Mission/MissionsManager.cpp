//============================================================================================
// Spirenkov Maxim, 2006
//============================================================================================
// MissionsManager
//============================================================================================


#include "MissionsManager.h"
#include "Mission.h"


CREATE_CLASS(MissionsManager)


MissionsManager * MissionsManager::manager = null;


MissionsManager::MissionsManager() : missions(_FL_),
										executeStack(_FL_),
										commands(_FL_)
{
	currentMission = null;
	commandsCount = 0;
	commands.AddElements(32);
	isTraceCommandsLog = false;
	waitPrecache = null;
	render = null;
}

MissionsManager::~MissionsManager()
{
	if(manager == this)
	{
		manager = null;
	}
	RELEASE(waitPrecache)
}

//Загрузить миссию
void MissionsManager::LoadMission(const char * name)
{
	SingleClassThread
	AddCommand(cmd_load, name);
}

//Загружена ли миссия
bool MissionsManager::IsLoadMission(const char * name)
{
	SingleClassThread
	for(long i = 0; i < missions; i++)
	{
		if(missions[i].name == name)
		{
			return missions[i].pointer->IsLoadDone();
		}
	}
	for(long i = 0; i < commands; i++)
	{
		if(commands[i].id == cmd_load)
		{
			if(commands[i].name == name)
			{
				return false;
			}
		}
	}
	api->Trace("MissionsManager error: Mission \"%s\" no finded!", name);
	return false;
}

//Выгрузить миссию
void MissionsManager::UnloadMission(const char * name)
{
	SingleClassThread
	AddCommand(cmd_unload, name);
}

//Залочить миссию, запрещая выгрузку
void MissionsManager::LockMission(const char * name)
{
	SingleClassThread
	long i = -1;
	if(GetMission(name, &i))
	{
		missions[i].isLock = true;
	}else{
		api->Trace("MissionsManager error: can't lock mission \"%s\" because it not found!", name);
	}
}

//Разлочить миссию, разрешая выгрузку
void MissionsManager::UnLockMission(const char * name)
{
	SingleClassThread
	long i = -1;
	if(GetMission(name, &i))
	{
		missions[i].isLock = false;
	}else{
		api->Trace("MissionsManager error: can't unlock mission \"%s\" because it not found!", name);
	}
}

//Выгрузить все миссии
void MissionsManager::UnloadAllMissions(bool unlockAll)
{
	SingleClassThread
	if(unlockAll)
	{
		for(long i = 0; i < missions; i++)
		{
			missions[i].isLock = false;
		}
	}
	AddCommand(cmd_unload_all, "");
}

//Запустить миссию
void MissionsManager::StartMission(const char * name, bool isExclusive)
{
	SingleClassThread
	AddCommand(isExclusive ? cmd_start_exclusive : cmd_start, name);
}

//Остановить исполнение текущей миссии
void MissionsManager::StopMission(const char * name)
{
	SingleClassThread
	AddCommand(cmd_stop, name);
}

//Перезапустить все объекты миссии
void MissionsManager::Restart(const char * name)
{
	SingleClassThread
	AddCommand(cmd_restart, name);
}

//Перезапустить все объекты миссии немедленно
void MissionsManager::RestartAll()
{
	CmdRestartMission(null);
}

//Узнать активна ли текущая миссия
bool MissionsManager::IsActive(IMission * mis)
{
	return (currentMission == mis);
}


//Создать миссию
Mission * MissionsManager::SysCreateMission(const char * name)
{
	Mission * mis = (Mission *)api->CreateObject("Mission");
	if(!mis) return null;
	float cnt = 0.0f;
	if(!mis->CreateMission(name, 100.0f, cnt))
	{
		delete mis;
		mis = null;
	}
	return mis;
}

//Создать миссию
void MissionsManager::SysDeleteMission(Mission * mis)
{
	delete mis;
}


//Исполнение команд
void MissionsManager::ExecuteCommands()
{
	SingleClassThread
	//Для начала обрабатываем команды
	Command * cmd = commands.GetBuffer();
	//Обрабатываем запросы на удаление
	bool skipDelete = false;
	for(long i = 0; i < commandsCount; i++)
	{
		if(cmd[i].id == cmd_unload_all)
		{
			CmdUnloadAllMissions();
			skipDelete = true;
		}
	}
	if(!skipDelete)
	{
		for(long i = 0; i < commandsCount; i++)
		{
			if(cmd[i].id == cmd_unload)
			{
				CmdUnloadMission(cmd[i].name);
			}
		}
	}
	//Обрабатываем запросы на рестарты
	for(long i = 0; i < commandsCount; i++)
	{
		if(cmd[i].id == cmd_restart)
		{
			CmdRestartMission(cmd[i].name);
		}
	}
	//Обрабатываем запросы на загрузку
	dword missionsForLoad = 0;
	const char * reloadName = null;
	for(long i = 0; i < commandsCount; i++)
	{
		if(cmd[i].id == cmd_load)
		{
			if(!GetMission(cmd[i].name))
			{
				missionsForLoad++;	
				if(!reloadName) reloadName = cmd[i].name;
			}
		}
	}
	if(missionsForLoad > 0)
	{
		render->EnableLoadingScreen(true, reloadName);
		float percentsPerMission = missionsForLoad > 0 ? 100.0f/missionsForLoad : 100.0f;
		float percentsCounter = 0.0f;
		for(long i = 0; i < commandsCount; i++)
		{
			if(cmd[i].id == cmd_load)
			{		
				CmdLoadMission(cmd[i].name, percentsPerMission, percentsCounter);
			}
		}
		render->EnableLoadingScreen(false);
	}
	//Обрабатываем запросы на остановку миссии
	for(long i = 0; i < commandsCount; i++)
	{
		if(cmd[i].id == cmd_stop)
		{
			CmdStopMission(cmd[i].name);
		}
	}
	//Обрабатываем запросы на запуск миссии
	for(long i = 0; i < commandsCount; i++)
	{
		if(cmd[i].id == cmd_start)
		{
			CmdStartMission(cmd[i].name, false);
		}else
		if(cmd[i].id == cmd_start_exclusive)
		{
			CmdStartMission(cmd[i].name, true);
		}
	}
	commandsCount = 0;
}

//Инициализация
bool MissionsManager::Init()
{
	if(manager)
	{
		api->Trace("Attempt repeatable load missions manager");
		return false;
	}
	manager = this;
	render = (IRender *)api->GetService("DX9Render");
	Assert(render);
	render->EnableLoadingScreen(true);
	//Пока включим группу
	IControlsService * ctrl = (IControlsService *)api->GetService("ControlsService");
	Assert(ctrl);
	ctrl->EnableControlGroup("mission", true); 
	//Ожидание прекэширования
	api->SetObjectExecution(this, "mission", 0x1001, &MissionsManager::WaitPrecache);
	waitPrecache = api->Storage().GetItemLong("system.WaitPrecache", _FL_);
	return true;
}

//Ожидание прекэширования
void __fastcall MissionsManager::WaitPrecache(float dltTime)
{
	if(api->Storage().GetLong("system.WaitPrecache", 0))
	{
		return;
	}
	api->DelObjectExecution(this, &MissionsManager::WaitPrecache);
	//Будем исполняться
	api->SetObjectExecution(this, "mission", 0x1001, &MissionsManager::Update);
	MissionsManagerService * srv = (MissionsManagerService *)api->GetService("MissionsManagerService");
	Assert(srv);
	srv->Activate();	
	//Запускаем миссию
	IFileService * fs = (IFileService *)api->GetService("FileService");
	Assert(fs);
	IIniFile * ini = fs->SystemIni();
	if(ini)
	{
		//Режим отладочных сообщений		
		isTraceCommandsLog = ini->GetLong("Mission", "Debug", 0) != 0;
		//Прогружаем всё необходимые миссии
		array<string> missionsForLoad(_FL_, 256);
		ini->GetStrings("Mission", "load", missionsForLoad);
		string startMission = ini->GetString("Mission", "start", "");
		float percentsPerMission = missionsForLoad.Size() > 0 ? 50.0f/missionsForLoad.Size() : 50.0f;
		float percentsCounter = 50.0f;
		for(dword i = 0; i < missionsForLoad.Size(); i++)
		{
			CmdLoadMission(missionsForLoad[i].c_str(), percentsPerMission, percentsCounter);
		}
		CmdStartMission(startMission.c_str(), true);
	}	
	render->EnableLoadingScreen(false);
}

//Исполнение миссий
void __fastcall MissionsManager::Update(float dltTime)
{
	if(executeStack)
	{
		//Текущие состояние игры
		if(!api->IsActive())
		{
			dltTime = 0.0f;
		}
		//Обрабатываем переключение миссий
		MissionExecuter & topme = executeStack[executeStack - 1];
		if(currentMission != topme.pointer && topme.pointer->IsLoadDone())
		{
			if(currentMission)
			{
				if(isTraceCommandsLog)
				{
					api->Trace("MissionsManager message: Process deactivate previous mission (%s) is start", currentMission->GetMissionName());
				}
				currentMission->MissionPause(true);
				if(isTraceCommandsLog)
				{
					api->Trace("MissionsManager message: Process deactivate previous mission (%s) is done", currentMission->GetMissionName());
				}
			}
			currentMission = executeStack[executeStack - 1].pointer;
			Assert(currentMission);
			if(isTraceCommandsLog)
			{
				api->Trace("MissionsManager message: Process activate new mission (%s) is start", currentMission->GetMissionName());
			}
			currentMission->MissionPause(false);
			if(isTraceCommandsLog)
			{
				api->Trace("MissionsManager message: Process activate new mission (%s) is done", currentMission->GetMissionName());
			}
		}		
		//Исполняем миссии
		for(long i = 0; i < executeStack; i++)
		{
			MissionExecuter & me = executeStack[i];
			if(!me.pointer->IsLoadDone())
			{
				continue;
			}
			if(i + 1 < executeStack)
			{
				if(!me.isStop)
				{
					me.pointer->FrameUpdate(0.0f);
				}			
			}else{
				me.pointer->FrameUpdate(dltTime);
			}		
		}
	}else{
		currentMission = null;
	}
}

//Загрузить миссию
void MissionsManager::CmdLoadMission(const char * name, float percents, float & counter)
{
	if(isTraceCommandsLog)
	{
		api->Trace("MissionsManager message: CmdLoadMission(%s)", name);
	}
	if(!name) name = "";
	//Ищим среди загруженых
	Mission * pointer = GetMission(name);
	if(pointer)
	{
		if(isTraceCommandsLog)
		{
			api->Trace("MissionsManager warning: skip CmdLoadMission(%s), mission already loaded", name);
		}
		return;
	}
	//Создаём и загружаем миссию
	pointer = (Mission *)api->CreateObject("Mission");
	Assert(pointer);
	api->DelObjectExecution(pointer, &Mission::FrameUpdate);
	if(!pointer->CreateMission(name, percents, counter))
	{
		delete pointer;
		api->Trace("MissionsManager error: Mission \"%s\" can't load!", name);
		return;
	}
	MissionDesc & desc = missions[missions.Add()];
	desc.pointer = pointer;
	desc.name = name;
	desc.isLock = false;
	//Повесим агента, следящего за удалением миссии
	MOSafePointer agentPtr;
	static const ConstString objName("__MissionsManagerDeleteAgent__");
	desc.pointer->CreateObject(agentPtr, "MissionsManagerDeleteAgent", objName, true);
	desc.pointer->MissionPause(true);
}

//Выгрузить миссию
void MissionsManager::CmdUnloadMission(const char * name)
{
	if(isTraceCommandsLog)
	{
		api->Trace("MissionsManager message: CmdUnloadMission(%s)", name);
	}
	long i = -1;
	Mission * mis = GetMission(name, &i);
	if(mis)
	{
		if(!missions[i].isLock)
		{
			RemoveMissionFromExecute(missions[i].pointer);
			mis->DeleteMission();
		}else{
			if(isTraceCommandsLog)
			{
				api->Trace("MissionsManager warning: can't unload mission (%s) because it locked", name);
			}
		}
	}else{
		api->Trace("MissionsManager error: can't unload mission \"%s\" because it not found!", name);
	}
}

//Выгрузить все миссии
void MissionsManager::CmdUnloadAllMissions()
{
	if(isTraceCommandsLog)
	{
		api->Trace("MissionsManager message: CmdUnloadAllMissions");
	}
	for(long i = 0; i < missions; i++)
	{
		if(!missions[i].isLock)
		{
			RemoveMissionFromExecute(missions[i].pointer);
			missions[i].pointer->DeleteMission();
		}
	}
}

//Запустить миссию
void MissionsManager::CmdStartMission(const char * name, bool isExclusive)
{
	if(isTraceCommandsLog)
	{
		api->Trace("MissionsManager message: CmdStartMission(%s, %s)", name, isExclusive ? "exclusive" : "no exclusive");
	}
	//Получаем поинтер на миссию
	Mission * mis = GetMission(name);
	if(!mis)
	{
		api->Trace("MissionsManager error: can't start mission \"%s\" because it not found!", name);
		return;
	}
	//Удаляем миссию со стека
	if(RemoveMissionFromExecute(mis))
	{
		if(isTraceCommandsLog)
		{
			api->Trace("MissionsManager warning: mission (%s) already started, remove it from execute stack for restart", name);
		}
	}
	//Выставляем режим
	for(long i = 0; i < executeStack; i++)
	{
		executeStack[i].isStop = isExclusive;
	}
	//Помещаем на стек новую миссию
	MissionExecuter & me = executeStack[executeStack.Add()];
	me.pointer = mis;
	me.isStop = false;
}

//Остановить исполнение текущей миссии
void MissionsManager::CmdStopMission(const char * name)
{
	if(isTraceCommandsLog)
	{
		api->Trace("MissionsManager message: CmdStopMission(%s)", name);
	}
	//Получаем поинтер на миссию
	Mission * mis = GetMission(name);
	if(!mis)
	{
		api->Trace("MissionsManager error: can't stop mission \"%s\" because it not found!", name);
		return;
	}
	//Удалим миссию со стека
	RemoveMissionFromExecute(mis);
}

//Отметить что миссия удаляется
void MissionsManager::CmdDeleteMission(IMission & mis)
{
	if(isTraceCommandsLog)
	{
		api->Trace("MissionsManager message: CmdDeleteMission(%s)", mis.GetMissionName());
	}
	//Удалим миссию со стека
	RemoveMissionFromExecute(&mis);
	//Удалим миссию из списка миссий
	for(long i = 0; i < missions; i++)
	{
		if(missions[i].pointer == &mis)
		{
			missions.DelIndex(i);
			i--;
		}
	}
}

//Перезапустить миссию(и)
void MissionsManager::CmdRestartMission(const char * name)
{
	for(dword i = 0; i < missions.Size(); i++)
	{
		//Фильтр миссии
		if(name)
		{
			if(missions[i].name != name)
			{
				continue;
			}
		}
		//Перезапускаем объекты
		Mission * miss = missions[i].pointer;
		miss->RestartAllObjects();
	}	
}

//Найти миссию по имени
__forceinline Mission * MissionsManager::GetMission(const char * name, long * index)
{
	for(long i = 0; i < missions; i++)
	{
		if(missions[i].name == name)
		{
			if(index) *index = i;
			return missions[i].pointer;
		}
	}
	return null;
}

//Добавить команду
__forceinline void MissionsManager::AddCommand(CommandId id, const char * name)
{
	if(commandsCount >= commands)
	{
		commands.AddElements(16);		
	}
	Command & cmd = commands[commandsCount++];
	cmd.id = id;
	cmd.name = name;
}

//Удалить миссию из стека исполнения
bool MissionsManager::RemoveMissionFromExecute(IMission * mis)
{
	//Удаляем миссию со стека и выставляем режимы
	for(long i = 0; i < executeStack; i++)
	{
		if(executeStack[i].pointer == mis)
		{
			executeStack.DelIndex(i);
			if(currentMission == mis)
			{
				((Mission *)mis)->MissionPause(true);
				currentMission = null;
				return true;
			}			
			i--;
		}
	}
	return false;
}


MissionsManagerDeleteAgent::~MissionsManagerDeleteAgent()
{
	MissionsManager * mng = MissionsManager::Ptr();
	if(mng)
	{
		mng->CmdDeleteMission(Mission());
	}
}

MOP_BEGINLIST(MissionsManagerDeleteAgent, "", '0.00', 0)
MOP_ENDLIST(MissionsManagerDeleteAgent)


CREATE_SERVICE(MissionsManagerService, 100)

MissionsManagerService::MissionsManagerService()
{
}

MissionsManagerService::~MissionsManagerService()
{
}

//Запустить исполнение сервиса
void MissionsManagerService::Activate()
{
	api->SetEndFrameLevel(this, 0x7fffff00);
}

//Исполнение в конце кадра
void MissionsManagerService::EndFrame(float dltTime)
{
	if(MissionsManager::Ptr())
	{
		MissionsManager::Ptr()->ExecuteCommands();
	}
}


#ifndef NO_TOOLS
#ifndef _XBOX

//Создать миссию
IMission * MissionsManagerService::CreateMission(const char * name)
{
	return MissionsManager::SysCreateMission(name);
}

//Удалить миссию
void MissionsManagerService::DeleteMission(IMission * mis)
{
	MissionsManager::SysDeleteMission((Mission *)mis);
}

//Удалить мэнеджер
void MissionsManagerService::DeleteManager()
{
	if(MissionsManager::Ptr())
	{
		delete MissionsManager::Ptr();
	}
}

#endif
#endif
