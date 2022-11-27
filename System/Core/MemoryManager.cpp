
#include <malloc.h>
#include "MemoryManager.h"

#define MemoryManager_CheckDword	'Good'
#define MemoryManager_MaxPoolSize	96


//Менеджер памяти
dword guardIn[256];
MemoryManager memoryManager;
dword guardOut[256];


//Память для блоков 0 размера
dword MemoryManager::emptyMemory = MemoryManager_CheckDword;
const char * MemoryManager::line = "---------------------------------------------------------------------------------------------";
dword MemoryManager::totalSize = 0;
dword MemoryManager::totalBlocks = 0;
dword MemoryManager::maxBlocks = 0;
dword MemoryManager::maxPoolBlocks = 0;
dword MemoryManager::maxSize = 0;
dword MemoryManager::poolBlocks = 0;
dword MemoryManager::allocsPerFrame = 0;
dword MemoryManager::deletesPerFrame = 0;
dword MemoryManager::maxUsed360 = 0;


MemoryManager::MemoryManager()
{
	if((sizeof(MemoryBlock) & 0xf) != 0)
	{
		throw "Invalidate block size";
	}
	for(long i = 0; i < ARRSIZE(guardIn); i++)
	{
		guardIn[i] = MemoryManager_CheckDword + i;
	}
	for(long i = 0; i < ARRSIZE(guardOut); i++)
	{
		guardOut[i] = MemoryManager_CheckDword - i;
	}
#ifndef MM_NO_DEBUGMEMORY
	for(long i = 0; i < sizeof(openBlock.guard)/sizeof(openBlock.guard[0]); i++)
	{
		openBlock.guard[i] = MemoryManager_CheckDword ^ i;
		closeBlock.guard[i] = MemoryManager_CheckDword ^ (rand()*i);
	}  
	descs = null;
	descsCount = 0;
	descsMax = 0;
	currentCheck = 0;
	strings = null;
	stringsSize = 0;
	stringsMax = 0;
	for(long i = 0; i < ARRSIZE(entry); i++) entry[i] = -1;
	isMemCheck = false;
	isPanicCheck = false;
	isPanicCheckEnable = false;
#endif
	isPoolEnable = false;
	lastFileName = null;
	lastFileLine = 0;
	totalSize = 0;
	totalBlocks = 0;
	poolBlocks = 0;
	maxBlocks = 0;
	maxSize = 0;
}

MemoryManager::~MemoryManager()
{
	ReleaseMemory(false);
}

void MemoryManager::ReleaseMemory(bool enableLogout)
{
	SingleClassThread
#ifndef MM_NO_DEBUGMEMORY
		//Освобождение неудалённых блоков
		if(enableLogout)
		{
			if(descsCount > 0)
			{
				CoreLogOut("%s\nMemory leaks:\n%s\n\n", line, line);
				for(long i = 0; i < descsCount; i++)
				{
					MemoryBlockDesc & d = descs[i];
					CoreLogOut("  Pointer: 0x%.8x, size: %u", d.block, d.blockSize);
					if(d.descIndex >= 0)
					{
						DescString & dss = (DescString &)strings[d.descIndex];
						CoreLogOut("    File: %s, line: %i", dss.fileName, dss.fileLine);
					}			
				}
				CoreLogOut("\n  Totoal blocks: %u", descsCount);
			}else{
				CoreLogOut("Memory manager released successful...");
			}
		}
		//Освободим массив описаний
		SysFree(descs);
		descs = null;
		descsCount = 0;		   
		descsMax = 0;
		SysFree(strings);
		strings = null;
		stringsSize = 0;
		stringsMax = 0;
#endif
		if(enableLogout)
		{
			CoreLogOut("  Peak allocate memory blocks: %u", maxBlocks);
			if(isPoolEnable) CoreLogOut("  Peak allocate memory blocks in pools: %u, (system blocks = %i)", maxPoolBlocks, maxBlocks - maxPoolBlocks);
			CoreLogOut("  Peak allocate memory size: %.2f Mb (%u bytes)", maxSize/1024.0/1024.0, maxSize);
#ifdef _XBOX
			CoreLogOut("  Pick memory usages on xbox360 = %.2f Mb (%u bytes)\n", maxUsed360/1024.0/1024.0, maxUsed360);
#endif
			CoreLogOut("\n");
		}
}

//Изменить объём выделеной памяти
void * MemoryManager::Reallocate(void * ptr, dword size, const char * fileName, long fileLine)
{
#ifndef MM_NO_DEBUGMEMORY
	ClassThreadLock
	allocsPerFrame++;
	//Непрерывная проверка памяти
	if(isPanicCheck) SysPanicCheckMemory();
	//Сохраним место последнего выделения памяти
	lastFileName = fileName;
	lastFileLine = fileLine;
	//Корректируем указатель
	if(ptr)
	{
		ptr = (MemoryBlock *)ptr - 1;
		CheckBlock((MemoryBlock *)ptr);
	}else{
		totalBlocks++;
		if(maxBlocks < totalBlocks) maxBlocks = totalBlocks;
	}
#ifdef MM_FILL_MEMORY
	dword currentBlockSize = 0;
	if(ptr)
	{
		currentBlockSize = SysSize(ptr);
	}
#endif
	//Выделяем память с учётом защитных блоков
	char * mem = null;
	if(isPoolEnable && size + sizeof(MemoryBlock) + sizeof(GuardBlock) <= MemoryManager_MaxPoolSize)
	{
		mem = (char *)PoolAlloc(size + sizeof(MemoryBlock) + sizeof(GuardBlock), ptr, fileName, fileLine);
	}else{
		mem = (char *)MemAlloc(size + sizeof(MemoryBlock) + sizeof(GuardBlock), ptr, fileName, fileLine);
	}
	if(!mem)
	{
		ClassThreadUnlock
		bool isPullAllocate = (isPoolEnable && size + sizeof(MemoryBlock) + sizeof(GuardBlock) <= MemoryManager_MaxPoolSize);
		CoreLogOut("Can't allocate memory! (%s)", isPullAllocate ? "alloc from pull" : "alloc from system memory");
		CoreLogOut("from '%s', line %d", fileName, fileLine);
		CoreLogOut("Total size = %u, blocks count = %u", totalSize, totalBlocks);
		CoreLogOut("Alloc size : %d bytes", size + sizeof(MemoryBlock) + sizeof(GuardBlock));
		SysPanicCheckMemory();
#ifdef _XBOX
		MEMORYSTATUS memStatus;
		GlobalMemoryStatus(&memStatus);
		CoreLogOut("Xbox360 available physmemory size : %u bytes", (dword)memStatus.dwAvailPhys);
		CoreLogOut("Xbox360 total physmemory size : %u bytes", (dword)memStatus.dwTotalPhys);
		CoreLogOut("Xbox360 used physmemory size : %u bytes", (dword)(memStatus.dwTotalPhys - memStatus.dwAvailPhys));
#endif
		ClassThreadLock
	}
	Assert(mem);
	if(!ptr)
	{	
		//Регестрируем запись
		if(descsCount >= descsMax)
		{
			descsMax = (descsMax + descsMax/16 + 4096) & ~4095;
			descs = (MemoryBlockDesc *)SysAlloc(descsMax*sizeof(MemoryBlockDesc), descs);
		}
		Assert(descs);
		descs[descsCount].blockSize = size;
		descs[descsCount].descIndex = GetStringDescIndex(fileName, fileLine);
		//Заполняем структуры выделенного блока
		((MemoryBlock *)mem)->descIndex = descsCount;
		((MemoryBlock *)mem)->openGuardBlock = openBlock;
		descsCount++;		
	}
	*(GuardBlock *)(mem + sizeof(MemoryBlock) + size) = closeBlock;
	descs[((MemoryBlock *)mem)->descIndex].block = (MemoryBlock *)mem;
	descs[((MemoryBlock *)mem)->descIndex].blockSize = size;
	long descIndex = descs[((MemoryBlock *)mem)->descIndex].descIndex;
	DescString * ds = (DescString *)(strings + descIndex);
	ds->numAllocBlocks++;
	mem += sizeof(MemoryBlock);
#ifdef MM_FILL_MEMORY
	if(!ptr)
	{
		memset(mem, MM_MEMORY_FILLER, size);
	}else{
		if(size > currentBlockSize)
		{
			memset(mem + currentBlockSize, MM_MEMORY_FILLER, size - currentBlockSize);
		}
	}
#endif
	ClassThreadUnlock
	return mem;
#else
	SingleClassThread
	//Для дежурной отладки сохраняем место выделения памяти
	lastFileName = fileName;
	lastFileLine = fileLine;
	//Выделяем память
	if(isPoolEnable && size <= MemoryManager_MaxPoolSize && size)
	{
		return PoolAlloc(size, ptr, fileName, fileLine);
	}
	return MemAlloc(size, ptr, fileName, fileLine);
#endif
}

//Освободить память
void MemoryManager::Free(void * ptr, const char * fileName, long fileLine)
{
	SingleClassThread
	if(!ptr) return;
	deletesPerFrame++;
	totalBlocks--;
#ifndef MM_NO_DEBUGMEMORY
	if(isPanicCheck) SysPanicCheckMemory();
	//Проверяем валидность
	MemoryBlock * block = (MemoryBlock *)ptr - 1;
	CheckBlock(block);
	//Удаляем регистрационную запись
	Assert(descs[descsCount - 1].block->descIndex == descsCount - 1);
	descs[descsCount - 1].block->descIndex = block->descIndex;
	descs[block->descIndex] = descs[descsCount - 1];
	descsCount--;
	ptr = block;
#endif
	FreeInternal(ptr);
}

//Вывести в лог статистику
void MemoryManager::TraceStatistic(CoreCommand_MemStat & stats)
{
	SingleClassThread

	CoreLogOut("\n\n%s\nMemory statistic:\n%s", line, line);
#ifndef MM_NO_DEBUGMEMORY
	if(stats.sortType != cmemstat_onlytotal)
	{
		if(isPanicCheck) PanicCheckMemory();
		//Собираем статистику по файлам
		StatData * dataPtr = (StatData *)SysAlloc( (descsCount + 1)*sizeof(StatData), null);
		long dataCount = 0;
		for(long i = 0; i < sizeof(entry)/sizeof(entry[0]); i++)
		{
			for(long idx = entry[i]; idx >= 0; )
			{
				DescString * ds = (DescString *)(strings + idx);
				ds->isUse = false;
				idx = ds->next;
			}
		}
		for(long i = 0; i < descsCount; i++)
		{
			//Проверим ограничение по размеру
			if(stats.minSize >= 0)
			{
				if(descs[i].blockSize < dword(stats.minSize)) continue;
			}
			if(stats.maxSize >= 0)
			{
				if(descs[i].blockSize > dword(stats.maxSize)) continue;
			}
			//Имя файла
#ifndef _XBOX
			if(descs[i].descIndex < 0)
			{
				_asm int 3;
			}
#endif
			long descIndex = descs[i].descIndex;
			DescString * ds = (DescString *)(strings + descIndex);
			//Посмотрим на имя файла
			if(stats.fileName)
			{
				if(!string::EqualPostfix(ds->fileName, stats.fileName)) continue;
			}
			//Определим блок в массив
			for(long j = 0; j < dataCount; j++)
			{
				if(dataPtr[j].descIndex == descIndex) break;
				DescString * dsj = (DescString *)(strings + dataPtr[j].descIndex);
				if(ds->fileLine == dsj->fileLine)
				{
					if(string::IsEqual(ds->fileName, dsj->fileName))
					{
						break;
					}
				}
			}
			if(j >= dataCount)
			{
				StatData & sd = dataPtr[dataCount++];
#ifndef _XBOX
				if(dataCount > descsCount)
				{
					_asm int 3;
				}
#endif
				sd.descIndex = descIndex;
				sd.blocksCount = 0;
				sd.totalSize = 0;
				sd.totalAllocs = 0;
			}
			StatData & sd = dataPtr[j];
			sd.blocksCount++;
			sd.totalSize += descs[i].blockSize;
			if(!ds->isUse)
			{
				sd.totalAllocs += ds->numAllocBlocks;
				ds->isUse = true;
			}
		}
		//Сортируем в требуемом порядке
		long outputLines = dataCount;
		const char * sortName = "No sorted";
		switch(stats.sortType)
		{
		case cmemstat_bysize20:
			outputLines = coremin(outputLines, 20);
		case cmemstat_bysize:
			sortName = "Sort by size";
			array<StatData>::QSort(TraceStat_SortBySize, dataPtr, dataCount);
			break;
		case cmemstat_byblocks20:
			outputLines = coremin(outputLines, 20);
		case cmemstat_byblocks:
			sortName = "Sort by blocks";
			array<StatData>::QSort(TraceStat_SortByBlocks, dataPtr, dataCount);
			break;
		case cmemstat_byfreq20:
			outputLines = coremin(outputLines, 20);
		case cmemstat_byfreq:
			sortName = "Sort by count of alloc";
			array<StatData>::QSort(TraceStat_SortByFreq, dataPtr, dataCount);
			break;
		case cmemstat_byfile:
			sortName = "Sort by file";
			array<StatData>::QSort(TraceStat_SortByFiles, dataPtr, dataCount);
			break;
		}
		CoreLogOut("%s\n%s\n", sortName, line, line);
		//Выводим полученную информацию в лог
		for(long i = 0; i < outputLines; i++)
		{
			DescString * ds = (DescString *)(strings + dataPtr[i].descIndex);
			CoreLogOut("    size: %8u, blocks: %8u, allocs: %8u, file: %s, line: %i", 
				dataPtr[i].totalSize,
				dataPtr[i].blocksCount,
				dataPtr[i].totalAllocs,
				ds->fileName,
				ds->fileLine);
		}
	}
#endif
	CoreLogOut("\nTotal use memory: %.2f Mb (%u bytes)", totalSize/1024.0/1024.0, totalSize);
#ifdef _XBOX
	MEMORYSTATUS memStatus;
	GlobalMemoryStatus(&memStatus);
	dword use360 = memStatus.dwTotalPhys - memStatus.dwAvailPhys;
	CoreLogOut("Total xbox360 use memory: %.2f Mb (%u bytes)", use360/1024.0/1024.0, use360);
#endif
	CoreLogOut("Peak allocate memory size: %.2f Mb (%u bytes)", maxSize/1024.0/1024.0, maxSize);
#ifdef _XBOX
	CoreLogOut("Pick memory usages on xbox360: %.2f Mb (%u bytes)", maxUsed360/1024.0/1024.0, maxUsed360);
#endif
	CoreLogOut("Total use blocks: %u", totalBlocks);
	CoreLogOut("Total use blocks in pools: %u", poolBlocks);

	CoreLogOut("%s\n\n", line);
}

bool MemoryManager::TraceStat_SortBySize(const StatData & d1, const StatData & d2)
{
	return d1.totalSize > d2.totalSize;
}

bool MemoryManager::TraceStat_SortByBlocks(const StatData & d1, const StatData & d2)
{
	return d1.blocksCount > d2.blocksCount;
}

bool MemoryManager::TraceStat_SortByFreq(const StatData & d1, const StatData & d2)
{
	return d1.totalAllocs > d2.totalAllocs;
}

bool MemoryManager::TraceStat_SortByFiles(const StatData & d1, const StatData & d2)
{
#ifndef MM_NO_DEBUGMEMORY
	DescString * ds1 = (DescString *)(memoryManager.strings + d1.descIndex);
	DescString * ds2 = (DescString *)(memoryManager.strings + d2.descIndex);
	long res = strcmp(ds1->fileName, ds2->fileName);
	if(res == 0)
	{
		return ds1->fileLine < ds2->fileLine;
	}
	return res < 0;
#else
	return false;
#endif
}

//Вернуть статистику
void MemoryManager::GetStatistic(CoreCommand_GetMemStat & stats)
{
	if(!stats.fileName || stats.fileName[0] == 0)
	{
		stats.totalAllocSize = totalSize;
		stats.numBlocks = totalBlocks;
		stats.allocsPerFrame = allocsPerFrame;
		stats.deletesPerFrame = deletesPerFrame;
		return;
	}
#ifndef MM_NO_DEBUGMEMORY
	SingleClassThread
	stats.totalAllocSize = 0;
	stats.numBlocks = 0;
	stats.allocsCount = 0;
	for(long i = 0; i < sizeof(entry)/sizeof(entry[0]); i++)
	{
		for(long idx = entry[i]; idx >= 0; )
		{
			DescString * ds = (DescString *)(strings + idx);
			ds->isUse = false;
			idx = ds->next;
		}
	}
	for(long i = 0; i < descsCount; i++)
	{
		//Имя файла
		Assert(descs[i].descIndex >= 0);
		long descIndex = descs[i].descIndex;
		DescString * ds = (DescString *)(strings + descIndex);
		//Посмотрим на имя файла
		if(stats.fileLine > 0 && ds->fileLine != stats.fileLine) continue;
		if(!string::EqualPostfix(ds->fileName, stats.fileName)) continue;
		stats.totalAllocSize += descs[i].blockSize;
		stats.numBlocks++;
		if(!ds->isUse)
		{			
			stats.allocsCount += ds->numAllocBlocks;
			ds->isUse = true;
		}
	}
#else
	stats.totalAllocSize = 0;
	stats.numBlocks = 0;
	stats.allocsPerFrame = 0;
	stats.deletesPerFrame = 0;
#endif
}

//Посчитать память занимаемую отладочной информацией менеджера
void MemoryManager::GetManagerStat(CoreCommand_GetMemManagerStat & stats)
{
#ifndef MM_NO_DEBUGMEMORY
	stats.allocsForBlockDesc = descsMax*sizeof(MemoryBlockDesc);
	stats.allocsForGuards = totalBlocks*(sizeof(MemoryBlock) + sizeof(GuardBlock));	
	stats.allocsForNameBase = stringsMax;
	stats.totalSizeUseForDebug = stats.allocsForBlockDesc + stats.allocsForGuards + stats.allocsForNameBase;
#endif
}

//Режим панической проверки
void MemoryManager::SetPanicCheckMemoryMode(bool isSet)
{
#ifndef MM_NO_DEBUGMEMORY
	isPanicCheck = isSet;
	if(isSet)
	{
		isPanicCheckEnable = true;
	}
#endif
}

//Запустить - остоновить усиленную проверку памяти
void MemoryManager::EnablePanicCheckMemory(bool isEnable)
{
#ifndef MM_NO_DEBUGMEMORY
	isPanicCheckEnable = isEnable;
#endif
}

//Стоит ли режим панической проверки
bool MemoryManager::IsPanicCheckMemoryMode()
{
#ifndef MM_NO_DEBUGMEMORY
	return isPanicCheck;
#else
	return false;
#endif
}

//Включить пулы памяти
void MemoryManager::EnableMemPools()
{
	isPoolEnable = true;
}

//Включены ли пулы памяти
bool MemoryManager::IsEnableMemPools()
{
	return isPoolEnable;
}

//Обновить состояние мэнеджера
void MemoryManager::Update()
{
	allocsPerFrame = 0;
	deletesPerFrame = 0;
}

//Проверить на целостность всю выделенную память
void MemoryManager::PanicCheckMemory()
{
#ifndef MM_NO_DEBUGMEMORY
	SingleClassThread
	bool cur = isPanicCheckEnable;
	isPanicCheckEnable = true;
	SysPanicCheckMemory();
	isPanicCheckEnable = cur;
#endif
}

//Сделать плановую проверку
void MemoryManager::CheckMemoryStep()
{
#ifndef MM_NO_DEBUGMEMORY
	SingleClassThread
	Assert(this == &memoryManager);
	Assert(emptyMemory == MemoryManager_CheckDword);
	for(long i = 0; i < ARRSIZE(guardIn); i++)
	{
		Assert(guardIn[i] == MemoryManager_CheckDword + i);
	}
	for(long i = 0; i < ARRSIZE(guardOut); i++)
	{
		Assert(guardOut[i] == MemoryManager_CheckDword - i);
	}
	if(descsCount <= 0) return;
	long count = descsCount/20;
	if(count < 1) count = 1;
	for(long i = 0; i < count; i++)
	{
		if(currentCheck < 0 || currentCheck >= descsCount)
		{
			currentCheck = 0;
		}
		CheckBlock(descs[currentCheck].block);
		currentCheck++;
	}
#endif
}

//-------------------------------------------------------------------------------------------------------
//Внутреннии незащищёные критическими секциями функции
//-------------------------------------------------------------------------------------------------------

//Проверить на целостность всю выделенную память
void MemoryManager::SysPanicCheckMemory()
{
#ifndef MM_NO_DEBUGMEMORY
	if(!isPanicCheckEnable) return;
	Assert(this == &memoryManager);
	Assert(emptyMemory == MemoryManager_CheckDword);
	for(long i = 0; i < ARRSIZE(guardIn); i++)
	{
		Assert(guardIn[i] == MemoryManager_CheckDword + i);
	}
	for(long i = 0; i < ARRSIZE(guardOut); i++)
	{
		Assert(guardOut[i] == MemoryManager_CheckDword - i);
	}
	for(long i = 0; i < descsCount; i++)
	{
		CheckBlock(descs[i].block);
	}
	Assert(_heapchk() == _HEAPOK);
#endif
}

//-------------------------------------------------------------------------------------------------------
//Работа с памятью менеджера
//-------------------------------------------------------------------------------------------------------

//Выделить память в пуле
__forceinline void * MemoryManager::PoolAlloc(dword size, void * ptr, const char * fileName, long fileLine)
{
	byte * mem = (byte *)pools.Alloc(size);
	if(ptr)
	{
		//Ищем получаем размер блока
		dword s = pools.Size(ptr);
		if(!s) s = SysSize(ptr);
		if(s > size) s = size;
		for(byte * dst = mem, * src = (byte *)ptr; s; s--)
		{
			*dst++ = *src++;
		}
		FreeInternal(ptr);
	}
	poolBlocks++;
	if(maxPoolBlocks < poolBlocks) maxPoolBlocks = poolBlocks;
	return mem;
}

//Освободить память в пуле
__forceinline bool MemoryManager::PoolFree(void * ptr)
{
	if(pools.Free(ptr))
	{
		poolBlocks--;
		return true;
	}	
	return false;
}

//Выделить память вне пула
__forceinline void * MemoryManager::MemAlloc(dword size, void * ptr, const char * fileName, long fileLine)
{
	if(ptr)
	{
		dword s = pools.Size(ptr);
		if(s)
		{
			//Перевыделение из пула
			byte * mem = (byte *)SysAlloc(size, null);
			if(s > size) s = size;
			for(byte * dst = mem, * src = (byte *)ptr; s; s--)
			{
				*dst++ = *src++;
			}
			FreeInternal(ptr);
			return mem;
		}
	}
	return SysAlloc(size, ptr);
}

//Освободить память
__forceinline void MemoryManager::FreeInternal(void * ptr)
{
	if(isPoolEnable)
	{
		if(PoolFree(ptr))
		{
			return;
		}
	}
	SysFree(ptr);
}

//Проверить блок памяти
void _fastcall MemoryManager::CheckBlock(MemoryBlock * block)
{
#ifndef MM_NO_DEBUGMEMORY
	if(!(block->openGuardBlock == openBlock))
	{
		if(IsValidateBlockPointer(block))
		{
			TraceDamagedBlocks();
			Assert(block->openGuardBlock == openBlock);
		}else{
			CoreLogOut("MemoryManager: Memory is damage (open guard block) 0x%.8x", block);
			Assert(false);
		}
	}
	if(!(block->descIndex >= 0 && block->descIndex < descsCount))
	{
		if(IsValidateBlockPointer(block))
		{
			TraceDamagedBlocks();
			Assert(block->descIndex >= 0 && block->descIndex < descsCount);
		}else{
			CoreLogOut("MemoryManager: Memory is damage (close guard block) 0x%.8x", block);
			Assert(false);
		}
	}
	if(!(descs[block->descIndex].block == block))
	{
		if(IsValidateBlockPointer(block))
		{
			TraceDamagedBlocks();
			Assert(descs[block->descIndex].block == block);
		}else{
			CoreLogOut("MemoryManager: Invalidate pointer 0x%.8x", block);
			Assert(false);
		}
	}
	if(!(*(GuardBlock *)((byte *)(block + 1) + descs[block->descIndex].blockSize) == closeBlock))
	{
		TraceDamagedBlocks();
		Assert(*(GuardBlock *)((byte *)(block + 1) + descs[block->descIndex].blockSize) == closeBlock);
	}
#endif
}

//Проверить на валидность поинтер
bool MemoryManager::IsValidateBlockPointer(MemoryBlock * block)
{
#ifndef MM_NO_DEBUGMEMORY
	for(long i = 0; i < descsCount; i++)
	{
		if(descs[i].block == block) return true;
	}
	return false;
#else
	return true;
#endif
}

//Проверить все блоки и вывести всю доступнукю информацию по разрушеным блокам
void MemoryManager::TraceDamagedBlocks()
{
#ifndef MM_NO_DEBUGMEMORY
	for(long i = 0; i < descsCount; i++)
	{
		MemoryBlock * block = descs[i].block;
		bool isDamaged = false;
		bool dmgOpenBlock = false;
		bool dmgCloseBlock = false;
		bool dmgIndex = false;
		if(!(block->openGuardBlock == openBlock))
		{
			isDamaged = true;
			dmgOpenBlock = true;
		}
		if(!(*(GuardBlock *)((byte *)(block + 1) + descs[i].blockSize) == closeBlock))
		{
			isDamaged = true;
			dmgCloseBlock = true;
		}
		if(block->descIndex != i)
		{
			isDamaged = true;
			dmgIndex = true;
		}
		if(!isDamaged) continue;
		CoreLogOut("%s\nMemory block is damaged", line);
		CoreLogOut("    addr: 0x%.8x", block);
		CoreLogOut("    size: %d", descs[i].blockSize);
		if(descs[i].descIndex >= 0)
		{
			DescString * ds = (DescString *)(strings + descs[i].descIndex);
			CoreLogOut("    alloc file name: %s", ds->fileName);
			CoreLogOut("    alloc file line: %i", ds->fileLine);
			CoreLogOut("    alloc blocks: %i", ds->numAllocBlocks);
		}
		if(dmgIndex)
		{
			CoreLogOut("    * Block desc index damaged");
		}
		if(dmgOpenBlock)
		{
			CoreLogOut("    * Open guard block damaged");
		}
		if(dmgCloseBlock)
		{
			CoreLogOut("    * Close guard block damaged");
		}
		CoreLogOut("    Previous block info");
		if(i > 0)
		{
			const char * file = "Unknown";
			long line = 0;
			if(descs[i - 1].descIndex >= 0)
			{
				DescString * ds = (DescString *)(strings + descs[i - 1].descIndex);
				file = ds->fileName;
				line = ds->fileLine;
			}			
			CoreLogOut("        alloc file name: %s", file);
			CoreLogOut("        alloc file line: %i", line);
		}else{
			CoreLogOut("        no previous blocks");
		}
		CoreLogOut("    Next block info");
		if(i + 1 < descsCount)
		{
			const char * file = "Unknown";
			long line = 0;
			if(descs[i + 1].descIndex >= 0)
			{
				DescString * ds = (DescString *)(strings + descs[i + 1].descIndex);
				file = ds->fileName;
				line = ds->fileLine;
			}			
			CoreLogOut("        alloc file name: %s", file);
			CoreLogOut("        alloc file line: %i", line);
		}else{
			CoreLogOut("        no next blocks");
		}
	}
#endif
}

//Получить индекс строки
__forceinline long MemoryManager::GetStringDescIndex(const char * fileName, long fileLine)
{
#ifndef MM_NO_DEBUGMEMORY
	static const char * emptyString = "";
	if(!fileName) fileName = emptyString;
	if(fileLine <= 0) fileLine = 0;
	//Ищим среди добавленных
	union GetDword
	{
		const char * str;
		struct
		{
			word lo;
			word hi;
		};
	} getDword;
	getDword.str = fileName;
	long eidx = (getDword.lo ^ getDword.hi ^ fileLine) & (ARRSIZE(entry) - 1);
	for(long i = entry[eidx]; i >= 0; )
	{
		DescString * desc = (DescString *)(strings + i);
		if(desc->fileNamePtr == fileName && desc->fileLine == fileLine)
		{
			return i;
		}
		i = desc->next;
	}
	//Добавляем
	long len = (long)strlen(fileName);
	if(stringsSize + len + sizeof(DescString) >= stringsMax)
	{
		stringsMax = (stringsMax + len + sizeof(DescString) + 4095) & ~4095;
		strings = (char *)SysAlloc(stringsMax, strings);
	}
	DescString & ds = (DescString &)*(strings + stringsSize);
	i = entry[eidx];
	if(i >= 0)
	{
		//Добавляемся в конец списка (для более качественной работы кэша)
		while(true)
		{
			DescString * desc = (DescString *)(strings + i);
			if(desc->next < 0) break;
			i = desc->next;
		}
		((DescString *)(strings + i))->next = stringsSize;
	}else{
		entry[eidx] = stringsSize;
	}
	ds.next = -1;
	ds.numAllocBlocks = 0;
	ds.fileNamePtr = fileName;
	ds.fileLine = fileLine;
	for(long i = 0; i <= len; i++) ds.fileName[i] = fileName[i];
	i = stringsSize;
	stringsSize += len + sizeof(DescString);
	return i;
#else
	return -1;
#endif
}

//-------------------------------------------------------------------------------------------------------
//Работа с системной памятью
//-------------------------------------------------------------------------------------------------------

//Выделить системную память
__forceinline void * MemoryManager::SysAlloc(dword size, void * ptr)
{
	//Обрабатываем блок 0-ого размера
	if(ptr == &emptyMemory)
	{
		ptr = null;
	}
	if(!size)
	{
		SysFree(ptr);
		return &emptyMemory;
	}
	//Перевыделяем (выделяем) память
	if(ptr)
	{
		totalSize -= SysSize(ptr);
	}
	totalSize += size;
	if(maxSize < totalSize) maxSize = totalSize;
	ptr = realloc(ptr, size);
#ifndef MM_NO_DEBUGMEMORY
	MEMORYSTATUS memStatus;
	GlobalMemoryStatus(&memStatus);
	dword s = dword(memStatus.dwTotalPhys - memStatus.dwAvailPhys);
	if(s > maxUsed360) maxUsed360 = s;
#endif
	return ptr;
}

//Освободить системную память
__forceinline void MemoryManager::SysFree(void * ptr)
{
	if(!ptr) return;
	if(ptr == &emptyMemory)
	{
		return;
	}
	totalSize -= SysSize(ptr);
	free(ptr);
}

//Получить размер системного блока
__forceinline dword MemoryManager::SysSize(void * ptr)
{
	if(!ptr) return 0;
	if(ptr == &emptyMemory) return 0;
	return (dword)_msize(ptr);
}

//-----------------------------------------------------------------------------------------------------------------
//MemoryPool
//-----------------------------------------------------------------------------------------------------------------

template<int Size> MemoryManager::MemoryPool<Size>::MemoryPool()
{
	minAddr = null;
	maxAddr = null;
	blocks = null;
	blocksCount = 0;
	blocksMax = 0;
}

template<int Size> MemoryManager::MemoryPool<Size>::~MemoryPool()
{
	if(blocks)
	{
		for(dword i = 0; i < blocksCount; i++)
		{
			MemoryManager::SysFree(blocks[i]);
		}
		MemoryManager::SysFree(blocks);
	}
	minAddr = null;
	maxAddr = null;
	blocks = null;
	blocksCount = 0;
	blocksMax = 0;
}

//Выделить память данного размера
template<int Size> void * MemoryManager::MemoryPool<Size>::Alloc()
{
	//Ищем кусок среди выделеных блоков
	for(dword i = 0; i < blocksCount; i++)
	{
		Block * b = blocks[i];
		for(long j = 0; j < (4096/Size + 31)/32; j++)
		{
			if(b->use[j] != 0xffffffff)
			{
				//Здесь есть свободный кусок, выделим его
				for(dword bit = 1, index = 0; b->use[j] & bit; bit <<= 1, index++);
				Assert(index < 32);
				b->use[j] |= bit;
				return AddAddress(b->pool[j*32 + index]);
			}
		}
	}
	//Ненашлось свободного куска, надо выделить новый блок
	if(blocksCount >= blocksMax)
	{
		blocksMax += 64;
		blocks = (Block **)MemoryManager::SysAlloc(blocksMax*sizeof(Block *), blocks);
	}
	Block * b = blocks[blocksCount++] = (Block *)MemoryManager::SysAlloc(sizeof(Block), null);
	for(long i = 0; i < ARRSIZE(b->use); i++)
	{
		b->use[i] = 0xffffffff;
	}
	for(long i = 0; i < 4096/Size/32; i++)
	{
		b->use[i] = 0;
	}
	for(i *= 32; i < 4096/Size; i++)
	{
		dword bit = 1 << (i & 31);
		b->use[i/32] &= ~bit;
	}
	//Выделим новый кусок в новом блоке
	b->use[0] |= 1;
	return AddAddress(b->pool[0]);
}

//Попытаться освободить память
template<int Size> bool MemoryManager::MemoryPool<Size>::Free(void * ptr)
{
	//Проверим глобально
	if(ptr < minAddr) return false;
	if(ptr > maxAddr) return false;
	//Проверим каждый блок в отдельности
	for(dword i = 0; i < blocksCount; i++)
	{
		if(ptr < blocks[i]->pool[0]) continue;
		if(ptr >= (byte *)blocks[i]->pool[0] + sizeof(blocks[i]->pool)) continue;
		break;
	}
	if(i >= blocksCount) return false;
	//Это тут, отметим удаление
	Block * b = blocks[i];
	Assert(((byte *)ptr - (byte *)b->pool)%Size == 0);
	long index = dword((byte *)ptr - (byte *)b->pool)/Size;
	dword bit = 1 << (index & 31);
	b->use[index/32] &= ~bit;
	return true;
}

//Добавить адрес в общий диапазон
template<int Size> __forceinline void * MemoryManager::MemoryPool<Size>::AddAddress(void * ptr)
{
	if(!minAddr || minAddr > ptr)
	{
		minAddr = ptr;
	}
	if(!maxAddr || maxAddr < ptr)
	{
		maxAddr = ptr;
	}
	return ptr;
}

//Получить размер блока по адресу
template<int Size> dword MemoryManager::MemoryPool<Size>::GetSize(void * ptr)
{
	//Проверим глобально
	if(ptr < minAddr) return false;
	if(ptr > maxAddr) return false;
	//Проверим каждый блок в отдельности
	for(dword i = 0; i < blocksCount; i++)
	{
		if(ptr < blocks[i]) continue;
		if(ptr >= (byte *)blocks[i]->pool[0] + sizeof(blocks[i]->pool)) continue;
		return Size;
	}
	return 0;
}

//-----------------------------------------------------------------------------------------------------------------
//MemoryPools
//-----------------------------------------------------------------------------------------------------------------


MemoryManager::MemoryPools::MemoryPools()
{
	pools[0] = &pool4;   pools[1] = &pool8; pools[2] = &pool12; pools[3] = &pool16;
	pools[4] = &pool20;  pools[5] = &pool24; pools[6] = &pool28; pools[7] = &pool32;
	pools[8] = &pool36;  pools[9] = &pool40; pools[10] = &pool44; pools[11] = &pool48;
	pools[12] = &pool52;  pools[13] = &pool56; pools[14] = &pool60; pools[15] = &pool64;
	pools[16] = &pool68;  pools[17] = &pool72; pools[18] = &pool76; pools[19] = &pool80;
	pools[20] = &pool84;  pools[21] = &pool88; pools[22] = &pool92; pools[23] = &pool96;
	minAddr = null;
	maxAddr = null;
}

__forceinline void * MemoryManager::MemoryPools::Alloc(dword size)
{
	Assert(size);
	dword index = (size - 1)/4;
	Assert(index < 24);
	void * mem = pools[index]->Alloc();
	Assert(mem);
	if(!minAddr || minAddr > mem)
	{
		minAddr = mem;
	}
	if(!maxAddr || maxAddr < mem)
	{
		maxAddr = mem;
	}
	return mem;
}

__forceinline bool MemoryManager::MemoryPools::Free(void * ptr)
{
	if(ptr > maxAddr) return false;
	if(ptr < minAddr) return false;
	for(long i = 0; i < 24; i++)
	{
		if(pools[i]->Free(ptr)) return true;
	}
	return false;
}

__forceinline dword MemoryManager::MemoryPools::Size(void * ptr)
{
	if(ptr > maxAddr) return 0;
	if(ptr < minAddr) return 0;
	dword s = 0;
	for(long i = 0; i < 24; i++)
	{		
		if(s = pools[i]->GetSize(ptr)) return s;
	}
	return 0;
}

