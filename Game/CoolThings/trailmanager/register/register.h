#pragma once

#include "..\..\..\..\common_h\core.h"
#include "..\..\..\..\common_h\templates.h"

template <int add_size> class Stack
{
public:

	Stack() : index(_FL_,add_size)
	{
		h = 0;
	}

	dword Get()
	{
		if( h > index - 1 )
			expand();

		return index[h++];
	}

	void Put(int val)
	{
		Assert(h > 0)

		index[--h] = val;
	}

private:

	void expand()
	{
		index.AddElements(add_size);

		for( int i = h ; i < h + add_size ; i++ )
			index[i] = i;
	}

private:

	array<int> index; int h;

};

template <class T, int add_size> class Register
{
public:

	Register() : data(_FL_,add_size)
	{
	}

	// простое сканирование

	T &operator [](int i)
	{
		return data[i];
	}

	operator int()
	{
		return data.Size();
	}

	// работа с массивом

	T &Get()
	{
		int i = index.Get();

		if( i > data - 1 )
			data.AddElements(add_size);

		return data[i];
	}

	void Release(int i)
	{
		index.Put(i);
	}

private:

	array<T> data; Stack<add_size> index;

};
