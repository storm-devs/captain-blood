

#include "ObjectsDeclarators.h"


ObjectsDeclarators::ObjectsDeclarators() : declDesc(_FL_)
{
	//Пока модификатор не стоит
	accessor = null;
	//Очищаем входную таблицу
	for(long i = 0; i < ARRSIZE(entryTable); i++)
	{
		entryTable[i] = null;
	}
	needUpdateHash = true;
}

ObjectsDeclarators::~ObjectsDeclarators()
{
}

void ObjectsDeclarators::Release()
{
	//Удаляем объекты в прямом порядке
	objects.ToFirst();
	while(true)
	{
		RegObject * pnt = objects.Get();
		if(!pnt) break;
		if(accessor->GetServiceFlag(pnt))
		{
			objects.Next();
		}else{
			delete pnt;
			objects.ToFirst();
		}
	}
	//Вызываем функцию перед удалением сервисов		
	RegObject * pnt = null;	
	for(objects.ToLast(); pnt = objects.Get(); objects.Prev())
	{
		Assert(accessor->GetServiceFlag(pnt));
		((Service *)pnt)->PreRelease();
	}
	//Удаляем сервисы в обратном
	objects.ToLast();
	while(pnt = objects.Get())
	{
		Assert(accessor->GetServiceFlag(pnt));
		delete pnt;
		objects.ToLast();
	}
}

void ObjectsDeclarators::SetAccessor(RegistryKeyAccessor * _accessor)
{
	accessor = _accessor;
}

//-------------------------------------------------------------------------------------------------------
//Работа с менеджером деклараторов
//-------------------------------------------------------------------------------------------------------

//Добавить список деклараторов
void ObjectsDeclarators::AddDecls(StormEngine_Declarator * first)
{
	if(!first) return;
	//Проверяем, если первый есть в списке, то пропустим добавления
	for(dword i = 0; i < declDesc.Size(); i++)
	{
		if(declDesc[i].decl == first)
		{
			return;
		}
	}
	//Добавляем цепочку
	DeclDesc desc;
	desc.next = null;
	for(StormEngine_Declarator * dcl = first; dcl; dcl = dcl->GetNext())
	{
		dword i = declDesc.Add();
		declDesc[i].decl = dcl;
		declDesc[i].hash = string::HashNoCase(dcl->ClassName());
		declDesc[i].next = null;
		declDesc[i].index = i;
	}
	needUpdateHash = true;
}		

//Построить хэшь-таблицу по деклараторам
void ObjectsDeclarators::BuildHashTable()
{
	if(!needUpdateHash) return;
	//Обнуляем входную таблицу
	for(dword i = 0; i < ARRSIZE(entryTable); i++)
	{
		entryTable[i] = null;
	}
	//Разрываем цепочки
	DeclDesc * declDescPtr = declDesc.GetBuffer();
	dword count = declDesc.Size();
	for(dword i = 0; i < count; i++)
	{
		declDescPtr[i].next = null;
	}
	//Строим заново хэшь-таблицу
	for(dword i = 0; i < count; i++)
	{
		DeclDesc & cur = declDescPtr[i];
		dword index = cur.hash & (ARRSIZE(entryTable) - 1);
		if(entryTable[index])
		{
			DeclDesc * desc = entryTable[index];
			while(true)
			{
				if(desc->hash == cur.hash)
				{
					if(string::IsEqual(desc->decl->ClassName(), cur.decl->ClassName()))
					{
						CoreLogOut("Game init error! Repeat class name: \"%s\" <-> \"%s\"\nTry check modules for fix problem.", desc->decl->ClassName(), cur.decl->ClassName());
						Sleep(1);
						throw "invalidate module(s)";
					}
				}
				if(!desc->next)
				{
					break;
				}
				desc = desc->next;
			}
			desc->next = &cur;
		}else{
			entryTable[index] = &cur;
		}
	}
	needUpdateHash = false;
}

//Создать объект по имени
RegObject * ObjectsDeclarators::CreateObject(const char * name, bool isCreateService)
{	
	if(!name || !name[0])
	{
		return null;
	}
	if(needUpdateHash)
	{
		BuildHashTable();
	}
	//Найдём описание
	dword hash = string::HashNoCase(name);
	dword index = hash & (ARRSIZE(entryTable) - 1);
	for(DeclDesc * desc = entryTable[index]; desc; desc = desc->next)
	{
		if(desc->hash == hash)
		{
			if(string::IsEqual(desc->decl->ClassName(), name))
			{
				break;
			}
		}
	}
	if(!desc)
	{
		return null;
	}
	//Если сервис, то его создаём всего 1 раз
	if(desc->decl->IsService())
	{
		desc->objects.ToFirst();
		RegObject * obj = desc->objects.Get();
		if(obj)
		{
			return obj;
		}
	}
	//Если желаемое не совпадает с дествительным...
	if(desc->decl->IsService() && !isCreateService) return null;
	//Есть описание, пробуем создать объект
	RegObject * obj = desc->decl->Create();
	if(!obj)
	{
		return null;
	}
	//Формируем запись для быстрого поиска
	accessor->Reset(obj);
	accessor->SetGlobalIndex(obj, objects.Add(obj));
	accessor->SetObjectIndex(obj, desc->objects.Add(obj));
	accessor->SetRegIndex(obj, desc->index);
	accessor->SetRegistryFlag(obj);
	if(desc->decl->IsService()) accessor->SetServiceFlag(obj);
	//Инициализируем объект
	if(!obj->Init())
	{
		delete obj;
		return null;
	}

	/*
	//!!! отладка
	dword count = 0;
	dword hits = 0;
	for(objects.ToFirst(); objects.Get(); objects.Next())
	{
		if(objects.Get() == obj)
		{
			Assert(!count);
			count++;
			continue;
		}
		if(accessor->GetRegistryKey(objects.Get()) == accessor->GetRegistryKey(obj))
		{
			hits++;
		}
	}
	Assert(objects.Test(obj, accessor->GetGlobalIndex(obj), accessor->GetGlobalIndexStep()));
	Assert(count == 1);
	*/

	return obj;
}

//Удалить объект из списков
void ObjectsDeclarators::RemoveObject(RegObject * obj)
{
	//Получаем регистрационное значение
	Assert(accessor->IsValidate(obj));
	//Если записи здесь нет, то не обрабатываем удаление
	if(!accessor->GetRegistryFlag(obj)) return;

	//!!! отладка
	/*
	dword count = 0;
	for(objects.ToFirst(); objects.Get(); objects.Next())
	{
		if(objects.Get() == obj)
		{
			Assert(!count);
			count++;
			continue;
		}
		Assert(accessor->GetRegistryKey(objects.Get()) != accessor->GetRegistryKey(obj));
	}
	Assert(count == 1);
	*/


	//Удаляем запись из глобального списка
	objects.Del(obj, accessor->GetGlobalIndex(obj), accessor->GetGlobalIndexStep());
	//Удаляем запись из описателя
	dword descIndex = accessor->GetRegIndex(obj);
	dword objsIndex = accessor->GetObjectIndex(obj);
	while(true)
	{
		if(declDesc[descIndex].objects.Del(obj, objsIndex, accessor->GetObjectIndexStep()))
		{
			return;
		}
		descIndex += accessor->GetRegIndexStep();
	}
	accessor->InvalidateKey(obj);
}

//Найти в объекты с заданным типом
ObjectsDeclarators::RegList * ObjectsDeclarators::Find(const char * name)
{
	//Найдём описание
	dword hash = string::HashNoCase(name);
	dword index = hash & (ARRSIZE(entryTable) - 1);
	for(DeclDesc * desc = entryTable[index]; desc; desc = desc->next)
	{
		if(desc->hash == hash)
		{
			if(strcmp(desc->decl->ClassName(), name) == 0)
			{
				break;
			}
		}
	}
	if(!desc) return null;
	return &desc->objects;
}

//Получить список всех зарегестрированных объектов
void ObjectsDeclarators::GetRegistryObjectsList(array<string> & objects)
{
	objects.DelAll();
	for(long i = 0; i < declDesc; i++)
	{
		if(declDesc[i].decl->IsService()) continue;
		objects[objects.Add()] = declDesc[i].decl->ClassName();
	}
}

//Создать все сервисы
bool ObjectsDeclarators::CreateServices()
{
	long errorCreationIndex = -1;
	array<ServiceInfo> services(_FL_, 256);
	for(long i = 0; i < declDesc; i++)
	{
		if(!declDesc[i].decl->IsService()) continue;
		long level = declDesc[i].decl->Level();
		for(long j = 0; j < services; j++)
		{
			if(level < services[j].decl->Level()) break;
		}
		ServiceInfo si;
		si.decl = declDesc[i].decl;
		si.creationTime = -1;
		services.Insert(si, (dword)j);
	}
	DWORD totalCreateTime = GetTickCount();
	for(long i = 0; i < services; i++)
	{
		DWORD startCreateTime = GetTickCount();
		if(!CreateObject(services[i].decl->ClassName(), true))
		{
			CoreLogOut("\n\n\n\n\nGame init error! Can't create service \"%s\"\n\n\n\n\n", services[i].decl->ClassName());
			errorCreationIndex = i;
			break;
		}
		DWORD endCreateTime = GetTickCount();
		services[i].creationTime = endCreateTime - startCreateTime;
	}
	totalCreateTime = GetTickCount() - totalCreateTime;
	CoreLogOut("\n\n\n\n\nServices create statistics:\n");
	for(long i = 0; i < services; i++)
	{		
		if(services[i].creationTime != -1)
		{
			CoreLogOut("%.3i: Service \"%s\", level = %i, ctration time = %.3fs", i, services[i].decl->ClassName(), services[i].decl->Level(), services[i].creationTime*0.001f);
		}else{
			if(i == errorCreationIndex)
			{
				CoreLogOut("%.3i*: Service \"%s\" error create and broke init chain", i, services[i].decl->ClassName());
			}else{
				CoreLogOut("%.3i: Service \"%s\" skip create", i, services[i].decl->ClassName());
			}
		}		
	}
	CoreLogOut("\nTotal creation time: %.3fs\n\n\n", totalCreateTime*0.001f);
	return (errorCreationIndex < 0);
}


//-------------------------------------------------------------------------------------------------------
//RegList
//-------------------------------------------------------------------------------------------------------

ObjectsDeclarators::RegList::RegList() : objects(_FL_)
{
	firstFree = -1;
	first = -1;
	last = -1;
	current = -1;
}

//Добавить объект
__forceinline long ObjectsDeclarators::RegList::Add(RegObject * obj)
{
	current = -1;
	Assert(obj);
	//Резервируем элемент
	long i = -1;
	if(firstFree < 0)
	{
		i = objects.Add();
	}else{
		i = firstFree;
		firstFree = objects[i].next;
	}
	//Включаем в конец списка
	if(first < 0)
	{
		first = i;
	}
	if(last >= 0)
	{
		objects[last].next = i;
	}
	RegInfo & ri = objects[i];
	ri.obj = obj;
	ri.prev = last;
	ri.next = -1;	
	last = i;
	return i;
}

//Удалить объект из списка
__forceinline bool ObjectsDeclarators::RegList::Del(RegObject * obj, long index, long step)
{
	for(long i = index; i < objects; i += step)
	{
		if(objects[i].obj == obj)
		{
			current = i;
			Del();
			return true;
		}
	}
	return false;
}

//Проверить на правильность регистрации
__forceinline bool ObjectsDeclarators::RegList::Test(RegObject * obj, long index, long step)
{
	for(long i = index; i < objects; i += step)
	{
		if(objects[i].obj == obj)
		{
			return true;
		}
	}
	return false;
}

//Удалить объект из списка
__forceinline void ObjectsDeclarators::RegList::Del()
{
	RegInfo & ri = objects[current];
	long next = objects[current].next;
	//Исключаем из списка
	if(ri.prev >= 0)
	{
		objects[ri.prev].next = ri.next;
	}else{
		first = ri.next;
	}
	if(ri.next >= 0)
	{
		objects[ri.next].prev = ri.prev;
	}else{
		last = ri.prev;
	}
	//Добавляем в список свободных
	ri.obj = null;
	ri.prev = -1;
	ri.next = firstFree;
	firstFree = current;
	//Поправляем итератор на следующего
	current = next;
}

//Получить объект
RegObject * ObjectsDeclarators::RegList::Get()
{
	if(current < 0) return null;
	return objects[current].obj;
}

