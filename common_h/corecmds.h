#ifndef _CoreCmds_h_
#define _CoreCmds_h_


//=================================================================================================================
//Команды для ядра
//=================================================================================================================

enum CoreCommandsID
{
	ccid_nope = 0,				//Ничего не делать
	ccid_assert_memcheck = 1,		//Произвести проверку памяти в assert
	ccid_memcheck,				//Произвести полную проверку памяти
	ccid_setstate,				//Установить стейт ядра
	ccid_getstate,				//Получить стейт ядра
	ccid_memstatistic,			//Вывести в лог полную статистику по памяти
	ccid_getmemstatistic,			//Получить базовую статистику по памяти
	ccid_getmemmanagermemstat,		//Получить внутреннюю статистику мэнеджера

	
	ccid_forcedworrd = 0x7fffffff
};


//=================================================================================================================
//Проверить память
//=================================================================================================================


struct CoreCommand_CheckMemory : public CoreCommand
{
	CoreCommand_CheckMemory()
	{
		commandID = ccid_memcheck;
	};
};


//=================================================================================================================
//Управление режимами ядра
//=================================================================================================================

enum CoreState
{
	corestate_none = 0,
	corestate_trace,			//Вывод в системный лог
	corestate_debugkeys,			//Отладочные клавиши
	corestate_memcheck,			//Выполнять переодическую фоновую проверку памяти
	corestate_panicmemcheck,		//Выполнять полную постоянную проверку памяти
	corestate_mempools,			//Включить систему пулов памяти (не выключаемо)
	corestate_mtmanual,			//Установить принудительный режим многопоточности
	corestate_lorestimer,			//Установить таймер в низкое разрешение
	corestate_fixedfps,			//Установить возможность включить фиксированные fps
	
	corestate_forcedworrd = 0x7fffffff
};

//Управление ядром
struct CoreCommand_StateOp : public CoreCommand
{
	__forceinline CoreCommand_StateOp(CoreCommandsID id, CoreState cs, long v)
	{
		commandID = id;
		state = cs;
		value = v;
	};

	CoreState state;				//Стейт
	long value;					//Состояние
};

struct CoreCommand_LockDebugKeys : public CoreCommand_StateOp
{
	__forceinline CoreCommand_LockDebugKeys(bool isLock) : CoreCommand_StateOp(ccid_setstate, corestate_debugkeys, !isLock)
	{
	};
};


//=================================================================================================================
//Получение статистики
//=================================================================================================================

enum CoreMemoryStatisticSortType
{
	cmemstat_nosort = 0,		//Вывести статистику по всем блокам неотсортированную
	cmemstat_bysize,		//Вывести статистику по всем блокам отсортированную по размеру
	cmemstat_byblocks,		//Вывести статистику по всем блокам отсортированную по количество занятых блоков
	cmemstat_byfreq,		//Вывести статистику по всем блокам отсортированную по количеству аллокаций
	cmemstat_bysize20,		//Вывести первых 20 по всем блокам отсортированную по размеру
	cmemstat_byblocks20,		//Вывести первых 20 по всем блокам отсортированную по количество занятых блоков
	cmemstat_byfreq20,		//Вывести первых 20 по всем блокам отсортированную по количеству аллокаций
	cmemstat_byfile,		//Вывести статистику по всем блокам отсортированную по именам файлов
	cmemstat_onlytotal,		//Вывести только общую статистику
};


//Получение статистики для блоков в заданном диапазоне
struct CoreCommand_MemStat : public CoreCommand
{
	__forceinline CoreCommand_MemStat(CoreMemoryStatisticSortType _sortType, long _minSize = -1, long _maxSize = -1, const char * _fileName = null)
	{
		commandID = ccid_memstatistic;
		sortType = _sortType;
		minSize = _minSize;
		maxSize = _maxSize;
		fileName = _fileName;
	};

	CoreMemoryStatisticSortType sortType;
	long minSize;
	long maxSize;
	const char * fileName;
};

//Получение базовой статистики
struct CoreCommand_GetMemStat : public CoreCommand
{
	__forceinline CoreCommand_GetMemStat(const char * _fileName = null, long _fileLine = -1)
	{
		commandID = ccid_getmemstatistic;
		totalAllocSize = 0;
		numBlocks = 0;
		allocsCount = 0;
		allocsPerFrame = 0;
		deletesPerFrame = 0;
		fileName = _fileName;
		fileLine = _fileLine;
	};
	dword totalAllocSize;
	dword numBlocks;
	dword allocsCount;
	dword allocsPerFrame;
	dword deletesPerFrame;
	const char * fileName;
	long fileLine;
};

//Получение статистики мэнеджера памяти
struct CoreCommand_GetMemManagerStat : public CoreCommand
{
	CoreCommand_GetMemManagerStat()
	{
		commandID = ccid_getmemmanagermemstat;
		totalSizeUseForDebug = 0;
		allocsForBlockDesc = 0;
		allocsForNameBase = 0;
		allocsForGuards = 0;
	}
	
	dword totalSizeUseForDebug;		//Общая память используемая для отладочной информации
	dword allocsForBlockDesc;		//Памяти на описание блоков
	dword allocsForNameBase;		//Памяти для базы имён
	dword allocsForGuards;			//Памяти на защитные блоки
};


#endif
