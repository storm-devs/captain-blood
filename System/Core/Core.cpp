

#include "core.h"
#include "MemoryManager.h"
#include <time.h>

extern MemoryManager memoryManager;
extern bool core_isEnableTrace;



Core::Core(const char * coreModuleNameForSkip) :	dlls(coreModuleNameForSkip),
													messages(_FL_, 4096),
													messagesInfo(_FL_),
													errors(_FL_)
{
	//Идентификатор основного потока
	threadId = ::GetCurrentThreadId();
	//Определяем режим для поточности
#ifndef _XBOX
	DWORD m1 = 0, m2 = 0;
	GetProcessAffinityMask(GetCurrentProcess(), &m1, &m2);
	for(long m = 1, count = 0; m; m <<= 1)
	{
		if(m1 & m)
		{
			count++;
		}
	}
	if(count <= 1)
	{
		mtmode = mt_none;
	}else
	if(count < 4)
	{
		mtmode = mt_core_2;
	}else
	if(count == 4)
	{
		mtmode = mt_core_4;
	}else{
		mtmode = mt_core_more;
	}
#else
	mtmode = mt_core_more;
//	memoryManager.TraceStatistic(CoreCommand_MemStat(cmemstat_onlytotal));
#endif
#ifndef STOP_DEBUG	
	//Инициализируем флаг watchdog
	memset(watchDogBuffer, 0, sizeof(watchDogBuffer));
	watchDogPtr = (dword *)AlignPtr(watchDogBuffer, 0xf);	
#endif
	//Аксессор
	decls.SetAccessor(&accsessor);
	execution.SetAccessor(&accsessor);
	//Начинаем базу декларированных объектов
	decls.AddDecls(StormEngine_Declarator::GetFirst());
	decls.BuildHashTable();
	//Таймер
	SetState(corestate_lorestimer, false);
	//Флаг выхода
	isExit = false;
	//Различные переменные, сообщающие о параметрах запуска
#ifdef _XBOX
	Storage().SetString("runtime.platform", "X360");
#else
	Storage().SetString("runtime.platform", "WIN32");
#endif
	Storage().SetString("system.core.id", "Storm engine v3.0");
	char tmpbuf[128];
	crt_snprintf(tmpbuf, sizeof(tmpbuf), "%s, %s", __DATE__, __TIME__);
	Storage().SetString("system.core.build", tmpbuf);	
	__time64_t ltime;
	_time64(&ltime);
	struct tm today;
	srand(((long)ltime & 0xfffff000) | rand());
	crt_localtime64(&today, &ltime);
	strftime(tmpbuf, sizeof(tmpbuf), "%d %B %Y, %H:%M:%S", &today);
	Storage().SetString("system.core.starttime", tmpbuf);
	dlls.PutInfoToStorage(Storage());
	dltTime = 0.0f;
	fixedDltTime = 0.01f;
	scaleDltTime = 0.0f;
	timeScale = 1.0f;
	fps = 0.0f;
	fpsTime = 0.0f;
	frames = 0;
	isFixedDltTime = false;
	fixedFPS = 0;
	fixedFPSDltTime = 0.0f;
	for(long i = 0; i < ARRSIZE(errorsEntry); i++)
	{
		errorsEntry[i] = -1;
	}
	isActive = true;
}

Core::~Core()
{
	decls.Release();
	statesBase.ForceRelease();
	dlls.Unload();
}


//Загрузить модули быстрого старта
bool Core::LoadQuickStartModules(const char * quickModules)
{
	if(!dlls.LoadQuickModules(quickModules))
	{
		return false;
	}
	for(long i = 0; i < dlls.GetCount(); i++)
	{
		decls.AddDecls(dlls.GetFirst(i));
	}
	decls.BuildHashTable();
	return true;
}


//Загрузить модули невошедшие в список быстрого старта
void Core::LoadModules()
{
	dlls.LoadModules();
	for(long i = 0; i < dlls.GetCount(); i++)
	{
		decls.AddDecls(dlls.GetFirst(i));
	}
	decls.BuildHashTable();
}

//------------------------------------------------------------------------------------------------
//Main functions
//------------------------------------------------------------------------------------------------

//Создать сервис
Service * Core::GetService(const char * service_name)
{	
	AssertCoreThread
	return (Service *)decls.CreateObject(service_name, false);
}

//Установить уровень исполнения перед началом кадра
void Core::SetStartFrameLevel(Service * s, dword level)
{
	AssertCoreThread
	execution.SetStartFrameLevel(s, level);
}

//Установить уровень исполнения после кадра
void Core::SetEndFrameLevel(Service * s, dword level)
{
	AssertCoreThread
	execution.SetEndFrameLevel(s, level);
}

//Создать зарегестрированный объект
RegObject * Core::CreateObject(const char * className)
{	
	AssertCoreThread
	return decls.CreateObject(className, false);
}

//Установить функцию объекта на исполнение
void Core::SetObjectExecutionFunc(RegObject * obj, const char * group, dword level, ObjectExecution func)
{
	AssertCoreThread
	execution.SetObjectExecution(obj, group, level, func);
}

//Удалить обработчик
void Core::DelObjectExecutionFunc(RegObject * obj, ObjectExecution func)
{
	AssertCoreThread
	execution.DelObjectExecution(obj, func);
}

//Удалить все обработчики данного объекта из группы
void Core::DelObjectExecutions(RegObject * obj, const char * group)
{
	AssertCoreThread
	execution.DelObjectExecutions(obj, group);
}

//Удалить все обработчики данного объекта
void Core::DelObjectExecutions(RegObject * obj)
{
	AssertCoreThread
	execution.DelObjectExecutions(obj);
}

//Установить уровень исполнения группы
void Core::SetGroupLevel(const char * group, dword level)
{
	AssertCoreThread
	execution.SetGroupLevel(group, level);
}

//Получить уровень исполнения группы
dword Core::GetGroupLevel(const char * group)
{
	AssertCoreThread
	return execution.GetGroupLevel(group);
}

//Проверить на валидность указатель объекта
bool Core::IsValidateRegObject(RegObject * obj, long * cacheValue)
{
	//Будет сделано в случае необходимости
	Assert(false);
	return true;
}

//Найти первый объект по имени
RegObject * Core::FindObject(const char * className)
{
	AssertCoreThread
	ObjectsDeclarators::RegList * list = decls.Find(className);
	if(!list) return null;
	list->ToFirst();
	return list->Get();
}

//Найти все объекты по имени
void Core::FindObject(const char * className, array<Object *> & objects)
{
	AssertCoreThread
	objects.Empty();
	ObjectsDeclarators::RegList * list = decls.Find(className);
	if(!list) return;
	list->ToFirst();
	for(RegObject * obj = null; obj = list->Get(); list->Next())
	{
		objects.Add(obj);		
	}
}

//Получить список всех зарегестрированных объектов
void Core::GetRegistryObjectsList(array<string> & objects)
{
	AssertCoreThread
	 decls.GetRegistryObjectsList(objects);
}

//Получить доступ к корневой папке переменных ядра
ICoreStorageFolder & Core::Storage()
{
	return statesBase.Root();
}

//------------------------------------------------------------------------------------------------
//Time functions
//------------------------------------------------------------------------------------------------

//Получить текущий временной промежуток
float Core::GetDeltaTime()
{
	return scaleDltTime;
}

//Получить текущий временной промежуток без учёта скалирования
float Core::GetNoScaleDeltaTime()
{
	return dltTime;
}

//Установить масштаб времени для всех
void Core::SetTimeScale(float scale)
{
	AssertCoreThread
	timeScale = scale;
	if(timeScale < 0.0f) timeScale = 0.0f;
	if(timeScale > 1000000.0f) timeScale = 1000000.0f;
}

//Получить масштаб времени для всех
float Core::GetTimeScale()
{
	return timeScale;
}

//Установить масштаб времени для группы
void Core::SetTimeScale(float scale, const char * group)
{
	AssertCoreThread
	if(timeScale < 0.0f) timeScale = 0.0f;
	if(timeScale > 1000000.0f) timeScale = 1000000.0f;
	execution.SetGroupTimeScale(scale, group);
}

//Получить масштаб времени для группы
float Core::GetTimeScale(const char * group)
{
	AssertCoreThread
	return execution.GetGroupTimeScale(group);
}

//Установить фиксированный временной отрезок
void Core::SetFixedDeltaTime(float dltTime)
{
	AssertCoreThread
	fixedDltTime = dltTime;
}

//Получить фиксированный временной отрезок
float Core::GetFixedDeltaTime()
{
	return fixedDltTime;
}

//Разрешить-запретить фиксированный временной отрезок
void Core::EnableFixedDeltaTime(bool isEnable)
{
	AssertCoreThread
	isFixedDltTime = isEnable;
}

//Фиксированный временной отрезок установлен или нет
bool Core::IsFixedDeltaTime()
{
	return isFixedDltTime;
}

//Получить текущую скорость в кадрах за секунду
float Core::EngineFps()
{
	return fps;
}


//------------------------------------------------------------------------------------------------
//System
//------------------------------------------------------------------------------------------------	

//Дать ядру команду
void Core::ExecuteCoreCommand(CoreCommand & cmd)
{
	static bool isAssertMemCheck = false;
	switch(cmd.commandID)
	{
	case ccid_assert_memcheck:
		
		if(!isAssertMemCheck)
		{
			{
				SingleClassThread
					isAssertMemCheck = true;
			}			
			memoryManager.PanicCheckMemory();
			{
				SingleClassThread
					isAssertMemCheck = false;
			}
		}
		return;
	case ccid_setstate:
		AssertCoreThread
		SetState(((CoreCommand_StateOp &)cmd).state, ((CoreCommand_StateOp &)cmd).value);
		return;
	case ccid_getstate:
		AssertCoreThread
		((CoreCommand_StateOp &)cmd).value = GetState(((CoreCommand_StateOp &)cmd).state);
		return;
	case ccid_memstatistic:
		memoryManager.TraceStatistic((CoreCommand_MemStat &)cmd);
		return;
	case ccid_memcheck:
		memoryManager.PanicCheckMemory();
		return;
	case ccid_getmemstatistic:
		memoryManager.GetStatistic((CoreCommand_GetMemStat &)cmd);
		return;
	case ccid_getmemmanagermemstat:
		memoryManager.GetManagerStat((CoreCommand_GetMemManagerStat &)cmd);
		return;
	}
}

//Вывести сообщение в системном логе
void _cdecl Core::Trace(const char * format,...)
{
	SingleClassThread
	CoreLogOutData(format, (&format) + 1);
}

//Вывести сообщение в системный лог
void Core::TraceData(const char * format, const void * data)
{
	SingleClassThread
	CoreLogOutData(format, data);
}

//Вывести сообщение в системный лог 1 раз
void _cdecl Core::Error(const char * format, ...)
{
	if(!format) return;
	static char buf[256];
	crt_vsnprintf(buf, 255, format, (va_list)((&format) + 1));
	buf[255] = 0;
	dword len = 0;
	dword hash = string::Hash(buf, len);
	{
		SingleClassThread
		long index = hash & ARRSIZE(errorsEntry);
		for(long i = errorsEntry[index]; i >= 0; )
		{
			ErrorRecord & err = errors[i];
			if(err.hash == hash && err.str.Len() == len)
			{
				if(err.str == buf)
				{
					return;
				}
			}
			i = err.next;
		}
		ErrorRecord & err = errors[i = errors.Add()];
		err.next = errorsEntry[index];
		errorsEntry[index] = i;
		err.hash = hash;
		err.str = buf;
	}
	Trace(buf);
}

//Активно ли приложение
bool Core::IsActive()
{
	return isActive;
}

//Завершить работу приложения
void Core::Exit()
{
	isExit = true;
}

//Изменить объём выделеной памяти
void * Core::Reallocate(void * ptr, dword size, const char * fileName, long fileLine)
{
	SingleClassThread
	return memoryManager.Reallocate(ptr, size, fileName, fileLine);
}

//Освободить память
void Core::Free(void * ptr, const char * fileName, long fileLine)
{
	SingleClassThread
	memoryManager.Free(ptr, fileName, fileLine);
}

//Получить состояние нажатости дебажной кнопки
bool Core::sysDebugKeyState(long id)
{
#ifndef STOP_DEBUG
	SingleClassThread
	if(!isDebugKeys || !isActive) return false;
#ifndef _XBOX
	if(id < __system_start_ex_keys)
	{
		if(id == VK_PAUSE)
		{
			return GetAsyncKeyState(id) != 0;
		}
		return GetAsyncKeyState(id) < 0;
	}else
	if(id > __system_start_ex_keys)
	{
		switch(id)
		{
		case num_lock_flag:
			if(GetKeyState(VK_NUMLOCK) & 0x0001) return true;
			break;
		case caps_lock_flag:
			if(GetKeyState(VK_CAPITAL) & 0x0001) return true;
			break;
		case scroll_lock_flag:
			if(GetKeyState(VK_SCROLL) & 0x0001) return true;		
			break;
		}
	}
#else
	if(id >= xb_dpad_up)
	{
		dword flags = 0;
		bool left = false;
		bool right = false;
		//Опрашиваем джойстики
		for(DWORD i = 0; i < XUSER_MAX_COUNT; i++ )
		{
			XINPUT_STATE inputState;
			if(XInputGetState(i, &inputState) != ERROR_SUCCESS) continue;
			flags |= inputState.Gamepad.wButtons;
			left |= (inputState.Gamepad.bLeftTrigger > 128);
			right |= (inputState.Gamepad.bRightTrigger > 128);
		}
		switch(id)
		{
		case xb_dpad_up:
			if(flags & XINPUT_GAMEPAD_DPAD_UP) return true;
			break;
		case xb_dpad_down:
			if(flags & XINPUT_GAMEPAD_DPAD_DOWN) return true;
			break;
		case xb_dpad_left:
			if(flags & XINPUT_GAMEPAD_DPAD_LEFT) return true;
			break;
		case xb_dpad_right:
			if(flags & XINPUT_GAMEPAD_DPAD_RIGHT) return true;
			break;
		case xb_start:
			if(flags & XINPUT_GAMEPAD_START) return true;
			break;
		case xb_back:
			if(flags & XINPUT_GAMEPAD_BACK) return true;
			break;
		case xb_left_thumb:
			if(flags & XINPUT_GAMEPAD_LEFT_THUMB) return true;
			break;
		case xb_right_thumb:
			if(flags & XINPUT_GAMEPAD_RIGHT_THUMB) return true;
			break;
		case xb_left_shoulder:
			if(flags & XINPUT_GAMEPAD_LEFT_SHOULDER) return true;
			break;
		case xb_right_shoulder:
			if(flags & XINPUT_GAMEPAD_RIGHT_SHOULDER) return true;
			break;
		case xb_a:
			if(flags & XINPUT_GAMEPAD_A) return true;
			break;
		case xb_b:
			if(flags & XINPUT_GAMEPAD_B) return true;
			break;
		case xb_x:
			if(flags & XINPUT_GAMEPAD_X) return true;
			break;
		case xb_y:
			if(flags & XINPUT_GAMEPAD_Y) return true;
			break;
		case xb_left_trigger:
			if(left) return true;
			break;
		case xb_right_trigger:
			if(right) return true;
			break;
		}
	}
#endif
#endif
	return false;	
}

//Получить количество системных сообщений, набравшихся за кадр
dword Core::GetSystemMessagesCount()
{
	AssertCoreThread
	return messagesInfo;
}

//Получить системное сообщение
bool Core::GetSystemMessage(dword index, CoreSystemMessage & msg)
{
	AssertCoreThread
	if(index >= (dword)messagesInfo) return false;
	SysMsgInfo & info = messagesInfo[index];
	msg.id = (const char *)&messages[info.name];
	msg.data = (const byte *)&messages[info.data];
	msg.size = info.size;
	return true;
}

//Получить конфигурацию ядер процессора
Core::MultiThreading Core::GetThreadingInfo()
{
	AssertCoreThread
	return mtmode;
}

//Удалить из списков регистрации объект
void Core::sysUnregistryObject(RegObject * obj)
{
	AssertCoreThread
	execution.DelObjectExecutions(obj);
	decls.RemoveObject(obj);
}

//Добавить значение к счётчику, который сбросится с конце кадра
void Core::sysAddPerformanceCounter(const char * name, float value)
{
#ifndef STOP_PROFILES
	AssertCoreThread
	pcnts.AddPerformanceCounter(name, value);
#endif
}

//Установить счётчик, который сбросится с конце кадра
void Core::sysSetPerformanceCounter(const char * name, float value)
{
#ifndef STOP_PROFILES
	AssertCoreThread
	pcnts.SetPerformanceCounter(name, value);
#endif
}

//Получить количество счётчиков
dword Core::sysGetNumberOfPerformanceCounters()
{
#ifndef STOP_PROFILES
	AssertCoreThread
	return pcnts.GetNumberOfPerformanceCounters();
#else
	return 0;
#endif
}

//Получить имя счётчика
const char * Core::sysGetPerformanceName(long index)
{
#ifndef STOP_PROFILES
	AssertCoreThread
	return pcnts.GetPerformanceName(index);
#else
	return null;
#endif
}

//Получить значение счётчика с предыдущего кадра
float Core::sysGetPerformanceCounter(long index)
{
#ifndef STOP_PROFILES
	AssertCoreThread
	return pcnts.GetPerformanceCounter(index);
#else
	return 0.0f;
#endif
}

//Получить значение счётчика с предыдущего кадра
float Core::sysGetPerformanceCounter(const char * name)
{
#ifndef STOP_PROFILES
	AssertCoreThread
	return pcnts.GetPerformanceCounter(name);
#else
	return 0.0f;
#endif
}

//Кадровое обновление
void Core::Update()
{
	SetWatchDog();
	AssertCoreThread
	static const float maxDltTime = (float)Core_MaxDltTime;
	//Обновляем состояние мэнеджера памяти
	bool numLockActive = DebugKeyState(ICore::num_lock_flag);
	bool isEnablePanicCheckMemory = (!fixedFPS && numLockActive);
	if(isMemCheck)
	{
		memoryManager.EnablePanicCheckMemory(isEnablePanicCheckMemory);
	}	
	memoryManager.Update();
	//Получаем временной промежуток
	if(isHiResTimer)
	{
		LARGE_INTEGER time;
		::QueryPerformanceCounter(&time);
		dltTime = (float)((time.QuadPart - hiResTime.QuadPart)*hiTicksToSeconds);
		hiResTime = time;
	}else{
		dword time = ::GetTickCount();
		dltTime = (time - loResTime)*0.001f;
		loResTime = time;
	}
	if(isFixedDltTime)
	{
		dltTime = fixedDltTime;
	}
	if(fixedFPS && numLockActive)
	{
		dltTime = fixedFPSDltTime;
	}
	if(dltTime < 0.0f)
	{
		Trace("Detect hight resolution timer problem (delta time < 0)! Switch timer to low resolition...");
		isHiResTimer = false;
		dltTime = 0.0f;
		loResTime = ::GetTickCount();
	}
	//Считаем FPS
	fpsTime += dltTime;
	frames++;
	if(fpsTime > 1.0f)
	{
		fps = frames/fpsTime;
		fpsTime = 0.0f;
		frames = 0;
	}
	//Если временной промежуток больше допустимого, то ограничиваем его
	if(dltTime > maxDltTime) dltTime = maxDltTime;
	scaleDltTime = dltTime*timeScale;
	if(scaleDltTime > maxDltTime) scaleDltTime = maxDltTime;
	if(!isActive)
	{
		dltTime = 0.0f;
		scaleDltTime = 0.0f;
	}
	//Исполняем сервисы и объекты
	execution.Execute(scaleDltTime);
	//Очищаем буфер сообщений
	messagesInfo.Empty();
	messages.Empty();
	//Выполняем проверку памяти
	if(isMemCheck)
	{
		memoryManager.CheckMemoryStep();
		if(isEnablePanicCheckMemory)
		{
			SingleClassThread
			memoryManager.PanicCheckMemory();
		}
	}
#ifndef STOP_PROFILES
	pcnts.NextFrame();
#endif
}

//Активно ли ядро
void Core::SetActive(bool isActive)
{
	this->isActive = isActive;
}

//Функция сообщающая о завершении работы
bool Core::IsExit()
{
#ifdef _XBOX
#ifndef STOP_DEBUG
	if(DebugKeyState(ICore::xb_right_shoulder, ICore::xb_left_trigger, ICore::xb_right_trigger))
	{
		return true;
	}
#endif
#endif
	return isExit;
}

//Добавить сообщение в буфер сообщений ядра (при количестве параметров == 0 сообщение пропускается)
void Core::AddSystemMessage(const char * id, const unsigned char * data, unsigned long size)
{
	AssertCoreThread
	//Добавляем описание
	SysMsgInfo & mi = messagesInfo[messagesInfo.Add()];
	mi.size = size;
	//Сохраняем идентификатор
	mi.name = messages;
	while(*id) messages.Add(*id++);
	messages.Add(0);
	//Сохраняем данные
	mi.data = messages;
	while(size--)
	{
		messages.Add(*data++);
	}
}


//Включить-выключить стейт ядра
void Core::SetState(CoreState state, long value)
{
	AssertCoreThread
	switch(state)
	{
	case corestate_trace:
		core_isEnableTrace = value != 0;
		break;
	case corestate_debugkeys:
		isDebugKeys = value != 0;
		break;
	case corestate_memcheck:
		isMemCheck = value != 0;
		break;
	case corestate_panicmemcheck:
		memoryManager.SetPanicCheckMemoryMode(value != 0);
		break;
	case corestate_mempools:
		if(value != 0) memoryManager.EnableMemPools();
		break;
	case corestate_mtmanual:
		if(value != 0)
		{
			mtmode = mt_core_more;
		}else{
			mtmode = mt_none;
		}		
		break;
	case corestate_lorestimer:
		isHiResTimer = false;
		loResTime = ::GetTickCount();
		LARGE_INTEGER freq;
		if(::QueryPerformanceFrequency(&freq))
		{
			if(freq.QuadPart != 0)
			{
				if(::QueryPerformanceCounter(&hiResTime))
				{
					hiTicksToSeconds = 1.0/double(freq.QuadPart);
					isHiResTimer = true;
				}
			}
		}
		if(value != 0)
		{
			isHiResTimer = false;
		}
		break;
	case corestate_fixedfps:
		fixedFPS = value;
		if(fixedFPS > 0)
		{
			if(fixedFPS < 10) fixedFPS = 10;
			if(fixedFPS > 100) fixedFPS = 100;
			fixedFPSDltTime = 1.0f/fixedFPS;
		}else{
			fixedFPS = 0;
			fixedFPSDltTime = 0.0f;
		}
		break;
	}
}

//Получить стейт ядра
long Core::GetState(CoreState state)
{
	AssertCoreThread
	switch(state)
	{
	case corestate_trace:
		return core_isEnableTrace;
	case corestate_debugkeys:
		return isDebugKeys;
	case corestate_memcheck:
		return isMemCheck;
	case corestate_panicmemcheck:
		return memoryManager.IsPanicCheckMemoryMode();
	case corestate_mempools:
		return memoryManager.IsEnableMemPools();
	case corestate_mtmanual:
		return mtmode != mt_none;
	case corestate_lorestimer:
		return !isHiResTimer;
	case corestate_fixedfps:
		return fixedFPS;
	}
	return false;
}

//Создать зарегистрированные сервисы
bool Core::CreateAllServices()
{
	AssertCoreThread
	return decls.CreateServices();
}

