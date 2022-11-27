

#ifndef _ObjectsDeclarators_h_
#define _ObjectsDeclarators_h_


#include "CoreBase.h"
#include "RegistryKeyAccessor.h"


class ObjectsDeclarators
{
public:
	class RegList
	{
		struct RegInfo
		{
			RegObject * obj;
			long prev;
			long next;
		};

	public:
		RegList();
		//Добавить объект
		long Add(RegObject * obj);
		//Удалить объект из списка
		bool Del(RegObject * obj, long index, long step);
		//Удалить объект из списка
		void Del();
		//Проверить на правильность регистрации
		bool Test(RegObject * obj, long index, long step);
		//Установить итератор на первый элемент
		__forceinline void ToFirst()
		{
			current = first;
		}
		//Установить итератор на последний элемент
		__forceinline void ToLast()
		{
			current = last;
		}
		//Шагнуть на предыдущий элемент
		__forceinline void Prev()
		{
			current = objects[current].prev;
		}
		//Шагнуть на следующий элемент
		__forceinline void Next()
		{
			current = objects[current].next;
		}
		//Получить объект
		RegObject * Get();


	private:
		long firstFree;					//Первый свободный индекс
		long first;						//Первый индекс элемента
		long last;						//Последний индекс элемента
		long current;					//Текущая позиция итератора
		array<RegInfo> objects;			//Объекты данного типа
	};

private:
	struct DeclDesc
	{
		StormEngine_Declarator * decl;	//Декларатор объекта
		DeclDesc * next;				//Следующий с таким
		dword hash;						//Хэшьзначение строки
		dword index;					//Индекс текущего описателя в массиве
		RegList objects;				//Спсок зарегистрённых объектов
	};

	struct ServiceInfo
	{
		StormEngine_Declarator * decl;	//Декларатор сервиса
		dword creationTime;				//Время создания сервиса
	};

public:
	ObjectsDeclarators();
	~ObjectsDeclarators();

	void Release();

	void SetAccessor(RegistryKeyAccessor * _accessor);

//-------------------------------------------------------------------------------------------------------
//Работа с менеджером деклараторов
//-------------------------------------------------------------------------------------------------------
public:
	//Добавить список деклараторов
	void AddDecls(StormEngine_Declarator * first);
	//Построить хэшь-таблицу по деклараторам
	void BuildHashTable();

	//Создать объект по имени
	RegObject * CreateObject(const char * name, bool isCreateService);
	//Удалить объект из списков
	void RemoveObject(RegObject * obj);
	//Найти в объекты с заданным типом
	RegList * Find(const char * name);
	//Получить список всех зарегестрированных объектов
	void GetRegistryObjectsList(array<string> & objects);

	//Создать все сервисы
	bool CreateServices();

//-------------------------------------------------------------------------------------------------------
protected:
	RegistryKeyAccessor * accessor;
	array<DeclDesc> declDesc;
	bool needUpdateHash;
	DeclDesc * entryTable[256];
	RegList objects;
};


#endif

