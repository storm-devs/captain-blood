#ifndef RING_BUFFER
#define RING_BUFFER


template<long itemCount, class _Ty>
class ringBuffer
{

	_Ty data[itemCount];

	dword readPtr;
	dword writePtr;



	__forceinline void incPtr (dword & ptr)
	{
		ptr++;
		if (ptr >= itemCount) ptr = 0;
	}


	__forceinline dword clampIndex (dword ptr)
	{
		return ptr % itemCount;
	}



	_Ty defaultObject;

public:

	ringBuffer()
	{
		readPtr = 0; //Отсюда забираем
		writePtr = 0; //Сюда добавляем
	}

	~ringBuffer()
	{

	}

	__forceinline void clear()
	{
		readPtr = 0;
		writePtr = 0;
	}

	__forceinline dword size() const
	{
		if (writePtr >= readPtr)
		{
			return writePtr - readPtr;
		}

		return itemCount - (readPtr - writePtr);
	}

	__forceinline long last() const
	{
		return (size() - 1);
	}

	

	void push(const _Ty & _T)
	{  
		data[writePtr] = _T;
		incPtr(writePtr);

		if (writePtr == readPtr)
		{
			//Убиваем старые данные автоматом
			incPtr(readPtr);
		}
	}

	const _Ty & pop ()
	{
		if (readPtr == writePtr)
		{
			//читать нечего, возвращаем объект по умолчанию...
			return defaultObject;
		}

		const _Ty & v = data[readPtr];
		incPtr(readPtr);
		return v;
	}

	//Обращение как к массиву
	__forceinline const _Ty & operator [] (dword dwIndex) const
	{ 
		Assert(dwIndex < size());
		dword dwIdx = clampIndex(readPtr + dwIndex);
		Assert(dwIdx < itemCount);
		return data[dwIdx];
	}

	__forceinline _Ty & operator [] (dword dwIndex)
	{
		Assert(dwIndex < size());
		dword dwIdx = clampIndex(readPtr + dwIndex);
		Assert(dwIdx < itemCount);
		return data[dwIdx];
	}


};

#endif