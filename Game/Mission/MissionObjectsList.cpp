// Spirenkov Maxim

#include "MissionObjectsList.h"
#include "MissionProfiler.h"

MissionObjectsList::MissionObjectsList() : elements(_FL_, 1),
											levels(_FL_, 256)

{
	IteratorsPull::Create();
	for(long i = 0; i < findTableSize; i++)
	{
		findTable[i] = -1;
	}
	firstExecute = -1;
	lastExecute = -1;
	firstFree = -1;
	currentOptimizeLevel = 0;
	iterators = null;
	id = MG_ERROR_ID;
}

MissionObjectsList::~MissionObjectsList()
{
	elements.Empty();
	levels.Empty();
	IteratorsPull::Release();
}

void MissionObjectsList::SetId(GroupId _id)
{
	id = _id;
}

void MissionObjectsList::Add(MissionObject * object, long level, Func func)
{
#ifndef STOP_PROFILES
	//Если профайлим то регистрируем событие
	if(MissionProfiler::IsActive())
	{
		MissionProfiler::ProfileData data;
		FillProfileData(data, object, func, level);
		MissionProfiler::RegistryInGroup(data);
	}
#endif
//	Assert(level >= ML_FIRST);
//	Assert(level <= ML_LAST);
	//Ищем среди добавленых на данный уровень
	dword entryIndex = GetEntryIndexFromPtr(object);
	long index = findTable[entryIndex];
	while(index >= 0)
	{
		Element & el = elements[index];
		if(el.object == object)
		{
			if(el.level == level)
			{
				if(el.func.funcUpdate == func.funcUpdate)
				{
					return;
				}
			}
		}
		index = el.nextFind;
	}
	//Добавляем новую запись
	if(firstFree >= 0)
	{
		index = firstFree;
		firstFree = elements[index].nextFree;
	}else{
		elements.SetAddElements(coremax(coremin((elements.Size() + 0xf) & ~0xf, 256), 16));
		index = elements.Add();
	}
	Element & el = elements[index];
	//Заполняем
	el.object = object;
	el.func = func;
	el.level = level;
	//Включаем в список поиска
	el.nextFind = findTable[entryIndex];	
	findTable[entryIndex] = index;
	//Определяем концы цепочки, куда влазить
	long levelIndex = GetLevelEntry(level);
	Level & lvl = levels[levelIndex];
	long * prev = null;
	long * next = null;
	if(lvl.firstElement >= 0)
	{
		//Будем встраиваться перед первым на этом уровне
		next = &elements[lvl.firstElement].prevExecute;
	}else{
		//Пытаемся найти кого-то уровнем выше
		for(levelIndex++; levelIndex < levels; levelIndex++)
		{
			if(levels[levelIndex].firstElement >= 0)
			{
				break;
			}
		}
		if(levelIndex < levels)
		{
			//Встраиваемся перед первым с более высоким уровнем
			Level & lvl_next = levels[levelIndex];
			next = &elements[lvl_next.firstElement].prevExecute;
		}else{
			//Будем самыми последними в списке
			next = &lastExecute;
		}
	}
	if(*next >= 0)
	{
		prev = &elements[*next].nextExecute;
	}else{
		prev = &firstExecute;
	}
	//Включаем в список исполняемых новый элемент
	el.prevExecute = *next;
	el.nextExecute = *prev;
	*next = index;
	*prev = index;
	lvl.firstElement = index;
}

void MissionObjectsList::Del(MissionObject * object, long level, Func func)
{
#ifndef STOP_PROFILES
	//Если профайлим то регистрируем событие
	if(MissionProfiler::IsActive())
	{
		MissionProfiler::ProfileData data;
		FillProfileData(data, object, func, level);
		MissionProfiler::UnregistryInGroupLevel(data);
	}
#endif
	//Ищем среди добавленых на данный уровень
	dword entryIndex = GetEntryIndexFromPtr(object);
	long * indexPtr = &findTable[entryIndex];
	while(*indexPtr >= 0)
	{
		Element & el = elements[*indexPtr];
		if(el.object == object)
		{
			if(el.level == level)
			{
				if(el.func.funcUpdate == func.funcUpdate)
				{
					DeleteElement(*indexPtr);
					return;
				}
			}
		}
		indexPtr = &el.nextFind;
	}
}

void MissionObjectsList::Del(MissionObject * object, Func func)
{
#ifndef STOP_PROFILES
	//Если профайлим то регистрируем событие
	if(MissionProfiler::IsActive() && object)
	{
		MissionProfiler::ProfileData data;
		FillProfileData(data, object, func, 0);
		MissionProfiler::UnregistryInGroup(data);
	}
#endif
	//Ищем среди добавленых на данный уровень
	dword entryIndex = GetEntryIndexFromPtr(object);
	long * indexPtr = &findTable[entryIndex];
	while(*indexPtr >= 0)
	{
		Element & el = elements[*indexPtr];
		if(el.object == object)
		{
			if(el.func.funcUpdate == func.funcUpdate)
			{
				DeleteElement(*indexPtr);
				continue;
			}
		}
		indexPtr = &el.nextFind;
	}
}

void MissionObjectsList::Del(MissionObject * object)
{
#ifndef STOP_PROFILES
	//Если профайлим то регистрируем событие
	if(MissionProfiler::IsActive() && object)
	{
		MissionProfiler::ProfileData data;
		Func func;
		func.ptr = null;
		FillProfileData(data, object, func, 0);
		MissionProfiler::UnregistryInGroup(data);
	}
#endif
	//Ищем среди добавленых на данный уровень
	dword entryIndex = GetEntryIndexFromPtr(object);
	long * indexPtr = &findTable[entryIndex];
	while(*indexPtr >= 0)
	{
		Element & el = elements[*indexPtr];
		if(el.object == object)
		{
			DeleteElement(*indexPtr);
			continue;
		}
		indexPtr = &el.nextFind;
	}
}

bool MissionObjectsList::OptimizeStep()
{
	if(levels.Size() == 0) return true;
	for(long i = 0; i < optimizeStepsPerFrame; i++)
	{
		if(currentOptimizeLevel >= levels.Size())
		{
			currentOptimizeLevel = 0;
			return true;
		}
		Level & lvl = levels[currentOptimizeLevel];		
		if(lvl.firstElement < 0)
		{
			//Попалась цепочка пустых элементов - вычищаем и выходим
			long first = currentOptimizeLevel;
			for(long last = first + 1; last < levels; last++)
			{
				if(levels[last].firstElement >= 0)
				{
					break;
				}
			}
			levels.DelRange(first, last - 1);
			break;
		}
	}
	return false;
}


//Создать итератор
MGIterator * MissionObjectsList::CreateIterator(const char * file, long line)
{
	return IteratorsPull::CreateIterator(this, file, line);
}

//Исполнить список
void MissionObjectsList::ExecuteUpdate(float dltTime, MGIterator * it)
{
	((MissionIterator *)it)->ExecuteUpdate(dltTime);
}

//Удалить все объекты группы
void MissionObjectsList::DeleteList()
{
	while(firstExecute >= 0)
	{
		MissionObject * mo = elements[firstExecute].object;
		delete mo;
	}
}

//Определить в списке объект или нет
bool MissionObjectsList::InList(MissionObject * object, dword uid, long & hashIndex)
{
	if(!object || uid == 0) return false;
	if(hashIndex >= 0 && hashIndex < elements)
	{
		Element & el = elements[hashIndex];
		if(el.object == object && el.object->GetObjectUID() == uid)
		{
			return true;
		}
	}
	//Ищем среди добавленых
	dword entryIndex = GetEntryIndexFromPtr(object);
	long index = findTable[entryIndex];
	while(index >= 0)
	{
		Element & el = elements[index];
		if(el.object == object && el.object->GetObjectUID() == uid)
		{
			hashIndex = index;
			return true;
		}
		index = el.nextFind;
	}
	hashIndex = -1;
	return false;
}

//Получить указатель на первый итератор данного списка
__forceinline MissionObjectsList::MissionIterator * & MissionObjectsList::GetPtrForFirstIterator()
{
	return iterators;
}

//Получить индекс первого элемента в списке
__forceinline long MissionObjectsList::GetFirstElement()
{
	return firstExecute;
}

//Получить индекс следующего элемента в списке
__forceinline long MissionObjectsList::GetNextElement(long current)
{
	if(current >= 0)
	{
		return elements[current].nextExecute;
	}
	return -1;
}

//Получить объект данного элемента
__forceinline MissionObject * MissionObjectsList::GetElementObject(long current)
{
	if(current >= 0)
	{
		return elements[current].object;
	}
	return null;
}

//Получить уровень данного элемента
__forceinline long MissionObjectsList::GetElementLevel(long current)
{
	if(current >= 0)
	{
		return elements[current].level;
	}
	return 0;
}

//Исполнить событие для итератора
__forceinline void MissionObjectsList::ExecuteEventForIterator(long elementIndex, MissionObject * sender)
{
	if(elementIndex >= 0)
	{
		Element & el = elements[elementIndex];
		if(el.func.funcEvent)
		{
#ifndef STOP_PROFILES
			if(!MissionProfiler::IsActive())
			{
#endif
				(el.object->*el.func.funcEvent)(id, sender);
#ifndef STOP_PROFILES
			}else{
				MissionProfiler::ProfileData data;
				FillProfileData(data, el.object, el.func, el.level);
#ifdef PROFILER_USE_STACK_TIMERS
				dword timerId = MissionProfiler::StartProfileTimer();
				(el.object->*el.func.funcEvent)(name, sender);
				data.time = MissionProfiler::StopProfileTimer(timerId);
#else
				ProfileTimer timer;
				(el.object->*el.func.funcEvent)(id, sender);
				timer.Stop();
				data.time = timer.GetTime64();
#endif
				MissionProfiler::AddRecord(data);
			}
#endif
		}		
	}
}

//Исполнить функцию обновления для итератора
__forceinline void MissionObjectsList::ExecuteUpdateForIterator(long elementIndex, float dltTime)
{
#ifndef STOP_PROFILES
	if(!MissionProfiler::IsActive())
	{
		Element & el = elements[elementIndex];
		(el.object->*el.func.funcUpdate)(dltTime, el.level);
	}else{		
		Element & el = elements[elementIndex];
		MissionProfiler::ProfileData data;
		FillProfileData(data, el.object, el.func, el.level);
#ifdef PROFILER_USE_STACK_TIMERS
		dword timerId = MissionProfiler::StartProfileTimer();
		(el.object->*el.func.funcUpdate)(dltTime, el.level);
		data.time = MissionProfiler::StopProfileTimer(timerId);
#else
		ProfileTimer timer;
		(el.object->*el.func.funcUpdate)(dltTime, el.level);
		timer.Stop();
		data.time = timer.GetTime64();
#endif
		MissionProfiler::AddRecord(data);
	}
#else
	Element & el = elements[elementIndex];
	(el.object->*el.func.funcUpdate)(dltTime, el.level);
#endif
}

//Заполнить данные профайла об объекте
__forceinline void MissionObjectsList::FillProfileData(MissionProfiler::ProfileData & data, MissionObject * mo, Func func, long level)
{
#ifndef STOP_PROFILES
	data.mission = &mo->Mission();
	data.object = mo;
	data.func = func.ptr;
	data.level = level;
	profileObjectId.Reserve(1024);
	profileObjectId = mo->GetObjectID().c_str();	
	data.objectID = profileObjectId.c_str();
	profileObjectType.Reserve(1024);
	profileObjectType = mo->GetObjectType();
	data.objectType = profileObjectType.c_str();
	data.time = 0;
#endif
}

//Удалить элемент
void MissionObjectsList::DeleteElement(long & findIndex)
{
	long index = findIndex;
	Element & el = elements[index];
	//Сообщаем активным итераторам о событии удаления
	MissionIterator::DeleteElement(index, iterators);
	//Проверяем необходимость скоректировать входной индекс уровня
	if(el.prevExecute < 0 || elements[el.prevExecute].level != el.level)
	{
		long levelIndex = GetLevelEntry(el.level);
		Level & lvl = levels[levelIndex];
		Assert(lvl.firstElement == index);
		if(el.nextExecute >= 0 && elements[el.nextExecute].level == el.level)
		{
			lvl.firstElement = el.nextExecute;
		}else{
			lvl.firstElement = -1;
		}
	}
	//Исключаем из списка исполнения
	if(el.prevExecute >= 0)
	{
		elements[el.prevExecute].nextExecute = el.nextExecute;
	}else{
		firstExecute = el.nextExecute;
	}
	if(el.nextExecute >= 0)
	{
		elements[el.nextExecute].prevExecute = el.prevExecute;
	}else{
		lastExecute = el.prevExecute;
	}
	//Исключаем из списка поиска по указателю
	findIndex = el.nextFind;
	//Включаем в список пустых
	el.object = null;
#ifndef STOP_ASSERTS	
	el.func.funcUpdate = null;
	el.nextExecute = -1;
	el.prevExecute = -1;
	el.nextFind = -1;
	el.level = 0;
#endif
	el.nextFree = firstFree;
	firstFree = index;
}

//Найти, а если нет, добавить запись для заданного уровня
long MissionObjectsList::GetLevelEntry(long level)
{
	//!!! FixMe надо сделать бинарный поиск уровня
	for(long i = 0; i < levels; i++)
	{
		long lvl = levels[i].level;
		if(lvl == level)
		{
			return i;
		}
		if(levels[i].level > level)
		{
			levels.Insert(i);
			levels[i].level = level;
			levels[i].firstElement = -1;
			return i;
		}
	}
	levels.SetAddElements(coremax(coremin((levels.Size() + 0xf) & ~0xf, 256), 16));
	i = levels.Add();
	levels[i].level = level;
	levels[i].firstElement = -1;
	return i;
}

//Вычислить индекс для входной таблице по адресу
__forceinline dword MissionObjectsList::GetEntryIndexFromPtr(MissionObject * object)
{
	long num = object - (MissionObject *)null;
	long hash = num ^ (num >> 4) ^ (num >> findTableBase) ^ (num >> findTableBase*2);
	dword index = hash & findTableMask;
	return index;	
}


//================================================================================================
//MissionIterator
//================================================================================================

MissionObjectsList::MissionIterator::MissionIterator()
{
	currentElement = -1;
	state = s_not_initialized;
	list = null;
	next = null;
	prev = null;
	cpp_file = null;
	cpp_line = -1;
	pull = null;
}

MissionObjectsList::MissionIterator::~MissionIterator()
{
}

//Закончить ли цикл
bool MissionObjectsList::MissionIterator::IsDone()
{
	return (state <= s_finishing);
}

//Взять следующий элемент
void MissionObjectsList::MissionIterator::Next()
{
	switch(state)
	{
	case s_finishing:
		return;
	case s_ok:
		currentElement = list->GetNextElement(currentElement);
		break;
	case s_moved_to_next:
		state = s_ok;
		break;
	default:
		Assert(false);
	}
	if(currentElement < 0)
	{
		state = s_finishing;
	}
}

//Получить элемент
MissionObject * MissionObjectsList::MissionIterator::Get()
{
	if(state == s_ok)
	{
		return list->GetElementObject(currentElement);
	}
	return null;
}

//Узнать уровень на котором находиться запись
long MissionObjectsList::MissionIterator::Level()
{
	if(state == s_ok)
	{
		return list->GetElementLevel(currentElement);
	}
	return 0;
}

//Освободить итератор
void MissionObjectsList::MissionIterator::Release()
{
	currentElement = -1;
	state = s_not_initialized;
	if(next)
	{
		next->prev = prev;
	}
	if(prev)
	{
		prev->next = next;
	}else{
		MissionIterator * & first = list->GetPtrForFirstIterator();
		first = next;
	}
	next = null;
	prev = null;
	list = null;
	pull->Use(this, false);	
	pull = null;
}

//Установить итератор на начало
void MissionObjectsList::MissionIterator::Reset()
{
	currentElement = list->GetFirstElement();
	if(currentElement >= 0)
	{
		state = s_ok;
	}else{
		state = s_finishing;
	}	
}

//Исполнить эвент, если это возможно
__forceinline void MissionObjectsList::MissionIterator::ExecuteEvent(MissionObject * sender)
{
	if(state == s_ok)
	{
		list->ExecuteEventForIterator(currentElement, sender);
	}
}

//Исполнить обновление	
__forceinline void MissionObjectsList::MissionIterator::ExecuteUpdate(float dltTime)
{
	if(state == s_ok)
	{
		list->ExecuteUpdateForIterator(currentElement, dltTime);
	}
}

//Удаление элемента
__forceinline void MissionObjectsList::MissionIterator::DeleteElement(long index, MissionIterator * itr)
{
	while(itr)
	{
		if(itr->currentElement == index)
		{
			itr->currentElement = itr->list->GetNextElement(itr->currentElement);
			if(itr->currentElement >= 0)
			{
				itr->state = s_moved_to_next;
			}else{
				itr->state = s_finishing;
			}
		}
		itr = itr->next;
	}
}

//Инициализировать итератор
__forceinline MissionObjectsList::MissionIterator * MissionObjectsList::MissionIterator::Init(IteratorsPull * myPull, MissionObjectsList * objList, const char * file, long line)
{
	Assert(!list);
	list = objList;	
	cpp_file = file;
	cpp_line = line;
	pull = myPull;
	pull->Use(this, true);
	MissionIterator * & first = list->GetPtrForFirstIterator();	
	next = first;
	prev = null;
	first = this;
	MissionObjectsList::MissionIterator::Reset();
	return this;
}

//Сообщение что не удалён
void MissionObjectsList::MissionIterator::TraceError()
{
	api->Trace("MGIterator not release, cpp(file: %s, line: %i)", cpp_file, cpp_line);
}


//================================================================================================
//IteratorsPull
//================================================================================================

//Первый в списке пулов итераторов
MissionObjectsList::IteratorsPull * MissionObjectsList::IteratorsPull::first = null;
//Счётчик заведённых групп для менеджмента пулов
long MissionObjectsList::IteratorsPull::refCounter = 0;


MissionObjectsList::IteratorsPull::IteratorsPull()
{
	use = 0;
	next = first;
	first = this;
}

MissionObjectsList::IteratorsPull::~IteratorsPull()
{
	if(use)
	{
		for(long i = 0, mask = 1; i < 32; i++, mask <<= 1)
		{
			if(use & mask)
			{
				iterators[i].TraceError();
			}
		}
	}
}

//Зарезервировать использование пулов
void MissionObjectsList::IteratorsPull::Create()
{
	refCounter++;
}

//Освободить использование пулов
void MissionObjectsList::IteratorsPull::Release()
{
	refCounter--;
	Assert(refCounter >= 0);
	if(refCounter == 0)
	{
		IteratorsPull * it = first;
		while(it)
		{
			IteratorsPull * deleteMe = it;
			it = it->next;
			delete deleteMe;
		}
		first = null;
	}
}

//Создать итератор
MissionObjectsList::MissionIterator * MissionObjectsList::IteratorsPull::CreateIterator(MissionObjectsList * list, const char * file, long line)
{
	//Пробуем найти итератор в созданных пулах
	for(IteratorsPull * pull = first; pull; pull = pull->next)
	{
		if(pull->use != 0xffffffff)
		{
			//Ищем свободный итератор
			for(dword index = 0, mask = 0xff; (pull->use & mask) == mask; mask <<= 8, index++);
			for(index *= 8, mask &= ~(mask << 1); (pull->use & mask) != 0; mask <<= 1, index++);
			//Инициализируем итератор и возвращаем на него указатель
			return pull->iterators[index].Init(pull, list, file, line);
		}
	}
	//Надо добавить новый пул
	pull = NEW IteratorsPull();
	return pull->iterators[0].Init(pull, list, file, line);
}

//Занять/освободить итератор
__forceinline void MissionObjectsList::IteratorsPull::Use(MissionIterator * it, bool isUse)
{
	dword index = it - &iterators[0];
	Assert(index < 32);
	dword mask = 1 << index;
	if(isUse)
	{
		use |= mask;
	}else{
		use &= ~mask;
	}
}



