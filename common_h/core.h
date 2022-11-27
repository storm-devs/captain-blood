#ifndef _ICore_h_
#define _ICore_h_

#ifndef NOMINMAX
	#define NOMINMAX
#endif

#include <float.h>
#include <assert.h>

#ifndef _XBOX

#include <windows.h>
#define core_unaligned 
#else
#include <xtl.h>
#define core_unaligned __unaligned
#endif

#include "cfg_switches.h"
#include "d_types.h"
#include "defines.h"
#include "CritSection.h"

//--------------------------------------------

//Дефолтовый уровень исполнения
#define Core_DefaultExecuteLevel		0x10000
//Максимально возможный временной промежуток на кадре
#define Core_MaxDltTime					(1.0f/15.0f)
//Папка в которой лежат dll
#define Core_DllsFolder					"Modules"


//--------------------------------------------

//Отладка
#ifdef STOP_ASSERTS

#define Assert(exp)		(0);
#define Verify(exp)		((int)(exp));
#define AssertCoreThread
#else

void __Storm_Assert__(bool expression, const char * file, long line, const char * str);

#ifndef _XBOX
#define Assert(exp) __Storm_Assert__((exp) != 0, __FILE__,__LINE__,#exp);
#define Verify(exp) Assert(exp)
#else
#include <assert.h>
#define Assert(exp) (void)( (!!(exp)) || (_wassert(_CRT_WIDE(#exp), _CRT_WIDE(__FILE__), __LINE__), 0) );
#define Verify(exp) Assert(exp)
#endif

#define AssertCoreThread Assert(::GetCurrentThreadId() == api->GetThreadId())

#endif

#ifndef _XBOX
#define FixMe	__Storm_Assert__(false, __FILE__,__LINE__, "Fix Me assert!");
#else
#include <assert.h>
#define FixMe	(void)(_wassert(_CRT_WIDE("Fix Me assert!"), _CRT_WIDE(__FILE__), __LINE__)); 
#endif

//--------------------------------------------


#define RESIZE(ptr, objs) resize(ptr, objs, __FILE__, __LINE__)
__forceinline void * _cdecl resize(void * ptr, dword size, const char * fileName, long fileLine);
template<class T> T * _cdecl Resize(T * ptr, dword objects, const char * fileName, long fileLine);


#ifndef _CORE_NO_NEW_

#define NEW new(__FILE__,__LINE__)
#define DEL delete(__FILE__, __LINE__)

void * _cdecl operator new(size_t size);
void _cdecl operator delete(void * ptr);
void * _cdecl operator new(size_t size, const char * fileName, long fileLine);
void _cdecl operator delete(void * ptr, const char * fileName, long fileLine);

#else

#define NEW new
#define DEL delete

#endif

//--------------------------------------------

#include "templates.h"

//--------------------------------------------

class ICore;

//Общий доступ к интерфейсу ядра
extern ICore * api;

//--------------------------------------------

//Базовый объект
class Object
{
public:
	virtual ~Object(){};

};

//Базовый объект, который может регистрироваться
class RegObject : public Object
{
	friend ICore;
	dword registryKey;

public:
	RegObject()
	{
		registryKey = 0;
	};

	virtual ~RegObject()
	{
		sysUnregistryObject();
		registryKey = -1;
	};
	void sysUnregistryObject();

public:
#ifndef _XBOX
	virtual bool VTBL_Mover0(){ return true; };
#ifdef GAME_DEMO
	virtual bool VTBL_Mover007(){ return true; };
#endif
#endif
#ifndef GAME_RUSSIAN
	//Инициализация
	virtual bool Init(){ return true; };
#else
	virtual bool VTBL_Mover1(){ return true; };
	virtual bool VTBL_Mover2(){ return true; };
	//Инициализация
	virtual bool Init(){ return true; };
	virtual bool VTBL_Mover3(){ return true; };
#endif

};

//Описание сервиса
class Service : public RegObject
{
public:
	virtual ~Service(){};

	//Вызываеться перед удалением сервисов
	virtual void PreRelease(){};

public:
#ifndef _XBOX
	virtual void VTBL_Mover4(){ };
#endif
#ifndef GAME_RUSSIAN
	//Исполнение в начале кадра
	virtual void StartFrame(float dltTime){};
	//Исполнение в конце кадра
	virtual void EndFrame(float dltTime){};
#else

	virtual void VTBL_Mover5(float dltTime){};

	//Исполнение в начале кадра
	virtual void StartFrame(float dltTime){};
	//Исполнение в конце кадра
	virtual void EndFrame(float dltTime){};

	virtual void VTBL_Mover6(float dltTime){};
	virtual void VTBL_Mover7(float dltTime){};
#endif


};

//--------------------------------------------

//Функция исполнения
typedef void (_fastcall RegObject::*ObjectExecution)(float dltTime);


//--------------------------------------------
//База данных ядра
//--------------------------------------------

//Базовый тип описывающий переменную ядра
class ICoreStorageItem
{
public:
	enum Type
	{
		t_error = 0,
		t_string = 1,
		t_long = 2,
		t_float = 3,
		t_folder = 4,
		t_forcedword = 0x7fffffff
	};

protected:
	virtual ~ICoreStorageItem(){};
public:
	//Удалить интерфейс на переменную
	virtual void Release() = null;
	//Проверить, существует ли переменная
	virtual bool IsValidate() = null;
};

class ICoreStorageString : public virtual ICoreStorageItem
{
protected:
	virtual ~ICoreStorageString(){};
public:
	//Установить
	virtual void Set(const char * value) = null;
	//Получить
	virtual const char * Get(const char * def = "", bool * isOk = null) = null;
};

class ICoreStorageLong : public virtual ICoreStorageItem
{
protected:
	virtual ~ICoreStorageLong(){};
public:
	//Установить
	virtual void Set(long value) = null;
	//Получить
	virtual long Get(long def = 0, bool * isOk = null) = null;
};

class ICoreStorageFloat : public virtual ICoreStorageItem
{
protected:
	virtual ~ICoreStorageFloat(){};
public:
	//Установить
	virtual void Set(float value) = null;
	//Получить
	virtual float Get(float def = 0, bool * isOk = null) = null;
};


class ICoreStorageFolder : public virtual ICoreStorageItem
{
protected:
	virtual ~ICoreStorageFolder(){};
public:
	//Установить элементу массива строковое поле
	virtual bool SetString(const char * name, const char * value) = null;
	//Получить строку
	virtual const char * GetString(const char * name, const char * def = "", bool * isOk = null) = null;
	//Получить интерфейс на переменную типа string, возвращает всегра не null
	virtual ICoreStorageString * GetItemString(const char * name, const char * fileName, long fileLine) = null;
	//Установить целочисленное поле
	virtual bool SetLong(const char * name, long value) = null;
	//Получить целочисленное поле
	virtual long GetLong(const char * name, long def = 0, bool * isOk = null) = null;
	//Получить интерфейс на переменную типа long, возвращает всегра не null
	virtual ICoreStorageLong * GetItemLong(const char * name, const char * fileName, long fileLine) = null;
	//Установить численное поле
	virtual bool SetFloat(const char * name, float value) = null;
	//Получить численное поле
	virtual float GetFloat(const char * name, float def = 0, bool * isOk = null) = null;
	//Получить интерфейс на переменную типа float, возвращает всегра не null
	virtual ICoreStorageFloat * GetItemFloat(const char * name, const char * fileName, long fileLine) = null;

	//Получить папку, если такой нет, то создать, если имя не занято, возвращает всегра не null
	virtual ICoreStorageFolder * GetItemFolder(const char * name, const char * fileName, long fileLine) = null;

	//Получить количество полей данной папки
	virtual dword GetItemsCount() = null;
	//Получить имя поля по индексу
	virtual const char * GetNameByIndex(long index) = null;
	//Получить тип поля по индексу
	virtual Type GetTypeByIndex(long index) = null;

	//Удалить содержимое папки
	virtual void Delete(const char * name) = null;

	//Вывести содержимое в строку, (перевод строки \n)
	virtual void Print(string & buffer, const char * name = null) = null;

	//Сохранить состояние папки, добавив данные в data
	virtual bool Save(const char * folderName, array<byte> & data) = null;
	//Востоновить состояние папки из data. readPointer - текущая позиция чтения из data
	virtual bool Load(const char * folderName, const void * data, dword size, dword & readPointer) = null;

};




//--------------------------------------------

struct CoreSystemMessage
{
	const char * id;
	const byte * data;
	dword size;
};

struct CoreCommand
{
	dword commandID;
};

//Интерфейс ядра
class ICore : public Object
{
	friend RegObject;
public:
	virtual ~ICore(){};

public:
	enum DebugKeyEx
	{
		__system_start_ex_keys = 256,
		num_lock_flag,
		caps_lock_flag,
		scroll_lock_flag,
		xb_dpad_up,
		xb_dpad_down,
		xb_dpad_left,
		xb_dpad_right,
		xb_start,
		xb_back,
		xb_left_thumb,
		xb_right_thumb,
		xb_left_shoulder,
		xb_right_shoulder,
		xb_a,
		xb_b,
		xb_x,
		xb_y,
		xb_left_trigger,
		xb_right_trigger,
	};

	enum MultiThreading
	{
		mt_none = 0,
		mt_core_2,
		mt_core_4,
		mt_core_more,
	};

//------------------------------------------------------------------------------------------------
//Main functions
//------------------------------------------------------------------------------------------------
public:
	//Создать сервис
	virtual Service * GetService(const char * service_name) = null;
	//Установить уровень исполнения перед началом кадра
	virtual void SetStartFrameLevel(Service * s, dword level) = null;
	//Установить уровень исполнения после кадра
	virtual void SetEndFrameLevel(Service * s, dword level) = null;

	//Создать зарегестрированный объект
	virtual RegObject * CreateObject(const char * className) = null;

	//Установить функцию объекта на исполнение
	virtual void SetObjectExecutionFunc(RegObject * obj, const char * group, dword level, ObjectExecution func) = null;
	//Установить функцию объекта на исполнение
	template<class T> __forceinline void SetObjectExecution(RegObject * obj, const char * group, dword level, T func)
	{
		SetObjectExecutionFunc(obj, group, level, (ObjectExecution)func);
	};
	//Удалить обработчик
	virtual void DelObjectExecutionFunc(RegObject * obj, ObjectExecution func) = null;
	//Удалить обработчик
	template<class T> __forceinline void DelObjectExecution(RegObject * obj, T func)
	{
		DelObjectExecutionFunc(obj, (ObjectExecution)func);
	}
	//Удалить все обработчики данного объекта из группы
	virtual void DelObjectExecutions(RegObject * obj, const char * group) = null;
	//Удалить все обработчики данного объекта
	virtual void DelObjectExecutions(RegObject * obj) = null;

	//Установить уровень исполнения группы
	virtual void SetGroupLevel(const char * group, dword level) = null;
	//Получить уровень исполнения группы
	virtual dword GetGroupLevel(const char * group) = null;

	//Найти первый объект по имени
	virtual RegObject * FindObject(const char * className) = null;
	//Найти все объекты по имени
	virtual void FindObject(const char * className, array<Object *> & objects) = null;

	//Получить список всех зарегестрированных объектов
	virtual void GetRegistryObjectsList(array<string> & objects) = null;

	//Получить доступ к корневой папке переменных ядра
	virtual ICoreStorageFolder & Storage() = null;

//------------------------------------------------------------------------------------------------
//Time functions
//------------------------------------------------------------------------------------------------
public:
	//Получить текущий временной промежуток
	virtual float GetDeltaTime() = null;
	//Получить текущий временной промежуток без учёта скалирования
	virtual float GetNoScaleDeltaTime() = null;
	//Установить масштаб времени для всех
	virtual void SetTimeScale(float scale) = null;
	//Получить масштаб времени для всех
	virtual float GetTimeScale() = null;
	//Установить масштаб времени для группы
	virtual void SetTimeScale(float scale, const char * group) = null;
	//Получить масштаб времени для группы
	virtual float GetTimeScale(const char * group) = null;
	//Установить фиксированный временной отрезок
	virtual void SetFixedDeltaTime(float dltTime) = null;
	//Получить фиксированный временной отрезок
	virtual float GetFixedDeltaTime() = null;
	//Разрешить-запретить фиксированный временной отрезок
	virtual void EnableFixedDeltaTime(bool isEnable) = null;
	//Фиксированный временной отрезок установлен или нет
	virtual bool IsFixedDeltaTime() = null;
	//Получить текущую скорость в кадрах за секунду
	virtual float EngineFps() = null;

//------------------------------------------------------------------------------------------------
//System
//------------------------------------------------------------------------------------------------	
public:	
	//Дать ядру команду
	virtual void ExecuteCoreCommand(CoreCommand & cmd) = null;

#ifndef STOP_DEBUG
	//Вывести сообщение в системный лог
	virtual void _cdecl Trace(const char * format, ...) = null;
	//Вывести сообщение в системный лог
	virtual void TraceData(const char * format, const void * data) = null;
	//Вывести сообщение в системный лог 1 раз
	virtual void _cdecl Error(const char * format, ...) = null;
#else
	//Вывести сообщение в системный лог
	inline void _cdecl Trace(const char * format, ...){};
	//Вывести сообщение в системный лог
	inline void TraceData(const char * format, const void * data){};
	//Вывести сообщение в системный лог 1 раз
	inline void _cdecl Error(const char * format, ...){};
#endif

	//Активно ли приложение
	virtual bool IsActive() = null;
	//Завершить работу приложения
	virtual void Exit() = null;

	//Изменить объём выделеной памяти
	virtual void * Reallocate(void * ptr, dword size, const char * fileName, long fileLine) = null;
	//Освободить память
	virtual void Free(void * ptr, const char * fileName, long fileLine) = null;

	//Получить состояние нажатости дебажной кнопки (см. DebugKeyEx)
	__forceinline bool DebugKeyState(long id)
	{
#ifndef STOP_DEBUG
		return sysDebugKeyState(id);
#else
		return false;
#endif
	}
	//Получить состояние нажатости 2-x дебажных кнопок одновременно (см. DebugKeyEx)
	__forceinline bool DebugKeyState(long id1, long id2)
	{
#ifndef STOP_DEBUG
		return (sysDebugKeyState(id1) && sysDebugKeyState(id2));
#else
		return false;
#endif
	}
	//Получить состояние нажатости 3-x дебажных кнопок одновременно (см. DebugKeyEx)
	__forceinline bool DebugKeyState(long id1, long id2, long id3)
	{
#ifndef STOP_DEBUG
		return (sysDebugKeyState(id1) && sysDebugKeyState(id2) && sysDebugKeyState(id3));
#else
		return false;
#endif
	}

	//Получить количество системных сообщений, набравшихся за кадр
	virtual dword GetSystemMessagesCount() = null;
	//Получить системное сообщение
	virtual bool GetSystemMessage(dword index, CoreSystemMessage & msg) = null;

	//Получить конфигурацию ядер процессора
	virtual MultiThreading GetThreadingInfo() = null;

	//Получить идентификатор потока ядра
	__forceinline dword GetThreadId()
	{
		return threadId;
	}


	//Добавить значение к счётчику, который сбросится с конце кадра
	__forceinline void AddPerformanceCounter(const char * name, float value)
	{
#ifndef STOP_PROFILES
		sysAddPerformanceCounter(name, value);
#endif
	}

	//Установить счётчик, который сбросится с конце кадра
	__forceinline void SetPerformanceCounter(const char * name, float value)
	{
#ifndef STOP_PROFILES
		sysSetPerformanceCounter(name, value);
#endif
	}

	//Получить количество счётчиков
	__forceinline dword GetNumberOfPerformanceCounters()
	{
#ifndef STOP_PROFILES
		return sysGetNumberOfPerformanceCounters();
#else
		return 0;
#endif
	}

	//Получить имя счётчика
	__forceinline const char * GetPerformanceName(long index)
	{
#ifndef STOP_PROFILES
		return sysGetPerformanceName(index);
#else
		return null;
#endif
	}

	//Получить значение счётчика с предыдущего кадра
	__forceinline float GetPerformanceCounter(long index)
	{
#ifndef STOP_PROFILES
		return sysGetPerformanceCounter(index);
#else
		return 0.0f;
#endif
	}

	//Получить значение счётчика с предыдущего кадра
	__forceinline float GetPerformanceCounter(const char * name)
	{
#ifndef STOP_PROFILES
		return sysGetPerformanceCounter(name);
#else
		return 0.0f;
#endif
	}

	//Установить флажёк для сигнализации работы
	__forceinline void SetWatchDog()
	{
#ifndef STOP_DEBUG
		*watchDogPtr = 1;
#endif
	}

	//Получить текущее состояние флажка, сигнализирующего работу
	__forceinline bool GetWatchDog(bool isReset)
	{
#ifndef STOP_DEBUG
		bool result = *watchDogPtr != 0;
		if(isReset) *watchDogPtr = 0;
		return result;
#else
		return true;
#endif
	}

protected:
	//Удалить из списков регистрации объект
	virtual void sysUnregistryObject(RegObject * obj) = null;
	//Получить доступ к registryValue
	static __forceinline dword & sysRegObjectRegistryKey(RegObject * obj)
	{
		return obj->registryKey;
	}

	//Получить состояние нажатости дебажной кнопки (см. DebugKeyEx)
	virtual bool sysDebugKeyState(long id) = null;
	//Добавить значение к счётчику, который сбросится с конце кадра
	virtual void sysAddPerformanceCounter(const char * name, float value) = null;
	//Установить счётчик, который сбросится с конце кадра
	virtual void sysSetPerformanceCounter(const char * name, float value) = null;
	//Получить количество счётчиков
	virtual dword sysGetNumberOfPerformanceCounters() = null;
	//Получить имя счётчика
	virtual const char * sysGetPerformanceName(long index) = null;
	//Получить значение счётчика с предыдущего кадра
	virtual float sysGetPerformanceCounter(long index) = null;
	//Получить значение счётчика с предыдущего кадра
	virtual float sysGetPerformanceCounter(const char * name) = null;

	dword threadId;
#ifndef STOP_DEBUG	
	dword * watchDogPtr;
	byte watchDogBuffer[24];
#endif
};

__forceinline void RegObject::sysUnregistryObject()
{
	api->sysUnregistryObject(this);
}

#ifndef _CORE_NO_NEW_

//Работа с памятью
__forceinline void * _cdecl operator new(size_t size)
{
	return api->Reallocate(null, (dword)size, null, 0);
};

__forceinline void _cdecl operator delete(void * ptr)
{
	api->Free(ptr, null, 0);
}

__forceinline void * _cdecl resize(void * ptr, dword size, const char * fileName, long fileLine)
{
	return api->Reallocate(ptr, size, fileName, fileLine);
}

template<class T> __forceinline T * _cdecl Resize(T * ptr, dword objects, const char * fileName, long fileLine)
{
	return (T *)api->Reallocate(ptr, objects*sizeof(T), fileName, fileLine);
}

__forceinline void * _cdecl operator new(size_t size, const char * fileName, long fileLine)
{
	return api->Reallocate(null, (dword)size, fileName, fileLine);
}

__forceinline void _cdecl operator delete(void * ptr, const char * fileName, long fileLine)
{
	api->Free(ptr, fileName, fileLine);
}

#else

__forceinline void * _cdecl resize(void * ptr, dword size, const char * fileName, long fileLine)
{
	return realloc(ptr, size);
}


template<class T> __forceinline T * _cdecl Resize(T * ptr, dword objects, const char * fileName, long fileLine)
{
	return (T *)realloc(ptr, objects*sizeof(T));
}


#endif


//Макросы для регистрации сервисов, объектов, динамических библиотек (dll)

#pragma optimize("", off)

class StormEngine_Declarator
{
public:
	StormEngine_Declarator()
	{
		next = first;
		first = this;
	};

	static __forceinline StormEngine_Declarator * GetFirst(){ return first; };
	__forceinline StormEngine_Declarator * GetNext(){ return next; };
	__forceinline const char * ClassName(){ return className; };
	__forceinline bool IsService(){ return isService; };
	__forceinline long Level(){ return level; };
	virtual RegObject * Create() = null;

protected:	
	const char * className;
	bool isService;
	long level;
private:
	StormEngine_Declarator * next;
	static StormEngine_Declarator * first;
};

template<class T> class StormEngine_DeclaratorTmpl : public StormEngine_Declarator
{
public:
	StormEngine_DeclaratorTmpl(const char * name, bool isServ, long lvl)
	{
		className = name;
		isService = isServ;
		level = lvl;
	};
private:
	virtual RegObject * Create()
	{
		return NEW T();
	};
};

#define INTERFACE_FUNCTION	ICore * api = null;																													\
							StormEngine_Declarator * StormEngine_Declarator::first = null;																		\
							extern "C"																															\
							{																																	\
								void __declspec(dllexport) _cdecl StormEngine_InitDll(ICore * core, StormEngine_Declarator * & first, const char * & verId, long init_rand)			\
								{																																\
									api = core;																													\
									first = StormEngine_Declarator::GetFirst();																					\
									srand(init_rand);																											\
									static char value[128];																										\
									crt_snprintf(value, sizeof(value), "%s, %s", __DATE__, __TIME__);															\
									verId = value;																												\
								}																																\
							};

#define CREATE_CLASSBYTYPE(objectClassName, boolIsService, creationLevel) StormEngine_DeclaratorTmpl<objectClassName> objectClassName##_StormEngine_Declarator_Object(#objectClassName, boolIsService, creationLevel);


#define CREATE_CLASS(objectClassName)	CREATE_CLASSBYTYPE(objectClassName, false, 0x7fffffff)
#define CREATE_SERVICE(objectClassName, creationLevel)	CREATE_CLASSBYTYPE(objectClassName, true, creationLevel)
#define CREATE_SERVICE_NAMED(objectDefinedName, objectClassName, creationLevel) StormEngine_DeclaratorTmpl<objectClassName> objectClassName##_StormEngine_Declarator_Object(objectDefinedName, true, creationLevel);

#pragma optimize("", on)

//
inline void __Storm_Assert__(bool expression, const char * file, long line, const char * str)
{
	
#ifdef WIN32
	static char messageBoxText[1024];
	static char assertText[1024];
#endif


	if(!expression)
	{
		if(str)
		{
			api->Trace("Assert failed in %s line %d, expression string %s", file, line, str);

#ifdef WIN32
			crt_snprintf(assertText, 1000, "Assert failed in %s line %d, expression string %s", file, line, str);
#endif


		}else{
			api->Trace("Assert failed in %s line %d", file, line);

#ifdef WIN32
			crt_snprintf(assertText, 1000, "Assert failed in %s line %d", file, line);
#endif

		}
		CoreCommand cmd;
		cmd.commandID = 1;	//Проверка памяти
		api->ExecuteCoreCommand(cmd);
#ifdef WIN32

		FILE * logFile = crt_fopen("storm3_crush_dump.txt", "a+");
		if (logFile)
		{
			fprintf(logFile, "%s\n", assertText);
			fclose(logFile);
		}

		crt_snprintf(messageBoxText, 1000, "%s\n\nPlease attach next files from game folder to bug report:\nsystem_log.txt\nstorm3_crush_dump.txt\nstorm3_crush_dump.dmp\n", assertText);
		::MessageBoxA(NULL, messageBoxText, "Assert failed!", MB_OK | MB_ICONERROR);

		_asm int 3;

		exit(0);
#endif
	}
}

#define coremin(a,b) (((a) < (b)) ? (a) : (b))
#define coremax(a,b) (((a) > (b)) ? (a) : (b))



class ProfileTimer
{
private:
#ifndef STOP_PROFILES
#pragma pack(push, 1)
	union
	{
		unsigned __int64 time64;
		struct
		{
			unsigned __int32 lo;
			unsigned __int32 hi;
		};
	};
#pragma pack(pop)
#endif

public:
	__forceinline ProfileTimer()
	{
#ifndef STOP_PROFILES
		Start();
#endif
	};

	__forceinline void Start()
	{
#ifndef STOP_PROFILES
		time64 = GetCounter();
#endif
	};

	__forceinline unsigned __int64 Stop()
	{
#ifndef STOP_PROFILES
		time64 = GetCounter() - time64;
		return time64;
#else
		return 0;
#endif
	};

	__forceinline unsigned int GetTime32()
	{
#ifndef STOP_PROFILES
		if(hi == 0) return lo;
		return 0xffffffff;
#endif
		return 1;
	};

	__forceinline unsigned __int64 GetTime64()
	{
#ifndef STOP_PROFILES
		return time64;
#endif
		return 1;
	};

	__forceinline double GetTime()
	{
#ifndef STOP_PROFILES
		return (double)time64;
#endif
		return 1.0;
	};

	__forceinline void AddToCounter(const char * name, float scale = 1.0f)
	{
#ifndef STOP_PROFILES
		Stop();
		api->AddPerformanceCounter(name, float(((double)time64)*scale));
#endif
	};

	__forceinline void SetToCounter(const char * name, float scale = 1.0f)
	{
#ifndef STOP_PROFILES
		Stop();
		api->SetPerformanceCounter(name, float(((double)time64)*scale));
#endif
	};

	static __forceinline unsigned __int64 GetCounter()
	{
#ifndef STOP_PROFILES
#ifndef _XBOX
		unsigned __int64 v;
		unsigned __int64 * pv = &v;
		__asm
		{
			rdtsc
				mov ecx, pv
				mov [ecx], eax 
				mov [ecx + 4], edx
		}
		return v;
#else
		return __mftb();
#endif		
#else
		return 0;
#endif
	};
};



#endif

