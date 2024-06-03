#ifndef NX_CHARACTER_ALLOCATOR
#define NX_CHARACTER_ALLOCATOR
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

#include "Nx.h"
#include "NxUserAllocator.h"

class CCTAllocator
	{
	public:
		NX_INLINE void* malloc(size_t size, NxMemoryType type)
			{
			return mAllocator->malloc(size, type);
			}

		NX_INLINE void* mallocDEBUG(size_t size, const char* fileName, int line, const char* className, NxMemoryType type)
			{
			return mAllocator->mallocDEBUG(size, fileName, line, className, type );
			}

		NX_INLINE void* realloc(void* memory, size_t size)
			{
			return mAllocator->realloc( memory, size );
			}

		NX_INLINE void free(void* memory)
			{
			if(memory)	mAllocator->free(memory);
			}

	static	NxUserAllocator*	mAllocator;
	};

#endif

