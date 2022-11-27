//Spirenkov Max. Таблица со строковыми ключами работающая только на расширение и имеющая постоянные адреса
//entrySize обязательно кратно степени 2. Чем больше нем быстрей поиск
//chunkSize количество элементов выделяемых пулом за 1 раз. Рекомендуеться кратность степени 2 при линейном доступе.

#pragma once

#include "array.h"

template<class T, long entrySize, long chunkSize> class HashTable
{
	struct Element
	{		
		dword hash;
		dword len;		
		dword strIndex;
		Element * next;
		T t;
	};

	struct PullLine
	{
		Element line[chunkSize];
	};

public:
	HashTable(const char * cppFileName, long cppFileLine) : strings(cppFileName, cppFileLine, 128), 
															pull(cppFileName, cppFileLine)

	{
		//Размер чанка должен быть больше нуля
		Assert(chunkSize > 0);
		//Размер входной таблицы являеться результатом 2 в степени и минимум должен быть 4
		Assert(entrySize >= 4);
		Assert((entrySize & (entrySize - 1)) == 0);
		//Очистим входную таблицу
		for(dword i = 0; i < entrySize; i++) entry[i] = null;
		//Сохраняем компромат
		allocCounter = chunkSize;
		cppName = cppFileName;
		cppLine = cppFileLine;
	}

	~HashTable()
	{
		PullLine ** lines = pull.GetBuffer();
		dword count = pull.Size();
		for(dword i = 0; i < count; i++)
		{
			delete lines[i];
		}
		pull.Empty();	
	}

	void DelAll()
	{
		PullLine ** lines = pull.GetBuffer();
		dword count = pull.Size();
		for(dword i = 0; i < count; i++)
		{
			delete lines[i];
		}
		pull.Empty();
		strings.Empty();
		for(dword i = 0; i < entrySize; i++) entry[i] = null;
		allocCounter = chunkSize;
	}

	inline bool AddObj(const char * str, T t)
	{		
		bool isFind = false;
		T * ptr = Add(str, &isFind);
		if(!isFind && ptr)
		{
			*ptr = t;
			return true;
		}
		return false;
	}

	inline bool AddObj(const ConstString & str, T t)
	{
		bool isFind = false;
		T * ptr = Add(str, &isFind);
		if(!isFind && ptr)
		{
			*ptr = t;
			return true;
		}
		return false;
	}

	inline bool AddObj(dword hash, dword len, const char * str, T t)
	{
		bool isFind = false;
		T * ptr = Add(hash, len, str, &isFind);
		if(!isFind && ptr)
		{
			*ptr = t;
			return true;
		}
		return false;
	}

	inline T * Add(const char * str, bool * isFind = null)
	{
		//Параметры строки
		dword len = 0;
		dword hash = string::HashNoCase(str, len);
		return Add(hash, len, str, isFind);
	}

	
	inline T * Add(const ConstString & str, bool * isFind = null)
	{
		return Add(str.Hash(), str.Len(), str.c_str(), isFind);
	}


	T * Add(dword hash, dword len, const char * str, bool * isFind = null)
	{
		Assert(string::NotEmpty(str));
		//Смотрим среди добавленых
		Element ** last = &entry[hash & (entrySize - 1)];
		while(*last != null)
		{
			Element * element = *last;
			if(element->hash == hash && element->len == len)
			{
				const char * testStr = &strings[element->strIndex];
				if(string::IsEqual(str, testStr))
				{
					//Ничего добавлять ненадо
					if(isFind) *isFind = true;
					return &element->t;
				}
			}
			last = &element->next;
		}
		//Добавляем новый элемент
		if(allocCounter >= chunkSize)
		{
			//Закончились элементы в линии, добавляем новую
			pull.Add(new(cppName, cppLine) PullLine);
			allocCounter = 0;
		}
		PullLine * pullLine = pull.LastE();
		Element & el = pullLine->line[allocCounter++];
		//Заполняем поля
		el.hash = hash;
		el.len = len;
		el.strIndex = strings.Size();
		el.next = null;
		//Включаем в список
		*last = &el;
		//Добавляем строку
		len++;
		strings.AddElements(len);
		char * dst = &strings[el.strIndex];
		const char * src = str;
		for(char * dst = &strings[el.strIndex]; len != 0; len--)  *dst++ = *src++;
		if(isFind) *isFind = false;
		return &el.t;
	}

	//Поиск с расчитаными параметрами строки
	T * Find(dword hash, dword len, const char * str)
	{
		//По пустой строке не ищим
		if(string::IsEmpty(str))
		{
			return null;
		}
		//Просматриваем списки
		for(Element * el = entry[hash & (entrySize - 1)]; el; el = el->next)
		{
			if(el->hash == hash && el->len == len)
			{
				const char * testStr = &strings[el->strIndex];
				if(string::IsEqual(str, testStr))
				{
					return &el->t;
				}
			}
		}
		return null;
	}

	//Поиск по строке
	inline T * Find(const char * str)
	{
		//Параметры строки
		dword len = 0;
		dword hash = string::HashNoCase(str, len);
		//Поиск
		return Find(hash, len, str);
	}

	//Поиск по строке
	inline T * Find(const ConstString & str)
	{
		//Поиск
		return Find(str.Hash(), str.Len(), str.c_str());
	}

	//Поиск с расчитаными параметрами строки
	inline T FindObj(dword hash, dword len, const char * str, T defVal)
	{
		T * ptr = Find(hash, len, str);
		if(ptr) return *ptr;
		return defVal;
	}

	//Поиск по строке
	inline T FindObj(const char * str, T defVal)
	{
		T * ptr = Find(str);
		if(ptr) return *ptr;
		return defVal;
	}

	//Поиск по строке
	inline T FindObj(const ConstString & str, T defVal)
	{
		T * ptr = Find(str);
		if(ptr) return *ptr;
		return defVal;
	}

	//Получить количество элементов в таблице
	inline dword GetCount()
	{
		return pull.Size()*(chunkSize - 1) + allocCounter;
	}

	//Доступиться до элемента
	inline T * GetByIndex(dword index)
	{
		dword pullIndex = index/chunkSize;
		dword elementIndex = index%chunkSize;
		PullLine * pullLine = pull[pullIndex];		
		if(pullIndex == pull.Last())
		{
			Assert(elementIndex < allocCounter);
		}
		Element & el = pullLine->line[elementIndex];
		return &el.t;
	}


private:
	array<char> strings;
	array<PullLine *> pull;
	dword allocCounter;
	const char * cppName;
	long cppLine;
	Element * entry[entrySize];
};










