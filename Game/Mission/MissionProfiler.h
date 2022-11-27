// Spirenkov Maxim

#ifndef _MissionProfiler_h_
#define _MissionProfiler_h_

#include "..\..\Common_h\Mission.h"


//#define PROFILER_USE_STACK_TIMERS


class MissionProfiler : public RegObject
{
	//Параметры хэшь-таблицы для пообъектных записей
	enum Consts
	{
		findTableBase = 12,
		findTableSize = (1 << findTableBase),
		findTableMask = findTableSize - 1,
	};


	typedef long safestring;

	//Папка запивей на данный объект
	struct ObjectFolder
	{
		void * object;				//Указатель на зарегистрированный объект
		long next;					//Следующий в списке
		long record;				//Записи завёденные на данный объект
		long missionIndex;			//Миссия в которой находиться объект
		safestring name;			//Имя объекта
		safestring type;			//Тип объекта
		dword regCount;				//Количество подписок в группы
		dword unregCount;			//Количество отписок из групп
	};

	//Запись профайла
	struct Record
	{
		long next;					//Следующий в списке		
		void * func;				//Функция, подписаная на данном уровне
		long level;					//Уровень подписки
		unsigned __int64 count;		//Счётчик добавлений
		unsigned __int64 max;		//Максимальное время исполнения
		double sum;					//Сумарное время исполнения		
		dword regCount;				//Количество подписок в группы
		dword unregCount;			//Количество отписок из групп
	};

	//Учтёное имя миссии
	struct MissionInfo
	{
		IMission * mission;			//Миссия
		safestring name;			//Имя миссии
	};

	//Пообъектная сортировка
	struct SortObjects
	{
		long index;					//Индекс папки объекта
		dword count;				//Количество объектов
		double middle;				//Среднее сумарное количество времени затраченное объектом
		double max;					//Максимальное количество времени затраченное объектом
	};

	//Таймер для измерения временного промежутка
	struct Timer
	{
		ProfileTimer timer;			//Таймер
		unsigned __int64 subTime;	//Время, которое надо вычесть из результата
		dword checkId;				//Идентификатор для контроля за правильностью использования стека
	};

	

public:
	struct ProfileData
	{
		IMission * mission;
		void * object;
		const char * objectID;
		const char * objectType;
		void * func;		
		long level;
		unsigned __int64 time;
	};

public:
	MissionProfiler();
	~MissionProfiler();

	//Начать измерения
	static void Start(dword detail);
	//Закончить измерения и вывести отчёт
	static void Stop();
	//Активен ли профайлер
	static bool IsActive();
	//Добавить запись профайла
	static void AddRecord(const ProfileData & data);
	//Добавить запись профайла
	static void RegistryInGroup(const ProfileData & data);
	//Добавить запись профайла
	static void UnregistryInGroupLevel(const ProfileData & data);
	//Добавить запись профайла
	static void UnregistryInGroup(const ProfileData & data);
	//Запустить счётчик и получить его идентификатор
	static dword StartProfileTimer();
	//Остановить счётчик
	static unsigned __int64 StopProfileTimer(dword id);
	
private:
	//Инициализация
	virtual bool Init();
	//Обновление глобальных параметров
	void __fastcall Update(float dltTime);
	//Добавить запись профайла
	void AddProfileRecord(const ProfileData & data);
	//Добавить запись профайла
	void AddProfileRegistry(const ProfileData & data);
	//Добавить запись профайла
	void AddProfileUnregistryLevel(const ProfileData & data);
	//Добавить запись профайла
	void AddProfileUnregistry(const ProfileData & data);
	//Найти/добавить папку на объект
	ObjectFolder & GetFolder(IMission * mission, void * object, const char * objectId, const char * objectType);
	//Найти/добавить запись к объекту
	Record & GetRecord(ObjectFolder & folder, void * func, long level);
	//Вывести отчёт
	void Out();


private:
	//Сортировка по объектам
	static bool QSort(SortObjects const & grtElm, SortObjects const & lesElm);
	//Сохранить строку
	safestring SafeString(const char * str);
	//Получить сохранёную строку
	const char * GetString(safestring str);
	//Печать чисел выровненых по левой границе
	void Print(char * buffer, dword size, const char * format, double num, bool sign);

private:
	long findTable[findTableSize];	//Таблица быстрого поиска объекта
	array<ObjectFolder> folders;	//Добавленные в профайл папки объектов
	array<Record> records;			//Добавленные в профайл записи профайла
	array<MissionInfo> missions;	//Исполненые миссии
	array<char> safeStrings;		//Сохранёные строки
	dword framesCounter;			//Счётчик кадров
	double profileTime;				//Общее время профайла
	double profileTriMillionsTotal;	//Сколько милионов треугольников нарисовано всего
	double profileBatchesTotal;		//Сколько отрисовок было всего
	dword profileTriCountMax;		//Максимальное количество треугольников в кадре
	dword profileBatchesMax;		//Максимальное количество отрисовок в кадре
	dword detailLevel;				//Форма отчёта, чем больше тем детальней
	bool isStop;					//Закончиваеться профайлинг до конца кадра
	IRender * render;				//IRender
	array<Timer>  timers;			//Стек таймеров
	dword timersIdCounter;			//Счётчик идентификаторов

private:
	static MissionProfiler * ptr;	//Указатель на единственный экземпляр профайлера
};


//Активен ли профайлер
__forceinline bool MissionProfiler::IsActive()
{
	return (ptr != null);
}

//Добавить запись профайла
__forceinline void MissionProfiler::AddRecord(const ProfileData & data)
{
	if(ptr)
	{
		ptr->AddProfileRecord(data);
	}
}

//Добавить запись профайла
__forceinline void MissionProfiler::RegistryInGroup(const ProfileData & data)
{
	if(ptr)
	{
		ptr->AddProfileRegistry(data);
	}
}

//Добавить запись профайла
__forceinline void MissionProfiler::UnregistryInGroupLevel(const ProfileData & data)
{
	if(ptr)
	{
		ptr->AddProfileUnregistryLevel(data);
	}
}

//Добавить запись профайла
__forceinline void MissionProfiler::UnregistryInGroup(const ProfileData & data)
{
	if(ptr)
	{
		ptr->AddProfileUnregistry(data);
	}
}

//Запустить счётчик и получить его идентификатор
__forceinline dword MissionProfiler::StartProfileTimer()
{
#ifndef STOP_DEBUG
	Timer & t = ptr->timers[ptr->timers.Add()];
	t.checkId = ptr->timersIdCounter++;
	t.subTime = 0;
	t.timer.Start();
	return t.checkId;
#else
	return 0;
#endif
}

//Остановить счётчик
__forceinline unsigned __int64 MissionProfiler::StopProfileTimer(dword id)
{
#ifndef STOP_DEBUG
	Timer & t = ptr->timers[ptr->timers - 1];
	t.timer.Stop();
	Assert(id == t.checkId);
	if(ptr->timers >= 2)
	{
		ptr->timers[ptr->timers - 2].subTime += t.timer.GetTime64();
	}
	unsigned __int64 time = t.timer.GetTime64() - t.subTime;
	ptr->timers.DelIndex(ptr->timers - 1);
	return time;
#else
	return 0;
#endif
}


#endif

