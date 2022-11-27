
#ifndef _Core_h_
#define _Core_h_


#include "CoreBase.h"
#include "DllManager.h"
#include "ObjectsDeclarators.h"
#include "ExecutionModule.h"
#include "PerformanceCounters.h"
#include "CoreStorage.h"

class Core : public ICore
{
	ClassThread

	//Система доступа к регистрационному ключу
	class Accessor : public RegistryKeyAccessor
	{
	protected:
		virtual dword & RegistryKey(RegObject * obj){ return Core::GetRegistryKey(obj); };
	};
	friend Accessor;
	static __forceinline dword & GetRegistryKey(RegObject * obj)
	{
		return ICore::sysRegObjectRegistryKey(obj);
	};

	struct SysMsgInfo
	{
		dword name;	//Индекс начала имени
		dword data;	//Индекс начала данных
		dword size;	//Размер данных
	};

	struct ErrorRecord
	{
		dword hash;
		string str;
		long next;
	};

public:
	Core(const char * coreModuleNameForSkip);
	virtual ~Core();

public:
	//Загрузить модули быстрого старта
	bool LoadQuickStartModules(const char * quickModules);
	//Загрузить модули невошедшие в список быстрого старта
	void LoadModules();

//------------------------------------------------------------------------------------------------
//Main functions
//------------------------------------------------------------------------------------------------
public:
	//Создать сервис
	virtual Service * GetService(const char * service_name);
	//Установить уровень исполнения перед началом кадра
	virtual void SetStartFrameLevel(Service * s, dword level);
	//Установить уровень исполнения после кадра
	virtual void SetEndFrameLevel(Service * s, dword level);

	//Создать зарегестрированный объект
	virtual RegObject * CreateObject(const char * className);

	//Установить функцию объекта на исполнение
	virtual void SetObjectExecutionFunc(RegObject * obj, const char * group, dword level, ObjectExecution func);
	//Удалить обработчик
	virtual void DelObjectExecutionFunc(RegObject * obj, ObjectExecution func);
	//Удалить все обработчики данного объекта из группы
	virtual void DelObjectExecutions(RegObject * obj, const char * group);
	//Удалить все обработчики данного объекта
	virtual void DelObjectExecutions(RegObject * obj);

	//Установить уровень исполнения группы
	virtual void SetGroupLevel(const char * group, dword level);
	//Получить уровень исполнения группы
	virtual dword GetGroupLevel(const char * group);

	//Проверить на валидность указатель объекта (зарегистрированного)
	virtual bool IsValidateRegObject(RegObject * obj, long * cacheValue = null);

	//Найти первый объект по имени
	virtual RegObject * FindObject(const char * className);
	//Найти все объекты по имени
	virtual void FindObject(const char * className, array<Object *> & objects);

	//Получить список всех зарегестрированных объектов
	virtual void GetRegistryObjectsList(array<string> & objects);

	//Получить доступ к корневой папке переменных ядра
	virtual ICoreStorageFolder & Storage();


//------------------------------------------------------------------------------------------------
//Time functions
//------------------------------------------------------------------------------------------------
public:
	//Получить текущий временной промежуток
	virtual float GetDeltaTime();
	//Получить текущий временной промежуток без учёта скалирования
	virtual float GetNoScaleDeltaTime();
	//Установить масштаб времени для всех
	virtual void SetTimeScale(float scale);
	//Получить масштаб времени для всех
	virtual float GetTimeScale();
	//Установить масштаб времени для группы
	virtual void SetTimeScale(float scale, const char * group);
	//Получить масштаб времени для группы
	virtual float GetTimeScale(const char * group);
	//Установить фиксированный временной отрезок
	virtual void SetFixedDeltaTime(float dltTime);
	//Получить фиксированный временной отрезок
	virtual float GetFixedDeltaTime();
	//Разрешить-запретить фиксированный временной отрезок
	virtual void EnableFixedDeltaTime(bool isEnable);
	//Фиксированный временной отрезок установлен или нет
	virtual bool IsFixedDeltaTime();
	//Получить текущую скорость в кадрах за секунду
	virtual float EngineFps();

//------------------------------------------------------------------------------------------------
//System
//------------------------------------------------------------------------------------------------	
public:	
	//Дать ядру команду
	virtual void ExecuteCoreCommand(CoreCommand & cmd);

	//Вывести сообщение в системный лог
	virtual void _cdecl Trace(const char * format, ...);
	//Вывести сообщение в системный лог
	virtual void TraceData(const char * format, const void * data);
	//Вывести сообщение в системный лог 1 раз
	virtual void _cdecl Error(const char * format, ...);

	//Активно ли приложение
	virtual bool IsActive();
	//Завершить работу приложения
	virtual void Exit();

	//Изменить объём выделеной памяти
	virtual void * Reallocate(void * ptr, dword size, const char * fileName, long fileLine);
	//Освободить память
	virtual void Free(void * ptr, const char * fileName, long fileLine);

	//Получить состояние нажатости дебажной кнопки
	virtual bool sysDebugKeyState(long id);

	//Получить количество системных сообщений, набравшихся за кадр
	virtual dword GetSystemMessagesCount();
	//Получить системное сообщение
	virtual bool GetSystemMessage(dword index, CoreSystemMessage & msg);

	//Получить конфигурацию ядер процессора
	virtual MultiThreading GetThreadingInfo();

	//Удалить из списков регистрации объект
	virtual void sysUnregistryObject(RegObject * obj);

	//Добавить значение к счётчику, который сбросится с конце кадра
	virtual void sysAddPerformanceCounter(const char * name, float value);
	//Установить счётчик, который сбросится с конце кадра
	virtual void sysSetPerformanceCounter(const char * name, float value);
	//Получить количество счётчиков
	virtual dword sysGetNumberOfPerformanceCounters();
	//Получить имя счётчика
	virtual const char * sysGetPerformanceName(long index);
	//Получить значение счётчика с предыдущего кадра
	virtual float sysGetPerformanceCounter(long index);
	//Получить значение счётчика с предыдущего кадра
	virtual float sysGetPerformanceCounter(const char * name);


//------------------------------------------------------------------------------------------------	
public:

	//Кадровое обновление
	void Update();
	//Активно ли ядро
	void SetActive(bool isActive);
	//Функция сообщающая о завершении работы
	bool IsExit();
	//Добавить сообщение в буфер сообщений ядра (при количестве параметров == 0 сообщение пропускается)
	void AddSystemMessage(const char * id, const unsigned char * data, unsigned long size);
	//Включить-выключить стейт ядра
	void SetState(CoreState state, long value);
	//Получить стейт ядра
	long GetState(CoreState state);
	//Создать зарегистрированные сервисы
	bool CreateAllServices();
	//Получить базу стейтов
	CoreStorage & GetCoreStorage();

//------------------------------------------------------------------------------------------------	
protected:
	//Мэнеджер dll
	DllManager dlls;
	//Менеджер объектов
	ObjectsDeclarators decls;
	Accessor accsessor;
	//Модуль исполнения
	ExecutionModule execution;
	//База данных переменных
	CoreStorage statesBase;
	//Системные сообщения, полученые на текущем кадре
	array<byte> messages;
	array<SysMsgInfo> messagesInfo;
	//Параметры таймера
	float dltTime;						//Временой промежуток на текущем кадре
	float fixedDltTime;					//Фиксированный временной отрезок
	float scaleDltTime;					//Масштабированный временой промежуток на текущем кадре
	float timeScale;					//Масштаб времени
	float fps;							//Средняя велечина кадров в секунду
	float fpsTime;						//Время с прошедшего замера fps
	dword frames;						//Кадров с прошедшего замера
	bool isFixedDltTime;				//Фиксированный временной отрезок
	bool isHiResTimer;					//Если запросы к таймеру прошли удачно, то работает таймер высокого разрешения
	dword loResTime;					//Текущее время в низком разрешении
	LARGE_INTEGER hiResTime;			//Текущее время в высоком разрешении
	double hiTicksToSeconds;			//Коэфициент преобразования тиков в высоком разрешении в секунды
	long fixedFPS;						//Внешние фиксирование фпс
	float fixedFPSDltTime;				//Временной отрезок фиксированных фпс	
	//Флаги
	bool isMemCheck;					//Включена фоновая проверка памяти
	bool isDebugKeys;					//Разрешены ли дебажные клавиши
	bool isExit;						//Флаг выхода
	bool isActive;						//Переменная состояния активности
	MultiThreading mtmode;				//Текущий режим многопоточности
	//Массив ошибок
	array<ErrorRecord> errors;
	long errorsEntry[1024];
	
#ifndef STOP_PROFILES
	PerformanceCounters pcnts;	//Счётчики профайла
#endif
};

//Получить базу стейтов
inline CoreStorage & Core::GetCoreStorage()
{
	return statesBase;
}


#endif





