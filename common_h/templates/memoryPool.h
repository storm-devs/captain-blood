#ifndef TEMPLATES_MEMORY_POOL
#define TEMPLATES_MEMORY_POOL


#include "tcommon.h"
#include "array.h"


//Непроинициализированная память
#define MEMORY_POOL_STATE_NOT_INITED 0

//Используемая память
#define MEMORY_POOL_STATE_USED 1

//Удаленная память
#define MEMORY_POOL_STATE_RELEASED 2



template<class _Ty, int _chunkSize>
class objectsPool
{
	const char * m_SourceFile;
	dword m_SourceLine;

	dword m_PhysicalAllocationsCount;
	dword m_AllocationsCount;
	dword m_ReleasesCount;
	dword m_MemoryUsed;

	dword m_ChunkSize;

	dword m_InfoSize;
	dword m_ObjectSizeOf;


	

	struct chuckData
	{
		byte* element_state;
		void* array_ptr;
		dword dwUsedElementsCount;

		chuckData()
		{
			element_state = NULL;
			array_ptr = NULL;
			dwUsedElementsCount = 0;
		}
	};

	array<chuckData> chunks;


	struct informationBlock
	{
		dword chunkIdx;
		dword elementIdx;
	};


	struct freeItem
	{
		dword chunkIdx;
		dword elementIdx;
		_Ty* ptr;
	};

	array<freeItem> freeItems;

public:


	

		/// Узнать кол-во удаленных объектов - для профайла
	__forceinline dword GetReleasesCount ()
	{
		return m_ReleasesCount;
	}

	/// Узнать кол-во выделенных объектов - для профайла
	__forceinline dword GetAllocationsCount ()
	{
		return m_AllocationsCount;
	}

	/// Узнать кол-во физический аллокаций памяти - для профайла
	/// (не совсем точное, не учитывает аллокации в массиве хранилище удаленных указателей и аллокации в chucks дате)
	__forceinline dword GetPhysicalAllocationsCount ()
	{
		return m_PhysicalAllocationsCount;
	}

	

	/// Узнать кол-во использованной памяти - для профайла
	__forceinline dword GetMemUsed ()
	{
		return m_MemoryUsed;
	}

	///Конструктор
	objectsPool() : chunks (__FILE__, __LINE__, 32),
                  freeItems (__FILE__, __LINE__, _chunkSize)
	{
		m_SourceFile = __FILE__;
		m_SourceLine = __LINE__;

		m_ChunkSize = _chunkSize;
		m_AllocationsCount = 0;
		m_ReleasesCount = 0;
		m_PhysicalAllocationsCount = 0;
		m_MemoryUsed = 0;


		m_ObjectSizeOf = sizeof(_Ty);
		m_InfoSize = sizeof(informationBlock);

		m_ObjectSizeOf += m_InfoSize;
		
	}

	///Деструктор
	~objectsPool()
	{
		Destroy();
	}

	///Удалить всю память
	void Destroy()
	{
		for (dword i = 0; i < chunks.Size(); i++)
		{
			byte* elementsState = chunks[i].element_state;
			_Ty* arrayPtr = (_Ty*)chunks[i].array_ptr;
			dword dwElementsCount = chunks[i].dwUsedElementsCount;

			for (dword n = 0; n < dwElementsCount; n++)
			{
				void * p = ((byte *)arrayPtr + (m_ObjectSizeOf * n));
				_Ty* ptr = (_Ty*)((byte*)p + m_InfoSize);

				if (elementsState[n] == MEMORY_POOL_STATE_USED)
				{
					ptr->~_Ty();
				}
					
			}


			free(chunks[i].array_ptr);
			free(chunks[i].element_state);
		}

		chunks.Free();
		freeItems.Free();

		m_AllocationsCount = 0;
		m_ReleasesCount = 0;
		m_PhysicalAllocationsCount = 0;
		m_MemoryUsed = 0;
	}



	///Выделить элемент из пула
	__forceinline _Ty* Allocate()
	{
		//Если есть среди удаленных элементов свободные - берем из них
		long elementCount = (long)freeItems.Size();
		if (elementCount > 0)
		{
			freeItem & free = freeItems[elementCount-1];

			chunks[free.chunkIdx].element_state[free.elementIdx] = MEMORY_POOL_STATE_USED;
			
			_Ty* newElement = free.ptr;
			
			freeItems.ExtractNoShift(elementCount-1);

			//Переинициализируем
			new('a', newElement) _Ty;

			m_AllocationsCount++;

			return newElement;
		}

		long lastChunk = (long)chunks.Size();
		lastChunk = lastChunk - 1;

		//Если есть чанки и в последнем чанке еще пустые элементы берем из чанка
		if (lastChunk >= 0)
		{
			chuckData & data = chunks[lastChunk];

			if (data.dwUsedElementsCount < m_ChunkSize)
			{
				chunks[lastChunk].element_state[data.dwUsedElementsCount] = MEMORY_POOL_STATE_USED;

				void * p = ((byte *)data.array_ptr + (m_ObjectSizeOf * data.dwUsedElementsCount));

				informationBlock* infoBlock = (informationBlock*)(p);
				infoBlock->chunkIdx = lastChunk;
				infoBlock->elementIdx = data.dwUsedElementsCount; 

				_Ty* newElement = (_Ty*)((byte*)p + m_InfoSize);

				data.dwUsedElementsCount++;

				//Переинициализируем
				new('a', newElement) _Ty;

				m_AllocationsCount++;

				return newElement;
			}
		}


		chuckData & data = chunks[chunks.Add()];

		

		data.array_ptr = NULL;
		

		data.array_ptr = realloc(data.array_ptr, m_ObjectSizeOf * m_ChunkSize);
		data.element_state = (byte*)realloc(data.element_state, sizeof(byte) * m_ChunkSize);


		memset(data.element_state, MEMORY_POOL_STATE_NOT_INITED, sizeof(byte) * m_ChunkSize);

		m_PhysicalAllocationsCount++;

		chunks[(chunks.Size()-1)].element_state[0] = MEMORY_POOL_STATE_USED;

		void * p = ((byte *)data.array_ptr);

		informationBlock* infoBlock = (informationBlock*)(p);
		infoBlock->chunkIdx = (chunks.Size()-1);
		infoBlock->elementIdx = 0; 

		_Ty* newElement = (_Ty*)((byte*)p + m_InfoSize);

		data.dwUsedElementsCount = 1;

		//Переинициализируем
		new('a', newElement) _Ty;

		m_AllocationsCount++;
		m_MemoryUsed += m_ObjectSizeOf * m_ChunkSize;

		return newElement;
	}

	///Удалить элемент из пула
	__forceinline void Free(_Ty * ptr)
	{
		if (ptr == NULL) return;

		informationBlock* infoBlock = (informationBlock*)((byte*)ptr - m_InfoSize);
		
		chunks[infoBlock->chunkIdx].element_state[infoBlock->elementIdx] = MEMORY_POOL_STATE_RELEASED;
		
		m_ReleasesCount++;

		//Деструктор дергаем
		ptr->~_Ty();

		//Добавляем в список мертвых
		freeItem & item = freeItems[freeItems.Add()];
		item.ptr = ptr;
		item.chunkIdx = infoBlock->chunkIdx;
		item.elementIdx = infoBlock->elementIdx;
		

	}


};


#endif