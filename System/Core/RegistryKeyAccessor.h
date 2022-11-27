
/*
	Формат регистрационного ключа для быстрого поиска:

		0 e s f rrrr.rrrr oooo.oooo gggg.gggg.gggg

			0 - контроль валидности ключа
			e 1 - используется в списке исполнения
			s 1 - зарегестрированный сервис, иначе объект
			f 1 - используется в списке регистрации
			r 8 bit - младший байт индекса деклоратора
			o 8 bit - младший байт индекса объекта данного типа
			g 12 bit - маладшая часть индекса в глобальном списке
*/



#ifndef _RegistryKeyAccessor_h_
#define _RegistryKeyAccessor_h_


#include "..\..\common_h\core.h"


class RegistryKeyAccessor
{
protected:
	virtual dword & RegistryKey(RegObject * obj) = null;

public:
	__forceinline bool IsValidate(RegObject * obj)
	{
		return !(RegistryKey(obj) & 0x80000000);
	};

	__forceinline void SetRegistryFlag(RegObject * obj)
	{
		RegistryKey(obj) |= 0x10000000;
	};

	__forceinline bool GetRegistryFlag(RegObject * obj)
	{
		return (RegistryKey(obj) & 0x10000000) != 0;
	};

	__forceinline void SetServiceFlag(RegObject * obj)
	{
		RegistryKey(obj) |= 0x20000000;
	};

	__forceinline bool GetServiceFlag(RegObject * obj)
	{
		return (RegistryKey(obj) & 0x20000000) != 0;
	};

	__forceinline void SetExecuteFlag(RegObject * obj)
	{
		RegistryKey(obj) |= 0x40000000;
	};

	__forceinline bool GetExecuteFlag(RegObject * obj)
	{
		return (RegistryKey(obj) & 0x40000000) != 0;
	};

	__forceinline void SetGlobalIndex(RegObject * obj, dword index)
	{
		dword & key = RegistryKey(obj);
		key = (key & 0xfffff000) | (index & 0xfff);
	};

	__forceinline dword GetGlobalIndex(RegObject * obj)
	{
		dword & key = RegistryKey(obj);
		return key & 0xfff;
	};

	static __forceinline dword GetGlobalIndexStep()
	{
		return 0x1000;
	};

	__forceinline void SetObjectIndex(RegObject * obj, dword index)
	{
		dword & key = RegistryKey(obj);
		key = (key & 0xfff00fff) | ((index & 0xff) << 12);
	};

	__forceinline dword GetObjectIndex(RegObject * obj)
	{
		dword & key = RegistryKey(obj);
		return (key >> 12) & 0xff;
	};

	static __forceinline dword GetObjectIndexStep()
	{		
		return 0x100;
	};

	__forceinline void SetRegIndex(RegObject * obj, dword index)
	{
		dword & key = RegistryKey(obj);
		key = (key & 0xf00fffff) | ((index & 0xff) << 20);
	};

	__forceinline dword GetRegIndex(RegObject * obj)
	{
		dword & key = RegistryKey(obj);
		return (key >> 20) & 0xff;
	};

	static __forceinline dword GetRegIndexStep()
	{
		return 0x100;
	};

	__forceinline void Reset(RegObject * obj)
	{
		RegistryKey(obj) = 0;
	}

	__forceinline void InvalidateKey(RegObject * obj)
	{
		RegistryKey(obj) = -1;
	}

	 __forceinline dword GetRegistryKey(RegObject * obj)
	 {
		 return RegistryKey(obj);
	 }

};


#endif

