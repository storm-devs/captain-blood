#pragma once

template <class T> struct Shell
{
	array<T> *in;
	array<T> *ex;

	Shell(array<T> *_in)
	{
		in = _in; ex = NULL;
	}

	void AddElements(dword cnt)
	{
		in->AddElements(cnt);
	}

	void setExtern(array<T> *_ex)
	{
		ex = _ex;
	}

	T &operator [](dword i)
	{
		return (*(ex ? ex : in))[i];
	}

	dword Add()
	{
		return in->Add();
	}

	void DelAll()
	{
		in->DelAll();
	}

	dword Size()
	{
		return (ex ? ex : in)->Size();
	}
};
