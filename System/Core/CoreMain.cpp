

#include "..\..\common_h\coremanager.h"
#include "core.h"
#include "MemoryManager.h"


//-------------------------------------------------------------------------------------------------

ICore * api = null;
StormEngine_Declarator * StormEngine_Declarator::first = null;
CoreSystemLogReciver * reciver = null;
extern MemoryManager memoryManager;

//-------------------------------------------------------------------------------------------------


__forceinline void * _cdecl operator new(size_t size, const char * fileName, long fileLine, const char * coreCreate)
{
	Assert(strcmp(coreCreate, "CoreCreate") == 0);
	api = (ICore *)memoryManager.Reallocate(null, (dword)size, fileName, fileLine);
	return api;
}

__forceinline void _cdecl operator delete(void * ptr, const char * fileName, long fileLine, const char * coreCreate)
{
	Assert(false);
	return memoryManager.Free(ptr, fileName, fileLine);
}


class CoreManagerImp : public CoreManager
{
public:


	virtual ~CoreManagerImp()
	{
	};

	//Создать
	virtual void CreateCore(CoreSystemLogReciver * _reciver, const char * coreModuleNameForSkip)
	{
		if(!api)
		{
			reciver = _reciver;
			new(_FL_, "CoreCreate") Core(coreModuleNameForSkip);
			Assert(api);
		}
	};

	//Удалить
	virtual void DeleteCore()
	{
		if(api)
		{
			//Удаляем ядро
			((Core *)api)->~Core();
			memoryManager.Free(api, _FL_);
			api = null;
			//Освобождаем оставшиюся память и выводим сообщения
			memoryManager.ReleaseMemory(true);
		}
	};

	//Загрузить модули быстрого старта
	virtual bool LoadQuickStartModules(const char * quickModules)
	{
		if(api)
		{
			return ((Core *)api)->LoadQuickStartModules(quickModules);
		}
		return false;
	}

	//Загрузить модули невошедшие в список быстрого старта
	virtual void LoadModules()
	{
		if(api)
		{
			((Core *)api)->LoadModules();
		}
	}

	//Обновить
	virtual void FrameUpdate()
	{
		if(api)
		{
			((Core *)api)->Update();
		}
	};

	//Получить указатель
	virtual ICore * GetCore()
	{
		return api;
	};

	//Активно ли ядро
	virtual void SetActive(bool isActive)
	{
		((Core *)api)->SetActive(isActive);
	}

	//Завершить работу приложения
	virtual bool IsExit()
	{
		if(api)
		{
			return ((Core *)api)->IsExit();
		}
		return true;
	}

	//Создать зарегистрированные сервисы (повторно созданные не создаёт)
	virtual bool CreateAllServices()
	{
		if(!api) return false;		
		return ((Core *)api)->CreateAllServices();
	}

	//Создать сервис
	virtual bool CreateCoreService(const char * service_name)
	{
		
		return api->GetService(service_name) != null;
	}

	//Создать объект
	virtual bool CreateObject(const char * className)
	{
		if(!api) return false;
		return api->CreateObject(className) != null;
	}


	//Установить числовую целочисленную переменную
	virtual void SetVarInt(const char * name, long v)
	{
		if(api) api->Storage().SetLong(name, v);
	}

	//Получить числовую целочисленную переменную
	virtual long GetVarInt(const char * name, long def, bool * res)
	{
		if(!api)
		{
			if(res) *res = false;
			return def;
		}
		return api->Storage().GetLong(name, def, res);
	}

	//Установить числовую переменную
	virtual void SetVarDouble(const char * name, double v)
	{
		if(api) api->Storage().SetFloat(name, (float)v);
	}

	//Получить числовую переменную
	virtual double GetVarDouble(const char * name, double def, bool * res)
	{
		if(!api)
		{
			if(res) *res = false;
			return def;
		}
		return api->Storage().GetFloat(name, (float)def, res);
	}

	//Утановить строковую переменную
	virtual void SetVarString(const char * name, const char * v)
	{
		if(api) api->Storage().SetString(name, v);
	}

	//Получить строковую переменную
	virtual const char * GetVarString(const char * name, const char * def, bool * res)
	{
		if(!api)
		{
			if(res) *res = false;
			return def;
		}
		return api->Storage().GetString(name, def, res);
	}

	CoreState TranslateState(CoreManagerCoreState state)
	{
		switch(state)
		{
		case cmcorestate_trace: return corestate_trace;
		case cmcorestate_debugkeys: return corestate_debugkeys;
		case cmcorestate_memcheck: return corestate_memcheck;
		case cmcorestate_panicmemcheck: return corestate_panicmemcheck;
		case cmcorestate_memorypools: return corestate_mempools;
		case cmcorestate_mtmanual: return corestate_mtmanual;
		case cmcorestate_lorestimer: return corestate_lorestimer;
		case cmcorestate_fixedfps: return corestate_fixedfps;
		}
		return corestate_none;
	}

	//Включить-выключить стейт ядра
	virtual void SetState(CoreManagerCoreState state, long value)
	{
		if(!api) return;
		api->ExecuteCoreCommand(CoreCommand_StateOp(ccid_setstate, TranslateState(state), value));
	}

	//Получить стейт ядра
	virtual long GetState(CoreManagerCoreState state)
	{
		if(!api) return false;
		CoreCommand_StateOp cmd(ccid_getstate, TranslateState(state), false);
		api->ExecuteCoreCommand(cmd);
		return cmd.value;
	}

	//Добавить сообщение в буфер сообщений ядра (при количестве параметров == 0 сообщение пропускается)
	virtual void AddSystemMessage(const char * id, const void * data, unsigned long size)
	{
		if(api)
		{		
			((Core *)api)->AddSystemMessage(id, (const unsigned char *)data, size);
		}
	}

	//Вывести сообщение в системный лог
	virtual void _cdecl Trace(const char * format, ...)
	{
		CoreLogOutData(format, (&format) + 1);
	}
};

//------------------------------------------------------------------------------------------------
//Вывод в лог
//------------------------------------------------------------------------------------------------

char coreLogOutDataBuffer[16384];
bool core_isEnableTrace = true;				//Разрешён ли вывод в лог

void CoreLogOutData(const char * format, const void * data)
{
	if(api) ((Core *)api)->SetWatchDog();
	if(!reciver || !core_isEnableTrace) return;
	crt_vsnprintf(coreLogOutDataBuffer, ARRSIZE(coreLogOutDataBuffer) - 4, format, (va_list)data);
	coreLogOutDataBuffer[ARRSIZE(coreLogOutDataBuffer) - 4] = 0;
	crt_strcat(coreLogOutDataBuffer, 16383, "\r\n");
	reciver->Out(coreLogOutDataBuffer);
}


//Мэнеджер ядра
CoreManagerImp coreManager;


//------------------------------------------------------------------------------------------------
//Экспорк в dll
//------------------------------------------------------------------------------------------------

#ifdef _USRDLL

extern "C"
{
	void __declspec(dllexport) _cdecl StormEngine_GetCoreManager(CoreManager * & pointerToCoreManager)
	{
		pointerToCoreManager = &coreManager;
	}
};

#else

CoreManager * _cdecl StormEngine_GetCoreManager()
{
	return &coreManager;
}

#endif

