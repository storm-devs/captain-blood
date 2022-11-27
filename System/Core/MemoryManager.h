
#ifndef _MemoryManager_h_
#define _MemoryManager_h_


#include "CoreBase.h"

//#define MM_NO_DEBUGMEMORY
//#define MM_FILL_MEMORY
#define MM_MEMORY_FILLER	0xff
//#define MM_MEMORY_FILLER	0x7f


#define MemoryManagerMemoryBlockSize		4	//x*4, x >= 1


class MemoryManager
{

	struct GuardBlock
	{
		dword guard[MemoryManagerMemoryBlockSize - 1];
		__forceinline bool operator == (const GuardBlock & b)
		{
			for(long i = 0; i < sizeof(guard)/sizeof(guard[0]); i++)
			{
				if(guard[i] != b.guard[i]) return false;
			}
			return true;
		}
	};

	struct MemoryBlock
	{
		long descIndex;				//Индекс описывающего блока (дескриптора)
		GuardBlock openGuardBlock;	//Защищающий код
	};

	struct MemoryBlockDesc
	{
		MemoryBlock * block;		//Указатель на блок памяти
		dword blockSize;			//Размер блока памяти (без данных менеджера)
		long descIndex;				//Индекс идентификатора места аллокации
	};

	struct DescString
	{
		long next;					//Индекс следующего блока
		const char * fileNamePtr;	//Указатель на исходную строку
		long fileLine;				//Общее количество алоцированных блоков за всё время
		dword numAllocBlocks;		//Общее количество алоцированных блоков за всё время
		bool isUse;					//Флаг для подсчёта алокаций
		char fileName[1];			//Сохранённая строка
	};


	class IMemoryPool
	{
	public:
		virtual ~IMemoryPool(){};
		//Выделить память данного размера
		virtual void * Alloc() = null;
		//Попытаться освободить память
		virtual bool Free(void * ptr) = null;
		//Получить размер блока по адресу
		virtual dword GetSize(void * ptr) = null;
	};

	template<int Size> class MemoryPool : public IMemoryPool
	{
		//Описание одного блока
		struct Block
		{
			dword use[(4096/Size + 31)/32];
			byte pool[4096/Size][Size];
		};

	public:
		MemoryPool();
		~MemoryPool();
		
		//Выделить память данного размера
		virtual void * Alloc();
		//Попытаться освободить память
		virtual bool Free(void * ptr);
		//Получить размер блока по адресу
		virtual dword GetSize(void * ptr);

	private:
		//Добавить адрес в общий диапазон
		void * AddAddress(void * ptr);

	private:
		void * minAddr;		//Минимальный используемый адрес
		void * maxAddr;		//Минимальный используемый адрес
		Block ** blocks;	//Выделенные блоки
		dword blocksCount;	//Количество блоков
		dword blocksMax;	//Размер массива для блоков
	};

	class MemoryPools
	{
	public:
		MemoryPools();
		void * Alloc(dword size);
		bool Free(void * ptr);
		dword Size(void * ptr);
	private:
		IMemoryPool * pools[24];
		void * minAddr;
		void * maxAddr;
		MemoryPool<4> pool4;   MemoryPool<8> pool8;   MemoryPool<12> pool12; MemoryPool<16> pool16;
		MemoryPool<20> pool20; MemoryPool<24> pool24; MemoryPool<28> pool28; MemoryPool<32> pool32;
		MemoryPool<36> pool36; MemoryPool<40> pool40; MemoryPool<44> pool44; MemoryPool<48> pool48;
		MemoryPool<52> pool52; MemoryPool<56> pool56; MemoryPool<60> pool60; MemoryPool<64> pool64;
		MemoryPool<68> pool68; MemoryPool<72> pool72; MemoryPool<76> pool76; MemoryPool<80> pool80;
		MemoryPool<84> pool84; MemoryPool<88> pool88; MemoryPool<92> pool92; MemoryPool<96> pool96;
	};

	template<int Size> friend class MemoryPool;

	struct StatData
	{
		long descIndex;
		dword blocksCount;
		dword totalSize;
		dword totalAllocs;
	};

public:
	MemoryManager();
	~MemoryManager();
	void ReleaseMemory(bool enableLogout);


//-------------------------------------------------------------------------------------------------------
//Работа с менеджером памяти
//-------------------------------------------------------------------------------------------------------
public:
	CritSection coreSection;
	//Выделить или изменить объём выделеной памяти
	void * Reallocate(void * ptr, dword size, const char * fileName, long fileLine);
	//Освободить память
	void Free(void * ptr, const char * fileName, long fileLine);

	//Освободить всю память
	void FreeMemory();

	//Вывести в лог статистику
	void TraceStatistic(CoreCommand_MemStat & stats);
	//Посчитать статистику по запросу
	void GetStatistic(CoreCommand_GetMemStat & stats);
	//Посчитать память занимаемую отладочной информацией менеджера
	void GetManagerStat(CoreCommand_GetMemManagerStat & stats);

private:
	static bool TraceStat_SortBySize(const StatData & d1, const StatData & d2);
	static bool TraceStat_SortByBlocks(const StatData & d1, const StatData & d2);
	static bool TraceStat_SortByFreq(const StatData & d1, const StatData & d2);
	static bool TraceStat_SortByFiles(const StatData & d1, const StatData & d2);

public:
	//Режим панической проверки
	void SetPanicCheckMemoryMode(bool isSet);
	//Стоит ли режим панической проверки
	bool IsPanicCheckMemoryMode();
	//Запустить - остоновить усиленную проверку памяти
	void EnablePanicCheckMemory(bool isEnable);

	//Включить пулы памяти
	void EnableMemPools();
	//Включены ли пулы памяти
	bool IsEnableMemPools();

	//Обновить состояние мэнеджера
	void Update();
	//Проверить на целостность всю выделенную память
	void PanicCheckMemory();
	//Сделать плановую проверку
	void CheckMemoryStep();

//-------------------------------------------------------------------------------------------------------
//Внутреннии незащищёные критическими секциями функции
//-------------------------------------------------------------------------------------------------------
private:
	//Проверить на целостность всю выделенную память
	void SysPanicCheckMemory();

//-------------------------------------------------------------------------------------------------------
//Работа с памятью менеджера
//-------------------------------------------------------------------------------------------------------
private:
	//Выделить память в пуле
	void * PoolAlloc(dword size, void * ptr, const char * fileName, long fileLine);
	//Освободить память в пуле
	bool PoolFree(void * ptr);
	//Выделить память вне пула
	void * MemAlloc(dword size, void * ptr, const char * fileName, long fileLine);
	//Освободить память
	void FreeInternal(void * ptr);
	//Проверить блок памяти
	void _fastcall CheckBlock(MemoryBlock * block);
	//Проверить на валидность поинтер
	bool IsValidateBlockPointer(MemoryBlock * block);
	//Проверить все блоки и вывести всю доступнукю информацию по разрушеным блокам
	void TraceDamagedBlocks();
	//Получить индекс строки
	long GetStringDescIndex(const char * fileName, long fileLine);

//-------------------------------------------------------------------------------------------------------
//Работа с системной памятью
//-------------------------------------------------------------------------------------------------------
private:
	//Выделить системную память
	static void * SysAlloc(dword size, void * ptr = null);
	//Освободить системную память
	static void SysFree(void * ptr);
	//Получить размер системного блока
	static dword SysSize(void * ptr);


//-------------------------------------------------------------------------------------------------------
private:
	ClassThread

private:
#ifndef MM_NO_DEBUGMEMORY
	//Таблицы описаний
	MemoryBlockDesc * descs;		//Массив опиcаний блоков
	long descsCount;				//Количество блоков (описаний)
	long descsMax;					//Размер массива
	long currentCheck;				//Индекс текущего проверяемого блока
	char * strings;					//Строки
	dword stringsSize;				//Размер используемой части буфера строк
	dword stringsMax;				//Размер буфера строк
	long entry[1024];				//Входная таблица для поиска строки
	bool isMemCheck;				//Проверять на целостность память в фоновом режиме
	bool isPanicCheck;				//Проверять на целостность всю память на каждом кадре и каждом запросе
	bool isPanicCheckEnable;		//Запустить-остановить паническую проверку
	GuardBlock openBlock;			//Открывающий блок
	GuardBlock closeBlock;			//Закрывающий блок
#endif
	bool isPoolEnable;				//Разрешена система сбора мелких блоков
	const char * lastFileName;		//Имя файла, где выделялась память в последний раз
	long lastFileLine;				//Линия в файле, где выделялась память в последний раз
	MemoryPools pools;				//Пулы памяти
	static dword totalSize;			//Выделенной в системе памяти
	static dword totalBlocks;		//Всего выделенно блоков
	static dword maxBlocks;			//Максимальное выделенное количество блоков в системе
	static dword maxPoolBlocks;		//Максимальное выделенное количество блоков в пулах
	static dword maxSize;			//Максимальное выделенное количество памяти в системе
	static dword poolBlocks;		//Блоков выделеных в пуле
	static dword allocsPerFrame;	//Алокаций за кадр
	static dword deletesPerFrame;	//Удалений за кадр
	static dword maxUsed360;		//Пик используемой памяти на xbox

	//
	static dword emptyMemory;		//Память для блоков 0 размера
	static const char * line;
};





#endif

