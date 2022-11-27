#pragma once

// интерфейс отложенных вызовов
template <typename O>
class IDeferrer
{
public:
	virtual ~IDeferrer() {}

	virtual void Call(O*) = null;
	virtual void Trace(string & str) {};

public:
};

// хелпер шаблон выбора одного из двух типов на основе логического
// значения вычисляемого на этапе компиляции
template<typename T, typename U, bool C> struct type_select
{
	typedef T result;
};

template<typename T, typename U> struct type_select<T,U,false>
{
	typedef U result;
};

// хелпер-шаблон для определения является ли параметр шаблона абстрактным типом
template<typename T>
struct is_abstract
{
private:
	struct yes_type { char c; };
	struct no_type { char c[2]; };

	template<class U>
	static no_type check_sig(U (*)[1]);
	template<class U>
	static yes_type check_sig(...);

	static const unsigned int s1 = sizeof(check_sig<T>(0));
public:
	static const bool result = (s1 == sizeof(yes_type));
};

// хелпер-шаблон для пробразования ссылки в чистый тип
// если это можно (нельзя например ссылки на абстрактный тип, ссылки на array<> )
template <typename T>
struct remove_ref_if_can
{
	typedef T result;
};

template <typename T>
struct remove_ref_if_can<T&>
{
	typedef typename type_select<T&, T, is_abstract<T>::result>::result result;
};


// специальные случаи (исключения из правил) - когда нельзя сохранять типы по значению (либо дорого, либо просто нельзя)
//////////////////////////////////////////////////////////////////////////
template <>
struct remove_ref_if_can<const array<unsigned int>&>
{
	typedef const array<unsigned int>& result;
};

template <>
struct remove_ref_if_can<array<byte>&>
{
	typedef array<byte>& result;
};
//////////////////////////////////////////////////////////////////////////


// хелпер-шаблон для преобразования указателя в чистый тип,
template <typename T>
struct remove_pointer
{
	typedef T result;
};

template <typename T>
struct remove_pointer<T*>
{
	typedef T result;
};

// хелпер-шаблон для преобразования указателя в чистый тип,
// если можно ( нельзя например указатель на абстрактный тип, нельзя const char* и т.п.)
template <typename T>
struct remove_ptr_if_can
{
	typedef T result;
};

template <typename T>
struct remove_ptr_if_can<T*>
{
	typedef typename type_select<T*, T, is_abstract<T>::result>::result result;
};

// шаблон-конвертатор ссылки в указатель
template<typename Source, typename Target>
struct value_to_pointer
{
	static Target convert(Source & src) { return src; }
};

template<typename Source, typename Target>
struct value_to_pointer<Source, Target*>
{
	static Target* convert(Source & src) { return &src; }
};

template<typename Source, typename Target>
struct value_to_pointer<Source&, Target&>
{
	static Target& convert(Source& src) { return src; }
};

template<typename Source, typename Target>
struct value_to_pointer<Source*, Target*>
{
	static Target* convert(Source* src) { return src; }
};

// шаблон для преобразования указателя в ссылку если это не абстрактный тип
// схема работы
// 1. это если тип Source не указатель - соотв. тогда он не конвертируется
template<typename Source>
struct pointer_to_value
{
	static Source& convert(Source& src) { return src; }
};
// 2. это если тип Source - ссылка - нужно для того чтобы не создавались ссылки
// на ссылки, которые запрещены в языке
template<typename Source>
struct pointer_to_value<Source&>
{
	static Source& convert(Source& src) { return src; }
};

// 3. это самый сложный случай. если Source указатель и если это не указатель
// на абстрактный класс (который как известно по значению нельзя использовать)
// то разыменовываем указатель, если класс абстрактный - то оставляем его указателем
template<typename Source>
struct pointer_to_value<Source*>
{
	// константа времени компиляции - определеяет требуется конвертирование типа или нет
	static const bool needConvert = is_abstract<remove_pointer<Source>::result>::result;
	// тип возвращаемый функций pointer_to_value::convert()
	// (либо указатель, либо ссылка в зависимости от значения needConvert)
	typedef typename type_select<Source*, Source&, needConvert>::result ReturnType;

	// хелперный класс - общий вид - случай когда надо конвертировать
	template <typename U, bool doNotConvert>
	struct convert_helper
	{
		static ReturnType convert(U src) { return *src; }
	};

	// хелперный класс - специализация для случая когда не надо конвертировать
	template <typename U>
	struct convert_helper<U, true>
	{
		static ReturnType convert(U src) { return src; }
	};

	// функция-конвертор
	static ReturnType convert(Source* src)
	{
		return convert_helper<Source*, needConvert>::convert(src);
	}
};


// шаблон класса отложенного вызова для функции без аргументов
template <typename O, typename R>
class Deferrer0 : public IDeferrer<O>
{
public:
	typedef R (O::*FuncType)();

	Deferrer0(const char * funcName, FuncType func) : 
		m_funcName(funcName),
		m_func(func) {}
	virtual void Call(O* obj)
	{
		(obj->*m_func)();
	}

	virtual void Trace(string & str)
	{
		str += m_funcName; str += "();";
	}

private:
	FuncType m_func;
	const char * m_funcName;
};

#ifndef STOP_DEBUG
	void DeferrerTrace(string & str, dword value);
	void DeferrerTrace(string & str, long value);
	void DeferrerTrace(string & str, int value);
	void DeferrerTrace(string & str, unsigned int value);
	void DeferrerTrace(string & str, float value);
	void DeferrerTrace(string & str, const Vector & value);
	void DeferrerTrace(string & str, const Matrix & value);
	void DeferrerTrace(string & str, const Vector4 & value);
	void DeferrerTrace(string & str, bool value);
	void DeferrerTrace(string & str, const Plane & value);
	void DeferrerTrace(string & str, PhysicsCollisionGroup value);
	void DeferrerTrace(string & str, IPhysMaterial * material);
	void DeferrerTrace(string & str, IAnimation * animation);
	void DeferrerTrace(string & str, IPhysRigidBody & body);
#endif

// шаблон класса отложенного вызова для функции с 1-м аргументом
template <typename O, typename R, typename P1>
class Deferrer1 : public IDeferrer<O>
{
public:
	typedef R (O::*FuncType)(P1);

	Deferrer1(const char * funcName, FuncType func, P1 par1 ) : 
		m_funcName(funcName), 
		m_func(func), 
		m_param1(pointer_to_value<P1>::convert(par1)) {}
	virtual void Call(O* obj)
	{
		(obj->*m_func)(value_to_pointer<INNER_P1, P1>::convert(m_param1));
	}

#ifndef STOP_DEBUG
	virtual void Trace(string & str)
	{
		str += m_funcName; str += "(";
		DeferrerTrace(str, m_param1);
		str += ");";
	}
#endif

private:
	FuncType	m_func;
	const char * m_funcName;
	typedef typename remove_ptr_if_can<typename remove_ref_if_can<P1>::result>::result INNER_P1;
	INNER_P1 m_param1;
};

// шаблон класса отложенного вызова для функции с 2-мя аргументами
template <typename O, typename R, typename P1, typename P2>
class Deferrer2 : public IDeferrer<O>
{
public:
	typedef R (O::*FuncType)(P1, P2);

	Deferrer2(const char * funcName, FuncType func, P1 par1, P2 par2) :
		m_funcName(funcName), 
		m_func(func),
		m_param1(pointer_to_value<P1>::convert(par1)),
		m_param2(pointer_to_value<P2>::convert(par2)) {}
	virtual void Call(O* obj)
	{
		(obj->*m_func)(	value_to_pointer<INNER_P1, P1>::convert(m_param1),
						value_to_pointer<INNER_P2, P2>::convert(m_param2));
	}

#ifndef STOP_DEBUG
	virtual void Trace(string & str)
	{
		str += m_funcName; str += "(";
		DeferrerTrace(str, m_param1); str += ", ";
		DeferrerTrace(str, m_param2);
		str += ");";
	}
#endif

private:
	FuncType	m_func;
	const char * m_funcName;
	typedef typename remove_ptr_if_can<typename remove_ref_if_can<P1>::result>::result INNER_P1;
	typedef typename remove_ptr_if_can<typename remove_ref_if_can<P2>::result>::result INNER_P2;
	INNER_P1			m_param1;
	INNER_P2			m_param2;
};

// шаблон класса отложенного вызова для функции с 3-мя аргументами
template <typename O, typename R, typename P1, typename P2, typename P3>
class Deferrer3 : public IDeferrer<O>
{
public:
	typedef R (O::*FuncType)(P1, P2, P3);

	Deferrer3(const char * funcName, FuncType func, P1 par1, P2 par2, P3 par3) :
		m_funcName(funcName), 
		m_func(func),
		m_param1(pointer_to_value<P1>::convert(par1)),
		m_param2(pointer_to_value<P2>::convert(par2)),
		m_param3(pointer_to_value<P3>::convert(par3)) {}
	virtual void Call(O* obj)
	{
		(obj->*m_func)(	value_to_pointer<INNER_P1, P1>::convert(m_param1),
						value_to_pointer<INNER_P2, P2>::convert(m_param2),
						value_to_pointer<INNER_P3, P3>::convert(m_param3));
	}

#ifndef STOP_DEBUG
	virtual void Trace(string & str)
	{
		str += m_funcName; str += "(";
		DeferrerTrace(str, m_param1); str += ", ";
		DeferrerTrace(str, m_param2); str += ", ";
		DeferrerTrace(str, m_param3);
		str += ");";
	}
#endif

private:
	FuncType	m_func;
	const char * m_funcName;
	typedef typename remove_ptr_if_can<typename remove_ref_if_can<P1>::result>::result INNER_P1;
	typedef typename remove_ptr_if_can<typename remove_ref_if_can<P2>::result>::result INNER_P2;
	typedef typename remove_ptr_if_can<typename remove_ref_if_can<P3>::result>::result INNER_P3;
	INNER_P1			m_param1;
	INNER_P2			m_param2;
	INNER_P3			m_param3;
};

// шаблон класса отложенного вызова для функции с 4-мя аргументами
template <typename O, typename R, typename P1, typename P2, typename P3, typename P4>
class Deferrer4 : public IDeferrer<O>
{
public:
	typedef R (O::*FuncType)(P1, P2, P3, P4);

	Deferrer4(const char * funcName, FuncType func, P1 par1, P2 par2, P3 par3, P4 par4) :
		m_funcName(funcName), 
		m_func(func),
		m_param1(pointer_to_value<P1>::convert(par1)),
		m_param2(pointer_to_value<P2>::convert(par2)),
		m_param3(pointer_to_value<P3>::convert(par3)),
		m_param4(pointer_to_value<P4>::convert(par4)) {}
	virtual void Call(O* obj)
	{
		(obj->*m_func)(	value_to_pointer<INNER_P1, P1>::convert(m_param1),
						value_to_pointer<INNER_P2, P2>::convert(m_param2),
						value_to_pointer<INNER_P3, P3>::convert(m_param3),
						value_to_pointer<INNER_P4, P4>::convert(m_param4));
	}

#ifndef STOP_DEBUG
	virtual void Trace(string & str)
	{
		str += m_funcName; str += "(";
		DeferrerTrace(str, m_param1); str += ", ";
		DeferrerTrace(str, m_param2); str += ", ";
		DeferrerTrace(str, m_param3); str += ", ";
		DeferrerTrace(str, m_param4);
		str += ");";
	}
#endif

private:
	FuncType	m_func;
	const char * m_funcName;
	typedef typename remove_ptr_if_can<typename remove_ref_if_can<P1>::result>::result INNER_P1;
	typedef typename remove_ptr_if_can<typename remove_ref_if_can<P2>::result>::result INNER_P2;
	typedef typename remove_ptr_if_can<typename remove_ref_if_can<P3>::result>::result INNER_P3;
	typedef typename remove_ptr_if_can<typename remove_ref_if_can<P4>::result>::result INNER_P4;
	INNER_P1			m_param1;
	INNER_P2			m_param2;
	INNER_P3			m_param3;
	INNER_P4			m_param4;
};


// шаблон класса отложенного вызова для функции с 5-ю аргументами
template <typename O, typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
class Deferrer5 : public IDeferrer<O>
{
public:
	typedef R (O::*FuncType)(P1, P2, P3, P4, P5);

	Deferrer5(const char * funcName, FuncType func, P1 par1, P2 par2, P3 par3, P4 par4, P5 par5) :
		m_funcName(funcName), 
		m_func(func),
		m_param1(pointer_to_value<P1>::convert(par1)),
		m_param2(pointer_to_value<P2>::convert(par2)),
		m_param3(pointer_to_value<P3>::convert(par3)),
		m_param4(pointer_to_value<P4>::convert(par4)),
		m_param5(pointer_to_value<P5>::convert(par5)) {}
	virtual void Call(O* obj)
	{
		(obj->*m_func)(	value_to_pointer<INNER_P1, P1>::convert(m_param1),
						value_to_pointer<INNER_P2, P2>::convert(m_param2),
						value_to_pointer<INNER_P3, P3>::convert(m_param3),
						value_to_pointer<INNER_P4, P4>::convert(m_param4),
						value_to_pointer<INNER_P5, P5>::convert(m_param5));
	}

#ifndef STOP_DEBUG
	virtual void Trace(string & str)
	{
		str += m_funcName; str += "(";
		DeferrerTrace(str, m_param1); str += ", ";
		DeferrerTrace(str, m_param2); str += ", ";
		DeferrerTrace(str, m_param3); str += ", ";
		DeferrerTrace(str, m_param4); str += ", ";
		DeferrerTrace(str, m_param5);
		str += ");";
	}
#endif

private:
	FuncType	m_func;
	const char * m_funcName;
	typedef typename remove_ptr_if_can<typename remove_ref_if_can<P1>::result>::result INNER_P1;
	typedef typename remove_ptr_if_can<typename remove_ref_if_can<P2>::result>::result INNER_P2;
	typedef typename remove_ptr_if_can<typename remove_ref_if_can<P3>::result>::result INNER_P3;
	typedef typename remove_ptr_if_can<typename remove_ref_if_can<P4>::result>::result INNER_P4;
	typedef typename remove_ptr_if_can<typename remove_ref_if_can<P5>::result>::result INNER_P5;

	INNER_P1			m_param1;
	INNER_P2			m_param2;
	INNER_P3			m_param3;
	INNER_P4			m_param4;
	INNER_P5			m_param5;
};

// шаблон класса отложенного вызова для функции с 6-ю аргументами
template <typename O, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
class Deferrer6 : public IDeferrer<O>
{
public:
	typedef R (O::*FuncType)(P1, P2, P3, P4, P5, P6);

	Deferrer6(const char * funcName, FuncType func, P1 par1, P2 par2, P3 par3, P4 par4, P5 par5, P6 par6) :
		m_funcName(funcName), 
		m_func(func),
		m_param1(pointer_to_value<P1>::convert(par1)),
		m_param2(pointer_to_value<P2>::convert(par2)),
		m_param3(pointer_to_value<P3>::convert(par3)),
		m_param4(pointer_to_value<P4>::convert(par4)),
		m_param5(pointer_to_value<P5>::convert(par5)),
		m_param6(pointer_to_value<P6>::convert(par6)){}
	virtual void Call(O* obj)
	{
		(obj->*m_func)(	value_to_pointer<INNER_P1, P1>::convert(m_param1),
						value_to_pointer<INNER_P2, P2>::convert(m_param2),
						value_to_pointer<INNER_P3, P3>::convert(m_param3),
						value_to_pointer<INNER_P4, P4>::convert(m_param4),
						value_to_pointer<INNER_P5, P5>::convert(m_param5),
						value_to_pointer<INNER_P6, P6>::convert(m_param6));
	}

#ifndef STOP_DEBUG
	virtual void Trace(string & str)
	{
		str += m_funcName; str += "(";
		DeferrerTrace(str, m_param1); str += ", ";
		DeferrerTrace(str, m_param2); str += ", ";
		DeferrerTrace(str, m_param3); str += ", ";
		DeferrerTrace(str, m_param4); str += ", ";
		DeferrerTrace(str, m_param5); str += ", ";
		DeferrerTrace(str, m_param6);
		str += ");";
	}
#endif

private:
	FuncType	m_func;
	const char * m_funcName;
	typedef typename remove_ptr_if_can<typename remove_ref_if_can<P1>::result>::result INNER_P1;
	typedef typename remove_ptr_if_can<typename remove_ref_if_can<P2>::result>::result INNER_P2;
	typedef typename remove_ptr_if_can<typename remove_ref_if_can<P3>::result>::result INNER_P3;
	typedef typename remove_ptr_if_can<typename remove_ref_if_can<P4>::result>::result INNER_P4;
	typedef typename remove_ptr_if_can<typename remove_ref_if_can<P5>::result>::result INNER_P5;
	typedef typename remove_ptr_if_can<typename remove_ref_if_can<P6>::result>::result INNER_P6;

	INNER_P1			m_param1;
	INNER_P2			m_param2;
	INNER_P3			m_param3;
	INNER_P4			m_param4;
	INNER_P5			m_param5;
	INNER_P6			m_param6;
};

class MemPool
{
public:
	MemPool();
	virtual ~MemPool();

	void * Alloc(dword size);
	void Free(void * ptr);
	void Reset();

	static MemPool * ptr;

private:
	char	* buffer;
	word	* list;
	word	lastIndex;
	dword	chunkSize;
	dword	numBlocks;
};


// переопределенный оператор new для деффереров
inline void * operator new(size_t size, char c1, char c2) 
{ 
	return MemPool::ptr->Alloc(dword(size));
	//return api->Reallocate(null, dword(size), _FL_);
}

// переопределенный оператор delete для деффереров
inline void operator delete(void * p, char c1, char c2) 
{
	// сюда не должно попадать
	//_asm int 3
	Assert(false);
}

#define NEW2 new('a', 'b')
//#define NEW2 NEW
//#define def_delete(X)	delete (X)

#ifdef STOP_DEBUG
	#define CONSTRUCT_MCALLS			m_calls(_FL_)
	#define DECLARE_MCALLS(classname)	array<IDeferrer<classname>*>	m_calls;
	#define def_delete(exp)				{ delete_deferrer(exp); }
	#define DELETE_MCALLS				{ for (long i = 0; i < m_calls; ++i) delete_deferrer(m_calls[i]); m_calls.DelAll(); }
	#define TRACE_MCALLS
	#define DEF_FREF(__func)			null, &__func
#else		
	#define MAX_MCALLS					16
	#define CONSTRUCT_MCALLS			m_calls(_FL_), m_copyCalls(_FL_, MAX_MCALLS + 1), m_copyRoller(0)
	#define DECLARE_MCALLS(classname)	array<IDeferrer<classname>*>	m_calls, m_copyCalls; dword m_copyRoller;
	#define def_delete(exp)				{ if (m_copyCalls.Size() < MAX_MCALLS) { m_copyCalls.Add(exp); m_copyRoller = m_copyCalls.Last(); } else { m_copyRoller = (m_copyRoller + 1) % MAX_MCALLS; delete_deferrer(m_copyCalls[m_copyRoller]); m_copyCalls[m_copyRoller] = exp; } }
	#define DELETE_MCALLS				{ for (long i=0; i<m_calls; i++) delete_deferrer(m_calls[i]); m_calls.DelAll(); for (long i=0; i<m_copyCalls; i++) delete_deferrer(m_copyCalls[i]); m_copyCalls.DelAll(); }
	#define TRACE_MCALLS				{ string str; if (m_copyCalls.Len()) { for (dword i=0, idx=0; i<MAX_MCALLS; i++) { dword n = (m_copyRoller + i + 1) % MAX_MCALLS; if (n > m_copyCalls.Last()) continue;	str.Empty(); api->Trace("== %d ================================================================", idx); m_copyCalls[n]->Trace(str);	api->Trace(str.c_str()); idx++;	if (n == m_copyRoller) break; } api->Trace("-- end ----------------------------------------------------------------"); } }
	#define DEF_FREF(__func)			#__func, &__func
#endif

template<class T> inline void delete_deferrer(T * _T)
{
	if (_T) 
		_T->~T();

	MemPool::ptr->Free(_T);
}

// шаблонные функции-хэлперы
template <typename O, typename R>
IDeferrer<O>* MakeDeferrer(const char * funcName, typename Deferrer0<O, R>::FuncType func)
{
//	api->Trace("Deferred call object: size = %d", sizeof(Deferrer0<O, F>));
	return NEW2 Deferrer0<O, R>(funcName, func);
}

template <typename O, typename R, typename P1>
IDeferrer<O>* MakeDeferrer(const char * funcName, typename Deferrer1<O, R, P1>::FuncType func, P1 param1)
{
//	api->Trace("Deferred call object: size = %d", sizeof(Deferrer1<O, F, P1>));
	return NEW2 Deferrer1<O, R, P1>(funcName, func, param1);
}

template <typename O, typename R, typename P1, typename P2>
IDeferrer<O>* MakeDeferrer(const char * funcName, typename Deferrer2<O, R, P1, P2>::FuncType func, P1 param1, P2 param2)
{
//	api->Trace("Deferred call object: size = %d", sizeof(Deferrer2<O, F, P1, P2>));
	return NEW2 Deferrer2<O, R, P1, P2>(funcName, func, param1, param2);
}

template <typename O, typename R, typename P1, typename P2, typename P3>
IDeferrer<O>* MakeDeferrer(const char * funcName, typename Deferrer3<O, R, P1, P2, P3>::FuncType func, P1 param1, P2 param2, P3 param3)
{
//	api->Trace("Deferred call object: size = %d", sizeof(Deferrer3<O, F, P1, P2, P3>));
	return NEW2 Deferrer3<O, R, P1, P2, P3>(funcName, func, param1, param2, param3);
}

template <typename O, typename R, typename P1, typename P2, typename P3, typename P4>
IDeferrer<O>* MakeDeferrer(const char * funcName, typename Deferrer4<O, R, P1, P2, P3, P4>::FuncType func, P1 param1, P2 param2, P3 param3, P4 param4)
{
//	api->Trace("Deferred call object: size = %d", sizeof(Deferrer4<O, F, P1, P2, P3, P4>));
	return NEW2 Deferrer4<O, R, P1, P2, P3, P4>(funcName, func, param1, param2, param3, param4);
}

template <typename O, typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
IDeferrer<O>* MakeDeferrer(const char * funcName, typename Deferrer5<O, R, P1, P2, P3, P4, P5>::FuncType func, P1 param1, P2 param2, P3 param3, P4 param4, P5 param5)
{
//	api->Trace("Deferred call object: size = %d", sizeof(Deferrer5<O, F, P1, P2, P3, P4, P5>));
	return NEW2 Deferrer5<O, R, P1, P2, P3, P4, P5>(funcName, func, param1, param2, param3, param4, param5);
}

template <typename O, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
IDeferrer<O>* MakeDeferrer(const char * funcName, typename Deferrer6<O, R, P1, P2, P3, P4, P5, P6>::FuncType func, P1 param1, P2 param2, P3 param3, P4 param4, P5 param5, P6 param6)
{
//	api->Trace("Deferred call object: size = %d", sizeof(Deferrer5<O, F, P1, P2, P3, P4, P5>));
	return NEW2 Deferrer6<O, R, P1, P2, P3, P4, P5, P6>(funcName, func, param1, param2, param3, param4, param5, param6);
}