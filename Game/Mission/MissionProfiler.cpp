// Spirenkov Maxim

#include "MissionProfiler.h"


CREATE_CLASS(MissionProfiler)

//Указатель на единственный экземпляр профайлера
MissionProfiler * MissionProfiler::ptr = null;


MissionProfiler::MissionProfiler() : folders(_FL_, 1024),
										records(_FL_, 4096),
										missions(_FL_, 16),
										safeStrings(_FL_, 16384),
										timers(_FL_, 256)
{
	folders.Reserve(4096);
	records.Reserve(16384);
	missions.Reserve(16);
	safeStrings.Reserve(65536);
	for(long i = 0; i < findTableSize; i++)
	{
		findTable[i] = -1;
	}
	framesCounter = 0;
	profileTime = 0.0;
	profileTriMillionsTotal = 0.0;
	profileBatchesTotal = 0.0;
	profileTriCountMax = 0;
	profileBatchesMax = 0;
	detailLevel = 100;
	isStop = false;
	render = null;
	timers.Reserve(256);
	timersIdCounter = 0xf00d;
}

MissionProfiler::~MissionProfiler()
{
	api->DelObjectExecution(this, &MissionProfiler::Update);
	Out();
	ptr = null;
}

//Начать измерения
void MissionProfiler::Start(dword detail)
{
	if(!ptr)
	{
		api->CreateObject("MissionProfiler");
		Assert(ptr);
		ptr->detailLevel = detail;
	}
}

//Закончить измерения и вывести отчёт
void MissionProfiler::Stop()
{
	if(ptr)
	{
		ptr->isStop = true;
	}
}

//Инициализация
bool MissionProfiler::Init()
{
	render = (IRender *)api->GetService("DX9Render");
	Assert(render);
	if(ptr)
	{
		return false;
	}
	ptr = this;
	api->SetObjectExecution(this, "mission", 0x7fffff, &MissionProfiler::Update);
	return true;
}

//Обновление глобальных параметров
void __fastcall MissionProfiler::Update(float dltTime)
{
	profileTime += api->GetNoScaleDeltaTime();
	IRender::PerfomanceInfo renderPI;
	render->GetPerfomanceInfo(renderPI);
	profileTriMillionsTotal += renderPI.dwPolyCount*0.000001;
	profileBatchesTotal += (double)renderPI.dwBatchCount;
	if(profileTriCountMax < renderPI.dwPolyCount) profileTriCountMax = renderPI.dwPolyCount;
	if(profileBatchesMax < renderPI.dwBatchCount) profileBatchesMax = renderPI.dwBatchCount;
	framesCounter++;
	if(isStop)
	{
		api->DelObjectExecution(this, &MissionProfiler::Update);	
		delete this;
	}
}

//Добавить запись профайла
void MissionProfiler::AddProfileRecord(const ProfileData & data)
{
	AssertCoreThread
	//Проверяем базовые указатели
	Assert(data.mission);
	Assert(data.object);
	Assert(data.func);
	//Папка на объект
	ObjectFolder & folder = GetFolder(data.mission, data.object, data.objectID, data.objectType);
	//Запись объекта
	Record & record = GetRecord(folder, data.func, data.level);
	record.count++;
	if(record.max < data.time)
	{
		record.max = data.time;
	}
	record.sum += (double)data.time;
}

//Добавить запись профайла
void MissionProfiler::AddProfileRegistry(const ProfileData & data)
{
	AssertCoreThread
	//Проверяем базовые указатели
	Assert(data.mission);
	Assert(data.object);
	//Папка на объект
	ObjectFolder & folder = GetFolder(data.mission, data.object, data.objectID, data.objectType);
	if(data.func)
	{
		//Запись объекта
		Record & record = GetRecord(folder, data.func, data.level);
		record.regCount++;		
	}else{
		folder.regCount++;
	}
}

//Добавить запись профайла
void MissionProfiler::AddProfileUnregistryLevel(const ProfileData & data)
{
	AssertCoreThread
	//Проверяем базовые указатели
	Assert(data.mission);
	Assert(data.object);
	Assert(data.func);
	//Папка на объект
	ObjectFolder & folder = GetFolder(data.mission, data.object, data.objectID, data.objectType);
	//Запись объекта
	Record & record = GetRecord(folder, data.func, data.level);
	record.unregCount++;
}

//Добавить запись профайла
void MissionProfiler::AddProfileUnregistry(const ProfileData & data)
{
	AssertCoreThread
	//Проверяем базовые указатели
	Assert(data.mission);
	Assert(data.object);
	//Папка на объект
	ObjectFolder & folder = GetFolder(data.mission, data.object, data.objectID, data.objectType);
	if(folder.record >= 0)
	{
		//Проходим по цепочке		
		for(long i = folder.record; i >= 0; )
		{
			Record & rec = records[i];
			if(data.func)
			{
				if(rec.func == data.func)
				{
					rec.unregCount++;
				}
			}else{
				rec.unregCount++;
			}
			i = rec.next;
		}
	}else{
		folder.unregCount++;
	}

}


//Найти/добавить папку на объект
MissionProfiler::ObjectFolder & MissionProfiler::GetFolder(IMission * mission, void * object, const char * objectId, const char * objectType)
{
	//Ищем индекс миссии
	MissionInfo * miss = missions.GetBuffer();
	dword misSize = missions.Size();
	for(dword i = 0; i < misSize; i++)
	{
		if(miss[i].mission == mission)
		{
			break;
		}
	}
	long missionIndex = i;
	if(i >= misSize)
	{
		missionIndex = missions.Add();
		missions[missionIndex].mission = mission;
		missions[missionIndex].name = SafeString(mission->GetMissionName());
	}	
	//Ищем папку записей на данный объект
	long num = (byte *)object - (byte *)null;
	long hash = num ^ (num >> 4) ^ (num >> findTableBase) ^ (num >> findTableBase*2);
	dword findIndex = hash & findTableMask;
	ObjectFolder * newFolder = null;
	if(findTable[findIndex] >= 0)
	{
		//Проходим по цепочке папок с данным хэшем
		long i = findTable[findIndex];
		while(true)
		{
			ObjectFolder & oh = folders[i];
			if(oh.object == object && oh.missionIndex == missionIndex)
			{
				return oh;
			}
			if(oh.next < 0)
			{
				//Не нашли записи, добавим новую
				newFolder = &folders[oh.next = folders.Add()];
				break;
			}
			i = oh.next;
		}
	}else{
		//Нет ещё записей в данной цепочке
		newFolder = &folders[findTable[findIndex] = folders.Add()];
	}
	if(newFolder)
	{
		newFolder->next = -1;
		newFolder->record = -1;
		newFolder->object = object;
		newFolder->missionIndex = missionIndex;
		newFolder->name = SafeString(objectId);
		newFolder->type = SafeString(objectType);
		newFolder->regCount = 0;
		newFolder->unregCount = 0;
	}
	return *newFolder;
}

//Найти/добавить запись к объекту
MissionProfiler::Record & MissionProfiler::GetRecord(ObjectFolder & folder, void * func, long level)
{
	Record * newRecord = null;
	if(folder.record >= 0)
	{
		//Проходим по цепочке
		long i = folder.record;
		while(true)
		{
			Record & rec = records[i];
			if(rec.func == func && rec.level == level)
			{
				return rec;
			}
			if(rec.next < 0)
			{
				i = records.Add();
				rec.next = i;
				newRecord = &records[i];
				break;
			}
			i = rec.next;
		}
	}else{
		//Добавляем новую запись
		long i = records.Add();
		folder.record = i;
		newRecord = &records[i];
	}
	//Заполняем параметры новой записи
	newRecord->next = -1;
	newRecord->func = func;
	newRecord->level = level;
	newRecord->count = 0;
	newRecord->max = 0;
	newRecord->sum = 0.0f;
	newRecord->regCount = 0;
	newRecord->unregCount = 0;
	return *newRecord;
}

//Вывести отчёт
void MissionProfiler::Out()
{
	array<SortObjects> sortList(_FL_, 4096);
	static const char * stopLine = "===========================================================================";
	api->Trace(stopLine);
	api->Trace("Mission object profile info start");
	api->Trace(stopLine);
	api->Trace("");
	//Выводим последовательно статистику для каждой миссии
	double full = 0.0;
	for(long m = 0; m < missions; m++)
	{
		api->Trace("Mission: %s", GetString(missions[m].name));
		api->Trace("");
		api->Trace(stopLine);
		api->Trace("");
		//Собираем статистику по типам
		sortList.Empty();
		double summary = 0.0;
		for(long i = 0; i < folders; i ++)
		{
			ObjectFolder & folder = folders[i];
			if(folder.missionIndex != m)
			{
				continue;
			}
			double middle = 0.0;
			for(long ri = folder.record; ri >= 0; )
			{
				Record & r = records[ri];
				if(r.count > 0)
				{
					middle += r.sum/r.count;
				}
				ri = r.next;
			}
			summary += middle;
			//Если добавлен такой тип то сумируем статистику
			const char * type = GetString(folders[i].type);
			for(long j = 0; j < sortList; j++)
			{
				SortObjects & so = sortList[j];
				if(string::IsEqual(type, GetString(folders[so.index].type)))
				{
					so.count++;
					so.middle += middle;
					if(so.max < middle)
					{
					   so.max = middle;
					}
					break;
				}
			}
			if(j >= sortList)
			{
				//Добавляем новую запись на новый тип
				SortObjects & so = sortList[sortList.Add()];
				so.index = i;
				so.count = 1;
				so.middle = middle;
				so.max = middle;
			}			
		}
		full += summary;
		//Сортируем
		sortList.QSort(&MissionProfiler::QSort);
		//Выводим статистику по типам
		api->Trace("Brake per type");
		api->Trace("");
		api->Trace("Brake      percents                            objectType      objects count           average                max");
		api->Trace("");
		long count = sortList.Size();
		if(detailLevel == 0)
		{
			if(count > 10) count = 10;
		}
		for(long i = 0; i < count; i++)
		{
			SortObjects & so = sortList[i];
			api->Trace("%5.0i        %6.2f        %30s     %8d         %15.0f    %15.0f",
				i + 1,
				so.middle/summary*100.0,
				GetString(folders[so.index].type),
				so.count,
				so.middle/so.count + 0.5,
				so.max + 0.5);
		}
		if(detailLevel < 2)
		{
			continue;
		}
		//Собираем статистику по объектам в список
		sortList.Empty();
		for(long i = 0; i < folders; i ++)
		{
			ObjectFolder & folder = folders[i];
			if(folder.missionIndex != m)
			{
				continue;
			}			
			SortObjects so;
			so.index = i;
			so.count = 0;
			so.middle = 0.0f;
			so.max = 0.0f;
			for(long ri = folder.record; ri >= 0; )
			{
				Record & r = records[ri];
				so.count++;
				if(r.count > 0)
				{
					so.middle += r.sum/r.count;
					so.max += r.max;
				}
				ri = r.next;
			}
			if(so.count > 0)
			{
				so.max /= so.count;
			}
			if(so.index >= 0)
			{
				sortList.Add(so);
			}
		}
		//!!!
		for(long i = 0; i < sortList; i++)
		{
			for(long j = i + 1; j < sortList; j++)
			{
				Assert(sortList[i].index != sortList[j].index);
			}
		}


		//Сортируем
		sortList.QSort(&MissionProfiler::QSort);
		//Выводим
		api->Trace("");
		api->Trace("Brake per object");
		api->Trace("");
		api->Trace("Brake     percents                                              objectID                        objectType                average                    max");
		api->Trace("");
		for(long i = 0; i < sortList; i++)
		{
			SortObjects & so = sortList[i];
			char percents[16];
			Print(percents, sizeof(percents), "%1.2f", (double)so.middle/summary*100.0, false);
			api->Trace("%5.0i     %s  %45s    %30s    %19.1f    %19.1f",
				i + 1, 
				percents,
				GetString(folders[so.index].name),
				GetString(folders[so.index].type),
				so.middle + 0.05,
				so.max + 0.05);
			if(detailLevel > 3)
			{
				const char * separator = "    ----------------------------------------------------------------------------------------------------------------------------------------------------";
				api->Trace(separator);				
				dword totalRegCount = folders[so.index].regCount;
				dword totalUnregCount = folders[so.index].unregCount;
				for(long ri = folders[so.index].record; ri >= 0; ri = records[ri].next)
				{
					totalRegCount += records[ri].regCount;
					totalUnregCount += records[ri].unregCount;
				}
				char treg[12];
				Print(treg, sizeof(treg), "%1.0f", (double)totalRegCount, false);
				char tureg[12];
				Print(tureg, sizeof(tureg), "%1.0f", (double)totalUnregCount, false);
				char oreg[12];
				Print(oreg, sizeof(oreg), "%1.0f", (double)folders[so.index].regCount, false);
				char oureg[12];
				Print(oureg, sizeof(oureg), "%1.0f", (double)folders[so.index].unregCount, false);
				api->Trace("    |     total reg: %s  total unreg: %s  obj reg: %s  obj unreg: %s  |", treg, tureg, oreg, oureg);
				if(folders[so.index].record >= 0)
				{
					api->Trace(separator);
					//Расписываем детально вызовы
					for(long ri = folders[so.index].record; ri >= 0; ri = records[ri].next)
					{
						Record & r = records[ri];
						char lpercents[10];
						double average;
						if(r.count >= 1)
						{
							average = r.sum/r.count;
							Print(lpercents, sizeof(lpercents), "%1.4f", average/summary*100.0, false);
						}else{
							average = 0.0;
							Print(lpercents, sizeof(lpercents), "%1.4f", 0.0, false);
						}					
						char level[12];
						Print(level, sizeof(level), "%1.0f", (double)r.level, true);
						char count[15];
						Print(count, sizeof(count), "%1.0f", (double)r.count, false);
						char reg[12];
						Print(reg, sizeof(reg), "%1.0f", (double)r.regCount, false);
						char unreg[12];
						Print(unreg, sizeof(unreg), "%1.0f", (double)r.unregCount, false);
						api->Trace("    |     %s      level:%s  count: %s  reg: %s  unreg: %s   |   %19.1f    %19.1f",
							lpercents,
							level,
							count,
							reg,
							unreg,
							average + 0.049,
							r.max + 0.049);
					}
				}
				api->Trace(separator);
			}

		}		

	}
	api->Trace("");
	api->Trace(stopLine);
	api->Trace("Avirage total ticks per frame: %.2f seconds", full);	
	api->Trace("Profiling counter: %u frames", framesCounter);
	api->Trace("Profiling time: %5.2f seconds", profileTime);
	api->Trace("Avirage FPS: %5.2f", framesCounter/profileTime);
	if(framesCounter)
	{
		api->Trace("Avirage triangles count per frame: %8.1f", (profileTriMillionsTotal/framesCounter)*1000000.0);
		api->Trace("Avirage batches count per frame: %8.1f", profileBatchesTotal/framesCounter);
		api->Trace("Maximum triangles count: %u", profileTriCountMax);
		api->Trace("Maximum batches count: %u", profileBatchesMax);
	}
	api->Trace(stopLine);
	api->Trace("Mission object profile info end");
	api->Trace(stopLine);
	api->Trace("\n\n");
}

//Сортировка по объектам
bool MissionProfiler::QSort(SortObjects const & grtElm, SortObjects const & lesElm)
{
	return lesElm.middle < grtElm.middle;
}

//Сохранить строку
__forceinline MissionProfiler::safestring MissionProfiler::SafeString(const char * str)
{
	safestring retValue = safeStrings.Size();
	while(*str)
	{
		safeStrings.Add(*str++);
	}
	safeStrings.Add(0);
	return retValue;
}

//Получить сохранёную строку
__forceinline const char * MissionProfiler::GetString(safestring str)
{
	return &safeStrings[str];
}

//Печать чисел выровненых по левой границе
__forceinline void MissionProfiler::Print(char * buffer, dword size, const char * format, double num, bool sign)
{
	crt_snprintf(buffer, size, format, num);
	for(dword i = 0; i < size - 1; i++)
	{
		if(!buffer[i])
		{
			for(; i < size - 1; i++)
			{
				buffer[i] = ' ';
			}
			break;
		}
	}
	if(sign && buffer[0] != '-')
	{
		for(long i = size - 1; i > 0; i--)
		{
			buffer[i] = buffer[i - 1];
		}
		buffer[0] = ' ';
	}
	buffer[size - 1] = 0;
}








