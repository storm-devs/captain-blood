
#ifndef _PerformanceCounters_h_
#define _PerformanceCounters_h_

#include "CoreBase.h"

class PerformanceCounters
{
	struct Element
	{
		dword hash;
		long next;
		string name;
		float current;
		float counter;
	};
public:
	PerformanceCounters();
	~PerformanceCounters();

public:
	//Добавить значение к счётчику, который сбросится с конце кадра
	void AddPerformanceCounter(const char * name, float value);
	//Установить счётчик, который сбросится с конце кадра
	void SetPerformanceCounter(const char * name, float value);
	//Получить количество счётчиков
	dword GetNumberOfPerformanceCounters();
	//Получить имя счётчика
	const char * GetPerformanceName(long index);
	//Получить значение счётчика с предыдущего кадра
	float GetPerformanceCounter(long index);
	//Получить значение счётчика с предыдущего кадра
	float GetPerformanceCounter(const char * name);

	//Момент следующего отсчёта
	void NextFrame();

private:
	//Найти или добавить счётчик
	Element & GetCounter(const char * name);

private:
	long entryTable[4096];
	array<Element> counters;
};

#endif
