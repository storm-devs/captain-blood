#ifndef _CoreManager_h_
#define _CoreManager_h_

class ICore;

//Приёмник лог-сообщений
class CoreSystemLogReciver
{
public:
	virtual ~CoreSystemLogReciver(){};

	//Принять сообщение
	virtual void Out(const char * msg) = 0;
};

enum CoreManagerCoreState
{
	cmcorestate_trace = 1,		//Вывод в системный лог
	cmcorestate_debugkeys,		//Отладочные клавиши
	cmcorestate_memcheck,		//Выполнять переодическую фоновую проверку памяти
	cmcorestate_panicmemcheck,	//Выполнять полную постоянную проверку памяти
	cmcorestate_memorypools,	//Включить пулы памяти
	cmcorestate_mtmanual,		//Установить принудительный режим многопоточности
	cmcorestate_lorestimer,		//Установить таймер в низкое разрешение
	cmcorestate_fixedfps,		//Установить возможность включить фиксированные fps
};

//Менеджер для работы с ядром
class CoreManager
{
public:
	virtual ~CoreManager(){};

	//Создать
	virtual void CreateCore(CoreSystemLogReciver * reciver, const char * coreModuleNameForSkip) = 0;
	//Удалить
	virtual void DeleteCore() = 0;
	//Загрузить модули быстрого старта
	virtual bool LoadQuickStartModules(const char * quickModules) = 0;
	//Загрузить модули невошедшие в список быстрого старта
	virtual void LoadModules() = 0;
	//Обновить
	virtual void FrameUpdate() = 0;
	//Получить указатель
	virtual ICore * GetCore() = 0;
	//Активно ли ядро
	virtual void SetActive(bool isActive) = 0;
	//Завершить работу приложения
	virtual bool IsExit() = 0;
	//Создать зарегистрированные сервисы (повторно созданные не создаёт)
	virtual bool CreateAllServices() = 0;
	//Создать объект
	virtual bool CreateObject(const char * className) = 0;
	//Установить числовую целочисленную переменную
	virtual void SetVarInt(const char * name, long v) = 0;
	//Получить числовую целочисленную переменную
	virtual long GetVarInt(const char * name, long def = 0, bool * res = 0) = 0;
	//Установить числовую переменную
	virtual void SetVarDouble(const char * name, double v) = 0;
	//Получить числовую переменную
	virtual double GetVarDouble(const char * name, double def = 0.0f, bool * res = 0) = 0;
	//Утановить строковую переменную
	virtual void SetVarString(const char * name, const char * v) = 0;
	//Получить строковую переменную
	virtual const char * GetVarString(const char * name, const char * def = 0, bool * res = 0) = 0;
	//Включить-выключить стейт ядра
	virtual void SetState(CoreManagerCoreState state, long value) = 0;
	//Получить стейт ядра
	virtual long GetState(CoreManagerCoreState state) = 0;
	//Добавить сообщение в буфер сообщений ядра
	virtual void AddSystemMessage(const char * id, const void * data, unsigned long size) = 0;
	//Вывести сообщение в системный лог
	virtual void _cdecl Trace(const char * format, ...) = 0;

};


//Имя функции экспорта
#define GetCoreManagerFunction		"StormEngine_GetCoreManager"

//Описание функции инициализации менеджера ядра из dll
typedef void (_cdecl * GetCoreManager)(CoreManager * & pointerToCoreManager);

//Описание функции инициализации менеджера ядра из текущего модуля
CoreManager * _cdecl StormEngine_GetCoreManager();

#endif

