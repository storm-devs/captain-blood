

#include "ExecutionModule.h"


ExecutionModule::ExecutionModule() : startFrame(_FL_),
									 endFrame(_FL_),
									 groups(_FL_)
{
	accessor = null;
	currentGroup = -1;
	currentObject = -1;
}

ExecutionModule::~ExecutionModule()
{
	for(long i = 0; i < groups; i++)
	{
		delete groups[i];
	}
	startFrame.Empty();
	endFrame.Empty();
	groups.Empty();
}

void ExecutionModule::SetAccessor(RegistryKeyAccessor * _accessor)
{
	accessor = _accessor;
}

//Установить уровень исполнения перед началом кадра
void ExecutionModule::SetStartFrameLevel(Service * s, dword level)
{
	SetServiceLevel(startFrame, s, level);
}

//Установить уровень исполнения после кадра
void ExecutionModule::SetEndFrameLevel(Service * s, dword level)
{
	SetServiceLevel(endFrame, s, level);
}

//Установить функцию объекта на исполнение
void ExecutionModule::SetObjectExecution(RegObject * obj, const char * group, dword level, ObjectExecution func)
{
	if(!obj || !func) return;
	//Отметим что объект имел дело с модулем исполнения
	accessor->SetExecuteFlag(obj);
	//Индекс группы
	long i = FindGroup(group);
	if(i < 0)
	{
		SetGroupLevel(group, Core_DefaultExecuteLevel);
		i = FindGroup(group);
		Assert(i >= 0);
	}
	//Если есть запись об функции, переместим её
	array<GroupElement> & elements = groups[i]->elements;
	for(long j = 0; j < elements; j++)
	{
		GroupElement & ge = elements[j];
		if(ge.func == func && ge.object == obj)
		{
			if(elements[j].level == level)
			{
				return;		//Всё уже так как хочет пользователь...
			}
			//Удалим запись
			elements.Extract(j);
			break;
		}
	}
	//Добавляем запись на нужный уровень
	GroupElement ge;
	ge.object = obj;
	ge.level = level;
	ge.func = func;
	for(long j = 0; j < elements; j++)
	{
		if(elements[j].level > level)
		{
			elements.Insert(ge, j);
			return;
		}
	}
	elements.Add(ge);
}

//Удалить обработчик
void ExecutionModule::DelObjectExecution(RegObject * obj, ObjectExecution func)
{
	if(!obj || !func) return;
	Assert(accessor->IsValidate(obj));
	if(!accessor->GetExecuteFlag(obj)) return;
	//Проходим по всем группам
	for(long i = 0; i < groups; i++)
	{
		array<GroupElement> & elements = groups[i]->elements;
		for(long j = 0; j < elements; j++)
		{
			GroupElement & ge = elements[j];
			if(ge.func == func && ge.object == obj)
			{
				if(currentGroup == i && currentObject == j)
				{
					currentObject--;
				}
				elements.Extract(j);
				break;
			}
		}
	}
}

//Удалить все обработчики данного объекта из группы
void ExecutionModule::DelObjectExecutions(RegObject * obj, const char * group)
{
	if(!obj) return;
	Assert(accessor->IsValidate(obj));
	if(!accessor->GetExecuteFlag(obj)) return;	
	long i = FindGroup(group);
	if(i < 0) return;
	array<GroupElement> & elements = groups[i]->elements;
	for(long j = 0; j < elements; j++)
	{
		if(elements[j].object == obj)
		{
			elements.Extract(j--);
		}
	}
}

//Удалить все обработчики данного объекта
void ExecutionModule::DelObjectExecutions(RegObject * obj)
{
	if(!obj) return;
	Assert(accessor->IsValidate(obj));
	if(!accessor->GetExecuteFlag(obj)) return;
	for(long i = 0; i < groups; i++)
	{
		array<GroupElement> & elements = groups[i]->elements;
		for(long j = 0; j < elements; j++)
		{
			if(elements[j].object == obj)
			{
				elements.Extract(j--);
			}
		}
	}
}

//Установить уровень исполнения группы
void ExecutionModule::SetGroupLevel(const char * group, dword level)
{
	Group * grp = null;
	long i = FindGroup(group);
	if(i >= 0)
	{
		grp = groups[i];
		if(grp->level == level)
		{
			return;
		}
		groups.Extract(i);
		grp->level = level;
	}else{
		grp = NEW Group();
		grp->name = group;
		grp->name.Lower();
		grp->level = level;
		grp->timeScale = 1.0f;
	}
	for(long i = 0; i < groups; i++)
	{
		if(groups[i]->level > level)
		{
			groups.Insert(grp, i);
			return;
		}
	}
	groups.Add(grp);
}

//Получить уровень исполнения группы
dword ExecutionModule::GetGroupLevel(const char * group)
{
	long i = FindGroup(group);
	if(i < 0) return Core_DefaultExecuteLevel;
	return groups[i]->level;
}

//Установить масштаб времени для группы
void ExecutionModule::SetGroupTimeScale(float scale, const char * group)
{
	long i = FindGroup(group);
	if(i >= 0)
	{		
		groups[i]->timeScale = scale;
	}
}

//Получить масштаб времени для группы
float ExecutionModule::GetGroupTimeScale(const char * group)
{
	long i = FindGroup(group);
	if(i < 0) return 1.0f;
	return groups[i]->timeScale;
}

//Исполнить все сервисы и установленные на исполнение объекты
void ExecutionModule::Execute(float dltTime)
{
	//Исполняем сервисы перед началом кадра
	for(long i = 0; i < startFrame; i++)
	{
		startFrame[i].s->StartFrame(dltTime);
	}
	//Исполняем группы
	for(currentGroup = 0; currentGroup < groups; currentGroup++)
	{
		array<GroupElement> & elements = groups[currentGroup]->elements;
		float dTime = dltTime*groups[currentGroup]->timeScale;
		if(dTime > (float)Core_MaxDltTime) dTime = (float)Core_MaxDltTime;
		for(currentObject = 0; currentObject < elements; currentObject++)
		{
			GroupElement & ge = elements[currentObject];
			(ge.object->*ge.func)(dTime);
		}
	}
	//Исполняем сервесы после кадра
	for(long i = 0; i < endFrame; i++)
	{
		endFrame[i].s->EndFrame(dltTime);
	}
}

//Найти индекс группы по имени
long ExecutionModule::FindGroup(const char * name)
{
	if(!name) name = "";
	for(long i = 0; i < groups; i++)
	{
		if(string::IsEqual(groups[i]->name.GetBuffer(), name)) return i;
	}
	return -1;
}

//Установить уровень исполнения сервиса
void ExecutionModule::SetServiceLevel(array<ServiceExecute> & list, Service * s, dword level)
{
	Assert(accessor->GetRegistryFlag(s));
	Assert(accessor->GetServiceFlag(s));
	for(long i = 0; i < list; i++)
	{
		if(list[i].s == s)
		{
			if(list[i].level == level) return;
			list.Extract(i);
		}
	}
	ServiceExecute se;
	se.s = s;
	se.level = level;
	for(long i = 0; i < list; i++)
	{
		if(list[i].level > level)
		{
			list.Insert(se, i);
			return;
		}
	}
	list.Add(se);
}

