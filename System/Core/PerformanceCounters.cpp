

#include "PerformanceCounters.h"


PerformanceCounters::PerformanceCounters() : counters(_FL_)
{
	for(long i = 0; i < ARRSIZE(entryTable); i++)
	{
		entryTable[i] = -1;
	}
}

PerformanceCounters::~PerformanceCounters()
{
}

//Добавить значение к счётчику, который сбросится с конце кадра
void PerformanceCounters::AddPerformanceCounter(const char * name, float value)
{
	Element & el = GetCounter(name);
	el.current += value;
}

//Установить счётчик, который сбросится с конце кадра
void PerformanceCounters::SetPerformanceCounter(const char * name, float value)
{
	Element & el = GetCounter(name);
	el.current = value;	
}

//Получить количество счётчиков
dword PerformanceCounters::GetNumberOfPerformanceCounters()
{
	return counters.Size();
}

//Получить имя счётчика
const char * PerformanceCounters::GetPerformanceName(long index)
{
	if(index >= 0 && index < counters)
	{
		return counters[index].name;
	}
	return null;
}

//Получить значение счётчика с предыдущего кадра
float PerformanceCounters::GetPerformanceCounter(long index)
{
	if(index >= 0 && index < counters)
	{
		return counters[index].counter;
	}
	return 0.0f;
}

//Получить значение счётчика с предыдущего кадра
float PerformanceCounters::GetPerformanceCounter(const char * name)
{
	Element & el = GetCounter(name);
	return el.counter;
}

//Момент следующего отсчёта
void PerformanceCounters::NextFrame()
{
	for(long i = 0; i < counters; i++)
	{
		Element & el = counters[i];
		el.counter = el.current;
		el.current = 0.0f;
#ifdef _XBOX
		PIXAddNamedCounter(el.counter, el.name);
#endif
	}
}

//Найти или добавить счётчик
PerformanceCounters::Element & PerformanceCounters::GetCounter(const char * name)
{
	if(!name) name = "";
	dword len;
	dword hash = string::Hash(name, len);
	long index = hash & (ARRSIZE(entryTable) - 1);
	long i = entryTable[index];
	long last = i;
	while(i >= 0)
	{
		
		Element & el = counters[i];
		if(el.hash == hash)
		{
			if(el.name.Len() == len)
			{
				if(strcmp(el.name, name) == 0)
				{
					break;
				}
			}
		}
		i = el.next;
	}
	if(i < 0)
	{
		Element & el = counters[i = counters.Add()];
		el.current = 0.0f;
		el.counter = 0.0f;
		el.hash = hash;
		el.name = name;
		el.next = -1;
		if(last >= 0)
		{
			counters[last].next = i;
		}else{
			entryTable[index] = i;
		}
	}
	return counters[i];
}

