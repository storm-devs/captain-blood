// Spirenkov Maxim

#ifndef _MissionObjectsList_h_
#define _MissionObjectsList_h_


#include "MissionProfiler.h"


class MissionObjectsList
{
public:
	//Исполняемая функция
	union Func
	{
		MOF_EVENT funcEvent;
		MOF_UPDATE funcUpdate;
		void * ptr;
	};

private:
	//Параметры хэшь-таблицы
	enum Consts
	{
		findTableBase = 10,
		findTableSize = (1 << findTableBase),
		findTableMask = findTableSize - 1,
		optimizeStepsPerFrame = 16,
	};

	//Елемент записи в группе
	struct Element
	{
		MissionObject * object;		//Указатель на зарегистрированный объект
		Func func;					//Исполняемая функция
		long nextExecute;			//Следующий в списке исполнения
		long prevExecute;			//Предыдущий в списке исполнения
		union
		{
			long nextFind;			//Следующий в списке поиска
			long nextFree;			//Следующий в списке свободных
		};
		long level;					//Текущий уровень
	};

	//Уровень
	struct Level
	{
		long level;					//Текущий уровень
		long firstElement;			//Первый в списке исполняемых на данном уровне
	};

	class IteratorsPull;

	//Класс для перебора элементов группы
	class MissionIterator : public MGIterator
	{
		enum State
		{
			s_not_initialized,
			s_finishing,			
			s_ok,
			s_moved_to_next,
			s_forcedword = 0x7fffffff
		};

	public:
		MissionIterator();
		virtual ~MissionIterator();
		//Закончить ли цикл
		virtual bool IsDone();
		//Взять следующий элемент
		virtual void Next();
		//Получить элемент
		virtual MissionObject * Get();
		//Узнать уровень на котором находиться запись
		virtual long Level();
		//Освободить итератор
		virtual void Release();
		//Установить итератор на начало
		virtual void Reset();
		//Исполнить эвент, если это возможно
		virtual void ExecuteEvent(MissionObject * sender);
		//Исполнить обновление	
		void ExecuteUpdate(float dltTime);
		//Удаление элемента
		static void DeleteElement(long index, MissionIterator * itr);
	
	public:
		//Инициализировать итератор
		MissionIterator * Init(IteratorsPull * myPull, MissionObjectsList * objList, const char * file, long line);
		//Сообщение что не удалён
		void TraceError();

	private:
		State state;					//Текущее состояние
		long currentElement;			//Индекс пекущего элемента в группе
		MissionObjectsList * list;		//Список, который используем в текущий момент времени
		MissionIterator * next;			//Следующий активный в данной группе элемент
		MissionIterator * prev;			//Предыдущий активный в данной группе элемент
		const char * cpp_file;			//Файл откуда был создан итератор
		long cpp_line;					//Строка Файла откуда был создан итератор
	private:
		IteratorsPull * pull;			//Пул, в котором находимся
	};

	friend class MissionIterator;

	class IteratorsPull
	{
	private:
		IteratorsPull();
		~IteratorsPull();

	public:
		//Зарезервировать использование пулов
		static void Create();
		//Освободить использование пулов
		static void Release();

	public:
		//Создать итератор
		static MissionIterator * CreateIterator(MissionObjectsList * list, const char * file, long line);

		//Занять/освободить итератор
		void Use(MissionIterator * it, bool isUse);

	private:
		dword use;						//Используемые итераторы
		IteratorsPull * next;			//Следующий пул
		static IteratorsPull * first;	//Первый в списке пулов итераторов
		static long refCounter;			//Счётчик заведённых групп для менеджмента пулов
		MissionIterator iterators[32];	//Пул итераторов		
	};


public:
	MissionObjectsList();
	~MissionObjectsList();
	void SetId(GroupId _id);

public:
	//Добавить подписку
	void Add(MissionObject * object, long level, Func func);
	//Отписать объект
	void Del(MissionObject * object, long level, Func func);
	//Отписать объект
	void Del(MissionObject * object, Func func);
	//Отписать объект
	void Del(MissionObject * object);
	//Распределённая оптимизация списка, возвращает true когда можно передать оптимизацию следующему списку
	bool OptimizeStep();
	
	//Создать итератор
	MGIterator * CreateIterator(const char * file, long line);
	
	//Исполнить список
	void ExecuteUpdate(float dltTime, MGIterator * it);

	//Удалить все объекты группы
	void DeleteList();
	
	//Определить в списке объект или нет
	bool InList(MissionObject * object, dword uid, long & hashIndex);

private:
	//Получить указатель на первый итератор данного списка
	MissionIterator * & GetPtrForFirstIterator();
	//Получить индекс первого элемента в списке
	long GetFirstElement();
	//Получить индекс следующего элемента в списке
	long GetNextElement(long current);
	//Получить объект данного элемента
	MissionObject * GetElementObject(long current);
	//Получить уровень данного элемента
	long GetElementLevel(long current);

	//Исполнить событие для итератора
	void ExecuteEventForIterator(long elementIndex, MissionObject * sender);
	//Исполнить функцию обновления для итератора
	void ExecuteUpdateForIterator(long elementIndex, float dltTime);

	//Заполнить данные профайла об объекте
	void FillProfileData(MissionProfiler::ProfileData & data, MissionObject * mo, Func func, long level);
	


private:
	//Удалить элемент
	void DeleteElement(long & findIndex);
	//Найти, а если нет, добавить запись для заданного уровня
	long GetLevelEntry(long level);
	//Вычислить индекс для входной таблице по адресу
	dword GetEntryIndexFromPtr(MissionObject * object);

private:	
	array<Element> elements;			//Список элементов группы
	array<Level> levels;				//Список уровней
	long findTable[findTableSize];		//Таблица быстрого поиска объекта
	long firstExecute;					//Первый исполняемый
	long lastExecute;					//Последний исполняемый
	long firstFree;						//Первый свободный
	dword currentOptimizeLevel;			//Текущий проверяемый уровень
	MissionIterator * iterators;		//Итераторы данной группы
	GroupId id;							//Тип списка
#ifndef STOP_PROFILES
	string profileObjectId;				//Сохранённый идентификатор объекта
	string profileObjectType;			//Сохранённый тип объекта
#endif
};


#endif
