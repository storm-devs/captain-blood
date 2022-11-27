/*
Ещё вариант пула памяти для блоков заданного размера

Конструировать объект необходимо так:



//Конструирование объекта выделенного в пуле

	...
	Pulls<MyObject> pulls;
	...

	dword code;
	void * ptr = pulls.Alloc(code);
	MyObject * p = new('a', ptr) MyObject();
	p->pullCode = code;

//Удаление объекта
	dword code = p->pullCode;
	p->~MyObject();
	pulls.Delete(code);

*/

#ifndef _MemPulls_h_
#define _MemPulls_h_

#include "tcommon.h"
#include "array.h"

template<int size> struct Pulls
{		
	struct Pull
	{
		byte buffer[size*32];
		dword use;
	};

public:
	Pulls() : pulls(_FL_)
	{
	}

	~Pulls()
	{
		Clear();
	}

	//Выделить в пуле элемент
	void * Alloc(dword & pullCode)
	{
		//Получаем свободный пул
		long i;
		for(i = 0; i < pulls; i++)
		{
			Pull & pull = *pulls[i];
			if(pull.use != 0xffffffff)
			{
				break;
			}
		}
		if(i >= pulls)
		{
			Pull * p = NEW Pull;
			p->use = 0;
			pulls.Add(p);
		}
		Pull & pull = *pulls[i];
		//Получаем свободный элемент
		for(dword j = 0, mask = 1; j < 32; j++, mask <<= 1)
		{
			if((pull.use & mask) == 0)
			{
				pull.use |= mask;
				pullCode = 0x40000000 | i;
				pullCode |= j << 24;
				return pull.buffer + j*size;
			}
		}
		Assert(false);
		return null;
	}

	//Удалить в пуле элемент
	void Delete(dword pullCode)
	{
		Assert((pullCode & 0xe0000000) == 0x40000000);
		dword pullIndex = pullCode & 0xffffff;
		dword useMask = (pullCode & 0x1f000000) >> 24;
		useMask = 1 << useMask;
		Pull & pull = *pulls[pullIndex];
		Assert(pull.use & useMask);
		pull.use &= ~useMask;
	}

	//Очистить пуллы
	void Clear()
	{
		for(long i = 0; i < pulls; i++)
		{
			delete pulls[i];
		}
		pulls.Free();
	}

private:
	array<Pull *> pulls;
};


#endif
