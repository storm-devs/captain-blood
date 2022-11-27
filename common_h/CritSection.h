#ifndef _CritSection_h_
#define _CritSection_h_


#ifndef _XBOX
#define NOMINMAX
#include <windows.h>
#else
#include <xtl.h>
#endif

//-----------------------------------------------------------------------------------------------------------
//Макрос для синхронизации функции
//-----------------------------------------------------------------------------------------------------------

#define SingleThread	static CritSection  __local_criticle_section__; SyncroCode __local_thread_safe_object__(__local_criticle_section__);

//-----------------------------------------------------------------------------------------------------------
//Макросы для синхронизации класса
//-----------------------------------------------------------------------------------------------------------

//Объявляется в классе
#define ClassThread CritSection  __object_criticle_section__;

//Объявляется в синхронизируемом методе класса
#define SingleClassThread	SyncroCode __local_class_thread_safe_object__(__object_criticle_section__);

//Объявляется в синхронизируемом методе класса
#define SingleExClassThread(classPointer)	SyncroCode __local_class_thread_safe_object__(classPointer->__object_criticle_section__);

//Залочить поток класса на себя
#define ClassThreadLock  __object_criticle_section__.Enter();
//Освободить поток класса
#define ClassThreadUnlock  __object_criticle_section__.Leave();

//-----------------------------------------------------------------------------------------------------------
//Класс-обёртка для критической секции
//-----------------------------------------------------------------------------------------------------------

class CritSection
{
public:
	CritSection()
	{
		::InitializeCriticalSection(&section);
	}
	~CritSection()
	{
		::LeaveCriticalSection(&section);
		DeleteCriticalSection(&section);
	}

	inline void Enter() const
	{
		::EnterCriticalSection(&section);
	}

	inline bool TryEnter() const
	{
		return ::TryEnterCriticalSection(&section) != 0;
	}

	inline void Leave() const
	{
		::LeaveCriticalSection(&section);
	}

private:
	mutable CRITICAL_SECTION section;
};


//-----------------------------------------------------------------------------------------------------------
//Класс для автоматического входа-выхода из критических секций
//-----------------------------------------------------------------------------------------------------------

class SyncroCode
{
public:
	SyncroCode(const CritSection & critSectionObject) : section(critSectionObject)
	{
		section.Enter();
	};

	~SyncroCode()
	{
		section.Leave();
	};

private:
	const CritSection & section;
};


#endif
