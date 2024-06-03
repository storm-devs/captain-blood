#ifndef NX_FOUNDATION_NXUSER_ALLOCATOR_DEFAULT
#define NX_FOUNDATION_NXUSER_ALLOCATOR_DEFAULT
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/
/** \addtogroup foundation
  @{
*/

#include "NxUserAllocator.h"
#include "Nx.h"

#include <stdlib.h>

#if defined(WIN32) && NX_DEBUG_MALLOC
  #include <crtdbg.h>
#endif

/**
\brief Default implementation of memory allocator using standard C malloc / free / realloc.

See #NxUserAllocator
*/
class NxUserAllocatorDefault : public NxUserAllocator
	{
	public:
		/**
		\brief Allocates size bytes of memory.

		Compatible with the standard C malloc().
		*/
		void* malloc(size_t size, NxMemoryType type)
			{
			return ::malloc(size);
			}
		void* malloc(size_t size)
			{
			return ::malloc(size);
			}

		/**
		\brief Allocates size bytes of memory.

		Same as above, but with extra debug info fields.
		*/
		void* mallocDEBUG(size_t size, const char* fileName, int line, const char* className, NxMemoryType type)
			{
#ifdef _DEBUG
	#if defined(WIN32) && NX_DEBUG_MALLOC
			return ::_malloc_dbg(size, _NORMAL_BLOCK, fileName, line);
	#else
			return ::malloc(size);
	#endif
#else
			NX_ASSERT(0);//Don't use debug malloc for release mode code!
			return 0;
#endif
			}
		void* mallocDEBUG(size_t size, const char* fileName, int line)
			{
#ifdef _DEBUG
	#if defined(WIN32) && NX_DEBUG_MALLOC
			return ::_malloc_dbg(size, _NORMAL_BLOCK, fileName, line);
	#else
			return ::malloc(size);
	#endif
#else
			NX_ASSERT(0);//Don't use debug malloc for release mode code!
			return 0;
#endif
			}

		/**
		\brief Resizes the memory block previously allocated with malloc() or
		realloc() to be size() bytes, and returns the possibly moved memory.

		Compatible with the standard C realloc().
		*/
		void* realloc(void* memory, size_t size)
			{
			return ::realloc(memory,size);
			}

		/**
		\brief Frees the memory previously allocated by malloc() or realloc().

		Compatible with the standard C free().
		*/
		void free(void* memory)
			{
			::free(memory);
			}

		void check()
		{
#if defined(WIN32) && defined(_DEBUG)
			_CrtCheckMemory();
#endif
		}
	};

 /** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
