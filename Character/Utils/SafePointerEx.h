
#ifndef _SafePointerEx_h_
#define _SafePointerEx_h_

#include "..\..\Common_h\mission.h"


template<class T> class MOSafePointerTypeEx
{
public:
	//Проверить указатель на корректность
	__forceinline bool Validate()
	{
		return ptr.Validate();
	}

	//Сбросить указатель
	__forceinline void Reset()
	{
		ptr.Reset();
	}

	//Получить указатель на объект
	__forceinline T * Ptr(){ return (T *)ptr.Ptr(); };

	//Получить указатель на объект с проверкой
	__forceinline T * SPtr(){ return (T *)ptr.SPtr(); };

	//Получить сэйфпоинтер
	__forceinline MOSafePointer & GetSPObject()
	{
		return ptr;
	}

	__forceinline bool FindObject(IMission* mission, const ConstString & id, const ConstString & type_name)
	{
		if (mission->FindObject(id,ptr))				
		{
			if (ptr.SPtr()->Is(type_name))
			{								
				return true;
			}
		}

		ptr.Reset();
		return false;
	}

private:
	MOSafePointer ptr;
};

#endif


