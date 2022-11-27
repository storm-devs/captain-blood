

#ifndef _ExecutionModule_h_
#define _ExecutionModule_h_


#include "CoreBase.h"
#include "RegistryKeyAccessor.h"

class ExecutionModule
{
	struct ServiceExecute
	{
		Service * s;
		dword level;
	};

	struct GroupElement
	{
		RegObject * object;
		ObjectExecution func;
		dword level;
	};

	struct Group
	{
		Group() : elements(_FL_){};
		string name;
		dword level;
		float timeScale;
		array<GroupElement> elements;
	};

public:
	ExecutionModule();
	~ExecutionModule();
	void SetAccessor(RegistryKeyAccessor * _accessor);

//-------------------------------------------------------------------------------------------------------
//Работа с менеджером dll
//-------------------------------------------------------------------------------------------------------
public:
	//Установить уровень исполнения перед началом кадра
	void SetStartFrameLevel(Service * s, dword level = Core_DefaultExecuteLevel);
	//Установить уровень исполнения после кадра
	void SetEndFrameLevel(Service * s, dword level = Core_DefaultExecuteLevel);

	//Установить функцию объекта на исполнение
	void SetObjectExecution(RegObject * obj, const char * group, dword level, ObjectExecution func);
	//Удалить обработчик
	void DelObjectExecution(RegObject * obj, ObjectExecution func);
	//Удалить все обработчики данного объекта из группы
	void DelObjectExecutions(RegObject * obj, const char * group);
	//Удалить все обработчики данного объекта
	void DelObjectExecutions(RegObject * obj);

	//Установить уровень исполнения группы
	void SetGroupLevel(const char * group, dword level);
	//Получить уровень исполнения группы
	dword GetGroupLevel(const char * group);
	
	//Установить масштаб времени для группы
	void SetGroupTimeScale(float scale, const char * group);
	//Получить масштаб времени для группы
	float GetGroupTimeScale(const char * group);
	
	//Исполнить все сервисы и установленные на исполнение объекты
	void Execute(float dltTime);


//-------------------------------------------------------------------------------------------------------
protected:
	//Найти индекс группы по имени
	long FindGroup(const char * name);
	//Установить уровень исполнения сервиса
	void SetServiceLevel(array<ServiceExecute> & list, Service * s, dword level);

//-------------------------------------------------------------------------------------------------------
protected:
	array<ServiceExecute> startFrame;
	array<ServiceExecute> endFrame;
	array<Group *> groups;
	RegistryKeyAccessor * accessor;
	long currentGroup;
	long currentObject;
};



#endif

