#ifndef NX_FOUNDATION_NXALLOCATOR_DEFAULT
#define NX_FOUNDATION_NXALLOCATOR_DEFAULT
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

#include "Nx.h"
#include "NxUserAllocator.h"

#include <stdlib.h>

#if defined(WIN32) && NX_DEBUG_MALLOC
	#include <crtdbg.h>
#endif
/**
\brief Default memory allocator using standard C malloc / free / realloc.
*/
class NxAllocatorDefault	
	{
	public:
		/**
		Allocates size bytes of memory.

		Compatible with the standard C malloc().

		\param size Number of bytes to allocate.
		\param type A hint about what the memory will be used for. See #NxMemoryType.
		*/
		NX_INLINE void* malloc(size_t size, NxMemoryType type)
			{
			NX_UNREFERENCED_PARAMETER(type);
			return ::malloc(size);
			}

		/**
		Allocates size bytes of memory.

		Same as above, but with extra debug info fields.

		\param size Number of bytes to allocate.
		\param fileName File which is allocating the memory.
		\param line Line which is allocating the memory.
		\param className Name of the class which is allocating the memory.
		\param type A hint about what the memory will be used for. See #NxMemoryType.
		*/
		NX_INLINE void* mallocDEBUG(size_t size, const char* fileName, int line, const char* className, NxMemoryType type)
			{
			NX_UNREFERENCED_PARAMETER(type);
			NX_UNREFERENCED_PARAMETER(className);
#ifdef _DEBUG
	#if defined(WIN32) && NX_DEBUG_MALLOC
			return ::_malloc_dbg(size, _NORMAL_BLOCK, fileName, line);
	#else
			NX_UNREFERENCED_PARAMETER(fileName);
			NX_UNREFERENCED_PARAMETER(line);
	// TODO: insert a Linux Debugger Function
			return ::malloc(size);
	#endif
#else
			NX_UNREFERENCED_PARAMETER(fileName);
			NX_UNREFERENCED_PARAMETER(line);
			NX_UNREFERENCED_PARAMETER(size);
			NX_ASSERT(0);//Don't use debug malloc for release mode code!
			return 0;
#endif
			}

		/**
		Resizes the memory block previously allocated with malloc() or
		realloc() to be size() bytes, and returns the possibly moved memory.

		Compatible with the standard C realloc().

		\param memory Memory block to change the size of.
		\param size New size for memory block.
		*/
		NX_INLINE void* realloc(void* memory, size_t size)
			{
			return ::realloc(memory,size);
			}

		/**
		Frees the memory previously allocated by malloc() or realloc().

		Compatible with the standard C free().

		\param memory Memory to free.
		*/
		NX_INLINE void free(void* memory)
			{
			if(memory)	::free(memory);	// Deleting null ptrs is valid, but still useless
			}

		/**
		Check a memory block is valid.
		*/
		NX_INLINE void check(void* memory)
			{
			NX_UNREFERENCED_PARAMETER(memory);
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
